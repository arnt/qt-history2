#ifndef PAINTWIDGET_H
#define PAINTWIDGET_H

#include <QFont>
#include <QList>
#include <QPointArray>
#include <QRect>
#include <QWidget>

class QPaintEvent;

enum Operation { NoTransformation, Translate, Rotate, Scale };

class PaintWidget : public QWidget
{
    Q_OBJECT

public:
    PaintWidget(QWidget *parent, bool fixed = false);
    QList<Operation> operations() const;
    void setOperations(const QList<Operation> operations);
    QSize minimumSizeHint() const;

protected:
    void paintEvent(QPaintEvent *event);

private:
    void drawCoordinates(QPainter &painter);
    void drawOutline(QPainter &painter);
    void drawShape(QPainter &painter);
    void transformPainter(QPainter &painter);

    bool fixed;
    QFont font;
    QList<Operation> transforms;
    QPointArray shape;
    QRect xBoundingRect;
    QRect yBoundingRect;
};

#endif
