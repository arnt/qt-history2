#include "qperf.h"
#include <qtextstream.h>
#include <qeucjpcodec.h>
#include <qfile.h>
#include <qmessagebox.h>

QString  ts_latin1_file( "ts_latin1_file.txt" );
QString  ts_unicode_file( "ts_unicode_file.txt" );
QString  ts_eucjp_file( "ts_eucjp_file.txt" );

// choose what character(s) are used to mark the end of a line
#define ENDL endl
//#define ENDL "\r\n"

static void textstream_init()
{
    const int lines = 500;

    {
	// create a sample text-file (Latin1)
	QFile f( ts_latin1_file );
	if ( f.open( IO_WriteOnly ) ) {
	    QTextStream t(&f);
	    t.setEncoding( QTextStream::Latin1 );
	    for (int i=0; i<lines; i++) {
		for (char c='a'; c<='z'; c++)
		    t << c;
		t << ENDL;
	    }
	    f.close();
	}
    }
    {
	// create a sample text-file (Unicode)
	QFile f( ts_unicode_file );
	if ( f.open( IO_WriteOnly ) ) {
	    QTextStream t(&f);
	    t.setEncoding( QTextStream::Unicode );
	    for (int i=0; i<lines; i++) {
		for (char c='a'; c<='z'; c++)
		    t << c;
		t << ENDL;
	    }
	    f.close();
	}
    }
    {
	// create a sample text-file (EucJp)
	QFile f( ts_eucjp_file );
	if ( f.open( IO_WriteOnly ) ) {
	    QTextStream t(&f);
	    t.setCodec( new QEucJpCodec );
	    for (int i=0; i<lines; i++) {
		for (char c='a'; c<='z'; c++)
		    t << c;
		t << ENDL;
	    }
	    f.close();
	}
    }
}

static int textstream_read( const char *fileName,
	const QTextStream::Encoding e, int numIter=10 )
{
    int i;

    for ( i=0; i<numIter; i++ ) {
	QFile f( fileName );
	if ( !f.open( IO_ReadOnly ) )
	    return i; // is this possible?
	QTextStream ts(&f);
	ts.setEncoding( e );
	ts.read();
	f.close();
    }
    return i;
}

static int textstream_read( const char *fileName,
	QTextCodec *tc, int numIter=10 )
{
    int i;

    QFile f( fileName );
    for ( i=0; i<numIter; i++ ) {
	if ( !f.open( IO_ReadOnly ) )
	    return i; // is this possible?
	QTextStream ts(&f);
	ts.setCodec( tc );
	ts.read();
	f.close();
    }
    return i;
}

static int textstream_read_latin1()
{
    return textstream_read( ts_latin1_file, QTextStream::Latin1 );
}

static int textstream_read_unicode()
{
    return textstream_read( ts_unicode_file, QTextStream::Unicode );
}

static int textstream_read_eucjp()
{
    return textstream_read( ts_eucjp_file, new QEucJpCodec );
}

QPERF_BEGIN(textstream,"QTextStream tests")
    QPERF(textstream_read_latin1,"Read from a Latin1-textfile to a string.")
    QPERF(textstream_read_unicode,"Read from a Unicode-textfile to a string.")
    QPERF(textstream_read_eucjp,"Read from a EucJp-textfile to a string.")
QPERF_END(textstream)


