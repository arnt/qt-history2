#include "scribblewidget.h"

ScribbleWidget::ScribbleWidget(QWidget *parent)
    : QWidget(parent)
{
    scribbling = false;
}

void ScribbleWidget::resizeEvent(QResizeEvent *e)
{
    image = QImage(e->size(), QImage::Format_RGB32);
    image.fill(QColor(Qt::white).rgb());
}

void ScribbleWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    lastPoint = event->pos();
    scribbling = true;
}

void ScribbleWidget::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && scribbling)
        drawLineTo(event->pos());
}

void ScribbleWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && scribbling) {
        drawLineTo(event->pos());
        scribbling = false;
    }
}

void ScribbleWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawImage(QPoint(0, 0), image);
}

void ScribbleWidget::drawLineTo(const QPoint &endPoint)
{
    QPainter painter(&image);
    painter.setPen(QPen(Qt::blue, 1, Qt::SolidLine,
                        Qt::RoundCap, Qt::RoundJoin));
    painter.drawLine(lastPoint, endPoint);
    update();
    lastPoint = endPoint;
}
