#include "MemThread.h"
#include "videopanel.h"
#include <QLabel>
#include <iostream>
#include <stdio.h>
using namespace std;

extern vector<qint64> lightList1;
extern vector<qint64> lightList2;
extern void write_io(int port,int value);

QMutex light1Mutex;
QMutex light2Mutex;

MemThread::MemThread(QObject *parent) : QThread(parent)
{

}

#define DEV_PATH "/dev/mem"
//extern unsigned char *memp1;
//extern unsigned char *memp2;

extern int gpioNG;

//Zhangleo add for cover check
int MemThread::read_gpio(){
    int ret=1;
    unsigned int    pgoffset, pgaddr;
    unsigned int    pagesize = sysconf(_SC_PAGESIZE);
    unsigned int    memaddr;
    unsigned char *p;
    int fd;

    fd = open(DEV_PATH, O_RDWR);
    if (fd <= 0) {
            printf("open error: %s\n", DEV_PATH);
            gpioNG=1;
            return 1;
    }

    //printf("enter mem thread %d\n",camera);
    memaddr  = 0xfd6e08a0;

    pgoffset = memaddr & (pagesize -1);
    pgaddr   = memaddr & ~(pagesize -1);
    p = (unsigned char*)mmap(NULL, pagesize, PROT_READ, MAP_SHARED, fd, pgaddr);
    if (p==0) {
        printf("readd mmap fail\n");
        gpioNG=1;
        return 1;
    }

    if(*((unsigned short *)(p + pgoffset))==0x0100){
        ret=0;
    }

    munmap(p, pagesize);
    close(fd);
    return ret;
}

int saveNum=0;
int countNum=0;
extern bool isSuspend;
extern int pushNumA;
extern int pushNumB;
extern int ironWidth;
extern int longNum;
extern int shortNum;
extern showNode showArray[MAX_SHOW];

int findNum=0;
int showNum=0;
int suspendNum=0;

extern int cfg_coverWidth;
extern int cfg_ironWidth;

void MemThread::run()
{
    int ret=1;
    unsigned int    pgoffset, pgaddr;
    unsigned int    pagesize = sysconf(_SC_PAGESIZE);
    unsigned int    memaddr;
    unsigned char *p;
    int fd;

    fd = open(DEV_PATH, O_RDWR);
    if (fd <= 0) {
            printf("open error: %s\n", DEV_PATH);
            gpioNG=1;
            return;
    }

    memaddr  = 0xfd6e08a0;

    pgoffset = memaddr & (pagesize -1);
    pgaddr   = memaddr & ~(pagesize -1);
    p = (unsigned char*)mmap(NULL, pagesize, PROT_READ, MAP_SHARED, fd, pgaddr);

    if (p==0) {
        printf("readd mmap fail\n");
        gpioNG=1;
        return;
    }

    int testNum=0;
    while(1){
        msleep(1);

        //test
//        testNum++;
//        if(testNum%1000==0){
//           write_io(0,1);
//           write_io(1,1);
//        }

        if(isSuspend==true){
             suspendNum++;
             if(suspendNum%60000==0){
                  qDebug("gpio return for suspend\n");
                  printf("gpio return for suspend\n");
             }
             return;
        }

#if SAVE_WAVE
        int ret=read_gpio();
        //save camera
        if(camera==1){
            if(status==0){
                //Find a cover
                if(ret==0){
                    printf("camera%d enter %d\n",camera,countNum);
                    qDebug("enter status1\n");
                    status=1;
                }
            }else if(status==1){
                saveNum++;
                qDebug("saveNum%d ret%d\n",saveNum,ret);
                if(ret==1){
                    status=0;
                    qDebug("%dend\n",countNum);
                }
                countNum++;
            }/*else if(status==2){
                if(ret==1){
                    status=0;
                    printf("camera%d end\n",camera);
                    qDebug("camera%d end\n",camera);
                }else{
                    qDebug("saveNum%d ret%d\n",saveNum,ret);
                }
            }*/
        }
#else
        int ret=1;

        if(*((unsigned short *)(p + pgoffset))==0x0100){
            ret=0;
        }

        const char* tstr=QDateTime::currentDateTime().toString("hh:mm:ss.zzz").toStdString().c_str();
        if(status==0){
            //Find a cover
            if(ret==0){
                //printf("findNum%d Mem status0\n",findNum);
                //qDebug("findNum%d Mem status0\n",findNum);
                status=1;
                waitNum=0;
            }
        }else if(status==1){
            waitNum++;
            if(waitNum>=cfg_ironWidth){
                status=3;
            }else{
                if(ret==1){
                    status=0;
                    if(waitNum>=2){
                       ironWidth+=waitNum;
                       printf("findNum%d Mem back to status0,waitNum%d,ironWidth%d,%s\n",findNum,waitNum,ironWidth,tstr);
                       qDebug("findNum%d Mem back to status0,waitNum%d,ironWidth%d,%s\n",findNum,waitNum,ironWidth,tstr);
                    }
                }
            }
        }else if(status==3){
            findNum++;
            printf("findNum%d Mem status3,%s\n",findNum,tstr);
            qDebug("findNum%d Mem status3,%s\n",findNum,tstr);
            QDateTime nowTime=  QDateTime::currentDateTime();
            beginTime=nowTime.toMSecsSinceEpoch();

            //add to lightList
            light1Mutex.lock();
            lightList1.push_back(nowTime.toMSecsSinceEpoch());
            light1Mutex.unlock();

            light2Mutex.lock();
            lightList2.push_back(nowTime.toMSecsSinceEpoch());
            light2Mutex.unlock();
            waitNum=0;
            status=4;
        }else if(status==4){
            QDateTime nowTime=  QDateTime::currentDateTime();
            int width=nowTime.toMSecsSinceEpoch()-beginTime;

//            if(ret==1){
//                status=0;
//                printf("Find Long%d,width=%d\n",findNum,width);
//                qDebug("Find Long%d,width=%d\n",findNum,width);
//            }
            //Have no cover
            if(ret==1&&width>cfg_coverWidth*0.8){
                    status=0;
                    if(width>cfg_coverWidth*1.2){
                        longNum++;
                        printf("Find Long%d,width=%d\n",findNum,width);
                        qDebug("Find Long%d,width=%d\n",findNum,width);
                    }else{
                        printf("Find Normal%d,width=%d\n",findNum,width);
                        qDebug("Find Normal%d,width=%d\n",findNum,width);
                    }
            }
        }
#endif
    }
    munmap(p, pagesize);
    close(fd);
}








