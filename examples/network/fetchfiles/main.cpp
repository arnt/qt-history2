/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qnetwork.h>

#include "fetchwidget.h"

int main( int argc, char** argv )
{
    QApplication app( argc, argv );
    qInitNetworkProtocols();
    FetchWidget fw;
    app.setMainWidget( &fw );
    fw.show();
    return app.exec();
}
