#include <qapplication.h>
#include "modal.h"

int main( int argc, char* argv[]  )
{
    QApplication a( argc, argv);
    QLabel l( "Main Application", 0);
    a.setMainWidget( &l );
    l.show();
    ( new Dialog( &l ) )->exec();
    return a.exec();
}
