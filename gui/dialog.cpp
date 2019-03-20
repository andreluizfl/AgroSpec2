#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    QVector<double> x(101), y(101);
    for (int i=0; i<101; ++i)
    {
      x[i] = i/50.0 - 1;
      y[i] = x[i]*x[i];
    }
    this->ui->widget->addGraph();
    this->ui->widget->graph(0)->setData(x, y);
    this->ui->widget->xAxis->setLabel("x");
    this->ui->widget->yAxis->setLabel("y");
    this->ui->widget->rescaleAxes();
    this->ui->widget->update();
}

Dialog::~Dialog()
{
    delete ui;
}
