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

    QSize minimumSizeHint() const;
    QList<Operation> operations() const;
    void setOperations(const QList<Operation> &operations);
    void setShape(const QPainterPath &shape);

protected:
    void paintEvent(QPaintEvent *event);

private:
    void drawCoordinates(QPainter &painter);
    void drawOutline(QPainter &painter);
    void drawShape(QPainter &painter);
    void transformPainter(QPainter &painter);

    QFont font;
    QList<Operation> transforms;
    QPainterPath painterShape;
    QRect xBoundingRect;
    QRect yBoundingRect;
};

#endif
