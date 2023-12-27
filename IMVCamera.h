/************************************************************************/
/* 以C++接口为基础，对常用函数进行二次封装，方便用户使用                */
/************************************************************************/

#ifndef _IMV_CAMERA_H_
#define _IMV_CAMERA_H_

#include "IMVApi.h"

#include <string.h>


#ifndef IMV_NULL
#define IMV_NULL    0
#endif
class IMVCamera
{
public:
    IMVCamera();
    ~IMVCamera();


    // ch:枚举设备 | en:Enumerate Device
     int EnumDevices(IMV_DeviceList *pDeviceList, unsigned int interfaceType);

    int saveConfig(char *specifiedStr);

    // ch:打开设备 | en:Open Device
    int Open(char *specifiedStr,int softTrigger,char *cfg);

    // ch:关闭设备 | en:Close Device
    int Close();


    // ch:注册图像数据回调 | en:Register Image Data CallBack
    int RegisterImageCallBack(IMV_FrameCallBack proc, void* pUser);

    // ch:开启抓图 | en:Start Grabbing
    int StartGrabbing();

    // ch:停止抓图 | en:Stop Grabbing
    int StopGrabbing();
    int CommandExecute(const char* strKey);
    int CommandExcuteLine(const char* strKey);
    int SetIO(int value);
private:
    void*               m_hDevHandle;

};

#endif//_MV_CAMERA_H_
