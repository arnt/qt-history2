#include <qpainter.h>
#include <qprinter.h>
#include <qapplication.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qpaintdevicemetrics.h>
#include <qpicture.h>
#include <qtextcodec.h>

static const struct {
    QFont::CharSet cs;
    uint mib;
} unicodevalues[] = {
    { QFont::KOI8R, 2084 },
    { QFont::ISO_8859_1, 4 },
    { QFont::ISO_8859_2, 5 },
    { QFont::ISO_8859_3, 6 },
    { QFont::ISO_8859_4, 7 },
    { QFont::ISO_8859_5, 8 },
    { QFont::ISO_8859_6, 82 },
    { QFont::ISO_8859_7, 10 },
    { QFont::ISO_8859_8, 85 },
    { QFont::ISO_8859_10, 13 },
    { QFont::ISO_8859_11, 2259 }, // aka tis620
    // makeFixedStrings() below assumes that this is last
    { QFont::ISO_8859_9, 12 }
};

main(int argc, char** argv)
{
    QApplication app(argc, argv);
    QPrinter printer;

    printer.setOutputFileName( "/tmp/pscharsets.ps" );
    printer.setOutputToFile( TRUE );

    printer.setFullPage( TRUE );
    printer.setOrientation( QPrinter::Landscape );
    printer.setColorMode( QPrinter::GrayScale );
    printer.setNumCopies( 1 );

    QPainter p( &printer );
    int i = 0;
    do {
	//    QFont( const QString &family, int pointSize,
	//	   int weight, bool italic, CharSet charSet );
	p.setFont( QFont( "Courier", 10,
			  QFont::Normal, FALSE, unicodevalues[i].cs ) );
	char stuff[129];
	int c = 0;
	while( c < 96 ) {
	    stuff[c] = c+160;
	    c++;
	}
	stuff[c] = '\0';
	QTextCodec * codec = QTextCodec::codecForMib( unicodevalues[i].mib );
	if ( codec )
	    p.drawText( 72, 72+14*i, 
			codec->toUnicode( stuff, 96 ) + 
			QString::fromLatin1( " ---- " ) +
			codec->name() );
    } while( unicodevalues[i++].cs != QFont::ISO_8859_9 );
}

