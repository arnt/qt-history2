/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt GUI Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qapplication.h>
#include "../qwidgetfactory.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    if ( argc == 1 )
	return 0;
    if ( argc == 3 )
	QWidgetFactory::loadImages( argv[ 2 ] );
    QWidget *w = QWidgetFactory::create( argv[ 1 ] );
    if ( !w )
	return 0;
    w->show();
    a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
    return a.exec();
}
