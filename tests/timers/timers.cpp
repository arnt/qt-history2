#include "timers.h"
#include <qpainter.h>
#include <qapp.h>
#include <qtimer.h>

Main::Main(QWidget* parent, const char* name, int f) :
    QWidget(parent, name, f)
{
    fucks = 0;
}

void Main::bang()
{
    debug( "%d timers so far", ++fucks );
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
    
    for( int i=0; i < 1024; i++ )
	QTimer::singleShot( 2000, &m, SLOT(bang()) );

    return app.exec();
}
