#include <QtGui>

#include "mainwindow.h"

MainWindow::MainWindow()
{
    QMenu *fileMenu = new QMenu(tr("&File"));

    fileMenu->addAction(tr("&Open"), this, SLOT(openFile()),
                        QKeySequence(tr("Ctrl+O", "File|Open")));

    printAction = fileMenu->addAction(tr("&Print"), this, SLOT(printFile()));
    printAction->setEnabled(false);

    fileMenu->addAction(tr("E&xit"), this, SLOT(close()),
                        QKeySequence(tr("Ctrl+Q", "File|Exit")));

    menuBar()->addMenu(fileMenu);

    editor = new QTextEdit(this);
    document = new QTextDocument(this);
    editor->setDocument(document);

    connect(editor, SIGNAL(selectionChanged()), this, SLOT(updateMenus()));

    setCentralWidget(editor);
    setWindowTitle(tr("Text Document Writer"));
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open file"), currentFile, "HTML files (*.html);;Text files (*.txt)");
    
    if (!fileName.isEmpty()) {
        QFileInfo info(fileName);
        if (info.completeSuffix() == "html") {
            QFile file(fileName);
            
            if (file.open(QIODevice::ReadOnly)) {
                editor->setHtml(file.readAll());
                file.close();
                currentFile = fileName;
            }
        } else if (info.completeSuffix() == "txt") {
            QFile file(fileName);
            
            if (file.open(QIODevice::ReadOnly)) {
                editor->setPlainText(file.readAll());
                file.close();
                currentFile = fileName;
            }
        }
        printAction->setEnabled(true);
    }
}

void MainWindow::printFile()
{
    QTextDocument *document = editor->document();

    QPrinter printer;

    QPrintDialog *dlg = new QPrintDialog(&printer, this);

    if (dlg->exec() != QDialog::Accepted)
        return;

    document->print(&printer);
}
