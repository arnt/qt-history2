#include <QtGui>

#include <math.h>

#include "mandelbrotwidget.h"

const double DefaultCenterX = -0.637011;
const double DefaultCenterY = -0.0395159;
const double DefaultScale = 0.00403897;

const double ZoomInFactor = 1.25;
const double ZoomOutFactor = 1 / ZoomInFactor;
const int ScrollStep = 20;

MandelbrotWidget::MandelbrotWidget(QWidget *parent)
    : QWidget(parent)
{
    setCursor(Qt::CrossCursor);
    setFocusPolicy(Qt::WheelFocus);
    centerX = DefaultCenterX;
    centerY = DefaultCenterY;
    prevScale = DefaultScale;
    curScale = DefaultScale;

    qRegisterMetaType<QImage>("QImage");
    connect(&thread, SIGNAL(finishedRendering(const QImage &)),
            this, SLOT(drawRenderedImage(const QImage &)));
}

QSize MandelbrotWidget::sizeHint() const
{
    return QSize(550, 400);
}

void MandelbrotWidget::zoom(double zoomFactor)
{
    curScale *= zoomFactor;
    thread.render(centerX, centerY, curScale, size());
    update();
}

void MandelbrotWidget::scroll(int deltaX, int deltaY)
{
    centerX += deltaX * curScale;
    centerY += deltaY * curScale;
    update();
    thread.render(centerX, centerY, curScale, size());
}

void MandelbrotWidget::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);

    if (pixmap.isNull()) {
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter,
                         tr("Rendering initial image, please wait..."));
        return;
    }

    if (curScale == prevScale) {
        painter.drawPixmap(pixmapDrawPoint, pixmap);
    } else {
        double scaleFactor = curScale / prevScale;
        int newWidth = int(pixmap.width() * scaleFactor);
        int newHeight = int(pixmap.height() * scaleFactor);
        int newStartX = (width() / 2) - newWidth / 2;
        int newStartY = (height() / 2) - newHeight / 2;

        QPixmap scaledPixmap(newWidth, newHeight);
        scaledPixmap.fill(Qt::black);

        QMatrix matrix;
        matrix.scale(1 / scaleFactor, 1 / scaleFactor);
        QPainter pixmapPainter(&scaledPixmap);
        pixmapPainter.drawPixmap(0, 0, pixmap, newStartX, newStartY,
                                 newWidth, newHeight);
        pixmapPainter.end();

        painter.drawPixmap(pixmapDrawPoint, scaledPixmap.transform(matrix));
    }

    QString text = tr("Use mouse wheel to zoom. "
                      "Press and hold left mouse button to scroll.");
    QFontMetrics metrics = painter.fontMetrics();
    int textWidth = metrics.width(text);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 127));
    painter.drawRect(width() / 2 - textWidth / 2 - 5, 0, textWidth + 10,
                     metrics.lineSpacing() + 5);
    painter.setPen(Qt::white);
    painter.drawText(width() / 2 - textWidth / 2,
                     metrics.leading() + metrics.ascent(), text);
}

void MandelbrotWidget::mousePressEvent(QMouseEvent *event)
{
    lastDragPos = event->pos();
}

void MandelbrotWidget::mouseMoveEvent(QMouseEvent *event)
{
    pixmapDrawPoint += event->pos() - lastDragPos;
    lastDragPos = event->pos();
    update();
}

void MandelbrotWidget::mouseReleaseEvent(QMouseEvent *event)
{
    pixmapDrawPoint += event->pos() - lastDragPos;
    lastDragPos = QPoint();

    int deltaX = width() / 2 - (pixmapDrawPoint.x() + pixmap.width() / 2);
    int deltaY = height() / 2 - (pixmapDrawPoint.y() + pixmap.height() / 2);
    scroll(deltaX, deltaY);
}

void MandelbrotWidget::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->delta() / 8;
    double numSteps = numDegrees / 15.0;
    zoom(pow(ZoomInFactor, numSteps));
}

void MandelbrotWidget::resizeEvent(QResizeEvent * /* event */)
{
    if (!pixmap.isNull()) {
        QPixmap newPixmap(width(), height());
        newPixmap.fill(Qt::black);

        QPainter painter(&newPixmap);
        painter.drawPixmap(pixmapDrawPoint, pixmap);
        painter.end();

        pixmapDrawPoint = QPoint();
        lastDragPos = QPoint();
        pixmap = newPixmap;
        update();
    }
    thread.render(centerX, centerY, curScale, size());
}

void MandelbrotWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Plus:
        zoom(ZoomInFactor);
        break;
    case Qt::Key_Minus:
        zoom(ZoomOutFactor);
        break;
    case Qt::Key_Left:
        scroll(-ScrollStep, 0);
        break;
    case Qt::Key_Right:
        scroll(+ScrollStep, 0);
        break;
    case Qt::Key_Down:
        scroll(0, -ScrollStep);
        break;
    case Qt::Key_Up:
        scroll(0, +ScrollStep);
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void MandelbrotWidget::drawRenderedImage(const QImage &image)
{
    if (!lastDragPos.isNull())
        return;

    pixmap = image;
    lastDragPos = QPoint();
    pixmapDrawPoint = QPoint();
    prevScale = curScale;
    update();
}
