#include <QtGui>

#include "renderarea.h"

RenderArea::RenderArea(const QPainterPath &path, QWidget *parent)
    : QWidget(parent)
{
    this->path = path;
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

void RenderArea::paintEvent(QPaintEvent *)
{
    QPainterPathStroker stroker;
    stroker.setWidth(2);
    stroker.setJoinStyle(Qt::RoundJoin);
    QPainterPath stroke = stroker.createStroke(path);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.scale(width() / 100.0, height() / 100.0);

    QBrush pathBrush(rect().topLeft(), QColor(219, 238, 188),
                     rect().bottomLeft(), QColor(59, 156, 69));
    QBrush strokeBrush(rect().topLeft(), QColor(150, 170, 140),
                       rect().bottomLeft(), QColor(0, 100, 20));

    painter.fillPath(path, pathBrush);
    painter.fillPath(stroke, strokeBrush);
}
