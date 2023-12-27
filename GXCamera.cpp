#include "GXCamera.h"

GXCamera::GXCamera()
{
g_hDevice= NULL;

}

GXCamera::~GXCamera()
{
   // if (m_hDevHandle)
    {
 //       MV_CC_DestroyHandle(m_hDevHandle);
 //       m_hDevHandle    = MV_NULL;
    }
}



// ch:枚举设备 | en:Enumerate Device
int GXCamera::EnumDevices(uint32_t* punNumDevices, uint32_t nTimeOut)
{
    return GXUpdateAllDeviceList(punNumDevices, nTimeOut);
}


// ch:打开设备 | en:Open Device
int GXCamera::Open(GX_OPEN_PARAM* pOpenParam, GX_DEV_HANDLE* phDevice)
{
    int ret=GXOpenDevice(pOpenParam, phDevice);
   // g_hDevice = phDevice;
    memcpy(&g_hDevice,phDevice,sizeof (GX_DEV_HANDLE));
    return  ret;
}

// ch:关闭设备 | en:Close Device
int GXCamera::Close()
{


    return  GXCloseLib();
}


// ch:注册图像数据回调 | en:Register Image Data CallBack
int GXCamera::RegisterImageCallBack (GX_DEV_HANDLE hDevice, void *pUserParam, GXCaptureCallBack callBackFun)
{
   // return MV_CC_RegisterImageCallBackForBGR(m_hDevHandle, cbOutput, pUser);
     return GXRegisterCaptureCallback(hDevice,(void *)pUserParam,(GXCaptureCallBack)callBackFun);;
}

// ch:开启抓图 | en:Start Grabbing
int GXCamera::StartGrabbing()
{
    return GXStreamOn(g_hDevice);
}

// ch:停止抓图 | en:Stop Grabbing
int GXCamera::StopGrabbing()
{
    return GXStreamOff(g_hDevice);
}


int GXCamera::CommandExecute(GX_FEATURE_ID_CMD featureID)
{
    return GXSendCommand(g_hDevice, featureID);
}
