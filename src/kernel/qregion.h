/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qregion.h#3 $
**
** Definition of QRegion class
**
** Author  : Haavard Nord
** Created : 940514
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QREGION_H
#define QREGION_H

#include "qshared.h"
#include "qrect.h"
#include "qstring.h"


class QRegion
{
friend class QPainter;
public:
    enum RegionType { Rectangle, Ellipse };

    QRegion();
    QRegion( const QRect &, RegionType = Rectangle );
    QRegion( const QPointArray & );
    QRegion( const QRegion & );
   ~QRegion();
    QRegion &operator=( const QRegion & );

    QRegion copy() const;

    bool    isNull()  const;

    bool    contains( const QPoint &p ) const;
    bool    contains( const QRect &r )	const;

    void    move( int dx, int dy );

    QRegion unite( const QRegion & )	const;
    QRegion intersect( const QRegion &) const;
    QRegion subtract( const QRegion & ) const;
    QRegion xor( const QRegion & )	const;

    bool    operator==( const QRegion & );
    bool    operator!=( const QRegion &r )
    			{ return !(operator==(r)); }

    friend QDataStream &operator<<( QDataStream &, const QRegion & );
    friend QDataStream &operator>>( QDataStream &, QRegion & );

private:
    void    cmd( int id, void *, const QRegion * = 0, const QRegion * = 0 );
    void    exec();
    struct QRegionData : QShared {		// region data
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


#define QRGN_NOP	0			// region stream commands
#define QRGN_SETRECT	1
#define QRGN_SETELLIPSE	2
#define QRGN_SETPTARRAY	3
#define QRGN_MOVE	4
#define QRGN_OR		5
#define QRGN_AND	6
#define QRGN_SUB	7
#define QRGN_XOR	8


#endif // QREGION_H
