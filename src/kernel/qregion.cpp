/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qregion.cpp#15 $
**
** Implementation of QRegion class
**
** Created : 950726
**
** Copyright (C) 1995-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qregion.h"
#include "qpntarry.h"
#include "qbuffer.h"
#include "qdstream.h"

RCSTAG("$Id: //depot/qt/main/src/kernel/qregion.cpp#15 $");


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

  \sa QPainter::setClipRegion(), QPainter::setClipRect()
*/


/*****************************************************************************
  QRegion member functions
 *****************************************************************************/

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
  Stores a region command in an internal buffer so it can be played back
  later using exec().

  All region commands (unite, intersect etc.) are stored in the buffer.
  We need to do this because it is not possible to access the native
  region data on most platforms. This seems to be the only way to support
  regions for QPictures.  We have sacrified speed for compatibility.
*/

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
	case QRGN_TRANSLATE:
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


/*!
  Executes region commands in the internal buffer and rebuild the original
  region.

  We do this when we read a region from the data stream.
*/

void QRegion::exec( const QByteArray &buffer )
{
    QBuffer buf( buffer );
    QDataStream s( &buf );
    buf.open( IO_ReadOnly );
    QRegion rgn;
#if defined(DEBUG)
    int test_cnt = 0;
#endif
    while ( !s.eof() ) {
	int id;
	s >> id;
#if defined(DEBUG)
	if ( test_cnt > 0 && id != QRGN_TRANSLATE )
	    warning( "QRegion::exec: Internal error" );
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
	else if ( id == QRGN_TRANSLATE ) {
	    QPoint p;
	    s >> p;
#if defined(DEBUG)
	    ASSERT( !rgn.data->bop.isNull() );
#endif
	    rgn.translate( p.x(), p.y() );
	}
	else if ( id >= QRGN_OR && id <= QRGN_XOR ) {
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
    return s << r.data->bop;
}

/*!
  \relates QRegion
  Reads a region from the stream and returns a reference to the stream.
*/

QDataStream &operator>>( QDataStream &s, QRegion &r )
{
    QByteArray b;
    s >> b;
    r.exec( b );
    return s;
}


// OBSOLETE
#if !(defined(__STRICT_ANSI__) && defined(_CC_GNU_)) && !defined(_CC_EDG_) && !defined(xor)
/*!
  OBSOLETE - Use eor() instead.
*/
QRegion QRegion::xor( const QRegion &r ) const
{
    return eor(r);
}
#endif

