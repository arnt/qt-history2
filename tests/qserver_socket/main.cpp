#include <qapplication.h>

#include "some.h"

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    Some some;
    QObject::connect( &some, SIGNAL(quitted()),
	    &a, SLOT(quit()) );

    a.setMainWidget( &some );
    some.show();
    return a.exec();
}
