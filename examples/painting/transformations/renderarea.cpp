#include <QtGui>

#include "renderarea.h"

RenderArea::RenderArea(QWidget *parent)
    : QWidget(parent)
{
    font.setPixelSize(12);
    QFontMetrics fontMetrics(font);
    xBoundingRect = fontMetrics.boundingRect(tr("x"));
    yBoundingRect = fontMetrics.boundingRect(tr("y"));
}

void RenderArea::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(event->rect(), QBrush(Qt::white));

    painter.translate(66, 66);

    painter.save();
    transformPainter(painter);
    drawShape(painter);
    painter.restore();

    drawOutline(painter);

    painter.save();
    transformPainter(painter);
    drawCoordinates(painter);
    painter.restore();
}

void RenderArea::drawCoordinates(QPainter &painter)
{
    painter.setPen(Qt::red);
    painter.setFont(font);

    painter.drawLine(0, 0, 50, 0);
    painter.drawLine(48, -2, 50, 0);
    painter.drawLine(48, 2, 50, 0);
    painter.drawText(60 - xBoundingRect.width() / 2,
                     0 + xBoundingRect.height() / 2, tr("x"));

    painter.drawLine(0, 0, 0, 50);
    painter.drawLine(-2, 48, 0, 50);
    painter.drawLine(2, 48, 0, 50);
    painter.drawText(0 - yBoundingRect.width() / 2,
                     60 + yBoundingRect.height() / 2, tr("y"));
}

void RenderArea::drawOutline(QPainter &painter)
{
    painter.setPen(Qt::darkGreen);
    painter.setPen(Qt::DashLine);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(0, 0, 100, 100);
}

void RenderArea::drawShape(QPainter &painter)
{
    painter.setBrush(Qt::black);
    painter.drawPath(painterShape);
}

QSize RenderArea::minimumSizeHint() const
{
    return QSize(232, 232);
}

QList<Operation> RenderArea::operations() const
{
    return transforms;
}

void RenderArea::setOperations(const QList<Operation> &operations)
{
    transforms = operations;
    update();
}

void RenderArea::setShape(const QPainterPath &shape)
{
    painterShape = shape;
    update();
}

void RenderArea::transformPainter(QPainter &painter)
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
            ;
        }
    }
}
