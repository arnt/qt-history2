#include <qapp.h>
#include <qregion.h>


void outr( QRect r )
{
    debug( "  %d,%d,%d,%d", r.x(), r.y(), r.width(), r.height() );
}

void print( QRegion region )
{
    QRect r = region.boundingRect();
    debug( "Bounding rect %d,%d,%d,%d", r.x(), r.y(), r.width(), r.height() );
    QArray<QRect> a = region.getRects();
    debug( "nrects = %d", a.size() );
    for ( int i=0; i<(int)a.size(); i++ )
	outr( a[i] );
}


int main( int argc, char **argv )
{
    QApplication app(argc,argv);

    QRegion empty;
    debug( "test empty region" );
    print( empty );

    QRegion r1( QRect(10,20, 50, 80) );
    QRegion r2( QRect(30,30, 50, 90) );

    debug( "\ntest union of two regions" );
    QRegion r3 = r1.unite( r2 );
    print( r3 );

    return 0;
}
