#include <qapplication.h>
#include <qfiledialog.h>

#include "playdisplay.h"

//
// main
//
int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    PlayDisplay play;

    a.setMainWidget( &play );
    if ( argc > 1 ) {
	play.show( argv[1] );
    } else {
	play.show( QFileDialog::getOpenFileName(
		    QString::null, "*.xml" ) );
    }
    return a.exec();
}
