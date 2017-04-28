/*
  ==============================================================================

    FilterEngine.cpp
    Created: 27 Apr 2017 11:14:15am
    Author:  David Bau

  ==============================================================================
*/

#include "FilterEngine.h"

FilterEngine::FilterEngine(SOFAData& sD)
    : sofaData(sD), firLength(sD.getLengthOfHRIR())
{
    //Initialize Variables
    fftLength = firLength * 2;
    complexLength = fftLength / 2 + 1;
    fftSampleScale = 1.0 / (float)fftLength;
    
    //Allocate Memory
    overlapAddOutBufferA_L = fftwf_alloc_real(fftLength);
    overlapAddOutBufferA_R = fftwf_alloc_real(fftLength);
    overlapAddOutBufferB_L = fftwf_alloc_real(fftLength);
    overlapAddOutBufferB_R = fftwf_alloc_real(fftLength);
    fftInputBuffer = fftwf_alloc_real(fftLength);
    complexBuffer = fftwf_alloc_complex(complexLength);
    src = fftwf_alloc_complex(complexLength);
    fftOutputBuffer_L = fftwf_alloc_real(fftLength);
    fftOutputBuffer_R = fftwf_alloc_real(fftLength);
    
    //Init FFTW Plans
    forward = fftwf_plan_dft_r2c_1d(fftLength, fftInputBuffer, complexBuffer, FFTW_ESTIMATE);
    inverse_L = fftwf_plan_dft_c2r_1d(fftLength, complexBuffer, fftOutputBuffer_L, FFTW_ESTIMATE);
    inverse_R = fftwf_plan_dft_c2r_1d(fftLength, complexBuffer, fftOutputBuffer_R, FFTW_ESTIMATE);
    
    prepareToPlay();
}

FilterEngine::~FilterEngine(){
    fftwf_free(overlapAddOutBufferA_L);
    fftwf_free(overlapAddOutBufferA_R);
    fftwf_free(overlapAddOutBufferB_L);
    fftwf_free(overlapAddOutBufferB_R);
    fftwf_free(fftInputBuffer);
    fftwf_free(complexBuffer);
    fftwf_free(src);
    fftwf_free(fftOutputBuffer_L);
    fftwf_free(fftOutputBuffer_R);
    fftwf_destroy_plan(forward);
    fftwf_destroy_plan(inverse_L);
    fftwf_destroy_plan(inverse_R);

}

void FilterEngine::prepareToPlay(){
    
    fifoIndex = 0;
    fifoOutToBeUsed = 0;
    
    for(int i = 0; i < fftLength; i++){
        overlapAddOutBufferA_L[i] = 0.0;
        overlapAddOutBufferA_R[i] = 0.0;
        overlapAddOutBufferB_L[i] = 0.0;
        overlapAddOutBufferB_R[i] = 0.0;
    }
    
}

void FilterEngine::process(const float* inBuffer, float* outBuffer_L, float* outBuffer_R, int numSamples, float azimuth, float elevation){
    
    
    for(int sample = 0; sample < numSamples; sample++){
        
        fftInputBuffer[fifoIndex] = inBuffer[sample];

        if(fifoOutToBeUsed == 0){
            outBuffer_L[sample] = overlapAddOutBufferA_L[fifoIndex + firLength] + overlapAddOutBufferB_L[fifoIndex];
            outBuffer_R[sample] = overlapAddOutBufferA_R[fifoIndex + firLength] + overlapAddOutBufferB_R[fifoIndex];
        }else{
            outBuffer_L[sample] = overlapAddOutBufferA_L[fifoIndex] + overlapAddOutBufferB_L[fifoIndex + firLength];
            outBuffer_R[sample] = overlapAddOutBufferA_R[fifoIndex] + overlapAddOutBufferB_R[fifoIndex + firLength];
        }

        fifoIndex++;
        
        if(fifoIndex == firLength){
            fifoIndex = 0;
            
//            int azimuthIndex = (sample - sample % 4);
//            int elevationIndex = (sample - sample % 4) + 1;
//            float azimuth = modBuffer[azimuthIndex];
//            float elevation = modBuffer[elevationIndex];
            
            fftwf_complex* hrtf = sofaData.getHRTFforAngle(elevation, azimuth);

            for(int i = 0; i< firLength; i++){
                fftInputBuffer[i+firLength] = 0.0; //Zweite Hälfte nullen
            }
            
            fftwf_execute(forward);
            
            memcpy(src, complexBuffer, sizeof(fftwf_complex) * complexLength);
            
            //Hier findet die Verarbeitung im Frequenzbereich statt!
            
            /* Linker Kanal */
            for ( int k=0; k<complexLength; k++ ) {
                // Y(k) = X(k) * H(K)
                // Yr = Xr * Hr - Xi * Hi
                // Yi = Xr * Hi + Xi * Hr
                complexBuffer[k][RE] = (src[k][RE] * hrtf[k][RE] - src[k][IM] * hrtf[k][IM]) * fftSampleScale;
                complexBuffer[k][IM] = (src[k][RE] * hrtf[k][IM] + src[k][IM] * hrtf[k][RE]) * fftSampleScale;
            }
            
            fftwf_execute(inverse_L);
            
            /* Rechter Kanal */
            for ( int k=0; k<complexLength; k++ ) {
                complexBuffer[k][RE] = (src[k][RE] * hrtf[k+complexLength][RE] - src[k][IM] * hrtf[k+complexLength][IM]) * fftSampleScale;
                complexBuffer[k][IM] = (src[k][RE] * hrtf[k+complexLength][IM] + src[k][IM] * hrtf[k+complexLength][RE]) * fftSampleScale;
            }
            
            fftwf_execute(inverse_R);
            
            int bufBytes = sizeof(float) * fftLength;
            if(fifoOutToBeUsed == 0){
                memcpy(overlapAddOutBufferA_L, fftOutputBuffer_L, bufBytes);
                memcpy(overlapAddOutBufferA_R, fftOutputBuffer_R, bufBytes);
                fifoOutToBeUsed = 1;
            }else{
                memcpy(overlapAddOutBufferB_L, fftOutputBuffer_L, bufBytes);
                memcpy(overlapAddOutBufferB_R, fftOutputBuffer_R, bufBytes);
                fifoOutToBeUsed = 0;
            }
        }
    }
    
    
}