#include <qprinter.h>
#include <qapp.h>
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
	p.setPen( red );
	p.save();
	p.drawRect( 10, 10, 100, 100 );
#if 0	
	p.drawPixmap( 10, 10, pix );
	p.rotate( -90 );
	p.drawImage( 300, 10, img );
#endif	
	p.restore();
        p.end();
    }
    return 0;
}

