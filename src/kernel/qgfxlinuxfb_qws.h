/*****************************************************************************
** $Id: $
**
** Implementation of QGfxRaster (unaccelerated graphics context) class for
** Embedded Qt
** Created : 940721
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

#ifndef QGFXLINUXFB_H
#define QGFXLINUXFB_H

#ifndef QT_NO_QWS_LINUXFB
#include "qgfx_qws.h"

class QLinuxFb_Shared
{
public:
    volatile int lastop;
    volatile int optype;
    volatile int fifocount;   // Accel drivers only
    volatile int fifomax;
    volatile unsigned int forecol;     // Foreground colour cacheing
    volatile unsigned int buffer_offset;   // Destination
    volatile int linestep;
    volatile unsigned int cliptop;    // Clip rectangle
    volatile unsigned int clipleft;
    volatile unsigned int clipright;
    volatile unsigned int clipbottom;
    volatile unsigned int rop;

};

class QLinuxFbScreen : public QScreen
{
public:
    QLinuxFbScreen( int display_id );
    virtual ~QLinuxFbScreen();

    virtual bool initDevice();
    virtual bool connect( const QString &displaySpec );

    virtual bool useOffscreen() { return false; }

    virtual void disconnect();
    virtual void shutdownDevice();
    virtual void setMode(int,int,int);
    virtual void save();
    virtual void restore();
    virtual void blank(bool on);
    virtual void set(unsigned int,unsigned int,unsigned int,unsigned int);
    virtual uchar * cache(int,int);
    virtual void uncache(uchar *);
    virtual int sharedRamSize(void *);

    QLinuxFb_Shared * shared;

protected:

    bool canaccel;
    int dataoffset;

private:

    void delete_entry(int);
    void insert_entry(int,int,int);

    int fd;
    int startupw;
    int startuph;
    int startupd;
    fb_cmap *startcmap;
};

#endif
#endif

