/****************************************************************************
**
** Definition of QGfxVga16 (graphics context) class for VGA cards.
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

#ifndef QGFXVGA16_QWS_H
#define QGFXVGA16_QWS_H

#ifndef QT_H
#include "qgfxlinuxfb_qws.h"
#endif // QT_H

// VGA16 code does not compile on sparc
#if defined(__sparc__) && !defined(QT_NO_QWS_VGA_16)
#define QT_NO_QWS_VGA16
#endif

#ifndef QT_NO_QWS_VGA16

class QVga16Screen : public QLinuxFbScreen
{

public:

    QVga16Screen(int display_id);
    virtual ~QVga16Screen();
    virtual bool connect(const QString &spec);
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

#endif // QT_NO_QWS_VGA16

#endif // QGFXVGA16_QWS_H

