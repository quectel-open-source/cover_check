#ifndef ModbusTcpClient_H
#define ModbusTcpClient_H


#include <QModbusDataUnit>
#include <QModbusClient>
#include "videopanel.h"



class ModbusTcpClient: public QObject
{


public:
     ModbusTcpClient(VideoPanel *v);
//    QStringList list;
    ~ModbusTcpClient();

 QModbusClient *modbusDevice;


private  slots:


    void on_connect();
    void onStateChanged(int state);
    void on_send(int addr,QString sendstr);
    void on_read();
    void readReady();
    void beat();
    void send_alarm();
    void send_result(QString value);
private:
    //QModbusClient被QModbusRtuSerialMaster和QModbusTcpClient继成
    QVector<quint16> m_holdingRegisters;
    QString qresult;


};

#endif // ModbusTcpClient_H
