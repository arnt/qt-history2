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

#include <qapplication.h>
#include "outlinetree.h"

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    OutlineTree outline( "todos.opml" );
    a.setMainWidget( &outline );
    outline.show();

    return a.exec();
}
