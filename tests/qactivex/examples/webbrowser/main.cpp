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

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    MainWindow *w = new MainWindow;
    a.setMainWidget( w );
    w->show();
    return a.exec();
}
