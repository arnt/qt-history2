#include "widgetstack.h"
#include <qlayout.h>
#include <qapp.h>
#include <qpushbt.h>
#include <qwidgetstack.h>

Main::Main(QWidget* parent, const char* name, int f) :
    QWidget(parent, name, f)
{
    QBoxLayout * b = new QBoxLayout( this, QBoxLayout::Down, 6 );
    ws = new QWidgetStack( this );
    ws->setBackgroundColor( yellow );
    ws->setFrameStyle( QFrame::Raised + QFrame::Panel );
    b->addWidget( ws, 1 );

    QWidget * w1 = new QWidget( ws, "42" );
    w1->setBackgroundColor( red );
    ws->addWidget( w1, 42 );

    QWidget * w2 = new QWidget( ws, "69" );
    w2->setBackgroundColor( blue );
    ws->addWidget( w2, 69 );
    
    b->addSpacing( 6 );

    QBoxLayout * v = new QBoxLayout( QBoxLayout::LeftToRight );
    b->addLayout( v );

    QPushButton * pb1 = new QPushButton( "42", this );
    pb1->setFixedSize( pb1->sizeHint() );
    v->addWidget( pb1 );
    v->addSpacing( 6 );
    
    QPushButton * pb2 = new QPushButton( "69", this );
    pb2->setFixedSize( pb2->sizeHint() );
    v->addWidget( pb2 );
    v->addStretch( 1 );

    b->activate();

    connect( pb1, SIGNAL(clicked()),
	     this, SLOT(raise42()) );
    connect( pb2, SIGNAL(clicked()),
	     this, SLOT(raise69()) );
}


void Main::raise42()
{
    ws->raiseWidget( 42 );
}


void Main::raise69()
{
    ws->raiseWidget( 69 );
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
