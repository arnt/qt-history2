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

#include "mainwindow.h"

#include <QApplication>
#include <QAxFactory>

QAXFACTORY_DEFAULT(MainWindow,
		   "{5f5ce700-48a8-47b1-9b06-3b7f79e41d7c}",
		   "{3fc86f5f-8b15-4428-8f6b-482bae91f1ae}",
		   "{02a268cd-24b4-4fd9-88ff-b01b683ef39d}",
		   "{4a43e44d-9d1d-47e5-a1e5-58fe6f7be0a4}",
		   "{16ee5998-77d2-412f-ad91-8596e29f123f}")

int main( int argc, char **argv ) 
{
    QApplication app( argc, argv );

    MainWindow mw;
    mw.show();

    return app.exec();;
}
