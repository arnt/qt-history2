#include "recreate.h"
#include <qpushbutton.h>
#include <qpainter.h>
#include <qapplication.h>

Main::Main(QWidget* parent, const char* name, int f) :
    QLabel("Press key to recreate", parent, name, f)
{
    f = new QFrame( this );
    f->setFrameStyle( QFrame::Raised  + QFrame::Box );
}

void Main::bang()
{
}


void Main::pang()
{
    
}


void Main::ting()
{
    
}


void Main::resizeEvent(QResizeEvent* )
{
    f->setGeometry( 10, 10, width()-20, height()-20 );
}

void Main::keyPressEvent(QKeyEvent*)
{
    recreate(0,0,QPoint(0,0),isVisible());
}



main(int argc, char** argv)
{
    QApplication::setColorSpec( QApplication::ManyColor );
    QApplication app(argc, argv);
    QApplication::setFont( QFont("Helvetica") );

    Main m;
    
    QPushButton * p;
    p = new QPushButton( 
    
    m.show();

    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

    return app.exec();
}
