/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qregion.h#54 $
**
** Definition of QRegion class
**
** Created : 940514
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QREGION_H
#define QREGION_H

#ifndef QT_H
#include "qshared.h"
#include "qrect.h"
#include "qstring.h"
#endif // QT_H


class Q_EXPORT QRegion
{
public:
    enum RegionType { Rectangle, Ellipse };

    QRegion();
    QRegion( int x, int y, int w, int h, RegionType = Rectangle );
    QRegion( const QRect &, RegionType = Rectangle );
    QRegion( const QPointArray &, bool winding=FALSE );
    QRegion( const QRegion & );
    QRegion( const QBitmap & );
   ~QRegion();
    QRegion &operator=( const QRegion & );

    bool    isNull()   const;
    bool    isEmpty()  const;

    bool    contains( const QPoint &p ) const;
    bool    contains( const QRect &r )	const;

    void    translate( int dx, int dy );

    QRegion unite( const QRegion & )	const;
    QRegion intersect( const QRegion &) const;
    QRegion subtract( const QRegion & ) const;
    QRegion eor( const QRegion & )	const;

    QRect   boundingRect() const;
    QArray<QRect> rects() const;
    void setRects( const QRect *, int );

    QRegion operator|( const QRegion & ) const;
    QRegion operator+( const QRegion & ) const;
    QRegion operator&( const QRegion & ) const;
    QRegion operator-( const QRegion & ) const;
    QRegion operator^( const QRegion & ) const;
    QRegion& operator|=( const QRegion & );
    QRegion& operator+=( const QRegion & );
    QRegion& operator&=( const QRegion & );
    QRegion& operator-=( const QRegion & );
    QRegion& operator^=( const QRegion & );

    bool    operator==( const QRegion & )  const;
    bool    operator!=( const QRegion &r ) const
			{ return !(operator==(r)); }

#if defined(_WS_WIN_)
    HRGN    handle() const { return data->rgn; }
#elif defined(_WS_X11_)
    Region  handle() const { return data->rgn; }
#elif defined(_WS_MAC_)
    void *  handle() const { return data->rgn; }
#elif defined(_WS_QWS_)
    // QGfx_QWS needs this for region drawing
    void * handle() const { return data->rgn; }
#endif

#ifndef QT_NO_DATASTREAM
    friend Q_EXPORT QDataStream &operator<<( QDataStream &, const QRegion & );
    friend Q_EXPORT QDataStream &operator>>( QDataStream &, QRegion & );
#endif
private:
    QRegion( bool );
    QRegion copy() const;
    void    detach();
#if defined(_WS_WIN_)
    QRegion winCombine( const QRegion &, int ) const;
#endif
    void    exec( const QByteArray &, int ver = 0 );
    struct QRegionData : public QShared {
#if defined(_WS_WIN_)
	HRGN   rgn;
#elif defined(_WS_X11_)
	Region rgn;
#elif defined(_WS_MAC_)
        void * rgn;
#elif defined(_WS_QWS_)
        void * rgn;
#endif
	bool   is_null;
    } *data;
};


#define QRGN_SETRECT		1		// region stream commands
#define QRGN_SETELLIPSE		2		//  (these are internal)
#define QRGN_SETPTARRAY_ALT	3
#define QRGN_SETPTARRAY_WIND	4
#define QRGN_TRANSLATE		5
#define QRGN_OR			6
#define QRGN_AND		7
#define QRGN_SUB		8
#define QRGN_XOR		9
#define QRGN_RECTS	       10


/*****************************************************************************
  QRegion stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator<<( QDataStream &, const QRegion & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QRegion & );


#endif // QREGION_H
