/****************************************************************************
** $Id: //depot/qt/main/examples/qfileiconview/main.cpp#10 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qnetwork.h>

#include "nntp.h"
#include "view.h"

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    qInitNetworkProtocols();
    QNetworkProtocol::registerNetworkProtocol( "nntp", new QNetworkProtocolFactory<Nntp> );
    
    View v;
    v.resize( 600, 600 );
    v.show();
    a.setMainWidget( &v );
    
    return a.exec();
}
