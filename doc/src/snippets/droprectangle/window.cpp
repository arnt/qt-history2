#include <QtGui>

#include "window.h"

Window::Window(QWidget *parent)
    : QWidget(parent)
{
    QLabel *textLabel = new QLabel(tr("Data:"), this);
    textInfo = new QTextBrowser(this);

    QLabel *mimeTypeLabel = new QLabel(tr("MIME types:"), this);
    mimeTypeInfo = new QComboBox(this);

    dropFrame = new QFrame(this);
    dropFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    QLabel *dropLabel = new QLabel(tr("Drop items here"), dropFrame);
    dropLabel->setAlignment(Qt::AlignHCenter);

    QVBoxLayout *dropFrameLayout = new QVBoxLayout(dropFrame);
    dropFrameLayout->addWidget(dropLabel);

    QHBoxLayout *dropLayout = new QHBoxLayout;
    dropLayout->addStretch(0);
    dropLayout->addWidget(dropFrame);
    dropLayout->addStretch(0);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(textLabel);
    mainLayout->addWidget(textInfo);
    mainLayout->addWidget(mimeTypeLabel);
    mainLayout->addWidget(mimeTypeInfo);
    mainLayout->addSpacing(32);
    mainLayout->addLayout(dropLayout);

    setAcceptDrops(true);
    setWindowTitle(tr("Drop Rectangle"));
}

void Window::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("text/plain")
        && event->answerRect().intersects(dropFrame->geometry()))

        event->acceptProposedAction();
}

void Window::dropEvent(QDropEvent *event)
{
    textInfo->setPlainText(event->mimeData()->text());
    mimeTypeInfo->clear();
    mimeTypeInfo->insertStringList(event->mimeData()->formats());

    event->acceptProposedAction();
}
