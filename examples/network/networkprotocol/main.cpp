/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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
