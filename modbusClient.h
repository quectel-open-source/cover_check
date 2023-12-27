#ifndef MODBUSCLIENT_H
#define MODBUSCLIENT_H

#include <QModbusDataUnit>
#include "modbus.h"
#include "videopanel.h"
#include <QString>


class ModbusClient: public QObject
{

Q_OBJECT
public:
     ModbusClient(VideoPanel *v);
    ~ModbusClient();

     modbus *_modbus;


private  slots:

    void readReady();
    void beat();
    void send_result(QString value);

private:
    //QModbusClient被QModbusRtuSerialMaster和QModbusTcpClient继成
    QString qresult;
    int _num;

};


#endif // MODBUSCLIENT_H
