/****************************************************************************
**
** Implementation of something useful.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** EDITIONS: UNKNOWN
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qfiledialog.h>
#include <qapplication.h>

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    QStrList s( QFileDialog::getOpenFileNames( 0, 0, 0 ) );
    printf( "got %d files\n", s.count() );
    s.first();
    do {
	if ( s.current() )
	    printf( "got <%s>\n", s.current() );
    } while( s.next() );
    return 0;
}
