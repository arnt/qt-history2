/****************************************************************************
** $Id: $
**
** Definition of some shared interal classes
**
** Created : 010427
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QINTERNAL_P_H
#define QINTERNAL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of a number of Qt sources files.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//
//
#ifndef QT_H
#include <qnamespace.h>
#include <qrect.h>
#endif // QT_H

class QWidget;
class QPainter;
class QPixmap;

class Q_EXPORT QSharedDoubleBuffer
{
public:
    QSharedDoubleBuffer( bool mustShare = TRUE, bool initializeBg = TRUE, QPixmap* pm = 0);
    QSharedDoubleBuffer( QWidget* widget, int x = 0, int y = 0, int w = -1, int h = -1 );
    QSharedDoubleBuffer( QPainter* painter, int x = 0, int y = 0, int w = -1, int h = -1 );
    QSharedDoubleBuffer( QWidget *widget, const QRect &r );
    QSharedDoubleBuffer( QPainter *painter, const QRect &r );
    ~QSharedDoubleBuffer();

    bool begin( QWidget* widget, int x = 0, int y = 0, int w = -1, int h = -1 );
    bool begin( QPainter* painter, int x = 0, int y = 0, int w = -1, int h = -1);
    bool begin( QWidget* widget, const QRect &r );
    bool begin( QPainter* painter, const QRect &r );
    bool end();

    QPainter* painter() const;

    bool isActive() const;
    bool isBuffered() const;
    void flush();

    static bool isDisabled() { return !dblbufr; }
    static QPixmap* getRawPixmap( int w, int h );
    static void cleanup();

private:
    QWidget* wid;
    int rx, ry, rw, rh;
    QPainter *p, *xp;
    QPixmap *pix, *xpix;
    uint mustsh : 1;
    uint initbg : 1;
    static bool dblbufr;
};

inline bool QSharedDoubleBuffer::begin( QWidget* widget, const QRect &r )
{ return begin( widget, r.x(), r.y(), r.width(), r.height() ); }

inline bool QSharedDoubleBuffer::begin( QPainter *painter, const QRect &r )
{ return begin( painter, r.x(), r.y(), r.width(), r.height() ); }

inline QPainter* QSharedDoubleBuffer::painter() const
{ return p; }

inline bool QSharedDoubleBuffer::isActive() const
{ return p != 0; }

inline bool QSharedDoubleBuffer::isBuffered() const
{ return pix != 0; }

#endif // QINTERNAL_P_H
