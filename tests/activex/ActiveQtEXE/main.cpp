#include <qapplication.h>
#include "testwidget.h"

extern "C" QWidget *axmain( QWidget *parent )
{
    return new TestWidget( parent );
}

int main( int argc, char **argv )
{
    QApplication app( argc, argv );

    QWidget *widget = axmain( 0 );
    app.setMainWidget( widget );
    widget->show();

    int res = app.exec();
    delete widget;

    return res;
}
