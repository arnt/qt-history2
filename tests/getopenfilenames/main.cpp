/****************************************************************************
** $Id$
**
** Implementation of something useful.
**
** Created : 979899
**
** Copyright (C) 1997 by Trolltech AS.  All rights reserved.
**
*****************************************************************************/

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
