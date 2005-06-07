/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QApplication>

#include "ftpwindow.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(ftp);

    QApplication app(argc, argv);
    FtpWindow ftpWin;
    ftpWin.show();
    return ftpWin.exec();
}
