/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "hello.h"
#include <qapplication.h>


/*
  The program starts here. It parses the command line and builds a message
  string to be displayed by the Hello widget.
*/

int main( int argc, char **argv )
{
    QApplication a(argc,argv);
    QString s;
    for ( int i=1; i<argc; i++ ) {
	s += argv[i];
	if ( i<argc-1 )
	    s += " ";
    }
    if ( s.isEmpty() )
	s = "Hello, World";
    Hello h( s );
#ifndef QT_NO_WIDGET_TOPEXTRA	// for Qt/Embedded minimal build
    h.setWindowTitle( "Qt says hello" );
#endif
    QObject::connect( &h, SIGNAL(clicked()), &a, SLOT(quit()) );
    h.setFont( QFont("times",32,QFont::Bold) );		// default font
    h.setBackgroundRole(QPalette::Base);		//  text bg color
    a.setMainWidget( &h );
    h.show();
    return a.exec();
}
