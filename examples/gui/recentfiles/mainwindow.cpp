#include <QtGui>

#include "mainwindow.h"

MainWindow::MainWindow()
{
    setAttribute(Qt::WA_DeleteOnClose);

    textEdit = new QTextEdit(this);
    setCentralWidget(textEdit);

    createActions();
    createMenus();
    (void)statusBar();

    readSettings();

    setWindowTitle(tr("Recent Files"));
}

void MainWindow::closeEvent(QCloseEvent *)
{
    writeSettings();
}

void MainWindow::newFile()
{
    MainWindow *other = new MainWindow();
    other->show();
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
        loadFile(fileName);
}

void MainWindow::save()
{
    if (curFile.isEmpty())
        saveAs();
    else
        saveFile(curFile);
}

void MainWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this);
    if (fileName.isEmpty())
        return;

    if (QFile::exists(fileName)) {
        int ret = QMessageBox::warning(this, tr("Recent Files"),
                     tr("File %1 already exists.\n"
                        "Do you want to overwrite it?")
                     .arg(QDir::convertSeparators(fileName)),
                     QMessageBox::Yes | QMessageBox::Default,
                     QMessageBox::No | QMessageBox::Escape);
        if (ret == QMessageBox::No)
            return;
    }
    if (!fileName.isEmpty())
        saveFile(fileName);
}

void MainWindow::about()
{
   QMessageBox::about(this, tr("About Recent Files"),
            tr("The <b>Recent Files</b> example demonstrates how to provide a "
               "recently used file menu in a Qt application."));
}

void MainWindow::createActions()
{
    newAct = new QAction(tr("&New"), this);
    newAct->setShortcut(tr("Ctrl+N"));
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, SIGNAL(activated()), this, SLOT(newFile()));

    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(activated()), this, SLOT(open()));

    saveAct = new QAction(tr("&Save"), this);
    saveAct->setShortcut(tr("Ctrl+S"));
    saveAct->setStatusTip(tr("Save the spreadsheet to disk"));
    connect(saveAct, SIGNAL(activated()), this, SLOT(save()));

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setStatusTip(tr("Save the spreadsheet under a new name"));
    connect(saveAsAct, SIGNAL(activated()), this, SLOT(saveAs()));

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
    }

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(activated()), qApp, SLOT(quit()));

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(activated()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(activated()), qApp, SLOT(aboutQt()));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();

    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void MainWindow::readSettings()
{
#if 0
    // TODO: enable settings code when the new QSettings is available
    QSettings settings("doc.trolltech.com", "Recent Files");
    recentFiles = settings.value("recentFiles").toStringList();
#endif
}

void MainWindow::writeSettings()
{
#if 0
    // TODO: enable settings code when the new QSettings is available
    QSettings settings("doc.trolltech.com", "Recent Files");
    settings.setValue("recentFiles", recentFiles);
#endif
}

void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this, tr("Recent Files"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    textEdit->setPlainText(in.read());
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->message(tr("File loaded"), 2000);
}

void MainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        QMessageBox::warning(this, tr("Recent Files"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << textEdit->plainText();
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->message(tr("File saved"), 2000);
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    if (curFile.isEmpty())
        setWindowTitle(tr("Recent Files"));
    else
        setWindowTitle(tr("%1 - %2").arg(strippedName(curFile))
                                    .arg(tr("Recent Files")));
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}
