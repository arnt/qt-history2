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

    QMenu *insertMenu = new QMenu(tr("&Insert"));

    QAction *calendarAction = insertMenu->addAction(tr("&Calendar"));
    calendarAction->setShortcut(tr("Ctrl+I"));

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(insertMenu);

    editor = new QTextEdit(this);

    connect(saveAction, SIGNAL(triggered()), this, SLOT(saveFile()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(calendarAction, SIGNAL(triggered()), this, SLOT(insertCalendar()));

    setCentralWidget(editor);
    setWindowTitle(tr("Text Document Writer"));
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

void MainWindow::insertCalendar()
{
    QTextCursor cursor(editor->textCursor());
    cursor.movePosition(QTextCursor::Start); 

    QTextCharFormat format(cursor.charFormat());
    format.setFontFamily("Courier");
    
    QTextCharFormat boldFormat = format;
    boldFormat.setFontWeight(QFont::Bold);

    cursor.insertBlock();
    cursor.insertText(" ", boldFormat);

    QDate date = QDate::currentDate();
    int year = date.year(), month = date.month();

    for (int weekDay = 1; weekDay <= 7; ++weekDay) {
        cursor.insertText(QString("%1 ").arg(QDate::shortDayName(weekDay), 3),
            boldFormat);
    }

    cursor.insertBlock();
    cursor.insertText(" ", format);

    for (int column = 1; column < QDate(year, month, 1).dayOfWeek(); ++column) {
        cursor.insertText("    ", format);
    }

    for (int day = 1; day <= date.daysInMonth(); ++day) {
        int weekDay = QDate(year, month, day).dayOfWeek();

        if (QDate(year, month, day) == date)
            cursor.insertText(QString("%1 ").arg(day, 3), boldFormat);
        else
            cursor.insertText(QString("%1 ").arg(day, 3), format);

        if (weekDay == 7) {
            cursor.insertBlock();
            cursor.insertText(" ", format);
        }
    }
}

bool MainWindow::writeXml(const QString &fileName)
{
    XmlWriter documentWriter(editor->document());

    QDomDocument *domDocument = documentWriter.toXml();
    QFile file(fileName);

    if (file.open(QFile::WriteOnly)) {
        QTextStream textStream(&file);
        textStream.setEncoding(QTextStream::UnicodeUTF8);
        
        textStream << domDocument->toString(1).toUtf8();
        file.close();
        return true;
    }
    else
        return false;
}
