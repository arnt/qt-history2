/****************************************************************************
** $Id: //depot/qt/main/tests/qtfb/qtfb.h#1 $
**
** Definition of Qt/FB central server classes
**
** Created : 991025
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#ifndef QTFB_H
#define QTFB_H

#include <qserversocket.h>
#include <qmap.h>

const int SWIDTH=640;
const int SHEIGHT=480;
const int QTFB_PORT=0x4642; // FB

class QtFBClient;


class QtFBServer : public QServerSocket {
    Q_OBJECT

public:
    QtFBServer( QObject *parent=0, const char *name=0 );
    ~QtFBServer();
    void newConnection( int socket );

    uchar* frameBuffer() { return framebuffer; }

    void sendMouseEvent(const QPoint& pos, int state);

private slots:
    void doClient();
private:
    typedef QMapIterator<int,QtFBClient*> ClientIterator;
    typedef QMap<int,QtFBClient*> ClientMap;

    int shmid;
    uchar* framebuffer;
    ClientMap client;
};

#endif
