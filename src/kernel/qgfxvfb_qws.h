/*****************************************************************************
** $Id: $
**
** Implementation of QGfxvfb (virtual frame buffer driver)
**
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

#ifndef QGFXVFB_QWS_H
#define QGFXVFB_QWS_H

#include "qgfx_qws.h"

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
	{ hdr->dirty = true; hdr->update = hdr->update.unite( r ); }

    bool success;
    unsigned char *shmrgn;
    QVFbHeader *hdr;
    QVFbMouseHandler *mouseHandler;
    QVFbKeyboardHandler *keyboardHandler;
};

#endif

#endif // QGFXVFB_QWS_H
