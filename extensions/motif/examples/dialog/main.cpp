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

#include <QApplication>

#include "mainwindow.h"

#include <QtMotif/QMotif>

int main( int argc, char **argv )
{
    XtSetLanguageProc( NULL, NULL, NULL );

    QMotif integrator( "dialog" );
    QApplication app( argc, argv );

    MainWindow mainwindow;
    mainwindow.show();

    return app.exec();
}
