#ifndef TESTWINDOW_H
#define TESTWINDOW_H

#include <QMainWindow>

namespace Ui {
class TestWindow;
}

class TestWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TestWindow(QWidget *parent = 0);
    ~TestWindow();

private slots:
    void on_pushButton_clicked();

    void on_Probe_clicked();

    void on_Open_clicked();

    void on_Close_clicked();

    void on_pushReleaseAll_clicked();

    void on_checkRemote_clicked(bool checked);

    void on_checkSource_clicked(bool checked);

private:
    Ui::TestWindow *ui;
};

#endif // TESTWINDOW_H
