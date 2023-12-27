#ifndef MODBUSCLIENT_H
#define MODBUSCLIENT_H


#include "modbus.h"
#include "videopanel.h"
#include <QString>
#include <QThread>

class ModbusTCPThread: public QThread
{
    Q_OBJECT

public:
    explicit ModbusTCPThread(QObject *parent = 0);
    modbus* _modbus;
private:



protected:
    void run();

signals:
    void getAlready();
    void startTrigger(int value);

public slots:


};


class ModbusTcpClient: public QObject
{

Q_OBJECT
public:
     ModbusTcpClient(VideoPanel *v);
    ~ModbusTcpClient();

     modbus* _modbus;
     uint16_t _numRecvRegs[1];
     uint16_t _counts[1];

signals:
     void startTrigger(int value);
     void sendPLCstate(int value);

private  slots:

    void send_result(int valueResult);
    void send_alarm(int valueAlarm);

public slots:
    void read_recv();

private:
    //QModbusClient被QModbusRtuSerialMaster和QModbusTcpClient继成
    QString qresult;
    int _num;

};


#endif // MODBUSCLIENT_H
