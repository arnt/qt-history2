/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qapplication.h>
#include <qfileinfo.h>
#include <qdir.h>
#include "dirview.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );

    DirectoryView mw;

    mw.addColumn( "Name" );
    mw.addColumn( "Type" );
    mw.setTreeStepSize( 20 );

    const QFileInfoList roots = QDir::drives();
    for (int i=0;i<roots.size();++i) {
	Directory * root = new Directory( &mw, roots.at(i).filePath() );
	if ( roots.size() <= 1 )
	    root->setOpen( TRUE ); // be interesting
    }

    mw.resize( 400, 400 );
    mw.setWindowTitle( "Qt Example - Directory Browser" );
    mw.setAllColumnsShowFocus( TRUE );
    a.setMainWidget( &mw );
    mw.show();

    return a.exec();
}
