#ifndef FTDICOMM_H
#define FTDICOMM_H
#include <QElapsedTimer>
#include "icommunication.h"
#include "ftdi.h"

#define DEFAULT_INBUFFER_SIZE 4*4096
#define DEFAULT_OUTBUFFER_SIZE 4*4096
#define DEFUALT_BAUDRATE 230400
#include <limits>
#define QINT64_INF std::numeric_limits < qint64 >::max()

class FTDIcomm : public ICommunication
{
private:
    typedef struct ftdi_context FTDI;
    typedef struct ftdi_device_list FTDI_devlist;
    FTDI *ftdi;
    FTDI_devlist * devlist;
    char* inbuffer;
    char* outbuffer;
    unsigned int inbuffer_size;
    unsigned int outbuffer_size;
    QElapsedTimer elapsed;
    QElapsedTimer last_op_elapsed;

    int datablock;
    qint64 timeout;
    int err;
    bool is_usb_openned;

public:
    unsigned int SendData2(void* buffer, int size);
    unsigned int ReceiveData2(void* buffer, int size);
    qint64 last_op_elapsedtime;
    bool last_op_has_timeout;
    unsigned int Write(void* buffer, int size);
    unsigned int Read(void* buffer, int size, qint64 timeout);

    FTDIcomm();
    ~FTDIcomm();

    unsigned int InitializeCommunication(void* data);
    unsigned int ConfigureCommunication(void* data);
    unsigned int ReleaseCommunication();

    unsigned int SendData(void* buffer, int size);
    unsigned int ReceiveData(void* buffer, int size);

    unsigned int ClearInBuffer();
    unsigned int ClearOutBuffer();

    void SetInBufferSize(int size);
    void SetOutBufferSize(int size);

    unsigned int GetInBufferSize();
    unsigned int GetOutBufferSize();


    void SetReadTimeout(unsigned long long int);
    unsigned long long int GetReadTimeout();
};

#endif // FTDICOMM_H
