#include "multi.h"

#include <qapplication.h>
#include <qpushbutton.h>
#include <qlayout.h>

Main::Main( QWidget* parent, const char* name, int f )
    : QWidget( parent, name, f )
{
    QHBoxLayout* lay = new QHBoxLayout( this, 5 );
    QPushButton* b = new QPushButton( "Push Me", this );
    lay->addWidget( b );
    VisualSomething* vs = new VisualSomething( this );
    lay->addWidget( vs );
    connect( b, SIGNAL( clicked()),
	     vs, SLOT( foo() ) );
}


void Something::foo()
{
    qDebug("Something::foo");
}

VisualSomething::VisualSomething( QWidget* parent, const char* name, int f )
    : QWidget( parent, name, f )
{
    setBackgroundColor( red );
}

void VisualSomething::foo()
{
    qDebug("VisualSomething::foo");
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    Main* m = new Main;
    app.setMainWidget(m);
    m->show();

    int result = app.exec();
    delete m;
    return result;
}
