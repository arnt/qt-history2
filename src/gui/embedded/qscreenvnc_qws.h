/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSCREENVNC_QWS_H
#define QSCREENVNC_QWS_H

#include <QtGui/qscreenlinuxfb_qws.h>

QT_MODULE(Gui)

#define VNCSCREEN_BASE QLinuxFbScreen

#ifndef QT_NO_QWS_VNC

class QVNCServer;
class QVNCHeader;
class QSharedMemory;

#ifdef qdoc
class QVNCScreen : public QScreen {
#else
class QVNCScreen : public VNCSCREEN_BASE {
#endif
public:
    explicit QVNCScreen(int display_id);
    virtual ~QVNCScreen();
    virtual bool initDevice();
    virtual bool connect(const QString &displaySpec);
    virtual void disconnect();
    virtual void shutdownDevice();
//    virtual QGfx * createGfx(unsigned char *,int,int,int,int);
    virtual void save();
    virtual void restore();
    virtual void setMode(int nw,int nh,int nd);

    virtual void setDirty(const QRect& r);

    bool success;
    QVNCServer *vncServer;
    unsigned char *shmrgn;
    QSharedMemory *shm;
    QVNCHeader *hdr;
    bool virtualBuffer;
};

#endif // QT_NO_QWS_VNC

#endif // QSCREENVNC_QWS_H
