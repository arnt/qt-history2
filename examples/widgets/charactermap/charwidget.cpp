#include <QtGui>

#include "charwidget.h"

static unsigned char cursorImage[] =
{
0x00, 0x00, 0x00, 0x00,            //"................................",
0x00, 0x00, 0x00, 0x00,            //"................................",
0x00, 0x00, 0x00, 0x00,            //"................................",
0x00, 0x00, 0x00, 0x00,            //"................................",
0x00, 0x80, 0x01, 0x00,            //"...............++...............",
0x00, 0x40, 0x02, 0x00,            //"..............+..+..............",
0x00, 0x70, 0x0e, 0x00,            //"............+++..+++............",
0x00, 0x48, 0x12, 0x00,            //"...........+..+..+..+...........",
0x00, 0x48, 0x12, 0x00,            //"...........+..+..+..+...........",
0x00, 0x48, 0x12, 0x00,            //"...........+..+..+..+...........",
0x00, 0x48, 0x72, 0x00,            //"...........+..+..+..+++.........",
0x00, 0x48, 0x92, 0x00,            //"...........+..+..+..+..+........",
0x00, 0x48, 0x92, 0x00,            //"...........+..+..+..+..+........",
0x00, 0x4e, 0x92, 0x00,            //".........+++..+..+..+..+........",
0x00, 0x49, 0x92, 0x00,            //"........+..+..+..+..+..+........",
0x00, 0x49, 0x92, 0x00,            //"........+..+..+..+..+..+........",
0x00, 0x49, 0x92, 0x00,            //"........+..+..+..+..+..+........",
0x00, 0x09, 0x90, 0x00,            //"........+..+........+..+........",
0x00, 0x09, 0x80, 0x00,            //"........+..+...........+........",
0x00, 0x09, 0x80, 0x00,            //"........+..+...........+........",
0x00, 0x09, 0x80, 0x00,            //"........+..+...........+........",
0x00, 0x01, 0x80, 0x00,            //"........+..............+........",
0x00, 0x01, 0x40, 0x00,            //"........+.............+.........",
0x00, 0x02, 0x40, 0x00,            //".........+............+.........",
0x00, 0x02, 0x20, 0x00,            //".........+...........+..........",
0x00, 0x04, 0x20, 0x00,            //"..........+..........+..........",
0x00, 0x08, 0x10, 0x00,            //"...........+........+...........",
0x00, 0xf0, 0x0f, 0x00,            //"............++++++++............",
0x00, 0x00, 0x00, 0x00,            //"................................",
0x00, 0x00, 0x00, 0x00,            //"................................",
0x00, 0x00, 0x00, 0x00,            //"................................",
0x00, 0x00, 0x00, 0x00             //"................................"
};

static unsigned char cursorMask[] =
{
0x00, 0x00, 0x00, 0x00,            //"................................",
0x00, 0x00, 0x00, 0x00,            //"................................",
0x00, 0x00, 0x00, 0x00,            //"................................",
0x00, 0x00, 0x00, 0x00,            //"................................",
0x00, 0x80, 0x01, 0x00,            //"...............++...............",
0x00, 0xc0, 0x03, 0x00,            //"..............++++..............",
0x00, 0xf0, 0x0f, 0x00,            //"............++++++++............",
0x00, 0xf8, 0x1f, 0x00,            //"...........++++++++++...........",
0x00, 0xf8, 0x1f, 0x00,            //"...........++++++++++...........",
0x00, 0xf8, 0x1f, 0x00,            //"...........++++++++++...........",
0x00, 0xf8, 0x7f, 0x00,            //"...........++++++++++++.........",
0x00, 0xf8, 0xff, 0x00,            //"...........+++++++++++++........",
0x00, 0xf8, 0xff, 0x00,            //"...........+++++++++++++........",
0x00, 0xfe, 0xff, 0x00,            //".........+++++++++++++++........",
0x00, 0xff, 0xff, 0x00,            //"........++++++++++++++++........",
0x00, 0xff, 0xff, 0x00,            //"........++++++++++++++++........",
0x00, 0xff, 0xff, 0x00,            //"........++++++++++++++++........",
0x00, 0xff, 0xff, 0x00,            //"........++++++++++++++++........",
0x00, 0xff, 0xff, 0x00,            //"........++++++++++++++++........",
0x00, 0xff, 0xff, 0x00,            //"........++++++++++++++++........",
0x00, 0xff, 0xff, 0x00,            //"........++++++++++++++++........",
0x00, 0xff, 0xff, 0x00,            //"........++++++++++++++++........",
0x00, 0xff, 0x7f, 0x00,            //"........+++++++++++++++.........",
0x00, 0xfe, 0x7f, 0x00,            //".........++++++++++++++.........",
0x00, 0xfe, 0x3f, 0x00,            //".........+++++++++++++..........",
0x00, 0xfc, 0x3f, 0x00,            //"..........++++++++++++..........",
0x00, 0xf8, 0x1f, 0x00,            //"...........++++++++++...........",
0x00, 0xf0, 0x0f, 0x00,            //"............++++++++............",
0x00, 0x00, 0x00, 0x00,            //"................................",
0x00, 0x00, 0x00, 0x00,            //"................................",
0x00, 0x00, 0x00, 0x00,            //"................................",
0x00, 0x00, 0x00, 0x00             //"................................"
};

CharView::CharView(QWidget *parent)
    : QWidgetView(parent)
{
    dragging = false;
    dragCursor = QCursor(QBitmap(32, 32, cursorImage, true),
                         QBitmap( 32, 32, cursorMask, true), -1, -1 );
    setMouseTracking(true);
}

void CharView::mousePressEvent(QMouseEvent *event)
{
    QPoint position;

    switch (event->button()) {
        case Qt::MidButton:
            dragPosition = event->pos();
            dragging = true;
            widget()->setCursor(dragCursor);
            break;
        default:
            QWidgetView::mousePressEvent(event);
    }
}

void CharView::mouseMoveEvent(QMouseEvent *event)
{
    if (dragging) {
        QPoint position = event->pos();
        int dx = position.x() - dragPosition.x();
        int dy = position.y() - dragPosition.y();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - dx);
        verticalScrollBar()->setValue(verticalScrollBar()->value() - dy);
        dragPosition = position;
    }
    else {
        static_cast<CharWidget *>(widget())->showToolTip(event->globalPos());
        QWidgetView::mouseMoveEvent(event);
    }
}

void CharView::mouseReleaseEvent(QMouseEvent *event)
{
    if (dragging) {
        dragging = false;
        widget()->unsetCursor();
    }
    else
        QWidgetView::mouseReleaseEvent(event);
}


CharWidget::CharWidget(QWidget *parent)
    : QWidget(parent)
{
    currentKey = -1;
    setMouseTracking(true);
}

void CharWidget::updateFont(const QString &fontFamily)
{
    displayFont.setFamily(fontFamily);
    displayFont.setPixelSize(16);
    update();
}

void CharWidget::updateStyle(const QString &fontStyle)
{
    QFontDatabase fontDatabase;
    displayFont = fontDatabase.font(displayFont.family(), fontStyle, 12);
    displayFont.setPixelSize(16);
    update();
}

QSize CharWidget::sizeHint() const
{
    return QSize(32*24, (65536/32)*24);
}

void CharWidget::mousePressEvent(QMouseEvent *event)
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

void CharWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(event->rect(), QBrush(Qt::white));
    painter.setPen(QPen(Qt::black));
    painter.setFont(displayFont);

    QRect redrawRect = event->rect();
    int beginRow = redrawRect.top()/24;
    int endRow = redrawRect.bottom()/24;
    int beginColumn = redrawRect.left()/24;
    int endColumn = redrawRect.right()/24;

    for (int row = beginRow; row <= endRow; ++row) {

        for (int column = beginColumn; column <= endColumn; ++column) {

            int key = row*32 + column;

            QFontMetrics fontMetrics(displayFont);

            painter.setClipRect(column*24, row*24, 24, 24);

            if (key == currentKey)
                painter.fillRect(column*24, row*24, 24, 24, QBrush(Qt::red));

            painter.drawRect(column*24, row*24, 24, 24);
            painter.drawText(column*24 + 12 - fontMetrics.width(QChar(key))/2,
                             row*24 + 4 + fontMetrics.ascent(),
                             QString(QChar(key)));
        }
    }
}

void CharWidget::showToolTip(const QPoint &position)
{
    QPoint widgetPosition = mapFromGlobal(position);
    int key = (widgetPosition.y()/24)*32 + widgetPosition.x()/24;
    QToolTip::showText(position, QString::number(key), this);
}
