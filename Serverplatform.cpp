#ifndef __SERVER_CONNECT_CPP__
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
#include "Util.hpp"


#include "packetParser.hpp"
#include "Serverplatform.h"

#define UDP_PORT 6111
#define MAXLINE 1024 
using namespace std;

struct TestBox {
    char header;
    short msgId;
    long msgNo;
};

Serverplatform::Serverplatform(SettingConfig *settingConfig)
{
    int nRet = 0;
    //    int nRet = pthread_create(&udpThread, NULL, udpServer, this);
    //    if (nRet < 0)
    //    {
    //        std::cerr << "udpServer failed" << std::endl;
    //    }
    printf("start server plat,line=%d\n",__LINE__);
    this->settingConfig = settingConfig;
    nRet = pserverFrameProcCreate();//��������֡��������
    if (nRet < 0)
    {
        std::cerr << "serverFrame failed" << std::endl;
    }
    nRet = ptcpServerThreadCreate();//����tcp server rcv����
    if (nRet < 0)
    {
        std::cerr << "tcp Server failed" << std::endl;
    }
    printf("start server plat,line=%d\n",__LINE__);
    std::cerr << "tcp Server create" << std::endl;
}

void *Serverplatform::udpServer(void *params)
{
    int sockfd;
    return 0;
    struct PMessageBox messageBox;
    //char *hello = "Hello from server";
    struct sockaddr_in servaddr, cliaddr;
    Serverplatform *pThis = (Serverplatform *) params;

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(UDP_PORT);

    // Bind the socket with the server address
    if ( ::bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    int len, n;
    
    len = sizeof(cliaddr);  //len is value/result
    Json::Reader reader;
    Json::Value value;

    while (true ) {
        memset(&messageBox, 0, sizeof(messageBox));
        cout << "size of messageBox: " << sizeof(PMessageBox) << endl;
        n = recvfrom(sockfd, &messageBox, sizeof(messageBox), 0, (struct sockaddr *)&cliaddr, (socklen_t *)&len);
        cout << "len: " << n << endl;
        std::cout << "header : " << messageBox.header << std::endl;
        cout << "msgId : " << messageBox.msgId << endl;
        cout << "msgNo : " << messageBox.msgNo << endl;
        cout << "bodyLength : " << messageBox.bodyLength << endl;
        cout << "body : " << messageBox.body << endl;

        if (reader.parse(messageBox.body, value))
        {
            if (value["serverIp"].isNull() || value["serverPort"].isNull())
            {
                cerr << "recv error body" << endl;
                continue;
            }
            cerr << "update web server config value serverIp: " << value["serverIp"].asString() <<  endl;
            if (strcmp(pThis->settingConfig->serverConfig.server_ip, const_cast<char *>(value["serverIp"].asString().c_str())) != 0
                    && pThis->settingConfig->serverConfig.port != stoi(value["serverPort"].asString().c_str()))
            {
                //TODO add tcp reconnect
                pThis->settingConfig->updateConfigJson(value);
            }
        }
    }
    /*sendto(sockfd, (const char *)hello, strlen(hello),
        MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
            len);
    printf("Hello message sent.\n");  */

    return 0;
}

static void hexDumpData(char *buff, int size)
{
    int i;
    for (i=0; i<size; i++)
    {
        if(i%16 == 0)
        {
            printf("\n");
        }
        printf(" %02x ", buff[i]);
    }
    printf("\n");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

static long int get_unix_time(void)
{
    time_t timep;
    time(&timep);
    return (long int)timep;
}

/*
 * ���ź���ͬ��
 */
static sem_t semSend; 			//���ڷ������ݵı���
static sem_t semRecv;			//���ڽ������ݵ�ͬ��
static sem_t semStopRcv;		//����ֹͣ��ͬ��

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

/* 
 * author: kelvin
 * connect server
 */
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
    printf("[%s]serverFd=[%d] ret=[%d]\n", __func__, serverFd, ret);
    if (ret < 0)
    {
        perror("error: socket connect!\n");
        close(serverFd);
        return - 1;
    }
    return serverFd;
}

/* 
 * author: kelvin
 * send data to serve
 */
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


/*
 *
 */
static int sendDateToServer(int fd, char *data, int size)
{
    int ret;
    sem_post(&semSend);
    ret = _sendDateToServer(fd, data, size);
    sem_wait(&semSend);
    return ret;
}
extern CamDevice*  vcamdevice[9];





int Serverplatform::senddata(void *user,char *data, int size)
{

    sendDateToServer(pfd,data,size);
#if 0
    unsigned char* jpgBuffer = NULL;

    int jpg_size;

    jpgBuffer = rgb2jpg((const char *) (char *) (data+20), 2448,2048, 0, &jpg_size);
    char path[128];
    memset(path,0,128);
    sprintf(path,"/home/ol/pre_result_%d_%d.jpg",0,1);
    printf("path=%d",path);
    FILE* fp = fopen(path, "wb");
    if (fp)
    {

        int res = fwrite(jpgBuffer, 1, jpg_size, fp);
        fclose(fp);
        printf("%s @ line:%d,res:%d,dst_size %d\n",__func__,__LINE__,res, jpg_size);
    }
#endif
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static PMessageBoxData serverToClientFrame;
static PMessageBoxData responseFrame;

#define PACKET_HEAD_HEADER_OFFSET				(0)
#define PACKET_HEAD_MSGID_OFFSET				(1)
#define PACKET_HEAD_MSGNO_OFFSET				(3)
#define PACKET_HEAD_BODYLENGTH_OFFSET			(11)
#define PACKET_HEAD_BODYLENGTH_LENGTH			(2)
#define PACKET_HEAD_LEN							(PACKET_HEAD_BODYLENGTH_OFFSET + PACKET_HEAD_BODYLENGTH_LENGTH)

#define HEADER_FLAG								(127)

static char *PpacketHeadDataCreate(short int msgId, long int msgNo, short int body_length)
{
    char *buf = (char *)malloc(PACKET_HEAD_LEN);
    buf[PACKET_HEAD_HEADER_OFFSET] = HEADER_FLAG;
    memcpy(buf + PACKET_HEAD_MSGID_OFFSET, &msgId, sizeof(msgId));
    printf("[%s]msgNo len:%d\n", __func__, sizeof(msgNo));
    memcpy(buf + PACKET_HEAD_MSGNO_OFFSET, &msgNo, sizeof(msgNo));
    memcpy(buf + PACKET_HEAD_BODYLENGTH_OFFSET, &body_length, sizeof(body_length));
    return buf;
}


// ��ȡ��ϵͳ�����ĵ���������ʱ��
inline uint64_t getTimeConvSeconds(timespec* curTime, uint32_t factor)
{
    // CLOCK_MONOTONIC����ϵͳ������һ����ʼ��ʱ,����ϵͳʱ�䱻�û��ı��Ӱ��
    clock_gettime( CLOCK_MONOTONIC, curTime );
    return static_cast<uint64_t>(curTime->tv_sec) * factor;
}

// ��ȡ��ϵͳ�����ĵ���������ʱ�� -- ת����λΪ΢��
static uint64_t getMonnotonicTime(void)
{
    timespec curTime;
    uint64_t result = getTimeConvSeconds( &curTime, 1000000 );
    result += static_cast<uint32_t>(curTime.tv_nsec) / 1000;
    return result;
}

// sem_trywait + usleep�ķ�ʽʵ��
// ����ź�������0��������ź�������������true
// ����ź���С��0���������ȴ�����������ʱʱ����false
static bool semWaitTimeout(sem_t *sem, size_t timeout)
{
    const size_t timeoutUs = timeout * 1000;        // ��ʱʱ���ɺ���ת��Ϊ΢��
    const size_t maxTimeWait = 10000;               // ����˯�ߵ�ʱ��Ϊ10000΢�룬Ҳ����10����
    size_t timeWait = 1;                            // ˯��ʱ�䣬Ĭ��Ϊ1΢��
    size_t delayUs = 0;                             // ʣ����Ҫ��ʱ˯��ʱ��
    const uint64_t startUs = getMonnotonicTime();   // ѭ��ǰ�Ŀ�ʼʱ�䣬��λ΢��
    uint64_t elapsedUs = 0;                         // ����ʱ�䣬��λ΢��
    int ret = 0;
    do
    {
        // ����ź�������0��������ź�������������true
        if( sem_trywait(sem) == 0 )
        {
            return true;
        }
        // ϵͳ�ź�����������false
        if( errno != EAGAIN )
        {
            return false;
        }
        // delayUsһ���Ǵ��ڵ���0�ģ���Ϊdo-while��������elapsedUs <= timeoutUs.
        delayUs = timeoutUs - elapsedUs;
        // ˯��ʱ��ȡ��С��ֵ
        timeWait = std::min( delayUs, timeWait );
        // ����˯�� ��λ��΢��
        ret = usleep( timeWait );
        if( ret != 0 )
        {
            return false;
        }
        // ˯����ʱʱ��˫������
        timeWait *= 2;
        // ˯����ʱʱ�䲻�ܳ������ֵ
        timeWait = std::min( timeWait, maxTimeWait );
        // ���㿪ʼʱ�䵽���ڵ�����ʱ�� ��λ��΢��
        elapsedUs = getMonnotonicTime() - startUs;
    } while( elapsedUs <= timeoutUs ); // �����ǰѭ����ʱ�䳬��Ԥ����ʱʱ�����˳�ѭ��
    // ��ʱ�˳����򷵻�false
    return false;
}

#define SEM_WAIT_TIMEOUT    (1000 * 3)

static void messageBoxDataShow(PMessageBoxData *message);

static int waitPacketResponse(long int msgNo)
{
    int ret;
    struct timespec ts;
    //�ȴ��յ��ظ�����������һ����ʱ���ƣ�����һ��ʱ����϶�Ϊ����
    ret = semWaitTimeout(&semRecv, SEM_WAIT_TIMEOUT);//����һ����ʱ�ȴ�����
    if(ret == false)
    {
        printf("timeout wait\n");
        return -1;
    }
    if(responseFrame.head.msgNo != msgNo)
    {
        printf("[%s]err msgNo,%ld,%ld\n", __func__, msgNo, responseFrame.head.msgNo);
    }
    messageBoxDataShow(&responseFrame);
    ret = messageResponseJsonBodyParse(responseFrame.body);
    return ret;
}

static  sem_t *getWaitPacketResponseSem(void)
{
    return &semRecv;
}

static void packetDataShow(char *data, int size)
{
    int i;
    printf("\n");
    for(i=0; i<size; i++)
    {
        printf(" %02x ", data[i]);
    }
    printf("\nbody:\n[%s]\n", data+PACKET_HEAD_LEN);
}

/*
 * ���͸������������ݵĽӿ�
 * parms: responseFlag 1:��ʾ��Ҫ�ȴ��ظ���0������Ҫ
 */
static int sendPacketToServer(int fd, char *data, int size, short int msgId, long int msgNo, int responseFlag)
{
    int ret = 0;
    printf("[%s]msgNo:%ld,size:%d\n", __func__, msgNo, size);
    char *head = PpacketHeadDataCreate(msgId, msgNo, (short int)size);
    sem_wait(&semSend);

    char *buf = (char *)malloc(PACKET_HEAD_LEN + size);
    memcpy(buf, head, PACKET_HEAD_LEN);
    memcpy(buf + PACKET_HEAD_LEN, data, size);
    _sendDateToServer(fd, buf, size + PACKET_HEAD_LEN);
    packetDataShow(buf, size + PACKET_HEAD_LEN);
    free(buf);
    free(head);
    if (responseFlag)
    {
        ret = waitPacketResponse(msgNo);
    }
    sem_post(&semSend);
    return ret;
}

static void messageBoxDataShow(PMessageBoxData *message)
{
    printf("[%s]head:%d\n", __func__, message->head.header);
    printf("[%s]msgId:%d\n", __func__, message->head.msgId);
    printf("[%s]msgNo:%ld\n", __func__, message->head.msgNo);
    printf("[%s]bodyLength:%d\n", __func__, message->head.bodyLength);
    printf("[%s]body:%s\n\n\n", __func__, message->body);
    hexDumpData(message->body, message->head.bodyLength);
}

static  short int frameMsgIdGet(char *buf)
{
    short int value = (((PMessageBoxHead *)buf)->msgId);
    printf("[%s]msgId:%d\n", __func__, value);
    return value;
}

static void forwardPacketResponseToClient(char *buff)
{
    memcpy(&responseFrame.head, buff, sizeof(PMessageBoxHead));
    responseFrame.body = (char *)malloc(responseFrame.head.bodyLength+1);
    memset(responseFrame.body, 0, responseFrame.head.bodyLength+1);
    memcpy(responseFrame.body, buff + sizeof(PMessageBoxHead), responseFrame.head.bodyLength);
    sem_post(&semRecv);//�ͷŵȴ�
}

static sem_t *semFrameStopGet(void);
static sem_t *semFrameRcvGet(void);

static void forwardPacketToFrameProc(char *buff)
{
    memcpy(&serverToClientFrame.head, buff, sizeof(PMessageBoxHead));
    serverToClientFrame.body = (char *)malloc(serverToClientFrame.head.bodyLength+1);
    memset(serverToClientFrame.body, 0, serverToClientFrame.head.bodyLength+1);
    memcpy(serverToClientFrame.body, buff + sizeof(PMessageBoxHead), serverToClientFrame.head.bodyLength);
    sem_post(semFrameRcvGet());//֪ͨFrame������frame
}

/*
 * ���������߳�
 * ��������֧��������˳�����
 */
typedef struct __MsgPicHead__
{
    int id; //相机ID
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
char *buf=NULL;
//extern ModbusTcpClient *modbusTcpClient;
static void* threadRecvEntry(void *parm)
{
    char buff[2048];
    int ret;
    int fd = (long int)parm;
    buf=(char *)malloc(25165824+20);//15040512
    char *pjpeg=buf;
    while(1)
    {
        printf("[%s]11cycle in\n", __func__);
        memset(pjpeg,0,25165824+20);//15040512
        ret = recv(fd, (char *)buf, 20, 0);
        if (ret <= 0)
        {
            printf("[%s]11recv error:%d\n", __func__, ret);
            break;
        }
        printf("[%s]11recv data len:%d\n", __func__, ret);
        MsgPicHead *mhead;
        mhead=(MsgPicHead*)malloc(sizeof(MsgPicHead));
        memcpy(mhead,buf,sizeof(MsgPicHead));

        hexDumpData(buf, ret);
        printf("[%s]22cycle in\n", __func__);
        printf("[%s]id =%d in\n", __func__,mhead->id);
        printf("[%s]msgnd =%d in\n", __func__,mhead->msgNo);
        printf("[%s]height =%d in\n", __func__,mhead->height);
        printf("[%s]width =%d in\n", __func__,mhead->width);
        printf("[%s]bodylength =%d in\n", __func__,mhead->bodylength);
        printf("buf=%d,pjpeg=%d\n",buf,pjpeg);
        memset(buf+20,0,mhead->bodylength);
        ret = 0;
        int recvt=0;
        buf=buf+20;
        while(recvt<mhead->bodylength)
        {
            ret = recv(fd, (char *)buf, mhead->bodylength, 0);
            recvt=recvt+ret;
            if (ret <= 0)
            {
                printf("[%s]22recv error:%d\n", __func__, ret);
                break;
            }

            buf = buf+ret;
            printf("recv len=%d,buf=%d,pjpeg=%d,recv=%d\n",ret,buf,pjpeg,recvt);
        }
        buf=pjpeg;
        printf("buf=%d,pjpeg=%d\n",buf,pjpeg);
        unsigned char* jpgBuffer = NULL;

        long jpg_size;


        hexDumpData((pjpeg+20), 40);
        printf("over,buff\n");
        // jpgBuffer = rgb2jpg((const char *) (char *) (pjpeg+20), 2448,2048, 0, &jpg_size);
        jpgBuffer = rgb2jpg((const char *) (char *) (pjpeg+20), mhead->width,mhead->height, 0, &jpg_size);
        hexDumpData((pjpeg+20), 40);
        printf("over,jpegbuff\n");
        char path1[128];
        memset(path1,0,128);
        sprintf(path1,"/home/user/result_%d_%d.jpg",mhead->id,mhead->msgNo);
        printf("path=%s",path1);
        FILE* fp = fopen(path1, "wb");
        if (fp)
        {

            int res = fwrite(jpgBuffer, 1, jpg_size, fp);
            fclose(fp);
            printf("%s @ line:%d,res:%d,dst_size %d\n",__func__,__LINE__,res, jpg_size);
        }



        sem_post(semFrameRcvGet());
#if 0
        if (frameMsgIdGet(buff) == MSG_ID_REPLY_COMMON)//�ǻظ���Ϣ��
        {
            forwardPacketResponseToClient(buff);
        }
        else//���յ��������������·�������
        {
            printf("[%s]\n", __func__);
            forwardPacketToFrameProc(buff);
        }
#endif
    }
    sem_post(&semStopRcv);
    pthread_exit(NULL);
}

//����һ����threadRecvEntry�Ĵ���˳����ƵĿ���

static int responseToServer(int fd, int result, int msgNo)
{
    char *resbuf = messageResponseJsonBodyCreate(result);
    int ret = sendPacketToServer(fd, resbuf, strlen(resbuf), PMSG_ID_REPLY_COMMON, msgNo, 0);
    return ret;
}

/*
 * �ϴ�ע����Ϣ
 */
int Serverplatform::pclientLogin(void)
{
    if (pfd < 0)
    {
        return -1;
    }
    char *body = messageLoginJsonBodyCreate(settingConfig->deviceStr, settingConfig->localIpaddr);
    int ret = sendPacketToServer(pfd, body, strlen(body), PMSG_ID_DEVICE_REGISTER, get_unix_time(), 1);
    free(body);
    return ret;
}

/*
 * �ϴ�������
 */
int Serverplatform::pclientHeadBeat(void)
{
    if (pfd < 0)
    {
        return -1;
    }
    if (ptcpConnectState != 1)
    {
        printf("wait headbeat,current tcp connect state disconnect\n");
        return -1;
    }
    char *body = messageHeatBeatJsonBodyCreate(settingConfig->deviceStr, settingConfig->localIpaddr);
    int ret = sendPacketToServer(pfd, body, strlen(body), PMSG_ID_HEART_BEAT, get_unix_time(), 1);
    free(body);
    return ret;
}

/*
 * �ϴ�����config�ļ�
 */
int Serverplatform::pclientConfigPush(void)
{
    if (pfd < 0)
    {
        return -1;
    }
    char *body = messageConfigJsonBodyCreate(CONFIG_JSON);
    int ret = sendPacketToServer(pfd, body, strlen(body), PMSG_ID_CONFIG_REPORT, get_unix_time(), 1);
    free(body);
    return ret;
}

#include <stdlib.h>
/*
 * �ϴ������
 */
int Serverplatform::pclientDetectResult(DetectResult *result)
{
    if (pfd < 0)
    {
        return -1;
    }
    if (ptcpConnectState != 1)
    {
        printf("current tcp connect state disconnect\n");
        return -1;
    }
    //this data is for test
    char    *channelNo;
    if(result->channelNo == 1)
    {
        channelNo              = "1";
    }
    else
    {
        channelNo              = "2";
    }
    char    *rawImgPath             = result->rawImgPath;
    char    *markImgPath            = result->markImgPath;

    char    *defectTypes            = result->defectTypes;
    char    *defectDesc             = result->defectDesc;
    long    snapTimeMillis          = result->snapTimeMillis;
    int     algorithmTimeConsuming  = result->algorithmTimeConsuming;
    int     checkTotalNumber        = result->checkTotalNumber;
    int     checkNgNumber           = result->checkNgNumber;
    double  yieldRate               = result->yieldRate;

    char long_buf[64];//��long ����ת����char��
    sprintf(long_buf, "%ld", snapTimeMillis);
    char *body = messageDetectResultJsonBodyCreate(channelNo, rawImgPath, markImgPath, defectTypes,
                                                   defectDesc, long_buf, algorithmTimeConsuming, checkTotalNumber, checkNgNumber, yieldRate);
    int ret = sendPacketToServer(pfd, body, strlen(body), PMSG_ID_OUTPUT_REPORT, get_unix_time(), 1);
    free(body);
    return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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
    sem_init(&semFrameRcv, 0, 0);
    sem_init(&semFrameStop, 0, 0);
}

static void messageFrameProcessDeInit(void)
{
    sem_destroy(&semFrameRcv);
    sem_destroy(&semFrameStop);
}

static void system_reboot(void)
{
    system("reboot");
}

/*
 * ���ڴ������յ�����Ϣ�����лظ�������
 */
void Serverplatform::pframeProcess(PMessageBoxData *message)
{
    int ret = false;
    struct PMessageBox box;
    memset(&box, 0, sizeof(struct PMessageBox));
    memcpy(&box, message, sizeof(PMessageBoxHead));
    memcpy(box.body, message->body, box.bodyLength);
    switch (box.msgId)
    {
    case PMSG_ID_DEVICE_REBOOT://reboot system
        system_reboot();
        break;
    default://process config json
        // ret = settingConfig->updateConfigJson(box);
        break;
    }
    responseToServer(pfd, ret, message->head.msgNo);
}

/*
 * �������յ���packet�߳�
 * ��������֧��������˳�����
 */
static int frame_stop = 0;
static  void* threadFrameProcEntry(void *params)
{
    frame_stop = 0;
    Serverplatform *pThis = (Serverplatform *)params;
    while (1)
    {
        printf("[%s]cycle in\n", __func__);
        sem_wait(&semFrameRcv);
        if (frame_stop == 1)
        {
            printf("[%s]exit from frame proc entry\n", __func__);
            break;
        }
        printf("[%s]get one frame from server\n", __func__);
        //messageBoxDataShow(&serverToClientFrame);
        //����frame;
        // pThis->pframeProcess(&serverToClientFrame);
        free(serverToClientFrame.body);
    }
    sem_post(&semFrameStop);
    pthread_exit(NULL);
}

static pthread_t	frameProcThreadId	= -1;

/*
 * ����һ�����ڽ���udp��tcp�����ݱ���
 */
int Serverplatform::pserverFrameProcCreate(void)
{
    messageFrameProcessInit();
    int ret = pthread_create(&frameProcThreadId, NULL, threadFrameProcEntry, (void *)this);
    usleep(1000 * 10);
    return ret;
}

int Serverplatform::pserverFrameProcStop(void)
{
    frame_stop = 1;
    sem_post(&semFrameRcv);
    sem_wait(&semFrameStop);
    messageFrameProcessDeInit();
    return 0;
}

static pthread_t	recvThreadId		= -1;
/*
 * ���ڴ������մ����������·������ݵ�֡
 * ����Ƿ��̰߳�ģ���ʱ��ɾ��
 */
int Serverplatform::ptcpServerCreate(void)
{
    return 0;
    int ret = 0;
    connectServerInit();
    ptcpConnectState = 0;
    //�ȴ�һ����Ч��IP��ַ��port�˿�
    //����ͨ���������ж����¼��ж�ͬ�������ö���..
    while (strlen(settingConfig->serverConfig.server_ip) < 6)
    {
        sleep(1);
    }
reConnect:
    pfd = connectServer(settingConfig->serverConfig.server_ip, settingConfig->serverConfig.port);
    if (pfd < 0)
    {
        sleep(1);
        printf("[%s]connect ip:%s,port:%d error\n", __func__, settingConfig->serverConfig.server_ip, settingConfig->serverConfig.port);
        goto reConnect;
    }
    pthread_attr_t attr;
    pthread_attr_init (&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&recvThreadId, &attr, threadRecvEntry, (void *)pfd);
    usleep(1000 * 10);
    ret = pclientLogin();
    if (!ret)
    {
        ptcpServereDestroy();
        goto reConnect;
    }
    ptcpConnectState = 1;
    pclientConfigPush();
    return ret;
}

#define HEART_BEAT_TIME     (30)   //��λs
#define BAND_IP_ADDRESS    "127.0.0.1"
#define SERVER_IP_ADDRESS  "127.0.0.1"
#define PORT 8883
void *ptcpServerThread(void *params)
{
    printf("[%s]wait for server ip and port\n", __func__);
    int ret = 0;
    Serverplatform *pThis = (Serverplatform *)params;
    connectServerInit();
    pThis->ptcpConnectState = 0;
    printf("[%s]wait for server ip and port\n", __func__);
    //�ȴ�һ����Ч��IP��ַ��port�˿�
    //����ͨ���������ж����¼��ж�ͬ�������ö���..
    //    while (strlen(pThis->settingConfig->serverConfig.server_ip) < 6)
    //    {
    //        printf("[%s]wait for server ip and port\n", __func__);
    //        sleep(1);
    //    }
    printf("start server plat,line=%d\n",__LINE__);
reConnect:
    //  pThis->pfd = connectServer(pThis->settingConfig->serverConfig.server_ip, pThis->settingConfig->serverConfig.port);
    printf("[%s]wait for server ip and port\n", __func__);
    pThis->pfd = connectServer(BAND_IP_ADDRESS, PORT);
    if (pThis->pfd < 0)
    {
        printf("[%s]connect ip:%s,port:%d error\n", __func__, pThis->settingConfig->dataOutput.dest_ip,
               pThis->settingConfig->dataOutput.dest_port);
        sleep(2);
        goto reConnect;
    }
    printf("start server plat,line=%d\n",__LINE__);
    ret = pthread_create(&recvThreadId, NULL, threadRecvEntry, (void *)pThis->pfd);
    usleep(1000 * 10);
#if 0
    ret = pThis->pclientLogin();
    if (!ret)
    {
        pThis->ptcpServereDestroy();
        //goto reConnect;
    }
    pThis->ptcpConnectState = 1;
    pThis->pclientConfigPush();
#endif

    while(1)
    {

        ret = pThis->pclientHeadBeat();
        if (!ret)
        {
            pThis->ptcpServereDestroy();
            goto reConnect;
        }
        sleep(HEART_BEAT_TIME);
    }
    pthread_exit(NULL);
}



static pthread_t tcpServerThreadId;
int Serverplatform::ptcpServerThreadCreate(void)
{

    static pthread_attr_t attr;
    pthread_attr_init(&attr);
    printf("start server plat,line=%d\n",__LINE__);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    int nRet = pthread_create(&tcpServerThreadId, &attr, ptcpServerThread, this);
    if (nRet < 0)
    {
        std::cerr << "tcpServer failed" << std::endl;
    }
    return nRet;
}



int Serverplatform::ptcpServereDestroy(void)
{
    //ֹͣrcv�߳�
    if(pfd > 0)
    {
        close(pfd);
    }
    sem_wait(&semStopRcv);
    //connectServerDeInit();
    pfd = -1;
    return 0;
}

int Serverplatform::ptcpServerRestart(void)
{
    ptcpServereDestroy();
    ptcpServerCreate();
}
#endif
