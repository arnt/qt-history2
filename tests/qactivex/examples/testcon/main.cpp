/****************************************************************************
** $Id: $
**
** Copyright (C) 2001-2002 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for the ActiveQt integration.
** This example program may be used, distributed and modified without 
** limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include "mainwindow.h"

int main( int argc, char **argv ) 
{
    QApplication app( argc, argv );

    MainWindow mw;
    mw.show();

    app.setMainWidget( &mw );
    return app.exec();;
}
