/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "textedit.h"
#include <QApplication>

int main( int argc, char ** argv )
{
    Q_INIT_RESOURCE(textedit);

    QApplication a( argc, argv );
    TextEdit mw;
    mw.resize( 700, 800 );
    mw.show();
    return a.exec();
}
