/****************************************************************************
**
** Implementation of QGfxMach64 (graphics context) class for Mach64 cards.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QGFXMACH64_QWS_H
#define QGFXMACH64_QWS_H

#ifndef QT_H
#include "qgfxlinuxfb_qws.h"
#endif // QT_H

#ifndef QT_NO_QWS_MACH64

class QMachScreen : public QLinuxFbScreen
{
public:
    QMachScreen( int display_id );
    virtual ~QMachScreen();

    virtual bool connect( const QString &spec );
    virtual bool initDevice();
    virtual int initCursor(void*, bool);
    virtual void shutdownDevice();
    virtual bool useOffscreen();
    virtual QGfx * createGfx(unsigned char *,int,int,int,int);

protected:
    virtual int pixmapOffsetAlignment();
    virtual int pixmapLinestepAlignment();
};

#endif // QT_NO_QWS_MACH64

#endif // QGFXMACH64_QWS_H

