/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/
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
#include "glbox.h"
#include <qapplication.h>
#include <qgl.h>

#include <qaxfactory.h>

QAXFACTORY_DEFAULT( GLBox, 
		    "{5fd9c22e-ed45-43fa-ba13-1530bb6b03e0}",
		    "{33b051af-bb25-47cf-a390-5cfd2987d26a}",
		    "{8c996c29-eafa-46ac-a6f9-901951e765b5}",
		    "{2c3c183a-eeda-41a4-896e-3d9c12c3577d}",
		    "{83e16271-6480-45d5-aaf1-3f40b7661ae4}"
		  )

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

    if ( !QAxFactory::isServer() ) {
	GLObjectWindow w;
	w.resize( 400, 350 );
	a.setMainWidget( &w );
	w.show();
	return a.exec();
    }
    return a.exec();
}

