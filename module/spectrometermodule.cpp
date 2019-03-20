#include "spectrometermodule.h"
#include "oceanspectrometer.h"
#include "listener.h"
#include <QThread>
#include <stdio.h>
#include "api/seabreezeapi/SeaBreezeAPI_Impl.h"
#include "datamanipulation.h"
#include "native/system/System.h"
#include "listener.h"

namespace Agrospec {

enum FUNC_NAMES{
    fn_unknown, //1
    fn_probeDevices, //2
    fn_getDeviceIDs, //3
    fn_openDevice,  //4
    fn_closeDevice,
    fn_getNumberOfSerialNumberFeatures,
    fn_getSerialNumberFeatures,
    fn_getSerialNumberMaximumLength,
    fn_getSerialNumber,
    fn_getNumberOfSpectrometerFeatures,
    fn_getSpectrometerFeatures,
    fn_spectrometerGetMinimumIntegrationTimeMicros,
    fn_spectrometerGetMaximumIntegrationTimeMicros,
    fn_spectrometerSetIntegrationTimeMicros,
    fn_spectrometerSetTriggerMode,
    fn_spectrometerGetMaximumIntensity,
    fn_spectrometerGetFormattedSpectrumLength,
    fn_spectrometerGetWavelengths,
    fn_spectrometerGetFormattedSpectrum,
    fn_spectrometerGetFormattedSpectrumRange
};


static int func_cod_counter = 0;
Listener* SpectrometerModule::listenerobj = nullptr;
FTDIcomm* SpectrometerModule::ftdi = nullptr;
void* SpectrometerModule::FunctionTable;
SeaBreezeAPI* SpectrometerModule::SeaBreezeModuleInstance = nullptr;
unsigned long  SpectrometerModule::NumberOfDevices = 0;
long*  SpectrometerModule::DeviceIDs = nullptr;
vector<ISpectrometer*>  SpectrometerModule::Spectrometers;

bool  SpectrometerModule::isremote = false;
bool  SpectrometerModule::issource = false;

double* SpectrometerModule::wavelengths_api;
float* SpectrometerModule::wavelengths_comm;
int SpectrometerModule::wavelengths_len;
double* SpectrometerModule::spectrum_api;
float* SpectrometerModule::spectrum_comm;
short* SpectrometerModule::spectrum_comm2;

SpectrometerModule::SpectrometerModule()
{

}

void SpectrometerModule::Initialize(){
    NumberOfDevices = 0;
    DeviceIDs = nullptr;
    //Spectrometers = new vector<ISpectrometer*>;
    //sbapi_probe_devices();
    sbapi_initialize();
    //SeaBreezeModuleInstance = SeaBreezeAPI::getInstance();

    wavelengths_api = nullptr;
    wavelengths_comm = nullptr;
    wavelengths_len = 0;
    spectrum_api = nullptr;
    spectrum_comm = nullptr;
    spectrum_comm2 = nullptr;

    //FTDI
    ftdi = new FTDIcomm();
    if(ftdi!=NULL){
        ftdi->InitializeCommunication(nullptr);
        ftdi->ConfigureCommunication(nullptr);
        listenerobj = new Listener(ftdi);
    }
}

void SpectrometerModule::Release(){

    if(wavelengths_api != nullptr){
        free(wavelengths_api);
    }
    if(wavelengths_comm != nullptr){
        free(wavelengths_comm);
    }
    if(spectrum_api != nullptr){
        free(spectrum_api);
    }
    if(spectrum_comm != nullptr){
        free(spectrum_comm);
    }
    if(spectrum_comm2 != nullptr){
        free(spectrum_comm2);
    }

    if(listenerobj!=nullptr){
        if(listenerobj->isRunning()){
            listenerobj->terminate();
            listenerobj->wait();
            //while(listenerobj->isRunning()){}
        }
        delete listenerobj;
    }
    //if(Spectrometers!=nullptr){
        while (!Spectrometers.empty())
        {
            delete Spectrometers.back();
            Spectrometers.pop_back();
        }
        Spectrometers.clear();
        //delete Spectrometers;
    //}
    //SeaBreezeAPI::shutdown();
    sbapi_shutdown();
    if(ftdi!=nullptr){
        ftdi->ReleaseCommunication();
        delete ftdi;
    }
}

void SpectrometerModule::EnableRemote(bool enabled){
    isremote = enabled;
    ftdi->ClearInBuffer();
    ftdi->ClearOutBuffer();
}

bool SpectrometerModule::IsRemote(){
    return isremote;
}

void SpectrometerModule::EnableSource(bool enabled){
    issource = enabled;
    if(issource){
        listenerobj->start();
        printf("Listener started!\n");
        fflush(stdout);
    }else{
        if(listenerobj!=nullptr){
            if(listenerobj->isRunning()){
                listenerobj->requestInterruption();
                listenerobj->quit();
                listenerobj->wait();
                printf("Listener stopped!\n");
                fflush(stdout);
            }
        }

    }
}

const SeaBreezeAPI* SpectrometerModule::GetSeaBreezeAPIInstance(){
    return SeaBreezeModuleInstance;
}

void SpectrometerModule::ReleaseSpectrometers()
{
    bool perform_delay = (!Spectrometers.empty() || DeviceIDs!=nullptr);

   while (!Spectrometers.empty())
   {
       delete Spectrometers.back();
       Spectrometers.pop_back();
   }
   Spectrometers.clear();
   if(DeviceIDs!=nullptr){
       delete DeviceIDs;
       DeviceIDs = nullptr;
   }
   if(perform_delay){
       QThread::currentThread()->msleep(500);
   }
}

void SpectrometerModule::DetectSpectrometers()
{
   int i;
   SpectrometerModule::ReleaseSpectrometers();

   /*
   //int nd = SeaBreezeModuleInstance->getNumberOfDeviceIDs();
   //printf("%d %d\n",NumberOfDevices,nd);
   while (!Spectrometers.empty())
   {
       delete Spectrometers.back();
       Spectrometers.pop_back();
   }
   Spectrometers.clear();
   if(DeviceIDs!=nullptr){
       delete DeviceIDs;
       DeviceIDs = nullptr;
   }*/

   NumberOfDevices = SpectrometerModule::probeDevices();
   //NumberOfDevices = SeaBreezeModuleInstance->probeDevices();

   if(NumberOfDevices>0){
        //nd = SeaBreezeModuleInstance->getNumberOfDeviceIDs();
        //printf("%d %d\n",NumberOfDevices,nd);
        if(DeviceIDs!=nullptr){
            delete DeviceIDs;
        }
        DeviceIDs = new long[NumberOfDevices];
        SpectrometerModule::getDeviceIDs(DeviceIDs,NumberOfDevices);

        for(i=0;i<NumberOfDevices;i++){
            OceanSpectrometer *os = new OceanSpectrometer(DeviceIDs[i]);
            Spectrometers.push_back(os);
        }
   }
}

void SpectrometerModule::Listening(){
    int bytes;
    int func_cod,err;
    while(true){
       err = 0;
       bytes = 0;
       func_cod = 0;
       while(!listenerobj->isInterruptionRequested() && (bytes = ftdi->ReceiveData((unsigned char*)&func_cod, sizeof(int))) == 0){
       }
       if(listenerobj->isInterruptionRequested()){
           printf("isInterruptionRequested\n");
           fflush(stdout);
           return;
       }
       printf("func_cod_requested:%d\n",func_cod);
       fflush(stdout);
       switch((FUNC_NAMES)func_cod){
       case fn_unknown:
           printf("purge buffers got 0\n");
           fflush(stdout);
            ftdi->ClearInBuffer();
            ftdi->ClearOutBuffer();
            break;
       case fn_probeDevices:
            NumberOfDevices = Agrospec::SpectrometerModule::probeDevices();
            break;

        case fn_getDeviceIDs:
            if(NumberOfDevices>0){
                if(DeviceIDs==nullptr){
                    DeviceIDs = new long[NumberOfDevices];
                }
                DeviceIDs = new long[NumberOfDevices];
                err = Agrospec::SpectrometerModule::getDeviceIDs(DeviceIDs,NumberOfDevices);
            }
            break;
        case fn_openDevice:
           Agrospec::SpectrometerModule::openDevice(0,&err);
           break;
        case fn_closeDevice:
           Agrospec::SpectrometerModule::closeDevice(0,&err);
           break;
        case fn_getNumberOfSerialNumberFeatures:
            Agrospec::SpectrometerModule::getNumberOfSerialNumberFeatures(0, &err);
            break;
        case fn_getSerialNumberFeatures:
            Agrospec::SpectrometerModule::getSerialNumberFeatures(0,&err, nullptr, 0);
            break;
        case fn_getSerialNumberMaximumLength:
           Agrospec::SpectrometerModule::getSerialNumberMaximumLength(0,0,&err);
           break;
        case fn_getSerialNumber:
           Agrospec::SpectrometerModule::getSerialNumber(0,0,&err,nullptr,0);
           break;

        case fn_getNumberOfSpectrometerFeatures:
           Agrospec::SpectrometerModule::getNumberOfSpectrometerFeatures(0,&err);
           break;

        case fn_getSpectrometerFeatures:
           Agrospec::SpectrometerModule::getSpectrometerFeatures(0,&err,nullptr,0);
           break;

        case fn_spectrometerGetMinimumIntegrationTimeMicros:
           Agrospec::SpectrometerModule::spectrometerGetMinimumIntegrationTimeMicros(0,0,&err);
           break;

        case fn_spectrometerGetMaximumIntegrationTimeMicros:
           Agrospec::SpectrometerModule::spectrometerGetMaximumIntegrationTimeMicros(0,0,&err);
           break;

        case fn_spectrometerSetIntegrationTimeMicros:
           Agrospec::SpectrometerModule::spectrometerSetIntegrationTimeMicros(0,0,&err,0);
           break;

        case fn_spectrometerSetTriggerMode:
           Agrospec::SpectrometerModule::spectrometerSetTriggerMode(0,0,&err,0);
           break;

        case fn_spectrometerGetMaximumIntensity:
           Agrospec::SpectrometerModule::spectrometerGetMaximumIntensity(0,0,&err);
           break;

       case fn_spectrometerGetFormattedSpectrumLength:
           Agrospec::SpectrometerModule::spectrometerGetFormattedSpectrumLength(0,0,&err);
           break;

        case fn_spectrometerGetWavelengths:
           Agrospec::SpectrometerModule::spectrometerGetWavelengths(0,0,&err,nullptr,0);
           break;

        case fn_spectrometerGetFormattedSpectrum:
           Agrospec::SpectrometerModule::spectrometerGetFormattedSpectrum(0,0,&err,nullptr,0);
           break;
       case fn_spectrometerGetFormattedSpectrumRange:
          Agrospec::SpectrometerModule::spectrometerGetFormattedSpectrumRange(0,0,&err,nullptr,0,0,0);
          break;

        default:
           printf("default opt\n");
           fflush(stdout);
           //ftdi_usb_purge_buffers(ftdi);
           break;
       }
       //ftdi_usb_purge_buffers(ftdi);
    }
}

int SpectrometerModule::probeDevices(){
    static int func_code = fn_probeDevices;
    int ret=0;
    int bytes = 0;
    printf("func_cod_call:%d\n",func_code);
    if(!IsRemote()){
        int num_specs = sbapi_probe_devices();//SeaBreezeModuleInstance->probeDevices();
        printf("IsNotRemote. probeDevices:%d\n",num_specs);
        return num_specs;
    }else{

        if(issource){
            //ret = SeaBreezeModuleInstance->probeDevices();
            ret = sbapi_probe_devices();
            NumberOfDevices = ret;
            printf("sending devs:%d\n",ret);
            fflush(stdout);
            //seabreeze::System::sleepMilliseconds(100);
            ftdi->SendData((unsigned char*)&ret,sizeof(int));
            printf("sent devs:%d\n",ret);
            fflush(stdout);
        }else{
            printf("sending func_cod\n");
            fflush(stdout);
            ftdi->SendData((unsigned char*)&func_code,sizeof(int));

            printf("receiving devs:%d\n",ret);
            fflush(stdout);
            while( (bytes = ftdi->ReceiveData((unsigned char*)&ret,sizeof(int))) == 0){}
            printf("received devs:%d\n",ret);
            fflush(stdout);
        }
        return ret;
    }
}

#define delay 0

void SpectrometerModule::SendParameters(void* par,int size){
    int bytes;
    seabreeze::System::sleepMilliseconds(delay);
    printf("sending...");
    fflush(stdout);
    bytes = ftdi->SendData((unsigned char*)par,size);
    if(bytes != size){
        printf("err sending parameters: bytes received:%d / bytes size:%d\n",bytes,size);
    }else{
        printf("sent... bytes received:%d / bytes size:%d\n",bytes,size);
        fflush(stdout);
        //seabreeze::System::sleepMilliseconds(delay);
    }
}

void SpectrometerModule::ReceiveParameters(void* par,int size){
    int bytes;
    //seabreeze::System::sleepMilliseconds(10);
    printf("receiving...");
    fflush(stdout);
    while( (bytes = ftdi->ReceiveData((unsigned char*)par,size)) == 0){
    }
    if(bytes != size){
        printf("err receiving parameters: bytes received:%d / bytes size:%d\n",bytes,size);
    }else{
        printf("received... bytes received:%d / bytes size:%d, value:%d\n",bytes,size,((int*)par)[0]);
        fflush(stdout);
        seabreeze::System::sleepMilliseconds(delay);
    }
}

int SpectrometerModule::getDeviceIDs(long* deviceIDs,unsigned int numberOfDevices){
    static int func_code = fn_getDeviceIDs;
    printf("func_cod_call:%d\n",func_code);
    int bytes, err=0;

    #pragma pack(push, 1) // exact fit - no padding
    typedef struct getDeviceIDsPar{
        int numberOfDevices;
    }getDeviceIDsPar;
    #pragma pack(pop)

    getDeviceIDsPar par;
    int* local_deviceIDs;
    printf("sizeofStruct:%d\n",sizeof(getDeviceIDsPar));

    if(!IsRemote()){
         //SeaBreezeModuleInstance->getDeviceIDs(deviceIDs,numberOfDevices);
        return sbapi_get_device_ids(deviceIDs,numberOfDevices);
    }else{
        if(issource){
            SpectrometerModule::ReceiveParameters((void*)&par,sizeof(getDeviceIDsPar));
            printf("ParReceived->numdevs:%d\n",par.numberOfDevices);

            //err = SeaBreezeModuleInstance->getDeviceIDs(deviceIDs,par.numberOfDevices);
            err = sbapi_get_device_ids(deviceIDs,par.numberOfDevices);
            local_deviceIDs = new int[par.numberOfDevices];

            for(int i=0;i<par.numberOfDevices;i++){
                local_deviceIDs[i] = static_cast<int>(DeviceIDs[i]);
            }
            //printf("err:%d,devid:%d\n",err,local_deviceIDs[0]);
            ftdi->SendData((unsigned char*)&err,sizeof(int));
            if(err!=0){
                ftdi->SendData((unsigned char*)local_deviceIDs,sizeof(int)*par.numberOfDevices);
            }
            delete local_deviceIDs;
        }else{
            par.numberOfDevices = numberOfDevices;

            printf("ParameterSent->numdevs:%d\n",par.numberOfDevices);

            ftdi->SendData((unsigned char*)&func_code,sizeof(int));
            SpectrometerModule::SendParameters((void*)&par,sizeof(getDeviceIDsPar));
            //ftdi->SendData((unsigned char*)&par.numberOfDevices,sizeof(int));
            local_deviceIDs = new int[par.numberOfDevices];
            while( (bytes = ftdi->ReceiveData((unsigned char*)&err,sizeof(int))) == 0){}
            if(err!=0){
                while( (bytes = ftdi->ReceiveData((unsigned char*)local_deviceIDs,numberOfDevices*sizeof(int))) == 0){}
                for(int i=0;i<par.numberOfDevices;i++){
                    deviceIDs[i] = static_cast<long>(local_deviceIDs[i]);
                }
            }
            delete local_deviceIDs;
        }
    }
}

int SpectrometerModule::openDevice(long ID, int* err){
    static int func_code = fn_openDevice;
    int ret=0;

    #pragma pack(push, 1) // exact fit - no padding
    typedef struct openDevicePar{
        unsigned int ID;
    }openDevicePar;
    #pragma pack(pop)
    openDevicePar par;

    if(!IsRemote()){
        //return SeaBreezeModuleInstance->openDevice(ID, err);
        return sbapi_open_device(ID,err);
    }else{
        if(issource){
            SpectrometerModule::ReceiveParameters((void*)&par,sizeof(openDevicePar));
            //ret = SeaBreezeModuleInstance->openDevice((long)par.ID, err);
            ret = sbapi_open_device((long)par.ID, err);
            ftdi->SendData((unsigned char*)err,sizeof(int));
        }else{
            par.ID = (unsigned int)ID;
            ftdi->SendData((unsigned char*)&func_code,sizeof(int));
            SpectrometerModule::SendParameters((void*)&par,sizeof(openDevicePar));
            int bytes;
            while( (bytes = ftdi->ReceiveData((unsigned char*)err,sizeof(int))) == 0){}
        }
        ret = (*err);
        return ret;
    }
}

void SpectrometerModule::closeDevice(long ID, int* err){
    static int func_code = fn_closeDevice;

    #pragma pack(push, 1) // exact fit - no padding
    typedef struct closeDevicePar{
        unsigned int ID;
    }closeDevicePar;
    #pragma pack(pop)
    closeDevicePar par;

    if(!IsRemote()){
        //SeaBreezeModuleInstance->closeDevice(ID, err);
        sbapi_close_device(ID,err);
    }else{
        if(issource){
            SpectrometerModule::ReceiveParameters((void*)&par,sizeof(closeDevicePar));
            //SeaBreezeModuleInstance->closeDevice((long)par.ID, err);
            sbapi_close_device((long)par.ID, err);
            ftdi->SendData((unsigned char*)err,sizeof(int));
        }else{
            par.ID = (unsigned int)ID;
            ftdi->SendData((unsigned char*)&func_code,sizeof(int));
            SpectrometerModule::SendParameters((void*)&par,sizeof(closeDevicePar));
            int bytes;
            while( (bytes = ftdi->ReceiveData((unsigned char*)err,sizeof(int))) == 0){}
        }
    }
}


/* Serial
int getNumberOfSerialNumberFeatures(long ID, int* err);
int getSerialNumberFeatures(long ID, int* err, long* serial_features_ids, int serial_num_features);
unsigned char getSerialNumberMaximumLength(long ID, long featureID, int* err);
int getSerialNumber(long ID, long featureID, int* err, char* serial, int serial_length);
*/

int SpectrometerModule::getNumberOfSerialNumberFeatures(long ID, int* err){
    static int func_code = fn_getNumberOfSerialNumberFeatures;
    int ret=0;
    printf("getNumberOfSerialNumberFeatures ...\n");
    fflush(stdout);
    #pragma pack(push, 1) // exact fit - no padding
    typedef struct getNumberOfSerialNumberFeaturesPar{
        unsigned int ID;
    }getNumberOfSerialNumberFeaturesPar;
    #pragma pack(pop)
    getNumberOfSerialNumberFeaturesPar par;

    if(!IsRemote()){
        //return SeaBreezeModuleInstance->getNumberOfSerialNumberFeatures(ID, err);
        return sbapi_get_number_of_serial_number_features(ID,err);
    }else{
        if(issource){
            SpectrometerModule::ReceiveParameters((void*)&par,sizeof(getNumberOfSerialNumberFeaturesPar));
            printf("ID serial: %d",par.ID);
            //ret = SeaBreezeModuleInstance->getNumberOfSerialNumberFeatures((long)par.ID, err);
            ret = sbapi_get_number_of_serial_number_features((long)par.ID, err);
            ftdi->SendData((unsigned char*)err,sizeof(int));
            printf("err valueer %d\n",*err);
            if((*err)==0){
                printf("sending ret:%d\n",ret);
                ftdi->SendData((unsigned char*)&ret,sizeof(int));
            }
        }else{
            par.ID = (unsigned int)ID;
            ftdi->SendData((unsigned char*)&func_code,sizeof(int));
            printf("sending par\n");
            SpectrometerModule::SendParameters((void*)&par,sizeof(getNumberOfSerialNumberFeaturesPar));
            int bytes;
            printf("receiving err\n");
            while( (bytes = ftdi->ReceiveData((unsigned char*)err,sizeof(int))) == 0){}
            printf("err valuer %d\n",*err);
            if((*err)==0){
                printf("receiving ret\n");
                while( (bytes = ftdi->ReceiveData((unsigned char*)&ret,sizeof(int))) == 0){}
                printf("received ret:%d\n",ret);
            }
        }
        printf("return3:%d\n",ret);
        return ret;
    }
}

int SpectrometerModule::getSerialNumberFeatures(long ID, int* err, long* serial_features_ids, int serial_num_features){
    static int func_code = fn_getSerialNumberFeatures;
    int ret=0;
    #pragma pack(push, 1) // exact fit - no padding
    typedef struct getSerialNumberFeaturesPar{
        unsigned int ID;
        unsigned int serial_num_features;
    }getSerialNumberFeaturesPar;
    #pragma pack(pop)
    getSerialNumberFeaturesPar par;
    int* serial_features_ids_comm;
    long* serial_features_ids_api;

    if(!IsRemote()){
        //return SeaBreezeModuleInstance->getSerialNumberFeatures(ID,err,serial_features_ids,serial_num_features);
        return sbapi_get_serial_number_features(ID,err,serial_features_ids,serial_num_features);
    }else{
        if(issource){
            SpectrometerModule::ReceiveParameters((void*)&par,sizeof(getSerialNumberFeaturesPar));
            serial_features_ids_comm = new int[par.serial_num_features];
            serial_features_ids_api = new long[par.serial_num_features];

            //ret = SeaBreezeModuleInstance->getSerialNumberFeatures(par.ID, err,serial_features_ids_api,par.serial_num_features);
            ret = sbapi_get_serial_number_features(par.ID, err,serial_features_ids_api,par.serial_num_features);

            for(int i=0;i<par.serial_num_features;i++){
                serial_features_ids_comm[i]=static_cast<int>(serial_features_ids_api[i]);//(int)serial_features_ids_api[i];
            }
            delete serial_features_ids_api;
            ftdi->SendData((unsigned char*)err,sizeof(int));
            if((*err)==0){
                ftdi->SendData((unsigned char*)serial_features_ids_comm,par.serial_num_features*sizeof(int));
                 ret = par.serial_num_features;
            }
            delete serial_features_ids_comm;
        }else{
            ftdi->SendData((unsigned char*)&func_code,sizeof(int));
            par.ID = (unsigned int)ID;
            par.serial_num_features = (unsigned int)serial_num_features;
            SpectrometerModule::SendParameters((void*)&par,sizeof(getSerialNumberFeaturesPar));
            int bytes;
            while( (bytes = ftdi->ReceiveData((unsigned char*)err,sizeof(int))) == 0){}
            if((*err)==0){
                serial_features_ids_comm = new int[par.serial_num_features];
                while( (bytes = ftdi->ReceiveData((unsigned char*)serial_features_ids_comm,par.serial_num_features*sizeof(int))) == 0){}
                for(int i=0;i<par.serial_num_features;i++){
                    serial_features_ids[i]=static_cast<long>(serial_features_ids_comm[i]);//(long)serial_features_ids_comm[i];
                }
                delete serial_features_ids_comm;
                ret = serial_num_features;
            }
        }
        return ret;
    }
}

unsigned char SpectrometerModule::getSerialNumberMaximumLength(long ID, long featureID, int* err){
    static int func_code = fn_getSerialNumberMaximumLength;
    unsigned char ret=0;

    #pragma pack(push, 1) // exact fit - no padding
    typedef struct getSerialNumberMaximumLength{
        unsigned int ID;
        unsigned int featureID;
    }getSerialNumberMaximumLength;
    #pragma pack(pop)
    getSerialNumberMaximumLength par;

    if(!IsRemote()){
        //return SeaBreezeModuleInstance->getSerialNumberMaximumLength(ID, featureID,err);
        return sbapi_get_serial_number_maximum_length(ID, featureID,err);
    }else{
        if(issource){
            SpectrometerModule::ReceiveParameters((void*)&par,sizeof(getSerialNumberMaximumLength));
            //ret = SeaBreezeModuleInstance->getSerialNumberMaximumLength(par.ID, par.featureID,err);
            ret = sbapi_get_serial_number_maximum_length(par.ID, par.featureID,err);
            printf("Seriallen sent: %d!!\n",(int)ret);
            fflush(stdout);
            ftdi->SendData((unsigned char*)err,sizeof(int));
            if((*err)==0){
                ftdi->SendData((unsigned char*)&ret,sizeof(unsigned char));
            }
        }else{
            ftdi->SendData((unsigned char*)&func_code,sizeof(int));
            par.ID = ID;
            par.featureID = featureID;
            SpectrometerModule::SendParameters((void*)&par,sizeof(getSerialNumberMaximumLength));
            int bytes;
            while( (bytes = ftdi->ReceiveData((unsigned char*)err,sizeof(int))) == 0){}
            if((*err)==0){
                while( (bytes = ftdi->ReceiveData((unsigned char*)&ret,sizeof(unsigned char))) == 0){}
                printf("Seriallen received: %d!!\n",(int)ret);
            }
        }
        return ret;
    }
}

int SpectrometerModule::getSerialNumber(long ID, long featureID, int* err, char* serial, int serial_length){
    static int func_code = fn_getSerialNumber;
    int ret=0;

    #pragma pack(push, 1) // exact fit - no padding
    typedef struct getSerialNumberPar{
        unsigned int ID;
        unsigned int featureID;
        int serial_length;
    }getSerialNumberPar;
    #pragma pack(pop)
    getSerialNumberPar par;
    char* serial_comm;
    if(!IsRemote()){
        //return SeaBreezeModuleInstance->getSerialNumber(ID, featureID, err,serial,serial_length);
        return sbapi_get_serial_number(ID, featureID, err,serial,serial_length);
    }else{
        if(issource){
            SpectrometerModule::ReceiveParameters((void*)&par,sizeof(getSerialNumberPar));
            serial_comm = new char[par.serial_length];
            //ret = SeaBreezeModuleInstance->getSerialNumber(par.ID, par.featureID, err,serial_comm,par.serial_length);
            ret = sbapi_get_serial_number(par.ID, par.featureID, err,serial_comm,par.serial_length);
            ftdi->SendData((unsigned char*)err,sizeof(int));
            if((*err)==0){
                ftdi->SendData((unsigned char*)serial_comm,par.serial_length*sizeof(char));
            }
            delete serial_comm;
        }else{
            ftdi->SendData((unsigned char*)&func_code,sizeof(int));
            par.ID = ID;
            par.featureID = featureID;
            par.serial_length = serial_length;
            SpectrometerModule::SendParameters((void*)&par,sizeof(getSerialNumberPar));
            int bytes;
            while( (bytes = ftdi->ReceiveData((unsigned char*)err,sizeof(int))) == 0){}
            if((*err)==0){
                while( (bytes = ftdi->ReceiveData((unsigned char*)serial,serial_length*sizeof(char))) == 0){}
                ret = serial_length;
            }
        }
        return ret;
    }

}

/*
    static int func_code = fn_;
    int ret=0;

    #pragma pack(push, 1) // exact fit - no padding
    typedef struct Par{

    }Par;
    #pragma pack(pop)
    Par par;
    //Local instanciation

    if(!IsRemote()){
        return SeaBreezeModuleInstance->
    }else{
        if(issource){
            SpectrometerModule::ReceiveParameters((void*)&par,sizeof(Par));

            ret = SeaBreezeModuleInstance->
            ftdi->SendData((unsigned char*)err,sizeof(int));
            if((*err)==0){
                //ftdi->SendData((unsigned char*) , );
            }


        }else{
            ftdi->SendData((unsigned char*)&func_code,sizeof(int));
            //Assign Par

            SpectrometerModule::SendParameters((void*)&par,sizeof(Par));
            int bytes;
            while( (bytes = ftdi->ReceiveData((unsigned char*)err,sizeof(int))) == 0){}
            if((*err)==0){
                //while( (bytes = ftdi->ReceiveData((unsigned char*) , )) == 0){}

            }
        }
        return ret;
    }
    */


int SpectrometerModule::getNumberOfSpectrometerFeatures(long ID,int* err){
    static int func_code = fn_getNumberOfSpectrometerFeatures;
    int ret=0;

    #pragma pack(push, 1) // exact fit - no padding
    typedef struct getNumberOfSpectrometerFeaturesPar{
        unsigned int ID;
    }getNumberOfSpectrometerFeaturesPar;
    #pragma pack(pop)
    getNumberOfSpectrometerFeaturesPar par;
    //Local instanciation
    if(!IsRemote()){
        //return SeaBreezeModuleInstance->getNumberOfSpectrometerFeatures(ID, err);
        return sbapi_get_number_of_spectrometer_features(ID, err);
    }else{
        if(issource){
            SpectrometerModule::ReceiveParameters((void*)&par,sizeof(getNumberOfSpectrometerFeaturesPar));
            //ret = SeaBreezeModuleInstance->getNumberOfSpectrometerFeatures(par.ID, err);
            ret  = sbapi_get_number_of_spectrometer_features(par.ID, err);
            ftdi->SendData((unsigned char*)err,sizeof(int));
            if((*err)==0){
                ftdi->SendData((unsigned char*)&ret,sizeof(int));
            }
        }else{
            ftdi->SendData((unsigned char*)&func_code,sizeof(int));
            //Assign Par
            par.ID = static_cast<unsigned int>(ID);
            SpectrometerModule::SendParameters((void*)&par,sizeof(getNumberOfSpectrometerFeaturesPar));
            int bytes;
            while( (bytes = ftdi->ReceiveData((unsigned char*)err,sizeof(int))) == 0){}
            if((*err)==0){
                while( (bytes = ftdi->ReceiveData((unsigned char*)&ret,sizeof(int) )) == 0){}
            }
        }
        return ret;
    }
}

int SpectrometerModule::getSpectrometerFeatures(long ID,int* err,long* spectrometer_features_ids,int spectrometer_num_features){
    static int func_code = fn_getSpectrometerFeatures;
    int ret=0;
    #pragma pack(push, 1) // exact fit - no padding
    typedef struct getSpectrometerFeaturesPar{
        unsigned int ID;
        int spectrometer_num_features;
    }getSpectrometerFeaturesPar;
    #pragma pack(pop)
    getSpectrometerFeaturesPar par;
    //Local instanciation
    int* spectrometer_features_ids_comm;
    long* spectrometer_features_ids_api;

    if(!IsRemote()){
        //return SeaBreezeModuleInstance->getSpectrometerFeatures(ID,err,spectrometer_features_ids,spectrometer_num_features);
        return sbapi_get_spectrometer_features(ID,err,spectrometer_features_ids,spectrometer_num_features);
    }else{
        if(issource){
            SpectrometerModule::ReceiveParameters((void*)&par,sizeof(getSpectrometerFeaturesPar));
            spectrometer_features_ids_comm = new int[par.spectrometer_num_features];
            spectrometer_features_ids_api = new long[par.spectrometer_num_features];
            //ret = SeaBreezeModuleInstance->getSpectrometerFeatures(par.ID,err,spectrometer_features_ids_api,par.spectrometer_num_features);
            ret = sbapi_get_spectrometer_features(par.ID,err,spectrometer_features_ids_api,par.spectrometer_num_features);
            for(int i=0;i<par.spectrometer_num_features;i++){
                spectrometer_features_ids_comm[i]= static_cast<int>(spectrometer_features_ids_api[i]);
            }
            ftdi->SendData((unsigned char*)err,sizeof(int));
            if((*err)==0){
                ftdi->SendData((unsigned char*)spectrometer_features_ids_comm,par.spectrometer_num_features*sizeof(int));
            }
            delete spectrometer_features_ids_comm;
            delete spectrometer_features_ids_api;
        }else{
            ftdi->SendData((unsigned char*)&func_code,sizeof(int));
            //Assign Par
            par.ID = static_cast<unsigned int>(ID);
            par.spectrometer_num_features = spectrometer_num_features;
            SpectrometerModule::SendParameters((void*)&par,sizeof(getSpectrometerFeaturesPar));
            int bytes;
            while( (bytes = ftdi->ReceiveData((unsigned char*)err,sizeof(int))) == 0){}
            if((*err)==0){
                ret = par.spectrometer_num_features;
                spectrometer_features_ids_comm = new int[par.spectrometer_num_features];
                while( (bytes = ftdi->ReceiveData((unsigned char*)spectrometer_features_ids_comm,par.spectrometer_num_features*sizeof(int) )) == 0){}
                for(int i=0;i<par.spectrometer_num_features;i++){
                    spectrometer_features_ids[i]= static_cast<long>(spectrometer_features_ids_comm[i]);
                }
                delete spectrometer_features_ids_comm;
            }
        }
        return ret;
    }
}

unsigned long long int SpectrometerModule::spectrometerGetMinimumIntegrationTimeMicros(long ID,long featureID,int* err){
    static int func_code = fn_spectrometerGetMinimumIntegrationTimeMicros;
    unsigned long long int ret=0;

    #pragma pack(push, 1) // exact fit - no padding
    typedef struct spectrometerGetMinimumIntegrationTimeMicrosPar{
        unsigned int ID;
        unsigned int featureID;
    }spectrometerGetMinimumIntegrationTimeMicrosPar;
    #pragma pack(pop)
    spectrometerGetMinimumIntegrationTimeMicrosPar par;
    //Local instanciation

    if(!IsRemote()){
        //return SeaBreezeModuleInstance->spectrometerGetMinimumIntegrationTimeMicros(ID,featureID,err);
        return sbapi_spectrometer_get_minimum_integration_time_micros(ID,featureID,err);
    }else{
        if(issource){
            SpectrometerModule::ReceiveParameters((void*)&par,sizeof(spectrometerGetMinimumIntegrationTimeMicrosPar));

            //ret = SeaBreezeModuleInstance->spectrometerGetMinimumIntegrationTimeMicros(par.ID,par.featureID,err);
            ret = sbapi_spectrometer_get_minimum_integration_time_micros(par.ID,par.featureID,err);

            ftdi->SendData((unsigned char*)err,sizeof(int));
            if((*err)==0){
                ftdi->SendData((unsigned char*)&ret, sizeof(unsigned long long int));
            }
        }else{
            ftdi->SendData((unsigned char*)&func_code,sizeof(int));
            //Assign Par
            par.ID = static_cast<unsigned int>(ID);
            par.featureID= static_cast<unsigned int>(featureID);
            SpectrometerModule::SendParameters((void*)&par,sizeof(spectrometerGetMinimumIntegrationTimeMicrosPar));
            int bytes;
            while( (bytes = ftdi->ReceiveData((unsigned char*)err,sizeof(int))) == 0){}
            if((*err)==0){
                while( (bytes = ftdi->ReceiveData((unsigned char*)&ret ,sizeof(unsigned long long int))) == 0){}
            }
        }
        return ret;
    }

}

unsigned long long int SpectrometerModule::spectrometerGetMaximumIntegrationTimeMicros(long ID,long featureID,int* err){
    static int func_code = fn_spectrometerGetMaximumIntegrationTimeMicros;
    unsigned long long int ret=0;

    #pragma pack(push, 1) // exact fit - no padding
    typedef struct spectrometerGetMaximumIntegrationTimeMicrosPar{
        unsigned int ID;
        unsigned int featureID;
    }spectrometerGetMaximumIntegrationTimeMicrosPar;
    #pragma pack(pop)
    spectrometerGetMaximumIntegrationTimeMicrosPar par;
    //Local instanciation

    if(!IsRemote()){
        //return SeaBreezeModuleInstance->spectrometerGetMaximumIntegrationTimeMicros(ID,featureID,err);
        //return sbapi_spectrometer_get_maximum_integration_time_micros(ID,featureID,err);
        return MAX_INTEGRATION_TIME;
    }else{
        if(issource){
            SpectrometerModule::ReceiveParameters((void*)&par,sizeof(spectrometerGetMaximumIntegrationTimeMicrosPar));

            //ret = SeaBreezeModuleInstance->spectrometerGetMaximumIntegrationTimeMicros(par.ID,par.featureID,err);
            //ret = sbapi_spectrometer_get_maximum_integration_time_micros(par.ID,par.featureID,err);
            ret = MAX_INTEGRATION_TIME;

            ftdi->SendData((unsigned char*)err,sizeof(int));
            if((*err)==0){
                ftdi->SendData((unsigned char*)&ret,sizeof(unsigned long long int));
            }
        }else{
            ftdi->SendData((unsigned char*)&func_code,sizeof(int));
            //Assign Par
            par.ID=static_cast<unsigned int>(ID);
            par.featureID=static_cast<unsigned int>(featureID);
            SpectrometerModule::SendParameters((void*)&par,sizeof(spectrometerGetMaximumIntegrationTimeMicrosPar));
            int bytes;
            while( (bytes = ftdi->ReceiveData((unsigned char*)err,sizeof(int))) == 0){}
            if((*err)==0){
                while( (bytes = ftdi->ReceiveData((unsigned char*)&ret,sizeof(unsigned long long int))) == 0){}
            }
        }
        return ret;
    }
}

void SpectrometerModule::spectrometerSetIntegrationTimeMicros(long ID,long featureID, int *err,unsigned long long int integration_time){
    static int func_code = fn_spectrometerSetIntegrationTimeMicros;
    #pragma pack(push, 1) // exact fit - no padding
    typedef struct spectrometerSetIntegrationTimeMicrosPar{
        unsigned ID;
        unsigned int featureID;
        unsigned long long int integration_time;
    }spectrometerSetIntegrationTimeMicrosPar;
    #pragma pack(pop)
    spectrometerSetIntegrationTimeMicrosPar par;
    //Local instanciation

    if(!IsRemote()){
        //return SeaBreezeModuleInstance->spectrometerSetIntegrationTimeMicros(ID,featureID,err,integration_time);
        return sbapi_spectrometer_set_integration_time_micros(ID,featureID,err,integration_time);
    }else{
        if(issource){
            SpectrometerModule::ReceiveParameters((void*)&par,sizeof(spectrometerSetIntegrationTimeMicrosPar));
            //ret = SeaBreezeModuleInstance->spectrometerSetIntegrationTimeMicros(par.ID,par.featureID,err,par.integration_time);

            sbapi_spectrometer_set_integration_time_micros(par.ID,par.featureID,err,par.integration_time);

            ftdi->SendData((unsigned char*)err,sizeof(int));
            if((*err)==0){
                //ftdi->SendData((unsigned char*) , );
            }
            seabreeze::System::sleepMilliseconds(50);
        }else{
            ftdi->SendData((unsigned char*)&func_code,sizeof(int));
            //Assign Par
            par.ID=static_cast<unsigned int>(ID);
            par.featureID=static_cast<unsigned int>(featureID);
            par.integration_time=integration_time;
            SpectrometerModule::SendParameters((void*)&par,sizeof(spectrometerSetIntegrationTimeMicrosPar));
            int bytes;
            while( (bytes = ftdi->ReceiveData((unsigned char*)err,sizeof(int))) == 0){}
            if((*err)==0){
                //while( (bytes = ftdi->ReceiveData((unsigned char*) , )) == 0){}
            }
            seabreeze::System::sleepMilliseconds(100);
        }
    }



}

void SpectrometerModule::spectrometerSetTriggerMode(long ID, long featureID, int* err, int mode){
    static int func_code = fn_spectrometerSetTriggerMode;
    #pragma pack(push, 1) // exact fit - no padding
    typedef struct spectrometerSetTriggerModePar{
        unsigned int ID;
        unsigned int featureID;
        int mode;
    }spectrometerSetTriggerModePar;
    #pragma pack(pop)
    spectrometerSetTriggerModePar par;
    //Local instanciation

    if(!IsRemote()){
        //SeaBreezeModuleInstance->spectrometerSetTriggerMode(ID,featureID, err,mode);
        sbapi_spectrometer_set_trigger_mode(ID,featureID, err,mode);
    }else{
        if(issource){
            SpectrometerModule::ReceiveParameters((void*)&par,sizeof(spectrometerSetTriggerModePar));

            //SeaBreezeModuleInstance->spectrometerSetTriggerMode(par.ID,par.featureID, err,par.mode);
            sbapi_spectrometer_set_trigger_mode(par.ID,par.featureID, err,par.mode);

            ftdi->SendData((unsigned char*)err,sizeof(int));
            if((*err)==0){
                //ftdi->SendData((unsigned char*) , );
            }
        }else{
            ftdi->SendData((unsigned char*)&func_code,sizeof(int));
            //Assign Par
            par.ID = static_cast<unsigned int>(ID);
            par.featureID = static_cast<unsigned int>(featureID);
            par.mode = mode;
            SpectrometerModule::SendParameters((void*)&par,sizeof(spectrometerSetTriggerModePar));
            int bytes;
            while( (bytes = ftdi->ReceiveData((unsigned char*)err,sizeof(int))) == 0){}
            if((*err)==0){
                //while( (bytes = ftdi->ReceiveData((unsigned char*) , )) == 0){}

            }
        }
    }

}

double SpectrometerModule::spectrometerGetMaximumIntensity(long ID,long featureID, int* err){
    static int func_code = fn_spectrometerGetMaximumIntensity;
    double ret=0;

    #pragma pack(push, 1) // exact fit - no padding
    typedef struct spectrometerGetMaximumIntensityPar{
        unsigned int ID;
        unsigned int featureID;
    }spectrometerGetMaximumIntensityPar;
    #pragma pack(pop)
    spectrometerGetMaximumIntensityPar par;
    //Local instanciation

    if(!IsRemote()){
        //return SeaBreezeModuleInstance->spectrometerGetMaximumIntensity(ID,featureID,err);
        return sbapi_spectrometer_get_maximum_intensity(ID,featureID,err);
    }else{
        if(issource){
            SpectrometerModule::ReceiveParameters((void*)&par,sizeof(spectrometerGetMaximumIntensityPar));

            //ret = SeaBreezeModuleInstance->spectrometerGetMaximumIntensity(par.ID,par.featureID,err);
            ret = sbapi_spectrometer_get_maximum_intensity(par.ID,par.featureID,err);

            ftdi->SendData((unsigned char*)err,sizeof(int));
            if((*err)==0){
                ftdi->SendData((unsigned char*)&ret ,sizeof(double) );
            }

        }else{
            ftdi->SendData((unsigned char*)&func_code,sizeof(int));
            //Assign Par
            par.ID = static_cast<unsigned int>(ID);
            par.featureID = static_cast<unsigned int>(featureID);
            SpectrometerModule::SendParameters((void*)&par,sizeof(spectrometerGetMaximumIntensityPar));
            int bytes;
            while( (bytes = ftdi->ReceiveData((unsigned char*)err,sizeof(int))) == 0){}
            if((*err)==0){
                while( (bytes = ftdi->ReceiveData((unsigned char*)&ret,sizeof(double))) == 0){}
            }
        }
        return ret;
    }
}

int SpectrometerModule::spectrometerGetFormattedSpectrumLength(long ID, long featuresID,int *err){
    static int func_code = fn_spectrometerGetFormattedSpectrumLength;
    int ret=0;
    #pragma pack(push, 1) // exact fit - no padding
    typedef struct spectrometerGetFormattedSpectrumLengthPar{
        unsigned int ID;
        unsigned int featuresID;
    }spectrometerGetFormattedSpectrumLengthPar;
    #pragma pack(pop)
    spectrometerGetFormattedSpectrumLengthPar par;
    //Local instanciation

    if(!IsRemote()){
        //return SeaBreezeModuleInstance->spectrometerGetFormattedSpectrumLength(ID, featuresID,err);
        return sbapi_spectrometer_get_formatted_spectrum_length(ID, featuresID,err);
    }else{
        if(issource){
            SpectrometerModule::ReceiveParameters((void*)&par,sizeof(spectrometerGetFormattedSpectrumLengthPar));

            //ret = SeaBreezeModuleInstance->spectrometerGetFormattedSpectrumLength(par.ID, par.featuresID,err);
            ret = sbapi_spectrometer_get_formatted_spectrum_length(par.ID, par.featuresID,err);
            ftdi->SendData((unsigned char*)err,sizeof(int));
            if((*err)==0){
                ftdi->SendData((unsigned char*)&ret ,sizeof(int));
            }
        }else{
            ftdi->SendData((unsigned char*)&func_code,sizeof(int));
            //Assign Par
            par.ID = static_cast<unsigned int>(ID);
            par.featuresID = static_cast<unsigned int>(featuresID);

            SpectrometerModule::SendParameters((void*)&par,sizeof(spectrometerGetFormattedSpectrumLengthPar));
            int bytes;
            while( (bytes = ftdi->ReceiveData((unsigned char*)err,sizeof(int))) == 0){}
            if((*err)==0){
                while( (bytes = ftdi->ReceiveData((unsigned char*)&ret ,sizeof(int))) == 0){}
            }
        }
        return ret;
    }
}

int throughput = 1024;

int SpectrometerModule::spectrometerGetWavelengths(long ID,long featureID, int* err,double * wavelengths,int len){
    static int func_code = fn_spectrometerGetWavelengths;
    int ret=0;
    int bytes;

    #pragma pack(push, 1) // exact fit - no padding
    typedef struct spectrometerGetWavelengthsPar{
        unsigned int ID;
        unsigned int featureID;
        int len;
    }spectrometerGetWavelengthsPar;
    #pragma pack(pop)
    spectrometerGetWavelengthsPar par;
    //Local instanciation


    if(!IsRemote()){
        //return SeaBreezeModuleInstance->spectrometerGetWavelengths(ID,featureID,err,wavelengths,len);
        return sbapi_spectrometer_get_wavelengths(ID,featureID,err,wavelengths,len);
    }else{
        if(issource){
            SpectrometerModule::ReceiveParameters((void*)&par,sizeof(spectrometerGetWavelengthsPar));
            if(wavelengths_len==0){
                wavelengths_api = (double*)malloc(sizeof(double)*par.len);
                wavelengths_comm =  (float*)malloc(sizeof(float)*par.len);

                spectrum_api = (double*)malloc(sizeof(double)*par.len);
                //spectrum_comm =  (float*)malloc(sizeof(float)*par.len);
                spectrum_comm2 =  (short*)malloc(sizeof(short)*par.len);

                wavelengths_len = par.len;
            }else{
                if(wavelengths_len != par.len){
                    wavelengths_api = (double*)realloc(wavelengths_api,sizeof(double)*par.len);
                    wavelengths_comm =  (float*)realloc(wavelengths_comm,sizeof(float)*par.len);

                    spectrum_api = (double*)realloc(spectrum_api,sizeof(double)*par.len);
                    //spectrum_comm =  (float*)realloc(spectrum_comm,sizeof(float)*par.len);
                    spectrum_comm2 =  (short*)realloc(spectrum_comm2,sizeof(short)*par.len);

                    wavelengths_len = par.len;
                }
            }
            //ret = SeaBreezeModuleInstance->spectrometerGetWavelengths(par.ID,par.featureID,err,wavelengths_api,par.len);
            ret = sbapi_spectrometer_get_wavelengths(par.ID,par.featureID,err,wavelengths_api,par.len);
            ftdi->SendData((unsigned char*)err,sizeof(int));
            for(int i=0;i<par.len;i++){
                wavelengths_comm[i] = static_cast<float>(wavelengths_api[i]);
            }
            if((*err)==0){
                //bytes = ftdi->SendData((unsigned char*)wavelengths_comm,par.len*sizeof(float)/2);
                //seabreeze::System::sleepMilliseconds(5);
                //bytes = ftdi->SendData(((unsigned char*)wavelengths_comm)+(par.len*sizeof(float)/2),par.len*sizeof(float)/2);
                //printf("wavelenlen:%d, bytes writen:%d",par.len,bytes);

                for(int i=0;i<par.len;i=i+throughput){
                    bytes = ftdi->SendData((unsigned char*)&wavelengths_comm[i],sizeof(float)*throughput);
                    //seabreeze::System::sleepMilliseconds(2);

                    if(bytes!=(sizeof(float)*throughput)){
                        printf("WriteDiff: %d, of %d\n",bytes,sizeof(float)*throughput);
                    }
                }
            }
        }else{
            printf("func_code request:%d",func_code);
            fflush(stdout);
            //ftdi_usb_purge_rx_buffer(ftdi);
            ftdi->SendData((unsigned char*)&func_code,sizeof(int));
            //Assign Par
            par.ID = static_cast<unsigned int>(ID);
            par.featureID = static_cast<unsigned int>(featureID);
            par.len = len;

            if(wavelengths_len==0){
                wavelengths_comm =  (float*)malloc(sizeof(float)*par.len);
                //spectrum_comm =  (float*)malloc(sizeof(float)*par.len);
                spectrum_comm2 =  (short*)malloc(sizeof(short)*par.len);
                wavelengths_len = par.len;
            }else{
                if(wavelengths_len != par.len){
                    wavelengths_comm =  (float*)realloc(wavelengths_comm,sizeof(float)*par.len);
                    //spectrum_comm =  (float*)realloc(spectrum_comm,sizeof(float)*par.len);

                    spectrum_comm2 =  (short*)realloc(spectrum_comm2,sizeof(short)*par.len);
                    wavelengths_len = par.len;
                }
            }
            SpectrometerModule::SendParameters((void*)&par,sizeof(spectrometerGetWavelengthsPar));

            while( (bytes = ftdi->ReceiveData((unsigned char*)err,sizeof(int))) == 0){}
            if((*err)==0){
                /*while( (bytes = ftdi->ReceiveData((unsigned char*)wavelengths_comm, par.len*sizeof(float)/2)) == 0){}
                ret = bytes;
                while( (bytes = ftdi->ReceiveData(((unsigned char*)wavelengths_comm)+par.len*sizeof(float)/2, par.len*sizeof(float)/2)) == 0){}
                ret += bytes;
                printf("wavelenlen:%d, bytes read:%d",par.len,bytes);
                */
                int totalbytes =0;
                int k=0;
                for(int i=0;i<par.len;i=i+throughput){
                    while( (bytes = ftdi->ReceiveData((unsigned char*)&wavelengths_comm[i], sizeof(float)*throughput)) == 0){}
                    if(bytes!=sizeof(float)*throughput){
                        printf("ReadDiff: %d, of %d\n",bytes,sizeof(float)*throughput);
                    }
                    totalbytes+=bytes;
                    k++;
                }
                printf("Total bytes wavelengths:%d, wavelengths:%d, k:%d",totalbytes,totalbytes/4,k);
                /*for(int i=0;i<totalbytes/4;i++){
                    printf("[%f]\t",wavelengths_comm[i]);
                }*/

                for(int i=0;i<par.len;i++){
                    wavelengths[i] = static_cast<double>(wavelengths_comm[i]);
                }
            }
        }
        return ret;
    }
}

int SpectrometerModule::spectrometerGetFormattedSpectrum(long ID, long featureID,int* err,double *spectrum, int len){
    static int func_code = fn_spectrometerGetFormattedSpectrum;
    int ret=0;

    #pragma pack(push, 1) // exact fit - no padding
    typedef struct spectrometerGetFormattedSpectrumPar{
        unsigned int ID;
        unsigned int featureID;
        int len;
    }spectrometerGetFormattedSpectrumPar;
    #pragma pack(pop)
    spectrometerGetFormattedSpectrumPar par;
    //Local instanciation

    if(!IsRemote()){
        //return SeaBreezeModuleInstance->spectrometerGetFormattedSpectrum(ID, featureID,err,spectrum,len);
        return sbapi_spectrometer_get_formatted_spectrum(ID, featureID,err,spectrum,len);
    }else{
        if(issource){
            SpectrometerModule::ReceiveParameters((void*)&par,sizeof(spectrometerGetFormattedSpectrumPar));
            //ret = SeaBreezeModuleInstance->spectrometerGetFormattedSpectrum(par.ID, par.featureID, err, spectrum_api, par.len);
            ret = sbapi_spectrometer_get_formatted_spectrum(par.ID, par.featureID, err, spectrum_api, par.len);
            ftdi->SendData((unsigned char*)err,sizeof(int));
            if((*err)==0){
                /*
                for(int i=0;i<par.len;i++){
                    spectrum_comm[i] = static_cast<float>(spectrum_api[i]);
                }*/
                DataManipulation::DataCommunicationConvertion_8_to_2(spectrum_comm2,spectrum_api,par.len);
                ftdi->SendData((unsigned char*)spectrum_comm2, par.len*sizeof(short));
                /*ftdi->SendData((unsigned char*)spectrum_comm, par.len*sizeof(float)/2 );
                seabreeze::System::sleepMilliseconds(10);
                ftdi->SendData(((unsigned char*)spectrum_comm)+par.len*sizeof(double)/2, par.len*sizeof(float)/2 );
                */

                /*
                for(int i=0;i<par.len;i=i+throughput){
                    ftdi->SendData((unsigned char*)&spectrum_comm[i], sizeof(float)*throughput);
                    //seabreeze::System::sleepMilliseconds(5);
                }*/
            }
        }else{
            printf("func_code request:%d",func_code);
            fflush(stdout);
            //ftdi_usb_purge_rx_buffer(ftdi);
            ftdi->SendData((unsigned char*)&func_code,sizeof(int));
            //Assign Par
            par.ID = static_cast<unsigned int>(ID);
            par.featureID = static_cast<unsigned int>(featureID);
            par.len = len;
            SpectrometerModule::SendParameters((void*)&par,sizeof(spectrometerGetFormattedSpectrumPar));
            int bytes;
            while( (bytes = ftdi->ReceiveData((unsigned char*)err,sizeof(int))) == 0){}
            if((*err)==0){
                /*while( (bytes = ftdi->ReceiveData((unsigned char*)spectrum_comm , par.len*sizeof(float)/2)) == 0){}
                ret = bytes;
                while( (bytes = ftdi->ReceiveData(((unsigned char*)spectrum_comm)+par.len*sizeof(float)/2 , par.len*sizeof(double)/2)) == 0){}
                ret += bytes;
                */
                //bytes = ftdi->ReceiveData((unsigned char*)spectrum_comm , par.len*sizeof(float));
                bytes = ftdi->ReceiveData((unsigned char*)spectrum_comm2 , par.len*sizeof(short));
                /*int totalbytes=0;
                for(int i=0;i<par.len;i=i+throughput){
                    while( (bytes = ftdi->ReceiveData((unsigned char*)&spectrum_comm[i] , sizeof(float)*throughput)) == 0){}
                    totalbytes+=bytes;
                }
                */
                int totalbytes = bytes;
                printf("Total bytes spectrum:%d, spectrums:%d",totalbytes,totalbytes/4);
                /*for(int i=0;i<par.len;i++){
                    spectrum[i] = static_cast<double>(spectrum_comm[i]);
                }*/
                DataManipulation::DataCommunicationConvertion_2_to_8(spectrum,spectrum_comm2,par.len);
            }
        }
        return ret;
    }
}


int SpectrometerModule::spectrometerGetFormattedSpectrumRange(long ID, long featureID,int* err,double *spectrum, int len, int a, int b){
    static int func_code = fn_spectrometerGetFormattedSpectrumRange;
    int ret=0;

    #pragma pack(push, 1) // exact fit - no padding
    typedef struct spectrometerGetFormattedSpectrumRangePar{
        unsigned int ID;
        unsigned int featureID;
        int len;
        int a;
        int b;
    }spectrometerGetFormattedSpectrumRangePar;
    #pragma pack(pop)
    spectrometerGetFormattedSpectrumRangePar par;
    //Local instanciation

    if(!IsRemote()){
        //return SeaBreezeModuleInstance->spectrometerGetFormattedSpectrum(ID, featureID,err,spectrum,len);
        ret  = sbapi_spectrometer_get_formatted_spectrum(ID, featureID,err,spectrum,len);
        for(int i=a;i<b;i++){
            spectrum[i-a]=spectrum[i];
        }
        return ret;
    }else{
        if(issource){
            SpectrometerModule::ReceiveParameters((void*)&par,sizeof(spectrometerGetFormattedSpectrumRangePar));
            //ret = SeaBreezeModuleInstance->spectrometerGetFormattedSpectrum(par.ID, par.featureID, err, spectrum_api, par.len);
            ret = sbapi_spectrometer_get_formatted_spectrum(par.ID, par.featureID, err, spectrum_api, par.len);
            ftdi->SendData((unsigned char*)err,sizeof(int));
            if((*err)==0){
                /*
                for(int i=0;i<par.len;i++){
                    spectrum_comm[i] = static_cast<float>(spectrum_api[i]);
                }*/
                for(int i=par.a;i<par.b;i++){
                    spectrum_api[i-par.a]=spectrum_api[i];
                }
                DataManipulation::DataCommunicationConvertion_8_to_2(spectrum_comm2,spectrum_api,par.b-par.a);
                ftdi->SendData((unsigned char*)spectrum_comm2, (par.b-par.a)*sizeof(short));
                /*ftdi->SendData((unsigned char*)spectrum_comm, par.len*sizeof(float)/2 );
                seabreeze::System::sleepMilliseconds(10);
                ftdi->SendData(((unsigned char*)spectrum_comm)+par.len*sizeof(double)/2, par.len*sizeof(float)/2 );
                */

                /*
                for(int i=0;i<par.len;i=i+throughput){
                    ftdi->SendData((unsigned char*)&spectrum_comm[i], sizeof(float)*throughput);
                    //seabreeze::System::sleepMilliseconds(5);
                }*/
            }
        }else{
            printf("func_code request:%d",func_code);
            fflush(stdout);
            //ftdi_usb_purge_rx_buffer(ftdi);
            ftdi->SendData((unsigned char*)&func_code,sizeof(int));
            //Assign Par
            par.ID = static_cast<unsigned int>(ID);
            par.featureID = static_cast<unsigned int>(featureID);
            par.len = len;
            par.a = a;
            par.b = b;
            SpectrometerModule::SendParameters((void*)&par,sizeof(spectrometerGetFormattedSpectrumRangePar));
            int bytes;
            while( (bytes = ftdi->ReceiveData((unsigned char*)err,sizeof(int))) == 0){}
            if((*err)==0){
                /*while( (bytes = ftdi->ReceiveData((unsigned char*)spectrum_comm , par.len*sizeof(float)/2)) == 0){}
                ret = bytes;
                while( (bytes = ftdi->ReceiveData(((unsigned char*)spectrum_comm)+par.len*sizeof(float)/2 , par.len*sizeof(double)/2)) == 0){}
                ret += bytes;
                */
                //bytes = ftdi->ReceiveData((unsigned char*)spectrum_comm , par.len*sizeof(float));
                bytes = ftdi->ReceiveData((unsigned char*)spectrum_comm2 , (par.b-par.a)*sizeof(short));
                /*int totalbytes=0;
                for(int i=0;i<par.len;i=i+throughput){
                    while( (bytes = ftdi->ReceiveData((unsigned char*)&spectrum_comm[i] , sizeof(float)*throughput)) == 0){}
                    totalbytes+=bytes;
                }
                */
                int totalbytes = bytes;
                printf("Total bytes spectrum:%d, spectrums:%d",totalbytes,totalbytes/4);
                /*for(int i=0;i<par.len;i++){
                    spectrum[i] = static_cast<double>(spectrum_comm[i]);
                }*/
                DataManipulation::DataCommunicationConvertion_2_to_8(spectrum,spectrum_comm2,(par.b-par.a));
            }
        }
        return ret;
    }
}


}
