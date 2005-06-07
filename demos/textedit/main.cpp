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
#include "textedit.h"

int main( int argc, char ** argv )
{
    Q_INIT_RESOURCE(textedit);

    QApplication a( argc, argv );
    TextEdit mw;
    mw.resize( 640, 800 );
    mw.show();
    return a.exec();
}
