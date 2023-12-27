#include "LightThread.h"
#include "videopanel.h"
#include <QLabel>
#include <iostream>
#include <stdio.h>
using namespace std;

extern vector<qint64> lightList1;
extern vector<qint64> lightList2;
//extern QMutex gpioMutex;
extern int pushNumA;
extern int pushNumC;
extern QMutex light1Mutex;
extern QMutex light2Mutex;
int light1Num=0;
int light2Num=0;

LightThread::LightThread(QObject *parent) : QThread(parent)
{
    lightList1.reserve(3000);
    lightList2.reserve(3000);
}

#define DEV_PATH "/dev/mem"
//void LightThread::write_mem(int value)
//{
//    unsigned int    memaddr, wdata;
//    unsigned int    pgoffset, pgaddr;
//    unsigned int    pagesize = sysconf(_SC_PAGESIZE);
//    unsigned char   *p;
//    int             fd;
//    QDateTime dateTime ;
//    QString timestamp ;
//    dateTime = QDateTime::currentDateTime();
//    timestamp = dateTime.toString("hh:mm:ss.zzz");

//    printf("write mem step0,time%s\n",(char*)timestamp.toStdString().data());
//    qDebug("write mem step0,time%s\n",(char*)timestamp.toStdString().data());

//    gpioMutex.lock();
//    fd = open(DEV_PATH, O_RDWR);
//    if (fd <= 0) {
//            printf("write_io  fd<=0 open error: %s\n", DEV_PATH);
//            qDebug("write_io  fd<=0 open error: %s\n", DEV_PATH);
//            gpioMutex.unlock();
//            return;
//    }

//    if(camera==1){
//        //camera1 pin9
//        memaddr  = 0xfd6e08a0;
//    }else{
//        //camera2 pin13
//        memaddr  = 0xfd6d06a0;
//    }

//    pgoffset = memaddr & (pagesize -1);
//    pgaddr   = memaddr & ~(pagesize -1);
//    p = (unsigned char *)mmap(NULL, pagesize, PROT_WRITE, MAP_SHARED, fd, pgaddr);
//    if (p == 0) {
//       printf("write_io mmap error\n");
//       qDebug("write_io mmap error\n");
//       gpioMutex.unlock();
//       return;
//    }

//    if(value==0){
//        wdata=0x200; //strtoul("0x0200", NULL, 16);
//    }else{
//        wdata=0x201; //strtoul("0x0201", NULL, 16);
//    }

//    printf("write mem step1\n");
//    qDebug("write mem step1\n");

//    *((unsigned short *)(p + pgoffset)) = (unsigned short)wdata;
//    munmap(p, pagesize);
//    close(fd);

//    gpioMutex.unlock();
//    printf("write mem step2 -- camera%d value%d OK!\n",camera,value);
//    qDebug("write mem step2 -- camera%d value%d OK!\n",camera,value);
//}

extern int cfg_abDelay;
extern int cfg_cdDelay;


void LightThread::run()
{
    unsigned int    memaddr, wdata;
    unsigned int    pgoffset, pgaddr;
    unsigned int    pagesize = sysconf(_SC_PAGESIZE);
    unsigned char   *p;
    int             fd;
    QDateTime dateTime ;
    QString timestamp ;
    dateTime = QDateTime::currentDateTime();
    timestamp = dateTime.toString("hh:mm:ss.zzz");

    printf("write mem step0,time%s\n",(char*)timestamp.toStdString().data());
    qDebug("write mem step0,time%s\n",(char*)timestamp.toStdString().data());

    fd = open(DEV_PATH, O_RDWR);
    if (fd <= 0) {
            printf("write_io  fd<=0 open error: %s\n", DEV_PATH);
            qDebug("write_io  fd<=0 open error: %s\n", DEV_PATH);
            return;
    }

    if(camera==1){
        //camera1 pin15
        memaddr  = 0xfd6d06e0;
    }else{
        //camera2 pin11
        memaddr  = 0xfd6d0680;
    }

    pgoffset = memaddr & (pagesize -1);
    pgaddr   = memaddr & ~(pagesize -1);
    p = (unsigned char *)mmap(NULL, pagesize, PROT_WRITE, MAP_SHARED, fd, pgaddr);
    if (p == 0) {
       printf("write_io mmap error\n");
       qDebug("write_io mmap error\n");
       return;
    }

    while(1){ 
        if(camera==1){
            light1Mutex.lock();
            if(lightList1.size()>0){
                qint64 firstTime=lightList1[0];
                QDateTime nowTime=  QDateTime::currentDateTime();
                int distant=static_cast<int>(nowTime.toMSecsSinceEpoch()-firstTime);
                //printf("distant%d\n",distant);
                if(distant>cfg_abDelay){
                    light1Num++;
                    QString timestamp;
                    timestamp = nowTime.toString("hh:mm:ss.zzz");
                    printf("lightList1 num%d cap%d,time%s\n",light1Num,lightList1.size(),(char*)timestamp.toStdString().data());
                    qDebug("lightList1 num%d cap%d,time%s\n",light1Num,lightList1.size(),(char*)timestamp.toStdString().data());
                    lightList1.erase(lightList1.begin());

                    *((unsigned short *)(p + pgoffset)) = (unsigned short)0x201;
                    msleep(30);
                    *((unsigned short *)(p + pgoffset)) = (unsigned short)0x200;
                }
            }
            light1Mutex.unlock();
        }else{
            light2Mutex.lock();
            if(lightList2.size()>0){

                qint64 firstTime=lightList2[0];
                QDateTime nowTime=  QDateTime::currentDateTime();
                int distant=static_cast<int>(nowTime.toMSecsSinceEpoch()-firstTime);
                //printf("distant%d\n",distant);
                if(distant>cfg_cdDelay){
                    light2Num++;
                    QString timestamp;
                    timestamp =  nowTime.toString("hh:mm:ss.zzz");
                    printf("lightList2 num%d cap%d,time%s\n",light2Num,lightList2.size(),(char*)timestamp.toStdString().data());
                    qDebug("lightList2 num%d cap%d,time%s\n",light2Num,lightList2.size(),(char*)timestamp.toStdString().data());
                    lightList2.erase(lightList2.begin());

                    *((unsigned short *)(p + pgoffset)) = (unsigned short)0x201;
                    msleep(30);
                    *((unsigned short *)(p + pgoffset)) = (unsigned short)0x200;
                }
            }
            light2Mutex.unlock();
        }
        msleep(1);
    }

    munmap(p, pagesize);
    close(fd);

}








