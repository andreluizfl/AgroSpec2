#include "listener.h"

namespace Agrospec {

Listener::Listener(FTDIcomm* myftdi)
{
    ftdi = myftdi;
    /*ftdi_setflowctrl(ftdi, SIO_RTS_CTS_HS);
    ftdi_setdtr(ftdi,1);
    ftdi_setrts(ftdi,1);*/
}

Listener::~Listener(){

}

void Listener::run(){
    /*ftdi_setflowctrl(ftdi, SIO_RTS_CTS_HS);
    ftdi_setdtr(ftdi,1);
    ftdi_setrts(ftdi,1);

    ftdi_usb_purge_buffers(ftdi);*/
    SpectrometerModule::Listening();

    /*int bytes;
    int func_cod,err;
    while(true){
       bytes = 0;
       func_cod = 0;
       while((bytes = ftdi_read_data( ftdi, (unsigned char*)&func_cod, sizeof(int))) == 0){
       }
       printf("func_cod_request:%d\n",func_cod);
       fflush(stdout);
       switch(func_cod){
        case 1:
          int num_devs = Agrospec::SpectrometerModule::probeDevices();
          break;

        case 2:
            if(NumberOfDevices>0){
                DeviceIDs = new long[NumberOfDevices];
                err = Agrospec::SpectrometerModule::getDeviceIDs(DeviceIDs,NumberOfDevices);
            }
            break;


       }
    }*/
}

}
