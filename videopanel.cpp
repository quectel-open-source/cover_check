#pragma execution_character_set("utf-8")
#include <QSqlError>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fstream>
#include <iostream>
#include <semaphore.h>
#include "videopanel.h"
#include "videobox.h"
#include "qevent.h"
#include "qmenu.h"
#include "qlayout.h"
#include "qlabel.h"
#include "qtimer.h"
#include "qdebug.h"
#include "Util.hpp"
#include "Serverplatform.h"
#include "ModbusTcpClient.h"
#include "frmmain.h"
#include "qsqldatabase.h"
#include "qsqlquery.h"
#include "frmmain.h"
#include "appinit.h"
#include "quihelper.h"
#include "qsqldatabase.h"
#include "qsqlquery.h"
#include "videobox.h"
#include "DaHengCamera/GxIAPI.h"
#include "DaHengCamera/DxImageProc.h"
#include "MvCamera.h"
#include  "IMVApi.h"
#include <unistd.h>
#include <QTcpSocket>
#include "modbus.h"
#include <exception>
#include <stdlib.h>

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/io.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>
#include <csignal>

#define SAVE_JPG 1
//Get description of error
#define ACQ_BUFFER_NUM                  5
#define ACQ_TRANSFER_SIZE               (64 * 1024)     ///< Size of data transfer block
#define ACQ_TRANSFER_NUMBER_URB 64                              ///< Qty. of data transfer block
#define FILE_NAME_LEN                   128                      ///< Save image file name length

AlgoThread* algoThreadab;
AlgoThread* algoThreadcd;

QString enterDay;
QString enterTime;

vector <qint64> lightList1;
vector <qint64> lightList2;

int gpioNG=0;
QMutex gpioMutex;

int showBegin=0;
int showEnd=0;
showNode showArray[MAX_SHOW];

extern bool isSuspend;
extern int findNum;
extern QString statusStr;
extern Serverplatform *serverplatform;
extern ModbusTcpClient *modbusTcpClient;
extern char *buf;
extern QSqlQuery query;
extern  QSqlDatabase dbConn;
extern int light1Num;
extern int light2Num;

CamDevice*  vcamdevice[4];
static pthread_t	recvThreadId		= -1;
#define HEART_BEAT_TIME     (2)  //��λs
//#define BAND_IP_ADDRESS    "10.88.110.212"
#define BAND_IP_ADDRESS    "127.0.0.1"
#define SERVER_IP_ADDRESS  "127.0.0.1"

#define PORT 8889
int getvalue=0;
static sem_t semSend; 			//���ڷ������ݵı���
static sem_t semRecv;			//���ڽ������ݵ�ͬ��
static sem_t semStopRcv;		//����ֹͣ��ͬ��
using namespace std;

typedef struct memBlockNode{
    char mem[6000*6000];
}memBlock;
#define MAX_MEM_NUM 10
memBlock memListAB[MAX_MEM_NUM];
memBlock memListCD[MAX_MEM_NUM];
int memEndIndexAB=0;
int memEndIndexCD=0;

int pushNumA=0;
int pushNumC=0;

int lostA=0;
int ironWidth=0;
int longNum=0;
int shortNum=0;
int batchNum=0;

extern int cfg_isSavePic;
extern int cfg_abDelay;
extern int cfg_cdDelay;
extern int cfg_coverWidth;
extern int cfg_ironWidth;

static void connectServerInit(void)
{
    sem_init(&semSend, 0, 1);
    sem_init(&semRecv, 0, 0);
    sem_init(&semStopRcv, 0, 0);

}

static void connectServerDeInit(void)
{
    sem_destroy(&semSend);
    sem_destroy(&semRecv);
    sem_destroy(&semStopRcv);
}
static sem_t semFrameRcv;			//���ڷַ�����֡��ͬ��
static sem_t semFrameStop;          //����ֹͣ��ͬ��

static  sem_t *semFrameStopGet(void)
{
    return &semFrameStop;
}

static  sem_t *semFrameRcvGet(void)
{
    return &semFrameRcv;
}

static void messageFrameProcessInit(void)
{
    sem_init(&semFrameRcv, 0, 1);
    sem_init(&semFrameStop, 0, 0);
}

static void messageFrameProcessDeInit(void)
{
    sem_destroy(&semFrameRcv);
    sem_destroy(&semFrameStop);
}

typedef struct __MsgPicHead__
{
    int id; //相机ID
    int msgNo; //对应相机的照片的ID
    int height;
    int width;

    int bodylength;

}MsgPicHead;

typedef struct __VMsgPicHead__
{
    int msgNo;
    int camid; //相机ID
    int campicid; //对应相机的照片的ID
    int label;
    int height;
    int width;

    int bodylength;

}VMsgPicHead;

typedef struct __VMessageBox__
{
    VMsgPicHead	head;
    char			*body;
}VPicData;

typedef struct __MessageBox__
{
    MsgPicHead	head;
    char			*body;
}PicData;
char *vbuf=NULL;


JsonIO::JsonIO()
{

}

JsonIO::~JsonIO()
{

}

void JsonIO::read()
{

}


void operAlready()
{

}


#define WORK_OK 0
#define WORK_NG 1
#define WORK_NULL 2

#define workerNum 6
#define cameraNum 2

//工位和相机关系;反向是因为8号相机最接近要推出的轴承
int wcRelation[cameraNum] = {5,4};

//九个相机的结果
typedef struct cameraPicNode
{
    int isGetPhoto; //是否拍照，如果拍照了，后面节拍内的重复拍照将丢弃
    char picName[128]; //图片名，用于传递给sonResultNode
    char orgPicName[128];
    int sonResult;
}cameraPicNode;

typedef struct sonResultNode
{
    char picName[128]; //图片名，主要插数据库时有用
    char orgPicName[128];
    int sonResult;
}sonResultNode;

cameraPicNode cameraPicArray[cameraNum];
//zl:循环队列，记录每个轴承子结果，纵向最多5个元素，因为第5个工位就出队列了
sonResultNode resultArray[workerNum][cameraNum];
//指向位置就是要出队列的位置
int raIndex = 0;
int resultLen = 0; //结果长度

//是否工作，当第一个工件位扫描到
bool isFirst = 0;
int _numOK = 0;
int _numNG = 0;
int _numTotal = 0;
int _numOil = 0;
int _numScratch = 0;
int _numCrush = 0;
int _numSwell = 0;
//int _numUsedMem[4]={0,0,0,0};

int lostC=0;

char picName[128] = {0};
char orgPicName[128] = {0};
QMap<int, QString> sqlResShow = {{0, "OK"}, {1, "NG"}, {2, "NULL"}};
int sqlPicCol[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8,9};
bool _isCallbackError = false;
bool _isPLCError = false;


// 在没有PLC的时候，可以用计时器来模拟; 若是异常,则return
int VideoPanel::onePush(){

    return 0;
}


static void* threadRecvEntry(void *parm)
{

}


static int _sendDateToServer(int fd, char *data, int size)
{
    int ret = -1;
    int send = 0;
    if (data == NULL)
    {
        printf("param err\n");
        return ret;
    }
    while (size > 0)
    {
        ret = write(fd, data + send, size);
        if (ret < 0)
        {
            printf("[%s]write data err:%d\n", __func__, ret);
            return ret;
        }
        size -= ret;
    }
    return 0;
}

static int v7ImResize(unsigned char * src, int ssize_width, int ssize_height,
                      unsigned char * dst, int dsize_width, int dsize_height)
{
    unsigned char* dstLine;
    if (NULL == src || NULL == dst)
    {
        return -1;
    }

    int srcWidth = ssize_width;
    int srcHeight = ssize_height;
    int srcPitch = ssize_width * 3;

    int dstWidth = dsize_width;
    int dstHeight = dsize_height;
    int dstPitch = dsize_width * 3;

    int hs = 0;
    int ws = 0;
    int he = srcHeight - 1;
    int we = srcWidth - 1;

    float xScale = (we - ws) / (float)dstWidth;
    float yScale = (he - hs) / (float)dstHeight;

    float *fy = new float[dstHeight];
    float *fx = new float[dstWidth];

    int *iy = new int[dstHeight];
    int *ix = new int[dstWidth];

    for (int j = 0; j<dstHeight; ++j)
    {
        int sy = floor((j + 0.5)*yScale - 0.5);
        float su = (j + 0.5)*yScale - 0.5 - sy;
        if (sy < 0) {
            su = 0, sy = 0;
        }
        if (sy >= (he - hs) - 1) {
            su = 0, sy = he - hs - 2;
        }
        fy[j] = su;
        iy[j] = sy;
    }

    for (int i = 0; i<dstWidth; ++i)
    {
        int sx = floor((i + 0.5)*xScale - 0.5);
        float sv = (i + 0.5)*xScale - 0.5 - sx;

        if (sx < 0) {
            sv = 0, sx = 0;
        }
        if (sx >= (we - ws) - 1) {
            sv = 0, sx = we - ws - 2;
        }
        fx[i] = sv;
        ix[i] = sx;
    }

    for (int j = 0; j < dstHeight; j++)
    {
        dstLine = dst + j*dstPitch;
        float cbufy[2];
        int sy = iy[j] + hs;
        cbufy[0] = 1.0f - fy[j];
        cbufy[1] = fy[j];
        for (int i = 0; i < dstWidth; i++)
        {
            int r, g, b;
            float cbufx[2];
            int sx = ix[i] + ws;
            cbufx[0] = 1.0f - fx[i];
            cbufx[1] = fx[i];
            b = (unsigned char)(src[sy*srcPitch + sx * 3] * cbufy[0] * cbufx[0] +
                    src[(sy + 1)*srcPitch + sx * 3] * cbufy[1] * cbufx[0] +
                    src[(sy + 1)*srcPitch + (sx + 1) * 3] * cbufy[1] * cbufx[1] +
                    src[sy*srcPitch + (sx + 1) * 3] * cbufy[0] * cbufx[1]);
            g = (unsigned char)(src[sy*srcPitch + sx * 3 + 1] * cbufy[0] * cbufx[0] +
                    src[(sy + 1)*srcPitch + sx * 3 + 1] * cbufy[1] * cbufx[0] +
                    src[(sy + 1)*srcPitch + (sx + 1) * 3 + 1] * cbufy[1] * cbufx[1] +
                    src[sy*srcPitch + (sx + 1) * 3 + 1] * cbufy[0] * cbufx[1]);
            r = (unsigned char)(src[sy*srcPitch + sx * 3 + 2] * cbufy[0] * cbufx[0] +
                    src[(sy + 1)*srcPitch + sx * 3 + 2] * cbufy[1] * cbufx[0] +
                    src[(sy + 1)*srcPitch + (sx + 1) * 3 + 2] * cbufy[1] * cbufx[1] +
                    src[sy*srcPitch + (sx + 1) * 3 + 2] * cbufy[0] * cbufx[1]);
            *dstLine++ = (uint8_t)b;
            *dstLine++ = (uint8_t)g;
            *dstLine++ = (uint8_t)r;
        }
    }
    delete[]fy;
    delete[]fx;
    delete[]iy;
    delete[]ix;
    return 0;
}

static int sendDateToServer(int fd, char *data, int size)
{
    int ret;
    VPicData p;
    VMsgPicHead h;

    sem_wait(&semSend);
    memcpy(&h,data,sizeof(VMsgPicHead));

    printf("[%s]write data start:%d\n", __func__, h.camid);
    ret = _sendDateToServer(fd, data, size);
    printf("[%s]write data over:%d\n", __func__, ret);

    sem_post(&semSend);
    return ret;
}

static int connectServer(char *ipAddress, int port)
{
    int serverFd;
    struct sockaddr_in serverListenAddr;
    bzero(&serverListenAddr, sizeof(serverListenAddr));
    serverListenAddr.sin_family = AF_INET;
    serverListenAddr.sin_port = htons(port);

    inet_pton(AF_INET, ipAddress, (void *) &serverListenAddr.sin_addr);
    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0)
    {
        perror("socket faild");
        return -1;
    }
    int ret = connect(serverFd, (const struct sockaddr *) &serverListenAddr, sizeof(struct sockaddr));
    printf("[%s  %s:%d]serverFd=[%d] ret=[%d]\n", __func__, ipAddress, port, serverFd, ret);
    if (ret < 0)
    {
        printf("[%s:%d]error: socket connect!\n", ipAddress, port);
        close(serverFd);
        return - 1;
    }
    return serverFd;
}

extern  unsigned char *file_to_buf(char *name, int *size);
void send_Json(int fd,char* filepath,int id)
{
    VPicData p;
    VMsgPicHead h;
    p.head.msgNo=100;

    p.head.width= 0;
    p.head.height=0;
    p.head.camid=id;

    int size=0;
    unsigned char *buf = file_to_buf(filepath,&size);

    p.head.bodylength=size;

    char *sendbuf =(char *)malloc((sizeof(VMsgPicHead)+size));
    memcpy(sendbuf,&p.head,sizeof (VMsgPicHead));
    memcpy(sendbuf+sizeof(VMsgPicHead),(unsigned char*)buf,size);


    printf("size=%d\n",size);

    sendDateToServer(fd,(char *)sendbuf,sizeof(VMsgPicHead)+size);
}

void *vtcpServerThread(void *params)
{
    printf("[%s]wait for server ip and port\n", __func__);
    int ret = 0;
    VideoPanel *pThis = (VideoPanel *)params;
    connectServerInit();
    messageFrameProcessInit();
    pThis->vtcpConnectState = 0;
    printf("[%s]wait for server ip and port\n", __func__);
    //�ȴ�һ����Ч��IP��ַ��port�˿�
    //����ͨ���������ж����¼��ж�ͬ�������ö���..
    //    while (strlen(pThis->settingConfig->serverConfig.server_ip) < 6)
    //    {
    //        printf("[%s]wait f vor server ip and port\n", __func__);
    //        sleep(1);
    //    }
    printf("start server plat,line=%d\n",__LINE__);
reConnect:
    //  pThis->pfd = connectServer(pThis->settingConfig->serverConfig.server_ip, pThis->settingConfig->serverConfig.port);
    printf("[%s]wait for server ip and port  [%s:%d]  line=%d\n", __func__, BAND_IP_ADDRESS, PORT, __LINE__);
    pThis->vfd = connectServer(BAND_IP_ADDRESS, PORT);
    if (pThis->vfd < 0)
    {
        //          printf("[%s]connect ip:%s,port:%d error\n", __func__, pThis->settingConfig->dataOutput.dest_ip,
        //                          pThis->settingConfig->dataOutput.dest_port);
        sleep(2);
        goto reConnect;
    }


    send_Json(pThis->vfd,"inference_yolov5.json",0);

    // send_Json(pThis->vfd,"/home/xian/zhangleo/photo/cpp2/qlzcjs/test_1.json",1);

    // send_Json(pThis->vfd,"/home/xian/zhangleo/photo/cpp2/qlzcjs/test_2.json",2);

    //send_Json(pThis->vfd,"/home/xian/zhangleo/photo/cpp2/qlzcjs/test_3.json",3);
    usleep(1000 * 10);
    printf("start server plat,line=%d\n",__LINE__);
    ret = pthread_create(&recvThreadId, NULL, threadRecvEntry, params);
    usleep(1000 * 10);

}

static pthread_t tcpServerThreadId;
int VideoPanel::vtcpServerThreadCreate(void)
{

    static pthread_attr_t attr;
    pthread_attr_init(&attr);
    printf("start server plat,line=%d\n",__LINE__);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    int nRet = pthread_create(&tcpServerThreadId, &attr, vtcpServerThread, this);
    if (nRet < 0)
    {
        std::cerr << "tcpServer failed" << std::endl;
    }
    return nRet;
}

int VideoPanel::vtcpServereDestroy(void)
{
    //ֹͣrcv�߳�
    if(vfd > 0)
    {
        //  close(vfd);
    }
    sem_wait(&semStopRcv);
    //connectServerDeInit();
    vfd = -1;
    return 0;
}

#include <QSerialPort>        //提供访问串口的功能
#include <QSerialPortInfo>    //提供系统中存在的串口的信息

//创建串口对象
QSerialPort serial;

int serialNum=0;
int warningNum=0;
QTime lastSerialTime;
int allSerialTime=0;

#define DEV_PATH "/dev/mem"

//stop machine
void write_stop(){
    QByteArray buffer;
        buffer[0]=0xab;
        buffer[1]=0x02;
        buffer[2]=0x00;
        buffer[3]=0x00;
    serial.write(buffer);
    printf("write stop\n");
    enterTime = QDateTime::currentDateTime().toString("MM-dd_hh-mm");
    qDebug("write stop!");
}


void write_mem(int port,int value)
{
//    unsigned int    memaddr, wdata;
//    unsigned int    pgoffset, pgaddr;
//    unsigned int    pagesize = sysconf(_SC_PAGESIZE);
//    unsigned char   *p;
//    int             fd;

//    fd = open(DEV_PATH, O_RDWR);
//    if (fd <= 0) {
//            printf("write_io  fd<=0 open error: %s\n", DEV_PATH);
//            return;
//    }

//    if(port==1){
//        //camera1
//        memaddr  = 0xfd6e08a0;
//    }else{
//        //camera2
//        memaddr  = 0xfd6d06a0;
//    }

//    pgoffset = memaddr & (pagesize -1);
//    pgaddr   = memaddr & ~(pagesize -1);
//    p = (unsigned char *)mmap(NULL, pagesize, PROT_WRITE, MAP_SHARED, fd, pgaddr);
//    if (p == 0) {
//       fprintf(stderr, "write_io mmap error\n");
//       return;
//    }

//    if(value==0){
//        wdata=strtoul("0x0200", NULL, 16);
//    }else{
//        wdata=strtoul("0x0201", NULL, 16);
//    }

//    *((unsigned short *)(p + pgoffset)) = (unsigned short)wdata;
//    munmap(p, pagesize);
//    close(fd);
//    printf("write mem port%d value%d OK!\n",port,value);
}


void write_io(int port,int value)
{
   /* unsigned int    memaddr, wdata;
    unsigned int    pgoffset, pgaddr;
    unsigned int    pagesize = sysconf(_SC_PAGESIZE);
    unsigned char   *p;
    int             fd;

    fd = open(DEV_PATH, O_RDWR);
    if (fd <= 0) {
            printf("write_io  fd<=0 open error: %s\n", DEV_PATH);
            return;
    }

    if(port==0){

        memaddr  = 0xfd6e08a0;
    }else{

        memaddr  = 0xfd6d0680;
    }

    pgoffset = memaddr & (pagesize -1);
    pgaddr   = memaddr & ~(pagesize -1);
    p = (unsigned char *)mmap(NULL, pagesize, PROT_WRITE, MAP_SHARED, fd, pgaddr);
    if (p == 0) {
       fprintf(stderr, "write_io mmap error\n");
       return;
    }

    if(value==0){
        wdata=strtoul("0x0200", NULL, 16);
    }else{
        wdata=strtoul("0x0201", NULL, 16);
    }

    *((unsigned short *)(p + pgoffset)) = (unsigned short)wdata;
    munmap(p, pagesize);
    close(fd);
    printf("write io port%d value%d OK!\n",port,value);*/
    //从接收缓冲区中读取数据
    QByteArray buffer;
    if(port==0){
        buffer[0]=0xab;
        buffer[1]=0x00;
        buffer[2]=0x01;
        buffer[3]=0x00;
    }else{
        buffer[0]=0xab;
        buffer[1]=0x00;
        buffer[2]=0x02;
        buffer[3]=0x00;
    }

    serial.write(buffer);
    printf("write io port%d\n",port);
}

int backNum=0;
char bwait[4];
int bwaitNum=0;
int errorNum=0;
extern int light1Num;
extern int light2Num;

void createBatchRoot(){
    QString dayStr=QDateTime::currentDateTime().toString("MM_dd");

    QDir dayDir(QString("/root/cover_result/")+dayStr);
    if(!dayDir.exists())//如果目标目录不存在，则进行创建
    {
        if(!dayDir.mkdir(dayDir.absolutePath()))
        {
            printf("[%s] 创建文件夹error:[%s]\n", __func__, dayStr.toLatin1().data());
        }
        printf("[%s] 创建文件夹success:[%s]\n", __func__, dayStr.toLatin1().data());
    }

    QString minStr=QDateTime::currentDateTime().toString("hh_mm");
    enterDay=QString("%1/b%2-%3").arg(dayStr).arg(batchNum).arg(minStr);
    QDir saveDir(QString("/root/cover_result/")+enterDay);
    if(!saveDir.exists())//如果目标目录不存在，则进行创建
    {
        if(!saveDir.mkdir(saveDir.absolutePath()))
        {
            printf("[%s] 创建文件夹error:[%s]\n", __func__, enterDay.toLatin1().data());
        }
        printf("[%s] 创建文件夹success:[%s]\n", __func__, enterDay.toLatin1().data());
    }

    // 2.1 创建文件夹
//    QString dirPathAll = QString("/root/cover_result/"+enterDay+"/All");
//    QDir savePicDirAll(dirPathAll);
//    if(!savePicDirAll.exists())//如果目标目录不存在，则进行创建
//    {
//        if(!savePicDirAll.mkdir(savePicDirAll.absolutePath()))
//        {
//            printf("[%s] 创建文件夹error:[%s]\n", __func__, dirPathAll.toLatin1().data());
//        }
//        printf("[%s] 创建文件夹success:[%s]\n", __func__, dirPathAll.toLatin1().data());
//    }

    QString dirPathok = QString("/root/cover_result/"+enterDay+"/OK");
    QDir savePicDirok(dirPathok);
    if(!savePicDirok.exists())//如果目标目录不存在，则进行创建
    {
        if(!savePicDirok.mkdir(savePicDirok.absolutePath()))
        {
            printf("[%s] 创建文件夹error:[%s]\n", __func__, dirPathok.toLatin1().data());
        }
        printf("[%s] 创建文件夹success:[%s]\n", __func__, dirPathok.toLatin1().data());
    }

    QString dirPathng = QString("/root/cover_result/"+enterDay+"/NG");
    QDir savePicDirng(dirPathng);
    if(!savePicDirng.exists())//如果目标目录不存在，则进行创建
    {
        if(!savePicDirng.mkdir(savePicDirng.absolutePath()))
        {
            printf("[%s] 创建文件夹error:[%s]\n", __func__, dirPathng.toLatin1().data());
        }
        printf("[%s] 创建文件夹success:[%s]\n", __func__, dirPathng.toLatin1().data());
    }

    QString dirPatha = QString("/root/cover_result/"+enterDay+"/a");
    QDir savePicDira(dirPatha);
    if(!savePicDira.exists())//如果目标目录不存在，则进行创建
    {
        if(!savePicDira.mkdir(savePicDira.absolutePath()))
        {
            printf("[%s] 创建文件夹error:[%s]\n", __func__, dirPatha.toLatin1().data());
        }
        printf("[%s] 创建文件夹success:[%s]\n", __func__, dirPatha.toLatin1().data());
    }

    QString dirPathb = QString("/root/cover_result/"+enterDay+"/b");
    QDir savePicDirb(dirPathb);
    if(!savePicDirb.exists())//如果目标目录不存在，则进行创建
    {
        if(!savePicDirb.mkdir(savePicDirb.absolutePath()))
        {
            printf("[%s] 创建文件夹error:[%s]\n", __func__, dirPathb.toLatin1().data());
        }
        printf("[%s] 创建文件夹success:[%s]\n", __func__, dirPathb.toLatin1().data());
    }

    QString dirPathc = QString("/root/cover_result/"+enterDay+"/c");
    QDir savePicDirc(dirPathc);
    if(!savePicDirc.exists())//如果目标目录不存在，则进行创建
    {
        if(!savePicDirc.mkdir(savePicDirc.absolutePath()))
        {
            printf("[%s] 创建文件夹error:[%s]\n", __func__, dirPathc.toLatin1().data());
        }
        printf("[%s] 创建文件夹success:[%s]\n", __func__, dirPathc.toLatin1().data());
    }

    QString dirPathd = QString("/root/cover_result/"+enterDay+"/d");
    QDir savePicDird(dirPathd);
    if(!savePicDird.exists())//如果目标目录不存在，则进行创建
    {
        if(!savePicDird.mkdir(savePicDird.absolutePath()))
        {
            printf("[%s] 创建文件夹error:[%s]\n", __func__, dirPathd.toLatin1().data());
        }
        printf("[%s] 创建文件夹success:[%s]\n", __func__, dirPathd.toLatin1().data());
    }
}

void clearAll(){
    QString qpath=QString("/root/cover_result/"+enterDay+"/result.txt");
    QFile file(qpath);
    if(file.open(QIODevice::ReadWrite)){
            QTextStream aStream(&file);
            aStream<<"batchNum:"<<batchNum<<endl;
            aStream<<"serialNum:"<<serialNum<<endl;
            aStream<<"pushNumA:"<<pushNumA<<endl;
            aStream<<"pushNumC:"<<pushNumC<<endl;
            aStream<<"NumOK:"<<_numOK<<endl;
            aStream<<"NumNG:"<<_numNG<<endl;
            aStream<<"NumTotal:"<<_numTotal<<endl;
            file.close();
    }
    
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString savePicDir=qpath;
    QString sqlStr=
            QString("insert into workpieces(date,pic_loc,numOK,numNG,numTotal,"
                    "oil,crush,swell,scratch,"
                    "cameraNum,serialNum,lostA,batch"
                    ") "
                    "values ('%1','%2',%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,'%13')"
                   )
    .arg(timestamp).arg(savePicDir)
    .arg(_numOK).arg(_numNG).arg(_numTotal)
    .arg(_numOil).arg(_numCrush).arg(_numSwell).arg(_numScratch)
     .arg(pushNumA).arg(serialNum).arg(lostA).arg(enterDay);
     
    bool isInsert = query.exec(sqlStr);

    if(isInsert==true){
        cout<<"insert ok"<<endl;
    }else{
        QSqlError err=query.lastError();
        printf("insert fail,%s,%s\n",err.driverText().toStdString().c_str(),err.databaseText().toStdString().c_str());
    }

    batchNum++;
    createBatchRoot();

    showBegin=0;
    showEnd=0;
    lostA=0;

    serialNum=0;
    pushNumA=0;
    pushNumC=0;
    _numOil=0;
    _numCrush=0;
    _numSwell=0;
    _numScratch=0;
    _numNG=0;
    _numOK=0;
    _numTotal=0;
    ironWidth=0;
    longNum=0;
    shortNum=0;
    findNum=0;
    light1Num=0;
    light2Num=0;
}

//编写的槽函数
void serialPort_readyRead()
{
    QDateTime dateTime ;
    QString timestamp ;
    dateTime = QDateTime::currentDateTime();
    timestamp = dateTime.toString("hh:mm:ss.zzz");

    //从接收缓冲区中读取数据
    QByteArray buffer = serial.readAll();
    if(buffer.size()!=4){
        for(int i=0;i<buffer.size();i++){
            bwait[bwaitNum]=buffer[i];
            printf("bwait[%d]=%x\n",i,buffer[i]&0xff);
            qDebug("bwait[%d]=%x\n",i,buffer[i]&0xff);

            bwaitNum++;
        }
        printf("recieve serial half %d\n",buffer.size());
        qDebug("recieve serial half %d\n",buffer.size());
        if(bwaitNum>=4){
            bwaitNum=0;
            buffer.resize(4);
            buffer[0]=bwait[0];
            buffer[1]=bwait[1];
            buffer[2]=bwait[2];
            buffer[3]=bwait[3];
        }else{
            return;
        }
    }

    qDebug("recieve rs232:%x,%x,%x,%x timestamp:%s\n",
           (unsigned char)buffer[0]&0xff,(unsigned char)buffer[1]&0xff
            ,(unsigned char)buffer[2]&0xff,(unsigned char)buffer[3]&0xff
            ,timestamp.toLatin1().data());
    printf("recieve rs232:%x,%x,%x,%x timestamp:%s\n",
           (unsigned char)buffer[0]&0xff,(unsigned char)buffer[1]&0xff
            ,(unsigned char)buffer[2]&0xff,(unsigned char)buffer[3]&0xff
            ,timestamp.toLatin1().data());

    if(isSuspend==true){
         qDebug("rs232 return for suspend\n");
         printf("rs232 return for suspend\n");
         return;
    }

    if((buffer[0]&0xff)==0xab&&(buffer[1]&0xff)==0x01
        &&(buffer[2]&0xff)==0x00&&(buffer[3]&0xff)==0x00){
        printf("recieve clear signal\n");
        clearAll();
        printf("recieve clear OK\n");

        emit vcamdevice[0]->m_v->sendstatus("清空状态");
        emit vcamdevice[0]->m_v->sendui();
        enterTime = QDateTime::currentDateTime().toString("MM-dd_hh-mm");
        qDebug("recieve stop!\n");
        return;
    }else if((buffer[0]&0xff)==0xab&&(buffer[1]&0xff)==0x00&&(buffer[2]&0xff)==0x07
             &&(buffer[3]&0xff)==0x00){
             backNum++;
             printf("recieve back serial NG:%x,%x,%x,%x backNum%d\n",(unsigned char)buffer[0]&0xff
                     ,(unsigned char)buffer[1]&0xff,
                     (unsigned char)buffer[2]&0xff,
                     (unsigned char)buffer[3]&0xff,backNum);

             return;
    }else if((buffer[0]&0xff)==0xab&&(buffer[1]&0xff)==0x00&&(buffer[2]&0xff)==0x06
             &&(buffer[3]&0xff)==0x00){
             backNum++;
             printf("recieve back serial OK:%x,%x,%x,%x backNum%d\n",(unsigned char)buffer[0]&0xff
                     ,(unsigned char)buffer[1]&0xff,
                     (unsigned char)buffer[2]&0xff,
                     (unsigned char)buffer[3]&0xff,backNum);

             return;
    }else if((buffer[0]&0xff)!=0xab||(buffer[1]&0xff)!=0x00||(buffer[2]&0xff)!=0x00
        ||(buffer[3]&0xff)!=0x00){
        errorNum++;
        printf("recieve bad serial:%x,%x,%x,%x backNum%d\n",(unsigned char)buffer[0]&0xff
                ,(unsigned char)buffer[1]&0xff,
                (unsigned char)buffer[2]&0xff,
                (unsigned char)buffer[3]&0xff,errorNum);

        return;
    }

    printf("recieve serial:%x,%x,%x,%x serialNum%d size%d\n",(unsigned char)buffer[0]&0xff
            ,(unsigned char)buffer[1]&0xff,(unsigned char)buffer[2]&0xff
            ,(unsigned char)buffer[3]&0xff,serialNum,buffer.size());

    std::cout<<"Reciever serial,Time"<<timestamp.toStdString()<<endl;
    QTime nowTime=QTime::currentTime();
    serialNum++;
    emit vcamdevice[0]->m_v->sendui();

    if(serialNum>1){
        int distant=lastSerialTime.msecsTo(nowTime);
        allSerialTime+=distant;
        if(warningNum>0){
            printf("+++++serial:%x%x%x%x serialNum:%d warningNum:%d thisTime=%d(ms) avgTime=%d(ms)\n",(unsigned char)buffer[0]&0xff,(unsigned char)buffer[1]&0xff,(unsigned char)buffer[2]&0xff,(unsigned char)buffer[3]&0xff,serialNum,warningNum,distant,(int)allSerialTime/serialNum);
        }else{
            printf("_____serial:%x%x%x%x serialNum:%d  thisTime=%d(ms) avgTime=%d(ms)\n",(unsigned char)buffer[0]&0xff,(unsigned char)buffer[1]&0xff,(unsigned char)buffer[2]&0xff,(unsigned char)buffer[3]&0xff,serialNum,distant,(int)allSerialTime/serialNum);
        }
    }else{
        printf("_____serial:%x%x%x%x serialNum:%d warningNum:%d\n",(unsigned char)buffer[0]&0xff,(unsigned char)buffer[1]&0xff,(unsigned char)buffer[2]&0xff,(unsigned char)buffer[3]&0xff,serialNum,warningNum);
    }
    lastSerialTime=nowTime;

//    if(pushNumA==pushNumB){
//        lostC=0;
//        lostLoc=0;
//        printf("clear lostC\n");
//        qDebug("clear lostC\n");
//    }

    if(showBegin==showEnd){ //resultList.size()<=0){  //
        warningNum++;
        printf("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
        printf("#####serial:Error for size0,serialNum:%d warningNum:%d\n",serialNum,warningNum);
        //printf("%d\n",1/0);

        QString showStr=QString("Get more signal%1").arg(warningNum);
        emit vcamdevice[0]->m_v->sendstatus(showStr);
        //send NG
        write_io(1,1);
        serialNum--;
        qDebug("back NG to plc,Get more signal %d\n",warningNum);
        return;
    }

    int re=showArray[showBegin].result;
//    if(re==-1){
//        warningNum++;
//        printf("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
//        printf("#####serial:get fast for size0,serialNum:%d warningNum:%d\n",serialNum,warningNum);

//        QString showStr=QString("Get more fast%1").arg(warningNum);
//        emit vcamdevice[0]->m_v->sendstatus(showStr);
//        //send NG
//        write_io(1,1);
//        serialNum--;
//        qDebug("back NG to plc,Get more fast %d\n",warningNum);
//        return;
//    }

    showBegin=(showBegin+1)%MAX_SHOW;

    //OK or no result
    if(re==1||re==-1){
        printf("Get result showBegin%d,showEnd%d...ok\n",showBegin,showEnd);
        write_io(0,1);
        qDebug("back OK to plc serialNum%d,time%s\n",serialNum,timestamp.toLatin1().data());
    }else{
        printf("Get result showBegin%d,showEnd%d...ok\n",showBegin,showEnd);
        write_io(1,1);
        qDebug("back NG to plc serialNum%d,time%s\n",serialNum,timestamp.toLatin1().data());
    }
}

VideoPanel::VideoPanel(QWidget *parent) : QWidget(parent)
{
//    printf("begin opencv\n");
//    cv::Mat imageSrc = cv::imread("93.jpg");  //93.jpg
//    printf("[w%d,h%d]\n",imageSrc.cols,imageSrc.rows);
//    cv::imwrite("result1.bmp", imageSrc);
//    printf("save opencv\n");
    srand(time(nullptr));
    this->initControl();
    this->initForm();
    this->initMenu();
    //resultList[3]=5;

    sqlstartvalue=0;
    num=0;
    int nRet = 0;
    CamRet[100]={0};
    _timeTotal = 0;

//    query.exec(QString("select * from workpieces order by id desc limit 1 "));
//    while(query.next()) {
//        pushNum  =query.value(0).toInt();
//        _numOK = query.value(13).toInt();
//        _numNG = query.value(14).toInt();
//        _numTotal = query.value(15).toInt();
//        //emit vcamdevice[0]->m_v->sendui(_numOK, _numNG, _numTotal);
//        printf("[onePush] [line=%d] history result -------- pushNum=%d, ok=%d, ng=%d, total=%d", __LINE__, pushNum, _numOK, _numNG, _numTotal);
//    }
//    printf("[sqlite] start server plat, line=%d, pushNum=%d\n",__LINE__, pushNum);

    //zl：把接受线程先注释掉
    //nRet = vtcpServerThreadCreate();//tcp server rcv
    //if (nRet < 0)
    //{
    //    printf("start server plat,line=%d\n",__LINE__);
    //}
    //printf("start server plat,line=%d\n",__LINE__);

    //QTimer::singleShot(1000, this, SLOT(play_video_all()));


    connect(this, &VideoPanel::sendCallbackImg, this, &VideoPanel::recvPicFromCallback);

    algoThreadab = new AlgoThread(this);
    connect(this, &VideoPanel::emit_infer_memab, algoThreadab, &AlgoThread::transform);
    printf("##################Algoab before run\n");
    algoThreadab->start();
    printf("##################Algoab run OK\n");

    algoThreadcd = new AlgoThread(this);
    connect(this, &VideoPanel::emit_infer_memcd, algoThreadcd, &AlgoThread::transform);
    printf("##################Algoab before run\n");
    algoThreadcd->start();
    printf("##################Algoab run OK\n");


    printf("##################Mem thread run\n");
    MemThread* memThread1 = new MemThread(this);
    memThread1->start();

    LightThread* lightThread1 = new LightThread(this);
    lightThread1->start();
    lightThread1->camera=1;

    LightThread* lightThread2 = new LightThread(this);
    lightThread2->start();
    lightThread2->camera=2;
    printf("##################Light thread OK\n");

    //设置串口名 ttyUSB0
    serial.setPortName("ttyUSB0");
    //设置波特率
    serial.setBaudRate(QSerialPort::Baud19200); //19200
    //设置数据位数
    serial.setDataBits(QSerialPort::Data8);
    //设置奇偶校验
    serial.setParity(QSerialPort::NoParity);
    //设置停止位
    serial.setStopBits(QSerialPort::OneStop);
    //设置流控制
    serial.setFlowControl(QSerialPort::NoFlowControl);
    //打开串口
    if(serial.open(QIODevice::ReadWrite)){
        printf("serial open OK");
        QObject::connect(&serial, &QSerialPort::readyRead, this, serialPort_readyRead);
    }else{
        printf("serial open fail!\n");
        statusStr="串口连接失败";
    }
}

void VideoPanel::loadHistoryResult()
{
    //emit vcamdevice[0]->m_v->sendui(_numOK, _numNG, _numTotal);
}

bool VideoPanel::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick) {
        QLabel *widget = (QLabel *) watched;
        if (!videoMax) {
            videoMax = true;
            videoBox->hide_video_all();
            gridLayout->addWidget(widget, 0, 0);
            widget->setVisible(true);
        } else {
            videoMax = false;
            videoBox->show_video_all();
        }
        widget->setFocus();
    } else if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = (QMouseEvent *)event;
        if (mouseEvent->button() == Qt::RightButton) {
            videoMenu->exec(QCursor::pos());
        }
    }

    return QWidget::eventFilter(watched, event);
}

QSize VideoPanel::sizeHint() const
{
    return QSize(1600, 600);
}

QSize VideoPanel::minimumSizeHint() const
{
    return QSize(80, 60);
}

void VideoPanel::initControl()
{
    gridLayout = new QGridLayout;
    gridLayout->setSpacing(1);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(gridLayout);
}

CamResult cret[4];

int qflag=1;

extern void getCurrentTimeSec(int channelId, bool is_raw,char * str);
extern void SavegetCurrentTimeSec(int channelId, int num,bool is_raw,char * str);
extern void getCurrentTimeDay(char *str);
int save_rampic(char *data,int w,int h,int size,int id);
int save_rampic(char *data,int w,int h,int size,int id)
{

    unsigned char* jpgBuffer = NULL;

    long jpg_size;


    // hexDumpData((pjpeg+20), 40);
    printf("over,buff\n");
    // jpgBuffer = rgb2jpg((const char *) (char *) (pjpeg+20), 2448,2048, 0, &jpg_size);
    jpgBuffer = rgb2jpg((const char *) (char *) (data), w,h, 0, &jpg_size);
    // hexDumpData((pjpeg+20), 40);
    printf("over,jpegbuff\n");
    char path[128];
    memset(path,0,128);
    sprintf(path,"/run/shm/ch_%d.jpg",id);

    printf("path=%s",path);
    FILE* fp = fopen(path, "wb");
    if (fp)
    {

        int res = fwrite(jpgBuffer, 1, jpg_size, fp);
        fclose(fp);
        free(jpgBuffer);
        printf("%s @ line:%d,res:%d,dst_size %d\n",__func__,__LINE__,res, jpg_size);
    }
    return  1;
}

void VideoPanel::updateSendJson(int jsonId)
{
    printf(" ----> [%d] start send json\n", jsonId);
//    send_Json(this->vfd, "inference_yolov5.json", 0);
    //quectel_infer.initial_inference("./inference_yolov5.json");
    printf(" ----> [%d] end send json: inference_yolov5.json\n", jsonId, jsonId);
}


int gxNum=0;
int lastTime=0;
void __stdcall VideoPanel::gxImageCallBack(GX_FRAME_CALLBACK_PARAM *pFrameData)
{

}

void testFree(const char * pData){
    pData=0;

    return;
}

void __stdcall VideoPanel::hkImageCallBack(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
{

}

static void GetErrorString(GX_STATUS emErrorStatus)
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


//Show error message, close device and lib
#define GX_VERIFY_EXIT(emStatus) \
    if (emStatus != GX_STATUS_SUCCESS)     \
{                                      \
    GetErrorString(emStatus);          \
    GXCloseLib();                      \
    printf("<App Exit!>\n");           }
// return emStatus;

static void displayDeviceInfo(IMV_DeviceList deviceInfoList)
{
    IMV_DeviceInfo* pDevInfo = NULL;
    unsigned int cameraIndex = 0;
    char vendorNameCat[11];
    char cameraNameCat[16];

    // 打印Title行
    // Print title line
    printf("\nIdx Type Vendor     Model      S/N             DeviceUserID    IP Address    \n");
    printf("------------------------------------------------------------------------------\n");

    for (cameraIndex = 0; cameraIndex < deviceInfoList.nDevNum; cameraIndex++)
    {
        pDevInfo = &deviceInfoList.pDevInfo[cameraIndex];
        // 设备列表的相机索引  最大表示字数：3
        // Camera index in device list, display in 3 characters
        printf("%-3d", cameraIndex + 1);

        // 相机的设备类型（GigE，U3V，CL，PCIe）
        // Camera type
        switch (pDevInfo->nCameraType)
        {
        case typeGigeCamera:printf(" GigE");break;
        case typeU3vCamera:printf(" U3V ");break;
        case typeCLCamera:printf(" CL  ");break;
        case typePCIeCamera:printf(" PCIe");break;
        default:printf("     ");break;
        }

        // 制造商信息  最大表示字数：10
        // Camera vendor name, display in 10 characters
        if (strlen(pDevInfo->vendorName) > 10)
        {
            memcpy(vendorNameCat, pDevInfo->vendorName, 7);
            vendorNameCat[7] = '\0';
            strcat(vendorNameCat, "...");
            printf(" %-10.10s", vendorNameCat);
        }
        else
        {
            printf(" %-10.10s", pDevInfo->vendorName);
        }

        // 相机的型号信息 最大表示字数：10
        // Camera model name, display in 10 characters
        printf(" %-10.10s", pDevInfo->modelName);

        // 相机的序列号 最大表示字数：15
        // Camera serial number, display in 15 characters
        printf(" %-15.15s", pDevInfo->serialNumber);

        // 自定义用户ID 最大表示字数：15
        // Camera user id, display in 15 characters
        if (strlen(pDevInfo->cameraName) > 15)
        {
            memcpy(cameraNameCat, pDevInfo->cameraName, 12);
            cameraNameCat[12] = '\0';
            strcat(cameraNameCat, "...");
            printf(" %-15.15s", cameraNameCat);
        }
        else
        {
            printf(" %-15.15s", pDevInfo->cameraName);
        }

        // GigE相机时获取IP地址
        // IP address of GigE camera
        if (pDevInfo->nCameraType == typeGigeCamera)
        {
            printf(" %s", pDevInfo->DeviceSpecificInfo.gigeDeviceInfo.ipAddress);
        }

        printf("\n");
    }

    return;
}

QTime  lastBTime;
void __stdcall VideoPanel::imvImageCallBack2(IMV_Frame* pFrame, void* pUser)
{
    pushNumC++;
    QTime nowTime=QTime::currentTime();
    QString timestamp = nowTime.toString("hh:mm:ss.zzz");

    printf("Push%d DahuaC Get frame blockId = %llu\n",pushNumC,pFrame->frameInfo.blockId);
    qDebug("Push%d DahuaC Get frame blockId = %llu,time=%s\n",
           pushNumC,pFrame->frameInfo.blockId,(char*)timestamp.toStdString().data());

    emit vcamdevice[0]->m_v->sendui();


    char path[256];
    char path2[256];

        int width=pFrame->frameInfo.width;
        int height=pFrame->frameInfo.height;
        if(width>10000||height/2>10000){
            printf("##########camera para is big than 2000!");
            return;
        }
        int i;
        memEndIndexCD=(memEndIndexCD+1)%MAX_MEM_NUM;
        unsigned char* rgb242 = (unsigned char*)memListCD[memEndIndexCD].mem;
        memEndIndexCD=(memEndIndexCD+1)%MAX_MEM_NUM;
        unsigned char* rgb24 = (unsigned char*)memListCD[memEndIndexCD].mem;

        unsigned char* buf = pFrame->pData;
        int halfHeight=height/2;
        for (i = 0; i < halfHeight; i++) {
             //memcpy(rgb242+i*width,buf+i*2*width,width);
             //memcpy(rgb24+i*width,buf+i*2*width+width,width);
            for(int j=0;j<width;j++){
                rgb242[i*width+j]=buf[i*2*width+width-1-j];
                rgb24[i*width+j]=buf[i*2*width+width+width-1-j];
            }
        }

    if(cfg_isSavePic==1){
        //存第一张图
        QImage image(rgb24,width,halfHeight,QImage::Format_Grayscale8);
        QString qpath=QString("/root/cover_result/"+enterDay+"/c/%1c.png").arg(pushNumC);
        image.save(qpath,"PNG",100);
        qDebug(qpath.toLatin1());

        //存第二张图
        QImage image2(rgb242,width,halfHeight,QImage::Format_Grayscale8);
        QString qpath2=QString("/root/cover_result/"+enterDay+"/d/%1d.png").arg(pushNumC);
        image2.save(qpath2,"PNG",100);
        qDebug(qpath2.toLatin1());

        //emit vcamdevice[0]->m_v->sendCallbackImg(image.scaled(vcamdevice[2]->mlabel->width(), vcamdevice[2]->mlabel->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation), vcamdevice[2]->mlabel);
        //emit vcamdevice[1]->m_v->sendCallbackImg(image2.scaled(vcamdevice[3]->mlabel->width(), vcamdevice[3]->mlabel->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation), vcamdevice[3]->mlabel);
    }

    emit vcamdevice[0]->m_v->emit_infer_memcd(1,rgb24,rgb242,pushNumC,width,height/2);
}


QTime  lastATime;

void clearNode(showNode *sd){
    sd->a=-1;
    sd->b=-1;
    sd->c=-1;
    sd->d=-1;

    sd->isOil=false;
    sd->isCrush=false;
    sd->isScratch=false;
    sd->isSwell=false;
    sd->result=-1;
}

void __stdcall VideoPanel::imvImageCallBack(IMV_Frame* pFrame, void* pUser)
{
    pushNumA++;
    QTime nowTime=QTime::currentTime();
    QString timestamp = nowTime.toString("hh:mm:ss.zzz");

    printf("Push%d DahuaA Get frame blockId = %llu\n",pushNumA,pFrame->frameInfo.blockId);
    qDebug("Push%d DahuaA Get frame blockId = %llu,time=%s\n",
           pushNumA,pFrame->frameInfo.blockId,(char*)timestamp.toStdString().data());

    clearNode(&showArray[showEnd]);
    showArray[showEnd].pushNum=pushNumA;
    showEnd=(showEnd+1)%MAX_SHOW;
    printf("push showList,showBegin%d,showEnd%d\n",showBegin,showEnd);

    emit vcamdevice[0]->m_v->sendui();

    char path[256];
    char path2[256];

    int width=pFrame->frameInfo.width;
    int height=pFrame->frameInfo.height;
    if(width>10000||height/2>10000){
        printf("##########camera para is big than 2000!");
        return;
    }


    int i;
    memEndIndexAB=(memEndIndexAB+1)%MAX_MEM_NUM;
    unsigned char* rgb242 = (unsigned char*)memListAB[memEndIndexAB].mem;
    memEndIndexAB=(memEndIndexAB+1)%MAX_MEM_NUM;
    unsigned char* rgb24 = (unsigned char*)memListAB[memEndIndexAB].mem;
    unsigned char* buf = pFrame->pData;

    int halfHeight=height/2;
    for (i = 0; i < halfHeight; i++) {
         memcpy(rgb242+i*width,buf+i*2*width,width);
         memcpy(rgb24+i*width,buf+i*2*width+width,width);
    }

    if(cfg_isSavePic==1){
//        QImage imageAll(buf,width,halfHeight*2,QImage::Format_Grayscale8);
//        QString qpathAll=QString("/root/cover_result/"+enterDay+"/%1all.jpg").arg(pushNumA);
//        imageAll.save(qpathAll,"JPG",100);
//        qDebug(qpathAll.toLatin1());

            //存第一张图
            QImage image(rgb24,width,halfHeight,QImage::Format_Grayscale8);
            QString qpath=QString("/root/cover_result/"+enterDay+"/a/%1a.png").arg(pushNumA);
            image.save(qpath,"PNG",100);
            qDebug(qpath.toLatin1());

            //存第二张图
            QImage image2(rgb242,width,halfHeight,QImage::Format_Grayscale8);
            QString qpath2=QString("/root/cover_result/"+enterDay+"/b/%1b.png").arg(pushNumA);
            image2.save(qpath2,"PNG",100);
            qDebug(qpath2.toLatin1());

            //emit vcamdevice[0]->m_v->sendCallbackImg(image.scaled(vcamdevice[0]->mlabel->width(), vcamdevice[0]->mlabel->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation), vcamdevice[0]->mlabel);
            //emit vcamdevice[1]->m_v->sendCallbackImg(image2.scaled(vcamdevice[1]->mlabel->width(), vcamdevice[1]->mlabel->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation), vcamdevice[1]->mlabel);
    }

    emit vcamdevice[0]->m_v->emit_infer_memab(0,rgb24,rgb242,pushNumA,width,height/2);

}

//
void VideoPanel::recvPicFromCallback(QImage image, QLabel *label)
{
    QPixmap pix = QPixmap::fromImage(image);

    printf("  pix.width=%d,  pix.height=%d,  label.width=%d,  label.height=%d\ns", pix.width(), pix.height(), label->width(), label->height());
    label->setPixmap(pix);
    printf("  recvPicFromCallback OK\ns");
}

//循环打开所有大恒设备
void VideoPanel::setDaheng(int ui32DeviceNum)
{
    GX_STATUS       emStatus = GX_STATUS_SUCCESS;
    GX_OPEN_PARAM  stOpenParam;
    size_t  nSize = 0;
    int ncam_index = 0;
    for(int i = 0; i<ui32DeviceNum; i++ )
    {
        gx_hDevice[i] = NULL;
        memset(&stOpenParam,0,sizeof(stOpenParam));
        stOpenParam.accessMode =  GX_ACCESS_EXCLUSIVE;
        stOpenParam.openMode = GX_OPEN_IP;//GX_OPEN_IP;
        char ip_str[128];
        memset(ip_str,0,128);
        //从ip10开始搜索
        ncam_index = i + 10;
        sprintf(ip_str,"192.168.2.%d",ncam_index);

        stOpenParam.pszContent= ip_str;
        printf("getCameraHandle num:%d , ip =%s\n",i,stOpenParam.pszContent);

        int nRet =  vcamdevice[i]->m_gxCamera->Open(&stOpenParam,&gx_hDevice[i]);
        if (nRet != GX_STATUS_SUCCESS)
        {
            printf("device [%d] open failed!\n", i);
            GetErrorString(emStatus);
            emStatus = GXCloseLib();
            //  return -1;
        }
        else{
            printf("device %s opened successfully!\n",stOpenParam.pszContent);
        }


        //Get string length of Serial number
        emStatus = GXGetStringLength(gx_hDevice[i], GX_STRING_DEVICE_SERIAL_NUMBER, &nSize);
        GX_VERIFY_EXIT(emStatus);

        //Alloc memory for Serial number
        char *  pszSerialNumber = new char[nSize];

        //Get Serial Number
        emStatus = GXGetString(gx_hDevice[i], GX_STRING_DEVICE_SERIAL_NUMBER, pszSerialNumber, &nSize);

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
        emStatus = GXIsImplemented(gx_hDevice[i], GX_ENUM_PIXEL_COLOR_FILTER, &g_bColorFilter);
        GX_VERIFY_EXIT(emStatus);

        emStatus = GXGetInt(gx_hDevice[i], GX_INT_PAYLOAD_SIZE, &g_nPayloadSize);
        GX_VERIFY_EXIT(emStatus);


        //Set acquisition mode
        emStatus                        = GXSetEnum(gx_hDevice[i], GX_ENUM_ACQUISITION_MODE, GX_ACQ_MODE_CONTINUOUS);
        GX_VERIFY_EXIT(emStatus);

        //Set trigger mode
        emStatus                        = GXSetEnum(gx_hDevice[i], GX_ENUM_TRIGGER_MODE, GX_TRIGGER_MODE_ON);
        GX_VERIFY_EXIT(emStatus);

        //设置相机的流通道包长属性,提高网络相机的采集性能
        bool bImplementPacketSize = false;
        uint32_t unPacketSize = 1500; //0;
        // 判断设备是否支持流通道数据包功能
        emStatus = GXIsImplemented(gx_hDevice[i], GX_INT_GEV_PACKETSIZE, &bImplementPacketSize);
        if (bImplementPacketSize)
        {
            //            // 获取当前网络环境的最优包长
            //            emStatus = GXGetOptimalPacketSize (gx_hDevice[i], &unPacketSize);
            // 将最优包长设置为当前设备的流通道包长值
            emStatus = GXSetInt (gx_hDevice[i], GX_INT_GEV_PACKETSIZE, unPacketSize);
            printf("[%s] id=%d, ret=%d  流通道包长值=%d\n", __FUNCTION__, i, emStatus, unPacketSize);

            GX_VERIFY_EXIT(emStatus);
        }
        else{
            printf("[%s] id=%d, ret=%d  不支持流通道包长\n", __FUNCTION__, i, emStatus);
        }

        //        // 硬触发
        //        emStatus = GXSendCommand(gx_hDevice[i], GX_TRIGGER_SOURCE_LINE2);
        //        printf("启动硬触发！！！");

        //        // 设置相机触发延迟:us
        //        emStatus = GXSetEnum(gx_hDevice[i], GX_ENUM_TRIGGER_SELECTOR, GX_ENUM_TRIGGER_SELECTOR_FRAME_START);
        //        GX_VERIFY_EXIT(emStatus);
        //        emStatus = GXSetFloat(gx_hDevice[i], GX_FLOAT_TRIGGER_DELAY, 2000000.0000);
        //        GX_VERIFY_EXIT(emStatus);

        //Set buffer quantity of acquisition queue
        uint64_t nBufferNum = ACQ_BUFFER_NUM;
        emStatus = GXSetAcqusitionBufferNumber(gx_hDevice[i], nBufferNum);
        GX_VERIFY_EXIT(emStatus);

        bool bStreamTransferSize = false;
        emStatus = GXIsImplemented(gx_hDevice[i], GX_DS_INT_STREAM_TRANSFER_SIZE, &bStreamTransferSize);
        GX_VERIFY_EXIT(emStatus);

        bool bStreamTransferNumberUrb = false;
        emStatus = GXIsImplemented(gx_hDevice[i], GX_DS_INT_STREAM_TRANSFER_NUMBER_URB, &bStreamTransferNumberUrb);
        GX_VERIFY_EXIT(emStatus);
        // Set whitbalance


        //Get the type of Bayer conversion. whether is a color camera.
        emStatus = GXIsImplemented(gx_hDevice[i], GX_ENUM_PIXEL_COLOR_FILTER, &g_bColorFilter);
        GX_VERIFY_EXIT(emStatus);
        if (g_bColorFilter)
        {
            emStatus = GXGetEnum(gx_hDevice[i], GX_ENUM_PIXEL_COLOR_FILTER, &g_i64ColorFilter[i]);
            GX_VERIFY_EXIT(emStatus);
        }else{
            g_i64ColorFilter[i] = GX_COLOR_FILTER_NONE;
        }
        printf("i=%d,g_i64ColorFilter[i]=%d\n",i,g_i64ColorFilter[i]);

        nRet = vcamdevice[i]->m_gxCamera->RegisterImageCallBack(gx_hDevice[i], vcamdevice[i],gxImageCallBack);
        if (nRet != GX_STATUS_SUCCESS)
        {
            //Release the memory allocated
            printf("GXRegisterCaptureCallback err,exit, ret=%d\n",nRet);
            GX_VERIFY_EXIT(emStatus);
        }else{
            printf("%d GXRegisterCaptureCallback success, ret=%d\n", i, nRet);
        }

        nRet =   vcamdevice[i]->m_gxCamera->StartGrabbing();
        if (nRet != GX_STATUS_SUCCESS)
        {
            //Release the memory allocated
            printf("GXStreamOn err,exit,i=%d\n",i);
            GX_VERIFY_EXIT(emStatus);
        }else{
            printf("GXStreamOn success\n");
        }

    }

    //    int ret = vcamdevice[0]->m_gxCamera->CommandExecute(GX_TRIGGER_SOURCE_LINE2);  //GX_TRIGGER_SOURCE_LINE2
    //    printf("[daheng]  hardware trigger!  ret=[%d]\n", ret);
}

bool VideoPanel::PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo)
{
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

        // ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name
        printf("--------------------Find HaiKang Device---------------\n");
        printf("Device Model Name: %s\n", pstMVDevInfo->SpecialInfo.stGigEInfo.chModelName);
        printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4);
        printf("UserDefinedName: %s\n\n" , pstMVDevInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
        printf("Device Model Name: %s\n", pstMVDevInfo->SpecialInfo.stGigEInfo.chModelName);
        printf("-------------------------------------------------------\n");
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


//只打开海康一个设备，并设回调
void VideoPanel::setHaikang(int nIndex){
    handle = NULL;
    int nRet;
    //枚举海康能找到的相机
    MV_CC_DEVICE_INFO_LIST stDeviceList;
    memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

    // 枚举设备
    // enum device
    nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
    if (MV_OK != nRet)
    {
        printf("MV_CC_EnumDevices fail! nRet [%x]\n", nRet);
        return;
    }

    if (stDeviceList.nDeviceNum > 0)
    {
        for (int i = 0; i < stDeviceList.nDeviceNum; i++)
        {
            printf("[device %d]:\n", i);
            MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
            if (NULL == pDeviceInfo)
            {
                return;
            }
            PrintDeviceInfo(pDeviceInfo);
        }
    }else{
        printf("Find No Devices!\n");
        return;
    }

    nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]);
    if (MV_OK != nRet)
    {
        printf("MV_CC_CreateHandle fail! nRet [%x]\n", nRet);
        return;
    }

    // 打开设备
    // open device
    nRet = MV_CC_OpenDevice(handle);
    if (MV_OK != nRet)
    {
        printf("MV_CC_OpenDevice fail! nRet [%x]\n", nRet);
        return;
    }

    // ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
    if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE)
    {
        int nPacketSize = MV_CC_GetOptimalPacketSize(handle);
        if (nPacketSize > 0)
        {
            nRet = MV_CC_SetIntValue(handle,"GevSCPSPacketSize",nPacketSize);
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

    // set trigger mode as on
    nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 1);
    if (MV_OK != nRet)
    {
        printf("MV_CC_SetTriggerMode fail! nRet [%x]\n", nRet);
        return;
    }

    // 设置触发源
    // set trigger source
    nRet = MV_CC_SetEnumValue(handle, "TriggerSource", MV_TRIGGER_SOURCE_SOFTWARE);
    if (MV_OK != nRet)
    {
        printf("MV_CC_SetTriggerSource fail! nRet [%x]\n", nRet);
        return;
    }

    // 注册抓图回调
    // register image callback
    nRet = MV_CC_RegisterImageCallBackEx(handle, hkImageCallBack, handle);
    if (MV_OK != nRet)
    {
        printf("MV_CC_RegisterImageCallBackEx fail! nRet [%x]\n", nRet);
        return;
    }

    // 开始取流
    // start grab image
    nRet = MV_CC_StartGrabbing(handle);
    if (MV_OK != nRet)
    {
        printf("MV_CC_StartGrabbing fail! nRet [%x]\n", nRet);
        return;
    }

}

//void VideoPanel::mkdir(char char* path){
//    int ret = mkdir(path.c_str());
//    if (ret && errno == EEXIST)
//    {
//        cout << "dir: " << path << " aleardy exist" << endl;
//    }
//    else if (ret)
//    {
//        cout << "create dir error: " << ret << ", :" << strerror(errno) << endl;
//    }
//    else
//    {
//        cout << "mkdir create dir succ: " << path << endl;
//    }
//}

void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static QMutex mutex;

    mutex.lock();
    QString text;

    switch(type)

    {

    case QtDebugMsg:

        text = QString("Debug:");

        break;



    case QtWarningMsg:

        text = QString("Warning:");

        break;



    case QtCriticalMsg:

        text = QString("Critical:");

        break;



    case QtFatalMsg:

        text = QString("Fatal:");

    }


    QString current_date_time = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString message = QString("%1 %2\n").arg(msg).arg(current_date_time);

    QFile file("/root/cover_result/"+enterTime+"_log.txt");

    file.open(QIODevice::WriteOnly | QIODevice::Append);

    QTextStream text_stream(&file);

    text_stream << message << endl;

    file.flush();

    file.close();



    mutex.unlock();

}

//void signalHandler( int signum )
//{
//    cout << "Interrupt signal (" << signum << ") received.\n";
//    qDebug("Find mem error!\n");
//    longNum=longNum+100;
//    emit vcamdevice[0]->m_v->sendui();
//}

int JsonIORead(QString path)
{
        Json::Reader reader;
        Json::Value root;
        std::ifstream in(path.toLatin1(), std::ios::binary);

        if (reader.parse(in, root))
        {
            Json::Value devalue=root["runConfig"];

            if (!devalue.empty())
            {
                cfg_isSavePic=devalue["isSavePic"].asInt();
                cfg_abDelay=devalue["abDelay"].asInt();
                cfg_cdDelay=devalue["cdDelay"].asInt();
                cfg_coverWidth=devalue["coverWidth"].asInt();
                cfg_ironWidth=devalue["ironWidth"].asInt();
                printf("**************videopanel read config %d\n",cfg_isSavePic);
            }

        }
    return 0;
}

void VideoPanel::initForm()
{
    JsonIORead("/root/run_config.json");
    qInstallMessageHandler(outputMessage);
    enterTime = QDateTime::currentDateTime().toString("MM-dd_hh-mm");

    //signal(SIGSEGV, signalHandler);
    qDebug("Begin Time");

    //设置样式表
#ifndef no_style
    QStringList qss;
    qss.append("QFrame{border:2px solid #000000;}");
    qss.append("QLabel{font:75 25px;color:#F0F0F0;border:2px solid #AAAAAA;background:#303030;}");
    qss.append("QLabel:focus{border:2px solid #00BB9E;background:#555555;}");
    this->setStyleSheet(qss.join(""));
#endif



    videoMax = false;
    videoCount = 4;
    videoType = "1_4";  //0_1


    for (int i = 0; i < videoCount; ++i) {
        vcamdevice[i] = new CamDevice;
        vcamdevice[i]->mlabel = new QLabel;
        vcamdevice[i]->mlabel->setObjectName(QString("video%1").arg(i + 1));
        vcamdevice[i]->mlabel->installEventFilter(this);
        vcamdevice[i]->mlabel->setFocusPolicy(Qt::StrongFocus);
        vcamdevice[i]->mlabel->setAlignment(Qt::AlignCenter);
        vcamdevice[i]->mlabel->setScaledContents(true);
        vcamdevice[i]->mlabel->setFrameShape(QFrame::Box);
        vcamdevice[i]->mlabel->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);

        //     vcamdevice[i]->mlabel->setText(QString("通道 %1").arg(i + 1));

        widgets << vcamdevice[i]->mlabel;
        vcamdevice[i]->m_pcMyCamera = new   CMvCamera;
        vcamdevice[i]->m_gxCamera = new   GXCamera;
        vcamdevice[i]->m_imvCamera = new   IMVCamera;
        vcamdevice[i]->id=i;
        vcamdevice[i]->m_hWnd=(void *)vcamdevice[i]->mlabel->winId();
        vcamdevice[i]->m_v=this;
        printf("i=%d,vcamdevice[0]->m_v=%d\n",i, vcamdevice[i]->m_v);
        printf("i=%d,vcamdevice[0]->m_hWnd=%d\n",i, vcamdevice[i]->m_hWnd);
    }


    //printf("init_camera started\n");
    GX_STATUS       emStatus = GX_STATUS_SUCCESS;
    uint32_t ui32DeviceNum = 0;

    //Initialize libary
    emStatus = GXInitLib();
    //Get device enumerated number
    emStatus = GXUpdateDeviceList(&ui32DeviceNum, 1000);

    //If no device found, app exit
    if (ui32DeviceNum > 0)
    {
        setDaheng(ui32DeviceNum);
        printf("Set Daheng OK！");
        GXCloseLib();
        return;
    }

    printf("Daheng device not found\n");
    GXCloseLib();

    //枚举大华软件能找到的相机
    IMV_DeviceList deviceInfoList;
    int ret = IMV_EnumDevices(&deviceInfoList, interfaceTypeAll);
    if (IMV_OK != ret)
    {
        printf("Enumeration devices failed! ErrorCode[%d]\n", ret);
    }else{
        printf("find Dahua camera,total num=%d pcs\n",deviceInfoList.nDevNum);
    }

//    // 打印相机基本信息（序号,类型,制造商信息,型号,序列号,用户自定义ID,IP地>址）
//    // Print camera info (Idx, Type, Vendor, Model, Serial number, DeviceUserID, IP Address)
    displayDeviceInfo(deviceInfoList);
    int nRet = vcamdevice[0]->m_imvCamera->Open("192.168.1.10",1,"/root/Shitai1.mvcfg");//"/root/Desktop/10.mvcfg"

    if (nRet == 0){
        printf("[dahua1 Open success!]   ret=%d\n", nRet);
    }else{
        printf("[dahua1 Open error!]   ret=%d\n", nRet);
        statusStr="相机没有找到";
    }

    nRet=vcamdevice[0]->m_imvCamera->RegisterImageCallBack(imvImageCallBack, vcamdevice[0]);
    if (IMV_OK == nRet)
    {
        printf("[dahua1 register success!]   ret=%d\n", nRet);
    }else{
        printf("[dahua1 register fail!]   ret=%d \n",nRet);
        statusStr="相机注册失败";
    }

    nRet=vcamdevice[0]->m_imvCamera->StartGrabbing();
    if (IMV_OK == nRet)
    {
        printf("[dahua1 start Grabbing success!]   ret=%d\n", nRet);
    }else{
        printf("[dahua1 start Grabbing fail!]   ret=%d\n", nRet);
    }

    //Open camera2
    nRet = vcamdevice[1]->m_imvCamera->Open("192.168.2.11",1,"/root/Shitai2.mvcfg");

    if (nRet == 0){
        printf("[dahua2 Open success!]   ret=%d\n", nRet);
    }else{
        printf("[dahua2 Open error!]   ret=%d\n", nRet);
    }

    nRet=vcamdevice[1]->m_imvCamera->RegisterImageCallBack(imvImageCallBack2, vcamdevice[1]);
    if (IMV_OK == nRet)
    {
        printf("[dahua2 register success!]   ret=%d\n", nRet);
    }else{
        printf("[dahua2 register fail!]   ret=%d \n",nRet);
    }

    nRet=vcamdevice[1]->m_imvCamera->StartGrabbing();
    if (IMV_OK == nRet)
    {
        printf("[dahua2 start Grabbing success!]   ret=%d\n", nRet);
    }else{
        printf("[dahua2 start Grabbing fail!]   ret=%d\n", nRet);
    }

    //init NG and OK line
    //emit vcamdevice[0]->m_v->modbussend_result(1);

    //设置海康相机，默认第一个
    //unsigned int nIndex = 0;
    //setHaikang(nIndex);

    //Create batch root
    createBatchRoot();
}


void VideoPanel::initMenu()
{
    videoMenu = new QMenu(this);

    //单独关联信号槽
    actionFull = new QAction("切换全屏模式", videoMenu);
    connect(actionFull, SIGNAL(triggered(bool)), this, SLOT(full()));
    actionPoll = new QAction("软件 启动", videoMenu);
    connect(actionPoll, SIGNAL(triggered(bool)), this, SLOT(pollSoftware()));
    actionPollHardware = new QAction("硬件 Stop", videoMenu);
    connect(actionPollHardware, SIGNAL(triggered(bool)), this, SLOT(pollHardware()));

    //通过QAction类方式添加子菜单
    videoMenu->addAction(actionFull);
    videoMenu->addAction(actionPoll);
    videoMenu->addAction(actionPollHardware);
    videoMenu->addSeparator();

    //直接通过文字的形式添加子菜单
    //    videoMenu->addAction("截图当前视频", this, SLOT(snapshot_video_one()));
    //    videoMenu->addAction("截图所有视频", this, SLOT(snapshot_video_all()));
    //    videoMenu->addSeparator();

    //实例化通道布局类
    videoBox = new VideoBox(this);
    QList<bool> enable;
    enable << true << true << true << true;
    videoBox->initMenu(videoMenu, enable);
    videoBox->setVideoType(videoType);
    videoBox->setLayout(gridLayout);
    videoBox->setWidgets(widgets);
    videoBox->show_video_all();
}

void VideoPanel::full()
{
    if (actionFull->text() == "切换全屏模式") {
        fullScreen(true);
        actionFull->setText("切换正常模式");
    } else {
        emit fullScreen(false);
        actionFull->setText("切换全屏模式");
    }

    //执行全屏处理
}

void VideoPanel::pollSoftware()
{
    _numCallback = 0;
    timer_poll();
//    for(int i=0;i<100;i++){

//    }
}

#include <sys/stat.h>
int testFlag=0;
int writeNum=0;
void VideoPanel::timer_poll()
{
    //vcamdevice[0]->m_imvCamera->SetIO("Line3");

//    int width = 1400;
//    int height = 3500;
//    int halfHeight = height/2;

//    printf("[%s] @@@@@@@@@@@@start zl软触发cut pic  %d\n", __FUNCTION__, _numCallback);
//    char* buf = (char*)malloc(width*height);
//    FILE* fp = fopen("/root/cover_result/soft.raw", "rb");
//    if (fp)
//    {
//        int res = fread(buf, 1,width*height, fp);
//        fclose(fp);

//    }

//    int i = 0;
//    int j = 0;
//    int j2 = 0;
//    unsigned char pt;

//    unsigned char* rgb242 = (unsigned char*)malloc(width*height/2);
//    unsigned char* rgb24 = (unsigned char*)malloc(width*height/2);

//    for (i = 0; i < halfHeight; i++) {
//        memcpy(rgb242+i*width,buf+i*2*width,width);
//        memcpy(rgb24+i*width,buf+i*2*width+width,width);
//    }
//    free(buf);

//    QTime startTime=QTime::currentTime();
//    QImage image(rgb24,width,halfHeight,QImage::Format_Grayscale8);
//    QString qpath=QString("/root/cover_result/%1_%2_0_a.bmp").arg(QDateTime::currentDateTime().toString("MM_dd_hh_mm")).arg(0);
//    image.save(qpath,"BMP",100);


//    QImage image2(rgb242,width,halfHeight,QImage::Format_Grayscale8);
//    QString qpath2=QString("/root/cover_result/%1_%2_0_b.bmp").arg(QDateTime::currentDateTime().toString("MM_dd_hh_mm")).arg(0);
//    image2.save(qpath2,"BMP",100);

//    QTime endTime=QTime::currentTime();
//    int distant=startTime.msecsTo(endTime);
//    printf("@@@@Save a bmp time:%d\n",distant);

//    emit vcamdevice[0]->m_v->sendCallbackImg(image.scaled(vcamdevice[0]->mlabel->width(), vcamdevice[0]->mlabel->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation), vcamdevice[0]->mlabel);
//    emit vcamdevice[1]->m_v->sendCallbackImg(image2.scaled(vcamdevice[1]->mlabel->width(), vcamdevice[1]->mlabel->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation), vcamdevice[1]->mlabel);

//    showNode sd;
//    sd.pushNum=pushNumA;
//    showList.push_back(sd);
//    emit vcamdevice[0]->m_v->emit_infer(qpath,0,0);
//    emit vcamdevice[0]->m_v->emit_infer(qpath2,1,0);
//    emit vcamdevice[0]->m_v->emit_infer(qpath,2,0);
//    emit vcamdevice[0]->m_v->emit_infer(qpath2,3,0);
//    printf("before write io\n");

    // write_io(0,0);
//    write_io(1,0);

//    vcamdevice[0]->m_imvCamera->Close();
//    vcamdevice[1]->m_imvCamera->Close();
    //write_io(0,testFlag);
    //write_io(1,testFlag);
    //testFlag=1-testFlag;

//    while(1){
//        write_io(1,0);
//        writeNum++;
//        printf("writeNum%d\n",writeNum);
//        Sleep(100);
//    }
    //QTimer::singleShot(1000, this, SLOT(play_video_all()));
//    QTimer *timer=new QTimer(this);
//    connect(timer,SIGNAL(timeout()),this,SLOT(play_video_all()));
//    timer->start(100);

//    int rnum;
//    for(int i=0;i<10;i++){
//      rnum=rand()%100;
//      printf("rnum%d\n",rnum);
//    }++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    QDateTime nowTime=  QDateTime::currentDateTime();
    //add to lightList
    lightList2.push_back(nowTime.toMSecsSinceEpoch());

    QTime t;
    t.start();
    while(t.elapsed()<300){
        printf("%d\n",t.elapsed());
    }

    nowTime=  QDateTime::currentDateTime();
    //add to lightList
    lightList2.push_back(nowTime.toMSecsSinceEpoch());

    printf("write mem end\n");
}


// hardware
void VideoPanel::pollHardware()
{
    printf("[%s] start stop!!!  \n", __FUNCTION__);
    write_stop();
}


// PLC state :
void VideoPanel::modbusRecvPLCstate(int value){
    if (value == 3){
        _isPLCError = true;
    }
    else if(value == 4) {  // 只清空OK NG数量,不清空状态
        //        pushNum = 0;
        _numOK = 0;
        _numNG = 0;
        _numTotal = 0;
        //emit vcamdevice[0]->m_v->sendui(0, 0, 0);
        //        isFirst = 0;
        //        _isCallbackError = false;
        //        raIndex = 0;
        //        resultLen = 0;
    }
    else {
        _isPLCError = false;
    }
}

void VideoPanel::myslot()
{

}


void VideoPanel::play_video_all()
{
    if(writeNum>100){
        return;
    }
    write_io(1,0);
    writeNum++;

    printf("writeNum%d\n",writeNum);
}

void VideoPanel::snapshot_video_one()
{

}

void VideoPanel::snapshot_video_all()
{

}
