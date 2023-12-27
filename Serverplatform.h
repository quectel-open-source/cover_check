#ifndef _SERVER_CONNECT_H__
#include <json/config.h>
#include <json/json.h>
#include "SettingConfig.h"
#include "videopanel.h"
#include "QObject"
#pragma pack (1)
enum {
    PMSG_ID_NONE = -1,
    PMSG_ID_UDP_SERVER,                  //UDP 广播报文
    PMSG_ID_REPLY_COMMON     = 100,      //通用应答
    PMSG_ID_DEVICE_REGISTER  = 200,      //设备注册
    PMSG_ID_HEART_BEAT,                  //心跳
    PMSG_ID_DEVICE_REBOOT,               //设备重启
    PMSG_ID_CONFIG_REPORT    = 300,      //配置上报
    PMSG_ID_OUTPUT_CONFIG,               //串口控制输出
    PMSG_ID_STORAGE_CONFIG,              //本地存储
    PMSG_ID_FTP_CONFIG,                  //FTP配置
    PMSG_ID_DATA_REPORT,                 //检测图片发送
    PMSG_ID_MODLE_UPGRADE,               //模型升级
   PMSG_ID_BIN_UPGRADE,                 //程序升级
    PMSG_ID_OUTPUT_REPORT    = 400,      //检测结果上报WEB
    PMSG_ID_DEVICE_CTRL,                 //检测启动暂停
    PMSG_ID_DEVICE_COUNT_CLS,            //计数清0
};

struct PMessageBox {
    char header;
    short msgId;
    long msgNo;
    short bodyLength;
    char body[1024];
};

class Serverplatform: public QObject
{
 //   Q_OBJECT;
public:
    pthread_t udpThread;
    SettingConfig *settingConfig;
    Serverplatform(SettingConfig *settingConfig);
    static void *udpServer(void *params);
    int ptcpConnectState;

   CamDevice  scamdev[10];
    int     ptcpServerCreate(void);
    int     ptcpServereDestroy(void);
    int     ptcpServerRestart(void);

    int     pserverFrameProcCreate(void);
    int     pserverFrameProcStop(void);
    void    pframeProcess(PMessageBoxData *message);
    int     ptcpServerThreadCreate(void);
    int     pfd;
    int      senddata(void *user,char *data, int size);
    int     pclientLogin(void);
    int     pclientConfigPush(void);
    int     pclientHeadBeat(void);
    int     pclientDetectResult(DetectResult *result);

};
#endif
