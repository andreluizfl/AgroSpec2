#ifndef ICOMMUNICATION_H
#define ICOMMUNICATION_H

class ICommunication
{
public:
    ICommunication(){}
    virtual unsigned int InitializeCommunication(void* data)=0;
    virtual unsigned int ConfigureCommunication(void* data)=0;
    virtual unsigned int ReleaseCommunication()=0;

    virtual unsigned int SendData(void* buffer, int size)=0;
    virtual unsigned int ReceiveData(void* buffer, int size)=0;

    virtual unsigned int ClearInBuffer()=0;
    virtual unsigned int ClearOutBuffer()=0;
    
    virtual void SetInBufferSize(int size)=0;
    virtual void SetOutBufferSize(int size)=0;
    
    virtual unsigned int GetInBufferSize()=0;
    virtual unsigned int GetOutBufferSize()=0;

    //virtual unsigned int Listening(unsigned int period)=0;
};

#endif // ICOMMUNICATION_H
