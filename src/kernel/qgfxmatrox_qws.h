/****************************************************************************
** $Id: $
**
** Definition of QGfxMatrox (graphics context) class for Matrox MGA cards
**
** Created : 20020214
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

#ifndef QGFXMATROX_QWS_H
#define QGFXMATROX_QWS_H

#include "qgfxlinuxfb_qws.h"

#ifndef QT_NO_QWS_MATROX

class QMatroxScreen : public QLinuxFbScreen
{
public:
    QMatroxScreen( int display_id );
    virtual ~QMatroxScreen();

    virtual bool connect( const QString &spec );
    virtual bool initDevice();
    virtual void shutdownDevice();
    virtual bool useOffscreen();
    virtual int initCursor(void*, bool);
    virtual QGfx * createGfx(unsigned char *,int,int,int,int);

protected:
    virtual int pixmapOffsetAlignment();
    virtual int pixmapLinestepAlignment();

private:
    unsigned int src_pixel_offset;
};


#endif // QT_NO_QWS_MATROX

#endif // QGFXMATROX_QWS_H
