/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qregion.h#2 $
**
** Definition of QRegion class
**
** Author  : Haavard Nord
** Created : 940514
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#ifndef QREGION_H
#define QREGION_H

#include "qshared.h"
#include "qrect.h"


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

    bool    isNull()  const;

    bool    contains( const QPoint &p ) const;
    bool    contains( const QRect &r )	const;

    void    move( int dx, int dy );

    QRegion unite( const QRegion & )	const;
    QRegion intersect( const QRegion &) const;
    QRegion subtract( const QRegion & ) const;
    QRegion xor( const QRegion & )	const;

private:
    struct QRegionData : QShared {		// region data
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


#endif // QREGION_H
