/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
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

    GLObjectWindow w;
    w.resize( 550, 350 );
    a.setMainWidget( &w );
    w.show();
    return a.exec();
}
