#ifndef RENDERAREA_H
#define RENDERAREA_H

#include <QFont>
#include <QList>
#include <QPainterPath>
#include <QRect>
#include <QWidget>

class QPaintEvent;

enum Operation { NoTransformation, Translate, Rotate, Scale };

class RenderArea : public QWidget
{
    Q_OBJECT

public:
    RenderArea(QWidget *parent = 0);

    void setOperations(const QList<Operation> &operations);
    void setShape(const QPainterPath &shape);

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent *event);

private:
    void drawCoordinates(QPainter &painter);
    void drawOutline(QPainter &painter);
    void drawShape(QPainter &painter);
    void transformPainter(QPainter &painter);

    QList<Operation> operations;
    QPainterPath shape;
    QRect xBoundingRect;
    QRect yBoundingRect;
};

#endif
