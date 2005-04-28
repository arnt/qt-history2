#ifndef INTERFACES_H
#define INTERFACES_H

class QImage;
class QPainter;
class QPainterPath;
class QPoint;
class QRect;
class QString;
class QStringList;

class BrushInterface
{
public:
    virtual QStringList brushes() const = 0;
    virtual QRect mousePress(const QString &brush, QPainter &painter,
                             const QPoint &pos) = 0;
    virtual QRect mouseMove(const QString &brush, QPainter &painter,
                            const QPoint &oldPos, const QPoint &newPos) = 0;
    virtual QRect mouseRelease(const QString &brush, QPainter &painter,
                               const QPoint &pos) = 0;
};

class ShapeInterface
{
public:
    virtual QStringList shapes() const = 0;
    virtual QPainterPath generateShape(const QString &shape,
                                       QWidget *parent) = 0;
};

class FilterInterface
{
public:
    virtual QStringList filters() const = 0;
    virtual QImage filterImage(const QString &filter, const QImage &image) = 0;
};

Q_DECLARE_INTERFACE(BrushInterface,
                    "http://trolltech.com/PlugAndPaint/BrushInterface");
Q_DECLARE_INTERFACE(ShapeInterface,
                    "http://trolltech.com/PlugAndPaint/ShapeInterface");
Q_DECLARE_INTERFACE(FilterInterface,
                    "http://trolltech.com/PlugAndPaint/FilterInterface");

#endif
