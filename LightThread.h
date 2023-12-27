#ifndef LIGHTTHREAD_H
#define LIGHTTHREAD_H

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

class LightThread: public QThread
{
    Q_OBJECT

public:
    explicit LightThread(QObject *parent = 0);
    void run();
    void write_mem(int value);
    int camera;
 };

#endif
