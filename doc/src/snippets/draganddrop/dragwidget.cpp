#include <QtGui>

#include "dragwidget.h"

DragWidget::DragWidget(QWidget *parent)
    : QFrame(parent)
{
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    dragDropLabel = new QLabel(tr("Drop items here"), this);
    dragDropLabel->setAlignment(Qt::AlignHCenter);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addStretch(0);
    layout->addWidget(dragDropLabel);
    layout->addStretch(0);

    setAcceptDrops(true);
}

void DragWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void DragWidget::dropEvent(QDropEvent *event)
{
    emit mimeTypes(event->mimeData()->formats());
    setData(event->mimeData()->formats()[0], event->mimeData()->data(mimeType));
    event->acceptProposedAction();
}

void DragWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        dragStartPosition = event->pos();
}

void DragWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - dragStartPosition).manhattanLength()
         < QApplication::startDragDistance())
        return;

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    mimeData->setData(mimeType, data);
    drag->setMimeData(mimeData);

    QDrag::DropAction dropAction = drag->start();

    QString actionText;
    switch (dropAction) {
        case QDrag::CopyAction:
            actionText = tr("The text was copied.");
            break;
        case QDrag::MoveAction:
            actionText = tr("The text was moved.");
            break;
        case QDrag::LinkAction:
            actionText = tr("The text was linked.");
            break;
        case QDrag::IgnoreAction:
            actionText = tr("The drag was ignored.");
            break;
        default:
            actionText = tr("Unknown action.");
            break;
    }
    emit dragResult(actionText);
}

void DragWidget::setData(const QString &mimetype, const QByteArray &newData)
{
    mimeType = mimetype;
    data = QByteArray(newData);

    dragDropLabel->setText(tr("%1 bytes").arg(data.size()));

    QStringList formats;
    formats << mimetype;
    emit mimeTypes(formats);
}
