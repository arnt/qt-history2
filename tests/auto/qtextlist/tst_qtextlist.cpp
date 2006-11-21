/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qtextdocument.h>
#include <qtextdocumentfragment.h>
#include <qtextlist.h>
#include <qabstracttextdocumentlayout.h>
#include <qtextcursor.h>
#include "../qtextdocument/common.h"

class QTextDocument;
class QTestDocumentLayout;

//TESTED_CLASS=
//TESTED_FILES=gui/text/qtextlist.h gui/text/qtextlist.cpp

class tst_QTextList : public QObject
{
    Q_OBJECT

public:
    tst_QTextList();


public slots:
    void init();
    void cleanup();
private slots:
    void item();
    void autoNumbering();
    void autoNumberingRTL();
    void formatChange();
    void cursorNavigation();
    void partialRemoval();
    void formatReferenceChange();
    void ensureItemOrder();
    void add();
    void defaultIndent();
    void blockUpdate();

private:
    QTextDocument *doc;
    QTextCursor cursor;
    QTestDocumentLayout *layout;
};

tst_QTextList::tst_QTextList()
{}

void tst_QTextList::init()
{
    doc = new QTextDocument();
    layout = new QTestDocumentLayout(doc);
    doc->setDocumentLayout(layout);
    cursor = QTextCursor(doc);
}

void tst_QTextList::cleanup()
{
    cursor = QTextCursor();
    delete doc;
    doc = 0;
}

void tst_QTextList::item()
{
    // this is basically a test for the key() + 1 in QTextList::item.
    QTextList *list = cursor.createList(QTextListFormat());
    QVERIFY(list->item(0).blockFormat().objectIndex() != -1);
}

void tst_QTextList::autoNumbering()
{
    QTextListFormat fmt;
    fmt.setStyle(QTextListFormat::ListLowerAlpha);
    QTextList *list = cursor.createList(fmt);
    QVERIFY(list);

    for (int i = 0; i < 27; ++i)
	cursor.insertBlock();

    QVERIFY(list->count() == 28);

    QVERIFY(cursor.currentList());
    QVERIFY(cursor.currentList()->itemNumber(cursor.block()) == 27);
    QVERIFY(cursor.currentList()->itemText(cursor.block()) == "ab.");
}

void tst_QTextList::autoNumberingRTL()
{
    QTextBlockFormat bfmt;
    bfmt.setLayoutDirection(Qt::RightToLeft);
    cursor.setBlockFormat(bfmt);

    QTextListFormat fmt;
    fmt.setStyle(QTextListFormat::ListUpperAlpha);
    QTextList *list = cursor.createList(fmt);
    QVERIFY(list);

    cursor.insertBlock();

    QVERIFY(list->count() == 2);

    QVERIFY(cursor.currentList()->itemText(cursor.block()) == ".B");
}

void tst_QTextList::formatChange()
{
    // testing the formatChanged slot in QTextListManager

    /* <initial block>
     * 1.
     * 2.
     */
    QTextList *list = cursor.insertList(QTextListFormat::ListDecimal);
    QTextList *firstList = list;
    cursor.insertBlock();

    QVERIFY(list && list->count() == 2);

    QTextBlockFormat bfmt = cursor.blockFormat();
//     QVERIFY(bfmt.object() == list);

    bfmt.setObjectIndex(-1);
    cursor.setBlockFormat(bfmt);

    QVERIFY(firstList->count() == 1);
}

void tst_QTextList::cursorNavigation()
{
    // testing some cursor list methods

    /* <initial block>
     * 1.
     * 2.
     */
    cursor.insertList(QTextListFormat::ListDecimal);
    cursor.insertBlock();

    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::NextBlock);
    cursor.movePosition(QTextCursor::NextBlock);
    QVERIFY(cursor.currentList());
    cursor.movePosition(QTextCursor::PreviousBlock);
    QVERIFY(cursor.currentList());
    QVERIFY(cursor.currentList()->itemNumber(cursor.block()) == 0);
}

void tst_QTextList::partialRemoval()
{
    /* this is essentially a test for PieceTable::removeBlock to not miss any
       blocks with the blockChanged signal emission that actually get removed.

       It creates two lists, like this:

       1. Hello World
       a. Foobar

       and then removes from within the 'Hello World' into the 'Foobar' .
       There used to be no emission for the removal of the second (a.) block,
       causing list inconsistencies.

       */

    QTextList *firstList = cursor.insertList(QTextListFormat::ListDecimal);

    QTextCursor selStart = cursor;
    selStart.movePosition(QTextCursor::PreviousCharacter);

    cursor.insertText("Hello World");

    selStart.movePosition(QTextCursor::NextCharacter);
    selStart.movePosition(QTextCursor::NextCharacter);
    selStart.clearSelection();

    QPointer<QTextList> secondList = cursor.insertList(QTextListFormat::ListCircle);
    cursor.insertText("Foobar");

    cursor.movePosition(QTextCursor::PreviousCharacter);
    QTextCursor selEnd = cursor;

    QTextCursor selection = selStart;
    selection.setPosition(selEnd.position(),  QTextCursor::KeepAnchor);

    selection.deleteChar();

    QVERIFY(!secondList);
    QVERIFY(!firstList->isEmpty());

    doc->undo();
}

void tst_QTextList::formatReferenceChange()
{
    QTextList *list = cursor.insertList(QTextListFormat::ListDecimal);
    cursor.insertText("Some Content...");
    cursor.insertBlock(QTextBlockFormat());

    cursor.setPosition(list->item(0).position());
    int listItemStartPos = cursor.position();
    cursor.movePosition(QTextCursor::NextBlock);
    int listItemLen = cursor.position() - listItemStartPos;
    layout->expect(listItemStartPos, listItemLen, listItemLen);

    QTextListFormat fmt = list->format();
    fmt.setStyle(QTextListFormat::ListCircle);
    list->setFormat(fmt);

    QVERIFY(layout->called);
    QVERIFY(!layout->error);
}

void tst_QTextList::ensureItemOrder()
{
    /*
     * Insert a new list item before the first one and verify the blocks
     * are sorted after that.
     */
    QTextList *list = cursor.insertList(QTextListFormat::ListDecimal);

    QTextBlockFormat fmt = cursor.blockFormat();
    cursor.movePosition(QTextCursor::Start);
    cursor.insertBlock(fmt);

    QCOMPARE(list->item(0).position(), 1);
    QCOMPARE(list->item(1).position(), 2);
}

void tst_QTextList::add()
{
    QTextList *list = cursor.insertList(QTextListFormat::ListDecimal);
    cursor.insertBlock(QTextBlockFormat());
    QCOMPARE(list->count(), 1);
    cursor.insertBlock(QTextBlockFormat());
    list->add(cursor.block());
    QCOMPARE(list->count(), 2);
}

// Task #72036
void tst_QTextList::defaultIndent()
{
    QTextListFormat fmt;
    QCOMPARE(fmt.indent(), 1);
}

void tst_QTextList::blockUpdate()
{
    // three items
    QTextList *list = cursor.insertList(QTextListFormat::ListDecimal);
    cursor.insertBlock();
    cursor.insertBlock();

    // remove second, needs also update on the third
    // since the numbering might have changed
    const int len = cursor.position() + cursor.block().length() - 1;
    layout->expect(1, len, len);
    list->remove(list->item(1));
    QVERIFY(!layout->error);
}

QTEST_MAIN(tst_QTextList)
#include "tst_qtextlist.moc"
