#include "sizeaware.h"
#include <qapplication.h>

int main( int argc, char ** argv )
{
    QApplication app( argc, argv );
    SizeAware *sizeaware = new SizeAware( 0, 0, true );
    sizeaware->exec();
    sizeaware->destroy();
    return 0; 
}
