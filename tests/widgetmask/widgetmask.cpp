#include "widgetmask.h"
#include <qpainter.h>
#include <qapplication.h>
#include <stdlib.h>

Main::Main(QWidget* parent, const char* name, int f) :
    QWidget(parent, name, f)
{
    QRegion rgn;
    for (int i=0; i<100; i++) {
	int x=rand()%width();
	int y=rand()%height();
	int w=rand()%(width()-x);
	int h=rand()%(height()-y);
	rgn = rgn.unite(QRegion(x,y,w,h));
    }
    setMask(rgn);
}

void Main::bang()
{
}

void Main::resizeEvent(QResizeEvent*)
{
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
    p.drawLine(0,0,width()-1,height()-1);
    p.drawLine(width()-1,0,0,height()-1);
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
