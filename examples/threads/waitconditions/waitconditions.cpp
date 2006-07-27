/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtCore>

#include <stdio.h>
#include <stdlib.h>

const int DataSize = 100000;
const int BufferSize = 8192;
char buffer[BufferSize];

QWaitCondition bufferNotEmpty;
QWaitCondition bufferNotFull;
QMutex mutex;
int numUsedBytes = 0;

class Producer : public QThread
{
public:
    void run();
};

void Producer::run()
{
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    
    for (int i = 0; i < DataSize; ++i) {
        mutex.lock();
        if (numUsedBytes == BufferSize)
            bufferNotFull.wait(&mutex);
        mutex.unlock();

        buffer[i % BufferSize] = "ACGT"[(int)qrand() % 4];

        mutex.lock();
        ++numUsedBytes;
        bufferNotEmpty.wakeAll();
        mutex.unlock();
    }
}

class Consumer : public QThread
{
public:
    void run();
};

void Consumer::run()
{
    for (int i = 0; i < DataSize; ++i) {
        mutex.lock();
        if (numUsedBytes == 0)
            bufferNotEmpty.wait(&mutex);
        mutex.unlock();

        fprintf(stderr, "%c", buffer[i % BufferSize]);

        mutex.lock();
        --numUsedBytes;
        bufferNotFull.wakeAll();
        mutex.unlock();
    }
    fprintf(stderr, "\n");
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Producer producer;
    Consumer consumer;
    producer.start();
    consumer.start();
    producer.wait();
    consumer.wait();
    return 0;
}
