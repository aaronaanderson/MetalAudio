#pragma once

#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Metal/Metal.hpp>

#include "BridgeHeader.h"

class Compute
{
public:
    Compute (int nBufferSamples = 512 * 2)
      : sampleBufferSize (nBufferSamples)
    {
        device = MTL::CreateSystemDefaultDevice();
        buildComputePipeline (device);
        
        sampleBuffer = device->newBuffer (sizeof (float) * sampleBufferSize, MTL::ResourceStorageModeShared);
        uniforms = device->newBuffer (sizeof (Uniforms), MTL::ResourceStorageModeShared);
    }
    ~Compute() 
    { 
        releaseResources(); 
    }
    void executeShader (float* audioBuffer, double phaseStart, double phaseIncrement, int nChannels)
    {
        MTL::CommandBuffer* commandBuffer = commandQue->commandBuffer();
        assert (commandBuffer);
    
        MTL::ComputeCommandEncoder* computeEncoder = commandBuffer->computeCommandEncoder();
        
        computeEncoder->setComputePipelineState (computePipelineState);
        computeEncoder->setBuffer (sampleBuffer, 0, 0);

        Uniforms u = {static_cast<float> (phaseStart), 
                      static_cast<float> (phaseIncrement), 
                      static_cast<uint32_t> (nChannels)};
        
        std::memcpy (uniforms->contents(), &u, sizeof (Uniforms));
        computeEncoder->setBuffer (uniforms, 0, 1);
    
        MTL::Size gridSize = MTL::Size (sampleBufferSize / u.numChannels, 1, 1);
    
        NS::UInteger maxSize = computePipelineState->maxTotalThreadsPerThreadgroup();
        if (maxSize > sampleBufferSize / u.numChannels)
            maxSize = sampleBufferSize / u.numChannels;
        MTL::Size threadgroupSize (maxSize, 1, 1);
    
        computeEncoder->dispatchThreads (gridSize, threadgroupSize);    
        computeEncoder->endEncoding();

        commandBuffer->commit();
        commandBuffer->waitUntilCompleted();

        auto* result = static_cast<float*> (sampleBuffer->contents());
        std::memcpy (audioBuffer, result, sizeof (float) * sampleBufferSize);       
    }
private:
    MTL::Device* device;
    MTL::CommandQueue* commandQue;
    MTL::ComputePipelineState* computePipelineState;
    MTL::Buffer* sampleBuffer;
    unsigned int sampleBufferSize;
    MTL::Buffer* uniforms;

    void buildComputePipeline (MTL::Device* d)
    {
        const char* kernelSrc = R"(
            #include <metal_stdlib>
            #import "../BridgeHeader.h"
            using namespace metal;

            kernel void generateSine(device float* result,
                                     device const Uniforms* uniforms,
                                     uint index [[thread_position_in_grid]])
            {
                float phase = uniforms->startPhase + (index * uniforms->phaseIncrement);
                for (uint32_t c = 0; c < uniforms->numChannels; c++)
                {
                    result[index * uniforms->numChannels + c] = sin(phase) * 0.1;
                }
            })";
        NS::Error* error = nullptr;
    
        MTL::Library* computeLibrary = d->newLibrary (NS::String::string (kernelSrc, NS::UTF8StringEncoding), nullptr, &error);
        if (!computeLibrary)
        {
            __builtin_printf ("%s", error->localizedDescription()->utf8String());
            assert(false);
        }
    
        MTL::Function* sineFunction = computeLibrary->newFunction (NS::String::string ("generateSine", NS::UTF8StringEncoding));
        computePipelineState = d->newComputePipelineState (sineFunction, &error);
        if (!computePipelineState)
        {
            __builtin_printf ("%s", error->localizedDescription()->utf8String());
            assert(false);
        }
    
        sineFunction->release();
        computeLibrary->release();

        commandQue = device->newCommandQueue();
    }
    void releaseResources()
    {
        uniforms->release();
        sampleBuffer->release();
        computePipelineState->release();
        commandQue->release();
        device->release();
    }
};