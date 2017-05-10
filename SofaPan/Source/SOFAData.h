/*
  ==============================================================================

    SOFAData.h
    Created: 4 Apr 2017 11:47:12am
    Author:  David Bau

  ==============================================================================
*/

#ifndef SOFAData_H_INCLUDED
#define SOFAData_H_INCLUDED

#include "fftw3.h"
extern "C" {
#include <netcdf.h>
}
#include "string.h"
#include <stdlib.h>
#include "math.h"
#include "../JuceLibraryCode/JuceHeader.h"

#define ERR_MEM_ALLOC   1
#define ERR_READFILE    2
#define ERR_UNKNOWN     3
#define ERR_OPENFILE    4
#define ERR_NOTSUP      5

typedef struct{
    int sampleRate;
    int numMeasurements;
    int numMeasurements0ev;
    int numSamples;
    int numReceivers;
    String dataType;
    String SOFAConventions;
    String listenerShortName;
    float minElevation;
    float maxElevation;
    
    int numberOfGlobalAttributes;
    //char** globalAttributeNames;
    //char** globalAttributeValues;
    Array<String> globalAttributeNames;
    Array<String> globalAttributeValues;
    
} sofaMetadataStruct;


class Single_HRIR_Measurement {
public:
    Single_HRIR_Measurement(int lengthHRIR, int lengthHRTF){
        HRIR = fftwf_alloc_real(lengthHRIR * 2);
        HRTF = fftwf_alloc_complex(lengthHRTF * 2);
        
        
    }
    ~Single_HRIR_Measurement(){
        fftwf_free(HRIR); //not allocated when freed
        fftwf_free(HRTF);
        
    }
    void setValues(float azimuth, float elevation, float distance){
        Elevation = elevation; Azimuth = azimuth; Distance = distance;
    }
    float *getHRIR(){return  HRIR;}
    fftwf_complex* getHRTF(){return HRTF;}
    
    float Elevation;
    float Azimuth;
    float Distance;
    int index;
    
private:
    float* HRIR;
    fftwf_complex* HRTF;
    
};





class SOFAData{
public:
    SOFAData(const char* filePath, int sampleRate);
    ~SOFAData();
    
    /** Will directly return the length of the new resampled HRTF */
    int setSampleRate(int newSampleRate);
    
    /**
     Get the closest HRTF for a given elevation & azimuth. The function will return a pointer to the TF-Values (complex numbers): the left TF first, followed directly by the right TF. The returned HRTF has a length of 2*(HRIRlength/2 + 1): [ L=N/2+1 | R=N/2+1 ].
     */
    fftwf_complex* getHRTFforAngle(float elevation, float azimuth, float radius);
    float* getHRIRForAngle(float elevation, float azimuth, float radius);

    sofaMetadataStruct getMetadata();
    
    /** Returns the length of the interpolated(!) Impulse Response in samples */
    int getLengthOfHRIR();
    
private:
    
    /** Number of Samples in the samplerate-converted HRIRs */
    int lengthOfHRIR;
    /** Has to be double the size of HRIR (due to linear convolution) */
    int lengthOfFFT;
    /** Number of Frequency-Bins FFTLength/2 + 1: will be the length of the samplerate-converted HRTFs */
    int lengthOfHRTF;
    
    
    fftwf_complex* theStoredHRTFs;
    int lengthOfHRTFStorage;
    int loadSofaFile(const char* filePath, int hostSampleRate);
    
    Single_HRIR_Measurement** loadedHRIRs;

    void errorHandling(int status);
    String errorDesc;
    void createPassThrough_FIR(int _sampleRate);
    
    sofaMetadataStruct sofaMetadata;
    
    String getSOFAGlobalAttribute(const char* attribute_ID, int ncid);
    
    
};



#endif  // SOFAData_H_INCLUDED
