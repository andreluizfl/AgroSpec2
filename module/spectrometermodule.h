#ifndef SPECTROMETERMODULE_H
#define SPECTROMETERMODULE_H

#include <vector>
#include "api/seabreezeapi/SeaBreezeAPI.h"
#include "api/SeaBreezeWrapper.h"
#include "interface/ispectrometer.h"
#include "communication/ftdicomm.h"

//typedef struct ftdi_context FTDI;
//typedef struct ftdi_device_list FTDI_devlist;
#define NUMFUNCTIONS 2

//typedef void (*FunctionRef)();

using namespace std;

namespace Agrospec {

class Listener;
class SpectrometerModule
{
//private:
public:
    static Listener* listenerobj;
    static FTDIcomm* ftdi;
    static void* FunctionTable;
    static SeaBreezeAPI* SeaBreezeModuleInstance;
    static unsigned long NumberOfDevices;
    static long* DeviceIDs;
    static vector<ISpectrometer*> Spectrometers;
    static bool isremote;
    static bool issource;

public:

    static void Initialize();
    static void Release();
    static const SeaBreezeAPI* GetSeaBreezeAPIInstance();

    static void EnableRemote(bool);
    static bool IsRemote();
    static void EnableSource(bool);

    static void ReleaseSpectrometers();
    static void DetectSpectrometers();
    SpectrometerModule();
    static void Listening();

    static int probeDevices();
    static int getDeviceIDs(long* deviceIDs,unsigned int numberOfDevices);

    static int openDevice(long ID,int* err);
    static void closeDevice(long ID,int* err);

    static int getNumberOfSerialNumberFeatures(long ID, int* err);
    static int getSerialNumberFeatures(long ID, int* err, long* serial_features_ids, int serial_num_features);
    static unsigned char getSerialNumberMaximumLength(long ID, long featureID, int* err);
    static int getSerialNumber(long ID, long featureID, int* err, char* serial, int serial_length);

    static int getNumberOfSpectrometerFeatures(long ID,int* error);
    static int getSpectrometerFeatures(long ID,int* error,long* spectrometer_features_ids,int spectrometer_num_features);

    static unsigned long long int spectrometerGetMinimumIntegrationTimeMicros(long ID,long featureID,int* error);
    static unsigned long long int spectrometerGetMaximumIntegrationTimeMicros(long ID,long featureID,int* error);

    static void spectrometerSetIntegrationTimeMicros(long ID,long featureID, int *error,unsigned long long int integration_time);
    static void spectrometerSetTriggerMode(long ID, long featureID, int* err, int mode);
    static double spectrometerGetMaximumIntensity(long ID,long featureID, int* error);
    static int spectrometerGetFormattedSpectrumLength(long ID, long featuresID,int *error);
    static int spectrometerGetWavelengths(long ID,long featureID, int* error,double * wavelengths,int len);
    static int spectrometerGetFormattedSpectrum(long ID, long featureID,int* error,double *spectrum, int len);

    static int spectrometerGetFormattedSpectrumRange(long ID, long featureID,int* error,double *spectrum,int len, int a, int b);


private:
    static void SendParameters(void* par,int size);
    static void ReceiveParameters(void* par,int size);

    static double* wavelengths_api;
    static float* wavelengths_comm;
    static int wavelengths_len;
    static double* spectrum_api;
    static float* spectrum_comm;

    static short* spectrum_comm2;


};

}
#endif // SPECTROMETERMODULE_H
