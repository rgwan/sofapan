/*
  ==============================================================================

    SOFAData.cpp
    Created: 4 Apr 2017 11:47:12am
    Author:  David Bau

  ==============================================================================
*/

#include "SOFAData.h"


int getIRLengthForNewSampleRate(int IR_Length, int original_SR, int new_SR);
//void error(char* errorMessage);
int sampleRateConversion(float* inBuffer, float* outBuffer, int n_inBuffer, int n_outBuffer, int originalSampleRate, int newSampleRate);

SOFAData::SOFAData(const char* filePath, int sampleRate){
    
    loadedHRIRs = NULL;
    
    // LOAD SOFA FILE
    int status = loadSofaFile(filePath, sampleRate);
    if(status)
        printf("\n SofaFileGoneWrong: %d \n", status);
    
    //Allocate and init FFTW
    float* fftInputBuffer = fftwf_alloc_real(lengthOfFFT);
    fftwf_complex* fftOutputBuffer = fftwf_alloc_complex(lengthOfHRTF);
    fftwf_plan FFT = fftwf_plan_dft_r2c_1d(lengthOfFFT, fftInputBuffer, fftOutputBuffer, FFTW_ESTIMATE);
    
    for(int i = 0; i < sofaMetadata.numMeasurements; i++){
        // LEFT
        
        //Write IR into inputBuffer and do zeropadding
        for(int k = 0; k < lengthOfHRIR; k++){
            fftInputBuffer[k] = loadedHRIRs[i]->getIR_Left()[k];
            fftInputBuffer[k+lengthOfHRIR] = 0.0;
        }
        
        //Do FFT HRIR->HRTF
        fftwf_execute(FFT);
        
        //Save TF
        for(int j = 0; j < lengthOfHRTF; j++){
            loadedHRIRs[i]->getHRTF()[j][0] = fftOutputBuffer[j][0]; //RE
            loadedHRIRs[i]->getHRTF()[j][1] = fftOutputBuffer[j][1]; //IM
        }
        
        // RIGHT
        
        for(int k = 0; k < lengthOfHRIR; k++){
            fftInputBuffer[k] = loadedHRIRs[i]->getIR_Right()[k];
            fftInputBuffer[k+lengthOfHRIR] = 0.0;
        }
        
        fftwf_execute(FFT);
        
        for(int j = 0; j < lengthOfHRTF; j++){
            loadedHRIRs[i]->getHRTF()[j+lengthOfHRTF][0] = fftOutputBuffer[j][0]; //RE
            loadedHRIRs[i]->getHRTF()[j+lengthOfHRTF][1] = fftOutputBuffer[j][1]; //IM
        }
        
    }
    
    
    
    fftwf_free(fftInputBuffer);
    fftwf_free(fftOutputBuffer);
    fftwf_destroy_plan(FFT);
    
//    printf("\n SOFAData: Sofa File Successfully loaded!! \n");
//    printf("\n SOFAData: SourcePositions: %d", sofaMetadata.numMeasurements);
//    printf("\n SOFAData: HRIRLength: %d", lengthOfHRIR);
//    printf("\n SOFAData: HRTFLength: %d", lengthOfHRTF);
    

    
}

SOFAData::~SOFAData(){
    
    printf("\n Destructing SOFAData");
    
    if(loadedHRIRs != NULL){
        for(int i = 0; i < sofaMetadata.numMeasurements; i++){
            delete loadedHRIRs[i];
        }
        free(loadedHRIRs);
    }
}

int SOFAData::getLengthOfHRIR(){
    return lengthOfHRIR;
}


fftwf_complex* SOFAData::getHRTFforAngle(float elevation, float azimuth){
 
    printf("getHRTF");
    int best_id = 0;
    
    float delta;
    float min_delta = 1000;
    for(int i = 0; i < sofaMetadata.numMeasurements; i++){
        delta = fabs(loadedHRIRs[i]->Elevation - elevation)
        + fabs(loadedHRIRs[i]->Azimuth - azimuth);
        if(delta < min_delta){
            min_delta = delta ;
            best_id = i;
        }
        //printf("\n %d Az: %.1f, El: %.1f, mD: %.1f, d: %.1f, b_id: %d", i, loadedHRIRs[i]->Azimuth, loadedHRIRs[i]->Elevation, min_delta, delta, best_id);
    }
    //printf("\n returning index: %d for angle: %f", best_id, azimuth);
   
    //printf("\n best id = %d", best_id);
//        for(int k = 0; k < lengthOfHRTF; k++){
//            printf("\n %d: %f ", k, loadedHRIRs[best_id]->getHRTF()[k][0]);
//        }
    
    
    
    return loadedHRIRs[best_id]->getHRTF();
}

int SOFAData::loadSofaFile(const char* filePath, int hostSampleRate){
    
    /* Some things are defined as variables in the SOFA-Convention, but are assumed here for the sake of simplicity:
     - Data.SamplingRate.Units is always "hertz"
     - Data.Delay is Zero, the TOA information is contained in the FIRs
     - Data.IR has always three dimensions
     - Data.IR has two receivers = two ears
     
     Furthermore, by now the only accepted DataType is "FIR" and SOFAConvention is "SimpleFreeFieldHRIR"
    */
    
    //open File in read mode (NC_NOWRITE) and get netcdf-ID of Sofa-File
    int  status;               /* error status */
    int  ncid;                 /* netCDF ID */
    if ((status = nc_open(filePath, NC_NOWRITE, &ncid)))
        return 1;
    
    //Get SOFA Data Type (should be FIR)
    size_t DataType_len;
    if ((status = nc_inq_attlen(ncid, NC_GLOBAL, "DataType", &DataType_len)))
        return 2;
    char DataType[DataType_len+1];
    if ((status = nc_get_att(ncid, NC_GLOBAL, "DataType", &DataType)))
        return 3;
    //Check if Type is FIR, else we cannot handle it
    if(strncmp(DataType, "FIR", DataType_len)){
        //error((char*)"Type is not FIR. Only FIR supported");
        return 4;
    }
    
    sofaMetadata.dataType = DataType;
    
    //Get Sampling Rate
    int SamplingRate, SamplingRate_id;
    status = nc_inq_varid(ncid, "Data.SamplingRate", &SamplingRate_id);
    status += nc_get_var_int(ncid, SamplingRate_id, &SamplingRate);
    if(status != NC_NOERR){
        //error((char*)"Load Sofa: Could not read Sample Rate");
        nc_close(ncid);
        return 5;
    }
    sofaMetadata.sampleRate = SamplingRate;
    
    
    //Get various Metadata
    size_t organizationLength;
    if((status = nc_inq_attlen(ncid, NC_GLOBAL, "Organization", &organizationLength)))
        return 6;
    char organization[organizationLength+1];
    if ((status = nc_get_att(ncid, NC_GLOBAL, "Organization", &organization)))
        return 7;
    
    sofaMetadata.organization = organization;
    
    /*
     .
     .  more to be added
     .
     */
    /* -- check if attribute "SOFAConventions" is "SimpleFreeFieldHRIR": -- */
    size_t i_att_len;
    nc_inq_attlen(ncid, NC_GLOBAL, "SOFAConventions", &i_att_len);
    char sofa_conventions[i_att_len + 1];
    nc_get_att_text(ncid, NC_GLOBAL, "SOFAConventions", sofa_conventions);
    *(sofa_conventions + i_att_len ) = 0; //??
    if ( strncmp( "SimpleFreeFieldHRIR" , sofa_conventions, i_att_len ) )
    {
        //error((char*) "Not a SimpleFreeFieldHRIR file!");
        nc_close(ncid);
        return 8;
    }
    

    
    //Get netcdf-ID of IR Data
    int DataIR_id;
    if ((status = nc_inq_varid(ncid, "Data.IR", &DataIR_id)))//Get Impulse Resopnse Data ID
        return 9;
    
    //Get Dimensions of Data IR
    int DataIR_dimidsp[MAX_VAR_DIMS];
    size_t dimM_len;//Number of Measurements
    size_t dimR_len;//Number of Receivers, in case of two ears dimR_len=2
    size_t dimN_len;//Number of DataSamples describing each Measurement
    
    if ((status = nc_inq_var(ncid, DataIR_id, 0, 0, 0, DataIR_dimidsp, 0)))
        return 10;
    if ((status = nc_inq_dimlen(ncid, DataIR_dimidsp[0], &dimM_len)))
        return 11;
    if ((status = nc_inq_dimlen(ncid, DataIR_dimidsp[1], &dimR_len)))
        return 12;
    if ((status = nc_inq_dimlen(ncid, DataIR_dimidsp[2], &dimN_len)))
        return 13;
    if(dimR_len != 2){
        //error((char*) "Only 2 ears allowed!");
        return 14;
    }
    sofaMetadata.numSamples = dimN_len; //Store Value in Struct metadata_struct
    sofaMetadata.numMeasurements = dimM_len;
    sofaMetadata.numReceivers = dimR_len;
    
    
    //now that the sampleRate of the SOFA, the host sample rate and the length N is known, the length of the interpolated HRTF can be calculated
    lengthOfHRIR = getIRLengthForNewSampleRate(dimN_len, sofaMetadata.sampleRate, hostSampleRate);
    lengthOfFFT = 2 * lengthOfHRIR;
    lengthOfHRTF = (lengthOfFFT * 0.5) + 1;
    
    //Get Source Positions (Azimuth, Elevation, Distance)
    int SourcePosition_id;
    if ((status = nc_inq_varid(ncid, "SourcePosition", &SourcePosition_id)))
        return 15;
    float* SourcePosition = NULL;
    SourcePosition = (float*)malloc(sizeof(float) * 3 * dimM_len); //Allocate Memory for Sourcepositions of each Measurement
    if ((status = nc_get_var_float(ncid, SourcePosition_id, SourcePosition)))// Store Sourceposition Data to Array
    {
        free(SourcePosition);
        return 16;
    };
    
    //Get Impluse Responses
    float *DataIR = NULL;
    DataIR = (float*)malloc(dimM_len * dimR_len * dimN_len * sizeof(float)); //Allocate Memory for Impulse Responses, for performance reasons data must be stored in arry instead of matrix
    if ((status = nc_get_var_float(ncid, DataIR_id, DataIR))) //Read and write Data IR to variable Data IR
    {
        free(SourcePosition);
        free(DataIR);
        return 17;
    };
    
    int numZeroElevation = 0;
    int u = 0;
    for (int i = 0; i < dimM_len; i++) {
        if (SourcePosition[u + 1] == 0) {
            numZeroElevation++;
        }
        u += 3;
    }
    sofaMetadata.numMeasurements0ev = numZeroElevation;
    
    loadedHRIRs = (Single_HRIR_Measurement**)malloc(sofaMetadata.numMeasurements * sizeof(Single_HRIR_Measurement));
    if(loadedHRIRs == NULL){
        //error((char*)"Couldn´t allocate memory for HRTF Data");
        return 666;
    }
//    printf("\n Size of HRIRMeasurement: %lu byte", sizeof(Single_HRIR_Measurement));
//    printf("\n Allocated memory: %lu byte", sofaMetadata.numMeasurements * sizeof(Single_HRIR_Measurement));
//    
    int i = 0, j = 0, l = 0, x = 0;
    for (i = 0; i < dimM_len; i++) {
        
        Single_HRIR_Measurement *measurement_object = new Single_HRIR_Measurement(lengthOfHRIR, lengthOfHRTF);
        //Temporary storage of HRIR-Data
        float *IR_Left = (float *)malloc(dimN_len * sizeof(float));
        float *IR_Right = (float *)malloc(dimN_len * sizeof(float));
        
        for (j = 0; j < dimN_len; j++) {
            IR_Right[j] = DataIR[i*(dimR_len*dimN_len) + j];
            
            IR_Left[j] = DataIR[i*(dimR_len*dimN_len) + 1 * dimN_len + j];
            
        };
        
        sampleRateConversion(IR_Right, measurement_object->getIR_Right(), dimN_len, lengthOfHRIR, sofaMetadata.sampleRate, hostSampleRate);
        sampleRateConversion(IR_Left, measurement_object->getIR_Left(), dimN_len, lengthOfHRIR, sofaMetadata.sampleRate, hostSampleRate);
        
        measurement_object->setValues(SourcePosition[l], SourcePosition[l + 1], SourcePosition[l + 2]);
        measurement_object->index = i;
        loadedHRIRs[i] = measurement_object;
        free(IR_Left);
        free(IR_Right);
        x++;
        
        l += 3;
    };
    
    
    
    
    free(SourcePosition);
    free(DataIR);
    if ((status = nc_close(ncid)))
    {
        //wenn es einem Fehler beim schließen gibt, muss ja nicht gleich alles weggeworfen werden
//        free(loadedHRIRs);
//        loadedHRIRs = NULL;
        return 1;
    };
    //SOFAFile_loaded_flag = 1;
    return 0;

}



sofaMetadataStruct SOFAData::getMetadata(){
    return sofaMetadata;
}



#pragma mark HELPERS

int sampleRateConversion(float* inBuffer, float* outBuffer, int n_inBuffer, int n_outBuffer, int originalSampleRate, int newSampleRate){
    
    float frequenzFaktor = (float)originalSampleRate / (float)newSampleRate;
    float f_index = 0.0f;
    int i_index = 0;
    float f_bruch = 0;
    int i_fganzezahl = 0;
    while ((i_fganzezahl+1) < n_inBuffer){
        if(i_index >= n_outBuffer || (i_fganzezahl+1) >= n_inBuffer) return 1;
        //Linear interpolieren
        outBuffer[i_index] = inBuffer[i_fganzezahl] * (1.0f - f_bruch) + inBuffer[i_fganzezahl + 1] * f_bruch;
        outBuffer[i_index] *= frequenzFaktor;
        //Berechnungen fuer naechste Runde.
        i_index++;
        f_index = i_index * frequenzFaktor;
        i_fganzezahl = (int)f_index;
        f_bruch = f_index - i_fganzezahl;
    }
    while(i_index < n_outBuffer){
        outBuffer[i_index] = 0.0;
        i_index++;
    }
    
    return 0;
    
}


int getIRLengthForNewSampleRate(int IR_Length, int original_SR, int new_SR){
    
    float frequenzFaktor = (float)original_SR / (float)new_SR;
    float newFirLength_f = (float)(IR_Length) / frequenzFaktor; //z.B. 128 bei 44.1kHz ---> 128/(44.1/96) = 278.6 bei 96kHz
    int fir_length = 2;
    while (fir_length < newFirLength_f) {
        fir_length *= 2;
    }
    
    return fir_length;
}


// TODO error Handling: kopiert, sollte hier eingefügt werden

//int SofaPanAudioProcessor::ErrorHandling_LoadSOFAFile(int status) {
//    switch (status) {
//        case 1:
//            pathToSOFAFile = String(SOFA_DEFAULT_PATH);
//            if (LoadSOFAFile()) {
//                createPassThrough_FIR();
//                AlertWindow::showNativeDialogBox("Binaural Renderer", "Could not find Default HRTF. No HRTF loaded.", false);
//            }
//            else {
//                AlertWindow::showNativeDialogBox("Binaural Renderer", "SOFA File could not be loaded. Default HRTF loaded.", false);
//            }
//    }
//    
//    //juce::String errorstring = static_cast <String> (loadSofa_errorstatus);
//    //AlertWindow::showMessageBox(AlertWindow::NoIcon, "SOFA Errorhandling", errorstring);
//    return 0;
//    
//}
//
//
//
//void SofaPanAudioProcessor::createPassThrough_FIR(){
//    metadata_struct.HRIR_Samplingrate = 44100;
//    metadata_struct.HRIR_numSamples = 256;
//    metadata_struct.HRIR_numMeasurements = 1;
//    myFile = (HRIR_Measurement**)malloc(1 * sizeof(HRIR_Measurement));
//    HRIR_Measurement *measurement_object = new HRIR_Measurement(256);
//    
//    
//    float *IR_Left = (float *)malloc(256 * sizeof(float));
//    float *IR_Right = (float *)malloc(256 * sizeof(float));
//    
//    IR_Left[0]  = 1.0;
//    IR_Right[0] = 1.0;
//    for (int j = 1; j < 256; j++) {
//        IR_Left[j]  = 0.0;
//        IR_Right[j] = 0.0;
//    };
//    float Azimuth = 0.0;
//    float Elevation = 0.0;
//    float Distance = 0.0;
//    measurement_object->setValues(IR_Left, IR_Right, Elevation, Azimuth, Distance);
//    myFile[0] = measurement_object;
//    SOFAFile_loaded_flag = 1;
//}
