#include <qdatetime.h>
#include <qmainwindow.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qmenubar.h>
#include <qapplication.h>
#include <qkeycode.h>
#include <qpainter.h>
#include <qlabel.h>

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
    QCanvasItemList l=canvas()->collisions(e->pos());
    for (QCanvasItemList::Iterator it=l.begin(); it!=l.end(); ++it) {
	moving = *it;
	moving_start = e->pos();
	return;
    }
    moving = 0;
}

void FigureEditor::contentsMouseMoveEvent(QMouseEvent* e)
{
    if ( moving ) {
	moving->moveBy(e->pos().x() - moving_start.x(),
		       e->pos().y() - moving_start.y());
	moving_start = e->pos();
    }
}



BouncyLogo::BouncyLogo(QCanvas* canvas) :
    QCanvasSprite(canvas)
{
    static QCanvasPixmapArray logo("qt-trans.xpm");
    setSequence(&logo);
    setAnimated(TRUE);
    initPos();
}

const int spaceship_rtti = 1234;

int BouncyLogo::rtti() const
{
    return spaceship_rtti;
}

void BouncyLogo::initPos()
{
    initSpeed();
    int trial=1000;
    do {
	move(lrand48()%canvas()->width(),lrand48()%canvas()->height());
	advance(0);
    } while (trial-- && xVelocity()==0.0 && yVelocity()==0.0);
}

void BouncyLogo::initSpeed()
{
    const double speed = 4.0;
    double d = drand48();
    setVelocity( d*speed*2-speed, (1-d)*speed*2-speed );
}

void BouncyLogo::advance(int stage)
{
    switch ( stage ) {
      case 0: {
	double vx = xVelocity();
	double vy = yVelocity();

	if ( vx == 0.0 && vy == 0.0 ) {
	    // stopped last turn
	    initSpeed();
	    vx = xVelocity();
	    vy = yVelocity();
	}

	double nx = x() + vx;
	double ny = y() + vy;

	if ( nx < 0 || nx >= canvas()->width() )
	    vx = -vy;
	if ( ny < 0 || ny >= canvas()->height() )
	    vy = -vy;

	for (int bounce=0; bounce<4; bounce++) {
	    QCanvasItemList l=collisions(FALSE);
	    for (QCanvasItemList::Iterator it=l.begin(); it!=l.end(); ++it) {
		QCanvasItem *hit = *it;
		if ( hit->rtti()==spaceship_rtti && hit->collidesWith(this) ) {
		    switch ( bounce ) {
		      case 0:
			vx = -vx;
			break;
		      case 1:
			vy = -vy;
			vx = -vx;
			break;
		      case 2:
			vx = -vx;
			break;
		      case 3:
			// Stop for this turn
			vx = 0;
			vy = 0;
			break;
		    }
		    setVelocity(vx,vy);
		    break;
		}
	    }
	}

	if ( x()+vx < 0 || x()+vx >= canvas()->width() )
	    vx = 0;
	if ( y()+vy < 0 || y()+vy >= canvas()->height() )
	    vy = 0;

	setVelocity(vx,vy);
      } break;
      case 1:
	QCanvasItem::advance(stage);
        break;
    }
}


Main::Main(QCanvas& c, QWidget* parent, const char* name, WFlags f) :
    QMainWindow(parent,name,f),
    canvas(c)
{
    editor = new FigureEditor(canvas,this);
    QMenuBar* menu = menuBar();

    QPopupMenu* file = new QPopupMenu;
    file->insertItem("&New view", this, SLOT(newView()), CTRL+Key_N);
    file->insertSeparator();
    file->insertItem("&Quit", qApp, SLOT(quit()), CTRL+Key_Q);
    menu->insertItem("&File", file);

    QPopupMenu* edit = new QPopupMenu;
    edit->insertItem("Add &Circle", this, SLOT(addCircle()), CTRL+Key_C);
    edit->insertItem("Add &Hexagon", this, SLOT(addHexagon()), CTRL+Key_H);
    edit->insertItem("Add &Polygon", this, SLOT(addPolygon()), CTRL+Key_P);
    edit->insertItem("Add &Line", this, SLOT(addLine()), CTRL+Key_L);
    edit->insertItem("Add &Rectangle", this, SLOT(addRectangle()), CTRL+Key_R);
    edit->insertItem("Add &Sprite", this, SLOT(addSprite()), CTRL+Key_S);
    menu->insertItem("&Edit", edit);

    options = new QPopupMenu;
    dbf_id = options->insertItem("Double buffer", this, SLOT(toggleDoubleBuffer()));
    options->setItemChecked(dbf_id, TRUE);
    menu->insertItem("&Options",options);

    QPopupMenu* help = new QPopupMenu;
    help->insertItem("&Help...", this, SLOT(help()));
    help->setItemChecked(dbf_id, TRUE);
    menu->insertItem("&Help",help);

    statusBar();

    setCentralWidget(editor);

    /*
    for (int test=0; test<200; test++) {
	addCircle();
	addHexagon();
	addLine();
	addRectangle();
	if ( test&10 == 0 ) {
	    addPolygon();
	}
	if ( test&30 == 0 ) {
	    addSprite();
	}
    }
    */
}

void Main::newView()
{
    // Open a new view... have it delete when closed.
    Main *m = new Main(canvas, 0, 0, WDestructiveClose);
    qApp->setMainWidget(m);
    m->show();
    qApp->setMainWidget(0);
}

void Main::help()
{
    static QMessageBox* about = new QMessageBox( "Qt Canvas Example",
	    "<h3>The QCanvas classes example</h3>"
	    "<ul>"
		"<li> Press CTRL-S for some sprites."
		"<li> Press CTRL-C for some circles."
		"<li> Press CTRL-L for some lines."
		"<li> Drag the objects around."
		"<li> Read the code!"
	    "</ul>", QMessageBox::Information, 1, 0, 0, this, 0, FALSE );
    about->setButtonText( 1, "Dismiss" );
    about->show();
}

void Main::aboutQt()
{
    QMessageBox::aboutQt( this, "Qt Canvas Example" );
}

void Main::toggleDoubleBuffer()
{
    bool s = !options->isItemChecked(dbf_id);
    options->setItemChecked(dbf_id,s);
    canvas.setDoubleBuffering(s);
}

void Main::addSprite()
{
    QCanvasItem* i = new BouncyLogo(&canvas);
    i->setZ(lrand48()%256);
}

void Main::addCircle()
{
    QCanvasPolygonalItem* i = new QCanvasEllipse(50,50,&canvas);
    i->setBrush( QColor(lrand48()%32*8,lrand48()%32*8,lrand48()%32*8) );
    i->move(lrand48()%canvas.width(),lrand48()%canvas.height());
    i->setZ(lrand48()%256);
}

void Main::addHexagon()
{
    QCanvasPolygon* i = new QCanvasPolygon(&canvas);
    const int size = 40;
    QPointArray pa(6);
    pa[0] = QPoint(2*size,0);
    pa[1] = QPoint(size,-size*173/100);
    pa[2] = QPoint(-size,-size*173/100);
    pa[3] = QPoint(-2*size,0);
    pa[4] = QPoint(-size,size*173/100);
    pa[5] = QPoint(size,size*173/100);
    i->setPoints(pa);
    i->setBrush( QColor(lrand48()%32*8,lrand48()%32*8,lrand48()%32*8) );
    i->move(lrand48()%canvas.width(),lrand48()%canvas.height());
    i->setZ(lrand48()%256);
}

void Main::addPolygon()
{
    QCanvasPolygon* i = new QCanvasPolygon(&canvas);
    const int size = 400;
    QPointArray pa(6);
    pa[0] = QPoint(0,0);
    pa[1] = QPoint(size,size/5);
    pa[2] = QPoint(size*4/5,size);
    pa[3] = QPoint(size/6,size*5/4);
    pa[4] = QPoint(size*3/4,size*3/4);
    pa[5] = QPoint(size*3/4,size/4);
    i->setPoints(pa);
    i->setBrush( QColor(lrand48()%32*8,lrand48()%32*8,lrand48()%32*8) );
    i->move(lrand48()%canvas.width(),lrand48()%canvas.height());
    i->setZ(lrand48()%256);
}

void Main::addLine()
{
    QCanvasLine* i = new QCanvasLine(&canvas);
    i->setPoints( lrand48()%canvas.width(), lrand48()%canvas.height(),
                  lrand48()%canvas.width(), lrand48()%canvas.height() );
    i->setPen( QColor(lrand48()%32*8,lrand48()%32*8,lrand48()%32*8) );
    i->setZ(lrand48()%256);
}

void Main::addRectangle()
{
    QCanvasPolygonalItem *i = new QCanvasRectangle( lrand48()%canvas.width(),lrand48()%canvas.height(),
			    200,200,&canvas);
    int z = lrand48()%256;
    i->setBrush( QColor(z,z,z) );
    i->setZ(z);
}

int main(int argc, char** argv)
{
    QApplication app(argc,argv);

    /*
    qDebug("sizeof(QCanvasPolygonalItem)=%d",sizeof(QCanvasPolygonalItem));
    qDebug("sizeof(QCanvasText)=%d",sizeof(QCanvasText));
    qDebug("sizeof(QWidget)=%d",sizeof(QWidget));
    qDebug("sizeof(QLabel)=%d",sizeof(QLabel));
    */

    QCanvas canvas(800,600);
    canvas.setAdvancePeriod(30);
    Main m(canvas);
    qApp->setMainWidget(&m);
    m.show();
    m.help();
    qApp->setMainWidget(0);

    QObject::connect( qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()) );

    return app.exec();
}


