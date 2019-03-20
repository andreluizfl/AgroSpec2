#ifndef OCEANSPECTROMETER_H
#define OCEANSPECTROMETER_H

#include "interface/ispectrometer.h"
#include "interface/itest.h"

namespace Agrospec {

class OceanSpectrometer: public ISpectrometer
{
public:
    OceanSpectrometer(int ID);
    ~OceanSpectrometer();


private:
    //Struct types
    typedef struct SerialFeature{
        long ID;
        int serial_length;
        char* serial;
        SerialFeature();
        ~SerialFeature();
    }SerialFeature;

    typedef struct SpectrometerFeature{
        long ID;
        ///Integration time
        unsigned long integration_time_min;
        unsigned long integration_time_max;
        unsigned long integration_time_current;
        ///Intensity
        double intensity_max;
        ///WavelengthxSpectrum
        int resolution;
        double* wavelengths;
        double* spectrum;

        SpectrometerFeature();
        ~SpectrometerFeature();
    }SpectrometerFeature;

    long 	deviceID;
    long 	featureID;
    long    specID;
    long    binID;
    long    sprocID;

    //Serial Number
    int serial_num_features;
    SerialFeature** serial_features;

    //Spectrometer
    int spectrometer_num_features;
    SpectrometerFeature** spectrometer_features;


    //Binning
    //Shutter
    //Lamp
    //Irradiance Calibration
    //Thermoeletric
    //Temperature
    //Spectral Processing
    //Data Buffer
    //Optical bench
    //Stray Light


public:
    const char* GetSerial();

    unsigned long GetIntegrationTimeMin();
    unsigned long GetIntegrationTimeMax();
    unsigned long GetIntegrationTime();
    void SetIntegrationTime(unsigned long it);

    double GetMaxIntensity();

    int GetNumberofWavelenghts();

    double* GetWavelenghts();
    double* GetSpectrum();
    double* GetTemperature();

    bool UpdateAll();
    bool UpdateRanged(int a, int b);
    
    

};

}

#endif // OCEANSPECTROMETER_H
