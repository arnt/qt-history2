#include "timers.h"
#include <qpainter.h>
#include <qapplication.h>
#include <qtimer.h>
#include <qlcdnumber.h>



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
    emit tick(num);
    if ( num > 1024 ) {
	timer.stop();
	emit stopped();
    }
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

	QLCDNumber *lcd = new QLCDNumber( 0, "lcdwidget");
	lcd->show();

	QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));
	QObject::connect(&obj, SIGNAL(tick(int)), lcd, SLOT(display(int)));

    } else {
	qDebug( "No GUI!!!" );
	QObject::connect(&obj, SIGNAL(stopped()), qApp, SLOT(quit()));
    }

    return app.exec();
}
