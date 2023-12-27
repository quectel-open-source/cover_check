#ifndef QUICKSETTHREAD_H
#define QUICKSETTHREAD_H

#include <QDate>
#include <QString>
#include <QThread>

#include <fstream>
#include <iostream>
#include <json/config.h>
#include <json/json.h>
#include <json/reader.h>
#include <QFile>

#include <vector>
#include <QMutex>
#include <QSerialPort>
#include <QEventLoop>
#include <QTimer>

using namespace std;


class QuickSetThread: public QThread
{
    Q_OBJECT

public:
    explicit QuickSetThread(QObject *parent = 0);

    void run();
    void saveToRun();
    void saveToLight();
    void setSerial();

    QSerialPort serial1;
    QSerialPort serial2;

    int light1Success=0;
    int light2Success=0;
    int lightSerial1_readyRead();
    int lightSerial2_readyRead();
    void Delay_MSec(unsigned int msec);
signals:
    void sendMsg(QString str);

};


#endif
