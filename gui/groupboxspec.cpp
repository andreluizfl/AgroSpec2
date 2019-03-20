#include "groupboxspec.h"
#include "ui_groupboxspec.h"
#include <stdio.h>

#include "qcustomplot.h"
#include "gui/dialog.h"

GroupBoxSpec::GroupBoxSpec(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::GroupBoxSpec)
{
    ui->setupUi(this);
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(UpdateGraph()));
    min_period = 100;
    period = 400;
}

GroupBoxSpec::~GroupBoxSpec()
{
    delete ui;
    delete timer;
}

void GroupBoxSpec::SetSpectrometer(ISpectrometer* spec){
    this->spec = spec;
    if(this->spec!=nullptr){
        QObject::connect(this->spec, SIGNAL(destroyed(QObject*)), this,  SLOT(RemoveSpectrometer(QObject*)));

        this->ui->serial_text->setText(QString::fromLatin1(spec->GetSerial()));

        this->ui->labelIt_2->setText(QString::number(spec->GetIntegrationTimeMin()));
        this->ui->labelIt_3->setText(QString::number(spec->GetIntegrationTimeMax()));

        this->ui->horizontalSliderIt->setMinimum((int)spec->GetIntegrationTimeMin());
        this->ui->horizontalSliderIt->setMaximum((int)spec->GetIntegrationTimeMax());
        this->ui->horizontalSliderIt->setValue((int)spec->GetIntegrationTime());

        this->ui->it_text->setText(QString::number(spec->GetIntegrationTime()));

        a=0;
        b = spec->GetNumberofWavelenghts();
        isranged = false;

        this->ui->lineEditA->setText(QString::number(a));
        this->ui->lineEditB->setText(QString::number(b));

        if(spec->GetNumberofWavelenghts()>0){
        QVector<double> x(spec->GetNumberofWavelenghts()),y(spec->GetNumberofWavelenghts());
        double* w = spec->GetWavelenghts(),*s = spec->GetSpectrum();
        for (int i=0; i<spec->GetNumberofWavelenghts(); i++)
        {
          x[i] = w[i];
          y[i] = s[i];
        }
        this->ui->widget->clearGraphs();
        this->ui->widget->addGraph();
        this->ui->widget->graph(0)->setData(x, y);
        this->ui->widget->xAxis->setLabel("x");
        this->ui->widget->yAxis->setLabel("y");
        //this->ui->widget->rescaleAxes();
        //this->ui->widget->yAxis->setRangeUpper(5000);

        this->ui->widget->xAxis->setRangeUpper(w[spec->GetNumberofWavelenghts()-1]);
        this->ui->widget->xAxis->setRangeLower(w[0]);

        this->ui->widget->yAxis->setRangeUpper(spec->GetMaxIntensity());
        this->ui->widget->yAxis->setRangeLower(0);
        //this->ui->widget->update();
        this->ui->widget->replot();
        //this->spec->deleting_event_add((DeletingSpectrometer)&this->RemoveSpectrometer);
        }

        this->setEnabled(true);
    }

}

void GroupBoxSpec::RemoveSpectrometer(QObject* obj){
    //ISpectrometer* spec = (ISpectrometer*) obj;
    //this->spec = nullptr;
    if(this!=nullptr){
        this->timer->stop();
        this->setEnabled(false);
    }
}

void GroupBoxSpec::on_pushButtonSetIt_clicked()
{
    int val = this->ui->it_text->text().toInt();
    this->ui->horizontalSliderIt->setValue(val);
    this->ui->horizontalSliderIt->update();
    //spec->SetIntegrationTime(val);
}

void GroupBoxSpec::on_horizontalSliderIt_valueChanged(int value)
{
    spec->SetIntegrationTime(value);
    this->ui->it_text->setText(QString::number(value));
}

void GroupBoxSpec::on_Plot_clicked()
{
    /*spec->Update();
    QVector<double> x(spec->GetNumberofWavelenghts()),y(spec->GetNumberofWavelenghts());
    double* w = spec->GetWavelenghts(),*s = spec->GetSpectrum();
    for (int i=0; i<spec->GetNumberofWavelenghts(); i++)
    {
      x[i] = w[i];
      y[i] = s[i];
    }


    //this->ui->widget->graph(0)->data()->clear();
    this->ui->widget->clearGraphs();
    this->ui->widget->addGraph();
    this->ui->widget->graph(0)->setData(x, y);
    this->ui->widget->xAxis->setLabel("x");
    this->ui->widget->yAxis->setLabel("y");
    //this->ui->widget->rescaleAxes();
    //this->ui->widget->yAxis->setRangeUpper(spec->GetMaxIntensity());

    this->ui->widget->xAxis->setRangeUpper(w[spec->GetNumberofWavelenghts()-1]);
    this->ui->widget->xAxis->setRangeLower(w[0]);

    this->ui->widget->yAxis->setRangeUpper(spec->GetMaxIntensity());
    this->ui->widget->yAxis->setRangeLower(0);


    this->ui->widget->replot();
    return;

   for (int i=0; i<101; ++i)
   {
     x[i] = i;
     y[i] = i;
   }
   this->ui->widget->addGraph();
   this->ui->widget->graph(0)->setData(x, y);
   this->ui->widget->xAxis->setLabel("x");
   this->ui->widget->yAxis->setLabel("y");
   //this->ui->widget->rescaleAxes();
   this->ui->widget->xAxis->setRangeUpper(w[spec->GetNumberofWavelenghts()-1]);
   this->ui->widget->xAxis->setRangeLower(w[0]);

   this->ui->widget->yAxis->setRangeUpper(spec->GetMaxIntensity());
   this->ui->widget->yAxis->setRangeLower(0);

   this->ui->widget->replot();

    Dialog* g = new Dialog(this);
    g->show();
    */
}

void GroupBoxSpec::UpdateGraph(){
    printf("period:%d\t",period);
    etimer.start();
    bool updated;
    if(isranged){
        updated = spec->UpdateRanged(a,b);
    }else{
       updated = spec->UpdateAll();
    }
    if(updated){
                double* w = spec->GetWavelenghts(),*s = spec->GetSpectrum();
        int len;
        if(isranged){
            len = b-a;
        }else{
            len = spec->GetNumberofWavelenghts();
        }
        QVector<double> x(len),y(len);

        for (int i=0; i<len; i++)
        {
          x[i] = w[i+a];
          y[i] = s[i];
        }
        this->ui->widget->clearGraphs();
        this->ui->widget->addGraph();
        this->ui->widget->graph(0)->setData(x, y);
        this->ui->widget->xAxis->setLabel("x");
        this->ui->widget->yAxis->setLabel("y");
        //this->ui->widget->rescaleAxes();
        //this->ui->widget->yAxis->setRangeUpper(5000);
        this->ui->widget->xAxis->setRangeUpper(w[b-1]);
        this->ui->widget->xAxis->setRangeLower(w[a]);
        this->ui->widget->yAxis->setRangeUpper(spec->GetMaxIntensity());
        this->ui->widget->yAxis->setRangeLower(0);
        this->ui->widget->replot();
    }else{
       ui->checkStreaming->setChecked(false);
       if(this->timer->isActive()){
            this->timer->stop();
       }
    }
    int elapsed = etimer.elapsed();

    int remainingTime = period - elapsed;
    if (remainingTime <= 0){
        period = 1.0*(float)elapsed;
    }else{
        period = period * (1 - ((float)remainingTime/(float)400));
    }
    if(period<min_period){
        period = min_period;
    }
    timer->setInterval(period);
}

void GroupBoxSpec::on_checkStreaming_clicked(bool checked)
{
    if(checked){
        //if(!this->timer->isActive()){
           timer->start(period);
        //}
    }else{
        //if(this->timer->isActive()){
            timer->stop();
        //}
    }
}

void GroupBoxSpec::on_pushButton_clicked()
{
    a = ui->lineEditA->text().toInt();
    b = ui->lineEditB->text().toInt();
    if(a<0){
        a=0;
        ui->lineEditA->setText(QString::number(a));
    }

    if(b>spec->GetNumberofWavelenghts()){
        b=spec->GetNumberofWavelenghts();
        ui->lineEditB->setText(QString::number(b));
    }

    if(a==0 && b==spec->GetNumberofWavelenghts()){
        isranged = false;
    }else{
        isranged = true;
    }

}

void GroupBoxSpec::on_Save_Aquisition_clicked()
{

    double *w = spec->GetWavelenghts();
    double *s = spec->GetSpectrum();
    int len;
    FILE* fp = fopen(this->ui->filenametext->text().toLatin1().data(),"r");
    if(fp!=NULL){
        fclose(fp);
        fp = fopen(this->ui->filenametext->text().toLatin1().data(),"a");

        if(isranged){
            len = b-a;
        }else{
            len = spec->GetNumberofWavelenghts();
        }

        for(int i=0;i<len;i++){
           fprintf(fp,"%.2lf",s[i]);
           if(i<len-1){
               fprintf(fp,"\t");
           }else{
               fprintf(fp,"\n");
           }
        }
        fclose(fp);


    }else{
        fp = fopen(this->ui->filenametext->text().toLatin1().data(),"a");
        if(isranged){
            len = b-a;
        }else{
            len = spec->GetNumberofWavelenghts();
        }
        fprintf(fp,"WS:\n");
        for(int i=0;i<len;i++){
           fprintf(fp,"%.2lf",w[i+a]);
           if(i<len-1){
               fprintf(fp,"\t");
           }else{
               fprintf(fp,"\n");
           }
        }

        fprintf(fp,"SP:\n");
        for(int i=0;i<len;i++){
           fprintf(fp,"%.2lf",s[i]);
           if(i<len-1){
               fprintf(fp,"\t");
           }else{
               fprintf(fp,"\n");
           }
        }

        fclose(fp);

    }










}
