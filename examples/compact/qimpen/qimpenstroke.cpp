/****************************************************************************
** $Id: //depot/qt/qws/util/qws/qws.h#4 $
**
** Definition of pen input method character
**
** Created : 20000414
**
** Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include <qfile.h>
#include <qtl.h>
#include <math.h>
#include <values.h>
#include "qimpenstroke.h"

#define QIMPEN_CORRELATION_POINTS   30

/*!
  \class QIMPenStroke qimpenstroke.h

  Handles a single stroke.  Can calculate closeness of match to
  another stroke.
*/

QIMPenStroke::QIMPenStroke()
{
}

QIMPenStroke::QIMPenStroke( const QIMPenStroke &st )
{
    startPoint = st.startPoint;
    links = st.links.copy();
}

QIMPenStroke &QIMPenStroke::operator=( const QIMPenStroke &s )
{
    clear();
    qDebug( "copy strokes %d", s.links.count() );
    startPoint = s.startPoint;
    links = s.links.copy();

    return *this;
}

void QIMPenStroke::clear()
{
    startPoint = QPoint(0,0);
    lastPoint = QPoint( 0, 0 );
    links.resize( 0 );
    tsig.resize( 0 );
    dsig.resize( 0 );
    asig.resize( 0 );
}

/*!
  Begin inputting a new stroke.
*/
void QIMPenStroke::beginInput( QPoint p )
{
    clear();
    startPoint = p;
    internalAddPoint( p );
}

/*!
  Add a point to the stroke's shape.
  Returns TRUE if the point was successfully added.
*/
bool QIMPenStroke::addPoint( QPoint p )
{
    if ( links.count() > 500 ) // sanity check (that the user is sane).
        return FALSE;

    int dx = p.x() - lastPoint.x();
    int dy = p.y() - lastPoint.y();
    if ( QABS( dx ) > 1 || QABS( dy ) > 1 ) {
	// The point is not adjacent to the previous point, so we fill
	// in with a straight line.  Some kind of non-linear
	// interpolation might be better.
	int x = lastPoint.x();
	int y = lastPoint.y();
	int ix = 1;
	int iy = 1;
	if ( dx < 0 ) {
	    ix = -1;
	    dx = -dx;
	}
	if ( dy < 0 ) {
	    iy = -1;
	    dy = -dy;
	}
	int d = 0;
	if ( dx < dy ) {
	    d = dx;
	    do {
		y += iy;
		d += dx;
		if ( d > dy ) {
		    x += ix;
		    d -= dy;
		}
		internalAddPoint( QPoint( x, y ) );
	    } while ( y != p.y() );
	} else {
	    d = dy;
	    do {
		x += ix;
		d += dy;
		if ( d > dx ) {
		    y += iy;
		    d -= dx;
		}
		internalAddPoint( QPoint( x, y ) );
	    } while ( x != p.x() );
	}
    } else {
	internalAddPoint( p );
    }

    return TRUE;
}

/*!
  Finish inputting a stroke.
*/
void QIMPenStroke::endInput()
{
    if ( links.count() < 3 ) {
	QIMPenGlyphLink gl;
	links.resize(1);
	gl.dx = 1;
	gl.dy = 0;
	links[0] = gl;
    }

    qDebug("Points: %d", links.count() );
}

/*!
  Return an indicator of the closeness of this stroke to \a pen.
  Lower value is better.
*/
int QIMPenStroke::match( QIMPenStroke *pen )
{
    if ( links.count() * 3 < pen->links.count() ||
         links.count() > pen->links.count() * 3 ) {
        return 200000;
    }
    createSignatures();
    pen->createSignatures();

    // do a correlation with the three available signatures.
    int err1 = MAXINT;
    int err2 = MAXINT;
    int err3 = MAXINT;

    // base has extra points at the start and end to enable
    // correlation of a sliding window with the pen supplied.
    QArray<int> base = createBase( tsig, 2 );
    for ( int i = 0; i < 4; i++ ) {
        int e = calcError( base, pen->tsig, i, TRUE );
        if ( e < err1 )
            err1 = e;
    }

    if ( err1 > 50 )   // no need for more matching
        return 200000;

    // maybe a sliding window is worthwhile for these too.
    err2 = calcError( dsig, pen->dsig, 0, FALSE );
    err3 = calcError( asig, pen->asig, 0, TRUE );

    // Some magic numbers here - the addition reduces the weighting of
    // the error and compensates for the different error scales.  I
    // consider the tangent signature to be the best indicator, so it
    // has the most weight.  This ain't rocket science.
    // Basically, these numbers are the tuning factors.
    int err = err1 * ( err2 + 50 ) * ( err3 + 20 );

//    qDebug( "err %d   ( %d, %d, %d)", err, err1, err2, err3 );

    return err;
}

/*!
  Return the bounding rect of this stroke.  It may have sides with
  negative coords since its origin is where the user started drawing
  the character.
*/
QRect QIMPenStroke::boundingRect()
{
    int x = startPoint.x();
    int y = startPoint.y();
    QRect br( x, y, 1, 1 );

    for ( unsigned i = 0; i < links.count(); i++ ) {
	x += links[i].dx;
	y += links[i].dy;
	if ( x < br.left() )
	    br.setLeft( x );
	if ( x > br.right() )
	    br.setRight( x );
	if ( y < br.top() )
	    br.setTop( y );
	if ( y > br.bottom() )
	    br.setBottom( y );
    }

    return br;
}


/*!
  Perform a correlation of the supplied arrays.  \a base should have
  win.count() + 2 * off points to enable sliding \a win over the
  \a base data.  If \a t is TRUE, the comparison takes into account
  the circular nature of the angular data.
  Returns the best (lowest error) match.
*/

int QIMPenStroke::calcError( const QArray<int> &base,
                           const QArray<int> &win, int off, bool t )
{
    int err = 0;

    for ( unsigned i = 0; i < win.count(); i++ ) {
        int d = QABS( base[i+off] - win[i] );
        if ( t && d > 128 )
            d -= 256;
        err += QABS( d );
    }

    err /= win.count();

    return err;
}

/*!
  Creates signatures used in matching if not already created.
*/
void QIMPenStroke::createSignatures()
{
    if ( tsig.isEmpty() )
	createTanSignature();
    if ( asig.isEmpty() )
	createAngleSignature();
    if ( dsig.isEmpty() )
	createDistSignature();
}

/*!
  Create a signature of the tangents to the user's stroke.
*/
void QIMPenStroke::createTanSignature()
{
    int dist = 5; // number of points to include in calculation
    if ( (int)links.count() <= dist ) {
        tsig.resize(1);
        int dx = 0;
        int dy = 0;
        for ( unsigned j = 0; j < links.count(); j++ ) {
            dx += links[j].dx;
            dy += links[j].dy;
        }
        tsig[0] = arcTan( dy, dx );
    } else {
        tsig.resize( (links.count()-dist+1) / 2 );
        int idx = 0;
        for ( unsigned i = 0; i < links.count() - dist; i += 2 ) {
            int dx = 0;
            int dy = 0;
            for ( int j = 0; j < dist; j++ ) {
                dx += links[i+j].dx;
                dy += links[i+j].dy;
            }
            tsig[idx++] = arcTan( dy, dx );
        }
    }

    tsig = scale( tsig, QIMPEN_CORRELATION_POINTS, TRUE );
//    smooth(tsig);
}

/*!
  Create a signature of the change in angle.
*/
void QIMPenStroke::createAngleSignature()
{
    if ( tsig.isEmpty() )
        createTanSignature();

    if ( tsig.count() < 5 ) {
        asig.resize( 1 );
        asig[0] = 0;
    } else {
        asig.resize( tsig.count() - 5 );

        for ( unsigned i = 0; i < asig.count(); i++ ) {
            asig[i] = QABS(tsig[i] - tsig[i+5]);
        }
    }
}

/*!
  Create a signature of the distance from the char's center of gravity
  to its points.
*/
void QIMPenStroke::createDistSignature()
{
    dsig.resize( (links.count()+1)/2 );
    QPoint c = calcCenter();
    QPoint pt( 0, 0 );

    int minval = MAXINT;
    int maxval = 0;
    int idx = 0;
    for ( unsigned i = 0; i < links.count(); i += 2 ) {
        int dx = c.x() - pt.x();
        int dy = c.y() - pt.y();
        if ( dx == 0 && dy == 0 )
            dsig[idx] = 0;
        else
            dsig[idx] = dx*dx + dy*dy;

        if ( dsig[idx] > maxval )
            maxval = dsig[idx];
        if ( dsig[idx] < minval )
            minval = dsig[idx];
        pt.rx() += links[i].dx;
        pt.ry() += links[i].dy;
        idx++;
    }

    // normalise 0-255
    int div = maxval - minval;
    if ( div == 0 ) div = 1;
    for ( unsigned i = 0; i < dsig.count(); i++ ) {
        dsig[i] = (dsig[i] - minval ) * 255 / div;
    }

    dsig = scale( dsig, QIMPEN_CORRELATION_POINTS );
}


/*!
  Scale the points in a array to \a count points.
  This is braindead at the moment (no smooth scaling) and fixing this is
  probably one of the simpler ways to improve performance.
*/
QArray<int> QIMPenStroke::scale( const QArray<int> &s, unsigned count, bool t )
{
    QArray<int> d(count);

    unsigned si = 0;
    if ( s.count() > count ) {
        unsigned next = 0;
        for ( unsigned i = 0; i < count; i++ ) {
            next = (i+1) * s.count() / count;
            int maxval = 0;
            if ( t ) {
                for ( unsigned j = si; j < next; j++ ) {
                    maxval = s[j] > maxval ? s[j] : maxval;
                }
            }
            int sum = 0;
            for ( unsigned j = si; j < next; j++ ) {
                if ( t && maxval - s[j] > 128 )
                    sum += 256;
                sum += s[j];
            }
            d[i] = sum / (next-si);
            if ( t && d[i] > 256 )
                d[i] %= 256;
            si = next;
        }
    } else {
        for ( unsigned i = 0; i < count; i++ ) {
            si = i * s.count() / count;
            d[i] = s[si];
        }
    }

    return d;
}

/*!
  Add another point to the stroke's shape.
*/
void QIMPenStroke::internalAddPoint( QPoint p )
{
    if ( p == lastPoint )
        return;

    if ( !lastPoint.isNull() ) {
        QIMPenGlyphLink gl;
        gl.dx = p.x() - lastPoint.x();
        gl.dy = p.y() - lastPoint.y();
        links.resize( links.size() + 1 );   //### resize by 1 is bad
        links[links.size() - 1] = gl;
    }

    lastPoint = p;
}

/*!
  Calculate the center of gravity of the stroke.
*/
QPoint QIMPenStroke::calcCenter()
{
    QPoint pt( 0, 0 );
    int ax = 0;
    int ay = 0;

    for ( unsigned i = 0; i < links.count(); i++ ) {
        pt.rx() += links[i].dx;
        pt.ry() += links[i].dy;
        ax += pt.x();
        ay += pt.y();
    }

    ax /= (int)links.count();
    ay /= (int)links.count();

    return QPoint( ax, ay );
}

/*!
  Calculate the arctan of the lengths supplied.
  The angle returned is in the range 0-255.
  \a dy and \a dx MUST be in the range 0-5 - I dont even check :-P
*/
int QIMPenStroke::arcTan( int dy, int dx )
{
    if ( dx == 0 ) {
        if ( dy >= 0 )
            return 64;
        else
            return 192;
    }

    if ( dy == 0 ) {
        if ( dx >= 0 )
            return 0;
        else
            return 128;
    }

    static int table[5][5] = {
        { 32, 19, 13, 10, 8  },
        { 45, 32, 24, 19, 16 },
        { 51, 40, 32, 26, 22 },
        { 54, 45, 37, 32, 27 },
        { 56, 49, 42, 37, 32 } };

    if ( dy > 0 ) {
        if ( dx > 0 )
            return table[dy-1][dx-1];
        else
            return 128 - table[dy-1][QABS(dx)-1];
    } else {
        if ( dx > 0 )
            return 256 - table[QABS(dy)-1][dx-1];
        else
            return 128 + table[QABS(dy)-1][QABS(dx)-1];
    }

    return 0;
}


/*!
  Silly name.  Create an array that has \a e points extra at the start and
  end to enable a sliding correlation to be performed.
*/
QArray<int> QIMPenStroke::createBase( const QArray<int> a, int e )
{
    QArray<int> ra( a.count() + 2*e );

    for ( int i = 0; i < e; i++ ) {
        ra[i] = a[e - i - 1];
        ra[a.count() + i] = a[a.count() - i - 1];
    }
    for ( unsigned i = 0; i < a.count(); i++ ) {
        ra[i+e] = a[i];
    }

    return ra;
}


/*!
  Smooth the points in an array.  Probably a bad idea.
*/
void QIMPenStroke::smooth( QArray<int> &sig)
{
    QArray<int> nsig = sig.copy();

    int a;
    for ( unsigned i = 1; i < sig.count()-2; i++ ) {
        a = 0;
        for ( int j = -1; j <= 1; j++ ) {
            a += sig[ i + j ];
        }
        nsig[i] = a / 3;
    }

    sig = nsig;
}

/*!
  Write the character's data to the stream.
*/
QDataStream &operator<< (QDataStream &s, const QIMPenStroke &ws)
{
    s << ws.startPoint;
    s << ws.links.count();
    for ( unsigned i = 0; i < ws.links.count(); i++ ) {
        s << (Q_INT8)ws.links[i].dx;
        s << (Q_INT8)ws.links[i].dy;
    }

    return s;
}

/*!
  Read the character's data from the stream.
*/
QDataStream &operator>> (QDataStream &s, QIMPenStroke &ws)
{
    Q_INT8 i8;
    s >> ws.startPoint;
    unsigned size;
    s >> size;
    ws.links.resize( size );
    for ( unsigned i = 0; i < size; i++ ) {
        s >> i8;
	ws.links[i].dx = i8;
        s >> i8;
	ws.links[i].dy = i8;
    }

    return s;
}


