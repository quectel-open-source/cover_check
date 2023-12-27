#ifndef MEMTHREAD_H
#define MEMTHREAD_H

#include <QDate>
#include <QString>
#include <QThread>
#include <vector>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/io.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

class MemThread: public QThread
{
    Q_OBJECT

public:
    explicit MemThread(QObject *parent = 0);
    void run();
    int read_gpio();
    int status=0;
    int waitNum=0;
    qint64 beginTime;
    unsigned char *memp;
 };

#endif
