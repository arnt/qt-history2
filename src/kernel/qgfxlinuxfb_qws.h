/*****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice.h#73 $
**
** Implementation of QGfxRaster (unaccelerated graphics context) class for
** Embedded Qt
** Created : 940721
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

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

    virtual bool useOffscreen() { return false; }

    virtual void disconnect();
    virtual void shutdownCard();
    virtual void setMode(int,int,int);
    virtual void save();
    virtual void restore();
    virtual void set(unsigned int,unsigned int,unsigned int,unsigned int);
    virtual uchar * cache(int);
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
    int dataoffset;
};

#endif

