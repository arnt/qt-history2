#include <qprinter.h>
#include <qapplication.h>
#include <qpainter.h>

#include <qpixmap.h>
#include <qimage.h>

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QImage  img("ttlogo.bmp");
    QPixmap pix("ttlogo.bmp");

    QPrinter prt;
    if ( prt.setup(0) )
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
