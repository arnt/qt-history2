/****************************************************************************
**
** Definition of QGfxMatrox (graphics context) class for Matrox MGA cards.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QGFXMATROX_QWS_H
#define QGFXMATROX_QWS_H

#ifndef QT_H
#include "qgfxlinuxfb_qws.h"
#endif // QT_H

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
