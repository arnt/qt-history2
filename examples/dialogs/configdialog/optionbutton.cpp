#include <QtGui>

#include "optionbutton.h"

OptionButton::OptionButton(int width, int height, int page, QWidget *parent)
    : QAbstractButton(parent), height(height), pageNumber(page), width(width)
{
}

void OptionButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    painter.fillRect(event->rect(), QBrush(Qt::white));
    QIcon thisIcon;
    QString thisText;
    QFont font = painter.font();

    if (!(thisIcon = icon()).isNull())
        painter.drawPixmap(4, 4, thisIcon.pixmap(width - 8, height - 8));

    if (!(thisText = text()).isEmpty()) {

        if (isChecked()) {
            font.setBold(true);
            painter.setFont(font);
        }
        painter.drawText(4 + (width-8-QFontMetrics(font).width(thisText))/2,
                         height - 4, thisText);
    }
}

QSize OptionButton::sizeHint() const
{
    return QSize(width, height);
}


int OptionButton::page()
{
    return pageNumber;
}
