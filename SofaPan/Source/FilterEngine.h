/*
  ==============================================================================

    FilterEngine.h
    Created: 27 Apr 2017 11:14:15am
    Author:  David Bau

  ==============================================================================
*/

#ifndef FILTERENGINE_H_INCLUDED
#define FILTERENGINE_H_INCLUDED

#include "SOFAData.h"
#include "fftw3.h"

#define RE 0
#define IM 1


class FilterEngine{

public:
    FilterEngine(SOFAData& sD);
    ~FilterEngine();
    
    void process(const float* inBuffer, float* outBuffer_L, float* outBuffer_R, int numSamples, float azimuth, float elevation);
    void prepareToPlay();
    int getComplexLength();
    
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
    float* weightingCurve;
    
    int fifoIndex;
    
    fftwf_complex* previousHRTF;
    float previousAzimuth;
    float previousElevation;
    
    
    
    
};



#endif  // FILTERENGINE_H_INCLUDED
