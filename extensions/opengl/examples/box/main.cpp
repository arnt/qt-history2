//
// Qt OpenGL example: Box
//
// A small example showing how a GLWidget can be used just as any Qt widget
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
    w.resize( 400, 350 );
    a.setMainWidget( &w );
    w.show();
    return a.exec();
}
