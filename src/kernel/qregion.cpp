/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qregion.cpp#4 $
**
** Implementation of QRegion class
**
** Author  : Haavard Nord
** Created : 950726
**
** Copyright (C) 1995 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#include "qregion.h"
#include "qpntarry.h"
#include "qbuffer.h"
#include "qdstream.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qregion.cpp#4 $";
#endif


/*!
  \class QRegion qregion.h
  \brief The QRegion class specifies a clip region for a painter.

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

  \sa QPainter::setClipRegion(), QPainter::setClipRect() */


// --------------------------------------------------------------------------
// QRegion member functions
//

void QRegion::cmd( int id, void *param, const QRegion *r1, const QRegion *r2 )
{
    QBuffer buf( data->bop );
    QDataStream s( &buf );
    buf.open( IO_WriteOnly );
    buf.at( buf.size() );
    s << id;
    switch ( id ) {
	case QRGN_SETRECT:
	case QRGN_SETELLIPSE:
	    s << *((QRect*)param);
	    break;
	case QRGN_SETPTARRAY_ALT:
	case QRGN_SETPTARRAY_WIND:
	    s << *((QPointArray*)param);
	    break;
	case QRGN_MOVE:
	    s << *((QPoint*)param);
	    break;
	case QRGN_OR:
	case QRGN_AND:
	case QRGN_SUB:
	case QRGN_XOR:
	    s << r1->data->bop << r2->data->bop;
	    break;
#if defined(CHECK_RANGE)
	default:
	    warning( "QRegion: Internal cmd error" );
#endif
    }
    buf.close();
}


void QRegion::exec()
{
    QBuffer buf( data->bop );
    QDataStream s( &buf );
    buf.open( IO_ReadOnly );
    QRegion rgn;
#if defined(DEBUG)
    int test_cnt = 0;
#endif
    while ( !s.eos() ) {
	int id;
	s >> id;
#if defined(DEBUG)
	if ( test_cnt > 0 && id != QRGN_MOVE )
	    warning( "QRegion: Internal exec error" );
	test_cnt++;
#endif
	if ( id == QRGN_SETRECT || id == QRGN_SETELLIPSE ) {
	    QRect r;
	    s >> r;
	    rgn = QRegion( r, id == QRGN_SETRECT ? Rectangle : Ellipse );
	}
	else if ( id == QRGN_SETPTARRAY_ALT || id == QRGN_SETPTARRAY_WIND ) {
	    QPointArray a;
	    s >> a;
	    rgn = QRegion( a, id == QRGN_SETPTARRAY_WIND );
	}
	else if ( id == QRGN_MOVE ) {
	    QPoint p;
	    s >> p;
	    rgn = *this;
	    rgn.move( p.x(), p.y() );
	}
	else if ( id >= QRGN_OR && id <= QRGN_XOR ) {
	    QByteArray bop1, bop2;
	    s >> bop1;
	    s >> bop2;
	    QRegion r1, r2;
	    r1.data->bop = bop1;
	    r2.data->bop = bop2;
	    r1.exec();
	    r2.exec();
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
		    rgn = r1.xor( r2 );
		    break;
	    }
	}
    }
    buf.close();
    *this = rgn;
}


// --------------------------------------------------------------------------
// QRegion stream functions
//

/*!
  \relates QRegion
  Writes a region to the stream.
*/

QDataStream &operator<<( QDataStream &s, const QRegion &r )
{
    return s << r.data->bop;
}

/*!
  \relates QRegion
  Reads a region from the stream.
*/

QDataStream &operator>>( QDataStream &s, QRegion &r )
{
    QRegion newr;
    QByteArray b;
    s >> b;
    newr.data->bop = b;
    newr.exec();
    r = newr;
    return s;
}
