#include <QApplication>

#include "gui/testwindow.h"
#include "module/modulemanager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "communication/ftdicomm.h"
//#include "ftdi.h"
//typedef ftdi_context FTDI;
//typedef ftdi_device_list FTDI_devlist;

#include "api/seabreezeapi/SeaBreezeAPI.h"
#include "api/SeaBreezeWrapper.h"

#include <unistd.h>
#ifdef __WIN32__
#include <windows.h>
#endif


int main(int argc, char *argv[])
{
/*
const int total= 2048*3;
char buffer[total];

#define __WIN32__
#ifdef __WIN32__
    FTDIcomm* ftdi = new FTDIcomm();
    printf("Init:%d\n",ftdi->InitializeCommunication(0));
    ftdi->ConfigureCommunication(0);
    while(true){
        printf("Press any key to send!");
        getchar();
        for(int i=0; i<total;i++){
            buffer[i]=(char)(i%128);
        }
        //ftdi->SendData((void*)&buffer,total);
        ftdi->Write((void*)&buffer,total);
        //ftdi->SendData2((void*)&buffer,total);
    }
    ftdi->ReleaseCommunication();
    delete ftdi;
    return 0;
#else
    int bytes;
    FTDIcomm* ftdi = new FTDIcomm();
    ftdi->InitializeCommunication(0);
    ftdi->ConfigureCommunication(1);
    while(true){
        //ftdi->ReceiveData((void*)&buffer,total);
        bytes = ftdi->Read((void*)&buffer,total,10000);
        printf("bytes:%d, timeout:%d op_elapsed:%d\n",bytes,ftdi->last_op_has_timeout,ftdi->last_op_elapsedtime);
        if(ftdi->last_op_has_timeout)
            break;
        for(int i=0; i<total;i++){

            if(buffer[i]!= (i%128)){
                printf("data error");
            }
            //printf("%d ",(int)buffer[i]);
        }
    }
    ftdi->ReleaseCommunication();
    delete ftdi;
    return 0;

#endif
*/
/*
#define __WIN32__
#ifdef __WIN32__
    int ret, i,opt=0;
    FTDI *ftdi;
    FTDI_devlist * devlist,*curdev;
    int retval = EXIT_SUCCESS;
    char data[32];
    int return_code;
    int errcode = 0;

    if ((ftdi = ftdi_new()) == 0)
    {
        fprintf(stderr, "ftdi_new failed\n");
        exit(EXIT_FAILURE);
    }

    if ((ret = ftdi_usb_find_all(ftdi, &devlist, 0, 0)) < 0)
    {
        fprintf(stderr, "ftdi_usb_find_all failed: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        ftdi_free(ftdi);
        exit(EXIT_FAILURE);
    }

    while(opt>=0){
        printf("Number of FTDI devices found: %d\nSelect one device:", ret);
        scanf("%d",&opt);
        if(opt>=0 && opt<ret){
            curdev = devlist;
            while(curdev != NULL && opt>0){
                curdev = curdev->next;
                opt--;
            }
            if(curdev!=NULL){
                int rc = ftdi_usb_open_dev(ftdi, curdev->dev);
                printf("rc=%d\n", rc);
                ftdi_set_baudrate(ftdi, 230400);
                ftdi_set_line_property(ftdi, 8, STOP_BIT_1, NONE);
                //ftdi_set_bitmode(ftdi, 0xFF, BITMODE_BITBANG);
                strcpy(data,"Hello Test World!");
                int msg_num=0;
                while(strcmp(data,"0")!=0){
                    //printf("Enter msg:");
                    //scanf("%s",data);

                    rc = ftdi_write_data(ftdi, (unsigned char*)data, strlen(data));
                    //rc = serial_sendPacket( ftdi, 1, (unsigned char*)data, strlen(data), errcode);
                    //printf("rc=%d\n", rc);
                    printf("MSG_NUM:%d\n",++msg_num);
                    sleep(2);
                }
                opt = -1;
                ftdi_usb_close(ftdi);
            }
        }
    }
    ftdi_list_free(&devlist);
    ftdi_free(ftdi);
    return 0;
#else
    int ret, i,opt=0,bytes;
        FTDI *ftdi;
        FTDI_devlist * devlist,*curdev;
        int retval = EXIT_SUCCESS;
        unsigned char data[32];

        unsigned char packet_type;
        unsigned char *payload = NULL;
        int payload_buf_size = 0;
        int payload_size;
        int return_code;


        if ((ftdi = ftdi_new()) == 0)
        {
            fprintf(stderr, "ftdi_new failed\n");
            exit(EXIT_FAILURE);
        }

        if ((ret = ftdi_usb_find_all(ftdi, &devlist, 0, 0)) < 0)
        {
            fprintf(stderr, "ftdi_usb_find_all failed: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
            ftdi_free(ftdi);
            exit(EXIT_FAILURE);
        }

        curdev = devlist;

        if(curdev!=NULL){
            int rc = ftdi_usb_open_dev(ftdi, curdev->dev);
            ftdi_set_baudrate(ftdi, 230400);
            ftdi_set_line_property(ftdi, 8 , STOP_BIT_1, NONE);
            ftdi_setflowctrl(ftdi, SIO_RTS_CTS_HS);
            ftdi_setdtr(ftdi,1);
            ftdi_setrts(ftdi,1);
            ftdi_usb_purge_buffers(ftdi);
            int msg_num=0;
            while(true){
                printf("Listening...\n");
                memset(data,0,32);
                while( (bytes = ftdi_read_data( ftdi, data, 32 )) == 0){
                }
                msg_num++;
                printf("[%d] Bytes read: %d\tData received:%s",msg_num,bytes,data);
            }
        }
    return 0;
#endif
*/
    int res;
    Agrospec::ModuleManager::InitializeGUIModules();
    QApplication a(argc, argv);
    //AgrospecWindow w;
    TestWindow w;
    w.show();
    res = a.exec();
    Agrospec::ModuleManager::ReleaseGUIModules();
    return res;
}
