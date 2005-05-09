/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************
**
** Qt tutorial 13
**
****************************************************************/

#include <QApplication>

#include "gameboard.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    GameBoard board;
    board.setGeometry(100, 100, 500, 355);
    board.show();
    return app.exec();
}
