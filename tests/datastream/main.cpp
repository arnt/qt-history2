#include <qdatastream.h>
#include <qbitarray.h>
#include <qwidget.h>
#include <qdatetime.h>
#include <qptrlist.h>
#include <qptrvector.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qpen.h>
#include <qpointarray.h>
#include <qwmatrix.h>
#include <qapplication.h>
#include <qbuffer.h>
#include <qbrush.h>
#include <qregion.h>
#include <qfile.h>

static const char* open_xpm[]={
"16 13 6 1",
". c None",
"b c #ffff00",
"d c #000000",
"* c #999999",
"c c #cccccc",
"a c #ffffff",
"...*****........",
"..*aaaaa*.......",
".*abcbcba******.",
".*acbcbcaaaaaa*d",
".*abcbcbcbcbcb*d",
"*************b*d",
"*aaaaaaaaaa**c*d",
"*abcbcbcbcbbd**d",
".*abcbcbcbcbcd*d",
".*acbcbcbcbcbd*d",
"..*acbcbcbcbb*dd",
"..*************d",
"...ddddddddddddd"};


#if QT_VERSION < 200
#define qDebug debug
#define qWarning warning
#endif

class Streamer: public QWidget
{
public:
    bool writeOut( QIODevice* dev, int ver, bool printable );
    bool readIn( QIODevice* dev, int ver, bool printable );

};

/*
void dumpColor( const QColor& c )
{
    qDebug( "   R:%0x G:%0x B:%0x", c.red(), c.green(), c.blue() );
}


void dumpColorGroup( const QColorGroup& g )
{
    dumpColor( g.foreground() );
    dumpColor( g.background() );
    dumpColor( g.light() );
    dumpColor( g.dark() );
    dumpColor( g.mid() );
    dumpColor( g.dark() );
    dumpColor( g.base() );
#if QT_VERSION >= 200
    dumpColor( g.midlight() );
    dumpColor( g.brightText() );
    dumpColor( g.buttonText() );
    dumpColor( g.shadow() );
    dumpColor( g.highlight() );
    dumpColor( g.highlightedText() );
#endif
}

void dumpPalette( const QPalette& p )
{
    qDebug( "Palette dump:" );
    //dumpColorGroup( p.normal() );
    //dumpColorGroup( p.active() );
    dumpColorGroup( p.disabled() );
}
*/

bool Streamer::writeOut( QIODevice* dev, int ver, bool printable  )
{
    QPixmap* pm = new QPixmap(open_xpm);
    Q_ASSERT( !pm->isNull() );

    QDataStream s( dev );
    s.setPrintableData( printable );

#if QT_VERSION >= 200
    s.setVersion( ver );
#else
    ver = ver;
#endif
	
    QBitArray d1( 3 );
    d1.fill( 1 );
    d1[1] = 0;
    s << d1;

    QBrush d2( red, CrossPattern );
    s << d2;

    QColor d3( 50, 100, 150 );
    s << d3;

#if QT_VERSION >= 200
    QCString d4( "Faen her koder man" );
#else
    QString d4( "Faen her koder man" );
#endif
    s << d4;

    QCursor d5( waitCursor );
    s << d5;

    QDate d6( 1976, 4, 5 );
    s << d6;

    QTime d7( 11, 45, 30, 791 );
    s << d7;

    QDateTime d8( d6, d7 );
    s << d8;

    QFont d9( "Courier", 18, QFont::Bold, TRUE ); //# what about charset?
    s << d9;

    QImage d12( open_xpm );
    //debug( "Orig alpha: %i", (int)d12.hasAlphaBuffer() );
    s << d12;

    QPalette d13( green );
    s << d13;

    QColorGroup d14( d13.normal() );
    s << d14;

    QPixmap d16( *pm );
    s << d16;

    QPoint d17( -109, 110 );
    s << d17;

    QRect d18( 99, -100, 101, 102 );
    s << d18;

    QPointArray d19( d18, TRUE );
    s << d19;


    QRegion d20( d18 );
    s << d20;

    QPen d15( blue, 3, DashDotLine );
#if QT_VERSION >= 210
    d15.setCapStyle( Qt::RoundCap );
    d15.setJoinStyle( Qt::RoundJoin );
#endif
    s << d15;

    QSize d21( 567, -568 );
    s << d21;

    QString d22( "Faen her spyr man" );
    s << d22;

    // QStringList: Qt 2.0 specific

    QWMatrix d23( 1.2, 2.3, 3.4, 4.5, 5.6, 6.7 );
    s << d23;

    QString d24; // null
    s << d24;
    QString d25=""; // empty
    s << d25;
    QString d26="nonempty";
    s << d26;

    return TRUE;
}



bool Streamer::readIn( QIODevice* dev, int ver, bool printable  )
{
    QPixmap* pm = new QPixmap(open_xpm);
    Q_ASSERT( !pm->isNull() );

    QDataStream s( dev );
    s.setPrintableData( printable );
#if QT_VERSION >= 200
    s.setVersion( ver );
#else
    ver = ver;
#endif

    QBitArray d1;
    s >> d1;
    Q_ASSERT( d1[0] == 1 );
    Q_ASSERT( d1[1] == 0 );
    Q_ASSERT( d1[2] == 1 );

    QBrush d2( red, CrossPattern );
    s >> d2;
    Q_ASSERT( d2 == QBrush( red, CrossPattern ) );

    QColor d3;
    s >> d3;
    Q_ASSERT( d3 == QColor ( 50, 100, 150 ) );

#if QT_VERSION >= 200
    QCString d4;
#else
    QString d4;
#endif
    s >> d4;
    Q_ASSERT( d4 == "Faen her koder man" ); 

    QCursor d5;
    s >> d5;
    Q_ASSERT( d5.shape() == QCursor(waitCursor).shape() ); //## lacks operator==

    QDate d6;
    s >> d6;
    Q_ASSERT( d6 == QDate( 1976, 4, 5 ) );

    QTime d7;
    s >> d7;
    Q_ASSERT( d7 == QTime( 11, 45, 30, 791 ) );

    QDateTime d8;
    s >> d8;
    Q_ASSERT( d8 == QDateTime( d6, d7 ) );

    QFont d9;
    s >> d9;
    Q_ASSERT( d9 == QFont( "Courier", 18, QFont::Bold, TRUE ) );

    QImage d12;
    s >> d12;
    Q_ASSERT( d12.size() == QImage( open_xpm ).size() );
    QImage ref( open_xpm );
    /*
    qDebug( "Alpha: %i %i", (int)d12.hasAlphaBuffer(), ref.hasAlphaBuffer() );
    qDebug( "Feil %i %i: %x != %x", 3, 0, d12.pixel( 3, 0 ), ref.pixel( 3, 0 ) );
    
     ################ Bug : ref and orig has ff in alpha; readback has 0
     ### (Was like this in 1.44 as well )

    for( int i = 0; i < d12.height(); i++ )
	for( int j = 0; j < d12.width(); j++ )
	    if ( d12.pixel( j, i ) != ref.pixel( j, i ) )
		qDebug( "Feil %i %i", j, i );

    */
    //Q_ASSERT( d12 == QImage( open_xpm ) );

    QPalette d13;
    s >> d13;
    Q_ASSERT( d13 == QPalette( green ) );

    QColorGroup d14;
    s >> d14;
    Q_ASSERT( d14 == QColorGroup( d13.normal() ) );


    QPixmap d16;
    s >> d16;
    Q_ASSERT( d16.size() == QPixmap( *pm ).size() ); //## lacks operator==


    QPoint d17;
    s >> d17;
    Q_ASSERT( d17 == QPoint( -109, 110 ) );


    QRect d18;
    s >> d18;
    Q_ASSERT( d18 == QRect( 99, -100, 101, 102 ) );


    QPointArray d19;
    s >> d19;
    Q_ASSERT( d19 == QPointArray( d18, TRUE ) );


    QRegion d20;
    s >> d20;
    Q_ASSERT( d20 == QRegion( d19 ) );

    QPen origPen( blue, 3, DashDotLine );
#if QT_VERSION >= 210
    if ( ver > 2 ) {
	origPen.setCapStyle( Qt::RoundCap );
	origPen.setJoinStyle( Qt::RoundJoin );
    }
#endif
    QPen d15;
    s >> d15;
    Q_ASSERT( d15 == origPen );

    QSize d21;
    s >> d21;
    Q_ASSERT( d21 == QSize( 567, -568 ) );

    QString d22;
    s >> d22;
    Q_ASSERT( d22 == QString( "Faen her spyr man" ) );

    // QStringList: Qt 2.0 specific

    QWMatrix d23;
    s >> d23;
    //    Q_ASSERT( d23 == QWMatrix( 1.2, 2.3, 3.4, 4.5, 5.6, 6.7 ) );
    QWMatrix m( 1.2, 2.3, 3.4, 4.5, 5.6, 6.7 );
    // Because of double vs. float rounding differences:
    Q_ASSERT( QABS(d23.m11() - m.m11()) < 1e-6 );
    Q_ASSERT( QABS(d23.m12() - m.m12()) < 1e-6 );
    Q_ASSERT( QABS(d23.m21() - m.m21()) < 1e-6 );
    Q_ASSERT( QABS(d23.m22() - m.m22()) < 1e-6 );
    Q_ASSERT( QABS(d23.dx() - m.dx()) < 1e-6 );
    Q_ASSERT( QABS(d23.dy() - m.dy()) < 1e-6 );

    QString d24;
    s >> d24;
    Q_ASSERT(d24.isNull());

    QString d25;
    s >> d25;
    Q_ASSERT(d25.isEmpty());

    QString d26;
    s >> d26;
    Q_ASSERT(d26=="nonempty");

    return TRUE;
}


//Usage: datastream [ -v1 | -v2 | -v3 ] [-internal] [filename]
// If filename given, it is read, otherwise output is written to default file
// If -internal is given, write and read to/from memory is done instead.

int main( int argc, char **argv )
{
    qDebug( "Starting..." );
    QApplication a(argc,argv);

    Streamer s;

    QDataStream vs;
    int ver = vs.version();
    int off = 0;
    if ( argc > 1 ) {
	QString arg1( argv[1] );
	if ( arg1 == "-v1" ) {
	    ver = 1;
	    off++;
	}
	else if ( arg1 == "-v2" ) {
	    ver = 2;
	    off++;
	}
	else if ( arg1 == "-v3" ) {
	    ver = 3;
	    off++;
	}
    }

    bool printable = FALSE;

    if ( argc > 1 ) {
	QString arg2( argv[1+off] );
	if ( arg2 == "-p" ) {
	    printable = TRUE;
	    qDebug( "%s: Using QDatastream printable format.", argv[0] );
	    off++;
	}
    }

#if QT_VERSION >= 200
    qDebug( "Using QDatastream format %i.", ver );
#endif

    if ( argc > 1 ) {
	QString arg2( argv[1+off] );
	if ( arg2 == "-internal" ) {
	    QByteArray ba(10000);
	    QBuffer b( ba );
	    b.open( IO_WriteOnly );
	    s.writeOut( &b, ver, printable );
	    b.close();
	    b.open( IO_ReadOnly );
	    s.readIn( &b, ver, printable );
	    b.close();
	    qDebug( "%s: Internal write and read done.", argv[0] );
	    return 0;
	}
    }



    if ( argc-off < 2 ) {
	// Write file
	QString fileName = "qdatastream-v";
#if QT_VERSION >= 200
	fileName += QString::number( ver );
	fileName += "-";
#endif
	fileName += QT_VERSION_STR;
	fileName += ".out";

	QFile f( fileName );
	if ( !f.open( IO_WriteOnly ) ) {
	    qWarning( "%s: Could not open %s for writing", argv[0], 
		     (const char*)fileName );
	    return 0;
	}
	 
	s.writeOut( &f, ver, printable );
	f.close();
	
	qDebug( "%s: Output written to %s.", argv[0], (const char*)fileName );
    }
    else {
	QString fileName( argv[1+off] );
	QFile f( fileName );
	if ( !f.open( IO_ReadOnly ) ) {
	    qWarning( "%s: Could not open %s for reading", argv[0], 
		     (const char*)fileName );
	    return 0;
	}
	 
	s.readIn( &f, ver, printable );
	f.close();
	qDebug( "%s: %s read.", argv[0], (const char*)fileName );
    }

    return 0;
}
