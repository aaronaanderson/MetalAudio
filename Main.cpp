#include "RtAudio.h"
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <string>

//=============================================


typedef float MY_TYPE;
#define FORMAT RTAUDIO_FLOAT32
#define SCALE  1.0

// Platform-dependent sleep routines.
#if defined( WIN32 )
  #include <windows.h>
  #define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds ) 
#else // Unix variants
  #include <unistd.h>
  #define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )
#endif

void errorCallback( RtAudioErrorType /*type*/, const std::string &errorText )
{
  // This example error handling function simply outputs the error message to stderr.
  std::cerr << "\nerrorCallback: " << errorText << "\n\n";
}

RtAudio dac (RtAudio::UNSPECIFIED, &errorCallback);

struct DeviceParameters
{
    DeviceParameters(RtAudio& dac)
    {
        outputParameters.deviceId = dac.getDefaultOutputDevice();
        outputParameters.nChannels = 2;
        outputParameters.firstChannel = 0;
    }
    unsigned int bufferFrames = 512;
    unsigned int sampleRate = 48000;
    RtAudio::StreamParameters outputParameters;
};
DeviceParameters deviceParameters (dac);

double phase = 0.0;
double phaseIncrement = 440.0 * 2.0 * 3.14159265 / static_cast<double> (deviceParameters.sampleRate);

static int audioCallback(void *outputBuffer, void *inputBuffer,
                         unsigned int nFrames, double streamTime,
                         RtAudioStreamStatus status, void *userData) 
{
    float* o = static_cast<float*> (outputBuffer);
    for (int i = 0; i < nFrames; i++)
    {
        for (int channel = 0; channel < 2; channel++)
        {
            o[i * 2 + channel] = static_cast<float> (std::sin(phase) * 0.1);
        }
        phase += phaseIncrement;
    }

    return 0;
}  
void cleanUp()
{
    if (dac.isStreamOpen())
        dac.closeStream();
}

int main()
{
    std::vector<unsigned int> deviceIds = dac.getDeviceIds();
    if (deviceIds.size() < 1) 
    {
        std::cout << "\nNo audio devices found!\n";
        exit( 1 );
    }
    dac.showWarnings (true);

    if (dac.openStream (&deviceParameters.outputParameters, nullptr, FORMAT, deviceParameters.sampleRate, &deviceParameters.bufferFrames, audioCallback))
    {
        std::cout << dac.getErrorText() << std::endl;
        cleanUp();
    }
    if (dac.isStreamOpen() == false)
    {
        std::cout << dac.getErrorText() << std::endl;
        cleanUp();
    }
    if (dac.startStream())
    {
        std::cout << dac.getErrorText() << std::endl;
        cleanUp();
    }

    // std::cout << "Enter any input to quit.\n";
    // std::string n;
    // std::cin >> n;

    std::string str = "";
    char ch;
    while ((ch = std::cin.get()) != 27) {
        SLEEP (100);
        str += ch;
    }

    cleanUp();
    return 0;
}

