#include <QtGui>

#include "renderarea.h"

RenderArea::RenderArea(QWidget *parent)
    : QWidget(parent)
{
    shape = Polygon;
    antialiased = false;
    pixmap.load(":/images/qt-logo.png");

    setBackgroundRole(QPalette::Base);
}

QSize RenderArea::minimumSizeHint() const
{
    return QSize(100, 100);
}

QSize RenderArea::sizeHint() const
{
    return QSize(400, 200);
}

void RenderArea::setShape(Shape shape)
{
    this->shape = shape;
    update();
}

void RenderArea::setPen(const QPen &pen)
{
    this->pen = pen;
    update();
}

void RenderArea::setBrush(const QBrush &brush)
{
    this->brush = brush;
    update();
}

void RenderArea::setAntialiased(bool antialiased)
{
    this->antialiased = antialiased;
    update();
}

void RenderArea::setTransformed(bool transformed)
{
    this->transformed = transformed;
    update();
}

void RenderArea::paintEvent(QPaintEvent *)
{
    static const int polygonPoints[8] = { 10, 80, 20, 10, 80, 30, 90, 70 };
    QPolygon polygon(4, polygonPoints);

    QRect rect(10, 20, 80, 60);

    QPainterPath path;
    path.moveTo(20, 80);
    path.lineTo(20, 30);
    path.curveTo(80, 0, 50, 50, 80, 80);

    int alpha = 30 * 16;
    int beta = 120 * 16;

    QPainter painter(this);
    painter.setPen(pen);
    painter.setBrush(brush);
    painter.setRenderHint(QPainter::Antialiasing, antialiased);
    painter.setRenderHint(QPainter::TextAntialiasing, antialiased);

    for (int x = 0; x < width(); x += 100) {
        for (int y = 0; y < height(); y += 100) {
            painter.save();
            painter.translate(x, y);
            if (transformed) {
                painter.translate(50, 50);
                painter.rotate(60.0);
                painter.scale(0.6, 0.9);
                painter.translate(-50, -50);
            }

            switch (shape) {
            case Line:
                painter.drawLine(rect.bottomLeft(), rect.topRight());
                break;
            case Points:
                painter.drawPoints(polygon);
                break;
            case Polyline:
                painter.drawPolyline(polygon);
                break;
            case Polygon:
                painter.drawPolygon(polygon);
                break;
            case Rect:
                painter.drawRect(rect);
                break;
            case RoundRect:
                painter.drawRoundRect(rect);
                break;
            case Ellipse:
                painter.drawEllipse(rect);
                break;
            case Arc:
                painter.drawArc(rect, alpha, beta);
                break;
            case Chord:
                painter.drawChord(rect, alpha, beta);
                break;
            case Pie:
                painter.drawPie(rect, alpha, beta);
                break;
            case Path:
                painter.drawPath(path);
                break;
            case Text:
                painter.drawText(rect, Qt::AlignCenter, tr("Qt by\nTrolltech"));
                break;
            case Pixmap:
                painter.drawPixmap(10, 10, pixmap);
            }
            painter.restore();
        }
    }
}
