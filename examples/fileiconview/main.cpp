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
#include "qfileiconview.h"

#include <qapplication.h>


int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    FileMainWindow mw;
    mw.resize( 680, 480 );
    a.setMainWidget( &mw );
    mw.fileView()->setDirectory( "/" );
    mw.show();
    return a.exec();
}
