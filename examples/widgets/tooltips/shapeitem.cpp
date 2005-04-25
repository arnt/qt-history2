#include <QtGui>

#include "shapeitem.h"

QPainterPath ShapeItem::path() const
{
    return myPath;
}

QPoint ShapeItem::position() const
{
    return myPosition;
}

QColor ShapeItem::color() const
{
    return myColor;
}

QString ShapeItem::toolTip() const
{
    return myToolTip;
}

void ShapeItem::setPath(const QPainterPath &path)
{
    myPath = path;
}

void ShapeItem::setToolTip(const QString &toolTip)
{
    myToolTip = toolTip;
}

void ShapeItem::setPosition(const QPoint &position)
{
    myPosition = position;
}

void ShapeItem::setColor(const QColor &color)
{
    myColor = color;
}
