#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H

#include <qpixmap.h>
#include <qcolor.h>

class Attributes
{
public:
    enum BackgroundFill { Solid, Gradient, Tiles, Pixmap };

    Attributes()
        : antialias(false),
          alpha(false),
          fillMode(Solid)
    {
        color = Qt::white;
        secondaryColor = Qt::black;
        pattern.load("dome.jpg");
        tile.load("qtlogo.png");
    }

    bool antialias;
    bool alpha;

    BackgroundFill fillMode;

    QPixmap pattern;
    QPixmap tile;
    QColor color;
    QColor secondaryColor;
};

#endif // ATTRIBUTES_H
