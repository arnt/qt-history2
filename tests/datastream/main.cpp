


#include <qdatastream.h>
#include <qbitarray.h>
#include <qwidget.h>
#include <qdatetime.h>
#include <qlist.h>
#include <qvector.h>
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


class Streamer: public QWidget
{
public:
    bool writeOut( QIODevice* dev );
    bool readIn( QIODevice* dev );

};


bool Streamer::writeOut( QIODevice* dev )
{
    QPixmap* pm = new QPixmap(open_xpm);
    ASSERT( !pm->isNull() );

    QDataStream s( dev );
    
    QBitArray d1( 3 );
    d1.fill( TRUE );
    d1[1] = FALSE;
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

    QPen d15( blue, 3, DashDotLine );
    s << d15;

    QPixmap d16( *pm );
    s << d16;

    QPoint d17( -109, 110 );
    s << d17;

    QRect d18( 99, -100, 101, 102 );
    s << d18;

    QPointArray d19( d18, TRUE );
    s << d19;

    QRegion d20( d19 );
    s << d20;

    QSize d21( 567, -568 );
    s << d21;

    QString d22( "Faen her spyr man" );
    s << d22;

    // QStringList: Qt 2.0 specific

    QWMatrix d23( 1.2, 2.3, 3.4, 4.5, 5.6, 6.7 );
    s << d23;

    return TRUE;
}



bool Streamer::readIn( QIODevice* dev )
{
    QPixmap* pm = new QPixmap(open_xpm);
    ASSERT( !pm->isNull() );

    QDataStream s( dev );
    s.setVersion( 1 );

    QBitArray d1;
    s >> d1;
    ASSERT( d1[0] == TRUE );
    ASSERT( d1[1] == FALSE );
    ASSERT( d1[2] == TRUE );

    QBrush d2( red, CrossPattern );
    s >> d2;
    ASSERT( d2 == QBrush( red, CrossPattern ) );

    QColor d3;
    s >> d3;
    ASSERT( d3 == QColor ( 50, 100, 150 ) );

#if QT_VERSION >= 200
    QCString d4;
#else
    QString d4;
#endif
    s >> d4;
    ASSERT( d4 == "Faen her koder man" ); 

    QCursor d5;
    s >> d5;
    ASSERT( d5.shape() == QCursor(waitCursor).shape() ); //## lacks operator==

    QDate d6;
    s >> d6;
    ASSERT( d6 == QDate( 1976, 4, 5 ) );

    QTime d7;
    s >> d7;
    ASSERT( d7 == QTime( 11, 45, 30, 791 ) );

    QDateTime d8;
    s >> d8;
    ASSERT( d8 == QDateTime( d6, d7 ) );

    QFont d9;
    s >> d9;
    ASSERT( d9 == QFont( "Courier", 18, QFont::Bold, TRUE ) );

    QImage d12;
    s >> d12;
    ASSERT( d12.size() == QImage( open_xpm ).size() );
    QImage ref( open_xpm );
    /*
    debug( "Alpha: %i %i", (int)d12.hasAlphaBuffer(), ref.hasAlphaBuffer() );
    debug( "Feil %i %i: %x != %x", 3, 0, d12.pixel( 3, 0 ), ref.pixel( 3, 0 ) );
    
     ################ Bug : ref and orig has ff in alpha; readback has 0
     ### (Was like this in 1.44 as well )

    for( int i = 0; i < d12.height(); i++ )
	for( int j = 0; j < d12.width(); j++ )
	    if ( d12.pixel( j, i ) != ref.pixel( j, i ) )
		debug( "Feil %i %i", j, i );

    */
    //ASSERT( d12 == QImage( open_xpm ) );

    QPalette d13;
    s >> d13;
    ASSERT( d13 == QPalette( green ) );

    QColorGroup d14;
    s >> d14;
    ASSERT( d14 == QColorGroup( d13.normal() ) );

    QPen d15;
    s >> d15;
    ASSERT( d15 == QPen( blue, 3, DashDotLine ) );

    QPixmap d16;
    s >> d16;
    ASSERT( d16.size() == QPixmap( *pm ).size() ); //## lacks operator==


    QPoint d17;
    s >> d17;
    ASSERT( d17 == QPoint( -109, 110 ) );

    QRect d18;
    s >> d18;
    ASSERT( d18 == QRect( 99, -100, 101, 102 ) );

    QPointArray d19;
    s >> d19;
    ASSERT( d19 == QPointArray( d18, TRUE ) );

    QRegion d20;
    s >> d20;
    ASSERT( d20 == QRegion( d19 ) );

    QSize d21;
    s >> d21;
    ASSERT( d21 == QSize( 567, -568 ) );

    QString d22;
    s >> d22;
    ASSERT( d22 == QString( "Faen her spyr man" ) );

    // QStringList: Qt 2.0 specific

    QWMatrix d23;
    s >> d23;
    //    ASSERT( d23 == QWMatrix( 1.2, 2.3, 3.4, 4.5, 5.6, 6.7 ) );
    QWMatrix m( 1.2, 2.3, 3.4, 4.5, 5.6, 6.7 );
    // Because of double vs. float rounding differences:
    ASSERT( QABS(d23.m11() - m.m11()) < 1e-6 );
    ASSERT( QABS(d23.m12() - m.m12()) < 1e-6 );
    ASSERT( QABS(d23.m21() - m.m21()) < 1e-6 );
    ASSERT( QABS(d23.m22() - m.m22()) < 1e-6 );
    ASSERT( QABS(d23.dx() - m.dx()) < 1e-6 );
    ASSERT( QABS(d23.dy() - m.dy()) < 1e-6 );

    return TRUE;
}



int main( int argc, char **argv )
{
    debug( "Starting..." );
    QApplication a(argc,argv);

    Streamer s;

    QFile f( "qdatastream-1.44.out" );
    //f.open( IO_WriteOnly );
    //    QByteArray ba( 10000 );
    //QBuffer b( ba );
    //b.open( IO_WriteOnly );
    //s.writeOut( &f );
    //f.close();

    
    f.open( IO_ReadOnly );
    s.readIn( &f );
    f.close();

    return 0;
}
