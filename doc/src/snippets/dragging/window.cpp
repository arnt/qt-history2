#include <QtGui>

#include "window.h"

Window::Window(QWidget *parent)
    : QWidget(parent)
{
    QLabel *nameLabel = new QLabel(tr("Comment:"), this);
    commentEdit = new QTextEdit(this);

    QLabel *dragLabel = new QLabel(tr("<p>Drag the icon to a filer "
                                      "window or the desktop background:</p>"),
                                      this);
    iconLabel = new QLabel(this);
    iconPixmap.load(":/images/file.png");
    iconLabel->setPixmap(iconPixmap);

    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(commentEdit, 1, 0, 1, 2);
    grid->addWidget(dragLabel, 2, 0);
    grid->addWidget(iconLabel, 2, 1);

    setWindowTitle(tr("Dragging"));
}

void Window::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton
        && iconLabel->geometry().contains(event->pos())) {

        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        mimeData->setText(commentEdit->toPlainText());
        drag->setMimeData(mimeData);
        drag->setPixmap(iconPixmap);

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
        (void) QMessageBox::information(this, tr("Information About The Drag"),
            actionText, QMessageBox::Ok);
    }
}
