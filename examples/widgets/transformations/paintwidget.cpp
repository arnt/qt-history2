#include <QtGui>

#include "paintwidget.h"

PaintWidget::PaintWidget(QWidget *parent, bool enable)
    : QWidget(parent), fixed(enable)
{
    shape = QPointArray(6);
    shape << QPoint(-45, -20);
    shape << QPoint(  0, -45);
    shape << QPoint( 45, -20);
    shape << QPoint( 45,  45);
    shape << QPoint(-45,  45);
    shape << QPoint(-45, -20);

    font.setPixelSize(12);
    QFontMetrics fontMetrics(font);
    xBoundingRect = fontMetrics.boundingRect(tr("x"));
    yBoundingRect = fontMetrics.boundingRect(tr("y"));
}

void PaintWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setBrush(Qt::white);
    painter.drawRect(event->rect());

    if (fixed)
        painter.translate(116, 116);
    else
        painter.translate(66, 66);

    painter.save();
    transformPainter(painter);
    drawShape(painter);
    painter.restore();

    if (!fixed)
        drawOutline(painter);

    painter.save();
    transformPainter(painter);
    drawCoordinates(painter);
    painter.restore();
}

void PaintWidget::drawCoordinates(QPainter &painter)
{
    painter.setPen(Qt::red);
    painter.setFont(font);

    painter.drawLine(0, 0, 50, 0);
    painter.drawLine(48, -2, 50, 0);
    painter.drawLine(48, 2, 50, 0);
    painter.drawText(60 - xBoundingRect.width()/2,
                     0 + xBoundingRect.height()/2, tr("x"));

    painter.drawLine(0, 0, 0, 50);
    painter.drawLine(-2, 48, 0, 50);
    painter.drawLine(2, 48, 0, 50);
    painter.drawText(0 - yBoundingRect.width()/2,
                     60 + yBoundingRect.height()/2, tr("y"));
}

void PaintWidget::drawOutline(QPainter &painter)
{
    painter.setPen(Qt::darkGreen);
    painter.setPen(Qt::DashLine);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(0, 0, 100, 100);
}

void PaintWidget::drawShape(QPainter &painter)
{
    painter.setBrush(Qt::black);
    painter.drawPolygon(shape);
    painter.setBrush(Qt::white);
    painter.drawRect(15, 5, 20, 35);
    painter.drawRect(-35, -15, 25, 25);
}

void PaintWidget::transformPainter(QPainter &painter)
{
    for (int i = 0; i < transforms.count(); ++i) {
        switch (transforms[i]) {
            case Translate:
                painter.translate(50, 50);
                break;
            case Scale:
                painter.scale(0.75, 0.75);
                break;
            case Rotate:
                painter.rotate(60);
                break;
            case NoTransformation:
            default:
                break;
        }
    }
}

void PaintWidget::setOperations(const QList<Operation> operations)
{
    transforms = operations;
    update();
}

QList<Operation> PaintWidget::operations() const 
{
    return transforms;
}

QSize PaintWidget::minimumSizeHint() const
{
    return QSize(232, 232);
}
