/* Patches the internal expiry date */

#include <qfile.h>

int old_date = 0x002565f7;	// May 1, 1998
int new_date = 0x002566cd;	// Dec 1, 1998


int main( int argc, char **argv )
{
    char *old_p = (char *)&old_date;
    char *new_p = (char *)&new_date;
    QFile f(argv[1]);
    if ( !f.open(IO_ReadOnly) ) {
	warning( "Could not open file %s", f.name() );
	return 1;
    }
    int offset = -1;
    QByteArray d(f.size());
    QByteArray buf(f.size());
    f.readBlock(buf.data(),buf.size());
    f.close();
    int sz = sizeof(int);
    char *p = buf.data();
    char *end = p + buf.size()-sz-1;
    while ( p < end ) {
	if ( memcmp(p,old_p,sz) == 0 ) {
	    if ( offset == -1 ) {
		offset = (int)p - (int)buf.data();
	    } else {
		warning( "Found the date more than one place in %s, bye",
			 f.name() );
		return 1;
	    }
	}
	p++;
    }
    if ( offset < 0 ) {
	warning( "No pattern found in %s", f.name() );
	return 1;
    }
    debug( "Found the pattern on position %d, patching...", offset );
    memcpy( buf.data()+offset, new_p, sz );
    debug( "Write back to file %s", f.name() );
    f.open( IO_WriteOnly );
    f.writeBlock(buf.data(),buf.size());
    f.close();
    return 0;
}
