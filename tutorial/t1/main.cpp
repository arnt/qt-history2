/****************************************************************
**
** Qt tutorial 1
**
****************************************************************/

#include <qapp.h>
#include <qpushbt.h>


int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QPushButton hello( "Hello world!" );
    hello.resize( 100, 30 );

    a.setMainWidget( &hello );
    hello.show();
    return a.exec();
}
