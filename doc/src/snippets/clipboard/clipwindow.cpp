#include <QtGui>

#include "clipwindow.h"

ClipWindow::ClipWindow(QWidget *parent)
    : QMainWindow(parent)
{
    clipboard = qApp->clipboard();

    QWidget *centralWidget = new QWidget(this);
    QWidget *currentItem = new QWidget(centralWidget);
    QLabel *mimeTypeLabel = new QLabel(tr("MIME types:"), currentItem);
    mimeTypeCombo = new QComboBox(currentItem);
    QLabel *dataLabel = new QLabel(tr("Data:"), currentItem);
    dataInfoLabel = new QLabel("", currentItem);

    previousItems = new QListWidget(centralWidget);

    connect(clipboard, SIGNAL(dataChanged()), this, SLOT(updateClipboard()));

    QVBoxLayout *currentLayout = new QVBoxLayout(currentItem);
    currentLayout->addWidget(mimeTypeLabel);
    currentLayout->addWidget(mimeTypeCombo);
    currentLayout->addWidget(dataLabel);
    currentLayout->addWidget(dataInfoLabel);
    currentLayout->addStretch(1);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->addWidget(currentItem, 1);
    mainLayout->addWidget(previousItems);

    setCentralWidget(centralWidget);
    setWindowTitle(tr("Clipboard"));
}

void ClipWindow::updateClipboard()
{
    QStringList formats = clipboard->mimeData()->formats();
    QByteArray data = clipboard->mimeData()->data(formats[0]);

    mimeTypeCombo->clear();
    mimeTypeCombo->insertStringList(formats);
    dataInfoLabel->setText(tr("%1 bytes").arg(data.size()));

    QListWidgetItem *newItem = new QListWidgetItem(previousItems);
    newItem->setText(tr("%1 (%2 bytes)").arg(formats[0]).arg(data.size()));
}
