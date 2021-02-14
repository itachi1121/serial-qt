#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QDebug>
#include <QTimer>
/***********************************全局变量***********************************/
QDateTime current_date_time;   //获取当前时间
//串口助手
bool EnableTimeFlag;    //使能时间戳
bool HexSendFlag;       //使能Hex发送
bool HexRecvFlag;       //使能Hex接收
bool RnSendFlag;        //使能回车换行
bool SendDisplayFlag;   //发送回显
//网口助手

/****************************************************************************/

MainWindow::MainWindow(QWidget *parent) :QMainWindow(parent),ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("多功能调试助手");
    ui->lineEdit_STime->setText("1000");
    SerialSetInit();   //串口助手界面初始化
    NetworkInit();     //网口助手初始化

    ui->comboBox_ComType->addItem("UART");
    ui->comboBox_ComType->addItem("USB");
    ui->comboBox_ComType->addItem("TCP/IP");
}


// 串口设置界面初始化
void MainWindow::SerialSetInit()
{
     //使用foreach获取有效的串口信息
     foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
     {
         //这里相当于自动识别串口号之后添加到了cmb，如果要手动选择可以用下面列表的方式添加进去
         Serial.setPort(info);
         if(Serial.open(QIODevice::ReadWrite))
         {
             //将串口号添加到cmb
             ui->comboBox_Port->addItem(info.portName());
             //关闭串口等待人为(打开串口按钮)打开
             Serial.close();
         }
     }
    // 填充波特率
    QStringList Baud;
    Baud<<"1200"<<"2400"<<"4800"<<"9600"<<"19200"<<"38400"<<"115200";
    ui->comboBox_Baud->addItems(Baud);
    // 填充数据位
    QStringList DataBit;
    DataBit<<"5"<<"6"<<"7"<<"8";
    ui->comboBox_DataBit->addItems(DataBit);
    // 填充停止位
    QStringList StopBit;
    StopBit<<"1"<<"1.5"<<"2";
    ui->comboBox_StopBit->addItems(StopBit);
    // 填充效验位
    QStringList CheckBit;
    CheckBit<<"奇效验"<<"偶效验"<<"无";
    ui->comboBox_CheckBit->addItems(CheckBit);

    //初始化默认参数
    ui->comboBox_Baud->setCurrentIndex(3); //默认9600
    ui->comboBox_DataBit->setCurrentIndex(3); //默认8bit Data
    ui->comboBox_StopBit->setCurrentIndex(0); //默认1bit Stop
    ui->comboBox_CheckBit->setCurrentIndex(2); //默认 无效验
}

// 网口助手界面初始化
void MainWindow::NetworkInit()
{

    ui->comboBox_StackType->addItem("UDP");
    ui->comboBox_StackType->addItem("TCP Server");
    ui->comboBox_StackType->addItem("TCP Client");

    ui->lineEdit_NetPort->setText("8080");


}

MainWindow::~MainWindow()
{
    delete ui;
}

//串口打开和关闭按钮
void MainWindow::on_pushButton_Open_clicked()
{
    //设置串口号;也就是说打开的是当前显示的串口
    if(ui->comboBox_Port->currentText().isEmpty())
    {
        QMessageBox::information(this,"提示","没有可用的串口");
        return;
    }
    Serial.setPortName(ui->comboBox_Port->currentText());
    if(ui->pushButton_Open->text() == "打开串口")
    {
           if(Serial.open(QIODevice::ReadWrite))//读写方式打开,成功后设置串口
           {
               //设置波特率
               Serial.setBaudRate(ui->comboBox_Baud->currentText().toInt());

               //设置数据位
               switch(ui->comboBox_DataBit->currentText().toInt())
               {
                     case 5:
                             Serial.setDataBits(QSerialPort::Data5);
                     break;
                     case 6:
                             Serial.setDataBits(QSerialPort::Data6);
                     break;
                     case 7:
                              Serial.setDataBits(QSerialPort::Data7);
                     break;
                     case 8:
                              Serial.setDataBits(QSerialPort::Data8);
                     break;
                     default:
                              QMessageBox::information(this,"提示","数据位配置出错");
                              return;
                     break;
               }

               //设置校验位
               if (ui->comboBox_CheckBit->currentText() == "奇效验")
               {
                   Serial.setParity(QSerialPort::OddParity);
               }
               else if (ui->comboBox_CheckBit->currentText() == "偶效验")
               {
                   Serial.setParity(QSerialPort::EvenParity);
               }
               else if (ui->comboBox_CheckBit->currentText() == "无")
               {
                   Serial.setParity(QSerialPort::NoParity);
               }

               //设置停止位
               if (ui->comboBox_StopBit->currentText().toFloat() == 1)
               {
                   Serial.setStopBits(QSerialPort::OneStop);
               }
               else if(ui->comboBox_StopBit->currentText().toFloat() == 1.5)
               {
                   Serial.setStopBits(QSerialPort::OneAndHalfStop);
               }
               else if(ui->comboBox_StopBit->currentText().toFloat() == 2)
               {
                   Serial.setStopBits(QSerialPort::TwoStop);
               }

               //设置流控制
               Serial.setFlowControl(QSerialPort::NoFlowControl);
               ui->pushButton_Open->setText("关闭串口");


               //建立串口接收的槽函数
               connect(&Serial,&QSerialPort::readyRead ,this,&MainWindow::ReadRecData);

              // timer0->start(100);

            }
           else//串口打开失败
           {
               QMessageBox::about(NULL, "提示", "打开出错，串口被占用！");
               return ;
           }
    }
    else if(ui->pushButton_Open->text() == "关闭串口")
    {

        Serial.close();//关串口
        ui->checkBox->setChecked(false);
        on_checkBox_clicked(false);
        ui->pushButton_Open->setText("打开串口");
    }
}

void MainWindow::Delay_MSec(unsigned int msec)
{
    QTime _Timer = QTime::currentTime().addMSecs(msec);

    while( QTime::currentTime() < _Timer )

    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

QString MainWindow::AddSpaceString(QString data)
{
    QString textString;
    for(int i = 0; i < data.length() / 2; i++)
    {
        textString += data.mid(i*2, 2);
        textString += " ";
    }
    return textString;
}

// 串口接收数据
void MainWindow::ReadRecData()
{
    QByteArray readData;
    QByteArray NewData;
    QString current_date;

    if(HexRecvFlag)  //判断是否使用HEX
    {
        //判断是否使用时间戳
        if(EnableTimeFlag == 1)
        {
            current_date_time = QDateTime::currentDateTime();
            current_date += "[";
            current_date += current_date_time.toString("yyyy-MM-dd hh:mm:ss");
            current_date += "]收->";
            Delay_MSec(300);
            readData = Serial.readAll(); //读取串口数据
            if(!readData.isNull())
            {
                QString readString = QString(readData.toHex().toUpper());
                QString textString = AddSpaceString(readString);
                ui->textEdit_Recv->append(current_date.toUtf8() + textString);
            }
        }
        else
        {
//           ui->textEdit_Recv->append(readData.toHex());
            readData = Serial.readAll(); //读取串口数据
            if(!readData.isNull())
            {
                QString readString = QString(readData.toHex().toUpper());
                QString textString = " " + AddSpaceString(readString);
                ui->textEdit_Recv->insertPlainText(textString); //insertPlainText不会自动换行，但是CPU占用率高，可能会卡
            }
        }
    }
    else
    {
        //判断是否使用时间戳
        if(EnableTimeFlag == 1)
        {
            current_date_time = QDateTime::currentDateTime();
            current_date += "[";
            current_date += current_date_time.toString("yyyy-MM-dd hh:mm:ss");
            current_date += "]收->";
            Delay_MSec(300);
            readData = Serial.readAll(); //读取串口数据
            if(!readData.isNull())
            {
                ui->textEdit_Recv->append(current_date.toUtf8() + readData);
            }
        }
        else
        {
            //ui->textEdit_Recv->append(readData);  //使用append会自动换行，但是只刷新更新的内容，CPU占用率低不会卡
            readData = Serial.readAll(); //读取串口数据
            if(!readData.isNull())
            {
                ui->textEdit_Recv->insertPlainText(QString(readData)); //insertPlainText不会自动换行，但是CPU占用率高，可能会卡
            }
        }
    }
#if 0
   if(!readData.isNull())//将读到的数据显示到数据接收区
   {
       if(HexRecvFlag)  //判断是否使用HEX
       {
           //判断是否使用时间戳
           if(EnableTimeFlag == 1)
           {
               current_date_time = QDateTime::currentDateTime();
               current_date += "[";
               current_date += current_date_time.toString("yyyy-MM-dd hh:mm:ss");
               current_date += "]收->";
               ui->textEdit_Recv->append(current_date.toUtf8() + readData.toHex());
           }
           else
           {
              ui->textEdit_Recv->append(readData.toHex());
           }

       }
       else
       {
           //判断是否使用时间戳
           if(EnableTimeFlag == 1)
           {
               Delay_MSec(100);
               current_date_time = QDateTime::currentDateTime();
               current_date += "[";
               current_date += current_date_time.toString("yyyy-MM-dd hh:mm:ss");
               current_date += "]收->";
               ui->textEdit_Recv->append(current_date.toUtf8() + readData);
           }
           else
           {
               //ui->textEdit_Recv->append(readData);  //使用append会自动换行，但是只刷新更新的内容，CPU占用率低不会卡
               ui->textEdit_Recv->insertPlainText(QString(readData)); //insertPlainText不会自动换行，但是CPU占用率高，可能会卡
           }
       }

   }
#endif
}

//清除接收窗口数据
void MainWindow::on_pushButton_ClearRecv_clicked()
{
   ui->textEdit_Recv->clear();
}

//清除发送窗口数据
void MainWindow::on_pushButton_2_clicked()
{
   ui->textEdit_Send->clear();
}


//将一个字符串转换成十六进制
QByteArray MainWindow::QString2Hex(QString str)
{
    QByteArray senddata;
    int hexdata,lowhexdata;
    int hexdatalen = 0;
    int len = str.length();
    senddata.resize(len/2);
    char lstr,hstr;
    for(int i=0; i<len; )
    {
        hstr=str[i].toLatin1();
        if(hstr == ' ')
        {
            i++;
            continue;
        }
        i++;
        if(i >= len)
        break;
        lstr = str[i].toLatin1();
        hexdata = ConvertHexChar(hstr);
        lowhexdata = ConvertHexChar(lstr);
        if((hexdata == 16) || (lowhexdata == 16))
            break;
        else
            hexdata = hexdata*16+lowhexdata;
        i++;
        senddata[hexdatalen] = (char)hexdata;
        hexdatalen++;
    }
    senddata.resize(hexdatalen);
    return senddata;
}


char MainWindow::ConvertHexChar(char ch)
{
    if((ch >= '0') && (ch <= '9'))
        return ch-0x30;
    else if((ch >= 'A') && (ch <= 'F'))
        return ch-'A'+10;
    else if((ch >= 'a') && (ch <= 'f'))
        return ch-'a'+10;
    else return (-1);
}

//发送数据
void MainWindow::on_pushButton_Send_clicked()
{
    QString DataStr;
    QString NewData;
    QString current_date;

    DataStr = ui->textEdit_Send->toPlainText();
    if(ui->pushButton_Open->text() == "打开串口")
    {
       QMessageBox::information(this,"提示","未打开串口");
       return;
    }

    if(RnSendFlag)
    {
        DataStr = DataStr + '\r';
    }
    if(HexSendFlag)        //使能Hex发送时
    {
        QStringList strList = DataStr.split(" ", QString::SkipEmptyParts);  //
        QByteArray str_send;

        for(int i = 0;i < strList.size(); i++)
        {
            str_send += QString2Hex(strList[i]);
        }

       ui->checkBox_SendRN->setChecked(false);
       if(EnableTimeFlag)  //判断是否使能时间戳
       {
           current_date_time = QDateTime::currentDateTime();
           current_date += "[";
           current_date += current_date_time.toString("yyyy-MM-dd hh:mm:ss");
           current_date += "]发->";
           NewData = current_date + DataStr;
            if(SendDisplayFlag == 1)
           {
               ui->textEdit_Recv->append(NewData);
           }
       }
       else
       {
          NewData = DataStr;
          if(SendDisplayFlag == 1)
          {
              ui->textEdit_Recv->append(NewData);
          }
       }
       Serial.write(str_send);//写入缓冲区
    }
    else           //不使能Hex发送时
    {
        if(EnableTimeFlag)  //判断是否使能时间戳
        {
            current_date_time = QDateTime::currentDateTime();
            current_date += "[";
            current_date += current_date_time.toString("yyyy-MM-dd hh:mm:ss");
            current_date += "]发->";
            NewData = current_date + DataStr.toUtf8();
            if(SendDisplayFlag == 1)
            {
                ui->textEdit_Recv->append(NewData);
            }
        }
        else
        {
           NewData = DataStr;
           if(SendDisplayFlag == 1)
           {
               ui->textEdit_Recv->append(NewData);
           }
        }
        Serial.write(DataStr.toUtf8());//写入缓冲区
    }

}

//使能时间戳
void MainWindow::on_checkBox_EnableTime_clicked(bool checked)
{
     if(checked == true)
     {
         EnableTimeFlag = 1;
     }
     else
     {
         EnableTimeFlag = 0;
     }
}
//使能定时发送
void MainWindow::on_checkBox_clicked(bool checked)
{
    if(checked == true)
    {
        if(ui->pushButton_Open->text() == "打开串口")
        {
           QMessageBox::information(this,"提示","未打开串口");
           ui->checkBox->setChecked(false);
           return;
        }
        quint32 stime= ui->lineEdit_STime->text().toInt();
        timer_id1 = startTimer(stime);
        ui->lineEdit_STime->setEnabled(false);
    }
    else
    {
       killTimer(timer_id1);
       ui->lineEdit_STime->setEnabled(true);
    }
}

// HEX 接收 按钮
void MainWindow::on_checkBox_HexRecv_clicked(bool checked)
{
    if(checked)
    {
        HexRecvFlag = 1;
    }

    else
        HexRecvFlag = 0;
}
// HEX 发送 按钮
void MainWindow::on_checkBox_HexSend_clicked(bool checked)
{
    if(checked)
        HexSendFlag = 1;
    else
        HexSendFlag = 0;
}
//发送新行 按钮
void MainWindow::on_checkBox_SendRN_clicked(bool checked)
{
    if(checked)
        RnSendFlag = 1;
    else
        RnSendFlag = 0;
}

//发送回显 按钮
void MainWindow::on_checkBox_SendDisplay_clicked(bool checked)
{
    if(checked)
        SendDisplayFlag = 1;
    else
        SendDisplayFlag = 0;
}

//定时器中断
void MainWindow:: timerEvent(QTimerEvent *ev)
{
    if(ev->timerId() == timer_id1)
    {
       on_pushButton_Send_clicked();
    }
}


void MainWindow::on_pushButton_FlushPort_clicked()
{
    ui->comboBox_Port->clear();  //清除串口下拉列表内容
    //使用foreach获取有效的串口信息
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
       //将串口号添加到cmb
       ui->comboBox_Port->addItem(info.portName());
    }
}
