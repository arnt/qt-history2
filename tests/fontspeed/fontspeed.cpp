#include <qapplication.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qdatetime.h>
#include <stdio.h>
#include <stdlib.h>

//#define TEST_COLORS

const int MAX_ITER  = 100000;
const int MAX_FONTS = 10;
typedef QFont *pQFont;
QFont **fontArray;


void createFontArray()
{
    fontArray = new pQFont[MAX_FONTS];
    for ( int i=0; i<MAX_FONTS; i++ ) {
	QString name  = "none";
	int boldness  = QFont::Normal;
	int pointSize = 10;
	switch ( rand()%4 ) {
	    case 0:
		name = "Times Roman";
		break;
	    case 1:
		name = "Helvetica";
		break;
	    case 2:
		name = "Courier";
		break;
	    case 3:
		name = "System";
		break;
	}
	switch ( rand()%3 ) {
	    case 0:
		boldness = QFont::Light;
		break;
	    case 1:
		boldness = QFont::Normal;
		break;
	    case 2:
		boldness = QFont::Bold;
		break;
	}
	pointSize = (rand()%50)+4;
	fontArray[i] = new QFont( name, pointSize, boldness );
    }
}
 
void deleteFontArray()
{
    for ( int i=0; i<MAX_FONTS; i++ )	
	delete fontArray[i];
    delete [] fontArray;
}


void drawFonts( QPainter *p )
{
    QRect vp = p->viewport();
    for ( int i=0; i<MAX_ITER; i++ ) {
	int x = rand()%vp.width();
	int y = rand()%vp.height();
	//	p->setFont( *fontArray[rand()%MAX_FONTS] );
	p->drawText( x, y, 100, 40, AlignRight, "text to draw" );
    }
}


int main( int argc, char **argv )
{
    QApplication a(argc,argv);
    createFontArray();
    QPixmap pm(256,256);
    pm.fill( white );
    QPainter p;
    p.begin( &pm );
    QTime t; t.start();
    drawFonts( &p );
    debug( "TIME = %d", t.elapsed() );
    p.end();
    deleteFontArray();
    return 0;
}
