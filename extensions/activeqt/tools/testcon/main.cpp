/****************************************************************************
**
** Copyright (C) 2001-$THISYEAR$ Trolltech AS.  All rights reserved.
**
** This file is part of an example program for the ActiveQt integration.
** This example program may be used, distributed and modified without 
** limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qaxfactory.h>

#include "mainwindow.h"

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
