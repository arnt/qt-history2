#include <QTimer>

#include "wigglywidget.h"

WigglyWidget::WigglyWidget(QWidget *parent)
    : QWidget(parent)
{
    _0to15 = 0;
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(animate()));
    timer->start(60);
}


void WigglyWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (rect().contains(event->pos()))
        emit clicked();
}


void WigglyWidget::paintEvent(QPaintEvent *)
{
    static int sine_table[16] = {
        0, 38, 71, 92, 100, 92, 71, 38,	0, -38, -71, -92, -100, -92, -71, -38};

    if (_text.isEmpty())
        return;

    QFont font(font());
    font.setPointSize(font.pointSize() + 20);
    QFontMetrics fm(font);
    int h = fm.height() * 2;
    int x = (width() - fm.width(_text)) / 2;
    int y = (h / 2) + fm.descent();
    QColor color;
    QPainter painter;
    painter.begin(this);
    painter.setFont(font);
    for (int i = 0; i < _text.length(); ++i) {
        int index = (_0to15 + i) & 15;
        color.setHsv((15 - index) * 16, 255, 255);
        painter.setPen(color);
        painter.drawText(x, y - ((sine_table[index] * h) / 800), _text.mid(i, 1));
        x += fm.width(_text[i]);
    }
    painter.end();
}


void WigglyWidget::animate()
{
    _0to15 = (_0to15 + 1) & 15;
    update();
}

