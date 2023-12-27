#ifndef __SETTING_CONFIG_H__
#define __SETTING_CONFIG_H__
#include <json/config.h>
#include <json/json.h>
#define CONFIG_JSON "/tmp/config.json"
#define DUAL_CAMERA 1

struct ServerConfig {
    char server_ip[32];
    int port;
};

struct ModelSetting {
    char model_url[256];
    char *model_name[64];
    char *model_version[32];
    char bin_url[256];
    char bin_name[64];
    char *bin_version[32];
};
struct LocalStorage {
    char storage_dir[128];
    int storage_days;
    int storage_raw;
};

struct FtpStorage {
    char dest_ip[64]; //目标IP
    int dest_port; //目标端口
    char name[32];  //用户名 
    char password[32]; //密码
    int strategy;   //
    int enable;
};

struct DataOutput {
    char dest_ip[64];
    int  dest_port;
    int try_times;
    int enable;
};

struct UartConfig {
    char uart_name[32];
    int speed;
    int bits;
    char parity;
    int stop;
};

#define CHANNEL_NUM 2
//0 表示 暂停，1表示启动
struct DetectCtrl{
    int run_state[CHANNEL_NUM];
    int cls_state[CHANNEL_NUM];
};

#pragma pack (1)
//临时添加，后面删除
typedef struct __MessageBoxHead__
{
    char	header;
    short	int msgId;
    long	int msgNo;
    short	int bodyLength;
}MessageBoxHead;

typedef struct ____MessageBox__
{
    MessageBoxHead	head;
    char			*body;
}MessageBoxData;

typedef struct __PMessageBoxHead__
{
    char	header;
    short	int msgId;
    long	int msgNo;
    short	int bodyLength;
}PMessageBoxHead;

typedef struct ____PMessageBox__
{
    PMessageBoxHead	head;
	char			*body;
}PMessageBoxData;

typedef struct _DetectResult_
{
    char    channelNo;
    char*   rawImgPath;
    char*   markImgPath;
    char*   defectTypes;
    char*   defectDesc;
    long    snapTimeMillis;
    int     algorithmTimeConsuming;
    int     checkTotalNumber;
    int     checkNgNumber;
    double  yieldRate;
} DetectResult;

class SettingConfig {
public:
    char *deviceStr;
    char localIpaddr[64];
    struct ModelSetting modelSetting;
    struct LocalStorage localStorage;
    struct FtpStorage ftpStorage;
    struct DataOutput dataOutput;
    struct UartConfig uartConfig;
    struct ServerConfig serverConfig;
    struct DetectCtrl   detectctrl;
    SettingConfig();

    void setModelSetting(ModelSetting modelSetting);
    void setLocalStorage(LocalStorage localStorage);
    void setFtpStorage(FtpStorage ftpStorage);
    void setDataOutput(DataOutput dataOutput);
    void setUartConfig(UartConfig uartConfig);
    bool updateConfigJson(Json::Value value);
    bool updateConfigJson(struct MessageBox box);
    bool parserLocalConfigValue(Json::Value value);
    void dumpConfig();
    bool updateDetectCtrl(struct MessageBox box);
    bool updateDetectCls(struct MessageBox box);
};
#endif
