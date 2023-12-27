#include "moduleview.h"
#include "ui_moduleview.h"
#include <fstream>
#include <iostream>
#include <json/config.h>
#include <json/json.h>
#include <json/reader.h>
#include <sys/time.h>
#include <unistd.h>
#include "ifaddrs.h"

#include<cstring>

using namespace std;
#define CONFIG_JSON "/home/q/Desktop/light_config.json"

extern QLabel* newLabel(QString text, int w, int h);
extern QLineEdit* newLineEdit(QString text, int w, int h, QList<int> validator, QString qssStr);
extern QCheckBox* newCheckBox(QString text, int w, int h, bool isChecked);//, QWidget* receiver, void* slot);
extern QToolButton* newToolButton(QString text, int w, int h, QString iconPath, bool checkable, bool isChecked, Qt::ToolButtonStyle style, QString qssStr);

extern void newHBoxLayout(QWidget* parent, QList<QWidget*> widgets, int splitW, int space, QList<int> margins, Qt::Alignment align);
extern void newVBoxLayout(QWidget* parent, QList<QWidget*> widgets, int splitW, int space, QList<int> margins, Qt::Alignment align);
extern QWidget* newWidgetLine(int w, int h, QString qssStr);

QString cfg_usb1="ttyUSB1";
int cfg_innerFreq1=18000;
int cfg_outputNum1=1970;

QString cfg_usb2="ttyUSB2";
int cfg_innerFreq2=18000;
int cfg_outputNum2=1970;

#include <QSerialPort>        //提供访问串口的功能
#include <QSerialPortInfo>    //提供系统中存在的串口的信息

//创建串口对象
QSerialPort serial1;
QSerialPort serial2;

int light1Success=0;
int light2Success=0;

static uint16_t crctalbeabs[] = {
    0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
    0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400
};

uint16_t Crc16(uint8_t* ptr, uint32_t len);

#define HIG_UINT16(a)   ( ((a)>>8) & 0xFF )
#define LOW_UINT16(a)   ( (a) & 0xFF )

#define HIG_UINT8(a)    ( ((a)>>4) & 0x0F )
#define LOW_UINT8(a)    ( (a) & 0x0F )

uint16_t Crc16(uint8_t* ptr, uint32_t len)
{
    uint16_t crc = 0xffff;
    uint32_t i;
    uint8_t ch;

    for (i = 0; i < len; i++) {
        ch = *ptr++;
        crc = crctalbeabs[(ch ^ crc) & 15] ^ (crc >> 4);
        crc = crctalbeabs[((ch >> 4) ^ crc) & 15] ^ (crc >> 4);
    }

    return crc;
}

AlgoSettingWidget::AlgoSettingWidget(QWidget* parent):QWidget(parent)
{
    init_UI();
    JsonIORead(CONFIG_JSON);
}

AlgoSettingWidget::~AlgoSettingWidget()
{

}

void AlgoSettingWidget::init_UI()
{
    int w = 90;
    int ww = 80;
    int h = 20;
    int www = 200;
    int hhh = 30;

    QLabel* labelLight1 = newLabel("光源1:", w, h);

    QLabel* label1 = newLabel("usb口:", w, h);
    lineEditUsb1 = newLineEdit("", www, hhh);
    QLabel* label2 = newLabel("内部信号频率:", w, h);
    lineEditInnerFreq1 = newLineEdit("", www, hhh);
    QLabel* label3 = newLabel("计数输出个数:", w, h);
    lineEditOutputNum1 = newLineEdit("", www, hhh);

    QWidget* wLine = newWidgetLine(0, 2, "border: 1px solid rgb(200, 200, 200);");
    QLabel* labelLight2 = newLabel("光源2:", w, h);
    QLabel* label4 = newLabel("usb口:", w, h);
    lineEditUsb2 = newLineEdit("", www, hhh);
    QLabel* label5 = newLabel("内部信号频率:", w, h);
    lineEditInnerFreq2 = newLineEdit("", www, hhh);
    QLabel* label6 = newLabel("计数输出个数:", w, h);
    lineEditOutputNum2 = newLineEdit("", www, hhh);
    labelStatus = newLabel("", www, 50);

    wParam123 = new QWidget;
    QGridLayout* layout123 = new QGridLayout(wParam123);

    layout123->addWidget(labelLight1, 0, 0);

    layout123->addWidget(label1, 1, 0);
    layout123->addWidget(lineEditUsb1, 1, 1);
    layout123->addWidget(label2, 2, 0);
    layout123->addWidget(lineEditInnerFreq1, 2, 1);
    layout123->addWidget(label3, 3, 0);
    layout123->addWidget(lineEditOutputNum1, 3, 1);

    layout123->addWidget(wLine, 4, 0,1,2);

    layout123->addWidget(labelLight2, 5, 0);
    layout123->addWidget(label4, 6, 0);
    layout123->addWidget(lineEditUsb2, 6, 1);
    layout123->addWidget(label5, 7, 0);
    layout123->addWidget(lineEditInnerFreq2,7, 1);
    layout123->addWidget(label6, 8, 0);
    layout123->addWidget(lineEditOutputNum2, 8, 1);
    layout123->addWidget(labelStatus,9, 0,1,2);

    layout123->setSpacing(10);
    layout123->setContentsMargins(0, 0, 10, 10);
    layout123->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    //123
    buttonYes = newToolButton("确认设置",www, 50, "", false, false, Qt::ToolButtonTextOnly);
    connect(buttonYes, &QToolButton::clicked, this, &AlgoSettingWidget::on_toolButton_clicked);

    QWidget* w123456 = new QWidget;
    newVBoxLayout(w123456, {wParam123, buttonYes}, 0, 0, {0,0,0,0}, Qt::AlignLeft | Qt::AlignTop);

    // zong
    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(w123456, 2, 0, 5, 4);


    layout->setSpacing(10);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
}

//编写的槽函数
int AlgoSettingWidget::lightSerial1_readyRead()
{
    //从接收缓冲区中读取数据
    QByteArray buffer = serial1.readAll();
    printf("recieve rs232:%x\n",
           (unsigned char)buffer[0]&0xff);

    if((buffer[0]&0xff)==0x5a){
         light1Success=1;
         return 1;
    }

    light1Success=0;
    return 0;
}

int AlgoSettingWidget::lightSerial2_readyRead()
{
    //从接收缓冲区中读取数据
    QByteArray buffer = serial2.readAll();
    printf("recieve rs232:%x\n",
           (unsigned char)buffer[0]&0xff);

    if((buffer[0]&0xff)==0x5a){
         light2Success=1;
         return 1;
    }

    light2Success=0;
    return 0;
}

void Delay_MSec(unsigned int msec)
{
    QEventLoop loop;//定义一个新的事件循环
    QTimer::singleShot(msec, &loop, SLOT(quit()));  //创建单次定时器，槽函数为事件循环的退出函数
    loop.exec();  //事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}


void AlgoSettingWidget::on_toolButton_clicked()
{
    int ret=JsonIOWrite(CONFIG_JSON);
    if(ret<0){
        return;
    }
    labelStatus->setText("<h2><font color=red>光源1串口参数保存成功</font></h2>");
    qDebug("光源1串口参数保存成功\n");
    printf("光源1串口参数保存成功\n");

    //设置串口名
    serial1.setPortName(cfg_usb1);
    //设置波特率
    serial1.setBaudRate(QSerialPort::Baud19200); //19200
    //设置数据位数
    serial1.setDataBits(QSerialPort::Data8);
    //设置奇偶校验
    serial1.setParity(QSerialPort::NoParity);
    //设置停止位
    serial1.setStopBits(QSerialPort::OneStop);
    //设置流控制
    serial1.setFlowControl(QSerialPort::NoFlowControl);
    //打开串口
    if(serial1.open(QIODevice::ReadWrite)){
        QObject::connect(&serial1, &QSerialPort::readyRead, this, &AlgoSettingWidget::lightSerial1_readyRead);
        qDebug("光源1串口连接成功\n");
        printf("光源1串口连接成功\n");
        labelStatus->setText("<h2><font color=red>光源1串口连接成功</font></h2>");
    }else{
        qDebug("serial1 open fail!\n");
        printf("serial1 open fail!\n");

        labelStatus->setText("<h2><font color=red>光源1串口连接失败</font></h2>");
        return;
    }

    unsigned char send[]={0x5a,0x08,0x20,0x00,0x4e,0x22,0x22,0x00,0x08,0x96,0x17,0x15,0x57};
    QByteArray byArr;
    byArr.resize(13);

    send[4]=((cfg_innerFreq1>>8)&0xff);
    send[5]=(cfg_innerFreq1&0xff);

    send[8]=((cfg_outputNum1>>8)&0xff);
    send[9]=(cfg_outputNum1&0xff);

    uint16_t re=Crc16(send+1, 9);  //5
    printf("crc16:%x %x\n",HIG_UINT16(re), LOW_UINT16(re));
    send[10]=HIG_UINT16(re);
    send[11]=LOW_UINT16(re);

    for(int i=0;i<13;i++) {
       byArr[i]=send[i];
       printf("b[%d]=0x%2x\n",i,send[i]);
    }

    serial1.write(byArr);
    Delay_MSec(5000);

    if(light1Success==1){
       labelStatus->setText("<h2><font color=red>光源1串口设置成功</font></h2>");
       qDebug("光源1串口设置成功\n");
       printf("光源1串口设置成功\n");
    }else{
       labelStatus->setText("<h2><font color=red>光源1串口设置失败</font></h2>");
       qDebug("光源1串口设置失败\n");
       printf("光源1串口设置失败\n");
       return;
    }
    serial1.close();

    labelStatus->setText("<h2><font color=red>光源2串口参数保存成功</font></h2>");
        qDebug("光源2串口参数保存成功\n");
        printf("光源2串口参数保存成功\n");

        //设置串口名
        serial2.setPortName(cfg_usb2);
        //设置波特率
        serial2.setBaudRate(QSerialPort::Baud19200); //29200
        //设置数据位数
        serial2.setDataBits(QSerialPort::Data8);
        //设置奇偶校验
        serial2.setParity(QSerialPort::NoParity);
        //设置停止位
        serial2.setStopBits(QSerialPort::OneStop);
        //设置流控制
        serial2.setFlowControl(QSerialPort::NoFlowControl);
        //打开串口
        if(serial2.open(QIODevice::ReadWrite)){
            QObject::connect(&serial2, &QSerialPort::readyRead, this, &AlgoSettingWidget::lightSerial2_readyRead);
            qDebug("光源2串口连接成功\n");
            printf("光源2串口连接成功\n");
            labelStatus->setText("<h2><font color=red>光源2串口连接成功</font></h2>");
        }else{
            qDebug("serial2 open fail!\n");
            printf("serial2 open fail!\n");

            labelStatus->setText("<h2><font color=red>光源2串口连接失败</font></h2>");
            return;
        }

        unsigned char send2[]={0x5a,0x08,0x20,0x00,0x4e,0x22,0x22,0x00,0x08,0x96,0x17,0x15,0x57};

        byArr.resize(13);

        send2[4]=((cfg_innerFreq2>>8)&0xff);
        send2[5]=(cfg_innerFreq2&0xff);

        send2[8]=((cfg_outputNum2>>8)&0xff);
        send2[9]=(cfg_outputNum2&0xff);

        re=Crc16(send2+1, 9);  //5
        printf("crc16:%x %x\n",HIG_UINT16(re), LOW_UINT16(re));
        send2[10]=HIG_UINT16(re);
        send2[11]=LOW_UINT16(re);

        for(int i=0;i<13;i++) {
           byArr[i]=send2[i];
           printf("b[%d]=0x%2x\n",i,send2[i]);
        }

        serial2.write(byArr);
        Delay_MSec(5000);

        if(light2Success==1){
           labelStatus->setText("<h2><font color=red>光源2串口设置成功</font></h2>");
           qDebug("光源2串口设置成功\n");
           printf("光源2串口设置成功\n");
        }else{
           labelStatus->setText("<h2><font color=red>光源2串口设置失败</font></h2>");
           qDebug("光源2串口设置失败\n");
           printf("光源2串口设置失败\n");
           return;
        }
        serial2.close();
}

int AlgoSettingWidget::JsonIORead(QString path)
{
    Json::Reader reader;
    Json::Value root;
    std::ifstream in(path.toLatin1(), std::ios::binary);

    if (reader.parse(in, root))
    {
        Json::Value devalue=root["light1"];

        cfg_usb1=QString::fromStdString(devalue["usb"].asString());
        cfg_innerFreq1=devalue["innerFreq"].asInt();
        cfg_outputNum1=devalue["outputNum"].asInt();

        devalue=root["light2"];

        cfg_usb2=QString::fromStdString(devalue["usb"].asString());
        cfg_innerFreq2=devalue["innerFreq"].asInt();
        cfg_outputNum2=devalue["outputNum"].asInt();
    }

    lineEditUsb1->setText(cfg_usb1);
    lineEditInnerFreq1->setText(QString::number(cfg_innerFreq1));
    lineEditOutputNum1->setText(QString::number(cfg_outputNum1));

    lineEditUsb2->setText(cfg_usb2);
    lineEditInnerFreq2->setText(QString::number(cfg_innerFreq2));
    lineEditOutputNum2->setText(QString::number(cfg_outputNum2));

    return 0;
}

int AlgoSettingWidget::JsonIOWrite(QString path)
{
    printf("start write json_path\n");

    if (lineEditUsb1->text().compare("") == 0 || lineEditInnerFreq1->text().compare("") == 0 || lineEditOutputNum1->text().compare("") == 0
    || lineEditUsb2->text().compare("") == 0 || lineEditInnerFreq2->text().compare("") == 0 || lineEditOutputNum2->text().compare("") == 0){
        QMessageBox mgBox(QMessageBox::Critical, "错误", "参数不可为0!!", QMessageBox::Yes | QMessageBox::No, this);
        mgBox.setFixedSize(400, 300);
        mgBox.exec();
        return -1;
    }

    cfg_usb1=lineEditUsb1->text();
    cfg_innerFreq1=lineEditInnerFreq1->text().toInt();
    cfg_outputNum1=lineEditOutputNum1->text().toInt();

    cfg_usb2=lineEditUsb2->text();
    cfg_innerFreq2=lineEditInnerFreq2->text().toInt();
    cfg_outputNum2=lineEditOutputNum2->text().toInt();

    Json::Value root;
    Json::Value light1;
    Json::Value light2;

    // 组装json内容
    light1["usb"] = cfg_usb1.toStdString();
    light1["innerFreq"] = cfg_innerFreq1;
    light1["outputNum"] = cfg_outputNum1;

    light2["usb"] = cfg_usb2.toStdString();
    light2["innerFreq"] = cfg_innerFreq2;
    light2["outputNum"] = cfg_outputNum2;

    root["light1"] = light1;
    root["light2"] = light2;

    // 将json内容（缩进格式）输出到文件
    Json::StyledWriter writer;
    ofstream os;
    os.open(path.toLatin1());
    os << writer.write(root);
    os.close();

    printf("end write json_path: success!  %s\n",path.toLatin1().data());
    return 0;
}

Moduleview::Moduleview(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Moduleview)
{
    ui->setupUi(this);
}

Moduleview::~Moduleview()
{

    delete ui;
}



void Moduleview::module_on_pushButton_clicked()
{

}
