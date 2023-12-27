#include "modbusClient.h"
#include "videopanel.h"
#include<iostream>
#include <stdio.h>

#define  MODBUS_IP "192.168.2.1"  //"192.168.250.1"
#define  MODBUS_PORT 502

using namespace std;

ModbusClient::ModbusClient(VideoPanel *v)
{
    printf("[ModbusTcpClient] ip=%s  port=%d\n", MODBUS_IP, MODBUS_PORT);
    // create a modbus object
    _modbus = new modbus(MODBUS_IP, MODBUS_PORT);
    // set slave id
    _modbus->modbus_set_slave_id(1);
    printf("[ModbusTcpClient] connect start\n");
    // connect with the server
    if (_modbus->modbus_connect())
    {
        printf("[ModbusTcpClient] connect success!\n");
    }
    else{
        printf("[ModbusTcpClient] connect failed!\n");
    }

    connect(v, &VideoPanel::modbusbeat, this, &ModbusClient::beat);
    connect(v, &VideoPanel::modbussend_result, this, &ModbusClient::send_result);
}



void ModbusClient::beat()
{
    if (_num == 0)
    {
        _modbus->modbus_write_register(0x01,0);
        printf("[ModbusTcpClient] beat! ---- %x", 0x01);
        _num = 1;
    }
    else
    {
        _modbus->modbus_write_register(0x01,1);
        printf("[ModbusTcpClient] beat! ---- %x", 0x01);
        _num = 0;
    }

}

void ModbusClient::send_result(QString value)
{
    // addr: 102  value :  ok  01 , ng 02 (100,00 00 01 or  00 00 02 )
    printf("[ModbusTcpClient] receive result:%s\n", value.toInt());
    _modbus->modbus_write_register(0x01, value.toInt());

}

void ModbusClient::readReady()//在这里读的数据
{
    uint16_t read_input_regs[1];
    _modbus->modbus_read_holding_registers(0x01, 1, read_input_regs);
    printf("[ModbusTcpClient] read:%d", read_input_regs[0]);
}






