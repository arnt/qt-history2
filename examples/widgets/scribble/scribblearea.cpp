#include <QtGui>

#include "scribblearea.h"

ScribbleArea::ScribbleArea(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StaticContents);
    modified = false;
    scribbling = false;
    penWidth = 1;
    penColor = Qt::blue;
}

bool ScribbleArea::openImage(const QString &fileName)
{
    QImage newImage;
    if (!newImage.load(fileName))
        return false;

    QPainter painter(&image);
    image.fill(qRgb(255, 255, 255));
    painter.drawImage(QPoint(0, 0), newImage);
    update();
    return true;
}

bool ScribbleArea::saveImage(const QString &fileName, const char *fileFormat)
{
    QImage visibleImage(width(), height(), QImage::Format_ARGB32);
    QPainter painter(&visibleImage);
    painter.drawImage(QPoint(0, 0), image);

    if (visibleImage.save(fileName, fileFormat)) {
        modified = false;
        return true;
    } else {
        return false;
    }
}

QColor ScribbleArea::getPenColor()
{
    return penColor;
}

int ScribbleArea::getPenWidth()
{
    return penWidth;
}

void ScribbleArea::setPenColor(const QColor &newColor)
{
    penColor = newColor;
}

void ScribbleArea::setPenWidth(int newWidth)
{
    penWidth = newWidth;
}

void ScribbleArea::clearImage()
{
    image.fill(qRgb(255, 255, 255));
    modified = true;
    update();
}

void ScribbleArea::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        startPoint = event->pos();
        scribbling = true;
    }
}

void ScribbleArea::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && scribbling)
        drawLineTo(event->pos());
}

void ScribbleArea::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && scribbling) {
        drawLineTo(event->pos());
        scribbling = false;
    }
}

void ScribbleArea::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    painter.drawImage(QPoint(0, 0), image);
}

void ScribbleArea::resizeEvent(QResizeEvent *event)
{
    if (width() > image.width() || height() > image.height()) {
        int newWidth = qMax(width() + 128, image.width());
        int newHeight = qMax(height() + 128, image.height());

        QImage newImage = QImage(newWidth, newHeight, QImage::Format_ARGB32);
        newImage.fill(qRgb(255, 255, 255));
        QPainter painter(&newImage);
        painter.drawImage(QPoint(0, 0), image);
        image = newImage;
        update();
    }
    QWidget::resizeEvent(event);
}

void ScribbleArea::drawLineTo(const QPoint &endPoint)
{
    QPainter painter(&image);
    painter.setPen(QPen(penColor, penWidth, Qt::SolidLine, Qt::RoundCap,
                        Qt::RoundJoin));
    painter.drawLine(startPoint, endPoint);
    modified = true;

    int rad = penWidth / 2;
    update(QRect(startPoint, endPoint).normalize()
                                      .adjusted(-rad, -rad, +rad, +rad));
    startPoint = endPoint;
}
