#include "qurl.h"
#include <qapplication.h>

int main( int argc, char* argv[]  )
{
    QUrl u( "ftp://reggie:blablub@ftp.troll.no/home/reggie/a_woooooohhhhhnsinn.html#woansinn" );
    
    qDebug( "URL: ftp://reggie:blablub@ftp.troll.no/home/reggie/a_woooooohhhhhnsinn.html#woansinn\n\n"
	    "protocol: %s\n"
	    "user: %s\n"
	    "passwd: %s\n"
	    "host: %s\n"
	    "path: %s\n"
	    "ref: %s\n",
	    u.protocol().latin1(),
	    u.user().latin1(),
	    u.pass().latin1(),
	    u.host().latin1(),
	    u.path().latin1(),
	    u.ref().latin1() );
}
