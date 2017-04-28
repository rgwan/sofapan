/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

#include "fftw3.h"
#include "SOFAData.h"
#include "FilterEngine.h"

//Interface für SOFA File:
extern "C" {
#include <netcdf.h>
}

#define ERRCODE 2
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); exit(ERRCODE);}

#ifdef _WIN64
#define SOFA_DEFAULT_PATH "C:\\Program Files\\Steinberg\\VstPlugins\\SOFAFiles\\mit_kemar_normal_pinna.sofa"
#elif _WIN32
#define SOFA_DEFAULT_PATH "C:\\Program Files (x86)\\Steinberg\\VstPlugins\\SOFAFiles\\mit_kemar_normal_pinna.sofa"
#elif __APPLE__
#define SOFA_DEFAULT_PATH "/Library/Audio/Plug-Ins/VST/SOFAFiles/mit_kemar_normal_pinna.sofa"
#endif

#define RE 0
#define IM 1

class HRIR_Measurement {
public:
    HRIR_Measurement(int length);
    ~HRIR_Measurement();
    
    void setValues(float *left, float *right, float elevation, float azimuth, float distance);
    float *getIR_Left();
    float *getIR_Right();
    
private:
    float *IR_Left;
    float *IR_Right;
    float Elevation;
    float Azimuth;
    float Distance;
};


class SofaPanAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    SofaPanAudioProcessor();
    ~SofaPanAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    int LoadSOFAFile();
    int ErrorHandling_LoadSOFAFile(int status);
    void setSOFAFilePath(String sofaString);
    
    //Parameters
    AudioParameterFloat* panParam;
    AudioParameterFloat* elevationParam;
    AudioParameterFloat* bypassParam;
    String pathToSOFAFile = String(SOFA_DEFAULT_PATH);
    sofaMetadataStruct metadata_sofafile;
    void initData(String sofaFile);
    bool updateSofaMetadataFlag;
    
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SofaPanAudioProcessor)
    
    //Zähler für periodische print-ausgaben
    int counter;
    
    /* Buffer wird mit samples gefüllt und der FFT übergeben */
    float* fftInputBuffer;
    
    /* Buffer, der von der iFFT wiederkommt und frisch transformierte Samples beinhält */
    float* fftOutputBufferL;
    float* fftOutputBufferR;
    
    /* Dieser Buffer wird von der forward FFT ausgespuckt und enthät den Fouriertransformierten Block (Magnitude+Phase).
     Dieser wird dann verarbeitet und der iFFT wieder übergeben
     Der Datentyp fftw_complex ist im prinzip nur ein 2-stelliges array von float*/
    fftwf_complex* complexBuffer;
    
    fftwf_complex* src;
    
    /* Der "Plan" ist die Kernkomponente von FFTW. Hier wird zu programmbeginn die auszuführende FFT definiert, die dann einfach mittels "execute(plan)" ausgeführt wird */
    fftwf_plan forward;
    fftwf_plan inverseL;
    fftwf_plan inverseR;
    
    // Buffer for collecting samples for the Fast Convolution
    // everything with a 2 = mirrored version to do crossfades between two HRTF-Convolutions
    AudioBuffer<float> collectSampleBuffer1;
    AudioBuffer<float> collectSampleBuffer2;
    
    // Outputbuffers for the iFFT
    // The A´s and B´s are the overlap-add pairs. The 1´s and 2´s are the two mirrored paths for crossfading
    AudioBuffer<float> overlapAddOutA1;
    AudioBuffer<float> overlapAddOutB1;
    AudioBuffer<float> overlapAddOutA2;
    AudioBuffer<float> overlapAddOutB2;
    
    int fifoIndex, fifoIndex2;
    int fifoOutToBeUsed1, fifoOutToBeUsed2; //0 oder 1
    
    int fir_length;
    int fftLength;
    float scale;
    int complexLength;
    
    void createPassThrough_FIR();
    
    float sampleRate_f;
    
    SOFAData* HRTFs;
    FilterEngine* Filter;
    

    
    
};


#endif  // PLUGINPROCESSOR_H_INCLUDED
