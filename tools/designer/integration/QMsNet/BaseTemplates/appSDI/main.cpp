#include <qapplication.h>
#include "sdiwindow.h"

int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    SDIWindow w;
    w.show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    return a.exec();
}
