#include <QtGui>

#include "window.h"

Window::Window(QWidget *parent)
    : QWidget(parent)
{
    QLabel *textLabel = new QLabel(tr("Data:"), this);
    textInfo = new QTextBrowser(this);

    QLabel *mimeTypeLabel = new QLabel(tr("MIME types:"), this);
    mimeTypeInfo = new QComboBox(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(textLabel);
    layout->addWidget(textInfo);
    layout->addWidget(mimeTypeLabel);
    layout->addWidget(mimeTypeInfo);

    setAcceptDrops(true);
    setWindowTitle(tr("Drop Events"));
}

void Window::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/plain"))
        event->acceptProposedAction();
}

void Window::dropEvent(QDropEvent *event)
{
    textInfo->setPlainText(event->mimeData()->text());
    mimeTypeInfo->clear();
    mimeTypeInfo->insertStringList(event->mimeData()->formats());

    event->acceptProposedAction();
}
