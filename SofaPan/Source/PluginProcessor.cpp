/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
SofaPanAudioProcessor::SofaPanAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    
    addParameter(panParam = new AudioParameterFloat("pan", "Panorama", 0.f, 1.f, 0.f));
    addParameter(bypassParam = new AudioParameterFloat("bypass", "Bypass", 0.f, 1.f, 0.f));
    
    SOFAFile_loaded_flag = 0;
    
    HRTFs = NULL;
    
    sampleRate_f = 0;
    
    //Juce AudioBuffers are created, but the size is set in the init()-method
    collectSampleBuffer1 = AudioBuffer<float>();
    collectSampleBuffer2 = AudioBuffer<float>();
    overlapAddOutA1 = AudioBuffer<float>();
    overlapAddOutB1 = AudioBuffer<float>();
    overlapAddOutA2 = AudioBuffer<float>();
    overlapAddOutB2 = AudioBuffer<float>();
    
    //FFTW related pointers initially set to NULL, allocation happens in the init()-method
    fftInputBuffer = NULL;
    complexBuffer = NULL;
    fftOutputBufferL = NULL;
    fftOutputBufferR = NULL;
    forward = NULL;
    inverseL = NULL;
    inverseR = NULL;
    src = NULL;
    
}

SofaPanAudioProcessor::~SofaPanAudioProcessor()
{
    fftwf_destroy_plan(forward);
    fftwf_destroy_plan(inverseL);
    fftwf_destroy_plan(inverseR);
    fftwf_free(fftInputBuffer);
    fftwf_free(complexBuffer);
    fftwf_free(fftOutputBufferL);
    fftwf_free(fftOutputBufferR);
    fftwf_free(src);
}

//==============================================================================
const String SofaPanAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SofaPanAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SofaPanAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

double SofaPanAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SofaPanAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SofaPanAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SofaPanAudioProcessor::setCurrentProgram (int index)
{
}

const String SofaPanAudioProcessor::getProgramName (int index)
{
    return String();
}

void SofaPanAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void SofaPanAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    printf("\n prepare to play \n");
    counter = 0;
    
    if(sampleRate != sampleRate_f){
        sampleRate_f = sampleRate;
        initData(pathToSOFAFile);
    }
    
    fifoIndex = 0;
    fifoIndex2 = fir_length / 2;
    fifoOutToBeUsed1 = 0;
    fifoOutToBeUsed2 = 0;
    
}

void SofaPanAudioProcessor::initData(String sofaFile){
    
    printf("\n initalise Data \n ");
    
    suspendProcessing(true);
    
    if(HRTFs!=NULL){
        delete HRTFs;
    }
    
    HRTFs = new SOFAData(pathToSOFAFile.toUTF8(), (int)sampleRate_f);
    metadata_sofafile = HRTFs->getMetadata();
    
    fir_length = HRTFs->getLengthOfHRIR();
    
    
    //Initialize Variables
    fftLength = fir_length * 2;
    complexLength = fftLength / 2 + 1;
    scale = 1.0 / (float)fftLength;
    
    fifoIndex = 0;
    fifoIndex2 = fir_length / 2;
    fifoOutToBeUsed1 = 0;
    fifoOutToBeUsed2 = 0;
    
    
    //ALLOCATE MEMORY ========================================================
    
    
    
    
    collectSampleBuffer1.setSize(1, fftLength);
    collectSampleBuffer2.setSize(1, fftLength);
    collectSampleBuffer1.clear();
    collectSampleBuffer2.clear();
    
    
    overlapAddOutA1.setSize(2, fftLength); //Stereo
    overlapAddOutB1.setSize(2, fftLength);
    overlapAddOutA2.setSize(2, fftLength); //Stereo
    overlapAddOutB2.setSize(2, fftLength);
    
    overlapAddOutA2.clear();
    overlapAddOutB2.clear();
    overlapAddOutA1.clear();
    overlapAddOutB1.clear();
    
    printf("\n oAU_A1: %d", overlapAddOutA1.getNumSamples());
    printf("\n oAU_B1: %d", overlapAddOutB1.getNumSamples());
    printf("\n oAU_A2: %d", overlapAddOutA2.getNumSamples());
    printf("\n oAU_B2: %d", overlapAddOutB2.getNumSamples());
    
    /* FFTW init ############################## */
    
    //first release old resources, if allocated
    if(fftInputBuffer != NULL) fftwf_free(fftInputBuffer);
    if(complexBuffer != NULL) fftwf_free(complexBuffer);
    if(fftOutputBufferL != NULL) fftwf_free(fftOutputBufferL);
    if(fftOutputBufferR != NULL) fftwf_free(fftOutputBufferR);
    if(src != NULL) fftwf_free(src);
    if(forward != NULL) fftwf_destroy_plan(forward);
    if(inverseL != NULL) fftwf_destroy_plan(inverseL);
    if(inverseR != NULL) fftwf_destroy_plan(inverseR);
    
    fftInputBuffer   =  fftwf_alloc_real(fftLength);
    complexBuffer    =  fftwf_alloc_complex(complexLength);
    
    fftOutputBufferL = fftwf_alloc_real(fftLength);
    fftOutputBufferR = fftwf_alloc_real(fftLength);
    
    /* Erstellt einen FFT-Plan, der ein reel^wertiges array (N samples) in ein komplexes array ((N/2)+1 paare aus magnitude + phase) transformiert
     Übergabeargumente: FFT-Länge N, Quell-Data, Ziel-Data, Optionale Flags
     Einmal erstellt, kann der Plan einfach mittels fftw_execute beliebig oft ausgeführt werden */
    forward = fftwf_plan_dft_r2c_1d(fftLength, fftInputBuffer, complexBuffer, FFTW_ESTIMATE);
    
    /* Erstellt analog zu oben einen iFFT-Plan, der ein komplexes array (magnitude + phase) in ein reelwertiges array (samples) rücktransformiert */
    inverseL = fftwf_plan_dft_c2r_1d(fftLength, complexBuffer, fftOutputBufferL, FFTW_ESTIMATE);
    inverseR = fftwf_plan_dft_c2r_1d(fftLength, complexBuffer, fftOutputBufferR, FFTW_ESTIMATE);
    
    //Intermediate Buffer, used for the complex multiplication
    src = fftwf_alloc_complex(complexLength);
    
    suspendProcessing(false);
}

void SofaPanAudioProcessor::releaseResources()
{
   
    printf("Playback Stop");
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.

}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SofaPanAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SofaPanAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    
    const int numberOfSamples = buffer.getNumSamples();
    
    if (bypassParam->get() == true) { return; }
    
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    
    
    /* PROCESSING */
    
    
    
    float* outBufferL = buffer.getWritePointer (0);
    float* outBufferR = buffer.getWritePointer (1);
    
    float* collectSamples1 = collectSampleBuffer1.getWritePointer(0);
    float* collectSamples2 = collectSampleBuffer2.getWritePointer(0);
    
    const float* overlapAdd_A_L_1 = overlapAddOutA1.getReadPointer(0);
    const float* overlapAdd_A_R_1 = overlapAddOutA1.getReadPointer(1);
    const float* overlapAdd_B_L_1 = overlapAddOutB1.getReadPointer(0);
    const float* overlapAdd_B_R_1 = overlapAddOutB1.getReadPointer(1);
    
    const float* overlapAdd_A_L_2 = overlapAddOutA2.getReadPointer(0);
    const float* overlapAdd_A_R_2 = overlapAddOutA2.getReadPointer(1);
    const float* overlapAdd_B_L_2 = overlapAddOutB2.getReadPointer(0);
    const float* overlapAdd_B_R_2 = overlapAddOutB2.getReadPointer(1);
    

    const float* inBuffer = buffer.getReadPointer(0);
    
    for(int sample = 0; sample < numberOfSamples; ++sample){
        
        // Beliebige Channel-Anzahl zu Mono
//        float monoSum;
//        for (int ch = 0; ch < totalNumInputChannels; ch++) {
//            const float* inBuffer = buffer.getReadPointer(ch);
//            monoSum += (inBuffer[sample] / totalNumInputChannels);
//        }
    
        
        /* Samples sammeln und samples ausgeben */
        //Samples sammeln für FFT
        collectSamples1[fifoIndex] = inBuffer[sample];
        collectSamples2[fifoIndex2] = inBuffer[sample];
        
        //Bereits (rück-)transformierte Samples sind in den fifoOut 1&2 gespeichert und können sampleweise ausgelesen werden. Dabei wird Overlap-Add angewandt (Es wird aus beiden buffern gleichzeitig gelesen, immer mit dem Versatz der halben Länge von fifoOut, nämlich der HRIRSize
        float outSampleL_1, outSampleR_1, outSampleL_2, outSampleR_2;
        if(fifoOutToBeUsed1 == 0){
            outSampleL_1 = overlapAdd_A_L_1[fifoIndex + fir_length] + overlapAdd_B_L_1[fifoIndex];
            outSampleR_1 = overlapAdd_A_R_1[fifoIndex + fir_length] + overlapAdd_B_R_1[fifoIndex];
        }else{
            outSampleL_1 = overlapAdd_A_L_1[fifoIndex] + overlapAdd_B_L_1[fifoIndex + fir_length];
            outSampleR_1 = overlapAdd_A_R_1[fifoIndex] + overlapAdd_B_R_1[fifoIndex + fir_length];
        }
        
        if(fifoOutToBeUsed2 == 0){
            outSampleL_2 = overlapAdd_A_L_2[fifoIndex2 + fir_length] + overlapAdd_B_L_2[fifoIndex2];
            outSampleR_2 = overlapAdd_A_R_2[fifoIndex2 + fir_length] + overlapAdd_B_R_2[fifoIndex2];
        }else{
            outSampleL_2 = overlapAdd_A_L_2[fifoIndex2] + overlapAdd_B_L_2[fifoIndex2 + fir_length];
            outSampleR_2 = overlapAdd_A_R_2[fifoIndex2] + overlapAdd_B_R_2[fifoIndex2 + fir_length];
        }
        
        float x = (float)fifoIndex / (float)fir_length;
        float fadeGain = x < 0.5 ? 2.0 * x : -2.0 * x + 2.0;
        
        //Hier muss jetzt geblendet werden!!!
        
        outBufferL[sample] = outSampleL_1 * fadeGain + outSampleL_2 * (1 - fadeGain);
        outBufferR[sample] = outSampleR_1 * fadeGain + outSampleR_2 * (1 - fadeGain);
        
        fifoIndex++;
        fifoIndex2++;
        
        
        
        
        
        //Reine Vorsichtsmaßnahme (Hard-Clip)
        //outSample = (outSample >  1.0) ?  1.0 : outSample;
        //outSample = (outSample < -1.0) ? -1.0 : outSample;
        
        
        
        
        
        
        //wenn genug samples gesammelt wurden => FFT ausführen!
        if(fifoIndex == fir_length)
        {
            
            float angle = panParam->get() * 360.0;
            fftwf_complex* hrtf = HRTFs->getHRTFforAngle(0.0, angle);
            
            fifoIndex = 0;
            
            for(int i = 0; i< fir_length; i++){
                fftInputBuffer[i] = collectSamples1[i];
                fftInputBuffer[i+fir_length] = 0.0; //Zweite Hälfte nullen
            }
            
            /* Da die Pläne "forward" und "inverse" bei Plugin-Erstellung festegelegt wurden und sich nicht mehr ändern, können diese einfach mit execute ausgeführt werden.
             FFT-Länge N = 2*HRIRSize
             forward = fftInputBuffer[N] ->  FFT -> complexBuffer[N/2 + 1]
             inverse = complexBuffer[N/2 + 1]  -> iFFT -> fftOutputBuffer[N]
             */
            
            
            fftwf_execute(forward);
            
            memcpy(src, complexBuffer, sizeof(fftwf_complex) * complexLength);
            
            //Hier findet die Verarbeitung im Frequenzbereich statt!
            
            /* Linker Kanal */
            for ( int k=0; k<complexLength; k++ ) {
                // Y(k) = X(k) * H(K)
                // Yr = Xr * Hr - Xi * Hi
                // Yi = Xr * Hi + Xi * Hr
                complexBuffer[k][RE] = (src[k][RE] * hrtf[k][RE] - src[k][IM] * hrtf[k][IM]) * scale;
                complexBuffer[k][IM] = (src[k][RE] * hrtf[k][IM] + src[k][IM] * hrtf[k][RE]) * scale;
            }
            
            fftwf_execute(inverseL);
            
            
            /* Rechter Kanal */
            for ( int k=0; k<complexLength; k++ ) {
                complexBuffer[k][RE] = (src[k][RE] * hrtf[k+complexLength][RE] - src[k][IM] * hrtf[k+complexLength][IM]) * scale;
                complexBuffer[k][IM] = (src[k][RE] * hrtf[k+complexLength][IM] + src[k][IM] * hrtf[k+complexLength][RE]) * scale;
            }
            
            
            fftwf_execute(inverseR);
            
            
            
            if(fifoOutToBeUsed1 == 0){
                //memcpy(fifoOut, fftOutputBuffer, sizeof(fifoOut));
                overlapAddOutA1.copyFrom(0, 0, fftOutputBufferL, fftLength);
                overlapAddOutA1.copyFrom(1, 0, fftOutputBufferR, fftLength);
                fifoOutToBeUsed1 = 1;
            }else{
                //memcpy(fifoOut2, fftOutputBuffer, sizeof(fifoOut2));
                overlapAddOutB1.copyFrom(0, 0, fftOutputBufferL, fftLength);
                overlapAddOutB1.copyFrom(1, 0, fftOutputBufferR, fftLength);
                fifoOutToBeUsed1 = 0;
            }
            
            
        }
        
        if(fifoIndex2 == fir_length)
        {
            
            float angle = panParam->get() * 360.0;
            fftwf_complex* hrtf = HRTFs->getHRTFforAngle(0.0, angle);
            
            fifoIndex2 = 0;
            
            
            
            for(int i = 0; i< fir_length; i++){
                fftInputBuffer[i] = collectSamples2[i];
                fftInputBuffer[i+fir_length] = 0.0; //Zweite Hälfte nullen
            }
            
            fftwf_execute(forward);
            
            memcpy(src, complexBuffer, sizeof(fftwf_complex) * complexLength);
            
            for ( int k=0; k<complexLength; k++ ) {
                complexBuffer[k][RE] = (src[k][RE] * hrtf[k][RE] - src[k][IM] * hrtf[k][IM]) * scale;
                complexBuffer[k][IM] = (src[k][RE] * hrtf[k][IM] + src[k][IM] * hrtf[k][RE]) * scale;
            }
            
            fftwf_execute(inverseL);
            
            for ( int k=0; k<complexLength; k++ ) {
                complexBuffer[k][RE] = (src[k][RE] * hrtf[k+complexLength][RE] - src[k][IM] * hrtf[k+complexLength][IM]) * scale;
                complexBuffer[k][IM] = (src[k][RE] * hrtf[k+complexLength][IM] + src[k][IM] * hrtf[k+complexLength][RE]) * scale;
            }
            
            fftwf_execute(inverseR);
            
            if(fifoOutToBeUsed2 == 0){
                //memcpy(fifoOut, fftOutputBuffer, sizeof(fifoOut));
                overlapAddOutA2.copyFrom(0, 0, fftOutputBufferL, fftLength);
                overlapAddOutA2.copyFrom(1, 0, fftOutputBufferR, fftLength);
                fifoOutToBeUsed2 = 1;
            }else{
                //memcpy(fifoOut2, fftOutputBuffer, sizeof(fifoOut2));
                overlapAddOutB2.copyFrom(0, 0, fftOutputBufferL, fftLength);
                overlapAddOutB2.copyFrom(1, 0, fftOutputBufferR, fftLength);
                fifoOutToBeUsed2 = 0;
            }
            
            
        }
        
        
        
        //
        //        float outL = in * ampLeft;
        //        float outR = in * ampRight;
        //
        //        outBufferL[sample] = outL;
        //        outBufferR[sample] = outR;
    }
    
    
    
    //counter ++;
    //if(counter > 1000)
    //{
    //    //        //printf("Param%f stereo%f Left%f Right%f \n", panning, stereoPan, ampLeft, ampRight);
    ////        printf("\n \n \n Printing every 40th sample of fftInputBuffer with %d samples: \n", fftSize*2);
    ////        for(int i = 0; i < fftSize * 2; i += 40 )
    ////        {
    ////            printf("%d:%.3f ", i, fftInputBuffer[i]);
    ////        }
    ////        printf("\n \n \n Printing every sample of Complex Buffer with %d samples: \n", fftSize + 1 );
    ////        for(int i = 0; i < fftSize + 1 ; i += 40 )
    ////        {
    ////            printf("%d:%.3f + i%.3f ", i, complexBuffer[i][0], complexBuffer[i][1]);
    ////        }
    //        counter = 0;
    //    }
}

//==============================================================================
bool SofaPanAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* SofaPanAudioProcessor::createEditor()
{
    return new SofaPanAudioProcessorEditor (*this);
}

//==============================================================================
void SofaPanAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SofaPanAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SofaPanAudioProcessor();
}



void SofaPanAudioProcessor::setSOFAFilePath(String sofaString){
    pathToSOFAFile = sofaString;
}

