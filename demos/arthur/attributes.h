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

#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H

#include <qpixmap.h>
#include <qcolor.h>
#include <qapplication.h>
#include <qpalette.h>
class Attributes
{
public:
    enum BackgroundFill { Solid, Gradient, Tiles, Pixmap };

    Attributes()
        : antialias(false),
          alpha(false),
          fillMode(Solid)
    {
        QPalette pal = QApplication::palette();
        QColor c = pal.base().color();
        if (!c.isValid())
            c = Qt::gray;

        color = c.light();
        secondaryColor = c.dark();
        pattern.load(":/res/bg1.jpg");
        tile.load(":/res/qtlogo.png");
    }

    bool antialias;
    bool alpha;

    BackgroundFill fillMode;

    QPixmap pattern;
    QPixmap tile;
    QColor color;
    QColor secondaryColor;
};

#endif
