#include "testwindow.h"
#include "ui_testwindow.h"
#include "module/spectrometermodule.h"

#include "gui/groupboxspec.h"

TestWindow::TestWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TestWindow)
{
    ui->setupUi(this);
    ui->Close->hide();
    ui->Open->hide();
    ui->Probe->hide();
    ui->plainTextEdit->hide();
}

TestWindow::~TestWindow()
{
    delete ui;
}

void TestWindow::on_pushButton_clicked()
{
    Agrospec::SpectrometerModule::DetectSpectrometers();
    this->ui->plainTextEdit->appendPlainText(QString::number(Agrospec::SpectrometerModule::NumberOfDevices));
    this->ui->NumDevs->setText(QString::number(Agrospec::SpectrometerModule::NumberOfDevices));

    /*if(Agrospec::SpectrometerModule::NumberOfDevices>0){
        this->ui->DevID->setText(QString::number(Agrospec::SpectrometerModule::DeviceIDs[0]));
    }*/

    if(Agrospec::SpectrometerModule::Spectrometers.size()>0){
        this->ui->DevID->setText(QString::number(Agrospec::SpectrometerModule::DeviceIDs[0]));
        this->ui->Spec1->SetSpectrometer(Agrospec::SpectrometerModule::Spectrometers[0]);
        if(Agrospec::SpectrometerModule::Spectrometers.size()>1){
            //this->ui->Spec2->SetSpectrometer(Agrospec::SpectrometerModule::Spectrometers[1]);
        }
    }

}

SeaBreezeAPI* api = nullptr;
long myID;

void TestWindow::on_Probe_clicked()
{
    api = SeaBreezeAPI::getInstance();
    this->ui->plainTextEdit->appendPlainText(QString::number(api->probeDevices()));
}

void TestWindow::on_Open_clicked()
{
    int err=0;
    api->getDeviceIDs(&myID,1);
    api->openDevice(myID,&err);
    this->ui->plainTextEdit->appendPlainText(QString::number(err));

}

void TestWindow::on_Close_clicked()
{
    int err=0;
    api->closeDevice(myID,&err);
    this->ui->plainTextEdit->appendPlainText(QString::number(err));

}

void TestWindow::on_pushReleaseAll_clicked()
{
    SpectrometerModule::ReleaseSpectrometers();
}

void TestWindow::on_checkRemote_clicked(bool checked)
{
    Agrospec::SpectrometerModule::EnableRemote(checked);
}

void TestWindow::on_checkSource_clicked(bool checked)
{
    Agrospec::SpectrometerModule::EnableSource(checked);
}
