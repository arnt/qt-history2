/*****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice.h#73 $
**
** Implementation of QGfxRaster (unaccelerated graphics context) class for
** Embedded Qt
** Created : 940721
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#ifndef QGFXLINUXFB_H
#define QGFXLINUXFB_H

#include "qgfx_qws.h"

class QLinuxFbScreen : public QScreen
{
public:
    QLinuxFbScreen( int display_id );
    virtual ~QLinuxFbScreen();

    virtual bool initCard();
    virtual bool connect( const QString &displaySpec );
    virtual void disconnect();
    virtual void shutdownCard();
    virtual void setMode(int,int,int);
    virtual void save();
    virtual void restore();
    virtual void set(unsigned int,unsigned int,unsigned int,unsigned int);
    virtual uchar * cache(int,int);
    virtual void uncache(uchar *);

protected:

    bool canaccel;

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

