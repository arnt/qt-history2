/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qregion.h#24 $
**
** Definition of QRegion class
**
** Created : 940514
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QREGION_H
#define QREGION_H

#include "qshared.h"
#include "qrect.h"
#include "qstring.h"


class QRegion
{
public:
    enum RegionType { Rectangle, Ellipse };

    QRegion();
    QRegion( const QRect &, RegionType = Rectangle );
    QRegion( const QPointArray &, bool winding=FALSE );
    QRegion( const QRegion & );
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
    QRegion xor( const QRegion & )	const;

    bool    operator==( const QRegion & )  const;
    bool    operator!=( const QRegion &r ) const
			{ return !(operator==(r)); }

#if defined(_WS_WIN_)
    HANDLE  handle() const { return data->rgn; }
#elif defined(_WS_PM_)
    HANDLE  handle() const { return data->rgn; }
#elif defined(_WS_X11_)
    Region  handle() const { return data->rgn; }
#endif

    friend QDataStream &operator<<( QDataStream &, const QRegion & );
    friend QDataStream &operator>>( QDataStream &, QRegion & );

private:
    QRegion( bool );
    QRegion copy() const;
    void    detach();
#if defined(_WS_WIN_)
    QRegion winCombine( const QRegion &, int ) const;
#endif
    void    cmd( int id, void *, const QRegion * = 0, const QRegion * = 0 );
    void    exec( const QByteArray & );
    struct QRegionData : QShared {
	QByteArray bop;
#if defined(_WS_WIN_)
	HANDLE rgn;
#elif defined(_WS_PM_)
	HANDLE rgn;
#elif defined(_WS_X11_)
	Region rgn;
#endif
    } *data;
#if defined(_WS_PM_)
    static HPS hps;
#endif
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


/*****************************************************************************
  QRegion stream functions
 *****************************************************************************/

QDataStream &operator<<( QDataStream &, const QRegion & );
QDataStream &operator>>( QDataStream &, QRegion & );


#endif // QREGION_H
