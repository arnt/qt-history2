/****************************************************************************
** $Id: $
**
** Definition of QGfxVoodoo (graphics context) class for Voodoo 3 cards
**
** Created : 20020214
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

#ifndef QGFXVOODOO_QWS_H
#define QGFXVOODOO_QWS_H

#include "qgfxlinuxfb_qws.h"

#ifndef QT_NO_QWS_VOODOO3

class QVoodooScreen : public QLinuxFbScreen
{
public:
    QVoodooScreen( int display_id );
    virtual ~QVoodooScreen();

    virtual bool connect( const QString &spec );
    virtual bool initDevice();
    virtual void shutdownDevice();
    virtual int initCursor(void *,bool);
    virtual bool useOffscreen();

    virtual QGfx * createGfx(unsigned char *,int,int,int,int);

    unsigned char * voodoo_regbase;
};

#endif // QT_NO_QWS_VOODOO3

#endif // QGFXVOODOO_QWS_H
