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


/* The Modulation Buffer has the same amount of samples as the audio buffer. It contains the modulation curve of up to 4 parameters, stored in serial. So every parameter has N=bufSize/4 values per buffer and the Updaterate of the Modulation is Samplerate/4.
    ModBuffer = [ P1_1 | P2_1 | P3_1 | P4_1 | P1_2 | P2_2 | .... | P4_N-1 | P1_N | P2_N | P3_N | P4_N]
 */
class FilterEngine{

public:
    FilterEngine(SOFAData& sD);
    ~FilterEngine();
    
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
    
    float* overlapAddOutBufferA_L;
    float* overlapAddOutBufferA_R;
    float* overlapAddOutBufferB_L;
    float* overlapAddOutBufferB_R;
    
    int fifoIndex;
    int fifoOutToBeUsed;
    
    
    
    
};



#endif  // FILTERENGINE_H_INCLUDED
