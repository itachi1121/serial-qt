#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


#include<QSerialPort>
#include<QSerialPortInfo>
#include<QTimer>
#include <QTime>
#include <QDateTime>
#include <QPainter>
#include <QLabel>




namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void timerEvent(QTimerEvent *ev);
    QTimer uart_time;
private slots:
    void on_pushButton_Open_clicked();
    void ReadRecData();

    void on_pushButton_ClearRecv_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_Send_clicked();

    void on_checkBox_EnableTime_clicked(bool checked);
    void on_checkBox_clicked(bool checked);



    void on_checkBox_HexRecv_clicked(bool checked);

    void on_checkBox_HexSend_clicked(bool checked);

    void on_checkBox_SendRN_clicked(bool checked);

    void on_checkBox_SendDisplay_clicked(bool checked);
    void on_pushButton_FlushPort_clicked();
    void Delay_MSec(unsigned int msec);
private:
    Ui::MainWindow *ui;
    QSerialPort Serial;

    int timer_id1;
    void SerialSetInit(void);
    void NetworkInit(void);

};

#endif // MAINWINDOW_H
