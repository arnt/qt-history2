#include <QtGui>

#include "dragwidget.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QFrame *centralWidget = new QFrame(this);

    QLabel *mimeTypeLabel = new QLabel(tr("MIME types:"), centralWidget);
    mimeTypeCombo = new QComboBox(centralWidget);

    dragWidget = new DragWidget(centralWidget);

    connect(dragWidget, SIGNAL(mimeTypes(const QStringList &)),
            this, SLOT(setMimeTypes(const QStringList &)));

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(mimeTypeLabel);
    mainLayout->addWidget(mimeTypeCombo);
    mainLayout->addSpacing(32);
    mainLayout->addWidget(dragWidget);

    statusBar();
    dragWidget->setData(QString("text/plain"), QByteArray("Hello world"));
    setCentralWidget(centralWidget);
    setWindowTitle(tr("Dragging"));
}

void MainWindow::setDragResult(const QString &actionText)
{
    statusBar()->message(actionText);
}

void MainWindow::setMimeTypes(const QStringList &types)
{
    mimeTypeCombo->clear();
    mimeTypeCombo->insertStringList(types);
}
