/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qasyncio.h#9 $
**
**		      ***   INTERNAL HEADER FILE   ***
**
**		This file is NOT a part of the Qt interface!
**
** Definition of asynchronous I/O classes
**
** Created : 970617
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QASYNCIO_H
#define QASYNCIO_H

#ifndef QT_H
#include "qobject.h"
#include "qsignal.h"
#include "qtimer.h"
#endif // QT_H

class QIODevice;

class Q_EXPORT QAsyncIO {
public:
    virtual ~QAsyncIO();
    void connect(QObject*, const char *member);

protected:
    void ready();

private:
    QSignal signal;
};

class Q_EXPORT QDataSink : public QAsyncIO {
public:
    // Call this to know how much I can take.
    virtual int readyToReceive()=0;
    virtual void receive(const uchar*, int count)=0;
    virtual void eof()=0;
    void maybeReady();
};

class Q_EXPORT QDataSource : public QAsyncIO {
public:
    virtual int readyToSend()=0; // returns -1 when never any more ready
    virtual void sendTo(QDataSink*, int count)=0;
    void maybeReady();

    virtual bool rewindable() const;
    virtual void enableRewind(bool);
    virtual void rewind();
};

class Q_EXPORT QIODeviceSource : public QDataSource {
    const int buf_size;
    uchar *buffer;
    QIODevice* iod;
    bool rew;

public:
    QIODeviceSource(QIODevice*, int bufsize=4096);
   ~QIODeviceSource();

    int readyToSend();
    void sendTo(QDataSink* sink, int n);
    bool rewindable() const;
    void enableRewind(bool on);
    void rewind();
};

class Q_EXPORT QDataPump : public QObject {
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
