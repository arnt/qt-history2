#include <QtGui>

#include "wigglywidget.h"

WigglyWidget::WigglyWidget(QWidget *parent)
    : QWidget(parent)
{
    setBackgroundRole(QPalette::Midlight);

    QFont newFont = font();
    newFont.setPointSize(newFont.pointSize() + 20);
    setFont(newFont);

    step = 0;
    timer.start(60, this);
}

void WigglyWidget::paintEvent(QPaintEvent * /* event */)
{
    static const int sine_table[16] = {
        0, 38, 71, 92, 100, 92, 71, 38,	0, -38, -71, -92, -100, -92, -71, -38
    };

    QFontMetrics metrics(font());
    int h = metrics.height() * 2;
    int x = (width() - metrics.width(text)) / 2;
    int y = (h / 2) + metrics.descent();
    QColor color;

    QPainter painter(this);
    for (int i = 0; i < text.size(); ++i) {
        int index = (step + i) % 16;
        color.setHsv((15 - index) * 16, 255, 255);
        painter.setPen(color);
        painter.drawText(x, y - ((sine_table[index] * h) / 800),
                         QString(text[i]));
        x += metrics.width(text[i]);
    }
}

void WigglyWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == timer.timerId()) {
        step = (step + 1) % 16;
        update();
    } else {
	QWidget::timerEvent(event);
    }
}
