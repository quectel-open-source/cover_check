#ifndef VIDEOPANEL_H
#define VIDEOPANEL_H

/**
 * 视频监控画面控件 整理:feiyangqingyun(QQ:517216493) 2019-04-11
 * 1. 可设定视频通道数量。
 * 2. 支持双击最大化再次双击还原。
 * 3. 支持4/6/8/9/13/16/25/36/64等通道布局。
 * 4. 内置了选中边框高亮等样式。
 * 5. 通用的视频通道布局盒子类，方便拓展其他布局。
 */
#include "AlgoThread.h"
#include "MemThread.h"
#include "LightThread.h"
#include <QWidget>
#include "MvCamera.h"
#include "GXCamera.h"
#include "IMVCamera.h"
#include <QMap>
#include <QMutex>
#define SAVE_PIC 0
#define INFER_ENABLE 1
#define SAVE_IN_INFER 1
#define SAVE_WAVE 0
#define MAX_SHOW 100

class QMenu;
class QLabel;
class QGridLayout;
class VideoBox;

class JsonIO
{
public:
    explicit JsonIO();
    ~JsonIO();

    void read();
    void write();

};

struct  CamResult
{
    int  Camid;
    int  Checkid;
    int CheckRet[4];
};

#ifdef quc
class Q_DECL_EXPORT VideoPanel : public QWidget
#else
class VideoPanel : public QWidget
#endif

{
    Q_OBJECT

public:
    explicit VideoPanel(QWidget *parent = 0);
    void static __stdcall hkImageCallBack(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser);
    void static __stdcall gxImageCallBack(GX_FRAME_CALLBACK_PARAM *pFrameData);
    void static __stdcall imvImageCallBack(IMV_Frame* pFrame, void* pUser);
    void static __stdcall imvImageCallBack2(IMV_Frame* pFrame, void* pUser);

    void setHaikang(int num);
    void setDaheng(int num);

    bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo);
    int onePush();
    void loadHistoryResult();

    int  checktotall;
    int checkng;
    int checkok;
    int  num;
    int  sqlstartvalue;
    int CamRet[100];

    void* handle;

    QMap<QImage, QLabel*> _map;
    int _numCallback;
    QTimer* _timer;
    int _timeTotal;
    void test(const char * pData);

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    bool videoMax;              //是否最大化
    int videoCount;             //视频通道个数
    QString videoType;          //当前画面类型
    QMenu *videoMenu;           //右键菜单
    QAction *actionFull;        //全屏动作
    QAction *actionPoll;        //轮询动作
    QAction *actionPollHardware;

    QGridLayout *gridLayout;    //通道表格布局
    QWidgetList widgets;        //视频控件集合
    VideoBox *videoBox;         //通道布局类
    GX_DEV_HANDLE   gx_hDevice[15];
    int64_t g_i64ColorFilter[15];

    IMV_DeviceList deviceInfoList;
    IMV_DeviceInfo	pDevInfo_0 ;

    MV_CC_DEVICE_INFO_LIST  m_stDevList;

    MV_CC_DEVICE_INFO stDevInfo_0 = {0};
    MV_GIGE_DEVICE_INFO stGigEDev_0 = {0};
    MV_CC_DEVICE_INFO stDevInfo_1 = {0};
    MV_GIGE_DEVICE_INFO stGigEDev_1 = {0};
    MV_CC_DEVICE_INFO stDevInfo_2 = {0};
    MV_GIGE_DEVICE_INFO stGigEDev_2 = {0};
    MV_CC_DEVICE_INFO stDevInfo_3 = {0};
    MV_GIGE_DEVICE_INFO stGigEDev_3 = {0};

public:
    QSize sizeHint()            const;
    QSize minimumSizeHint()     const;

int vtcpServerThreadCreate(void);
  int     vtcpServereDestroy(void);
 int     vfd;
     int vtcpConnectState;
private slots:
    void initControl();
    void initForm();
    void initMenu();
    void full();
    void pollSoftware();
    void myslot();

    void recvPicFromCallback(QImage image, QLabel *label);

public slots:
    void timer_poll();
    void pollHardware();
    void modbusRecvPLCstate(int value);

    void updateSendJson(int jsonId);

private slots:
    void play_video_all();
    void snapshot_video_one();
    void snapshot_video_all(); 
signals:
    //全屏切换信号
    void modbussend(int addr,QString sendstr);
    void modbusbeat();
    void modbussend_result(int value);
    void modbusAlarm(int value);
    void modbusRecv();

    void fullScreen(bool v);
    void senduitotall(int value);
    void senduing(int value);
    void senduiok(int value);
    void sendui();
    void sendstatus(QString str);

    void sendCallbackImg(QImage image, QLabel *label);
    void emit_infer(QString path,int flag,int pushNum);

    void emit_infer_memab(int flag,unsigned char* pa,unsigned char* pb,int pushNum,int width,int height);
    void emit_infer_memcd(int flag,unsigned char* pa,unsigned char* pb,int pushNum,int width,int height);

};
struct CamDevice
{
    int  id;
    QLabel *mlabel;
    void *m_hWnd;
    CMvCamera*              m_pcMyCamera;
    GXCamera*              m_gxCamera;
    IMVCamera*              m_imvCamera;
    bool                    m_bGrabbing;
    VideoPanel*        m_v;
};


#endif // VIDEOPANEL_H
