#include <qdatetime.h>
#include <qmainwindow.h>
#include <qstatusbar.h>
#include <qmenubar.h>
#include <qapplication.h>
#include <qkeycode.h>
#include <qpainter.h>

#include "canvas.h"

#include <stdlib.h>

FigureEditor::FigureEditor(
	QCanvas& c, QWidget* parent,
	const char* name, WFlags f) :
    QCanvasView(&c,parent,name,f)
{
}

void FigureEditor::contentsMousePressEvent(QMouseEvent* e)
{
    for (QCanvasIterator i = canvas()->at(e->pos()); i; ++i) {
	if ( i.exact() ) {
	    moving = *i;
	    moving_start = e->pos();
	    return;
	}
    }
    moving = 0;
}

void FigureEditor::contentsMouseMoveEvent(QMouseEvent* e)
{
    if ( moving ) {
	moving->moveBy(e->pos().x() - moving_start.x(),
		       e->pos().y() - moving_start.y());
	moving_start = e->pos();

	// You can call for extra updates whenevever you want.
	canvas()->update();
    }
}



SpaceShip::SpaceShip()
{
    static QCanvasPixmapSequence spaceship("sprites/spaceship%02d.png");
    setSequence(&spaceship);
    const double speed = 4.0;
    double d = drand48();
    setVelocity(d*speed*2-speed, (1-d)*speed*2-speed);
}

void SpaceShip::forward()
{
    double vx = xVelocity();
    double vy = yVelocity();
    double nx = x() + vx;
    double ny = y() + vy;
    if ( nx < 0 || nx >= canvas()->width() )
	vx = -vy;
    if ( ny < 0 || ny >= canvas()->height() )
	vy = -vy;
    setVelocity(vx,vy);
    QCanvasItem::forward();
}


Main::Main() :
    canvas(1000,1000)
{
    editor = new FigureEditor(canvas,this);
    QMenuBar* menu = menuBar();

    QPopupMenu* file = new QPopupMenu;
    file->insertItem("Quit", qApp, SLOT(quit()), CTRL+Key_Q);
    menu->insertItem("&File", file);

    QPopupMenu* edit = new QPopupMenu;
    edit->insertItem("Add &Circle", this, SLOT(addCircle()), CTRL+Key_C);
    edit->insertItem("Add &Hexagon", this, SLOT(addHexagon()), CTRL+Key_H);
    edit->insertItem("Add &Rectangle", this, SLOT(addRectangle()), CTRL+Key_R);
    edit->insertItem("Add &Sprite", this, SLOT(addSprite()), CTRL+Key_S);
    menu->insertItem("&Edit", edit);

    QPopupMenu* options = new QPopupMenu;
    menu->insertItem("&Options",options);

    statusBar();

    setCentralWidget(editor);

    canvas.setAdvancePeriod(30);

    for (int test=0; test<1000; test++) {
	addCircle();
	addHexagon();
	addRectangle();
    }
}

void Main::addSprite()
{
    QCanvasItem* i = new SpaceShip;
    i->move(lrand48()%canvas.width(),lrand48()%canvas.height());
}

void Main::addCircle()
{
    QCanvasPolygonalItem* i = new QCanvasEllipse(50,50);
    i->setBrush( QColor(lrand48()%32*8,lrand48()%32*8,lrand48()%32*8) );
    i->move(lrand48()%canvas.width(),lrand48()%canvas.height());
}

void Main::addHexagon()
{
    QCanvasPolygon* i = new QCanvasPolygon;
    QPointArray pa(6);
    pa[0] = QPoint(20,0);
    pa[1] = QPoint(10,-17);
    pa[2] = QPoint(-10,-17);
    pa[3] = QPoint(-20,0);
    pa[4] = QPoint(-10,17);
    pa[5] = QPoint(10,17);
    i->setPoints(pa);
    i->setBrush( QColor(lrand48()%32*8,lrand48()%32*8,lrand48()%32*8) );
    i->move(lrand48()%canvas.width(),lrand48()%canvas.height());
}

void Main::addRectangle()
{
    QCanvasPolygonalItem *i = new QCanvasRectangle( lrand48()%canvas.width(),lrand48()%canvas.height(),
			    50,50);
    int z = lrand48()%256;
    //i->setBrush( QColor(lrand48()%32*8,z,lrand48()%32*8) );
    i->setBrush( QColor(z,z,z) );
    i->setZ(z);
}

int main(int argc, char** argv)
{
    QApplication app(argc,argv);

    Main m;
    m.resize(800,600);
    app.setMainWidget(&m);
    m.show();

    return app.exec();
}


