#include "IMVCamera.h"
#include <unistd.h>
#include<stdio.h>
IMVCamera::IMVCamera()
{
    m_hDevHandle = IMV_NULL;
}

IMVCamera::~IMVCamera()
{
    if (m_hDevHandle)
    {
        IMV_DestroyHandle(m_hDevHandle);
        m_hDevHandle    = IMV_NULL;
    }
}

// ch:获取SDK版本号 | en:Get SDK Version
//int IMVCamera::GetSDKVersion()
//{
//    return MV_CC_GetSDKVersion();
//}

// ch:枚举设备 | en:Enumerate Device
int IMVCamera::EnumDevices(IMV_DeviceList *pDeviceList, unsigned int interfaceType)
{
    return IMV_EnumDevices(pDeviceList, interfaceType);
}


//// ch:判断设备是否可达 | en:Is the device accessible
//bool IMVCamera::IsDeviceAccessible(MV_CC_DEVICE_INFO* pstDevInfo, unsigned int nAccessMode)
//{
//    return MV_CC_IsDeviceAccessible(pstDevInfo, nAccessMode);
//}

static int setLineTriggerConf(IMV_HANDLE devHandle)
{
    int ret = IMV_OK;

    // 设置触发源为外部触发
    // Set trigger source to Line1
    ret = IMV_SetEnumFeatureSymbol(devHandle, "TriggerSource", "Line2");
    if (IMV_OK != ret)
    {
        printf("Set triggerSource value failed! ErrorCode[%d]\n", ret);
        return ret;
    }

    ret = IMV_SetEnumFeatureSymbol(devHandle, "AcquisitionMode", "Continuous");
    if (IMV_OK != ret)
    {
        printf("Set triggerSelector value failed! ErrorCode[%d]\n", ret);
        return ret;
    }

    // 设置触发器
    // Set trigger selector to FrameStart
    ret = IMV_SetEnumFeatureSymbol(devHandle, "TriggerSelector", "LineStart");
    if (IMV_OK != ret)
    {
        printf("Set triggerSelector value failed! ErrorCode[%d]\n", ret);
        return ret;
    }

    // 设置触发模式
    // Set trigger mode to On
    ret = IMV_SetEnumFeatureSymbol(devHandle, "TriggerMode", "On");
    if (IMV_OK != ret)
    {
        printf("Set triggerMode value failed! ErrorCode[%d]\n", ret);
        return ret;
    }

    // 设置外触发为上升沿（下降沿为FallingEdge）
    // Set trigger activation to RisingEdge(FallingEdge in opposite)
    ret = IMV_SetEnumFeatureSymbol(devHandle, "TriggerActivation", "RisingEdge");
    if (IMV_OK != ret)
    {
        printf("Set triggerActivation value failed! ErrorCode[%d]\n", ret);
        return ret;
    }

    return ret;
}


static int setSoftTriggerConf(IMV_HANDLE devHandle,int softTrigger)
{
        int ret = IMV_OK;

        // 设置触发源为软触发
        // Set trigger source to Software
        if(softTrigger == 1)
        {
        ret = IMV_SetEnumFeatureSymbol(devHandle, "TriggerSource", "Software");
        if (IMV_OK != ret)
        {
                printf("Set triggerSource value failed! ErrorCode[%d]\n", ret);
                return ret;
        }
}else{
            ret = IMV_SetEnumFeatureSymbol(devHandle, "TriggerSource", "Line1");
            if (IMV_OK != ret)
            {
                    printf("Set triggerSource value failed! ErrorCode[%d]\n", ret);
                    return ret;
            }
        }
        // 设置触发器
        // Set trigger selector to FrameStart
        ret = IMV_SetEnumFeatureSymbol(devHandle, "TriggerSelector", "FrameStart");
        if (IMV_OK != ret)
        {
                printf("Set triggerSelector value failed! ErrorCode[%d]\n", ret);
                return ret;
        }
        // 设置触发模式
        // Set trigger mode to On
        ret = IMV_SetEnumFeatureSymbol(devHandle, "TriggerMode", "On");
        if (IMV_OK != ret)
        {
                printf("Set triggerMode value failed! ErrorCode[%d]\n", ret);
                return ret;
        }

        return ret;
}


// ch:打开设备 | en:Open Device
int IMVCamera::saveConfig(char *cfg)
{
    int nRet;
    nRet = IMV_SaveDeviceCfg(m_hDevHandle, cfg);
    if (IMV_OK != nRet)
    {
         printf("Save camera configuration fail! ErrorCode[%d]\n", nRet);
         return nRet;
    }

    nRet = IMV_ClearFrameBuffer(m_hDevHandle);
    if (IMV_OK != nRet)
    {
        printf("Clear Frame! ErrorCode[%d]\n", nRet);
        return nRet;
    }

    printf("fun=%s,line=%d,ret=%d,m_hDevHandle=%d\n",__func__,__LINE__,nRet,m_hDevHandle);
    return nRet;
}

// ch:打开设备 | en:Open Device
int IMVCamera::Open(char *specifiedStr,int softTrigger,char *cfg)
{

    if (IMV_NULL == specifiedStr)
    {

        return IMV_ERROR;
    }


    int nRet  = IMV_CreateHandle(&m_hDevHandle,  modeByIPAddress, specifiedStr);
    if (IMV_OK != nRet)
    {
        printf("Dahua inner create fail!");
        return nRet;
    }

    nRet = IMV_Open(m_hDevHandle);
    if (IMV_OK != nRet)
    {
        printf("Dahua inner open fail!");
        IMV_DestroyHandle(m_hDevHandle);
        m_hDevHandle = IMV_NULL;
        return nRet;
    }

    //Don't change
//    nRet = setLineTriggerConf(m_hDevHandle);
//    if (IMV_OK != nRet)
//    {
//        printf("set trigger line fail!");
//        IMV_DestroyHandle(m_hDevHandle);
//        m_hDevHandle = IMV_NULL;
//        return nRet;
//    }

    IMV_ErrorList errorList;
    memset(&errorList, 0, sizeof(IMV_ErrorList));
    nRet =IMV_LoadDeviceCfg(m_hDevHandle,cfg,&errorList);
    if (IMV_OK != nRet)
    {
         printf("Load camera configuration fail! ErrorCode[%d]\n", nRet);
         return nRet;
    }


    printf("fun=%s,line=%d,ret=%d,m_hDevHandle=%d\n",__func__,__LINE__,nRet,m_hDevHandle);
    return nRet;
}

// ch:关闭设备 | en:Close Device
int IMVCamera::Close()
{
    if (IMV_NULL == m_hDevHandle)
    {
        return IMV_ERROR;
    }

    int nRet=IMV_Close(m_hDevHandle);
    if (IMV_OK != nRet)
    {
        printf("Close fail! ErrorCode[%d]\n", nRet);
        return nRet;
    }

    nRet = IMV_DestroyHandle(m_hDevHandle);
    if(IMV_OK != nRet){
        printf("Destroy fail! ErrorCode[%d]\n", nRet);
    }
    m_hDevHandle = IMV_NULL;
    printf("Destroy camera OK\n");
    return nRet;
}

//// ch:判断相机是否处于连接状态 | en:Is The Device Connected
//bool IMVCamera::IsDeviceConnected()
//{
//    return MV_CC_IsDeviceConnected(m_hDevHandle);
//}


//IN IMV_HANDLE handle, IN IMV_FrameCallBack proc, IN void* pUser);
// ch:注册图像数据回调 | en:Register Image Data CallBack
int IMVCamera::RegisterImageCallBack(IMV_FrameCallBack proc, void* pUser)
{
   printf("register m_hDevHandle=%d\n",m_hDevHandle);
   return IMV_AttachGrabbing(m_hDevHandle,(IMV_FrameCallBack) proc, pUser);
}

// ch:开启抓图 | en:Start Grabbing
int IMVCamera::StartGrabbing()
{

    return IMV_StartGrabbing(m_hDevHandle);
}

// ch:停止抓图 | en:Stop Grabbing
int IMVCamera::StopGrabbing()
{
    return IMV_StopGrabbing(m_hDevHandle);
}

int IMVCamera::SetIO(int value)
{
    int ret = IMV_OK;
    // 获取属性值
    ret = IMV_SetIntFeatureValue(m_hDevHandle, "UserOutputValueAll", value);
    if (IMV_OK != ret)
    {
         printf("[IMVCamera::SetIO] failed! ErrorCode[%d]\n", ret);
         return ret;
    }else{
         printf("[IMVCamera::SetIO] set value%d\n",value);
    }
}


int IMVCamera::CommandExecute(const char* strKey)
{

    return IMV_ExecuteCommandFeature(m_hDevHandle, strKey);
}

int IMVCamera::CommandExcuteLine(const char* strKey)
{

    return IMV_SetEnumFeatureSymbol(m_hDevHandle, "TriggerSource", strKey);
}
