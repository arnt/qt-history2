#include <qapplication.h>
#include "mainwindow.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    MainWindow * mw = new MainWindow;
    mw->show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    return a.exec();
}
