#include <QtGui>

#include "characterwidget.h"

CharacterWidget::CharacterWidget(QWidget *parent)
    : QWidget(parent)
{
    currentKey = -1;
    setMouseTracking(true);
}

void CharacterWidget::updateFont(const QString &fontFamily)
{
    displayFont.setFamily(fontFamily);
    displayFont.setPixelSize(16);
    update();
}

void CharacterWidget::updateStyle(const QString &fontStyle)
{
    QFontDatabase fontDatabase;
    displayFont = fontDatabase.font(displayFont.family(), fontStyle, 12);
    displayFont.setPixelSize(16);
    update();
}

QSize CharacterWidget::sizeHint() const
{
    return QSize(32*24, (65536/32)*24);
}

void CharacterWidget::mouseMoveEvent(QMouseEvent *event)
{
    showToolTip(event->globalPos());
}

void CharacterWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        currentKey = (event->y()/24)*32 + event->x()/24;
        if (QChar(currentKey).category() != QChar::NoCategory)
            emit characterSelected(QString(QChar(currentKey)));
        update();
    }
    else
        QWidget::mousePressEvent(event);
}

void CharacterWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(event->rect(), QBrush(Qt::white));
    painter.setFont(displayFont);

    QRect redrawRect = event->rect();
    int beginRow = redrawRect.top()/24;
    int endRow = redrawRect.bottom()/24;
    int beginColumn = redrawRect.left()/24;
    int endColumn = redrawRect.right()/24;

    painter.setPen(QPen(Qt::gray));
    for (int row = beginRow; row <= endRow; ++row) {
        for (int column = beginColumn; column <= endColumn; ++column) {
            painter.drawRect(column*24, row*24, 24, 24);
        }
    }

    QFontMetrics fontMetrics(displayFont);
    painter.setPen(QPen(Qt::black));
    for (int row = beginRow; row <= endRow; ++row) {

        for (int column = beginColumn; column <= endColumn; ++column) {

            int key = row*32 + column;
            painter.setClipRect(column*24, row*24, 24, 24);

            if (key == currentKey)
                painter.fillRect(column*24, row*24, 24, 24, QBrush(Qt::red));

            painter.drawText(column*24 + 12 - fontMetrics.width(QChar(key))/2,
                             row*24 + 4 + fontMetrics.ascent(),
                             QString(QChar(key)));
        }
    }
}

void CharacterWidget::showToolTip(const QPoint &position)
{
    QPoint widgetPosition = mapFromGlobal(position);
    int key = (widgetPosition.y()/24)*32 + widgetPosition.x()/24;
    QToolTip::showText(position, QString::number(key), this);
}
