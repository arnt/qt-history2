#include "widgetspeed.h"
#include <qpainter.h>
#include <qapplication.h>

#include <qlayout.h>
#include <qpushbutton.h>

Main::Main(int d, QWidget* parent, const char* name, int f) :
    QWidget(parent, name, f)
{
    QGridLayout *l = new QGridLayout( this, d, d );
    
    for ( int r = 0; r < d; r++ )
	for ( int c = 0; c < d; c++ ) {
	    QPushButton *pb = new QPushButton( "0", this );
	    l->addWidget( pb, r, c );
	}
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

void Main::paintEvent(QPaintEvent* e)
{
}

main(int argc, char** argv)
{
    QApplication::setColorSpec( QApplication::ManyColor );
    QApplication app(argc, argv);
    QApplication::setFont( QFont("Helvetica") );

    int d = 10;
    if ( argc > 1 ) {
	QString s = argv[1];
	d = s.toInt();
    }
    
    Main m(d);
    m.show();

    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

    return app.exec();
}
