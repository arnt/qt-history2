#include <qprinter.h>
#include <qapplication.h>
#include <qpainter.h>

#include <qpixmap.h>
#include <qimage.h>

const char* oriNames[] = { "Portrait", "Landscape" };

const char* pgSzNames[] = { "A4", "B5", "Letter", "Legal", "Executive",
  "A0", "A1", "A2", "A3", "A5", "A6", "A7", "A8", "A9", "B0", "B1",
  "B10", "B2", "B3", "B4", "B6", "B7", "B8", "B9", "C5E", "Comm10E",
  "DLE", "Folio", "Ledger", "Tabloid", "NPageSize" };

const char* pgOrNames[] = { "FirstPageFirst", "LastPageFirst" };

const char* coMdNames[] = { "GrayScale", "Color" };


void dump( const QPrinter& p )
{
    qDebug( "\n---------------- Dump: ------------------------" );
    qDebug( "printerName: '%s'", p.printerName().latin1() );
    qDebug( "outputToFile: %i", p.outputToFile() );
    qDebug( "outputFileName: '%s'", p.outputFileName().latin1() );
    qDebug( "printProgram: '%s'", p.printProgram().latin1() );
    qDebug( "printerSelectionOption: '%s'", p.printerSelectionOption().latin1() );
    qDebug( "docName: '%s'", p.docName().latin1() );
    qDebug( "creator: '%s'", p.creator().latin1() );
    qDebug( "orientation: %s", oriNames[p.orientation()] );
    qDebug( "pageSize: %s", pgSzNames[p.pageSize()] );
    qDebug( "pageOrder: %s", pgOrNames[p.pageOrder()] );
    qDebug( "colorMode: %s", coMdNames[p.colorMode()] );
    qDebug( "fullPage: %i", p.fullPage() );
    QSize m = p.margins();
    qDebug( "margins: %i, %i", m.width(), m.height() );
    qDebug( "from/toPage: %i - %i", p.fromPage(), p.toPage() );
    qDebug( "min/maxPge: %i - %i", p.minPage(), p.maxPage() );
    qDebug( "numCopies: %i\n", p.numCopies() );
}

int main( int argc, char **argv )
{
    bool prtest = FALSE;
    if ( argc > 1 )
	prtest = TRUE;

    QApplication a( argc, argv );

    QImage  img("ttlogo.bmp");
    QPixmap pix("ttlogo.bmp");

    QPrinter prt;
    dump( prt );
    
    bool setupOk = prt.setup( 0 );
    qDebug( "Setup: returned %i", setupOk );
    dump( prt );

    if ( setupOk && prtest )
    {
        QPainter p;
        p.begin( &prt );
	p.setPen( Qt::red );
	p.save();
	p.drawRect( 10, 10, 100, 100 );
	p.drawPixmap( 10, 10, pix );
	p.drawImage( 10, 300, img );
	img = img.smoothScale( 112, 65 );
	p.drawImage( 400,  10, img );
	p.drawImage( 400,  80, img );
	p.drawImage( 400, 150, img );
	p.restore();
        p.end();
    }
    return 0;
}
