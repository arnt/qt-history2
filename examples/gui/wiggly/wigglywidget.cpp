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
    timerId = startTimer(60);
}

void WigglyWidget::paintEvent(QPaintEvent *)
{
    static int sine_table[16] = {
        0, 38, 71, 92, 100, 92, 71, 38,	0, -38, -71, -92, -100, -92, -71, -38
    };

    QFontMetrics fm(font());
    int h = fm.height() * 2;
    int x = (width() - fm.width(text)) / 2;
    int y = (h / 2) + fm.descent();
    QColor color;

    QPainter painter(this);
    for (int i = 0; i < text.length(); ++i) {
        int index = (step + i) % 16;
        color.setHsv((15 - index) * 16, 255, 255);
        painter.setPen(color);
        painter.drawText(x, y - ((sine_table[index] * h) / 800),
                         QString(text[i]));
        x += fm.width(text[i]);
    }
}

void WigglyWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == timerId) {
        step = (step + 1) % 16;
        update();
    } else {
	QWidget::timerEvent(event);
    }
}
