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
#include <qpainter.h>
#include <qpicture.h>

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
    Streamer();
    void paintPrimitives( QPainter& p );
    void paintAll( QPaintDevice* pd );

    bool writeOut( const QString& fileName, int ver=0 );
    bool readIn( const QString& fileName );

    void setDirectMode( bool on );
protected:
    void paintEvent( QPaintEvent * );
    void keyPressEvent( QKeyEvent * );

    QPicture dispPic;
    bool direct;
};


Streamer::Streamer()
    : QWidget()
{
    direct = FALSE;
}


void Streamer::setDirectMode( bool on )
{
    direct = on;
}

void Streamer::paintPrimitives( QPainter& p )
{
    p.drawPoint( 50, 50 );
    
    QPointArray a0( QRect( 120, 20, 70, 70 ) );
    p.drawPoints( a0 );

    p.moveTo( 320, 20 );
    p.lineTo( 380, 80 );

    p.drawLine( 480, 20, 420, 80 );

    p.drawRect( QRect( 520, 20, 70, 70 ) );

    // ########### Not well supported by QPicture 
    //p.drawWinFocusRect( QRect( 620, 20, 70, 70 ) );

    p.drawRoundRect( QRect( 720, 20, 70, 70 ), 50, 50 );

    p.drawEllipse( QRect( 820, 20, 70, 70 ) );

    p.fillRect( 920, 20, 70, 70, QBrush( red ) );
    p.eraseRect( 950, 50, 20, 20 );

    p.drawArc( 20, 120, 70, 70, 100*16, 160*16 );

    p.drawChord( 120, 120, 70, 70, 100*16, 160*16 );

    const QCOORD c1[] = { 220, 120, 280, 180, 280, 120, 220, 180 };
    QPointArray a1( 4, c1 );
    p.drawLineSegments( a1 );

    const QCOORD c2[] = { 320, 120, 380, 180, 380, 120, 320, 180 };
    QPointArray a2( 4, c2 );
    p.drawPolyline( a2 );

    const QCOORD c3[] = { 450, 120, 420, 180, 480, 180, 480, 150 };
    QPointArray a3( 4, c3 );
    p.drawPolygon( a3 );

    const QCOORD c4[] = { 520, 120, 530, 180, 570, 120, 580, 180 };
    QPointArray a4( 4, c4 );
    p.drawCubicBezier( a4 );
    
    QPixmap pm( open_xpm );
    p.drawPixmap( 650 - pm.width()/2, 150-pm.height()/2, pm );

    QImage im( open_xpm );
    p.drawImage( 750 - im.width()/2, 150-im.height()/2, im );

    p.drawTiledPixmap( 820, 120, 70, 70, pm );

    p.drawText( 920, 120, 70, 70, AlignCenter | WordBreak, "Trolls R Qt" );
}

void Streamer::paintAll( QPaintDevice* pd )
{
    QPainter p;

    p.begin( pd );

    paintPrimitives( p );

    p.translate( 0, 200 );
    p.setFont( QFont("Courier", 18, QFont::Bold, TRUE) );
    QPen pen( blue, 3 );
#if QT_VERSION >= 210
    pen.setCapStyle( Qt::RoundCap );
    pen.setJoinStyle( Qt::RoundJoin );
#endif
    p.setPen( pen );
    p.setBrush( QBrush( green, CrossPattern ) );
    p.setBackgroundMode( OpaqueMode );
    p.setBackgroundColor( yellow );
    paintPrimitives( p );

    /*
    p.translate( 0, 200 );
    p.rotate( 5 );
    p.scale( 1, 1.2 );
    paintPrimitives( p );
    */

    p.end();

}

void Streamer::paintEvent( QPaintEvent * )
{
    if ( direct ) {
	paintAll( this );
    }
    else {
	QPainter p( this );
	if ( !dispPic.isNull() )
	    p.drawPicture( dispPic );
    }
}


void Streamer::keyPressEvent( QKeyEvent * )
{
    close();
}

bool Streamer::writeOut( const QString& fileName, int ver )
{
#if QT_VERSION >= 200
    QPicture pic( ver );
#else
    QPicture pic;
    ver = ver;
#endif
    paintAll( &pic );
  
    dispPic.setData( pic.data(), pic.size() );	// operator= is private in 1.x
    return pic.save( fileName );
}



bool Streamer::readIn( const QString& fileName )
{
    QPicture pic;
    if ( pic.load( fileName ) ) {
	dispPic.setData( pic.data(), pic.size() ); // operator= private in 1.x
	return TRUE;
    }
    return FALSE;
}


//Usage: datastream [ -v1 | -v2 ] [-internal] [filename]
// If filename given, it is read, otherwise output is written to default file
// If -internal is given, write and read to/from memory is done instead.

int main( int argc, char **argv )
{
    debug( "Starting..." );
    QApplication a(argc,argv);

    Streamer s;

    int off = 0;

    QDataStream vs;
    int ver = vs.version();
    if ( argc > 1 ) {
	QString arg1( argv[1] );
	if ( arg1 == "-v1" ) {
	    ver = 1;
	    off = 1;
	}
	else if ( arg1 == "-v2" ) {
	    ver = 2;
	    off = 1;
	}
	else if ( arg1 == "-v3" ) {
	    ver = 3;
	    off = 1;
	}
    }

#if QT_VERSION >= 200
    debug( "Using QPicture major format %i.", ver );
#endif


    bool dir = FALSE;

    if ( argc > 1 ) {
	QString arg2( argv[1+off] );
	if ( arg2 == "-direct" )
	    dir = TRUE;
    }


    if ( !dir ) {

	if ( argc-off < 2 ) {
	    // Write file
	    QString fileName = "qpicture-v";
#if QT_VERSION >= 200
	    fileName += QString::number( ver );
	    fileName += "-";
#endif
	    fileName += QT_VERSION_STR;
	    fileName += ".pic";

	    QFile f( fileName );
	    if ( !f.open( IO_WriteOnly ) ) {
		return 0;
	    }
	 
	    bool res = s.writeOut( fileName, ver );
	    if ( res )
		debug( "%s: Output written to %s.", argv[0], 
		       (const char*)fileName );
	    else
		warning( "%s: Could not open %s for writing", argv[0], 
			 (const char*)fileName );
	}
	else {
	    QString fileName( argv[1+off] );

	    bool res = s.readIn( fileName );
	    if ( res )
		debug( "%s: %s read.", argv[0], 
		       (const char*)fileName );
	    else
		warning( "%s: Could not open %s for reading", argv[0], 
			 (const char*)fileName );
	}
    }
    else {
	s.setDirectMode( TRUE );
    }

    a.setMainWidget( &s );
    s.setMinimumWidth( 1000 );
    s.show();
    return a.exec();
}
