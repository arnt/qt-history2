#include <qapplication.h>
#include <qstylesheet.h>
#include "helpmainwindow.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    
    HelpMainWindow mw;
    mw.resize( QSize( 800, 600 ).boundedTo( a.desktop()->size() ) );
    a.setMainWidget( &mw );
    mw.show();

    return a.exec();
}

