#include <qpen.h>
#include "mypainter.h"

MyPainter::MyPainter()
{
}

MyPainter::MyPainter( const QPaintDevice * pd, bool unclipped )
    : QPainter( pd, unclipped )
{
}

MyPainter::MyPainter( const QPaintDevice * pd, const QWidget * copyAttributes, bool unclipped )
    : QPainter( pd, copyAttributes, unclipped )
{
}

MyPainter::~MyPainter()
{
}

void MyPainter::drawPolyPolygon (
     const QPointArray &pa, // one point array containing all polygons
                            // sequentially
     const int *start,      // start[i] contains start index of polygon i
                            // start[ncomp+1] contains total number of points
     int ncomp,             // number of components
     bool winding           // is just passed to drawPolygon
     )
{   
    int i;

    if( ncomp ==1 ) {  // just 1 component - this is simple
	drawPolygon(pa,winding);
	return;
    }

    // add points of backward chain
    QPointArray mypa( pa );
    int size = mypa.size() + ncomp - 1;
    mypa.resize( size );
    for( i = 0 ; i<ncomp-1 ; i++ ){
	mypa.setPoint( --size, mypa.point(start[i]) );
    }

    // remove pen temporaryly and fill "chained" polygon
    QPen penBak = pen();
    setPen( NoPen );
    drawPolygon( mypa, winding );
    setPen( penBak );

    // draw outline of components
    for( i = 0 ; i<ncomp ; i++ )
	drawPolyline( mypa, start[i], start[i+1]-start[i] );
}
