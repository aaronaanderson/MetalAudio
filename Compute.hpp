#pragma once

//=============================================METAL
#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define MTK_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>

#include <simd/simd.h>

#include "BridgeHeader.h"
//=============================================

class Compute
{
public:
    Compute(int nBufferSamples = 512 * 2)
      : sampleBufferSize (nBufferSamples)
    {
        // build compute pipeline
        device = MTL::CreateSystemDefaultDevice();
        buildComputePipeline (device);
        commandQue = device->newCommandQueue();
    }
    ~Compute()
    {
        computePipelineState->release();
        commandQue->release();
        device->release();
    }
    void executeShader (float* buffer, double phaseStart, double phaseIncrement, int nChannels)
    {

        MTL::CommandBuffer* commandBuffer = commandQue->commandBuffer();
        assert (commandBuffer);
    
        MTL::ComputeCommandEncoder* computeEncoder = commandBuffer->computeCommandEncoder();
    
        computeEncoder->setComputePipelineState (computePipelineState);
        computeEncoder->setBuffer (sampleBuffer, 0, 0);

        Uniforms u = {(float)phaseStart, (float)phaseIncrement, (uint32_t)nChannels};
        MTL::Buffer* uniforms = device->newBuffer (sizeof (Uniforms),MTL::ResourceStorageModeShared);
        computeEncoder->setBuffer (uniforms, 0, 1);
    
        MTL::Size gridSize = MTL::Size( sampleBufferSize, 1, 1 );
    
        NS::UInteger threadGroupSize = computePipelineState->maxTotalThreadsPerThreadgroup();
        MTL::Size threadgroupSize( threadGroupSize, 1, 1 );
    
        computeEncoder->dispatchThreads (gridSize, threadgroupSize);
    
        computeEncoder->endEncoding();
    
        commandBuffer->commit();
        commandBuffer->waitUntilCompleted();

        auto* r = static_cast<float*> (sampleBuffer->contents());
        for (auto i = 0; i < sampleBufferSize; i++)
            buffer[i] = r[i];
    }
private:
    MTL::Device* device;
    MTL::CommandQueue* commandQue;
    MTL::ComputePipelineState* computePipelineState;
    MTL::Buffer* sampleBuffer;
    unsigned int sampleBufferSize;
    MTL::Fence* fence;

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
                
            })";
        NS::Error* error = nullptr;
    
        MTL::Library* computeLibrary = d->newLibrary( NS::String::string(kernelSrc, NS::UTF8StringEncoding), nullptr, &error );
        if ( !computeLibrary )
        {
            __builtin_printf( "%s", error->localizedDescription()->utf8String() );
            assert(false);
        }
    
        MTL::Function* sineFunction = computeLibrary->newFunction( NS::String::string("generateSine", NS::UTF8StringEncoding) );
        computePipelineState = d->newComputePipelineState( sineFunction, &error );
        if ( !computePipelineState )
        {
            __builtin_printf( "%s", error->localizedDescription()->utf8String() );
            assert(false);
        }
    
        sineFunction->release();
        computeLibrary->release();
    }
};