#include "QuickSetThread.h"
#include <QLabel>
#include <iostream>
#include <stdio.h>
#include <QSerialPort>

#include <json/config.h>
#include <json/json.h>
#include <json/reader.h>

using namespace std;

extern int cfg_isSavePic;
extern int cfg_abDelay;
extern int cfg_cdDelay;
extern int cfg_coverWidth;
extern int cfg_ironWidth;

extern int cfg_innerFreq1;
extern int cfg_outputNum1;

extern int cfg_innerFreq2;
extern int cfg_outputNum2;
extern QString cfg_usb1;
extern QString cfg_usb2;

extern float cfg_coverHigh;
extern float cfg_beltRate;
int cfg_cameraWidth1;
int cfg_cameraWidth2;

#define HIG_UINT16(a)   ( ((a)>>8) & 0xFF )
#define LOW_UINT16(a)   ( (a) & 0xFF )

#define HIG_UINT8(a)    ( ((a)>>4) & 0x0F )
#define LOW_UINT8(a)    ( (a) & 0x0F )


uint16_t quickCrctalbeabs[] = {
    0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
    0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400
};

uint16_t quickCrc16(uint8_t* ptr, uint32_t len)
{
    uint16_t crc = 0xffff;
    uint32_t i;
    uint8_t ch;

    for (i = 0; i < len; i++) {
        ch = *ptr++;
        crc = quickCrctalbeabs[(ch ^ crc) & 15] ^ (crc >> 4);
        crc = quickCrctalbeabs[((ch >> 4) ^ crc) & 15] ^ (crc >> 4);
    }

    return crc;
}

QuickSetThread::QuickSetThread(QObject *parent) : QThread(parent)
{

}

void QuickSetThread::saveToRun(){
    Json::Value root;
    Json::Value run;
    // 组装json内容
    run["isSavePic"]  = cfg_isSavePic;
    run["abDelay"] = cfg_abDelay;
    run["cdDelay"] = cfg_cdDelay;
    run["coverWidth"] = cfg_coverWidth;
    run["ironWidth"] = cfg_ironWidth;

    root["runConfig"] = run;
    // 将json内容（缩进格式）输出到文件
    Json::StyledWriter writer;
    ofstream os;
    QString path="/root/run_config.json";
    os.open(path.toLatin1());
    os << writer.write(root);
    os.close();
}

void QuickSetThread::saveToLight(){
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
    QString path="/root/light_config.json";
    os.open(path.toLatin1());
    os << writer.write(root);
    os.close();
}

//编写的槽函数
int QuickSetThread::lightSerial1_readyRead()
{
    //从接收缓冲区中读取数据
    QByteArray buffer = serial1.readAll();
    sendMsg(QString("recieve rs232:%1\n").arg((unsigned char)buffer[0]&0xff));

    if((buffer[0]&0xff)==0x5a){
         light1Success=1;
         return 1;
    }

    light1Success=0;
    return 0;
}

int QuickSetThread::lightSerial2_readyRead()
{
    //从接收缓冲区中读取数据
    QByteArray buffer = serial2.readAll();
    sendMsg(QString("recieve rs232:%1\n").arg((unsigned char)buffer[0]&0xff));

    if((buffer[0]&0xff)==0x5a){
         light2Success=1;
         return 1;
    }

    light2Success=0;
    return 0;
}

void QuickSetThread::Delay_MSec(unsigned int msec)
{
    QEventLoop loop;//定义一个新的事件循环
    QTimer::singleShot(msec, &loop, SLOT(quit()));  //创建单次定时器，槽函数为事件循环的退出函数
    loop.exec();  //事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}


void QuickSetThread::setSerial()
{
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
        sendMsg("光源1串口连接成功\n");
    }else{
        sendMsg("serial1 open fail!\n");
        return;
    }

    unsigned char send[]={0x5a,0x08,0x20,0x00,0x4e,0x22,0x22,0x00,0x08,0x96,0x17,0x15,0x57};
    QByteArray byArr;
    byArr.resize(13);

    send[4]=((cfg_innerFreq1>>8)&0xff);
    send[5]=(cfg_innerFreq1&0xff);

    send[8]=((cfg_outputNum1>>8)&0xff);
    send[9]=(cfg_outputNum1&0xff);

    uint16_t re=quickCrc16(send+1, 9);  //5
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
       sendMsg("光源1串口设置成功\n");
    }else{
       sendMsg("光源1串口设置失败\n");
       return;
    }
    serial1.close();

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
            sendMsg("光源2串口连接成功\n");

        }else{
            sendMsg("serial2 open fail!\n");
            return;
        }

        unsigned char send2[]={0x5a,0x08,0x20,0x00,0x4e,0x22,0x22,0x00,0x08,0x96,0x17,0x15,0x57};

        byArr.resize(13);

        send2[4]=((cfg_innerFreq2>>8)&0xff);
        send2[5]=(cfg_innerFreq2&0xff);

        send2[8]=((cfg_outputNum2>>8)&0xff);
        send2[9]=(cfg_outputNum2&0xff);

        re=quickCrc16(send2+1, 9);  //5
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
           sendMsg("光源2串口设置成功\n");
        }else{
           sendMsg("光源2串口设置失败\n");
           return;
        }
        serial2.close();


        sendMsg("设置光学参数OK");

}



//extern void sigsegv_test();
void QuickSetThread::run()
{
    int ret=0;
    if(cfg_coverHigh<0.000001){
        sendMsg("屏蔽壳宽度设置过小，设置不成功！");
        return;
    }
    if(cfg_beltRate<1){
        sendMsg("皮带速率设置过小，设置不成功！");
        return;
    }
    float scale=(cfg_coverHigh/21.4)*(350/cfg_beltRate);
    //计算参数
    cfg_coverWidth=scale*100;
    cfg_ironWidth=scale*8;
    if(cfg_ironWidth<3){
        cfg_ironWidth=3;
    }
    QString temp=QString("coverWidth%1 ironWidth%2\n").arg(cfg_coverWidth).arg(cfg_ironWidth);
    sendMsg(temp);

    cfg_abDelay=(583*350)/cfg_beltRate;
    cfg_cdDelay=(1680*350)/cfg_beltRate;

    cfg_innerFreq1=18000/(350/cfg_beltRate);
    cfg_outputNum1=1970*scale;

    cfg_innerFreq2=18000/(350/cfg_beltRate);
    cfg_outputNum2=1970*scale;

    cfg_cameraWidth1=1920*scale;
    cfg_cameraWidth2=1920*scale;

    temp=QString("outputNum%1 cameraWidth%2\n").arg(cfg_outputNum1).arg(cfg_cameraWidth1);
    sendMsg(temp);

    saveToRun();
    sendMsg("设置运行参数OK");
    saveToLight();
    sendMsg("设置Light参数OK");
    sendMsg("Please to confirm one by one!");
    //setSerial();


}








