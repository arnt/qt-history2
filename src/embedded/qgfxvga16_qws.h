/****************************************************************************
** $Id: $
**
** Definition of QGfxVga16 (graphics context) class for VGA cards
**
** Created :
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
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

#ifndef QGFXVGA16_QWS_H
#define QGFXVGA16_QWS_H

#include "qgfxlinuxfb_qws.h"

// VGA16 code does not compile on sparc
#if defined(__sparc__) && !defined(QT_NO_QWS_VGA_16)
#define QT_NO_QWS_VGA_16
#endif

#ifndef QT_NO_QWS_VGA_16

class QVga16Screen : public QLinuxFbScreen
{

public:

    QVga16Screen( int display_id );
    virtual ~QVga16Screen();
    virtual bool connect( const QString &spec );
    virtual bool initDevice();
    virtual int initCursor(void*, bool);
    virtual void shutdownDevice();
    virtual bool useOffscreen();
    virtual QGfx * createGfx(unsigned char *,int,int,int,int);
    virtual int alloc(unsigned int, unsigned int, unsigned int);
    int pixmapDepth() const;

protected:

    virtual int pixmapOffsetAlignment();
    virtual int pixmapLinestepAlignment();

private:

    int shmId;
};

#endif // QT_NO_QWS_VGA_16

#endif // QGFXVGA16_QWS_H

