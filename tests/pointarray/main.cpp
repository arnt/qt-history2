

#include <qpointarray.h>


int isP1( QPoint p )
{
    if ( p.x() == 1 && p.y() == 1 )
	return 1;
    return 0;
}

void changeIt( QPoint* p )
{
    *p = QPoint( 5, 5 );
}

int checkConst( const QPointArray& a1 )
{
    QPoint p1( 1, 1 );
    QPoint p2( 2, 2 );

    ASSERT( a1[0] == p2 );
    ASSERT( a1[1] == p1 );
    ASSERT( a1[2] == p1 );

    ASSERT( a1[0] != p1 );
    ASSERT( a1[1] != p2 );
    ASSERT( a1[2] != p2 );

    QPoint p( a1[1] );
    ASSERT( p == p1 );
    ASSERT( isP1( p ) );

    ASSERT( isP1( a1[1] ) );
    ASSERT( !isP1( a1[0] ) );

    int i = 2;
    uint u = 2;
    ASSERT( a1[i] == a1[u] );
    ASSERT( a1.at(1) == a1.at(u) );
    ASSERT( a1[i] == p1 );
    ASSERT( a1[u] == p1 );
    ASSERT( p1 == a1[i] );
    ASSERT( p1 == a1[u] );
    
    ASSERT( 1 == a1[2].x() );
    ASSERT( a1[2].y() == 1 );

    return 1;
}


int main() {

    QPointArray a1( 3 );
    QPoint p1( 1, 1 );
    QPoint p2( 2, 2 );

    a1.fill( p1 );
    ASSERT( a1[0] == p1 );
    ASSERT( a1[1] == p1 );
    ASSERT( a1[2] == p1 );

    ASSERT( a1[0] != p2 );
    ASSERT( a1[1] != p2 );
    ASSERT( a1[2] != p2 );

    a1[0] = p2;
    ASSERT( a1[0] == p2 );
    ASSERT( a1[0] != p1 );

    QPoint p( a1[1] );
    ASSERT( p == p1 );
    ASSERT( isP1( p ) );

    ASSERT( isP1( a1[1] ) );
    ASSERT( !isP1( a1[0] ) );

    int i = 2;
    uint u = 2;
    ASSERT( a1[i] == a1[u] );
    ASSERT( a1.at(1) == a1.at(u) );
    ASSERT( a1[i] == p1 );
    ASSERT( a1[u] == p1 );
    ASSERT( p1 == a1[i] );
    ASSERT( p1 == a1[u] );
    
    QPoint p3( 3, 3 );
    QPoint p4( 4, 4 );
    a1[2] += p3;
    ASSERT( a1[2] == p4 );
    ASSERT( a1[2].x() == 4 );
    ASSERT( a1[2].y() == 4 );
    a1[2] -= p3;
    ASSERT( a1[2] == p1 );
    ASSERT( a1[2].x() == 1 );
    ASSERT( a1[2].y() == 1 );    

    ASSERT( checkConst( a1 ) );

    QPoint p5( 5, 5 );
    changeIt( &a1[1] );
    ASSERT( a1[1] == p5 );

    return 0;
}
