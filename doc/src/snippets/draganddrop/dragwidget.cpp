#include <QtGui>

#include "dragwidget.h"

DragWidget::DragWidget(QWidget *parent)
    : QFrame(parent)
{
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    dragDropLabel = new QLabel("", this);
    dragDropLabel->setAlignment(Qt::AlignHCenter);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addStretch(0);
    layout->addWidget(dragDropLabel);
    layout->addStretch(0);

    setAcceptDrops(true);
}

// Accept all actions, but deal with them separately later.
void DragWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void DragWidget::dropEvent(QDropEvent *event)
{
    if (event->source() == this && event->possibleActions() & Qt::MoveAction)
        return;

    if (event->possibleActions() == Qt::MoveAction) {
        event->acceptProposedAction();
        // Process the data from the event.
        emit dragResult(tr("The data was moved here."));
    } else if (event->possibleActions() == Qt::CopyAction) {
        event->acceptProposedAction();
        // Process the data from the event.
        emit dragResult(tr("The data was copied here."));
    } else {
        // Ignore the drop.
        return;
    }
    // End of quote

    emit mimeTypes(event->mimeData()->formats());
    setData(event->mimeData()->formats()[0],
            event->mimeData()->data(event->mimeData()->formats()[0]));
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

    Qt::DropAction dropAction = drag->start(Qt::CopyAction | Qt::MoveAction);

    switch (dropAction) {
        case Qt::CopyAction:
            emit dragResult(tr("The text was copied."));
            break;
        case Qt::MoveAction:
            emit dragResult(tr("The text was moved."));
            break;
        default:
            emit dragResult(tr("Unknown action."));
            break;
    }
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
