#include <QtGui>

#include "mainwindow.h"

MainWindow::MainWindow()
{
    QMenu *fileMenu = new QMenu(tr("&File"));

    fileMenu->addAction(tr("E&xit"), this, SLOT(close()),
        QKeySequence(tr("Ctrl+Q", "File|Exit")));

    QMenu *editMenu = new QMenu(tr("&Edit"));

    cutAction = editMenu->addAction(tr("Cu&t"), this, SLOT(cutSelection()),
        QKeySequence(tr("Ctrl+X", "Edit|Cut")));
    copyAction = editMenu->addAction(tr("&Copy"), this, SLOT(copySelection()),
        QKeySequence(tr("Ctrl+C", "Edit|Copy")));
    pasteAction = editMenu->addAction(tr("&Paste"), this,
        SLOT(pasteSelection()), QKeySequence(tr("Ctrl+V", "Edit|Paste")));

    QMenu *selectMenu = new QMenu(tr("&Select"));
    selectMenu->addAction(tr("&Word"), this, SLOT(selectWord()));
    selectMenu->addAction(tr("&Line"), this, SLOT(selectLine()));
    selectMenu->addAction(tr("&Block"), this, SLOT(selectBlock()));
    selectMenu->addAction(tr("&Frame"), this, SLOT(selectFrame()));

    QMenu *insertMenu = new QMenu(tr("&Insert"));

    insertMenu->addAction(tr("&List"), this, SLOT(insertList()),
        QKeySequence(tr("Ctrl+L", "Insert|List")));

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(editMenu);
    menuBar()->addMenu(selectMenu);
    menuBar()->addMenu(insertMenu);

    editor = new QTextEdit(this);
    document = new QTextDocument(this);
    editor->setDocument(document);

    connect(editor, SIGNAL(selectionChanged()), this, SLOT(updateMenus()));

    updateMenus();

    setCentralWidget(editor);
    setWindowTitle(tr("Text Document Writer"));
}

void MainWindow::cutSelection()
{
    QTextCursor cursor = editor->textCursor();
    if (cursor.hasSelection()) {
        selection = cursor.selection();
        cursor.removeSelectedText();
    }
}

void MainWindow::copySelection()
{
    QTextCursor cursor = editor->textCursor();
    if (cursor.hasSelection()) {
        selection = cursor.selection();
        cursor.clearSelection();
    }
}

void MainWindow::pasteSelection()
{
    QTextCursor cursor = editor->textCursor();
    cursor.insertFragment(selection);
}

void MainWindow::selectWord()
{
    QTextCursor cursor = editor->textCursor();
    QTextBlock block = cursor.block();

    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::StartOfWord);
    cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    cursor.endEditBlock();

    editor->setTextCursor(cursor);
}

void MainWindow::selectLine()
{
    QTextCursor cursor = editor->textCursor();
    QTextBlock block = cursor.block();

    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::StartOfLine);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    cursor.endEditBlock();

    editor->setTextCursor(cursor);
}

void MainWindow::selectBlock()
{
    QTextCursor cursor = editor->textCursor();
    QTextBlock block = cursor.block();

    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    cursor.endEditBlock();

    editor->setTextCursor(cursor);
}

void MainWindow::selectFrame()
{
    QTextCursor cursor = editor->textCursor();
    QTextFrame *frame = cursor.currentFrame();

    cursor.beginEditBlock();
    cursor.setPosition(frame->firstPosition());
    cursor.setPosition(frame->lastPosition(), QTextCursor::KeepAnchor);
    cursor.endEditBlock();

    editor->setTextCursor(cursor);
}

void MainWindow::insertList()
{
    QTextCursor cursor = editor->textCursor();
    cursor.beginEditBlock();

    QTextList *list = cursor.currentList();
    QTextListFormat listFormat;
    if (list) {
        listFormat = list->format();
        listFormat.setIndent(listFormat.indent() + 1);
    }

    listFormat.setStyle(QTextListFormat::ListDisc);
    cursor.insertList(listFormat);

    cursor.endEditBlock();
}

void MainWindow::updateMenus()
{
    QTextCursor cursor = editor->textCursor();
    cutAction->setEnabled(cursor.hasSelection());
    copyAction->setEnabled(cursor.hasSelection());

    pasteAction->setEnabled(!selection.isEmpty());
}
