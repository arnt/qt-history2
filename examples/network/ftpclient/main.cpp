/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qnetwork.h>
#include <qsplitter.h>

#include "ftpmainwindow.h"

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    // call this to register the built-in network protocols, e.g. FTP
    // and HTTP.
    qInitNetworkProtocols();

    FtpMainWindow m;
    a.setMainWidget( &m );
    QValueList<int> sizes;
    sizes << 300 << 70 << 300;
    m.mainSplitter()->setSizes( sizes );
    m.resize( 800, 600 );
    m.show();
    return a.exec();
}
