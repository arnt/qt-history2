#include <QtGui>

#include "mainwindow.h"
#include "xmlwriter.h"

MainWindow::MainWindow()
{
    QMenu *fileMenu = new QMenu(tr("&File"));

    QAction *saveAction = fileMenu->addAction(tr("&Save"));
    saveAction->setShortcut(tr("Ctrl+S"));

    QAction *quitAction = fileMenu->addAction(tr("E&xit"));
    quitAction->setShortcut(tr("Ctrl+Q"));

    menuBar()->addMenu(fileMenu);
    editor = new QTextEdit();

    QTextCursor cursor(editor->textCursor());
    cursor.movePosition(QTextCursor::Start);

    int rows = 11;
    int columns = 4;

    QTextTableFormat tableFormat;
    QVector<QTextLength> constraints;
    constraints << QTextLength(QTextLength::PercentageLength, 16);
    constraints << QTextLength(QTextLength::PercentageLength, 28);
    constraints << QTextLength(QTextLength::PercentageLength, 28);
    constraints << QTextLength(QTextLength::PercentageLength, 28);
    tableFormat.setColumnWidthConstraints(constraints);
    QTextTable *table = cursor.insertTable(rows, columns, tableFormat);

    int column;
    int row;
    QTextTableCell cell;
    QTextCursor cellCursor;

    cell = table->cellAt(0, 0);
    cellCursor = cell.firstCursorPosition();
    cellCursor.insertText(tr("Week"));

    for (column = 1; column < columns; ++column) {
        cell = table->cellAt(0, column);
        cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(tr("Team %1").arg(column));
    }

    for (row = 1; row < rows; ++row) {
        cell = table->cellAt(row, 0);
        cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(tr("%1").arg(row));

        for (column = 1; column < columns; ++column) {
            if ((row-1) % 3 == column-1) {
                cell = table->cellAt(row, column);
                QTextCursor cellCursor = cell.firstCursorPosition();
                cellCursor.insertText(tr("On duty"));
            }
        }
    }

    connect(saveAction, SIGNAL(triggered()), this, SLOT(saveFile()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

    setCentralWidget(editor);
    setWindowTitle(tr("Text Document Tables"));
}

void MainWindow::saveFile()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save document as:"), "", tr("XML (*.xml)"));

    if (!fileName.isEmpty()) {
        if (writeXml(fileName))
            setWindowTitle(fileName);
        else
            QMessageBox::warning(this, tr("Warning"),
                tr("Failed to save the document."), QMessageBox::Cancel,
                QMessageBox::NoButton);
    }
}
bool MainWindow::writeXml(const QString &fileName)
{
    XmlWriter documentWriter(editor->document());

    QDomDocument *domDocument = documentWriter.toXml();
    QFile file(fileName);

    if (file.open(QIODevice::WriteOnly)) {
        QTextStream textStream(&file);
        textStream.setEncoding(QTextStream::UnicodeUTF8);
        
        textStream << domDocument->toString(1).toUtf8();
        file.close();
        return true;
    }
    else
        return false;
}
