#include <qapplication.h>
#include <qpushbutton.h>
#include "modal.h"

int main( int argc, char* argv[]  )
{
    QApplication a( argc, argv);
    QPushButton l( "Main Application", 0);
    a.setMainWidget( &l );
    l.show();
    ( new Dialog( &l ) )->exec();
    return a.exec();
}
