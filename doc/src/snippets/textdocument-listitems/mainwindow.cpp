#include <QtGui>

#include "mainwindow.h"

MainWindow::MainWindow()
{
    QMenu *fileMenu = new QMenu(tr("&File"));

    fileMenu->addAction(tr("E&xit"), this, SLOT(close()),
        QKeySequence(tr("Ctrl+Q", "File|Exit")));

    QMenu *showMenu = new QMenu(tr("&Show"));
    showMenu->addAction(tr("&List Items"), this, SLOT(showListItems()));

    QMenu *insertMenu = new QMenu(tr("&Insert"));

    insertMenu->addAction(tr("&List"), this, SLOT(insertList()),
        QKeySequence(tr("Ctrl+L", "Insert|List")));

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(showMenu);
    menuBar()->addMenu(insertMenu);

    editor = new QTextEdit(this);
    document = new QTextDocument(this);
    editor->setDocument(document);

    setCentralWidget(editor);
    setWindowTitle(tr("Text Document List Items"));
}

void MainWindow::showListItems()
{
    QTextCursor cursor = editor->textCursor();
    QTextList *list = cursor.currentList();

    if (!list)
        return;

    cursor.beginEditBlock();
    for (int index = 0; index < list->count(); ++index) {
        QTextBlock listItem = list->item(index);
        QTextBlockFormat newBlockFormat = listItem.blockFormat();
        newBlockFormat.setBackgroundColor(Qt::lightGray);
        QTextCursor itemCursor = cursor;
        itemCursor.setPosition(listItem.position());
        //itemCursor.movePosition(QTextCursor::StartOfBlock);
        itemCursor.movePosition(QTextCursor::EndOfBlock,
                                QTextCursor::KeepAnchor);
        itemCursor.setBlockFormat(newBlockFormat);
        /*
        ...
        */
    }
    cursor.endEditBlock();
}

void MainWindow::insertList()
{
    QTextCursor cursor = editor->textCursor();
    cursor.beginEditBlock();

    QTextList *list = cursor.currentList();
    QTextListFormat listFormat;
    if (list)
        listFormat = list->format();

    listFormat.setStyle(QTextListFormat::ListDisc);
    listFormat.setIndent(listFormat.indent() + 1);
    cursor.insertList(listFormat);

    cursor.endEditBlock();
}
