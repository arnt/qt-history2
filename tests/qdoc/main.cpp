#include <qapplication.h>
#include <qstylesheet.h>
#include "helpmainwindow.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    
    // Many HTML files omit the </p> or </li>, so we add this for efficiency:
    QStyleSheet::defaultSheet()->item("p")->setSelfNesting( FALSE );
    QStyleSheet::defaultSheet()->item("li")->setSelfNesting( FALSE );
    
    HelpMainWindow mw;
    a.setMainWidget( &mw );
    mw.resize( QSize( 800, 600 ).boundedTo( a.desktop()->size() ) );
    mw.show();

    return a.exec();
}

