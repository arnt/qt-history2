/*
  editdistance.cpp
*/

#include "editdistance.h"

int editDistance( const QString& s, const QString& t )
{
#define D( i, j ) d[(i) * n + (j)]
    int i;
    int j;
    int m = s.length() + 1;
    int n = t.length() + 1;
    int *d = new int[m * n];
    int result;

    for ( i = 0; i < m; i++ )
	D( i, 0 ) = i;
    for ( j = 0; j < n; j++ )
	D( 0, j ) = j;
    for ( i = 1; i < m; i++ ) {
	for ( j = 1; j < n; j++ ) {
	    if ( s[i - 1] == t[j - 1] ) {
		D( i, j ) = D( i - 1, j - 1 );
	    } else {
		int x = D( i - 1, j );
		int y = D( i - 1, j - 1 );
		int z = D( i, j - 1 );
		D( i, j ) = 1 + QMIN( QMIN(x, y), z );
	    }
	}
    }
    result = D( m - 1, n - 1 );
    delete[] d;
    return result;
#undef D
}

QString nearestName( const QString& actual, const Set<QString>& candidates )
{
    int deltaBest = 10000;
    int numBest;
    QString best;

    Set<QString>::ConstIterator c = candidates.begin();
    while ( c != candidates.end() ) {
	int delta = editDistance( actual, *c );
	if ( delta < deltaBest ) {
	    deltaBest = delta;
	    numBest = 1;
	    best = *c;
	} else if ( delta == deltaBest ) {
	    numBest++;
	}
	++c;
    }

    if ( numBest == 1 && deltaBest <= 2 &&
	 actual.length() + best.length() >= 5 ) {
	return best;
    } else {
	return "";
    }
}
