/****************************************************************************
** $Id$
**
** ...
**
** Copyright (C) 2002 Trolltech AS.  All rights reserved.
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

#ifndef QGFXREPEATER_QWS_H
#define QGFXREPEATER_QWS_H

#ifndef QT_H
#include "qgfx_qws.h"
#endif // QT_H

#ifndef QT_NO_QWS_REPEATER

#include "qptrlist.h"

class QScreenRec;

class QRepeaterScreen : public QScreen
{
public:

    QRepeaterScreen(int);
    virtual ~QRepeaterScreen();

    virtual bool connect(const QString &);
    virtual QGfx * createGfx(unsigned char *,int,int,int,int);
    virtual bool initDevice();
    virtual void disconnect() {}
    virtual void setMode(int,int,int) {}
    virtual int initCursor(void *,bool=FALSE);
    virtual void setDirty(const QRect &);
    virtual int sharedRamSize(void *);
    QImage * readScreen(int,int,int,int,QRegion &);
    QRegion getRequiredUpdate(int,int,int,int,int,int);

private:

    bool sw_cursor_exists;

    QPtrList<QScreenRec> screens;

};

#endif // QT_NO_QWS_REPEATER

#endif // QGFXREPEATER_QWS_H
