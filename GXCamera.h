/************************************************************************/
/* 以C++接口为基础，对常用函数进行二次封装，方便用户使用                */
/************************************************************************/

#ifndef _GX_CAMERA_H_
#define _GX_CAMERA_H_

#include "DaHengCamera/GxIAPI.h"
#include "DaHengCamera/DxImageProc.h"
#include <string.h>


class GXCamera
{
public:
    GXCamera();
    ~GXCamera();


    // ch:枚举设备 | en:Enumerate Device
    static int EnumDevices(uint32_t* punNumDevices, uint32_t nTimeOut);


    // ch:打开设备 | en:Open Device
    int Open(GX_OPEN_PARAM* pOpenParam, GX_DEV_HANDLE* phDevice);

    // ch:关闭设备 | en:Close Device
    int Close();


    // ch:注册图像数据回调 | en:Register Image Data CallBack
    int RegisterImageCallBack(GX_DEV_HANDLE hDevice, void *pUserParam, GXCaptureCallBack callBackFun);

    // ch:开启抓图 | en:Start Grabbing
    int StartGrabbing();

    // ch:停止抓图 | en:Stop Grabbing
    int StopGrabbing();

    int  CommandExecute(GX_FEATURE_ID_CMD featureID);
public:
    GX_DEV_HANDLE g_hDevice;

};

#endif//_MV_CAMERA_H_
