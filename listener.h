#ifndef _LISTENER_H
#define _LISTENER_H

#include <QThread>
#include "module/spectrometermodule.h"
#include "communication/ftdicomm.h"

namespace Agrospec {

//typedef struct ftdi_context FTDI;
//typedef struct ftdi_device_list FTDI_devlist;

class Listener : public QThread
{
    Q_OBJECT
private:
    FTDIcomm* ftdi;
    int bytes;
    int func_cod;


public:
    Listener(FTDIcomm* myftdi);

    ~Listener();
    void run();
};

}

#endif // LISTENER_H
