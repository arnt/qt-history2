#include <qapplication.h>
#include <qstylesheet.h>
#include "helpmainwindow.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    
    HelpMainWindow mw;
    a.setMainWidget( &mw );
    mw.resize( QSize( 800, 600 ).boundedTo( a.desktop()->size() ) );
    mw.show();

    return a.exec();
}

