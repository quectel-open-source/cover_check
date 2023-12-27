#include "ModbusTcpClient.h"
#include "videopanel.h"
#include<iostream>
#include <stdio.h>
using namespace std;

#define  MODBUS_IP "192.168.1.232"  //"192.168.250.1"
#define  MODBUS_PORT 10000


using namespace std;

int mbStatus=0;
typedef struct mbPort{
    int port;
    int life;
}mp;
vector<mp> mbPortList;

ModbusTCPThread::ModbusTCPThread(QObject *parent) : QThread(parent)
{


}




void ModbusTCPThread::run()
{
//    int ret=0;

//    while(1){
//        ret=_modbus->read_gpio();
//        if(ret==1){
//            emit getAlready();
//        }
//        msleep(_modbus->waitTime);
//    }

}



ModbusTcpClient::ModbusTcpClient(VideoPanel *v)
{
    //printf("[ModbusTcpClient] ip=%s  port=%d\n", MODBUS_IP, MODBUS_PORT);
    // create a modbus object
    //_modbus = new modbus(MODBUS_IP, MODBUS_PORT);
    // set slave id
//    _modbus->modbus_set_slave_id(1);
//    printf("[ModbusTcpClient] connect start\n");

//    // connect with the server
//    if (_modbus->modbus_connect())
//    {
//        printf("[ModbusTcpClient] connect success!\n");
//    }
//    else{
//        printf("[ModbusTcpClient] connect failed!\n");
//    }

    //connect(this, &ModbusTcpClient::startTrigger, v, &VideoPanel::pollHardware);
    //connect(this, &ModbusTcpClient::sendPLCstate, v, &VideoPanel::modbusRecvPLCstate);

    // start thread
//    ModbusTCPThread* modbusThread = new ModbusTCPThread(this);
//    modbusThread->_modbus=_modbus;
//    //connect(modbusThread, &ModbusTCPThread::getAlready, v, &VideoPanel::operAlready);
//    printf("[ModbusTCPThread] ModbusTCPThread start running!\n");
//    modbusThread->start();
//    connect(v, &VideoPanel::modbussend_result, this, &ModbusTcpClient::send_result);
}

ModbusTcpClient::~ModbusTcpClient()
{
    delete _modbus;
}

// send result ---- addr:x03  value:  OK  1, NG  2,  null  3
void ModbusTcpClient::send_result(int valueResult)
{
    //printf("[ModbusTcpClient] 0x03  send result:%d\n", valueResult);
    //_modbus->modbus_write_register(0x03, valueResult);

    _modbus->modbus_write_io(0,0);
    _modbus->modbus_write_io(1,0);
    //_modbus->modbus_read_io();

    //uint16_t read_input_regs[1];
    //_modbus->modbus_read_holding_registers(0x03, 1, read_input_regs);
    //printf("[ModbusTcpClient] 0x03  recv result:%d\n", read_input_regs[0]);
}

// send alarm ---- addr:x04  value:  zhengchang  1, yichang  2(unload etc..)
void ModbusTcpClient::send_alarm(int valueAlarm)
{
    printf("[ModbusTcpClient] 0x04  send result modbus:%d\n", valueAlarm);
    _modbus->modbus_write_register(0x04, valueAlarm);

}

// recv No.5 Station modbus  ---- addr:x02  value:  weidaowei  1, daowei  2
void ModbusTcpClient::read_recv()
{
    _modbus->modbus_read_holding_registers(0x02, 1, _numRecvRegs);
    printf("[ModbusTcpClient] recv:%d", _numRecvRegs[0]);
    if (_numRecvRegs[0] == 2)
    {
        _numRecvRegs[0] = 1;
        emit startTrigger(2);
    }
}







