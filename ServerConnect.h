#ifndef _SERVER_CONNECT_H__
#include <json/config.h>
#include <json/json.h>
#include "SettingConfig.h"
#include <QMainWindow>
//#include <QModbusDataUnit>
//#include <QModbusClient>
#pragma pack (1)
enum {
    MSG_ID_NONE = -1,
    MSG_ID_UDP_SERVER,                  //UDP 广播报文
    MSG_ID_REPLY_COMMON     = 100,      //通用应答
    MSG_ID_DEVICE_REGISTER  = 200,      //设备注册
    MSG_ID_HEART_BEAT,                  //心跳
    MSG_ID_DEVICE_REBOOT,               //设备重启
    MSG_ID_CONFIG_REPORT    = 300,      //配置上报
    MSG_ID_OUTPUT_CONFIG,               //串口控制输出
    MSG_ID_STORAGE_CONFIG,              //本地存储
    MSG_ID_FTP_CONFIG,                  //FTP配置
    MSG_ID_DATA_REPORT,                 //检测图片发送
    MSG_ID_MODLE_UPGRADE,               //模型升级
    MSG_ID_BIN_UPGRADE,                 //程序升级
    MSG_ID_OUTPUT_REPORT    = 400,      //检测结果上报WEB
    MSG_ID_DEVICE_CTRL,                 //检测启动暂停
    MSG_ID_DEVICE_COUNT_CLS,            //计数清0
};

struct MessageBox {
    char header;
    short msgId;
    long msgNo;
    short bodyLength;
    char body[1024];
};

class ServerConnect {
public:
    pthread_t udpThread;
    SettingConfig *settingConfig;
    ServerConnect(SettingConfig *settingConfig);
    static void *udpServer(void *params);
    int tcpConnectState;

   
    int     tcpServerCreate(void);
    int     tcpServereDestroy(void);
    int     tcpServerRestart(void);

    int     serverFrameProcCreate(void);
    int     serverFrameProcStop(void);    
    void    frameProcess(MessageBoxData *message);
    int     tcpServerThreadCreate(void);
    int     fd;

    int     clientLogin(void);
    int     clientConfigPush(void);
    int     clientHeadBeat(void);
    int     clientDetectResult(DetectResult *result);
};
#endif
