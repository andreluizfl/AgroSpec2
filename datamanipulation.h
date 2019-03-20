#ifndef DATAMANIPULATION_H
#define DATAMANIPULATION_H


class DataManipulation
{
public:
    #pragma pack(push, 1)
    typedef struct data3bytes{
        short integer;
        char decimal;
    }data3bytes;
    #pragma pack(pop)

    DataManipulation();
    static void DataCommunicationConvertion_8_to_4(float* data4, double* data8, int size);
    static void DataCommunicationConvertion_8_to_3(data3bytes* data3, double* data8, int size);
    static void DataCommunicationConvertion_8_to_2(short* data2, double* data8, int size);

    static void DataCommunicationConvertion_4_to_8(double* data8, float* data4, int size);
    static void DataCommunicationConvertion_3_to_8(double* data8, data3bytes* data3, int size);
    static void DataCommunicationConvertion_2_to_8(double* data8 , short* data2, int size);
};

#endif // DATAMANIPULATION_H
