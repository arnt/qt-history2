#include <qapp.h>

#include "mdlg.h"
int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    MDialog m;

    a.setMainWidget( &m );
    m.resize( 300, 250 );
    m.show();
    a.exec();
}
