/*****************************************************************************
** $Id$
**
** Implementation of QGfxRaster (unaccelerated graphics context) class for
** Embedded Qt
** Created : 940721
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

#ifndef QGFXTRANSFORMED_QWS_H
#define QGFXTRANSFORMED_QWS_H

#include "qgfx_qws.h"

#ifndef QT_NO_QWS_TRANSFORMED

#ifndef Q_OS_QNX6
#define QT_TRANS_SCREEN_BASE    QLinuxFbScreen
//#define QT_TRANS_SCREEN_BASE  QVFbScreen
#include "qgfxlinuxfb_qws.h"
#else
#define QT_TRANS_SCREEN_BASE    QQnxScreen
#include "qwsgfx_qnx.h"
#endif

#define QT_TRANS_CURSOR_BASE    QScreenCursor
#define QT_TRANS_GFX_BASE       QGfxRaster
//#define QT_TRANS_CURSOR_BASE  QVFbScreenCursor
//#define QT_TRANS_GFX_BASE     QGfxVFb

class QTransformedScreen : public QT_TRANS_SCREEN_BASE
{
public:
    QTransformedScreen( int display_id );
    virtual ~QTransformedScreen();

    virtual bool connect( const QString &displaySpec );
    virtual int initCursor(void* e, bool init);
    virtual QGfx * createGfx(unsigned char *,int,int,int,int);

    enum Transformation { None, Rot90, Rot180, Rot270 };
    Transformation transformation() const { return trans; }

    virtual bool isTransformed() const;
    virtual QSize mapToDevice( const QSize & ) const;
    virtual QSize mapFromDevice( const QSize & ) const;
    virtual QPoint mapToDevice( const QPoint &, const QSize & ) const;
    virtual QPoint mapFromDevice( const QPoint &, const QSize & ) const;
    virtual QRect mapToDevice( const QRect &, const QSize & ) const;
    virtual QRect mapFromDevice( const QRect &, const QSize & ) const;
    virtual QImage mapToDevice( const QImage & ) const;
    virtual QImage mapFromDevice( const QImage & ) const;
    virtual QRegion mapToDevice( const QRegion &, const QSize & ) const;
    virtual QRegion mapFromDevice( const QRegion &, const QSize & ) const;
    virtual int transformOrientation() const;

private:
    Transformation trans;
    QScreen *driver;
};

#endif // QT_NO_QWS_TRANSFORMED

#endif // QGFXTRANSFORMED_QWS_H
