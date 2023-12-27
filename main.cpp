#include "frmmain.h"
#include "appinit.h"
#include "quihelper.h"
#include "qsqldatabase.h"
#include "qsqlquery.h"
#include "videobox.h"
#include "DaHengCamera/GxIAPI.h"
#include "DaHengCamera/DxImageProc.h"
#include "MvCamera.h"
#include <unistd.h>
#include <QTcpSocket>
//#include <cpuid.h>
#include <sstream>

//#define IP    "10.88.207.20"
#define IP "10.88.211.121"
//#define IP "10.88.110.212"
#define SERVER_IP_ADDRESS   "127.0.0.1"
#define SERVER_PORT 8818

#include<iostream>
#include<string>
#include "UartControl.hpp"
#include <semaphore.h>
#include "Util.hpp"
#include "SettingConfig.h"
#include "FtpUtil.h"
#include "ServerConnect.h"
#include "Serverplatform.h"
#include "ModbusTcpClient.h"

#include "MvCameraControl.h"

//Show error message, close device and lib
#define GX_VERIFY_EXIT(emStatus) \
    if (emStatus != GX_STATUS_SUCCESS)     \
{                                      \
    GetErrorString(emStatus);          \
    GXCloseLib();                      \
    printf("<App Exit!>\n");           \
    return emStatus;                   \
    }


const int               FAILURE = 1;
const int               SUCCESS = 0;

bool debug = false;
long captureCount_ch1 = 0;
long ngCount_ch1 = 0;
long okCount_ch1 =0 ;

long captureCount_ch2 = 0;
long ngCount_ch2 = 0;
long okCount_ch2 = 0;




GX_DEV_HANDLE g_hDevice = NULL;
//daheng camera1
GX_DEV_HANDLE   g_hDevice1 = NULL;
GX_DEV_HANDLE   g_hDevice2 = NULL;
GX_STATUS               emStatus = GX_STATUS_SUCCESS;

QList <int>       g_servertCmdList;
QList <char *>       g_servertCmdLineList;
//list change to QList

struct publicParam {
    int serverFd;
};





//Get description of error
#define ACQ_BUFFER_NUM                  5
#define ACQ_TRANSFER_SIZE               (64 * 1024)     ///< Size of data transfer block
#define ACQ_TRANSFER_NUMBER_URB 64                              ///< Qty. of data transfer block
#define FILE_NAME_LEN                   128                      ///< Save image file name length
void GetErrorString(GX_STATUS);
void *RemovePics(void* pParam);


void* setdata(void* pParam);
int getCameraHandle(int id);

QTcpSocket *qlsocket;
char currentCameraId;

struct QuectelSummary
{
    SettingConfig *settingConfig;
    FtpUtil *ftpUtil;
    ServerConnect *serverConnect;
} quectelSummary;

#include <iostream>
#include <signal.h>
#include <execinfo.h>

#define DBG_ASSERT(x) do { \
    if (x) { break; } \
    std::cout << "###### file:" <<  __FILE__ << " line:" <<  __LINE__ << " ######" << std::endl;\
    void *pptrace_raw[32] = {0}; \
    char **pptrace_str = NULL; \
    int  trace_num = 0, iloop = 0; \
    trace_num = backtrace(pptrace_raw, 32); \
    pptrace_str = (char **)backtrace_symbols(pptrace_raw, trace_num); \
    for (iloop=0; iloop<trace_num; iloop++) { std::cout << pptrace_str[iloop] << std::endl; } \
    if (pptrace_str) { delete pptrace_str; } \
} while (0);


void sigsegv_test()
{
    std::cout << __func__ << " begin" << std::endl;
    char *buff = NULL;
    buff[1] = buff[1]; /* will crash here */
    std::cout << __func__ << " end" << std::endl;
}


void sigsegvhandle(int signo) {
     std::cout << "sigsegvhandle received signal: " << signo << std::endl;
     /* output callstack */
     DBG_ASSERT(0);
     /* reset signal handle to default */
     signal(signo, SIG_DFL);
     /* will receive SIGSEGV again and exit app */
}

void stop_detect(int channel) {
    printf("%s started,channel :%d\n",__func__,channel);
    if ((channel == 1) && (g_hDevice1)) {
        GXStreamOff(g_hDevice1);
    }
    if ((channel == 2) && (g_hDevice2)) {
        GXStreamOff(g_hDevice2);
    }

    printf("%s done \n",__func__);
}
void resume_detect(int channel){
    printf("%s started,channel :%d\n",__func__,channel);
    if ((channel == 1) && (g_hDevice1)) {
        GXStreamOn(g_hDevice1);
    }
    if ((channel == 2)&& (g_hDevice2)) {
        GXStreamOn(g_hDevice2);
    }
    printf("%s done \n",__func__);
}


void GetErrorString(GX_STATUS emErrorStatus)
{
    char *                  error_info = NULL;
    size_t                  size = 0;
    GX_STATUS               emStatus = GX_STATUS_SUCCESS;

    // Get length of error description
    emStatus                        = GXGetLastError(&emErrorStatus, NULL, &size);

    if (emStatus != GX_STATUS_SUCCESS)
    {
        printf("<Error when calling GXGetLastError>\n");
        return;
    }

    // Alloc error resources
    error_info      = new char[size];

    if (error_info == NULL)
    {
        printf("<Failed to allocate memory>\n");
        return;
    }

    // Get error description
    emStatus = GXGetLastError(&emErrorStatus, error_info, &size);

    if (emStatus != GX_STATUS_SUCCESS)
    {
        printf("<Error when calling GXGetLastError>\n");
    }
    else
    {
        printf("%s\n", (char *) error_info);
    }

    // Realease error resources
    if (error_info != NULL)
    {
        delete[] error_info;
        error_info      = NULL;
    }
}

long difftimeval(const struct timeval * start, const struct timeval * end)
{
    long                    d;
    time_t                  s;
    suseconds_t     u;

    s = end->tv_sec - start->tv_sec;
    u = end->tv_usec - start->tv_usec;
    d = s;
    d *= 1000000;
    d += u;
    return d;
}

#define CHECK_NULL_RET(value, ret)  \
    do                                                      \
{                                                       \
    if (value == NULL)              \
{                                               \
    return ret;                     \
    }                                               \
    }while(0)                                       \




int clientDetectResultPush(DetectResult *result)
{
    CHECK_NULL_RET(quectelSummary.serverConnect, -1);
    return quectelSummary.serverConnect->clientDetectResult(result);
}

int clientHeatBeatPush(void)
{
    CHECK_NULL_RET(quectelSummary.serverConnect, -1);
    return quectelSummary.serverConnect->clientHeadBeat();
}

long gettimems(const struct timeval * time)
{
    long    d;
    d = time->tv_usec / 1000 + time->tv_sec * 1000;
    printf("[%s]getTimes:%d\n", __func__, d);
    return d;
}

typedef struct __MsgPicHead__
{
    short	int id; //相机ID
    int msgNo; //对应相机的照片的ID
    int height;
    int width;
    int bodylength;
}MsgPicHead;

typedef struct __MessageBox__
{
    MsgPicHead	head;
    char			*body;
}PicData;

QString connectState(int state)
{
    QString str;
    switch (state) {
    case QTcpSocket::UnconnectedState:
        str = "UnconnectedState";
        break;

    case QTcpSocket::HostLookupState:
        str = "HostLookupState";
        break;

    case QTcpSocket::ConnectingState:
        str = "ConnectingState";
        break;

    case QTcpSocket::ConnectedState:
        str = "ConnectedState";
        break;

    case QTcpSocket::BoundState:
        str = "BoundState";
        break;

    case QTcpSocket::ListeningState:
        str = "ListeningState";
        break;

    case QTcpSocket::ClosingState:
        str = "ClosingState";
        break;

    default:
        break;
    }

    return str;
}
Serverplatform *serverplatform;
CMvCamera  *cMvCamera;
ModbusTcpClient *modbusTcpClient;
QString s1,s2;
void captureCB(GX_FRAME_CALLBACK_PARAM *pFrameData)
{

    struct timeval startTime;
    struct timeval captureTime;
    struct timeval costTime;
    gettimeofday(&startTime,NULL);
    gettimeofday(&costTime,NULL);
    uint32_t width = pFrameData->nWidth;
    uint32_t height = pFrameData->nHeight;
    uint32_t rawSize = 0;
    char predLabel[3];
    int channelId = (int)(size_t)pFrameData->pUserParam;
    char picPath[128] = {0};
    char resultPath[128] = {0};
    char realPath[128] = {0};
    float okRate_ch1,okRate_ch2;

    char defectDesc[128] = {0};

    printf("%s: get a frame frame id %llu w*h %d*%d，channelId %d\n",__func__,pFrameData->nFrameID,width,height,channelId);
    if (channelId == 1) {
        captureCount_ch1++;
    } else if (channelId == 2) {
        captureCount_ch2++;
    }
    if (!(captureCount_ch1 % 500) || !(captureCount_ch2 % 500)) {
        pthread_t rm_id;
        int nRet = pthread_create(&rm_id, NULL, RemovePics, NULL);
        printf("second rm: %d\n",nRet);
    }

    bool bRet;
    rawSize = width * height;
    unsigned char* rgb24 = NULL;
    rgb24 = (unsigned char*)malloc(rawSize *3);
    unsigned char* jpgBuffer = NULL;

    long jpg_size;
    if(rgb24){
        memset(rgb24,0,rawSize);
        DxRaw8toRGB24((unsigned char*)pFrameData->pImgBuf,rgb24,width,height,RAW2RGB_NEIGHBOUR,BAYERRG,false);
        jpgBuffer = rgb2jpg((const char *) (char *) rgb24, width,height, 0, &jpg_size);
        printf("rgb2jpg done,jpg_size %d %p %p\n",jpg_size, jpgBuffer, &jpgBuffer);
        getCurrentTimeSec(channelId,true,picPath);
        FILE* fp = fopen("/tmp/color_20220822628.jpg", "wb");
        QDateTime dateTime = QDateTime::currentDateTime();
        // 字符串格式化
        QString timestamp = dateTime.toString("yyyy-MM-dd hh:mm:ss.zzz");
        // 获取毫秒值
        int ms = dateTime.time().msec();
        qDebug() <<"ms = "<<ms;
        if (fp)
        {

            int res = fwrite(jpgBuffer, 1, jpg_size, fp);
            fclose(fp);
            printf("%s @ line:%d,res:%d,dst_size %d\n",__func__,__LINE__,res, jpg_size);
        }
        printf("%s called @ line %d,date=%s\n",__func__,__LINE__,__TIME__);
        //  return;

        if (quectelSummary.settingConfig->ftpStorage.enable)
        {
            bRet = quectelSummary.ftpUtil->FtpUpload((char *)jpgBuffer, jpg_size, picPath);
            //      cout << "ftpUpload bRet: " << bRet << endl;
        }


#if 1

        PicData p;
        MsgPicHead h;
        p.head.id=1;
        p.head.msgNo=1;
        p.head.width=width;
        p.head.height=height;
        p.head.bodylength=rawSize *3;
        char *buf =(char *)malloc((sizeof(MsgPicHead)+rawSize *3));
        memcpy(buf,&p.head,sizeof (MsgPicHead));
        memcpy(buf+sizeof(MsgPicHead),(unsigned char*)rgb24,(rawSize *3));

#endif

#if 0
        if (qtlYOLOV5BuilderInfer) {
            qtlYOLOV5BuilderInfer->inferenceOnMatV(srcImage.data,dstData,predLabel,width,height,cv::Size(768, 768));
            cv::Mat resultDstImage(dstImage.rows, dstImage.cols, CV_8UC3, dstData);
            getCurrentTimeSec(channelId,false,resultPath);
            printf("resultPath %s\n",resultPath);
            cv::cvtColor(resultDstImage, resultDstImage, COLOR_RGB2BGR);
            cv::imwrite(resultPath, resultDstImage);
        }
#endif                                                                                                                                                                           290,1-8       39%

        free(rgb24);
        free(jpgBuffer);

        send_cmd_to_uart(predLabel);
        gettimeofday(&captureTime,NULL);
        long capt = difftimeval(&startTime,&captureTime);
        if (!strcmp(predLabel,"00"))
        {
            if (channelId == 1) {
                okCount_ch1++;
            } else if (channelId == 2) {
                okCount_ch2++;
            }
            strcpy(defectDesc,"OK");
        } else
        {
            if (channelId == 1) {
                ngCount_ch1++;
            } else if (channelId == 2) {
                ngCount_ch2++;
            }
            strcpy(defectDesc,"NG");
        }
        if (channelId == 1) {
            okRate_ch1 = okCount_ch1*1.0/captureCount_ch1;
            printf("ch1,total cost:%ld us,captureCount:%d,ngCount%d,okCount%d,okRate:%.4f\n",capt,
                   captureCount_ch1,ngCount_ch1,okCount_ch1,okCount_ch1*1.0/captureCount_ch1);
        } else if(channelId == 2) {
            okRate_ch2 = okCount_ch2*1.0/captureCount_ch2;
            printf("ch2,total cost:%ld us,captureCount:%d,ngCount%d,okCount%d,okRate:%.4f\n",capt,
                   captureCount_ch2,ngCount_ch2,okCount_ch2,okCount_ch2*1.0/captureCount_ch2);
        }

        memcpy(realPath,resultPath + 12,23);

        DetectResult result;
        result.algorithmTimeConsuming = capt / 1000;
        result.channelNo = channelId;
        result.defectDesc = defectDesc;
        result.defectTypes = predLabel;
        result.rawImgPath = realPath;
        result.markImgPath = realPath;
        result.snapTimeMillis = gettimems(&costTime);
        if (channelId == 1) {
            result.checkNgNumber = ngCount_ch1;
            result.checkTotalNumber = captureCount_ch1;
            result.yieldRate = okRate_ch1;
        } else if (channelId == 2) {
            result.checkNgNumber = ngCount_ch2;
            result.checkTotalNumber = captureCount_ch2;
            result.yieldRate = okRate_ch2;
        }
        clientDetectResultPush(&result);
    }
}


static void* WorkThread(void* pUser)
{
    int nRet = MV_OK;

    MVCC_STRINGVALUE stStringValue = {0};
    char camSerialNumber[256] = {0};
    nRet = MV_CC_GetStringValue(pUser, "DeviceSerialNumber", &stStringValue);
    if (MV_OK == nRet)
    {
        memcpy(camSerialNumber, stStringValue.chCurValue, sizeof(stStringValue.chCurValue));
    }
    else
    {
        printf("Get DeviceUserID Failed! nRet = [%x]\n", nRet);
    }

    MVCC_INTVALUE stParam;
    memset(&stParam, 0, sizeof(MVCC_INTVALUE));
    nRet = MV_CC_GetIntValue(pUser, "PayloadSize", &stParam);
    if (MV_OK != nRet)
    {
        printf("Get PayloadSize fail! nRet [0x%x]\n", nRet);
        return NULL;
    }

    MV_FRAME_OUT_INFO_EX stImageInfo = {0};
    memset(&stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));
    unsigned char * pData = (unsigned char *)malloc(sizeof(unsigned char) * stParam.nCurValue);
    if (NULL == pData)
    {
        return NULL;
    }
    unsigned int nDataSize = stParam.nCurValue;

    return 0;
}




void __stdcall ImageCallBackEx(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
{

    if (pFrameInfo)
    {
        printf("GetOneFrame, Width[%d], Height[%d], nFrameNum[%d]\n",
               pFrameInfo->nWidth, pFrameInfo->nHeight, pFrameInfo->nFrameNum);
    }

    int nRet = MV_OK;

    MVCC_STRINGVALUE stStringValue = {0};
    char camSerialNumber[256] = {0};
    nRet = MV_CC_GetStringValue(pUser, "DeviceSerialNumber", &stStringValue);
    if (MV_OK == nRet)
    {
        memcpy(camSerialNumber, stStringValue.chCurValue, sizeof(stStringValue.chCurValue));
    }
    else
    {
        printf("Get DeviceUserID Failed! nRet = [%x]\n", nRet);
    }


    MVCC_INTVALUE stParam;
    memset(&stParam, 0, sizeof(MVCC_INTVALUE));
    nRet = MV_CC_GetIntValue(pUser, "PayloadSize", &stParam);


    struct timeval startTime;
    struct timeval captureTime;
    struct timeval costTime;
    gettimeofday(&startTime,NULL);
    gettimeofday(&costTime,NULL);
    uint32_t width = 	pFrameInfo->nWidth;
    uint32_t height =  pFrameInfo->nHeight;
    uint32_t rawSize = 0;
    char predLabel[3];
    //    int channelId = (int)(size_t)pFrameData->pUserParam;
    char picPath[128] = {0};
    char resultPath[128] = {0};
    char realPath[128] = {0};
    float okRate_ch1,okRate_ch2;
    char defectDesc[128] = {0};
    bool bRet;
    rawSize = width * height;
    unsigned char* rgb24 = NULL;
    rgb24 = (unsigned char*)malloc(rawSize *3);
    unsigned char* jpgBuffer = NULL;
    long jpg_size;
    if(rgb24){
        memset(rgb24,0,rawSize);

        printf("%s called @ line %d\n",__func__,__LINE__);
        // DxRaw8toRGB24((unsigned char*)pData,rgb24,width,height,RAW2RGB_NEIGHBOUR,BAYERRG,false);

        printf("%s called @ line %d\n",__func__,__LINE__);

        jpgBuffer = rgb2jpg((const char *) (char *) pData, width,height, 0, &jpg_size);

        printf("rgb2jpg done,jpg_size %d %p %p\n",jpg_size, jpgBuffer, &jpgBuffer);

        //  getCurrentTimeSec(channelId,true,picPath);
        FILE* fp = fopen("/tmp/color_20220822628.jpg", "wb");
        QDateTime dateTime = QDateTime::currentDateTime();
        // 字符串格式化
        QString timestamp = dateTime.toString("yyyy-MM-dd hh:mm:ss.zzz");
        // 获取毫秒值
        int ms = dateTime.time().msec();
        qDebug() <<"ms = "<<ms;
        printf("%s called @ line %d ,ms=%d\n",__func__,__LINE__,ms);
        if (fp)
        {
            VideoBox *v = new VideoBox();
            //socket->write()

            //  connect(v,SIGNAL(true),ui,SLOT(show_video()));
            // emit videoBox.show_video();
            // show_video();
            // connect( w,SIGNAL(true),Videoview,SLOT(show_video()));
            //w.Videoview->addAction();
            int res = fwrite(jpgBuffer, 1, jpg_size, fp);
            fclose(fp);
            printf("%s @ line:%d,res:%d,dst_size %d\n",__func__,__LINE__,res, jpg_size);
        }
        printf("%s called @ line %d,date=%s\n",__func__,__LINE__,__TIME__);
        //  return;

        if (quectelSummary.settingConfig->ftpStorage.enable)
        {
            bRet = quectelSummary.ftpUtil->FtpUpload((char *)jpgBuffer, jpg_size, picPath);
            //      cout << "ftpUpload bRet: " << bRet << endl;
        }




#if 1


        PicData p;
        MsgPicHead h;
        p.head.id=1;
        p.head.msgNo=1;
        p.head.width=width;
        p.head.height=height;
        p.head.bodylength=rawSize *3;
        char *buf =(char *)malloc((sizeof(MsgPicHead)+rawSize *3));
        memcpy(buf,&p.head,sizeof (MsgPicHead));
        memcpy(buf+sizeof(MsgPicHead),(unsigned char*)rgb24,(rawSize *3));

        printf("%s called @ line %d\n",__func__,__LINE__);


        //serverplatform->senddata(buf,rawSize *3);

#endif
        return;
    }



}

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo)
{
    printf("[device line=%d:\n", __LINE__);
    if (NULL == pstMVDevInfo)
    {
        printf("The Pointer of pstMVDevInfo is NULL!\n");
        return false;
    }

    if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE)
    {
        int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
        int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
        int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
        int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

        printf("Device Model Name: %s\n", pstMVDevInfo->SpecialInfo.stGigEInfo.chModelName);
        printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4);
        printf("UserDefinedName: %s\n\n" , pstMVDevInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
    }
    else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE)
    {
        printf("Device Model Name: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chModelName);
        printf("UserDefinedName: %s\n\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
    }
    else
    {
        printf("Not support.\n");
    }

    return true;
}

#define  CAMERA_NUM  1
void* handle[CAMERA_NUM] = {NULL};
int init_MVS_camera()
{
    int nRet = MV_OK;



    MV_CC_DEVICE_INFO_LIST stDeviceList;
    memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

    nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE , &stDeviceList);
    if (MV_OK != nRet)
    {
        printf("MV_CC_EnumDevices fail! nRet [%x]\n", nRet);
        return -1;
    }
    unsigned int nIndex = 0;
    if (stDeviceList.nDeviceNum > 0)
    {
        for (int i = 0; i < stDeviceList.nDeviceNum; i++)
        {
            printf("[device %d]:\n", i);
            MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
            printf("[device line=%d:\n", __LINE__);
            if (NULL == pDeviceInfo)
            {
                break;
            }
            printf("[device line=%d:\n", __LINE__);
            PrintDeviceInfo(pDeviceInfo);
            printf("[device line=%d:\n", __LINE__);
        }
    }
    else
    {
        printf("Find No Devices!\n");
        return -1;
    }
    printf("[device line=%d:\n", __LINE__);
    if(stDeviceList.nDeviceNum < CAMERA_NUM)
    {
        printf("only have %d camera\n", stDeviceList.nDeviceNum);
        return -1;
    }

    printf("Start %d camera Grabbing Image test\n", CAMERA_NUM);

    for(int i = 0; i < CAMERA_NUM; i++)
    {
        //		printf("Please Input Camera Index: ");
        //		scanf("%d", &nIndex);

        nRet = MV_CC_CreateHandle(&handle[i], stDeviceList.pDeviceInfo[nIndex]);
        if (MV_OK != nRet)
        {
            printf("MV_CC_CreateHandle fail! nRet [%x]\n", nRet);
            MV_CC_DestroyHandle(handle[i]);
            return -1;
        }

        nRet = MV_CC_OpenDevice(handle[i]);
        if (MV_OK != nRet)
        {
            printf("MV_CC_OpenDevice fail! nRet [%x]\n", nRet);
            MV_CC_DestroyHandle(handle[i]);
            return -1;
        }

        if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE)
        {
            int nPacketSize = MV_CC_GetOptimalPacketSize(handle[i]);
            if (nPacketSize > 0)
            {
                nRet = MV_CC_SetIntValue(handle[i],"GevSCPSPacketSize",nPacketSize);
                if(nRet != MV_OK)
                {
                    printf("Warning: Set Packet Size fail nRet [0x%x]!\n", nRet);
                }
            }
            else
            {
                printf("Warning: Get Packet Size fail nRet [0x%x]!\n", nPacketSize);
            }
        }
    }


    for(int i = 0; i < CAMERA_NUM; i++)
    {


        //		nRet = MV_CC_SetBoolValue(handle, "AcquisitionFrameRateEnable", false);
        //		if (MV_OK != nRet)
        //		{
        //			printf("set AcquisitionFrameRateEnable fail! nRet [%x]\n", nRet);
        //			break;
        //		}


        nRet = MV_CC_SetEnumValue(handle[i], "TriggerMode", 1);
        if (MV_OK != nRet)
        {
            printf("Cam[%d]: MV_CC_SetTriggerMode fail! nRet [%x]\n", i, nRet);
        }


        nRet = MV_CC_SetEnumValue(handle[i], "TriggerSource", MV_TRIGGER_SOURCE_SOFTWARE);
        if (MV_OK != nRet)
        {
            printf("MV_CC_SetTriggerSource fail! nRet [%x]\n", nRet);
            break;
        }

        nRet = MV_CC_RegisterImageCallBackForRGB(handle[i], ImageCallBackEx, handle[i]);
        if (MV_OK != nRet)
        {
            printf("MV_CC_RegisterImageCallBackEx fail! nRet [%x]\n", nRet);
            break;
        }


        nRet = MV_CC_StartGrabbing(handle[i]);
        if (MV_OK != nRet)
        {
            printf("Cam[%d]: MV_CC_StartGrabbing fail! nRet [%x]\n",i, nRet);
            return -1;
        }

        //		pthread_t nThreadID;
        //		nRet = pthread_create(&nThreadID, NULL ,WorkThread , handle[i]);
        //		if (nRet != 0)
        //		{
        //			printf("Cam[%d]: thread create failed.ret = %d\n",i, nRet);
        //			return -1;
        //		}

        nRet =  MV_CC_SetEnumValue(handle[0],"TriggerSource", MV_TRIGGER_SOURCE_SOFTWARE);
        if (MV_OK != nRet)
        {
            printf("Cam[%d]: MV_CC_StartGrabbing fail! nRet [%x]\n",i, nRet);
            return -1;
        }
    }



}


int getCameraHandle(int id) {
    GX_STATUS               emStatus = GX_STATUS_SUCCESS;
    size_t                  nSize = 0;

    printf("getCameraHandle id:%d\n",id);

    emStatus = GXOpenDeviceByIndex(id, &g_hDevice);

    if (emStatus != GX_STATUS_SUCCESS)
    {
        printf("device open failed!\n");
        GetErrorString(emStatus);
        emStatus = GXCloseLib();
        return -1;
    }

    //Get Device Info
    printf("device %d opened successfully!\n",id);

    //Get string length of Serial number
    emStatus = GXGetStringLength(g_hDevice, GX_STRING_DEVICE_SERIAL_NUMBER, &nSize);
    GX_VERIFY_EXIT(emStatus);

    //Alloc memory for Serial number
    char *  pszSerialNumber = new char[nSize];

    //Get Serial Number
    emStatus = GXGetString(g_hDevice, GX_STRING_DEVICE_SERIAL_NUMBER, pszSerialNumber, &nSize);

    if (emStatus != GX_STATUS_SUCCESS)
    {
        delete[] pszSerialNumber;
        pszSerialNumber         = NULL;
        GX_VERIFY_EXIT(emStatus);
    }

    printf("<Serial Number : %s>\n", pszSerialNumber);
    //Release memory for Serial number
    delete[] pszSerialNumber;
    pszSerialNumber         = NULL;
    bool g_bColorFilter = false;
    int64_t g_nPayloadSize = 0;
    emStatus = GXIsImplemented(g_hDevice, GX_ENUM_PIXEL_COLOR_FILTER, &g_bColorFilter);
    GX_VERIFY_EXIT(emStatus);

    emStatus = GXGetInt(g_hDevice, GX_INT_PAYLOAD_SIZE, &g_nPayloadSize);
    GX_VERIFY_EXIT(emStatus);


    //Set acquisition mode
    emStatus                        = GXSetEnum(g_hDevice, GX_ENUM_ACQUISITION_MODE, GX_ACQ_MODE_CONTINUOUS);
    GX_VERIFY_EXIT(emStatus);

    //Set trigger mode
    emStatus                        = GXSetEnum(g_hDevice, GX_ENUM_TRIGGER_MODE, GX_TRIGGER_MODE_ON);
    GX_VERIFY_EXIT(emStatus);


    //Set buffer quantity of acquisition queue
    uint64_t nBufferNum = ACQ_BUFFER_NUM;
    emStatus = GXSetAcqusitionBufferNumber(g_hDevice, nBufferNum);
    GX_VERIFY_EXIT(emStatus);
    bool bStreamTransferSize = false;
    emStatus = GXIsImplemented(g_hDevice, GX_DS_INT_STREAM_TRANSFER_SIZE, &bStreamTransferSize);
    GX_VERIFY_EXIT(emStatus);

    bool bStreamTransferNumberUrb = false;
    emStatus = GXIsImplemented(g_hDevice, GX_DS_INT_STREAM_TRANSFER_NUMBER_URB, &bStreamTransferNumberUrb);
    GX_VERIFY_EXIT(emStatus);

    //Set trigger source
    //    emStatus = GXSetEnum(g_hDevice, GX_ENUM_TRIGGER_SOURCE, GX_TRIGGER_SOURCE_LINE0);
    //    GX_VERIFY_EXIT(emStatus);

    //Set whitbalance
    //   emStatus = GXSetEnum(g_hDevice, GX_ENUM_BALANCE_WHITE_AUTO,GX_BALANCE_WHITE_AUTO_CONTINUOUS);
    //    GX_VERIFY_EXIT(emStatus);

    //Set buffer quantity of acquisition queue
    //        uint64_t                nBufferNum = ACQ_BUFFER_NUM;

    //      emStatus                        = GXSetAcqusitionBufferNumber(g_hDevice, nBufferNum);
    //    GX_VERIFY_EXIT(emStatus);

    //        //Set size of data transfer block
    //        emStatus                        = GXSetInt(g_hDevice, GX_DS_INT_STREAM_TRANSFER_SIZE, ACQ_TRANSFER_SIZE);
    //        GX_VERIFY_EXIT(emStatus);

    //        //Set qty. of data transfer block
    //        emStatus                        = GXSetInt(g_hDevice, GX_DS_INT_STREAM_TRANSFER_NUMBER_URB, ACQ_TRANSFER_NUMBER_URB);
    //        GX_VERIFY_EXIT(emStatus);

    GXRegisterCaptureCallback(g_hDevice,(void *)id,(GXCaptureCallBack)captureCB);
    printf("GXStreamOn\n");

    //Device start acquisition
    emStatus = GXStreamOn(g_hDevice);
    if (id ==1) {
        g_hDevice1 = g_hDevice;
    } else if (id == 2) {
        g_hDevice2 = g_hDevice;
    }

    if (emStatus != GX_STATUS_SUCCESS)
    {
        //Release the memory allocated
        GX_VERIFY_EXIT(emStatus);
    }

    return 0;
}


std::string GetCPUId()
{
    std::string strCPUId;
    unsigned int level = 1;
    unsigned eax = 3 /* processor serial number */, ebx = 0, ecx = 0, edx = 0;
    //__get_cpuid(level, &eax, &ebx, &ecx, &edx);
    // byte swap
    int first = ((eax >> 24) & 0xff) | ((eax << 8) & 0xff0000) | ((eax >> 8) & 0xff00) | ((eax << 24) & 0xff000000);
    int last = ((edx >> 24) & 0xff) | ((edx << 8) & 0xff0000) | ((edx >> 8) & 0xff00) | ((edx << 24) & 0xff000000);
    // tranfer to string
    std::stringstream ss;
    ss << std::hex << first;
    ss << std::hex << last;
    ss >> strCPUId;
    return strCPUId;
}

static void getDiskId()
{
    qDebug() << endl << "------------------------begin------------------------" << endl;

    QProcess process;
    process.start(" hdparm -i /dev/sda");					//  核心
    process.waitForStarted();
    process.waitForFinished();
    QString str = process.readAllStandardOutput();
    int NoPos = str.indexOf("SerialNo=");
    int CoPos = str.indexOf("Config=");
    if(NoPos != -1 && CoPos != -1)
    {
        QString nStr = str.mid(NoPos+9,CoPos-NoPos-9);
        QString dStr;
        for(short index=0;index<nStr.size();++index)
        {
            char chr = nStr[index].toLatin1();
            if((chr >= '0' && chr <= '9') || (chr >= 'A' && chr <= 'Z') || (chr >= 'a' && chr <= 'z'))
            {
                dStr.append(chr);
            }
        }
        qDebug() << "result:" << dStr << endl;
    }
    qDebug() << endl << "------------------------end------------------------" << endl;
}

QSqlQuery query;
QSqlDatabase dbConn;
using namespace std;
int main(int argc, char *argv[])
{
   string cpustring;

   signal(SIGSEGV, sigsegvhandle);


   cpustring = GetCPUId();
   printf("get cpuid = %s \n",cpustring.c_str());


//    printf("please input key........\n");
//    string keystring;
//    getline(cin,keystring);
//    if(strcmp("quectel123",keystring.c_str()))
//    {
//     printf("err key,exit........\n");
//      return 0;
//    }
    QUIHelper::initMain();
    QApplication a(argc, argv);
    AppInit::Instance()->start();

    QUIHelper::setFont();
    QUIHelper::setCode();
    a.setWindowIcon(QIcon(":/main/icon/m1.png"));
    //加载样式表
    QFile file(":/qss/blacksoft.css");
    if (file.open(QFile::ReadOnly)) {
        QString qss = QLatin1String(file.readAll());
        QString paletteColor = qss.mid(20, 7);
        qApp->setPalette(QPalette(QColor(paletteColor)));
        qApp->setStyleSheet(qss);
        file.close();
    }
    dbConn = QSqlDatabase::addDatabase("QSQLITE");
    dbConn.setDatabaseName(qApp->applicationDirPath() + "/TA.db");
    printf("query database dir:  %s\n", qApp->applicationDirPath().toStdString().c_str());
    if (dbConn.open()) {
        qDebug() << "连接数据库成功!";
    } else {
        qDebug() << "连接数据库失败!";
    }

    QString opeStr;
    query = QSqlQuery(dbConn);

    opeStr = "create table workpieces(date var(255), batch var(255), numOK int, numNG int, numTotal int,"
             "oil int, crush int,swell int, scratch int, "
             "cameraNum int, serialNum int, lostA int, pic_loc varchar(255) "
             ");";
    bool ret=query.exec(opeStr);
    if(ret){
        printf("******Create table ok!\n");
    }else{
        printf("******Create table fail!\n");
    }

    pthread_t rm_id;
    int nRet = pthread_create(&rm_id, NULL, RemovePics, NULL);
    printf("first rm: %d\n",nRet);
    pthread_join(rm_id,NULL);

//    SettingConfig *settingConfig = new SettingConfig();
//    ServerConnect *serverConnect = new ServerConnect(settingConfig);
//    // serverplatform = new Serverplatform(settingConfig);
//    FtpUtil *ftpUtil = new FtpUtil(settingConfig);

    cMvCamera=new CMvCamera();

//    quectelSummary.settingConfig = settingConfig;
//    quectelSummary.serverConnect = serverConnect;
//    quectelSummary.ftpUtil = ftpUtil;
//    UartConfig uartConfig;
//    uartConfig = settingConfig->uartConfig;
//    printf("uartConfig uartName :%s,bau %d,bits :%d,paity %c,stop %d\n",uartConfig.uart_name,uartConfig.speed
//           ,uartConfig.bits,uartConfig.parity,uartConfig.stop);
//    get_uart_fd(uartConfig.uart_name,uartConfig.speed,
//                uartConfig.bits,uartConfig.parity,uartConfig.stop);

    frmMain w;
    //w.resize(1200,800);
    QUIHelper::setFormInCenter(&w);
    w.setStyleSheet("background-color:black");
    w.show();

    return a.exec();
}


void *RemovePics(void* pParam)
{
    std::string result_path = "/data/local/";
    int remove_days = 2;
    printf("%s called\n",__func__);
    remove_pics(result_path,remove_days);
    printf("%s done\n",__func__);
    return 0;
}
