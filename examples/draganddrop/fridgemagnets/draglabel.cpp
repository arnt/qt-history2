#include <QtGui>

#include "draglabel.h"

DragLabel::DragLabel(const QString &text, QWidget *parent)
    : QLabel(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    QFontMetrics fm(font());
    QSize size = fm.size(Qt::TextSingleLine, text);

    QImage image(size.width() + 12, size.height() + 12, QImage::Format_ARGB32);
    image.fill(qRgba(255, 255, 255, 0));

    QPainter painter;
    painter.begin(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(Qt::white);
    painter.drawRoundRect(0, 0, image.width()-1, image.height()-1, 25, 25);

    painter.setFont(font());
    painter.setPen(Qt::black);
    painter.setBrush(Qt::black);
    painter.drawText(QRect(QPoint(6, 6), size), Qt::AlignCenter, text);
    painter.end();

    setPixmap(QPixmap::fromImage(image));
    labelText = text;
}

void DragLabel::mousePressEvent(QMouseEvent *event)
{
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    dataStream << labelText << QPoint(event->pos() - rect().topLeft());

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-fridgemagnet", itemData);
    mimeData->setText(labelText);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setHotSpot(event->pos() - rect().topLeft());
    drag->setPixmap(*pixmap());

    hide();

    if (drag->start(Qt::MoveAction) == Qt::MoveAction)
        close();
    else
        show();
}
