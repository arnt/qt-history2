#include <qbitarray.h>
#include <stdio.h>

/*
  Shows all data allocated by a QBitArray, including the padded 0s at the
  end.
*/

void showAll( const QBitArray b )
{
    char *d = b.data();
    
    int len = ((b.size() - 1)/8 + 1) *8;
    for ( int i = 0 ; i < len; i++ ) {
	char ch   = d[i>>3];
	char mask = 1 << (i & 7);
	if ( ch & mask )
	    printf("1");
	else
	    printf("0");
    }
}

void doIt( int i )
{
    QBitArray a ( i );

    a.fill( 1 );
    showAll(a);
}


int main( int, char ** )
{    
    for( int i = 0 ; i < 34 ; i++ ) {
	doIt( i );
	printf("\n");
    }
}
