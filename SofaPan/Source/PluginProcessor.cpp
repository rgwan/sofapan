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
    myFile = NULL;
    
}

SofaPanAudioProcessor::~SofaPanAudioProcessor()
{
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
    //INITIALISIERUNGEN =============================================================
    
    counter = 0;
    
    //Load SOFA File
    int loadSOFA_status;
    if (!SOFAFile_loaded_flag) {
        if (loadSOFA_status = LoadSOFAFile())ErrorHandling_LoadSOFAFile(loadSOFA_status);
    };
    
    
    //Get Information for SampleRate Conversion
    int oldFirLength = metadata_struct.HRIR_numSamples;
    int hrirSampleRate = metadata_struct.HRIR_Samplingrate;
    float frequenzFaktor = (float)hrirSampleRate / sampleRate;
    
    //Calculate Length of interpolated IR´s by finding the next power of two
    float newFirLength_f = (float)(oldFirLength) / frequenzFaktor; //z.B. 128 bei 44.1kHz ---> 128/(44.1/96) = 278.6 bei 96kHz
    fir_length = 2;
    while (fir_length < newFirLength_f) {
        fir_length *= 2;
    }
    
    //Initialize Variables
    fftLength = fir_length * 2.0; //z.B. 512
    complexLength = fftLength / 2 + 1; //z.B. 257
    scale = 1.0 / (float)fftLength;
    fifoIndex = 0;
    fifoIndex2 = fir_length / 2;
    fifoOutToBeUsed1 = 0;
    fifoOutToBeUsed2 = 0;
    
    int numHRIRs = metadata_struct.HRIR_numMeasurements;
    const int lengthOfHRTFArray = complexLength * numHRIRs;
    
    
    
    //ALLOCATE MEMORY ========================================================
    
    
    //Die Input Buffer zum Ansammeln der Eingangssamples
    collectSampleBuffer1 = AudioBuffer<float>(1, fftLength); //Mono
    collectSampleBuffer2 = AudioBuffer<float>(1, fftLength);
    collectSampleBuffer1.clear();
    collectSampleBuffer2.clear();
    
    // Die Output buffer der iFFT erstellen
    overlapAddOutA1 = AudioBuffer<float>(2, fftLength); //Stereo
    overlapAddOutB1 = AudioBuffer<float>(2, fftLength);
    overlapAddOutA1.clear();
    overlapAddOutB1.clear();
    
    overlapAddOutA2 = AudioBuffer<float>(2, fftLength); //Stereo
    overlapAddOutB2 = AudioBuffer<float>(2, fftLength);
    overlapAddOutA2.clear();
    overlapAddOutB2.clear();
    
    
    /* FFTW initialisieren ############################## */
    
    fftInputBuffer   =  fftwf_alloc_real(fftLength);
    complexBuffer    =  fftwf_alloc_complex(complexLength);
    
    fftOutputBufferL = fftwf_alloc_real(fftLength);
    fftOutputBufferR = fftwf_alloc_real(fftLength);
    
    /* Erstellt einen FFT-Plan, der ein reelwertiges array (N samples) in ein komplexes array ((N/2)+1 paare aus magnitude + phase) transformiert
     Übergabeargumente: FFT-Länge N, Quell-Data, Ziel-Data, Optionale Flags
     Einmal erstellt, kann der Plan einfach mittels fftw_execute beliebig oft ausgeführt werden */
    forward = fftwf_plan_dft_r2c_1d(fftLength, fftInputBuffer, complexBuffer, FFTW_ESTIMATE);
    
    /* Erstellt analog zu oben einen iFFT-Plan, der ein komplexes array (magnitude + phase) in ein reelwertiges array (samples) rücktransformiert */
    inverseL = fftwf_plan_dft_c2r_1d(fftLength, complexBuffer, fftOutputBufferL, FFTW_ESTIMATE);
    inverseR = fftwf_plan_dft_c2r_1d(fftLength, complexBuffer, fftOutputBufferR, FFTW_ESTIMATE);
    
    //Large Array for storing all HRTFs in serial
    firCoeffsL = fftwf_alloc_complex(lengthOfHRTFArray);
    firCoeffsR = fftwf_alloc_complex(lengthOfHRTFArray);
    
    //Intermediate Buffer, used for the complex multiplication
    src = fftwf_alloc_complex(complexLength);
    
    
    
    
    
    
    //HRIRS transformieren und abspeichern ====================================================
    
    float* IRInterpolated_l = (float *)malloc(fir_length * sizeof(float));
    float* IRInterpolated_r = (float *)malloc(fir_length * sizeof(float));
    
    
    //Schleife für jede HRIR
    for (int i = 0; i < numHRIRs; i++){
        
        //Get the current HRTF
        int HRTFIndex = complexLength * i;
        HRIR_Measurement *object = myFile[i];
        float *ir_left = object->getIR_Left();
        float *ir_right = object->getIR_Right();
        
        //Sample Rate Conversion (L+R)
        float f_index = 0.0f;
        int i_index = 0;
        float f_bruch = 0;
        int i_fganzezahl = 0;
        while ((i_fganzezahl+1) < oldFirLength){
            //Linear interpolieren
            IRInterpolated_l[i_index] = ir_left[i_fganzezahl] * (1.0f - f_bruch) + ir_left[i_fganzezahl + 1] * f_bruch;
            IRInterpolated_l[i_index] *= frequenzFaktor;
            IRInterpolated_r[i_index] = ir_right[i_fganzezahl] * (1.0f - f_bruch) + ir_right[i_fganzezahl + 1] * f_bruch;
            IRInterpolated_r[i_index] *= frequenzFaktor;
            //Berechnungen fuer naechste Runde.
            i_index++;
            f_index = i_index * frequenzFaktor;
            i_fganzezahl = (int)f_index;
            f_bruch = f_index - i_fganzezahl;
        }
        while(i_index < fir_length){
            IRInterpolated_l[i_index] = 0.0;
            IRInterpolated_r[i_index] = 0.0;
            i_index++;
        }
        
        //L: Prepare the IR for FFT (->Zeropadding)
        for(int j = 0; j < fir_length; j++){
            fftInputBuffer[j] = IRInterpolated_l[j];
            fftInputBuffer[j+fir_length] = 0.0;
        }
        //L: Do FFT
        fftwf_execute(forward);
        //L: Store result in big array
        for (int k = 0; k < complexLength; k++){
            firCoeffsL[HRTFIndex + k][0] = complexBuffer[k][0]; //RE
            firCoeffsL[HRTFIndex + k][1] = complexBuffer[k][1]; //IM
        }
        
        //R: Prepare the IR for FFT (->Zeropadding)
        for(int j = 0; j < fir_length; j++){
            fftInputBuffer[j] = IRInterpolated_r[j];
            fftInputBuffer[j+fir_length] = 0.0;
        }
        //R: Do FFT
        fftwf_execute(forward);
        //R: Store result in big array
        for (int k = 0; k < complexLength; k++){
            firCoeffsR[HRTFIndex + k][0] = complexBuffer[k][0];
            firCoeffsR[HRTFIndex + k][1] = complexBuffer[k][1];
        }
        
    }
}

void SofaPanAudioProcessor::releaseResources()
{
    printf("Playback Stop");
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    fftwf_destroy_plan(forward);
    fftwf_destroy_plan(inverseL);
    fftwf_destroy_plan(inverseR);
    fftwf_free(fftInputBuffer);
    fftwf_free(complexBuffer);
    fftwf_free(fftOutputBufferL);
    fftwf_free(fftOutputBufferR);
    fftwf_free(src);
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
    
    //Der Buffer für die Monosumme
    monoSumBuffer = AudioBuffer<float>(1, numberOfSamples);
    monoSumBuffer.clear();
    float* monoSum = monoSumBuffer.getWritePointer(0);
    
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
    
    // Beliebige Channel-Anzahl zu Mono
    for (int ch = 0; ch < totalNumInputChannels; ch++) {
        const float* inBuffer = buffer.getReadPointer(ch);
        for (int i = 0; i < numberOfSamples; i++) {
            monoSum[i] += (inBuffer[i] / totalNumInputChannels);
        }
    }
    
    for(int sample = 0; sample < numberOfSamples; ++sample){
        /* Samples sammeln und samples ausgeben */
        //Samples sammeln für FFT
        collectSamples1[fifoIndex] = monoSum[sample];
        collectSamples2[fifoIndex2] = monoSum[sample];
        
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
            
            int selectHRTF = (int)(((float)metadata_struct.HRIR_numMeasurements - 1.0) * panParam->get());
            //printf("selectHRTF; %d", selectHRTF);
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
                complexBuffer[k][RE] = (src[k][RE] * firCoeffsL[selectHRTF*complexLength + k][RE] - src[k][IM] * firCoeffsL[selectHRTF*complexLength + k][IM]) * scale;
                complexBuffer[k][IM] = (src[k][RE] * firCoeffsL[selectHRTF*complexLength + k][IM] + src[k][IM] * firCoeffsL[selectHRTF*complexLength + k][RE]) * scale;
            }
            
            fftwf_execute(inverseL);
            
            
            /* Rechter Kanal */
            for ( int k=0; k<complexLength; k++ ) {
                complexBuffer[k][RE] = (src[k][RE] * firCoeffsR[selectHRTF*complexLength + k][RE] - src[k][IM] * firCoeffsR[selectHRTF*complexLength + k][IM]) * scale;
                complexBuffer[k][IM] = (src[k][RE] * firCoeffsR[selectHRTF*complexLength + k][IM] + src[k][IM] * firCoeffsR[selectHRTF*complexLength + k][RE]) * scale;
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
            
            int selectHRTF = (int)(((float)metadata_struct.HRIR_numMeasurements - 1.0) * panParam->get());
            fifoIndex2 = 0;
            
            for(int i = 0; i< fir_length; i++){
                fftInputBuffer[i] = collectSamples2[i];
                fftInputBuffer[i+fir_length] = 0.0; //Zweite Hälfte nullen
            }
            
            fftwf_execute(forward);
            
            memcpy(src, complexBuffer, sizeof(fftwf_complex) * complexLength);
            
            for ( int k=0; k<complexLength; k++ ) {
                complexBuffer[k][RE] = (src[k][RE] * firCoeffsL[selectHRTF*complexLength + k][RE] - src[k][IM] * firCoeffsL[selectHRTF*complexLength + k][IM]) * scale;
                complexBuffer[k][IM] = (src[k][RE] * firCoeffsL[selectHRTF*complexLength + k][IM] + src[k][IM] * firCoeffsL[selectHRTF*complexLength + k][RE]) * scale;
            }
            
            fftwf_execute(inverseL);
            
            for ( int k=0; k<complexLength; k++ ) {
                complexBuffer[k][RE] = (src[k][RE] * firCoeffsR[selectHRTF*complexLength + k][RE] - src[k][IM] * firCoeffsR[selectHRTF*complexLength + k][IM]) * scale;
                complexBuffer[k][IM] = (src[k][RE] * firCoeffsR[selectHRTF*complexLength + k][IM] + src[k][IM] * firCoeffsR[selectHRTF*complexLength + k][RE]) * scale;
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

int SofaPanAudioProcessor::LoadSOFAFile()
{
    if(myFile != NULL) {
        free(myFile);
    }
    
    //open File in read mode (NC_NOWRITE) and get netcdf-ID of Sofa-File
    int status, ncid;
    if ((status = nc_open(pathToSOFAFile.toUTF8(), NC_NOWRITE, &ncid)))
        return 1;
    //AlertWindow::showMessageBox(AlertWindow::InfoIcon, "Inside Sofa", pathToSOFAFile);
    
    //Get SOFA Data Type (should be FIR)
    size_t DataType_len;
    if ((status = nc_inq_attlen(ncid, NC_GLOBAL, "DataType", &DataType_len)))
        return 1;
    
    printf("dataType_len = %zu", DataType_len);
    char DataType[3];
    if ((status = nc_get_att(ncid, NC_GLOBAL, "DataType", &DataType)))
        return 1;
    
    //Get Sampling Rate
    int SamplingRate_id, SamplingRate;
    if ((status = nc_inq_varid(ncid, "Data.SamplingRate", &SamplingRate_id)))
        return 1;
    if ((status = nc_get_var_int(ncid, SamplingRate_id, &SamplingRate)))
        return 1;
    metadata_struct.HRIR_Samplingrate = SamplingRate; //write Sampling Rate to HRTF struct
    
    //Get Sampling Rate Units
    size_t SamplingRateUnits_len;
    if ((status = nc_inq_attlen(ncid, SamplingRate_id, "Units", &SamplingRateUnits_len)))
        return 1;
    char SamplingRateUnits[5];
    if ((status = nc_get_att(ncid, SamplingRate_id, "Units", &SamplingRateUnits)))
        return 1;
    
    //Get netcdf-ID of IR Data
    int DataIR_id;
    if ((status = nc_inq_varid(ncid, "Data.IR", &DataIR_id)))//Get Impulse Resopnse Data ID
        return 1;
    
    //Get Dimensions of Data IR
    nc_type xtypep;
    int DataIR_ndimsp;
    int DataIR_dimidsp[MAX_VAR_DIMS];
    int DataIR_attidsp;
    size_t dimM_len;//Number of Measurements
    size_t dimR_len;//Number of Receivers, in case of two ears dimR_len=2
    size_t dimN_len;//Number of DataSamples describing each Measurement
    
    
    if ((status = nc_inq_var(ncid, DataIR_id, 0, &xtypep, &DataIR_ndimsp, DataIR_dimidsp, &DataIR_attidsp)))
        return 1;
    if ((status = nc_inq_dimlen(ncid, DataIR_dimidsp[0], &dimM_len)))
        return 1;
    if ((status = nc_inq_dimlen(ncid, DataIR_dimidsp[1], &dimR_len)))
        return 1;
    if ((status = nc_inq_dimlen(ncid, DataIR_dimidsp[2], &dimN_len)))
        return 1;
    metadata_struct.HRIR_numSamples = dimN_len; //Store Value in Struct metadata_struct
    
    int dimM_p = DataIR_dimidsp[0];
    int dimR_p = DataIR_dimidsp[1];
    int dimN_p = DataIR_dimidsp[2];
    std::cout << "n Dimensions of DataIR: " << DataIR_ndimsp << std::endl;
    std::cout << "dimM: " << dimM_len << std::endl; //Number of Measurements
    std::cout << "dimR: " << dimR_len << std::endl; //Number of Receivers, in case of two ears dimR_len=2
    std::cout << "dimN: " << dimN_len << std::endl; //Number of DataSamples describing each Measurement
    
    if (dimR_len != 2) return 1; //File ist nicht Stereo
    
    //Get Source Positions (Azimuth, Elevation, Distance)
    int SourcePosition_id;
    if ((status = nc_inq_varid(ncid, "SourcePosition", &SourcePosition_id)))
        return 1;
    float *SourcePosition;
    SourcePosition = (float*)malloc(sizeof(float) * 3 * dimM_len); //Allocate Memory for Sourcepositions of each Measurement
    if ((status = nc_get_var_float(ncid, SourcePosition_id, SourcePosition)))// Store Sourceposition Data to Array
    {
        free(SourcePosition);
        return 1;
    };
    
    //Get Impluse Responses
    float *DataIR;
    DataIR = (float*)malloc(dimM_len * 2 * dimN_len * sizeof(float)); //Allocate Memory for Impulse Responses, for performance reasons data must be stored in arry instead of matrix
    if ((status = nc_get_var_float(ncid, DataIR_id, DataIR))) //Read and write Data IR to variable Data IR
    {
        free(SourcePosition);
        free(DataIR);
        return 1;
    };
    
    int numZeroElevation = 0;
    int u = 0;
    for (int i = 0; i < dimM_len; i++) {
        if (SourcePosition[u + 1] == 0) {
            numZeroElevation++;
        }
        u += 3;
    }
    metadata_struct.HRIR_numMeasurements = numZeroElevation;
    
    myFile = (HRIR_Measurement**)malloc(numZeroElevation * sizeof(HRIR_Measurement));
    
    //Store each measurement with Elevation = 0 in struct myFile
    int i = 0, j = 0, l = 0, x = 0;
    for (i = 0; i < dimM_len; i++) {
        if (SourcePosition[l + 1] == 0) {
            HRIR_Measurement *measurement_object = new HRIR_Measurement(dimN_len);
            float *IR_Left = (float *)malloc(dimN_len * sizeof(float));
            float *IR_Right = (float *)malloc(dimN_len * sizeof(float));
            
            for (j = 0; j < dimN_len; j++) {
                IR_Left[j] = DataIR[i*(dimR_len*dimN_len) + 1 * dimN_len + j];//matrix[i][u][v]= array[ i*(R*N) + u*N + v] : Matrix to Arry Convertion of C++
                IR_Right[j] = DataIR[i*(dimR_len*dimN_len) + 2 * dimN_len + j];
            };
            float Azimuth = SourcePosition[l];
            float Elevation = SourcePosition[l + 1];
            float Distance = SourcePosition[l + 2];
            measurement_object->setValues(IR_Left, IR_Right, Elevation, Azimuth, Distance);
            myFile[x] = measurement_object;
            
            x++;
        }
        l += 3;
    };
    
    
    free(SourcePosition);
    free(DataIR);
    if (status = nc_close(ncid))
    {
        free(myFile);
        return 1;
    };
    SOFAFile_loaded_flag = 1;
    return 0;
}

int SofaPanAudioProcessor::ErrorHandling_LoadSOFAFile(int status) {
    switch (status) {
        case 1:
            pathToSOFAFile = String(SOFA_DEFAULT_PATH);
            if (LoadSOFAFile()) {
                createPassThrough_FIR();
                AlertWindow::showNativeDialogBox("Binaural Renderer", "Could not find Default HRTF. No HRTF loaded.", false);
            }
            else {
                AlertWindow::showNativeDialogBox("Binaural Renderer", "SOFA File could not be loaded. Default HRTF loaded.", false);
            }
    }
    
    //juce::String errorstring = static_cast <String> (loadSofa_errorstatus);
    //AlertWindow::showMessageBox(AlertWindow::NoIcon, "SOFA Errorhandling", errorstring);
    return 0;
    
}
void SofaPanAudioProcessor::setSOFAFilePath(String sofaString){
    pathToSOFAFile = sofaString;
}

HRIR_Measurement::HRIR_Measurement(int length) {
    IR_Left = (float*)malloc(length * sizeof(float));
    IR_Right = (float*)malloc(length * sizeof(float));
}

HRIR_Measurement::~HRIR_Measurement() {
    free(IR_Left);
    free(IR_Right);
}

void HRIR_Measurement::setValues(float *left, float *right, float elevation, float azimuth, float distance) {
    IR_Left = left;
    IR_Right = right;
    Elevation = elevation;
    Azimuth = azimuth;
    Distance = distance;
}

float *HRIR_Measurement::getIR_Left(){
    return IR_Left;
}

float *HRIR_Measurement::getIR_Right() {
    return IR_Right;
}

void SofaPanAudioProcessor::createPassThrough_FIR(){
    metadata_struct.HRIR_Samplingrate = 44100;
    metadata_struct.HRIR_numSamples = 256;
    metadata_struct.HRIR_numMeasurements = 1;
    myFile = (HRIR_Measurement**)malloc(1 * sizeof(HRIR_Measurement));
    HRIR_Measurement *measurement_object = new HRIR_Measurement(256);
    
    
    float *IR_Left = (float *)malloc(256 * sizeof(float));
    float *IR_Right = (float *)malloc(256 * sizeof(float));
    
    IR_Left[0]  = 1.0;
    IR_Right[0] = 1.0;
    for (int j = 1; j < 256; j++) {
        IR_Left[j]  = 0.0;
        IR_Right[j] = 0.0;
    };
    float Azimuth = 0.0;
    float Elevation = 0.0;
    float Distance = 0.0;
    measurement_object->setValues(IR_Left, IR_Right, Elevation, Azimuth, Distance);
    myFile[0] = measurement_object;
    SOFAFile_loaded_flag = 1;
}


