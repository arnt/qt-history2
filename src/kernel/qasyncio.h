/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qasyncio.h#2 $
**
**		      ***   INTERNAL HEADER FILE   ***
**
**		This file is NOT a part of the Qt interface!
**
** Definition of asynchronous I/O classes
**
** Created : 970617
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
** --------------------------------------------------------------------------
** Internal header file containing private classes used by QMovie.
*****************************************************************************/

#ifndef QASYNCIO_H
#define QASYNCIO_H

#include "qobject.h"
#include "qsignal.h"
#include "qtimer.h"

class QIODevice;

class QAsyncIO {
public:
    void connect(QObject*, const char* member);

protected:
    void ready();

private:
    QSignal signal;
};

class QDataSink : public QAsyncIO {
public:
    // Call this to know how much I can take.
    virtual int readyToReceive()=0;
    virtual void receive(const uchar*, int count)=0;
    virtual void eof()=0;
    void maybeReady();
};

class QDataSource : public QAsyncIO {
public:
    virtual int readyToSend()=0; // returns -1 when never any more ready
    virtual void sendTo(QDataSink*, int count)=0;
    void maybeReady();

    virtual bool rewindable() const;
    virtual void enableRewind(bool);
    virtual void rewind();
};

class QIODeviceSource : public QDataSource {
    const int buf_size;
    uchar *buffer;
    QIODevice* iod;
    bool rew;

public:
    QIODeviceSource(QIODevice*, int bufsize=4096);
    virtual ~QIODeviceSource();

    int readyToSend();
    void sendTo(QDataSink* sink, int n);
    bool rewindable() const;
    void enableRewind(bool on);
    void rewind();
};

class QDataPump : public QObject {
    Q_OBJECT
    int interval;
    QTimer timer;
    QDataSource* source;
    QDataSink* sink;

public:
    QDataPump(QDataSource*, QDataSink*);

private slots:
    void kickStart();
    void tryToPump();
};

#endif
