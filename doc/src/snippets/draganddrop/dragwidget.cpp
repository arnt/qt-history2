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

void DragWidget::dragMoveEvent(QDragMoveEvent *event)
{
    // Accept all actions, but deal with them separately later.
    event->acceptProposedAction();
}

void DragWidget::dropEvent(QDropEvent *event)
{
    QMenu actionMenu(this);
    QAction *copyAction = 0;
    QAction *moveAction = 0;
    QAction *linkAction = 0;
    QAction *ignoreAction = 0;
    if (event->possibleActions() & QDrag::CopyAction)
        copyAction = actionMenu.addAction(tr("Copy"));
    if (event->possibleActions() & QDrag::MoveAction)
        moveAction = actionMenu.addAction(tr("Move"));
    if (event->possibleActions() & QDrag::LinkAction)
        linkAction = actionMenu.addAction(tr("Link"));
    if (event->possibleActions() & QDrag::IgnoreAction)
        ignoreAction = actionMenu.addAction(tr("Ignore"));

    QAction *result = actionMenu.exec(QCursor::pos());

    if (copyAction && result == copyAction) {
        event->setDropAction(QDrag::CopyAction);
        emit dragResult(tr("The data was copied here."));
    } else if (moveAction && result == moveAction) {
        event->setDropAction(QDrag::MoveAction);
        emit dragResult(tr("The data was moved here."));
    } else if (linkAction && result == linkAction) {
        event->setDropAction(QDrag::LinkAction);
        emit dragResult(tr("The data was linked here."));
    } else {
        event->setDropAction(QDrag::IgnoreAction);
        emit dragResult(tr("The data was ignored."));
        return;
    }

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

    QDrag::DropAction dropAction = drag->start(QDrag::CopyAction
        | QDrag::MoveAction | QDrag::LinkAction | QDrag::IgnoreAction);

    switch (dropAction) {
        case QDrag::CopyAction:
            emit dragResult(tr("The text was copied."));
            break;
        case QDrag::MoveAction:
            emit dragResult(tr("The text was moved."));
            break;
        case QDrag::LinkAction:
            emit dragResult(tr("The text was linked."));
            break;
        case QDrag::IgnoreAction:
            emit dragResult(tr("The drag was ignored."));
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
