/****************************************************************************
**
** Implementation of QGfxvfb (virtual frame buffer driver).
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

#ifndef QGFXVFB_QWS_H
#define QGFXVFB_QWS_H

#ifndef QT_H
#include "qgfx_qws.h"
#endif // QT_H

#ifndef QT_NO_QWS_VFB

#include "qvfbhdr.h"


class QVFbMouseHandler;
class QVFbKeyboardHandler;

class QVFbScreen : public QScreen
{
public:
    QVFbScreen( int display_id );
    virtual ~QVFbScreen();
    virtual bool initDevice();
    virtual bool connect( const QString &displaySpec );
    virtual void disconnect();
    virtual int initCursor(void*, bool);
    virtual void shutdownDevice();
    virtual QGfx * createGfx(unsigned char *,int,int,int,int);
    virtual void save();
    virtual void restore();
    virtual void setMode(int nw,int nh,int nd);

    virtual void setDirty( const QRect& r )
	{ hdr->dirty = TRUE; hdr->update = hdr->update.unite( r ); }

    bool success;
    unsigned char *shmrgn;
    QVFbHeader *hdr;
    QVFbMouseHandler *mouseHandler;
    QVFbKeyboardHandler *keyboardHandler;
};

#endif

#endif // QGFXVFB_QWS_H
