/*****************************************************************************
** $Id: $
**
** Implementation of QGfxvnc (remote frame buffer driver)
** Proof of concept driver only.
** 
** Created : 20000703
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

#ifndef QGFXVNC_QWS_H
#define QGFXVNC_QWS_H

#if defined(Q_OS_QNX6)
#define VNCSCREEN_BASE QQnxScreen
#include "qwsgfx_qnx.h"
#else
#define VNCSCREEN_BASE QLinuxFbScreen
#include "qgfxlinuxfb_qws.h"
#endif

#ifndef QT_NO_QWS_VNC

#include "qsharedmemory_p.h"

class QVNCServer;
class QVNCHeader;

class QVNCScreen : public VNCSCREEN_BASE {
public:
    QVNCScreen( int display_id );
    virtual ~QVNCScreen();
    virtual bool initDevice();
    virtual bool connect( const QString &displaySpec );
    virtual void disconnect();
    virtual int initCursor(void*, bool);
    virtual void shutdownDevice();
    virtual QGfx * createGfx(unsigned char *,int,int,int,int);
    virtual void save();
    virtual void restore();
    virtual void setMode(int nw,int nh,int nd);

    virtual void setDirty( const QRect& r );

    bool success;
    QVNCServer *vncServer;
    unsigned char *shmrgn;
    QSharedMemory shm;
    QVNCHeader *hdr;
    bool virtualBuffer;
};

#endif // QT_NO_QWS_VNC

#endif // QGFXVNC_QWS_H

