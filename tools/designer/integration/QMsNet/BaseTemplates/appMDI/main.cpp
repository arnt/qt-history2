#include <qapplication.h>
#include "mdiwindow.h"

int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    MDIWindow w;
    w.show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    return a.exec();
}
