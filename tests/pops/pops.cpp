#include "pops.h"
#include <qpainter.h>
#include <qapp.h>
#include <qpopmenu.h>
#include <qmenubar.h>

Main::Main(QWidget* parent, const char* name, int f) :
    QWidget(parent, name, f)
{
	mb = new QMenuBar(this);
	startTimer(100);
}

void Main::bang()
{
}

void Main::resizeEvent(QResizeEvent*)
{
}

void Main::keyPressEvent(QKeyEvent*)
{
}

void Main::keyReleaseEvent(QKeyEvent*)
{
}

void Main::timerEvent(QTimerEvent* e)
{
	static QPopupMenu* m=0;
	if (m) {
		delete m;
		m = 0;
	} else {
		m = new QPopupMenu;
		m->insertItem("This is a test");
		m->insertItem("This is a test");
		m->insertItem("This is a test");
		m->insertItem("This is a test");
		m->insertItem("This is a test");
		m->insertItem("This is a test");
		m->insertItem("This is a test");
		m->insertItem("This is a test");
		mb->insertItem("File",m);
	}
}

void Main::paintEvent(QPaintEvent* e)
{
    QPainter p(this);
    p.setClipRect(e->rect());

    // ...
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
