/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
    main.cpp

    Provides the main function for the QImage color separation example.
*/

#include <QApplication>

#include "viewer.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Viewer viewer;
    viewer.show();
    return app.exec();
}
