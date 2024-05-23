#include <cmath>
#include <string>

#include <cmath>

#include "DAC.hpp"
//=============================================METAL
#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define MTK_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>

#include <simd/simd.h>
//=============================================

DeviceParameters deviceParameters (512, 48000);

double phase = 0.0;
double phaseIncrement = 440.0 * 2.0 * M_PI / static_cast<double> (deviceParameters.sampleRate);

void audioCallback(float* outputBuffer, float* inputBuffer,
                   int numFrames, int numInputChannels, int numOutputChannels) 
{
    for (int i = 0; i < numFrames; i++)
    {
        for (int channel = 0; channel < numOutputChannels; channel++)
            outputBuffer[i * numOutputChannels + channel] = static_cast<float> (std::sin(phase) * 0.1);

        phase += phaseIncrement;
    }
}  

int main()
{  
    deviceParameters.sampleRate = 48000;
    deviceParameters.bufferFrames = 512;
    DAC dac (audioCallback, deviceParameters);
    dac.startStream();

    std::cout << "Enter any input to quit.\n";
    std::string n;
    std::cin >> n;

    return 0;
}

