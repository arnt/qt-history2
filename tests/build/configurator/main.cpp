#include <qapplication.h>

#include "configure.h"

int main( int argc, char** argv )
{
    QApplication app( argc, argv );
    QObject::connect( &app, SIGNAL( lastWindowClosed() ), &app, SLOT( quit() ) );

    ConfigureQtDialogImpl* w = new ConfigureQtDialogImpl();

    w->show();
    app.setMainWidget( w );

    app.exec();

    return 0;
}
