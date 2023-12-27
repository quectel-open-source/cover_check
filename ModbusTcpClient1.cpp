#include "ModbusTcpClient.h"
#include <QModbusRtuSerialMaster>
#include <QSerialPort>
#include <QModbusDataUnit>
#include <QDebug>
#include <QUrl>
#include <QModbusTcpClient>
#include  "videopanel.h"
#include <unistd.h>
#define  MODBUS_IP "192.168.2.1"  //"192.168.250.1"
#define  MODBUS_PORT 502

ModbusTcpClient::ModbusTcpClient(VideoPanel *v)
{
    modbusDevice = new QModbusTcpClient();//QModbusTcpClient用于TCP通信中的client
    if (modbusDevice->state() != QModbusDevice::ConnectedState) {//如果处在非连接状态，进行连接
        printf("ModbusTcpClient  ip=%s  port=%d\n", MODBUS_IP, MODBUS_PORT);
        printf("ModbusTcpClient start connect,line=%d \n",__LINE__);
        modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter,MODBUS_IP);
        modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, MODBUS_PORT);

        modbusDevice->setTimeout(1000);//连接超时1S
        modbusDevice->setNumberOfRetries(3);//连接失败重试三次连接
        printf("ModbusTcpClient start connect after 3,line=%d \n",__LINE__);
        if (!modbusDevice->connectDevice()) {
            printf("ModbusTcpClient start connect failed!,line=%d \n",__LINE__);
        }
        else{
            printf("ModbusTcpClient start connect success!,line=%d \n",__LINE__);
        }
    }else{//处在连接状态进行断开连接的操作
        printf("ModbusTcpClient start connect,line=%d \n",__LINE__);
        modbusDevice->disconnectDevice();
    }

    connect(modbusDevice, &QModbusClient::stateChanged,
            this, &ModbusTcpClient::onStateChanged);//连接状态发生改变时改变connect按钮上的显示文字（connect or discennect）

    connect(v, &VideoPanel::modbussend,
            this, &ModbusTcpClient::on_send);

    connect(v, &VideoPanel::modbusbeat,
            this, &ModbusTcpClient::beat);


    connect(v, &VideoPanel::modbussend_result,
            this, &ModbusTcpClient::send_result);
}

ModbusTcpClient::~ModbusTcpClient()
{
    if (modbusDevice)
        modbusDevice->disconnectDevice();
    delete modbusDevice;

}
int mmflag=1;
void ModbusTcpClient::on_send(int addr,QString sendstr)
{

    printf("ModbusTcpClient start connect,line=%d \n",__LINE__);


    if (!modbusDevice)//如果设备没有被创建就返回
        return;
    printf("ModbusTcpClient start connect,line=%d \n",__LINE__);
    QString strDis,str1;

    str1=sendstr;

    QByteArray str2 = QByteArray::fromHex (str1.toLatin1().data());//按十六进制编码接入文本
    //qDebug()<<str2;
    QString str3 = str2.toHex().data();//以十六进制显示

    //   statusBar()->clearMessage();//清除状态栏显示的信息
    QModbusDataUnit writeUnit(QModbusDataUnit::HoldingRegisters,addr,10);//发送的数据信息（数据类型 ，起始地址，个数）
    //    QModbusDataUnit::RegisterType table = writeUnit.registerType();//得到寄存器类型传给table
    for (uint i = 0; i < writeUnit.valueCount(); i++) {
        int j = 2*i;
        QString st = str3.mid (j,2);
        bool ok;
        int hex =st.toInt(&ok,16);
        writeUnit.setValue(i,hex);//设置发送数据为123456789
    }
    //   auto *reply = modbusDevice->sendWriteRequest(writeUnit, 1);//发送数据 1是server address
    modbusDevice->sendWriteRequest(writeUnit, 1);
    //    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, 1)) {
    //        if (!reply->isFinished()) {   //reply Returns true when the reply has finished or was aborted.
    //            connect(reply, &QModbusReply::finished, this, [this, reply]() {
    //                if (reply->error() == QModbusDevice::ProtocolError) {
    //                    statusBar()->showMessage(tr("Write response error: %1 (Mobus exception: 0x%2)")
    //                        .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16),
    //                        5000);
    //                } else if (reply->error() != QModbusDevice::NoError) {
    //                    statusBar()->showMessage(tr("Write response error: %1 (code: 0x%2)").
    //                        arg(reply->errorString()).arg(reply->error(), -1, 16), 5000);
    //                }
    //                reply->deleteLater();
    //            });
    //        } else {
    //            // broadcast replies return immediately
    //            reply->deleteLater();
    //        }
    //    } else {
    //     //   statusBar()->showMessage(tr("Write error: ") + modbusDevice->errorString(), 5000);
    //    }
}


void ModbusTcpClient::send_result(QString value)
{
    // addr: 102  value :  ok  01 , ng 02 (100,00 00 01 or  00 00 02 )
    printf("ModbusTcpClient start connect,line=%d \n",__LINE__);

    qresult=value;
    if (!modbusDevice)//如果设备没有被创建就返回
        return;
    printf("ModbusTcpClient start connect,line=%d \n",__LINE__);
    QString strDis,str1;



retry:
    QModbusDataUnit readUnit(QModbusDataUnit::HoldingRegisters,400,4);
    if (auto *reply = modbusDevice->sendReadRequest(readUnit, 1)) {
        if (!reply->isFinished())
        {
            connect(reply, &QModbusReply::finished, this, &ModbusTcpClient::readReady);

        }
        //    connect(reply, &QModbusReply::finished, this, &ModbusTcpClient::readReady);
        else
            delete reply; // broadcast replies return immediately
    } else {
        printf("modbus read failed \n");
        // statusBar()->showMessage(tr("Read error: ") + modbusDevice->errorString(), 5000);
    }
    return;

    QByteArray str2 = QByteArray::fromHex (str1.toLatin1().data());//按十六进制编码接入文本
    //qDebug()<<str2;
    QString str3 = str2.toHex().data();//以十六进制显示

    //   statusBar()->clearMessage();//清除状态栏显示的信息
    QModbusDataUnit writeUnit(QModbusDataUnit::HoldingRegisters,400,10);//发送的数据信息（数据类型 ，起始地址，个数）
    //    QModbusDataUnit::RegisterType table = writeUnit.registerType();//得到寄存器类型传给table
    for (uint i = 0; i < writeUnit.valueCount(); i++) {
        int j = 2*i;
        QString st = str3.mid (j,2);
        bool ok;
        int hex =st.toInt(&ok,16);
        qDebug()<<hex;
        printf("ModbusTcpClient start connect,hex=%d,line=%d \n",hex,__LINE__);

        writeUnit.setValue(i,hex);

    }

    modbusDevice->sendWriteRequest(writeUnit, 1);

}

void ModbusTcpClient::readReady()//在这里读的数据
{
    QString Qvalue;
    auto reply = qobject_cast<QModbusReply *>(sender());//QModbusReply这个类存储了来自client的数据,sender()返回发送信号的对象的指针
    if (!reply)
        return;
    //数据从QModbusReply这个类的resuil方法中获取,也就是本程序中的reply->result()
    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        for (uint i = 0; i < unit.valueCount(); i++) {
            const QString entry = tr("Address: %1, Value: %2,i=%3").arg(unit.startAddress())
                    .arg(QString::number(unit.value(i),
                                         unit.registerType() <= QModbusDataUnit::Coils ? 10 : 16).arg(i));
            //            ui->readValue->addItem(entry);//把数据放在QListWidget中
            if( i==0)
            {
                Qvalue.append("0");
                Qvalue.append((QString::number(unit.value(i),unit.registerType() <= QModbusDataUnit::Coils ? 10 : 16)));

            }
            if(i==1)
            {
                if( (QString::number(unit.value(i),unit.registerType() <= QModbusDataUnit::Coils ? 10 : 16)) == "0")
                {
                    //    sleep(1);
                    usleep(200000);

                    send_result(qresult);
                }else if ( (QString::number(unit.value(i),unit.registerType() <= QModbusDataUnit::Coils ? 10 : 16)) == "1"  )
                {
                    Qvalue.append("0");
                    Qvalue.append("0");
                    Qvalue.append("0");
                    Qvalue.append(qresult);
                    qDebug()  << "  send";
                    qDebug()  <<Qvalue;
                    on_send(400,Qvalue);
                }
            }

        }
    } else if (reply->error() == QModbusDevice::ProtocolError) {

    } else {

    }

    reply->deleteLater();
}

void ModbusTcpClient::beat()
{

    if (!modbusDevice)//如果设备没有被创建就返回
        return;

    QString strDis,str1;
    if(mmflag==1)
    {
        str1 ="1";
        mmflag =0;}else
    {
        str1 ="0";
        mmflag =1;}
    QByteArray str2 = QByteArray::fromHex (str1.toLatin1().data());//按十六进制编码接入文本
    QString str3 = str2.toHex().data();//以十六进制显示

    QModbusDataUnit writeUnit(QModbusDataUnit::HoldingRegisters,400,2);//发送的数据信息（数据类型 ，起始地址，个数）
    //    QModbusDataUnit::RegisterType table = writeUnit.registerType();//得到寄存器类型传给table
    for (uint i = 0; i < writeUnit.valueCount(); i++) {
        int j = 2*i;
        QString st = str3.mid (j,2);
        bool ok;
        int hex =st.toInt(&ok,16);
        writeUnit.setValue(i,hex);//设置发送数据为123456789
    }
    //   auto *reply = modbusDevice->sendWriteRequest(writeUnit, 1);//发送数据 1是server address
    modbusDevice->sendWriteRequest(writeUnit, 1);

    //    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, 1)) {
    //        if (!reply->isFinished()) {   //reply Returns true when the reply has finished or was aborted.
    //            connect(reply, &QModbusReply::finished, this, [this, reply]() {
    //                if (reply->error() == QModbusDevice::ProtocolError) {
    //                    statusBar()->showMessage(tr("Write response error: %1 (Mobus exception: 0x%2)")
    //                        .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16),
    //                        5000);
    //                } else if (reply->error() != QModbusDevice::NoError) {
    //                    statusBar()->showMessage(tr("Write response error: %1 (code: 0x%2)").
    //                        arg(reply->errorString()).arg(reply->error(), -1, 16), 5000);
    //                }
    //                reply->deleteLater();
    //            });
    //        } else {
    //            // broadcast replies return immediately
    //            reply->deleteLater();
    //        }
    //    } else {
    //     //   statusBar()->showMessage(tr("Write error: ") + modbusDevice->errorString(), 5000);
    //    }
}

void ModbusTcpClient::onStateChanged(int state)//更新connect按钮的显示状态
{
    printf("ModbusTcpClient start connect,state=%d,line=%d \n",state,__LINE__);
    if(state== 2)
    {
        //   on_send(100);
    }
    //    if (state == QModbusDevice::UnconnectedState)
    //        ui->connectButton->setText(tr("Connect"));
    //    else if (state == QModbusDevice::ConnectedState)
    //        ui->connectButton->setText(tr("Disconnect"));
}

void ModbusTcpClient::on_read()
{
    if (!modbusDevice)
        return;
    QModbusDataUnit readUnit(QModbusDataUnit::HoldingRegisters,0,10);
    if (auto *reply = modbusDevice->sendReadRequest(readUnit, 1)) {
        if (!reply->isFinished())
        {
            for (uint i = 0; i < readUnit.valueCount(); i++) {
                //                const QString entry = tr("Address: %1, Value: %2").arg(readUnit.startAddress())
                //                                         .arg(QString::number(readUnit.value(i),
                //                                              readUnit.registerType() <= QModbusDataUnit::Coils ? 10 : 16));

                printf("modbus read  \n",readUnit.value(i));
            }
        }
        //    connect(reply, &QModbusReply::finished, this, &ModbusTcpClient::readReady);
        else
            delete reply; // broadcast replies return immediately
    } else {
        printf("modbus read failed \n");

    }
}


