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
#include <qapplication.h>
#include <qstringlist.h>
#include <qdir.h>
#include "kioskwidget.h"
#include "../themes/wood.h"

main(int argc, char **argv)
{
    QApplication app(argc,argv);
    QApplication::setFont( QFont( "helvetica", 16, QFont::Bold ) );
    QApplication::setStyle( new NorwegianWoodStyle( 30 ) );
    QDir dir( "." );

    QStringList files;
    if ( argc > 1 ) {
	for ( int i = 1; i < argc ; i++ )
	    if ( dir.exists( argv[i] )  )
		      files.append( argv[i] );
    } 
    
    if ( files.count() < 1 ) {
	files = dir.entryList( "*.mpg" );
    }
    
    KioskWidget mw( files );
    mw.showMaximized();

    
    QObject::connect(qApp,SIGNAL(lastWindowClosed()),qApp,SLOT(quit()));
    return app.exec();
}
