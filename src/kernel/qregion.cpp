/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qregion.cpp#35 $
**
** Implementation of QRegion class
**
** Created : 950726
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
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
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qregion.h"
#include "qpointarray.h"
#include "qbuffer.h"
#include "qdatastream.h"

/*!
  \class QRegion qregion.h
  \brief The QRegion class specifies a clip region for a painter.

  \ingroup drawing

  A region can be a rectangle, an ellipse, a polygon or a combination
  of these.

  Regions are combined by creating a new region which is a
  union, intersection or difference between any two regions.

  The region XOR operation is defined as:
  \code
    a XOR b = (a UNION b) - (a INTERSECTION b)
  \endcode

  Example of using complex regions:
  \code
    void MyWidget::paintEvent( QPaintEvent * )
    {
	QPainter p;				// our painter
	QRegion r1( QRect(100,100,200,80),	// r1 = elliptic region
		    QRegion::Ellipse );
	QRegion r2( QRect(100,120,90,30) );	// r2 = rectangular region
	QRegion r3 = r1.intersect( r2 );	// r3 = intersection
	p.begin( this );			// start painting widget
	p.setClipRegion( r3 );			// set clip region
	...					// paint clipped graphics
	p.end();				// painting done
    }
  \endcode

  \sa QPainter::setClipRegion(), QPainter::setClipRect()
*/


/*****************************************************************************
  QRegion member functions
 *****************************************************************************/

/*!
  Constructs a rectangular or elliptic region.

  \a x, \a y, \a w, and \a h specify the region rectangle.
  \a t is the region type: QRegion::Rectangle (default) or
  QRegion::Ellipse.
*/
QRegion::QRegion( int x, int y, int w, int h, RegionType t )
{
    QRegion tmp(QRect(x,y,w,h),t);
    tmp.data->ref();
    data = tmp.data;
}

/*!
  Detaches from shared region data to makes sure that this region is the
  only one referring the data.

  If multiple regions share common data, this region dereferences the data
  and gets a copy of the data. Nothing is done if there is just a single
  reference.
*/

void QRegion::detach()
{
    if ( data->count != 1 )
	*this = copy();
}


/*!
  Executes region commands in the internal buffer and rebuild the original
  region.

  We do this when we read a region from the data stream.

  If \a ver is non-0, uses the format version \a ver on reading the
  byte array.

*/

void QRegion::exec( const QByteArray &buffer, int ver )
{
    QBuffer buf( buffer );
    QDataStream s( &buf );
    if ( ver )
	s.setVersion( ver );
    buf.open( IO_ReadOnly );
    QRegion rgn;
#if defined(CHECK_STATE)
    int test_cnt = 0;
#endif
    while ( !s.eof() ) {
	int id;
	s >> id;
#if defined(CHECK_STATE)
	if ( test_cnt > 0 && id != QRGN_TRANSLATE )
	    warning( "QRegion::exec: Internal error" );
	test_cnt++;
#endif
	if ( id == QRGN_SETRECT || id == QRGN_SETELLIPSE ) {
	    QRect r;
	    s >> r;
	    rgn = QRegion( r, id == QRGN_SETRECT ? Rectangle : Ellipse );
	} else if ( id == QRGN_SETPTARRAY_ALT || id == QRGN_SETPTARRAY_WIND ) {
	    QPointArray a;
	    s >> a;
	    rgn = QRegion( a, id == QRGN_SETPTARRAY_WIND );
	} else if ( id == QRGN_TRANSLATE ) {
	    QPoint p;
	    s >> p;
	    rgn.translate( p.x(), p.y() );
	} else if ( id >= QRGN_OR && id <= QRGN_XOR ) {
	    QByteArray bop1, bop2;
	    QRegion r1, r2;
	    s >> bop1;	r1.exec( bop1 );
	    s >> bop2;	r2.exec( bop2 );
	    switch ( id ) {
		case QRGN_OR:
		    rgn = r1.unite( r2 );
		    break;
		case QRGN_AND:
		    rgn = r1.intersect( r2 );
		    break;
		case QRGN_SUB:
		    rgn = r1.subtract( r2 );
		    break;
		case QRGN_XOR:
		    rgn = r1.eor( r2 );
		    break;
	    }
	} else if ( id == QRGN_RECTS ) {
	    // (This is the only form used in Qt 2.0)
	    Q_UINT32 n;
	    s >> n;
	    QRect r;
	    for ( int i=0; i<(int)n; i++ ) {
		s >> r;
		rgn = rgn.unite( QRegion(r) );
	    }
	}
    }
    buf.close();
    *this = rgn;
}


/*****************************************************************************
  QRegion stream functions
 *****************************************************************************/

/*!
  \relates QRegion
  Writes a region to the stream and returns a reference to the stream.
*/

QDataStream &operator<<( QDataStream &s, const QRegion &r )
{
    QArray<QRect> a = r.rects();
    if ( a.isEmpty() ) {
	s << (Q_UINT32)0;
    } else {
	if ( s.version() == 1 ) {
	    int i;
	    for ( i=(int)a.size()-1; i>0; i-- ) {
		s << (Q_UINT32)(12+i*24);
		s << (int)QRGN_OR;
	    }
	    for ( i=0; i<(int)a.size(); i++ ) {
		s << (Q_UINT32)(4+8) << (int)QRGN_SETRECT << a[i];
	    }
	}
	else {
	    s << (Q_UINT32)(4+4+16*a.size()); // 16: storage size of QRect
	    s << (int)QRGN_RECTS;
	    s << (Q_UINT32)a.size();
	    for ( int i=0; i<(int)a.size(); i++ )
		s << a[i];
	}
    }
    return s;
}

/*!
  \relates QRegion
  Reads a region from the stream and returns a reference to the stream.
*/

QDataStream &operator>>( QDataStream &s, QRegion &r )
{
    QByteArray b;
    s >> b;
    r.exec( b, s.version() );
    return s;
}
