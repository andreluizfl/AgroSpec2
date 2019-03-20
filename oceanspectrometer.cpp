#include "oceanspectrometer.h"
#include "module/spectrometermodule.h"
#include "api/seabreezeapi/SeaBreezeAPI.h"
#include <stdio.h>
#include <string.h>

namespace Agrospec {


OceanSpectrometer::SerialFeature::SerialFeature(){
    this->ID = -1;
    this->serial_length = 0;
    this->serial = nullptr;
}


OceanSpectrometer::SerialFeature::~SerialFeature(){
    if(this->serial!=nullptr){
        delete this->serial;
    }
}

OceanSpectrometer::SpectrometerFeature::SpectrometerFeature(){
    this->ID = -1;
    this->integration_time_min = -1;
    this->integration_time_max = -1;
    this->integration_time_current = -1;;
    this->intensity_max = 0;
    this->resolution = 0;
    this->wavelengths = nullptr;
    this->spectrum = nullptr;
}

OceanSpectrometer::SpectrometerFeature::~SpectrometerFeature(){
    if(this->wavelengths!=nullptr){
        delete this->wavelengths;
    }
    if(this->spectrum!=nullptr){
        delete this->spectrum;
    }
}

OceanSpectrometer::OceanSpectrometer(int ID): ISpectrometer()
{
    int i;
    this->deviceID = ID;
    SeaBreezeAPI* api = SpectrometerModule::GetSeaBreezeAPIInstance();
    int error=0;
    if(SpectrometerModule::openDevice(ID,&error)==0){
        printf("Has opened!!\n");
    }else{
        printf("Opened error %d!!\n",error);
    }

    //Serial Number
    serial_num_features = 0;
    serial_features = nullptr;

    serial_num_features = SpectrometerModule::getNumberOfSerialNumberFeatures(ID,&error);
    if(serial_num_features>0){
        long * serial_features_ids = new long[serial_num_features];
        serial_features = new SerialFeature*[serial_num_features];
        int num_fet = SpectrometerModule::getSerialNumberFeatures(ID,&error,serial_features_ids,serial_num_features);
        for(i=0;i<serial_num_features;i++){
            serial_features[i] = new SerialFeature();
            serial_features[i]->ID = serial_features_ids[i];
            serial_features[i]->serial_length = (int)SpectrometerModule::getSerialNumberMaximumLength(ID,serial_features[i]->ID,&error);
            serial_features[i]->serial = new char[serial_features[i]->serial_length];
            SpectrometerModule::getSerialNumber(ID,serial_features[i]->ID,&error,serial_features[i]->serial,serial_features[i]->serial_length);
            printf("Serial: %s!!\n",(int)serial_features[i]->serial);
        }
        delete serial_features_ids;
    }

    //Spectrometer
    spectrometer_num_features = 0;
    spectrometer_features = nullptr;

    spectrometer_num_features = SpectrometerModule::getNumberOfSpectrometerFeatures(ID,&error);
    printf("spectrometer_num_features:%d\n",spectrometer_num_features);
    fflush(stdout);
    if(spectrometer_num_features>0){

        long* spectrometer_features_ids =  new long[spectrometer_num_features];
        spectrometer_features = new SpectrometerFeature*[spectrometer_num_features];

        SpectrometerModule::getSpectrometerFeatures(ID,&error,spectrometer_features_ids,spectrometer_num_features);

        if(error!=0){printf("Err:%d",error);}
        for(i=0;i<spectrometer_num_features;i++){
            spectrometer_features[i] = new SpectrometerFeature();
            spectrometer_features[i]->ID = spectrometer_features_ids[i];

            spectrometer_features[i]->integration_time_min = SpectrometerModule::spectrometerGetMinimumIntegrationTimeMicros(ID,spectrometer_features[i]->ID,&error);
            if(error!=0){printf("Err:%d",error);}
            spectrometer_features[i]->integration_time_max = SpectrometerModule::spectrometerGetMaximumIntegrationTimeMicros(ID,spectrometer_features[i]->ID,&error);
            spectrometer_features[i]->integration_time_max = spectrometer_features[i]->integration_time_max>MAX_INTEGRATION_TIME?MAX_INTEGRATION_TIME:spectrometer_features[i]->integration_time_max;
            if(error!=0){printf("Err:%d",error);}
            spectrometer_features[i]->integration_time_current = spectrometer_features[i]->integration_time_min;
            SpectrometerModule::spectrometerSetIntegrationTimeMicros(ID,spectrometer_features[i]->ID,&error,spectrometer_features[i]->integration_time_current);
            if(error!=0){printf("Err:%d",error);}

            SpectrometerModule::spectrometerSetTriggerMode(ID,spectrometer_features[i]->ID,&error,0);
            if(error!=0)printf("Err:%d",error);


            spectrometer_features[i]->intensity_max = SpectrometerModule::spectrometerGetMaximumIntensity(ID,spectrometer_features[i]->ID,&error);
            if(error!=0){printf("Err:%d",error);}

            spectrometer_features[i]->resolution = SpectrometerModule::spectrometerGetFormattedSpectrumLength(ID,spectrometer_features[i]->ID,&error);
            if(error!=0){printf("Err:%d",error);}

            spectrometer_features[i]->wavelengths = new double[spectrometer_features[i]->resolution];
            spectrometer_features[i]->spectrum = new double[spectrometer_features[i]->resolution];

            SpectrometerModule::spectrometerGetWavelengths(ID,spectrometer_features[i]->ID,&error,spectrometer_features[i]->wavelengths,spectrometer_features[i]->resolution);
            if(error!=0){printf("Err:%d",error);}

            //memset(spectrometer_features[i]->spectrum,0,sizeof(double)*spectrometer_features[i]->resolution);

            SpectrometerModule::spectrometerGetFormattedSpectrum(ID,spectrometer_features[i]->ID,&error,spectrometer_features[i]->spectrum,spectrometer_features[i]->resolution);
            if(error!=0){printf("Err:%d",error);}
        }
        delete spectrometer_features_ids;

    }

    //int shutter = sbapi_get_number_of_shutter_features (ID, &error);
    //printf("shutter:%d\n",shutter);
    //int ligth_source = sbapi_get_number_of_light_source_features (ID, &error);
    //printf("ligth_source:%d\n",ligth_source);

    //int sp = sbapi_get_number_of_spectrum_processing_features (ID, &error);
    //printf("spectrum_processing:%d\n",sp);
}

OceanSpectrometer::~OceanSpectrometer()
{
    int i,error;
    SeaBreezeAPI* api = SpectrometerModule::GetSeaBreezeAPIInstance();

    /*for(i=0;i<this->deleting_event_stack.size();i++){
        DeletingSpectrometer event = this->deleting_event_stack[i];
        if(event!=nullptr){
            event(this);
        }
    }*/

    //printf("Herdeira Destroi!\n");
    if(this->serial_features!=nullptr){
        for(i=0;i<this->serial_num_features;i++){
            delete this->serial_features[i];
        }
        delete this->serial_features;
    }

    if(this->spectrometer_features!=nullptr){
        for(i=0;i<this->spectrometer_num_features;i++){
            delete this->spectrometer_features[i];
        }
        delete this->spectrometer_features;
    }
    SpectrometerModule::closeDevice(this->deviceID,&error);
    if(error!=0){printf("Err:%d",error);}
}

const char* OceanSpectrometer::GetSerial(){
    if(this->serial_num_features>0 && this->serial_features!=nullptr){
        return (const char*)this->serial_features[0]->serial;
    }
}

double* OceanSpectrometer::GetTemperature(){
    return nullptr;
}

unsigned long OceanSpectrometer::GetIntegrationTimeMin(){
    if(this->spectrometer_num_features>0 && this->spectrometer_features!=nullptr){
        return this->spectrometer_features[0]->integration_time_min;
    }else{
        return 0;
    }
}

unsigned long OceanSpectrometer::GetIntegrationTimeMax(){
    if(this->spectrometer_num_features>0 && this->spectrometer_features!=nullptr){
        return this->spectrometer_features[0]->integration_time_max;
    }else{
        return 0;
    }
}

unsigned long OceanSpectrometer::GetIntegrationTime(){
    if(this->spectrometer_num_features>0 && this->spectrometer_features!=nullptr){
        return this->spectrometer_features[0]->integration_time_current;
    }else{
        return 0;
    }
}

void OceanSpectrometer::SetIntegrationTime(unsigned long it){
    int error=0;
    if(this->spectrometer_num_features>0 && this->spectrometer_features!=nullptr){
        spectrometer_features[0]->integration_time_current = it;
        SpectrometerModule::spectrometerSetIntegrationTimeMicros(this->deviceID,spectrometer_features[0]->ID,&error,spectrometer_features[0]->integration_time_current);
        if(error!=0){printf("Err:%d",error);}
    }
}

double OceanSpectrometer::GetMaxIntensity(){
    if(spectrometer_num_features>0 && this->spectrometer_features!=nullptr){
        return spectrometer_features[0]->intensity_max;
    }else{
        return 0.0;
    }
}

int OceanSpectrometer::GetNumberofWavelenghts(){
    if(spectrometer_num_features>0 && this->spectrometer_features!=nullptr){
        return spectrometer_features[0]->resolution;
    }else{
        return 0.0;
    }
}

double* OceanSpectrometer::GetWavelenghts(){
    if(spectrometer_num_features>0 && this->spectrometer_features!=nullptr){
        return spectrometer_features[0]->wavelengths;
    }else{
        return nullptr;
    }
}

double* OceanSpectrometer::GetSpectrum(){
    if(spectrometer_num_features>0 && this->spectrometer_features!=nullptr){
        return spectrometer_features[0]->spectrum;
    }else{
        return nullptr;
    }
}

bool OceanSpectrometer::UpdateRanged(int a, int b){
    int i,error;
    if(spectrometer_num_features>0 && this->spectrometer_features!=nullptr){
        //double * temp = spectrometer_features[0]->spectrum;
        //spectrometer_features[0]->spectrum = new double[spectrometer_features[0]->resolution];
        memset(spectrometer_features[0]->spectrum,0,sizeof(double)*spectrometer_features[0]->resolution);
        SpectrometerModule::spectrometerGetFormattedSpectrumRange(this->deviceID,spectrometer_features[0]->ID,&error,spectrometer_features[0]->spectrum,spectrometer_features[0]->resolution,a,b);
        if(error!=0){
            printf("Err Update:%d",error);
            return false;
        }
        return true;
        /*for(i=0;i<spectrometer_features[0]->resolution;i++){
            if(spectrometer_features[0]->spectrum[i]!=temp[i]){
                //printf("%lf %lf\n",spectrometer_features[0]->spectrum[i],temp[i]);
            }
        }*/
        //delete temp;
    }else{
         return false;
    }
}


bool OceanSpectrometer::UpdateAll(){
    int i,error;
    if(spectrometer_num_features>0 && this->spectrometer_features!=nullptr){
        //double * temp = spectrometer_features[0]->spectrum;
        //spectrometer_features[0]->spectrum = new double[spectrometer_features[0]->resolution];
        memset(spectrometer_features[0]->spectrum,0,sizeof(double)*spectrometer_features[0]->resolution);
        SpectrometerModule::spectrometerGetFormattedSpectrum(this->deviceID,spectrometer_features[0]->ID,&error,spectrometer_features[0]->spectrum,spectrometer_features[0]->resolution);
        if(error!=0){
            printf("Err Update:%d",error);
            return false;
        }
        return true;
        /*for(i=0;i<spectrometer_features[0]->resolution;i++){
            if(spectrometer_features[0]->spectrum[i]!=temp[i]){
                //printf("%lf %lf\n",spectrometer_features[0]->spectrum[i],temp[i]);
            }
        }*/
        //delete temp;
    }else{
         return false;
    }
}

}
