#include <QtGui>

#include "window.h"

MyWidget::MyWidget(QWidget *parent)
    : QWidget(parent)
{
    QLabel *textLabel = new QLabel(tr("Data:"), this);
    dataLabel = new QLabel(this);
    dataLabel->setFixedSize(200, 200);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(textLabel);
    layout->addWidget(dataLabel);

    setAcceptDrops(true);
    setWindowTitle(tr("Drop Events"));
}

void MyWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasText() || event->mimeData()->hasImage())
        event->acceptProposedAction();
}

void MyWidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasText())
        dataLabel->setText(event->mimeData()->text());
    else if (event->mimeData()->hasImage()) {
        QVariant imageData = event->mimeData()->imageData();
        dataLabel->setPixmap(qvariant_cast<QPixmap>(imageData));
    }
    event->acceptProposedAction();
}

void MyWidget::mousePressEvent(QMouseEvent *event)
{
    QString text = dataLabel->text();
    QPixmap iconPixmap(32, 32);
    iconPixmap.fill(qRgba(255, 0, 0, 127));
    QImage image(100, 100, QImage::Format_RGB32);
    image.fill(qRgb(0, 0, 255));

    if (event->button() == Qt::LeftButton) {

        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        mimeData->setText(text);
        mimeData->setImageData(image);
        drag->setMimeData(mimeData);
        drag->setPixmap(iconPixmap);

        Qt::DropAction dropAction = drag->exec();
        // ...
        event->accept();
    }
    else if (event->button() == Qt::MidButton) {

        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        mimeData->setImageData(image);
        drag->setMimeData(mimeData);
        drag->setPixmap(iconPixmap);

        Qt::DropAction dropAction = drag->exec();
        // ...
        event->accept();
    }
}
