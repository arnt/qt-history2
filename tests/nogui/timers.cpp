#include "timers.h"
#include <qpainter.h>
#include <qapplication.h>
#include <qtimer.h>




Obj::Obj(QObject* parent, const char* name) :
    QObject(parent, name )
{
    num = 0;

    //    connect( &timer, SIGNAL( timeout() ), this, SIGNAL(tick()) );
    connect( &timer, SIGNAL( timeout() ), this, SLOT( bang()) );
    timer.start( 0 );

}

void Obj::bang()
{
    qDebug( "OBJ %d timers so far", ++num );
    emit tick();
    if ( num > 1024 ) {
	timer.stop();
	emit stopped();
    }
}



Main::Main(QWidget* parent, const char* name, int f) :
    QWidget(parent, name, f)
{
    fucks = 0;
}

void Main::bang()
{
    qDebug( "%d timers so far", ++fucks );
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
    bool useGui = argc % 2;

    QApplication app(argc, argv, useGui );

    Obj obj( 0, "timerobj" );
    
    if ( useGui ) {
	qDebug( "Using GUI." );
    
    
	QApplication::setFont( QFont("Helvetica") );

	Main *m = new Main( 0, "timerwidget");
	m->show();

	QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));
	QObject::connect(&obj, SIGNAL(tick()), m, SLOT(bang()));

    } else {
	qDebug( "No GUI!!!" );
	QObject::connect(&obj, SIGNAL(stopped()), qApp, SLOT(quit()));
    }
    
    return app.exec();
}
