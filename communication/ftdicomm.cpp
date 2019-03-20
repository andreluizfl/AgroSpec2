#include "ftdicomm.h"
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <QThread>

FTDIcomm::FTDIcomm(){
    int ret;
    FTDI_devlist *curdev;
    is_usb_openned = false;
    ftdi = nullptr;
    devlist = nullptr;
    if ((ftdi = ftdi_new()) == 0)
    {
        fprintf(stderr, "ftdi_new failed\n");
        exit(1);
    }
    if ((ret = ftdi_usb_find_all(ftdi, &devlist, 0, 0)) < 0)
    {
        fprintf(stderr, "ftdi_usb_find_all failed: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        ftdi_free(ftdi);
        exit(1);
    }
}

FTDIcomm::~FTDIcomm(){

    if(devlist!=nullptr){
        ftdi_list_free(&devlist);
    }
    if(ftdi!=nullptr){
        ftdi_free(ftdi);
    }
}

unsigned int FTDIcomm::InitializeCommunication(void* data){
    err = ftdi_usb_open_dev(ftdi, devlist->dev);
    err += ftdi_set_baudrate(ftdi, DEFUALT_BAUDRATE);
    err += ftdi_set_line_property(ftdi, BITS_8, STOP_BIT_1, NONE);
    err += ftdi_read_data_set_chunksize(ftdi,DEFAULT_INBUFFER_SIZE);
    err += ftdi_write_data_set_chunksize(ftdi,DEFAULT_OUTBUFFER_SIZE);
    err += ftdi_usb_purge_buffers(ftdi);
    return err;
}

unsigned int FTDIcomm::ConfigureCommunication(void* data){

    /*
     * SIO_DISABLE_FLOW_CTRL
    SIO_RTS_CTS_HS
    SIO_DTR_DSR_HS
    SIO_XON_XOFF_HS
    */
    unsigned int chunk;
    ftdi_read_data_get_chunksize(ftdi,&chunk);
    printf("ChunkSize:%d",(int)chunk);
    double latency_estimated = 1000.0*((double)chunk)/((double)DEFUALT_BAUDRATE);
    unsigned char latency = (unsigned char)floor(latency_estimated);
    latency = 20;
    timeout = 100;
    datablock = 2048;
    ftdi_set_latency_timer(ftdi,latency);
    ftdi_get_latency_timer(ftdi,&latency);
    printf("Latency %d\n",(int)latency);

    SetReadTimeout(QINT64_INF);
    //ftdi_setflowctrl(ftdi, SIO_DISABLE_FLOW_CTRL);
    ftdi_setflowctrl(ftdi, SIO_RTS_CTS_HS);

    ftdi_setdtr(ftdi,1);
    ftdi_setrts(ftdi,1);
    
    //ftdi_setflowctrl(ftdi, SIO_DTR_DSR_HS);
    //ftdi_setflowctrl(ftdi,SIO_XON_XOFF_HS);
    if(data == 0){
        //ftdi_setflowctrl(ftdi, SIO_DISABLE_FLOW_CTRL);

    }else{
        //ftdi_setflowctrl(ftdi, SIO_DISABLE_FLOW_CTRL)
        //ftdi_setflowctrl(ftdi, SIO_RTS_CTS_HS);

    }
    ftdi_usb_purge_buffers(ftdi);
}

unsigned int FTDIcomm::ReleaseCommunication(){
    if(is_usb_openned){
        if(ftdi!=nullptr){
            return ftdi_usb_close(ftdi);
        }
    }
    return -1;
}

unsigned int FTDIcomm::SendData(void* buffer, int size){
    int write=0;
    int bytes;
    int remaining = size;
    elapsed.start();
    while(write!=size){
        while( (bytes = ftdi_write_data(ftdi,(unsigned char*)buffer,remaining)) == 0){}
        write+=bytes;
        remaining-=bytes;
        buffer = ((void*)buffer)+bytes;
        //printf("write:%d\n",write);
    }
    printf("elaped:%d\n",elapsed.elapsed());
    return write;
}

unsigned int FTDIcomm::SendData2(void* buffer, int size){
    int i;
    int write=0,bytes;
    for(i=0;i<size;i=i+4){
        while( (bytes = ftdi_write_data(ftdi,(void*)&(((unsigned char*)buffer)[i]),4)) == 0){}
        //write+=Write((void*)&(((char*)buffer)[i]),1);
    }
    write=size;
    return write;
    /*int write=0;
    int bytes;
    int remaining = size;
    elapsed.start();
    while(write!=size){
        while( (bytes = ftdi_write_data(ftdi,(unsigned char*)buffer,remaining)) == 0){}
        write+=bytes;
        remaining-=bytes;
        buffer = ((void*)buffer)+bytes;
        printf("write:%d\n",write);
    }
    printf("elaped:%d\n",elapsed.elapsed());
    return write;*/
}


unsigned int FTDIcomm::ReceiveData(void* buffer, int size){
    int bytes;
    int read=0;
    int remaining = size;
    bool first_receiving = false;
    elapsed.start();
    QElapsedTimer timeout_counter;
    while(read!=size){
        while( (bytes = ftdi_read_data(ftdi,buffer,remaining)) == 0 ){
            if(!first_receiving){
                elapsed.start();
            }else{
                if(timeout_counter.hasExpired(timeout)){
                    last_op_has_timeout = true;
                    printf("read timeout!!!\n");
                    return read;
                }
            }

        }
        first_receiving = true;
        timeout_counter.start();
        read+=bytes;
        remaining-=bytes;
        buffer = ((void*)buffer)+bytes;
        //printf("read:%d\n",read);
    }
    last_op_elapsedtime = elapsed.elapsed();
    printf("elapsed:%d\n",last_op_elapsedtime);
    return read;
    //return ftdi_read_data( ftdi, (unsigned char*)&buffer, size);
}

unsigned int FTDIcomm::ReceiveData2(void* buffer, int size){
    int bytes;
    int read=0;
    int remaining = size;
    bool first_receiving = false;
    elapsed.start();
    QElapsedTimer timeout_counter;
    while(read!=size){
        while( (bytes = ftdi_read_data(ftdi,buffer,remaining)) == 0 ){
            if(!first_receiving){
                //first_receiving = true;
                elapsed.restart();
            }else{
                if(timeout_counter.hasExpired(timeout)){
                    printf("read timeout");
                    return read;
                }
            }
        }
        first_receiving = true;
        timeout_counter.start();
        read+=bytes;
        remaining-=bytes;
        buffer = ((void*)buffer)+bytes;
        printf("read:%d\n",read);
    }
    printf("elaped:%d\n",elapsed.elapsed());
    return read;
}

unsigned int FTDIcomm::Write(void* buffer, int size){
    int write=0;
    int bytes;
    int remaining = size;
    last_op_elapsed.start();
    while(write!=size){
        while( (bytes = ftdi_write_data(ftdi,(unsigned char*)buffer,remaining)) == 0){}

        //printf("bytes:%d",bytes);
        write+=bytes;
        remaining-=bytes;
        buffer = ((void*)buffer)+bytes;
    }
    last_op_elapsedtime = last_op_elapsed.elapsed();
    return write;
}

unsigned int FTDIcomm::Read(void* buffer, int size, qint64 ms_timeout){
    int bytes;
    int read=0;
    int remaining = size;
    last_op_elapsed.start();
    last_op_has_timeout = false;
    while(read!=size){
        while( (bytes = ftdi_read_data(ftdi,buffer,remaining)) == 0 ){
            if(last_op_elapsed.hasExpired(ms_timeout)){
                last_op_has_timeout = true;
                return read;
            }
        }
        //last_op_elapsed.restart();
        read+=bytes;
        remaining-=bytes;
        buffer = ((void*)buffer)+bytes;
        //printf("bytes:%d read:%d\n",bytes,read);
    }
    last_op_elapsedtime = last_op_elapsed.elapsed();
    return read;
}

unsigned int FTDIcomm::ClearInBuffer(){
    ftdi_usb_purge_rx_buffer(ftdi);
    return 0;
}

unsigned int FTDIcomm::ClearOutBuffer(){
    ftdi_usb_purge_tx_buffer(ftdi);
    return 0;
}

void FTDIcomm::SetInBufferSize(int size){
    ftdi_read_data_set_chunksize(ftdi,size);
}

void FTDIcomm::SetOutBufferSize(int size){
    ftdi_write_data_set_chunksize(ftdi,size);
}

unsigned int FTDIcomm::GetInBufferSize(){
    unsigned int chunk;
    ftdi_read_data_get_chunksize(ftdi,&chunk);
    return chunk;
}

unsigned int FTDIcomm::GetOutBufferSize(){
    unsigned int chunk;
    ftdi_write_data_get_chunksize(ftdi,&chunk);
    return chunk;
}

void FTDIcomm::SetReadTimeout(unsigned long long int tm){
    timeout = tm;
}

unsigned long long int FTDIcomm::GetReadTimeout(){
    return timeout;
}
