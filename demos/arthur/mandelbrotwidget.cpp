#include <qcursor.h>
#include <qimage.h>
#include <qfontmetrics.h>
#include <qpainter.h>

#include "mandelbrotwidget.h"

const double defaultCX = 0.349507;
const double defaultCY = -0.085964;
const double defaultScale = 0.000004;

MandelbrotWidget::MandelbrotWidget(QWidget *parent)
    : DemoWidget(parent)
{
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setCursor(Qt::CrossCursor);
    setFocusPolicy(Qt::WheelFocus);
    cx = defaultCX;
    cy = defaultCY;
    lastScale = scale = defaultScale;

    qRegisterMetaType<QImage>("QImage");
    connect(&renderThread, SIGNAL(renderingDone(const QImage &)),
            this, SLOT(drawRenderedImage(const QImage &)));

}

QSize MandelbrotWidget::sizeHint() const
{
    return QSize(400, 300);
}

void MandelbrotWidget::zoom(Zoom z)
{
    scale *= (z == ZoomIn) ? 1.25 : 0.8;
    renderThread.startRendering(this, cx, cy, scale, width(), height());
    update();
}

void MandelbrotWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    if (!pixmap.isNull()) {
        if (scale != lastScale) {
            double scaleFactor = scale / lastScale;
            int newWidth = (int) (pixmap.width() * scaleFactor);
            int newHeight = (int) (pixmap.height() * scaleFactor);
            int newStartX = (width() / 2) - newWidth / 2;
            int newStartY = (height() / 2) - newHeight / 2;

            QPixmap scaledPixmap(newWidth, newHeight);
            scaledPixmap.fill(Qt::black);


            QMatrix matrix;
            matrix.scale(1/scaleFactor, 1/scaleFactor);
            QPainter px(&scaledPixmap);
            px.drawPixmap(0, 0, pixmap, newStartX, newStartY, newWidth, newHeight);
            px.end();

            p.drawPixmap(pixmapDrawPoint, scaledPixmap.transform(matrix));
        } else {
            p.drawPixmap(pixmapDrawPoint, pixmap);
        }

        QString text = tr("cx = %1, cy = %2, scale = %3").arg(cx).arg(cy).arg(scale);
        QFontMetrics fm = p.fontMetrics();
        p.setBrush(Qt::black);
        p.fillRect(0, 0, fm.width(text), fm.lineSpacing(), Qt::black);
        p.setPen(Qt::white);
        p.drawText(0, fm.leading() + fm.ascent(), text);
    } else {
        p.fillRect(rect(), Qt::black);
        p.setPen(Qt::white);
        p.drawText(rect(), Qt::AlignCenter, tr("Rendering initial image, please wait..."));
    }
}

void MandelbrotWidget::mousePressEvent(QMouseEvent *event)
{
    moveAnchor = event->pos();
}

void MandelbrotWidget::mouseMoveEvent(QMouseEvent *event)
{
    pixmapDrawPoint += event->pos() - moveAnchor;
    moveAnchor = event->pos();

    update();
}

void MandelbrotWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        cx = defaultCX;
        cy = defaultCY;
        scale = lastScale = defaultScale;
        renderThread.startRendering(this, cx, cy, scale, width(), height());

        pixmapDrawPoint = moveAnchor = QPoint();
        pixmap = QPixmap();
        update();
        return;
    }

    pixmapDrawPoint += event->pos() - moveAnchor;
    moveAnchor = QPoint();

    int x = width()/2 - (pixmapDrawPoint.x() + pixmap.width()/2);
    int y = height()/2 - (pixmapDrawPoint.y() + pixmap.height()/2);
    cx += x * scale;
    cy += y * scale;

    update();

    renderThread.startRendering(this, cx, cy, scale, width(), height());
}

void MandelbrotWidget::wheelEvent(QWheelEvent *event)
{
    zoom((event->delta() > 0) ? ZoomIn : ZoomOut);
}

void MandelbrotWidget::resizeEvent(QResizeEvent *)
{
    if (!pixmap.isNull()) {
        QPixmap pm(width(), height());
        pm.fill(Qt::black);

        QPainter px(&pm);
        px.drawPixmap(pixmapDrawPoint, pixmap);
        px.end();

        pixmapDrawPoint = moveAnchor = QPoint();
        pixmap = pm;
        update();
    }

    renderThread.startRendering(this, cx, cy, scale, width(), height());
}

void MandelbrotWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
        zoom(ZoomOut);
        break;
    case Qt::Key_Down:
        zoom(ZoomIn);
        break;
    }
}

void MandelbrotWidget::drawRenderedImage(const QImage &image)
{
    if (!moveAnchor.isNull())
        return;

    pixmap = image;
    moveAnchor = QPoint();
    pixmapDrawPoint = QPoint();
    lastScale = scale;
    update();
}
