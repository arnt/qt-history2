#include <QtGui>

#include "mainwindow.h"
#include "previewform.h"

MainWindow::MainWindow()
{
    textEdit = new QTextEdit;
    textEdit->setLineWrapMode(QTextEdit::NoWrap);
    setCentralWidget(textEdit);

    previewForm = new PreviewForm(this);

    createActions();
    createMenus();

    setWindowTitle(tr("Codecs"));
    resize(500, 400);
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QFile::ReadOnly)) {
            QMessageBox::warning(this, tr("Codecs"),
                                 tr("Cannot read file %1:\n%2")
                                 .arg(fileName)
                                 .arg(file.errorString()));
            return;
        }

        QByteArray data = file.readAll();

        previewForm->setEncodedData(data);
        if (previewForm->exec())
            textEdit->setPlainText(previewForm->decodedString());
    }
}

void MainWindow::save()
{
    QString fileName = QFileDialog::getSaveFileName(this);
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QFile::WriteOnly)) {
            QMessageBox::warning(this, tr("Codecs"),
                                 tr("Cannot write file %1:\n%2")
                                 .arg(fileName)
                                 .arg(file.errorString()));
            return;
        }

        QAction *action = qobject_cast<QAction *>(sender());
        QByteArray codecName = action->iconText().toAscii();
	QTextCodec *codec = QTextCodec::codecForName(codecName);
        file.write(codec->fromUnicode(textEdit->toPlainText()));
    }
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Codecs"),
            tr("The <b>Codecs</b> example demonstrates how to read and write "
               "files using various encodings."));
}

void MainWindow::aboutToShowSaveAsMenu()
{
    QString currentText = textEdit->toPlainText();

    foreach (QAction *action, saveAsActs) {
        QByteArray codecName = action->iconText().toAscii();
        QTextCodec *codec = QTextCodec::codecForName(codecName);
        action->setVisible(codec && codec->canEncode(currentText));
    }
}

void MainWindow::createActions()
{
    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    foreach (int mib, QTextCodec::availableMibs()) {
        QTextCodec *codec = QTextCodec::codecForMib(mib);
        QString text = tr("%1...").arg(QString(codec->name()));

        QAction *action = new QAction(text, this);
        connect(action, SIGNAL(triggered()), this, SLOT(save()));
        saveAsActs.append(action);
    }

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    aboutAct = new QAction(tr("&About"), this);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::createMenus()
{
    saveAsMenu = new QMenu(tr("&Save As"));
    foreach (QAction *action, saveAsActs)
        saveAsMenu->addAction(action);
    connect(saveAsMenu, SIGNAL(aboutToShow()),
            this, SLOT(aboutToShowSaveAsMenu()));

    fileMenu = new QMenu(tr("&File"));
    fileMenu->addAction(openAct);
    fileMenu->addMenu(saveAsMenu);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    helpMenu = new QMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);

    menuBar()->addMenu(fileMenu);
    menuBar()->addSeparator();
    menuBar()->addMenu(helpMenu);
}
