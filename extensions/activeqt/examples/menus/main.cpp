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
#include <qaxfactory.h>

#include "menus.h"

QAXFACTORY_DEFAULT(QMenus,
		    "{4dc3f340-a6f7-44e4-a79b-3e9217695fbd}",
		    "{9ee49617-7d5c-441a-b833-4b068d40d751}",
		    "{13eca64b-ee2a-4f3c-aa04-5d9d975979a7}",
		    "{ce947ee3-0403-4fdc-895a-4fe779394b46}",
		    "{8de435ce-8d2a-46ac-b3b3-cb800d0847c7}");

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QWidget *window = 0;
    if ( !QAxFactory::isServer() ) {
	window = new QMenus();
	window->show();
    }

    return a.exec();
}
