/*
  test.cpp

  Test driver for regular expressions.
*/

#include <qregexp.h>
#include <stdio.h>

void myMessageOutput( QtMsgType type, const char *msg )
{
    switch ( type ) {
    case QtDebugMsg:
	fprintf( stderr, "%s\n", msg );
	break;
    case QtWarningMsg:
    case QtFatalMsg:
	fprintf( stderr, "Error: %s\n", msg );
	abort();
    }
}

int main()
{
    qInstallMsgHandler( myMessageOutput );
    char buf[200000];
    while ( fgets(buf, sizeof(buf), stdin) != 0 ) {
	QStringList s = QStringList::split( QString(" # "), QString(buf),
					    TRUE );
	if ( s.count() != 3 )
	    fprintf( stderr, "Something is wrong with the test file\n" );
	QRegExp rx( s[0].isNull() ? QString::fromLatin1("") : s[0] );
	rx.setMinimal( FALSE );
	if ( !rx.isValid() )
	    qDebug( "Not valid" );
	int pos;
	QString ss = "";

	do {
	    ss += s[1].isNull() ? QString::fromLatin1("") : s[1];
	} while ( /* ss.length() > 0 && ss.length() < 100 */ FALSE );
//	for ( int i = 0; i < 10; i++ )
	    pos = rx.search( ss );
	printf( "%d %d", pos, rx.matchedLength() );

#ifndef QT_NO_REGEXP_CAPTURE
	for ( uint i = 1; i < rx.capturedTexts().count(); i++ )
	    printf( " '%s'", rx.capturedTexts()[i].latin1() );
#endif
 	printf( "\n" );
	fprintf( stderr, "%s", s[2].latin1() );
	printf( "\n" );
	fprintf( stderr, "\n" );
    }
    return 0;
}
