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

#ifndef QGFXVNC_QWS_H
#define QGFXVNC_QWS_H

#define VNCSCREEN_BASE QLinuxFbScreen
#include "QtGui/qscreenlinuxfb_qws.h"

#ifndef QT_NO_QWS_VNC

class QVNCServer;
class QVNCHeader;
class QSharedMemory;

class QVNCScreen : public VNCSCREEN_BASE {
public:
    explicit QVNCScreen(int display_id);
    virtual ~QVNCScreen();
    virtual bool initDevice();
    virtual bool connect(const QString &displaySpec);
    virtual void disconnect();
    virtual int initCursor(void*, bool);
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

#endif // QGFXVNC_QWS_H
