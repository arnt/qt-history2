#include <QtGui>

#include "renderarea.h"

RenderArea::RenderArea(const QPainterPath &path, QWidget *parent)
    : QWidget(parent), path(path)
{
    strokeWidth = 1;
    rotationAngle = 0;
    setBackgroundRole(QPalette::Base);
}

QSize RenderArea::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize RenderArea::sizeHint() const
{
    return QSize(100, 100);
}

void RenderArea::setFillRule(Qt::FillRule rule)
{
    path.setFillRule(rule);
    update();
}

void RenderArea::setFillGradient(const QColor &color1, const QColor &color2)
{
    fillColor1 = color1;
    fillColor2 = color2;
    update();
}

void RenderArea::setStrokeWidth(int width)
{
    strokeWidth = width;
    update();
}

void RenderArea::setStrokeGradient(const QColor &color1, const QColor &color2)
{
    strokeColor1 = color1;
    strokeColor2 = color2;
    update();
}

void RenderArea::setRotationAngle(int degrees)
{
    rotationAngle = degrees;
    update();
}

void RenderArea::paintEvent(QPaintEvent *)
{
    QPainterPathStroker stroker;
    stroker.setWidth(strokeWidth);
    stroker.setCapStyle(Qt::RoundCap);
    stroker.setJoinStyle(Qt::RoundJoin);
    QPainterPath stroke = stroker.createStroke(path);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.scale(width() / 100.0, height() / 100.0);
    painter.translate(50.0, 50.0);
    painter.rotate((double)-rotationAngle);
    painter.translate(-50.0, -50.0);

    QBrush pathBrush(rect().topLeft(), fillColor1,
                     rect().bottomLeft(), fillColor2);
    QBrush strokeBrush(rect().topLeft(), strokeColor1,
                       rect().bottomLeft(), strokeColor2);

    painter.fillPath(path, pathBrush);
    painter.fillPath(stroke, strokeBrush);
}
