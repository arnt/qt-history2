//
// Qt OpenGL example: Shared Box
//
// A small example showing how to use OpenGL display list sharing
// 
// File: main.cpp
//
// The main() function 
// 

#include "globjwin.h"
#include <qapp.h>

/*
  The main program is here. 
*/

int main( int argc, char **argv )
{
    QApplication::setColorSpec( QApplication::CustomColor );
    QApplication a(argc,argv);			

    GLObjectWindow w;
    w.resize( 550, 350 );
    a.setMainWidget( &w );
    w.show();
    return a.exec();
}
