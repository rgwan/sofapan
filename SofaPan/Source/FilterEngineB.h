/*
  ==============================================================================

    FilterEngineB.h
    Created: 28 Apr 2017 11:09:46am
    Author:  David Bau

  ==============================================================================
*/

#ifndef FILTERENGINEB_H_INCLUDED
#define FILTERENGINEB_H_INCLUDED


#include "SOFAData.h"
#include "fftw3.h"

#define RE 0
#define IM 1


class FilterEngineB{
    
public:
    FilterEngineB(SOFAData& sD);
    ~FilterEngineB();
    
    void process(const float* inBuffer, float* outBuffer_L, float* outBuffer_R, int numSamples, float azimuth, float elevation);
    void prepareToPlay();
    
private:
    SOFAData& sofaData;
    
    const int firLength;
    int fftLength;
    int complexLength;
    float fftSampleScale;
    
    fftwf_plan forward;
    fftwf_plan inverse_L;
    fftwf_plan inverse_R;
    
    float* fftInputBuffer;
    fftwf_complex* complexBuffer;
    fftwf_complex* src;
    float* fftOutputBuffer_L;
    float* fftOutputBuffer_R;
    
    float* inputBuffer;
    float* lastInputBuffer;
    float* outputBuffer_L;
    float* outputBuffer_R;
    
    int fifoIndex;
    
    fftw_complex* previousHRTF;
    float previousAzimuth;
    float previousElevation;
    
    
    
    
};


#endif  // FILTERENGINEB_H_INCLUDED
