#pragma once 

#include "RtAudio.h"
#include <cstdlib>
#include <iostream>

void errorCallback (RtAudioErrorType /*type*/, const std::string &errorText)
{
  // This example error handling function simply outputs the error message to stderr.
  std::cerr << "\nerrorCallback: " << errorText << "\n\n";
}

struct DeviceParameters
{
    DeviceParameters(unsigned int blockSize = 512, unsigned int sr = 48000)
      : bufferFrames (blockSize),
        sampleRate (sr)
    {
        inputParameters.deviceId = 0;
        inputParameters.firstChannel = 0;
        inputParameters.nChannels = 1;

        outputParameters.deviceId = 0;
        outputParameters.firstChannel = 0;
        outputParameters.nChannels = 2;
    }
    unsigned int bufferFrames;
    unsigned int sampleRate;
    RtAudio::StreamParameters outputParameters;
    RtAudio::StreamParameters inputParameters;
};

using customCallback = void (*) (float* output, float* input, int bufferSize, int numChannelsIn, int numChannelsOut);

static int rtAudioCallback (void *outputBuffer, void *inputBuffer,
                            unsigned int nFrames, double streamTime,
                            RtAudioStreamStatus status, void *userData); 

class DAC 
{
public:
    DAC (customCallback c, DeviceParameters dp = DeviceParameters())
      : dac (RtAudio::UNSPECIFIED, &errorCallback), 
        callback (c), 
        deviceParameters (dp)
    {
        if (deviceParameters.inputParameters.deviceId == 0)
            deviceParameters.inputParameters.deviceId = dac.getDefaultInputDevice();
        
        if (deviceParameters.outputParameters.deviceId == 0)
            deviceParameters.outputParameters.deviceId = dac.getDefaultOutputDevice();

        std::vector<unsigned int> deviceIds = dac.getDeviceIds();
        if (deviceIds.size() < 1) 
        {
            std::cout << "\nNo audio devices found!\n";
            exit (1);
        }
        dac.showWarnings (true);
        startStream();
    }
    ~DAC() { endStream(); }
    int getNumInputChannels() { return deviceParameters.inputParameters.nChannels; }
    int getNumOutputChannels() { return deviceParameters.outputParameters.nChannels; }
    void startStream()
    {
        if (dac.openStream (&deviceParameters.outputParameters, 
                            nullptr, 
                            RTAUDIO_FLOAT32, 
                            deviceParameters.sampleRate, 
                            &deviceParameters.bufferFrames, 
                            rtAudioCallback, 
                            this))
        {
            std::cout << dac.getErrorText() << std::endl;
            endStream();
            return;
        }
        if (dac.isStreamOpen() == false)
        {
            std::cout << dac.getErrorText() << std::endl;
            endStream();
            return;
        }
        if (dac.startStream())
        {
            std::cout << dac.getErrorText() << std::endl;
            endStream();
            return;
        }
    }
    void endStream()
    {
        if (dac.isStreamOpen())
            dac.closeStream();
    }
    customCallback callback;
private:
    RtAudio dac; 
    DeviceParameters deviceParameters;
};

static int rtAudioCallback(void *outputBuffer, void *inputBuffer,
                           unsigned int nFrames, double streamTime,
                           RtAudioStreamStatus status, void *userData) 
{
    auto* o = static_cast<float*> (outputBuffer);
    auto* i = static_cast<float*> (inputBuffer);
    auto* dac = static_cast<DAC*> (userData);
    if (dac != nullptr)
        dac->callback (o, i, nFrames, dac->getNumInputChannels(), dac->getNumOutputChannels());

    return 0;
} 