//
// Qt OpenGL example: Texture
//
// File: main.cpp
//
// The main() function 
// 

#include "globjwin.h"
#include <qapplication.h>
#include <qgl.h>

/*
  The main program is here. 
*/

int main( int argc, char **argv )
{
    QApplication::setColorSpec( QApplication::CustomColor );
    QApplication a(argc,argv);

    if ( !QGLFormat::hasOpenGL() ) {
	qWarning( "This system has no OpenGL support. Exiting." );
	return -1;
    }
    
    GLObjectWindow* w = new GLObjectWindow;
    w->resize( 400, 350 );
    a.setMainWidget( w );
    w->show();
    int result = a.exec();
    delete w;
    return result;
}
