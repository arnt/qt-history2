#include <qapplication.h>
#include "setupimpl.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );

    Setup s;
    s.show();

    return a.exec();
}
