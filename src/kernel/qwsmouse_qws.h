/****************************************************************************
** $Id: $
**
** Definition of Qt/FB central server classes
**
** Created : 991025
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QWSMOUSE_H
#define QWSMOUSE_H

#ifndef QT_H
#include <qobject.h>
#include <qpointarray.h>
#include <qptrlist.h>
#include "qsocketnotifier.h"
#endif // QT_H

class QWSPointerCalibrationData
{
public:
    enum Location { TopLeft = 0, BottomLeft = 1, BottomRight = 2, TopRight = 3,
		    Center = 4, LastLocation = Center };
    QPoint devPoints[5];
    QPoint screenPoints[5];
};

class QWSMouseHandler : public QObject {
    Q_OBJECT
public:
    QWSMouseHandler();
    virtual ~QWSMouseHandler();

    virtual void clearCalibration() {}
    virtual void calibrate( QWSPointerCalibrationData * ) {}
    virtual void getCalibration( QWSPointerCalibrationData * ) {}

protected:
    enum {mouseBufSize = 128};

    void mouseChanged(const QPoint& pos, int bstate);
};


#if !defined(QT_QWS_IPAQ) && !defined(QT_QWS_CASSIOPEIA)
# define QT_NO_QWS_MOUSE_CALIBRATED
#endif

#ifndef QT_NO_QWS_MOUSE_CALIBRATED
class QCalibratedMouseHandler : public QWSMouseHandler
{
    Q_OBJECT
public:
    QCalibratedMouseHandler();

    virtual void clearCalibration();
    virtual void calibrate( QWSPointerCalibrationData * );
    virtual void getCalibration( QWSPointerCalibrationData * );

protected:
    void readCalibration();
    void writeCalibration();
    QPoint transform( const QPoint & );
    bool sendFiltered( const QPoint &, int button );
    void setFilterSize( int );

private:
    int a, b, c;
    int d, e, f;
    int s;
    QPointArray samples;
    unsigned int currSample;
    unsigned int numSamples;
};
#endif

/********************* PRIVATE CLASSES FOLLOW *******************************/

enum MouseProtocol { Unknown = -1, Auto = 0,
		     MouseMan, IntelliMouse, Microsoft,
		     QVFBMouse, TPanel, BusMouse,
		     FirstAuto = MouseMan,
		     LastAuto = Microsoft };

#ifndef QT_NO_QWS_MOUSE_AUTO
class QAutoMouseSubHandler;
class QAutoMouseHandlerPrivate : public QWSMouseHandler {
    Q_OBJECT
public:
    QAutoMouseHandlerPrivate();
    ~QAutoMouseHandlerPrivate();

private:
    enum { max_dev=32 };
    QAutoMouseSubHandler *sub[max_dev];
    QPtrList<QSocketNotifier> notifiers;
    int nsub;
    int retries;

private slots:
    void readMouseData(int);

private:
    void openDevices();
    void closeDevices();
    void notify(int fd);
    bool sendEvent(QAutoMouseSubHandler& h);
    
};
#endif



#ifndef QT_NO_QWS_MOUSE_MANUAL
class QWSMouseHandlerPrivate : public QWSMouseHandler {
    Q_OBJECT
public:
    QWSMouseHandlerPrivate( MouseProtocol protocol, QString mouseDev );
    ~QWSMouseHandlerPrivate();

private:
    int mouseFD;
    int mouseIdx;
    uchar mouseBuf[mouseBufSize];
    MouseProtocol mouseProtocol;
    void handleMouseData();

private slots:
    void readMouseData();

private:
    int obstate;
    QTimer *rtimer;
};
#endif



#ifdef QT_QWS_CASSIOPEIA
/*
 * Handler for /dev/tpanel Linux kernel driver
 */

class QVrTPanelHandlerPrivate : public QCalibratedMouseHandler {
    Q_OBJECT
public:
    QVrTPanelHandlerPrivate(MouseProtocol, QString dev);
    ~QVrTPanelHandlerPrivate();

private:
    int mouseFD;
    MouseProtocol mouseProtocol;
private slots:
    void sendRelease();
    void readMouseData();
private:
    QTimer *rtimer;
    int mouseIdx;
    uchar mouseBuf[mouseBufSize];
};
#endif


#if defined(QT_QWS_IPAQ) || defined(QT_QWS_EBX)
class QTPanelHandlerPrivate : public QCalibratedMouseHandler
{
     Q_OBJECT
public:
    QTPanelHandlerPrivate(MouseProtocol, QString dev);
    ~QTPanelHandlerPrivate();

private:
    static const int mouseBufSize = 2048;
    int mouseFD;
    QPoint oldmouse;
    bool waspressed;
    QPointArray samples;
    unsigned int currSample;
    unsigned int numSamples;
    int mouseIdx;
    uchar mouseBuf[mouseBufSize];
    
private slots:
    void readMouseData();   
};
#endif

#ifdef QT_QWS_YOPY
// YOPY touch panel support based on changes contributed by Ron Victorelli
// (victorrj at icubed.com) to Custom TP driver.
//
class QYopyTPanelHandlerPrivate : public QWSMouseHandler {
    Q_OBJECT
public:
    QYopyTPanelHandlerPrivate(MouseProtocol, QString dev);
    ~QYopyTPanelHandlerPrivate();

private:
    int mouseFD;
    int prevstate;
private slots:
    void readMouseData();

};
#endif

#ifdef QT_QWS_CUSTOMTOUCHPANEL
class QCustomTPanelHandlerPrivate : public QWSMouseHandler {
    Q_OBJECT
public:
    QCustomTPanelHandlerPrivate(MouseProtocol, QString dev);
    ~QCustomTPanelHandlerPrivate();

private:
    int mouseFD;
private slots:
    void readMouseData();
};
#endif

#ifndef QT_NO_QWS_VFB
class QVFbMouseHandlerPrivate : public QWSMouseHandler {
    Q_OBJECT
public:
    QVFbMouseHandlerPrivate(MouseProtocol, QString dev);
    ~QVFbMouseHandlerPrivate();

    bool isOpen() const { return mouseFD > 0; }

private:
    int mouseFD;
    int mouseIdx;
    uchar mouseBuf[mouseBufSize];
private slots:
    void readMouseData();
};
#endif

#endif

