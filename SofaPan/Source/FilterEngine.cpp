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
    inputBuffer = fftwf_alloc_real(firLength);
    lastInputBuffer = fftwf_alloc_real(firLength);
    outputBuffer_L = fftwf_alloc_real(firLength);
    outputBuffer_R = fftwf_alloc_real(firLength);

    fftInputBuffer = fftwf_alloc_real(fftLength);
    complexBuffer = fftwf_alloc_complex(complexLength);
    src = fftwf_alloc_complex(complexLength);
    fftOutputBuffer_L = fftwf_alloc_real(fftLength);
    fftOutputBuffer_R = fftwf_alloc_real(fftLength);
    
    //Init FFTW Plans
    forward = fftwf_plan_dft_r2c_1d(fftLength, fftInputBuffer, complexBuffer, FFTW_ESTIMATE);
    inverse_L = fftwf_plan_dft_c2r_1d(fftLength, complexBuffer, fftOutputBuffer_L, FFTW_ESTIMATE);
    inverse_R = fftwf_plan_dft_c2r_1d(fftLength, complexBuffer, fftOutputBuffer_R, FFTW_ESTIMATE);
    
    weightingCurve = (float*)malloc(firLength*sizeof(float));
    for(int i = 0; i < firLength; i++){
        float theta = M_PI * 0.5 * (float)i / (float)firLength;
        weightingCurve[i] = cosf(theta)*cosf(theta);
    }
    
    prepareToPlay();
}

FilterEngine::~FilterEngine(){
    fftwf_free(inputBuffer);
    fftwf_free(lastInputBuffer);
    fftwf_free(outputBuffer_L);
    fftwf_free(outputBuffer_R);
    fftwf_free(fftInputBuffer);
    fftwf_free(complexBuffer);
    fftwf_free(src);
    fftwf_free(fftOutputBuffer_L);
    fftwf_free(fftOutputBuffer_R);
    fftwf_destroy_plan(forward);
    fftwf_destroy_plan(inverse_L);
    fftwf_destroy_plan(inverse_R);
    free(weightingCurve);

}

void FilterEngine::prepareToPlay(){
    
    fifoIndex = 0;
    
    for(int i = 0; i < firLength; i++){
        lastInputBuffer[i] = 0.0;
        outputBuffer_L[i] = 0.0;
        outputBuffer_R[i] = 0.0;
    }
    
    previousHRTF = sofaData.getHRTFforAngle(0.0, 0.0, 1.0);
    
    previousAzimuth = 0.0;
    previousElevation = 0.0;
    
}

void FilterEngine::process(const float* inBuffer, float* outBuffer_L, float* outBuffer_R, int numSamples, AudioParameterFloat* azimuthParam, AudioParameterFloat* elevationParam, AudioParameterFloat* distanceParam){
    
    
    for(int sample = 0; sample < numSamples; sample++){
        
        inputBuffer[fifoIndex] = inBuffer[sample];

        outBuffer_L[sample] = outputBuffer_L[fifoIndex];
        outBuffer_R[sample] = outputBuffer_R[fifoIndex];


        fifoIndex++;
        
        if(fifoIndex == firLength){
            fifoIndex = 0;
            
            for(int i = 0; i < firLength; i++){
                fftInputBuffer[i] = lastInputBuffer[i];
                fftInputBuffer[i+firLength] = inputBuffer[i];
                
                lastInputBuffer[i] = inputBuffer[i];
            }
            
//            int azimuthIndex = (sample - sample % 4);
//            int elevationIndex = (sample - sample % 4) + 1;
//            float azimuth = modBuffer[azimuthIndex];
//            float elevation = modBuffer[elevationIndex];
            float azimuth = azimuthParam->get() * 360.0;
            float elevation = (elevationParam->get()-0.5) * 180.0;
            float distance = distanceParam->get();
            
            fftwf_complex* hrtf = sofaData.getHRTFforAngle(elevation, azimuth, distance);

//            for(int i = 0; i< firLength; i++){
//                fftInputBuffer[i+firLength] = 0.0; //Zweite Hälfte nullen
//            }
            
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
            
            for(int i = 0; i < firLength; i++){
                if(weightingCurve[i] > 1.0) printf("  %d Exceeds 1: %.3f", i, weightingCurve[i]);
                outputBuffer_L[i] = fftOutputBuffer_L[i + firLength] * (1.0 - weightingCurve[i]);
                outputBuffer_R[i] = fftOutputBuffer_R[i + firLength] * (1.0 - weightingCurve[i]);
            }

            
            
            //if((previousAzimuth != azimuth || previousElevation != elevation) && previousHRTF!=NULL ){
            
            //fftwf_complex* previousHRTF = sofaData.getHRTFforAngle(previousElevation , previousAzimuth);
            
                for ( int k=0; k<complexLength; k++ ) {
                    complexBuffer[k][RE] = (src[k][RE] * previousHRTF[k][RE] - src[k][IM] * previousHRTF[k][IM]) * fftSampleScale;
                    complexBuffer[k][IM] = (src[k][RE] * previousHRTF[k][IM] + src[k][IM] * previousHRTF[k][RE]) * fftSampleScale;
                }
                fftwf_execute(inverse_L);
                
                for ( int k=0; k<complexLength; k++ ) {
                    complexBuffer[k][RE] = (src[k][RE] * previousHRTF[k+complexLength][RE] - src[k][IM] * previousHRTF[k+complexLength][IM]) * fftSampleScale;
                    complexBuffer[k][IM] = (src[k][RE] * previousHRTF[k+complexLength][IM] + src[k][IM] * previousHRTF[k+complexLength][RE]) * fftSampleScale;
                }

            
            
            //}
            
            for(int i = 0; i < firLength; i++){
                outputBuffer_L[i] += fftOutputBuffer_L[i + firLength] * weightingCurve[i];
                outputBuffer_R[i] += fftOutputBuffer_R[i + firLength] * weightingCurve[i];
            }
            
            previousHRTF = hrtf;
            previousAzimuth = azimuth;
            previousElevation = elevation;
            
            
            
        }
    }
    
    
}

int FilterEngine::getComplexLength(){
    return complexLength;
}
