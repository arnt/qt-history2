#include <qapplication.h>
#include "metric.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    dialog *w = new dialog;
    w->show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    return a.exec();
}
