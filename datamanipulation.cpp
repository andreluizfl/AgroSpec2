#include "datamanipulation.h"
#include <cmath>
DataManipulation::DataManipulation()
{

}


void DataManipulation::DataCommunicationConvertion_8_to_4(float* data4, double* data8, int size){

}

void DataManipulation::DataCommunicationConvertion_8_to_3(data3bytes* data3, double* data8, int size){

}

void DataManipulation::DataCommunicationConvertion_8_to_2(short* data2, double* data8, int size){
    for(int i=0;i<size;i++){
        data2[i] = static_cast<short>(round(data8[i]));
    }
}

void DataManipulation::DataCommunicationConvertion_4_to_8(double* data8, float* data4, int size){

}

void DataManipulation::DataCommunicationConvertion_3_to_8(double* data8, data3bytes* data3, int size){

}

void DataManipulation::DataCommunicationConvertion_2_to_8(double* data8 , short* data2, int size){
    for(int i=0;i<size;i++){
        data8[i] = static_cast<double>(data2[i]);
    }

}
