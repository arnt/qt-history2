#include <QtGui>
#include <math.h>

#include "window.h"

Window::Window(QWidget *parent)
    : QWidget(parent)
{
    text = QString("Support for text rendering and layout in Qt 4 has been "
                   "redesigned around a system that allows textual content to "
                   "be represented in a more flexible way than was possible "
                   "with Qt 3. Qt 4 also provides a more convenient "
                   "programming interface for editing documents. These "
                   "improvements are made available through a reimplementation "
                   "of the existing text rendering engine, and the "
                   "introduction of several new classes. "
                   "See the relevant module overview for a detailed discussion "
                   "of this framework. The following sections provide a brief "
                   "overview of the main concepts behind Scribe.");

    setWindowTitle(tr("Plain Text Layout"));
}

void Window::paintEvent(QPaintEvent *event)
{
    QTextLayout textLayout(text, font);
    qreal margin = 10;
    qreal radius = qMin(width()/2.0, height()/2.0) - margin;
    QFontMetrics fm(font);

    qreal lineHeight = fm.height();
    qreal y = 0;

    textLayout.beginLayout();

    while (1) {
        // create a new line 
        QTextLine line = textLayout.createLine();
        if (!line.isValid())
            break;

        qreal x1 = qMax(0.0, pow(pow(radius,2)-pow(radius-y,2), 0.5));
        qreal x2 = qMax(0.0, pow(pow(radius,2)-pow(radius-(y+lineHeight),2), 0.5));
        qreal x = qMax(x1, x2) + margin;
        qreal lineWidth = (width() - margin) - x;

        line.setLineWidth(lineWidth);
        line.setPosition(QPointF(x, margin+y));
        y += line.height();
    }

    textLayout.endLayout();

    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), Qt::white);
    painter.setBrush(QBrush(Qt::black));
    painter.setPen(QPen(Qt::black));
    textLayout.draw(&painter, QPoint(0,0));

    painter.setBrush(QBrush(QColor("#a6ce39")));
    painter.setPen(QPen(Qt::black));
    painter.drawEllipse(QRectF(-radius, margin, 2*radius, 2*radius));
    painter.end();
}
