#include <QtGui>

#include "mainwindow.h"

MainWindow::MainWindow()
{
    QMenu *fileMenu = new QMenu(tr("&File"));

    fileMenu->addAction(tr("E&xit"), this, SLOT(close()),
        QKeySequence(tr("Ctrl+Q", "File|Exit")));

    QMenu *actionsMenu = new QMenu(tr("&Actions"));
    actionsMenu->addAction(tr("&Highlight List Items"),
                        this, SLOT(highlightListItems()));
    actionsMenu->addAction(tr("&Show Current List"), this, SLOT(showList()));

    QMenu *insertMenu = new QMenu(tr("&Insert"));

    insertMenu->addAction(tr("&List"), this, SLOT(insertList()),
        QKeySequence(tr("Ctrl+L", "Insert|List")));

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(insertMenu);
    menuBar()->addMenu(actionsMenu);

    editor = new QTextEdit(this);
    document = new QTextDocument(this);
    editor->setDocument(document);

    setCentralWidget(editor);
    setWindowTitle(tr("Text Document List Items"));
}

void MainWindow::highlightListItems()
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
        processListItem(listItem);
        */
    }
    cursor.endEditBlock();
}

void MainWindow::showList()
{
    QTextCursor cursor = editor->textCursor();
    QTextFrame *frame = cursor.currentFrame();

    if (!frame)
        return;

    QTreeWidget *treeWidget = new QTreeWidget;
    treeWidget->setColumnCount(1);
    QStringList headerLabels;
    headerLabels << tr("Lists");
    treeWidget->setHeaderLabels(headerLabels);

    QTreeWidgetItem *parentItem = 0;
    QTreeWidgetItem *item;
    QTreeWidgetItem *lastItem = 0;
    parentItems.clear();
    previousItems.clear();

    QTextFrame::iterator it;
    for (it = frame->begin(); !(it.atEnd()); ++it) {

        QTextBlock block = it.currentBlock();

        if (block.isValid()) {

            QTextList *list = block.textList();

            if (list) {
                int index = list->itemNumber(block);
                if (index == 0) {
                    parentItems.append(parentItem);
                    previousItems.append(lastItem);
                    listStructures.append(list);
                    parentItem = lastItem;
                    lastItem = 0;

                    if (parentItem != 0)
                        item = new QTreeWidgetItem(parentItem, lastItem);
                    else
                        item = new QTreeWidgetItem(treeWidget, lastItem);

                } else {

                    while (parentItem != 0 && listStructures.last() != list) {
                        listStructures.pop_back();
                        parentItem = parentItems.takeLast();
                        lastItem = previousItems.takeLast();
                    }
                    if (parentItem != 0)
                        item = new QTreeWidgetItem(parentItem, lastItem);
                    else
                        item = new QTreeWidgetItem(treeWidget, lastItem);
                }
                item->setText(0, block.text());
                lastItem = item;
                /*
                processListItem(list, index);
                */
            }
        }
    }

    treeWidget->setWindowTitle(tr("List Contents"));
    treeWidget->show();
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
