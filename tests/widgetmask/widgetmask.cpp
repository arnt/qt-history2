#include "widgetmask.h"
#include <qbitmap.h>
#include <qpainter.h>
#include <qapplication.h>
#include <stdlib.h>

Main::Main(QWidget* parent, const char* name, int f) :
    QWidget(parent, name, WPaintClever)
{
#if USE_REGION_MASKS
    QRegion rgn;
    for (int i=0; i<100; i++) {
	const int outside=0;
	int x=rand()%(width()+2*outside)-outside;
	int y=rand()%(height()+2*outside)-outside;
	int w=rand()%(width()-x+outside);
	int h=rand()%(height()-y+outside);
	rgn = rgn.unite(QRegion(x,y,w,h));
    }
    setMask(rgn);
#endif
}

void Main::bang()
{
}

void Main::resizeEvent(QResizeEvent*)
{
    QBitmap bm(size(),TRUE);
    {
	QPainter p(&bm);
	p.fillRect(rect(),color1);
	p.setPen(color0);
	p.setBrush(color0);
	srand(123);
	for (int i=0; i<20; i++) {
	    int x=rand()%(width());
	    int y=rand()%(height());
	    int w=rand()%(width()-x);
	    int h=rand()%(height()-y);
	    int c=rand();
	    p.drawRoundRect(x,y,w,h,w/4,h/4);
	}
    }
    setMask(bm);
}

void Main::keyPressEvent(QKeyEvent*)
{
    clearMask();
}

void Main::keyReleaseEvent(QKeyEvent*)
{
}

void Main::paintEvent(QPaintEvent* e)
{
    QPainter p(this);
    p.setClipRect(e->rect());
    p.setPen(black);
    srand(123);
    for (int i=0; i<20; i++) {
	int x=rand()%(width());
	int y=rand()%(height());
	int w=rand()%(width()-x);
	int h=rand()%(height()-y);
	int c=rand();
	p.setBrush(QColor(c&0xffffff));
	p.drawRoundRect(x,y,w,h,w/4,h/4);
    }
}

main(int argc, char** argv)
{
    QApplication::setColorSpec( QApplication::ManyColor );
    QApplication app(argc, argv);
    QApplication::setFont( QFont("Helvetica") );

    Main m;
    m.show();

    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

    return app.exec();
}
