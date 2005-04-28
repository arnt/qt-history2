#include <QtGui>

#include "interfaces.h"
#include "paintarea.h"

PaintArea::PaintArea(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StaticContents);
    setAttribute(Qt::WA_NoBackground);

    theImage = QImage(500, 400, QImage::Format_ARGB32);
    theImage.fill(qRgb(255, 255, 255));
    color = Qt::blue;
    thickness = 3;
    brushInterface = 0;
    lastPos = QPoint(-1, -1);
}

bool PaintArea::openImage(const QString &fileName)
{
    QImage image;
    if (!image.load(fileName))
        return false;

    theImage = image;
    update();
    return true;
}

bool PaintArea::saveImage(const QString &fileName)
{
    if (theImage.save(fileName, "png")) { // ### guess from file name
        return true;
    } else {
        return false;
    }
}

void PaintArea::setImage(const QImage &image)
{
    theImage = image;
    update();
}

void PaintArea::insertShape(const QPainterPath &path)
{
    pendingPath = path;
    setCursor(Qt::CrossCursor);
}

void PaintArea::setBrushColor(const QColor &color)
{
    this->color = color;
}

void PaintArea::setBrushWidth(int width)
{
    thickness = width;
}

void PaintArea::setBrush(BrushInterface *brushInterface, const QString &brush)
{
    this->brushInterface = brushInterface;
    this->brush = brush;
}

QSize PaintArea::sizeHint() const
{
    return theImage.size();
}

void PaintArea::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    painter.drawImage(QPoint(0, 0), theImage);
}

void PaintArea::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (!pendingPath.isEmpty()) {
            QPainter painter(&theImage);
            setupPainter(painter);

            QRectF boundingRect = pendingPath.boundingRect();
            QLinearGradient gradient(boundingRect.topRight(),
                                     boundingRect.bottomLeft());
            gradient.setColorAt(0.0, QColor(color.red(), color.green(),
                                            color.blue(), 63));
            gradient.setColorAt(1.0, QColor(color.red(), color.green(),
                                            color.blue(), 191));
            painter.setBrush(gradient);
            painter.translate(event->pos() - boundingRect.center());
            painter.drawPath(pendingPath);

            pendingPath = QPainterPath(); // ### clear()
            unsetCursor();
            update();
        } else {
            if (brushInterface) {
                QPainter painter(&theImage);
                setupPainter(painter);
                QRect rect = brushInterface->mousePress(brush, painter,
                                                        event->pos());
                update(rect);
            }

            lastPos = event->pos();
        }
    }
}

void PaintArea::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && lastPos != QPoint(-1, -1)) {
        if (brushInterface) {
            QPainter painter(&theImage);
            setupPainter(painter);
            QRect rect = brushInterface->mouseMove(brush, painter, lastPos,
                                                   event->pos());
            update(rect);
        }

        lastPos = event->pos();
    }
}

void PaintArea::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && lastPos != QPoint(-1, -1)) {
        if (brushInterface) {
            QPainter painter(&theImage);
            setupPainter(painter);
            QRect rect = brushInterface->mouseRelease(brush, painter,
                                                      event->pos());
            update(rect);
        }

        lastPos = QPoint(-1, -1);
    }
}

void PaintArea::setupPainter(QPainter &painter)
{
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(color, thickness, Qt::SolidLine, Qt::RoundCap,
                   Qt::RoundJoin));
}
