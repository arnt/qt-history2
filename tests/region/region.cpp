#include <qapplication.h>
#include <qregion.h>
#include <qpointarray.h>


void outr( QRect r )
{
    qDebug( "  %d,%d,%d,%d", r.x(), r.y(), r.width(), r.height() );
}

void print( QRegion region )
{
    QRect r = region.boundingRect();
    qDebug( "Bounding rect %d,%d,%d,%d", r.x(), r.y(), r.width(), r.height() );
    QMemArray<QRect> a = region.rects();
    qDebug( "nrects = %d", a.size() );
    for ( int i=0; i<(int)a.size(); i++ )
	outr( a[i] );
}


int main( int argc, char **argv )
{
    QApplication app(argc,argv);

    QRegion empty;
    qDebug( "test empty region" );
    print( empty );

    QRegion r1( QRect(10,20, 50, 80) );
    QRegion r2( QRect(30,30, 50, 90) );

    qDebug( "\ntest union of two regions" );
    QRegion r3 = r1.unite( r2 );
    print( r3 );

    qDebug( "\ntest elliptic regions" );
    QRegion r4( QRect(100,100,15,15), QRegion::Ellipse );
    print( r4 );

    qDebug("\nempty polygon");
    QPointArray pa;
    QRegion r5(pa);
    print(r5);

    app.unlock();
    return 0;
}
