#include "dict.h"
#include <qdict.h>
#include <qstring.h>
#include <qdatetm.h>
#include <stdlib.h>

QString keyFor( int i )
{
    QString key;
    key.sprintf("KEY%05d",i);
    return key;
}

main(int argc, char** argv)
{
    int nins = argv[1] ? atoi(argv[1]) : 3;
    QDict<int> dict(5);

    for ( int i=0; i<nins; i++ ) {
	int* d = new int;
	*d = i;
	dict.insert(keyFor(i),d);
    }

    QTime timer;
    for ( int j=nins/500; j<1000000; j+=1+j/10 ) {
	timer.start();
	dict.resize( j );
	int ms_r = timer.elapsed();
	int n=0;
	timer.start();
	for ( QDictIterator<int> it(dict); it.current(); ++it ) {
	    n++;
	    if ( keyFor( *it.current() ) != it.currentKey() ) {
		fatal("Wrong key");
	    }
	}
	if ( n != nins )
	    fatal("Too few");
	int ms_i = timer.elapsed();
	timer.start();
	for ( int i = 0; i<nins; i++ ) {
	    dict.find( keyFor(i) );
	}
	int ms_f = timer.elapsed();
	debug("resize(%d) took %dms, iteration took %dms, find took %0.1f\265s", j, ms_r, ms_i, 1000.0*ms_f/nins);
    }
    dict.resize( 10 );
}
