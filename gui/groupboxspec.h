#ifndef GROUPBOXSPEC_H
#define GROUPBOXSPEC_H

#include <QGroupBox>
#include <QTimer>
#include <QElapsedTimer>
#include "interface/ispectrometer.h"

using namespace Agrospec;

namespace Ui {
class GroupBoxSpec;
}

class GroupBoxSpec : public QGroupBox
{
    Q_OBJECT

public:
    explicit GroupBoxSpec(QWidget *parent = 0);
    ~GroupBoxSpec();

    void SetSpectrometer(ISpectrometer* spec);

private slots:
    void RemoveSpectrometer(QObject* obj);

    void on_pushButtonSetIt_clicked();

    void on_horizontalSliderIt_valueChanged(int value);

    void on_Plot_clicked();

    void on_checkStreaming_clicked(bool checked);
    void on_pushButton_clicked();

    void on_Save_Aquisition_clicked();

public slots:
    void UpdateGraph();
private:
    ISpectrometer* spec;
    Ui::GroupBoxSpec *ui;
    QTimer *timer;
    QElapsedTimer etimer;
    int period;
    int min_period;
    bool isranged;
    int a;int b;
};

#endif // GROUPBOXSPEC_H
