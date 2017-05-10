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
    addParameter(elevationParam = new AudioParameterFloat("elevation", "Elevation", 0.f, 1.f, 0.5f));
    addParameter(testSwitchParam = new AudioParameterBool("test", "Test Switch", false));
    
    HRTFs = NULL;
    Filter = NULL;
    FilterB = NULL;
    
    sampleRate_f = 0;
    
    updateSofaMetadataFlag = false;

    
    
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
    counter = 0;
    
    if(sampleRate != sampleRate_f){
        sampleRate_f = sampleRate;
        initData(pathToSOFAFile);
    }else{
        Filter->prepareToPlay();
    }

    
    
}

void SofaPanAudioProcessor::initData(String sofaFile){
    
    printf("\n initalise Data \n ");
    
    suspendProcessing(true);
    
    if(HRTFs!=NULL)
        delete HRTFs;
    HRTFs = new SOFAData(pathToSOFAFile.toUTF8(), (int)sampleRate_f);
    metadata_sofafile = HRTFs->getMetadata();
    
    updateSofaMetadataFlag = true;
    
    if(Filter != NULL)
        delete Filter;
    Filter = new FilterEngine(*HRTFs);
    if(FilterB != NULL)
        delete FilterB;
    FilterB = new FilterEngineB(*HRTFs);
    
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
    
    
    float* outBufferL = buffer.getWritePointer (0);
    float* outBufferR = buffer.getWritePointer (1);
    const float* inBuffer = buffer.getReadPointer(0);
    
    float azimuth = panParam->get() * 360.0;
    float elevation = (elevationParam->get()-0.5) * 180.0;
    
    if(!*testSwitchParam)
        Filter->process(inBuffer, outBufferL, outBufferR, numberOfSamples, azimuth, elevation);
    else
        FilterB->process(inBuffer, outBufferL, outBufferR, numberOfSamples, azimuth, elevation);


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

fftwf_complex* SofaPanAudioProcessor::getCurrentHRTF(){
    
    if(HRTFs == NULL)
        return NULL;
    float azimuth = panParam->get() * 360.0;
    float elevation = (elevationParam->get()-0.5) * 180.0;
    fftwf_complex* hrtf = HRTFs->getHRTFforAngle(elevation, azimuth, 1.0);
    return hrtf;
}

float* SofaPanAudioProcessor::getCurrentHRIR(){
    
    float azimuth = panParam->get() * 360.0;
    float elevation = (elevationParam->get()-0.5) * 180.0;
//        for(int i = 0; i< getComplexLength()-1; i++){
//            printf("Processor: \n%d [%f]", i,  hrir.IR_Left[i]);
//        }

   
    
    return HRTFs->getHRIRForAngle(elevation, azimuth, 1.0);
}

int SofaPanAudioProcessor::getSampleRate(){
    return (int)sampleRate_f;
}

int SofaPanAudioProcessor::getComplexLength(){
    return Filter->getComplexLength();
}

