/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>

#include "characterwidget.h"

CharacterWidget::CharacterWidget(QWidget *parent)
    : QWidget(parent)
{
    lastKey = -1;
    setMouseTracking(true);
}

void CharacterWidget::updateFont(const QFont &font)
{
    displayFont.setFamily(font.family());
    displayFont.setPixelSize(16);
    update();
}

void CharacterWidget::updateStyle(const QString &fontStyle)
{
    QFontDatabase fontDatabase;
    const QFont::StyleStrategy oldStrategy = displayFont.styleStrategy();
    displayFont = fontDatabase.font(displayFont.family(), fontStyle, 12);
    displayFont.setPixelSize(16);
    displayFont.setStyleStrategy(oldStrategy);
    update();
}

void CharacterWidget::updateFontMerging(bool enable)
{
    if (enable)
        displayFont.setStyleStrategy(QFont::PreferDefault);
    else
        displayFont.setStyleStrategy(QFont::NoFontMerging);
    update();
}

QSize CharacterWidget::sizeHint() const
{
    return QSize(32*24, (65536/32)*24);
}

void CharacterWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPoint widgetPosition = mapFromGlobal(event->globalPos());
    uint key = (widgetPosition.y()/24)*32 + widgetPosition.x()/24;

    QString text = QString::fromLatin1("<p>Character: <span style=\"font-size: 24pt; font-family: %1\">").arg(displayFont.family())
                  + QChar(key)
                  + QString::fromLatin1("</span><p>Value: 0x")
                  + QString::number(key, 16);
    QToolTip::showText(event->globalPos(), text, this);
}

void CharacterWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        lastKey = (event->y()/24)*32 + event->x()/24;
        if (QChar(lastKey).category() != QChar::NoCategory)
            emit characterSelected(QString(QChar(lastKey)));
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

            if (key == lastKey)
                painter.fillRect(column*24, row*24, 24, 24, QBrush(Qt::red));

            painter.drawText(column*24 + 12 - fontMetrics.width(QChar(key))/2,
                             row*24 + 4 + fontMetrics.ascent(),
                             QString(QChar(key)));
        }
    }
}
