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
#include <qdebug.h>

#include <qtextcursor.h>
#include <qtextdocumentfragment.h>
#include <qtextformat.h>
#include <qtextobject.h>
#include <qtexttable.h>
#include <qabstracttextdocumentlayout.h>
#include <qtextlist.h>
#include <qtextcodec.h>
#include <qurl.h>
#include "common.h"

class QTextDocument;

//TESTED_CLASS=
//TESTED_FILES=gui/text/qtextdocument.h gui/text/qtextdocument.cpp

class tst_QTextDocument : public QObject
{
    Q_OBJECT

public:
    tst_QTextDocument();
    virtual ~tst_QTextDocument();


public slots:
    void init();
    void cleanup();
private slots:
    void getSetCheck();
    void isEmpty();
    void find_data();
    void find();
    void find2();
    void findWithRegExp_data();
    void findWithRegExp();
    void findMultiple();
    void basicIsModifiedChecks();
    void moreIsModified();
    void isModified2();
    void isModified3();
    void noundo_basicIsModifiedChecks();
    void noundo_moreIsModified();
    void noundo_isModified2();
    void noundo_isModified3();
    void mightBeRichText();

    void toHtml_data();
    void toHtml();

    void setFragmentMarkersInHtmlExport();

    void toHtmlBodyBgColor();

    void cursorPositionChanged();

    void textFrameIterator();

    void codecForHtml();

    void markContentsDirty();

    void clonePreservesTitle();
    void clonePreservesPageSize();
    void clonePreservesDefaultFont();
    void clonePreservesRootFrameFormat();
    void clonePreservesResources();
    void clonePreservesUserStates();
    void blockCount();
    void defaultStyleSheet();

    void resolvedFontInEmptyFormat();

    void defaultRootFrameMargin();

    void clearResources();

    void setPlainText();

    void deleteTextObjectsOnClear();

    void maximumBlockCount();
    void adjustSize();
    void initialUserData();

    void html_defaultFont();

    void blockCountChanged();

    void nonZeroDocumentLengthOnClear();

private:
    QTextDocument *doc;
    QTextCursor cursor;
    QFont defaultFont;
    QString htmlHead;
    QString htmlTail;
};

class MyAbstractTextDocumentLayout : public QAbstractTextDocumentLayout
{
public:
    MyAbstractTextDocumentLayout(QTextDocument *doc) : QAbstractTextDocumentLayout(doc) {}
    void draw(QPainter *, const PaintContext &) {}
    int hitTest(const QPointF &, Qt::HitTestAccuracy) const { return 0; }
    int pageCount() const { return 0; }
    QSizeF documentSize() const { return QSizeF(); }
    QRectF frameBoundingRect(QTextFrame *) const { return QRectF(); }
    QRectF blockBoundingRect(const QTextBlock &) const { return QRectF(); }
    void documentChanged(int, int, int) {}
};

// Testing get/set functions
void tst_QTextDocument::getSetCheck()
{
    QTextDocument obj1;
    // QAbstractTextDocumentLayout * QTextDocument::documentLayout()
    // void QTextDocument::setDocumentLayout(QAbstractTextDocumentLayout *)
    QPointer<MyAbstractTextDocumentLayout> var1 = new MyAbstractTextDocumentLayout(0);
    obj1.setDocumentLayout(var1);
    QCOMPARE(static_cast<QAbstractTextDocumentLayout *>(var1), obj1.documentLayout());
#if QT_VERSION >= 0x040200
    // QTextDocument in Qt < 4.2 crashes on this. Qt >= 4.2 should handle this gracefully
    obj1.setDocumentLayout((QAbstractTextDocumentLayout *)0);
    QVERIFY(var1.isNull());
    QVERIFY(obj1.documentLayout());
#else
    delete var1;
#endif

    // bool QTextDocument::useDesignMetrics()
    // void QTextDocument::setUseDesignMetrics(bool)
    obj1.setUseDesignMetrics(false);
    QCOMPARE(false, obj1.useDesignMetrics());
    obj1.setUseDesignMetrics(true);
    QCOMPARE(true, obj1.useDesignMetrics());
}

tst_QTextDocument::tst_QTextDocument()
{
}

tst_QTextDocument::~tst_QTextDocument()
{
}

void tst_QTextDocument::init()
{
    doc = new QTextDocument;
    cursor = QTextCursor(doc);
    defaultFont = QFont();

    htmlHead = QString("<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
                       "p, li { white-space: pre-wrap; }\n"
                       "</style></head>"
                       "<body style=\" font-family:'%1'; font-size:%2pt; font-weight:%3; font-style:%4;\">\n");
    htmlHead = htmlHead.arg(defaultFont.family()).arg(defaultFont.pointSizeF()).arg(defaultFont.weight() * 8).arg((defaultFont.italic() ? "italic" : "normal"));

    htmlTail = QString("</body></html>");
}

void tst_QTextDocument::cleanup()
{
    cursor = QTextCursor();
    delete doc;
    doc = 0;
}

void tst_QTextDocument::isEmpty()
{
    QVERIFY(doc->isEmpty());
}

void tst_QTextDocument::find_data()
{
    QTest::addColumn<QString>("haystack");
    QTest::addColumn<QString>("needle");
    QTest::addColumn<int>("flags");
    QTest::addColumn<int>("from");
    QTest::addColumn<int>("anchor");
    QTest::addColumn<int>("position");

    QTest::newRow("1") << "Hello World" << "World" << int(QTextDocument::FindCaseSensitively) << 0 << 6 << 11;

    QTest::newRow("2") << QString::fromAscii("Hello") + QString(QChar::ParagraphSeparator) + QString::fromAscii("World")
                    << "World" << int(QTextDocument::FindCaseSensitively) << 1 << 6 << 11;

    QTest::newRow("3") << QString::fromAscii("Hello") + QString(QChar::ParagraphSeparator) + QString::fromAscii("World")
                    << "Hello" << int(QTextDocument::FindCaseSensitively | QTextDocument::FindBackward) << 10 << 0 << 5;
    QTest::newRow("4wholewords") << QString::fromAscii("Hello Blah World")
                              << "Blah" << int(QTextDocument::FindWholeWords) << 0 << 6 << 10;
    QTest::newRow("5wholewords") << QString::fromAscii("HelloBlahWorld")
                              << "Blah" << int(QTextDocument::FindWholeWords) << 0 << -1 << -1;
    QTest::newRow("6wholewords") << QString::fromAscii("HelloBlahWorld Blah Hah")
                              << "Blah" << int(QTextDocument::FindWholeWords) << 0 << 15 << 19;
    QTest::newRow("7wholewords") << QString::fromAscii("HelloBlahWorld Blah Hah")
                              << "Blah" << int(QTextDocument::FindWholeWords | QTextDocument::FindBackward) << 23 << 15 << 19;

    QTest::newRow("across-paragraphs") << QString::fromAscii("First Parag\nSecond Parag with a lot more text")
                                       << "Parag" << int(QTextDocument::FindBackward)
                                       << 15 << 6 << 11;

    QTest::newRow("nbsp") << "Hello" + QString(QChar(QChar::Nbsp)) +"World" << " " << int(QTextDocument::FindCaseSensitively) << 0 << 5 << 6;
}

void tst_QTextDocument::find()
{
    QFETCH(QString, haystack);
    QFETCH(QString, needle);
    QFETCH(int, flags);
    QFETCH(int, from);
    QFETCH(int, anchor);
    QFETCH(int, position);

    cursor.insertText(haystack);
    cursor = doc->find(needle, from, QTextDocument::FindFlags(flags));

    if (anchor != -1) {
        QCOMPARE(cursor.anchor(), anchor);
        QCOMPARE(cursor.position(), position);
    } else {
        QVERIFY(cursor.isNull());
    }

    //search using a regular expression
    QRegExp expr(needle);
    expr.setPatternSyntax(QRegExp::FixedString);
    QTextDocument::FindFlags flg(flags);
    expr.setCaseSensitivity((flg & QTextDocument::FindCaseSensitively) ? Qt::CaseSensitive : Qt::CaseInsensitive);
    cursor = doc->find(expr, from, flg);

    if (anchor != -1) {
        QCOMPARE(cursor.anchor(), anchor);
        QCOMPARE(cursor.position(), position);
    } else {
        QVERIFY(cursor.isNull());
    }
}

void tst_QTextDocument::findWithRegExp_data()
{
    QTest::addColumn<QString>("haystack");
    QTest::addColumn<QString>("needle");
    QTest::addColumn<int>("flags");
    QTest::addColumn<int>("from");
    QTest::addColumn<int>("anchor");
    QTest::addColumn<int>("position");

    // match integers 0 to 99
    QTest::newRow("1") << "23" << "^\\d\\d?$" << int(QTextDocument::FindCaseSensitively) << 0 << 0 << 2;
    // match ampersands but not &amp;
    QTest::newRow("2") << "His &amp; hers & theirs" << "&(?!amp;)"<< int(QTextDocument::FindCaseSensitively) << 0 << 15 << 16;
    //backward search
    QTest::newRow("3") << QString::fromAscii("HelloBlahWorld Blah Hah")
                              << "h" << int(QTextDocument::FindBackward) << 18 << 8 << 9;

}

void tst_QTextDocument::findWithRegExp()
{
    QFETCH(QString, haystack);
    QFETCH(QString, needle);
    QFETCH(int, flags);
    QFETCH(int, from);
    QFETCH(int, anchor);
    QFETCH(int, position);

    cursor.insertText(haystack);
    //search using a regular expression
    QRegExp expr(needle);
    QTextDocument::FindFlags flg(flags);
    expr.setCaseSensitivity((flg & QTextDocument::FindCaseSensitively) ? Qt::CaseSensitive : Qt::CaseInsensitive);
    cursor = doc->find(expr, from, flg);

    if (anchor != -1) {
        QCOMPARE(cursor.anchor(), anchor);
        QCOMPARE(cursor.position(), position);
    } else {
        QVERIFY(cursor.isNull());
    }
}

void tst_QTextDocument::find2()
{
    doc->setPlainText("aaa");
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    QTextCursor hit = doc->find("a", cursor);
    QCOMPARE(hit.position(), 2);
    QCOMPARE(hit.anchor(), 1);
}

void tst_QTextDocument::findMultiple()
{
    const QString text("foo bar baz foo bar baz");
    doc->setPlainText(text);

    cursor.movePosition(QTextCursor::Start);
    cursor = doc->find("bar", cursor);
    QCOMPARE(cursor.selectionStart(), text.indexOf("bar"));
    QCOMPARE(cursor.selectionEnd(), cursor.selectionStart() + 3);
    cursor = doc->find("bar", cursor);
    QCOMPARE(cursor.selectionStart(), text.lastIndexOf("bar"));
    QCOMPARE(cursor.selectionEnd(), cursor.selectionStart() + 3);

    cursor.movePosition(QTextCursor::End);
    cursor = doc->find("bar", cursor, QTextDocument::FindBackward);
    QCOMPARE(cursor.selectionStart(), text.lastIndexOf("bar"));
    QCOMPARE(cursor.selectionEnd(), cursor.selectionStart() + 3);
    cursor = doc->find("bar", cursor, QTextDocument::FindBackward);
    QCOMPARE(cursor.selectionStart(), text.indexOf("bar"));
    QCOMPARE(cursor.selectionEnd(), cursor.selectionStart() + 3);


    QRegExp expr("bar");
    expr.setPatternSyntax(QRegExp::FixedString);

    cursor.movePosition(QTextCursor::End);
    cursor = doc->find(expr, cursor, QTextDocument::FindBackward);
    QCOMPARE(cursor.selectionStart(), text.lastIndexOf("bar"));
    QCOMPARE(cursor.selectionEnd(), cursor.selectionStart() + 3);
    cursor = doc->find(expr, cursor, QTextDocument::FindBackward);
    QCOMPARE(cursor.selectionStart(), text.indexOf("bar"));
    QCOMPARE(cursor.selectionEnd(), cursor.selectionStart() + 3);

    cursor.movePosition(QTextCursor::Start);
    cursor = doc->find(expr, cursor);
    QCOMPARE(cursor.selectionStart(), text.indexOf("bar"));
    QCOMPARE(cursor.selectionEnd(), cursor.selectionStart() + 3);
    cursor = doc->find(expr, cursor);
    QCOMPARE(cursor.selectionStart(), text.lastIndexOf("bar"));
    QCOMPARE(cursor.selectionEnd(), cursor.selectionStart() + 3);
}

void tst_QTextDocument::basicIsModifiedChecks()
{
    QSignalSpy spy(doc, SIGNAL(modificationChanged(bool)));

    QVERIFY(!doc->isModified());
    cursor.insertText("Hello World");
    QVERIFY(doc->isModified());
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.takeFirst().at(0).toBool());

    doc->undo();
    QVERIFY(!doc->isModified());
    QCOMPARE(spy.count(), 1);
    QVERIFY(!spy.takeFirst().at(0).toBool());

    doc->redo();
    QVERIFY(doc->isModified());
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.takeFirst().at(0).toBool());
}

void tst_QTextDocument::moreIsModified()
{
    QVERIFY(!doc->isModified());

    cursor.insertText("Hello");
    QVERIFY(doc->isModified());

    doc->undo();
    QVERIFY(!doc->isModified());

    cursor.insertText("Hello");

    doc->undo();
    QVERIFY(!doc->isModified());
}

void tst_QTextDocument::isModified2()
{
    // reported on qt4-preview-feedback
    QVERIFY(!doc->isModified());

    cursor.insertText("Hello");
    QVERIFY(doc->isModified());

    doc->setModified(false);
    QVERIFY(!doc->isModified());

    cursor.insertText("Hello");
    QVERIFY(doc->isModified());
}

void tst_QTextDocument::isModified3()
{
    QVERIFY(!doc->isModified());

    doc->setUndoRedoEnabled(false);
    doc->setUndoRedoEnabled(true);

    cursor.insertText("Hello");

    QVERIFY(doc->isModified());
    doc->undo();
    QVERIFY(!doc->isModified());
}

void tst_QTextDocument::noundo_basicIsModifiedChecks()
{
    doc->setUndoRedoEnabled(false);
    QSignalSpy spy(doc, SIGNAL(modificationChanged(bool)));

    QVERIFY(!doc->isModified());
    cursor.insertText("Hello World");
    QVERIFY(doc->isModified());
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.takeFirst().at(0).toBool());

    doc->undo();
    QVERIFY(doc->isModified());
    QCOMPARE(spy.count(), 0);

    doc->redo();
    QVERIFY(doc->isModified());
    QCOMPARE(spy.count(), 0);
}

void tst_QTextDocument::noundo_moreIsModified()
{
    doc->setUndoRedoEnabled(false);
    QVERIFY(!doc->isModified());

    cursor.insertText("Hello");
    QVERIFY(doc->isModified());

    doc->undo();
    QVERIFY(doc->isModified());

    cursor.insertText("Hello");

    doc->undo();
    QVERIFY(doc->isModified());
}

void tst_QTextDocument::noundo_isModified2()
{
    // reported on qt4-preview-feedback
    QVERIFY(!doc->isModified());

    cursor.insertText("Hello");
    QVERIFY(doc->isModified());

    doc->setModified(false);
    QVERIFY(!doc->isModified());

    cursor.insertText("Hello");
    QVERIFY(doc->isModified());
}

void tst_QTextDocument::noundo_isModified3()
{
    doc->setUndoRedoEnabled(false);
    QVERIFY(!doc->isModified());

    cursor.insertText("Hello");

    QVERIFY(doc->isModified());
    doc->undo();
    QVERIFY(doc->isModified());
}

void tst_QTextDocument::mightBeRichText()
{
    const char qtDocuHeader[] = "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n"
                                "<!DOCTYPE html\n"
                                "    PUBLIC ""-//W3C//DTD XHTML 1.0 Strict//EN\" \"DTD/xhtml1-strict.dtd\">\n"
                                "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">";
    QVERIFY(Qt::mightBeRichText(QString::fromLatin1(qtDocuHeader)));
}

Q_DECLARE_METATYPE(QTextDocumentFragment)

#define CREATE_DOC_AND_CURSOR() \
        QTextDocument doc; \
        doc.setDefaultFont(defaultFont); \
        QTextCursor cursor(&doc);

void tst_QTextDocument::toHtml_data()
{
    QTest::addColumn<QTextDocumentFragment>("input");
    QTest::addColumn<QString>("expectedOutput");

    {
        CREATE_DOC_AND_CURSOR();

        cursor.insertText("Blah");

        QTest::newRow("simple") << QTextDocumentFragment(&doc) << QString("<p DEFAULTBLOCKSTYLE>Blah</p>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        cursor.insertText("&<>");

        QTest::newRow("entities") << QTextDocumentFragment(&doc) << QString("<p DEFAULTBLOCKSTYLE>&amp;&lt;&gt;</p>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextCharFormat fmt;
        fmt.setFontFamily("Times");
        cursor.insertText("Blah", fmt);

        QTest::newRow("font-family") << QTextDocumentFragment(&doc)
                                  << QString("<p DEFAULTBLOCKSTYLE><span style=\" font-family:'Times';\">Blah</span></p>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextBlockFormat fmt;
        fmt.setNonBreakableLines(true);
        cursor.insertBlock(fmt);
        cursor.insertText("Blah");

        QTest::newRow("pre") << QTextDocumentFragment(&doc)
                          <<
                             QString("EMPTYBLOCK") +
                             QString("<pre DEFAULTBLOCKSTYLE>Blah</pre>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextCharFormat fmt;
        fmt.setFontPointSize(40);
        cursor.insertText("Blah", fmt);

        QTest::newRow("font-size") << QTextDocumentFragment(&doc)
                                << QString("<p DEFAULTBLOCKSTYLE><span style=\" font-size:40pt;\">Blah</span></p>");
    }

#if QT_VERSION < 0x040200
    {
        CREATE_DOC_AND_CURSOR();

        QTextCharFormat fmt;
        fmt.setProperty(QTextFormat::FontSizeIncrement, 2);
        cursor.insertText("Blah", fmt);

        QTest::newRow("logical-font-size") << QTextDocumentFragment(&doc)
                                        << QString("<p DEFAULTBLOCKSTYLE><font size=\"5\">Blah</font></p>");
    }
#else
    {
        CREATE_DOC_AND_CURSOR();

        QTextCharFormat fmt;
        fmt.setProperty(QTextFormat::FontSizeIncrement, 2);
        cursor.insertText("Blah", fmt);

        QTest::newRow("logical-font-size") << QTextDocumentFragment(&doc)
                                        << QString("<p DEFAULTBLOCKSTYLE><span style=\" font-size:x-large;\">Blah</span></p>");
    }
#endif

    {
        CREATE_DOC_AND_CURSOR();

        cursor.insertText("Foo");

        QTextCharFormat fmt;
        fmt.setFontPointSize(40);
        cursor.insertBlock(QTextBlockFormat(), fmt);

        fmt.clearProperty(QTextFormat::FontPointSize);
        cursor.insertText("Blub", fmt);

        // this is only half-way correct, we may not actually want to use
        // 40 as the font size for 'Blub', but at least this way is better than
        // emitting zero as point size, which previously was the case.
        QTest::newRow("no-font-size") << QTextDocumentFragment(&doc)
#if QT_VERSION >= 0x040200
                                   << QString("<p DEFAULTBLOCKSTYLE>Foo</p>\n<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:40pt;\">Blub</p>");
#else
                                   << QString("<p DEFAULTBLOCKSTYLE>Foo</p><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:40pt;\">Blub</p>");
#endif
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextBlockFormat fmt;
        fmt.setLayoutDirection(Qt::RightToLeft);
        cursor.insertBlock(fmt);
        cursor.insertText("Blah");

        QTest::newRow("rtl") << QTextDocumentFragment(&doc)
                          <<
                             QString("EMPTYBLOCK") +
                             QString("<p dir='rtl' DEFAULTBLOCKSTYLE>Blah</p>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextBlockFormat fmt;
        fmt.setAlignment(Qt::AlignJustify);
        cursor.insertBlock(fmt);
        cursor.insertText("Blah");

        QTest::newRow("blockalign") << QTextDocumentFragment(&doc)
                                 <<
                                    QString("EMPTYBLOCK") +
                                    QString("<p align=\"justify\" DEFAULTBLOCKSTYLE>Blah</p>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextBlockFormat fmt;
        fmt.setAlignment(Qt::AlignCenter);
        cursor.insertBlock(fmt);
        cursor.insertText("Blah");

        QTest::newRow("blockalign2") << QTextDocumentFragment(&doc)
                                  <<
                                    QString("EMPTYBLOCK") +
                                    QString("<p align=\"center\" DEFAULTBLOCKSTYLE>Blah</p>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextBlockFormat fmt;
        fmt.setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
        cursor.insertBlock(fmt);
        cursor.insertText("Blah");

        QTest::newRow("blockalign3") << QTextDocumentFragment(&doc)
                                  <<
                                    QString("EMPTYBLOCK") +
                                    QString("<p align=\"right\" DEFAULTBLOCKSTYLE>Blah</p>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextBlockFormat fmt;
        fmt.setBackground(QColor("#0000ff"));
        cursor.insertBlock(fmt);
        cursor.insertText("Blah");

        QTest::newRow("bgcolor") << QTextDocumentFragment(&doc)
                                 << QString("EMPTYBLOCK") +
                                    QString("<p OPENDEFAULTBLOCKSTYLE background-color:#0000ff;\">Blah</p>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextCharFormat fmt;
        fmt.setFontWeight(40);
        cursor.insertText("Blah", fmt);

        QTest::newRow("font-weight") << QTextDocumentFragment(&doc)
                                  << QString("<p DEFAULTBLOCKSTYLE><span style=\" font-weight:320;\">Blah</span></p>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextCharFormat fmt;
        fmt.setFontItalic(true);
        cursor.insertText("Blah", fmt);

        QTest::newRow("font-italic") << QTextDocumentFragment(&doc)
                                  << QString("<p DEFAULTBLOCKSTYLE><span style=\" font-style:italic;\">Blah</span></p>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextCharFormat fmt;
        fmt.setFontUnderline(true);
        fmt.setFontOverline(false);
        cursor.insertText("Blah", fmt);

        QTest::newRow("text-decoration-1") << QTextDocumentFragment(&doc)
                                  << QString("<p DEFAULTBLOCKSTYLE><span style=\" text-decoration: underline;\">Blah</span></p>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextCharFormat fmt;
        fmt.setForeground(QColor("#00ff00"));
        cursor.insertText("Blah", fmt);

        QTest::newRow("color") << QTextDocumentFragment(&doc)
                            << QString("<p DEFAULTBLOCKSTYLE><span style=\" color:#00ff00;\">Blah</span></p>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextCharFormat fmt;
        fmt.setBackground(QColor("#00ff00"));
        cursor.insertText("Blah", fmt);

        QTest::newRow("span-bgcolor") << QTextDocumentFragment(&doc)
                            << QString("<p DEFAULTBLOCKSTYLE><span style=\" background-color:#00ff00;\">Blah</span></p>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextCharFormat fmt;
        fmt.setVerticalAlignment(QTextCharFormat::AlignSubScript);
        cursor.insertText("Blah", fmt);

        QTest::newRow("valign-sub") << QTextDocumentFragment(&doc)
                                 << QString("<p DEFAULTBLOCKSTYLE><span style=\" vertical-align:sub;\">Blah</span></p>");

    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextCharFormat fmt;
        fmt.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
        cursor.insertText("Blah", fmt);

        QTest::newRow("valign-super") << QTextDocumentFragment(&doc)
                                   << QString("<p DEFAULTBLOCKSTYLE><span style=\" vertical-align:super;\">Blah</span></p>");

    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextCharFormat fmt;
        fmt.setAnchor(true);
        fmt.setAnchorName("blub");
        cursor.insertText("Blah", fmt);

        QTest::newRow("named anchor") << QTextDocumentFragment(&doc)
                                   << QString("<p DEFAULTBLOCKSTYLE><a name=\"blub\"></a>Blah</p>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextCharFormat fmt;
        fmt.setAnchor(true);
        fmt.setAnchorHref("http://www.kde.org/");
        cursor.insertText("Blah", fmt);

        QTest::newRow("href anchor") << QTextDocumentFragment(&doc)
                                  << QString("<p DEFAULTBLOCKSTYLE><a href=\"http://www.kde.org/\">Blah</a></p>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        cursor.insertTable(2, 2);

        QTest::newRow("simpletable") << QTextDocumentFragment(&doc)
                                  << QString("<table border=\"1\" cellspacing=\"2\">"
#if QT_VERSION >= 0x040200
                                             "\n<tr>\n<td></td>\n<td></td></tr>"
                                             "\n<tr>\n<td></td>\n<td></td></tr>"
#else
                                             "<tr><td></td><td></td></tr>"
                                             "<tr><td></td><td></td></tr>"
#endif
                                             "</table>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextTable *table = cursor.insertTable(1, 4);
        table->mergeCells(0, 0, 1, 2);
        table->mergeCells(0, 2, 1, 2);

        QTest::newRow("tablespans") << QTextDocumentFragment(&doc)
                                 << QString("<table border=\"1\" cellspacing=\"2\">"
#if QT_VERSION >= 0x040200
                                             "\n<tr>\n<td colspan=\"2\"></td>\n<td colspan=\"2\"></td></tr>"
#else
                                             "<tr><td colspan=\"2\"></td><td colspan=\"2\"></td></tr>"
#endif
                                             "</table>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextTableFormat fmt;
        fmt.setBorder(1);
        fmt.setCellSpacing(3);
        fmt.setCellPadding(3);
        fmt.setBackground(QColor("#ff00ff"));
        fmt.setWidth(QTextLength(QTextLength::PercentageLength, 50));
        fmt.setAlignment(Qt::AlignHCenter);
        fmt.setPosition(QTextFrameFormat::FloatRight);
        cursor.insertTable(2, 2, fmt);

        QTest::newRow("tableattrs") << QTextDocumentFragment(&doc)
                                  << QString("<table border=\"1\" style=\" float: right;\" align=\"center\" width=\"50%\" cellspacing=\"3\" cellpadding=\"3\" bgcolor=\"#ff00ff\">"
                                             "\n<tr>\n<td></td>\n<td></td></tr>"
                                             "\n<tr>\n<td></td>\n<td></td></tr>"
                                             "</table>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextTableFormat fmt;
        fmt.setBorder(1);
        fmt.setCellSpacing(3);
        fmt.setCellPadding(3);
        fmt.setBackground(QColor("#ff00ff"));
        fmt.setWidth(QTextLength(QTextLength::PercentageLength, 50));
        fmt.setAlignment(Qt::AlignHCenter);
        fmt.setPosition(QTextFrameFormat::FloatRight);
        fmt.setLeftMargin(25);
        fmt.setBottomMargin(35);
        cursor.insertTable(2, 2, fmt);

        QTest::newRow("tableattrs2") << QTextDocumentFragment(&doc)
                                  << QString("<table border=\"1\" style=\" float: right; margin-top:0px; margin-bottom:35px; margin-left:25px; margin-right:0px;\" align=\"center\" width=\"50%\" cellspacing=\"3\" cellpadding=\"3\" bgcolor=\"#ff00ff\">"
                                             "\n<tr>\n<td></td>\n<td></td></tr>"
                                             "\n<tr>\n<td></td>\n<td></td></tr>"
                                             "</table>");
    }

#if QT_VERSION >= 0x040200
    {
        CREATE_DOC_AND_CURSOR();

        QTextTableFormat fmt;
        fmt.setHeaderRowCount(2);
        cursor.insertTable(4, 2, fmt);

        QTest::newRow("tableheader") << QTextDocumentFragment(&doc)
                                  << QString("<table border=\"1\" cellspacing=\"2\">"
                                             "<thead>\n<tr>\n<td></td>\n<td></td></tr>"
                                             "\n<tr>\n<td></td>\n<td></td></tr></thead>"
                                             "\n<tr>\n<td></td>\n<td></td></tr>"
                                             "\n<tr>\n<td></td>\n<td></td></tr>"
                                             "</table>");
    }
#endif

    {
        CREATE_DOC_AND_CURSOR();

        QTextTable *table = cursor.insertTable(2, 2);
        QTextTable *subTable = table->cellAt(0, 1).firstCursorPosition().insertTable(1, 1);
        subTable->cellAt(0, 0).firstCursorPosition().insertText("Hey");

        QTest::newRow("nestedtable") << QTextDocumentFragment(&doc)
                                  << QString("<table border=\"1\" cellspacing=\"2\">"
#if QT_VERSION >= 0x040200
                                             "\n<tr>\n<td></td>\n<td>\n<table border=\"1\" cellspacing=\"2\">\n<tr>\n<td>\n<p DEFAULTBLOCKSTYLE>Hey</p></td></tr></table></td></tr>"
                                             "\n<tr>\n<td></td>\n<td></td></tr>"
#else
                                             "<tr><td></td><td><table border=\"1\" cellspacing=\"2\"><tr><td><p DEFAULTBLOCKSTYLE>Hey</p></td></tr></table></td></tr>"
                                             "<tr><td></td><td></td></tr>"
#endif
                                             "</table>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextTableFormat fmt;
        QVector<QTextLength> widths;
        widths.append(QTextLength());
        widths.append(QTextLength(QTextLength::PercentageLength, 30));
        widths.append(QTextLength(QTextLength::FixedLength, 40));
        fmt.setColumnWidthConstraints(widths);
        cursor.insertTable(1, 3, fmt);

        QTest::newRow("colwidths") << QTextDocumentFragment(&doc)
                                  << QString("<table border=\"1\" cellspacing=\"2\">"
#if QT_VERSION >= 0x040200
                                             "\n<tr>\n<td></td>\n<td width=\"30%\"></td>\n<td width=\"40\"></td></tr>"
#else
                                             "<tr><td></td><td width=\"30%\"></td><td width=\"40\"></td></tr>"
#endif
                                             "</table>");
    }

    // ### rowspan/colspan tests, once texttable api for that is back again
    //
    {
        CREATE_DOC_AND_CURSOR();

        QTextTable *table = cursor.insertTable(1, 1);
        QTextCursor cellCurs = table->cellAt(0, 0).firstCursorPosition();
        QTextCharFormat fmt;
        fmt.setBackground(QColor("#ffffff"));
        cellCurs.mergeBlockCharFormat(fmt);

        QTest::newRow("cellproperties") << QTextDocumentFragment(&doc)
                                     << QString("<table border=\"1\" cellspacing=\"2\">"
#if QT_VERSION >= 0x040200
                                                "\n<tr>\n<td bgcolor=\"#ffffff\"></td></tr>"
#else
                                                "<tr><td bgcolor=\"#ffffff\"></td></tr>"
#endif
                                                "</table>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        // ### fixme: use programmatic api as soon as we can create floats through it
        const char html[] = "<html><body>Blah<img src=\"image.png\" width=\"10\" height=\"20\" style=\"float: right;\" />Blubb</body></html>";

        QTest::newRow("image") << QTextDocumentFragment::fromHtml(QString::fromLatin1(html))
                            << QString("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Blah<img src=\"image.png\" width=\"10\" height=\"20\" style=\"float: right;\" />Blubb</p>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextImageFormat fmt;
        fmt.setName("foo");
        fmt.setVerticalAlignment(QTextCharFormat::AlignMiddle);
        cursor.insertImage(fmt);

        QTest::newRow("image-malign") << QTextDocumentFragment(&doc)
                            << QString("<p DEFAULTBLOCKSTYLE><img src=\"foo\" style=\"vertical-align: middle;\" /></p>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        QString txt = QLatin1String("Blah");
        txt += QChar::LineSeparator;
        txt += QLatin1String("Bar");
        cursor.insertText(txt);

        QTest::newRow("linebreaks") << QTextDocumentFragment(&doc)
                                 << QString("<p DEFAULTBLOCKSTYLE>Blah<br />Bar</p>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextBlockFormat fmt;
        fmt.setTopMargin(10);
        fmt.setBottomMargin(20);
        fmt.setLeftMargin(30);
        fmt.setRightMargin(40);
        cursor.insertBlock(fmt);
        cursor.insertText("Blah");

        QTest::newRow("blockmargins") << QTextDocumentFragment(&doc)
                          <<
#if QT_VERSION >= 0x040100
                             QString("EMPTYBLOCK") +
#endif
                             QString("<p style=\" margin-top:10px; margin-bottom:20px; margin-left:30px; margin-right:40px; -qt-block-indent:0; text-indent:0px;\">Blah</p>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextList *list = cursor.insertList(QTextListFormat::ListDisc);
        cursor.insertText("Blubb");
        cursor.insertBlock();
        cursor.insertText("Blah");
        QCOMPARE(list->count(), 2);

        QTest::newRow("lists") << QTextDocumentFragment(&doc)
                          <<
                             QString("EMPTYBLOCK") +
#if QT_VERSION >= 0x040200
                             QString("<ul style=\"-qt-list-indent: 1;\"><li DEFAULTBLOCKSTYLE>Blubb</li>\n<li DEFAULTBLOCKSTYLE>Blah</li></ul>");
#else
                             QString("<ul style=\"-qt-list-indent: 1;\"><li DEFAULTBLOCKSTYLE>Blubb<li DEFAULTBLOCKSTYLE>Blah</ul>");
#endif
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextList *list = cursor.insertList(QTextListFormat::ListDisc);
        cursor.insertText("Blubb");

        cursor.insertBlock();

        QTextCharFormat blockCharFmt;
        blockCharFmt.setForeground(QColor("#0000ff"));
        cursor.mergeBlockCharFormat(blockCharFmt);

        QTextCharFormat fmt;
        fmt.setForeground(QColor("#ff0000"));
        cursor.insertText("Blah", fmt);
        QCOMPARE(list->count(), 2);

        QTest::newRow("charfmt-for-list-item") << QTextDocumentFragment(&doc)
                          <<
                             QString("EMPTYBLOCK") +
#if QT_VERSION >= 0x040200
                             QString("<ul style=\"-qt-list-indent: 1;\"><li DEFAULTBLOCKSTYLE>Blubb</li>\n<li style=\" color:#0000ff;\" DEFAULTBLOCKSTYLE><span style=\" color:#ff0000;\">Blah</span></li></ul>");
#else
                             QString("<ul style=\"-qt-list-indent: 1;\"><li DEFAULTBLOCKSTYLE>Blubb<li style=\" color:#0000ff;\" DEFAULTBLOCKSTYLE><span style=\" color:#ff0000;\">Blah</span></ul>");
#endif
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextBlockFormat fmt;
        fmt.setIndent(3);
        cursor.insertBlock(fmt);
        cursor.insertText("Test");

        QTest::newRow("block-indent") << QTextDocumentFragment(&doc)
                                   <<
#if QT_VERSION >= 0x040100
                                    QString("EMPTYBLOCK") +
#endif
                                    QString("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:3; text-indent:3px;\">Test</p>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextListFormat fmt;
        fmt.setStyle(QTextListFormat::ListDisc);
        fmt.setIndent(4);
        cursor.insertList(fmt);
        cursor.insertText("Blah");

        QTest::newRow("list-indent") << QTextDocumentFragment(&doc)
                                  <<
                                    QString("EMPTYBLOCK") +
#if QT_VERSION >= 0x040200
                                    QString("<ul style=\"-qt-list-indent: 4;\"><li DEFAULTBLOCKSTYLE>Blah</li></ul>");
#else
                                    QString("<ul style=\"-qt-list-indent: 4;\"><li DEFAULTBLOCKSTYLE>Blah</ul>");
#endif
    }

    {
        CREATE_DOC_AND_CURSOR();

        cursor.insertBlock();


        QTest::newRow("emptyblock") << QTextDocumentFragment(&doc)
                                    // after insertBlock() we /do/ have two blocks in the document, so also expect
                                    // these in the html output
                                    << QString("EMPTYBLOCK") + QString("EMPTYBLOCK");
    }

    {
        CREATE_DOC_AND_CURSOR();

        // if you press enter twice in an empty textedit and then insert 'Test'
        // you actually get three visible paragraphs, two empty leading ones and
        // a third with the actual text. the corresponding html representation
        // therefore should also contain three paragraphs. that only works in >= 4.1,
        // 4.0.x needs this additional block.
#if QT_VERSION < 0x040100
        cursor.insertBlock();
#endif

        cursor.insertBlock();
        QTextCharFormat fmt;
        fmt.setForeground(QColor("#00ff00"));
        fmt.setProperty(QTextFormat::FontSizeIncrement, 1);
        cursor.mergeBlockCharFormat(fmt);

        fmt.setProperty(QTextFormat::FontSizeIncrement, 2);
        cursor.insertText("Test", fmt);

#if QT_VERSION < 0x040200
        QTest::newRow("blockcharfmt") << QTextDocumentFragment(&doc)
                                   << QString("EMPTYBLOCK<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; color:#00ff00;\"><font size=\"4\"><font size=\"5\">Test</font></font></p>");
#else
        QTest::newRow("blockcharfmt") << QTextDocumentFragment(&doc)
                                   << QString("EMPTYBLOCK<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:large; color:#00ff00;\"><span style=\" font-size:x-large;\">Test</span></p>");
#endif
    }

#if QT_VERSION >= 0x040100
    {
        CREATE_DOC_AND_CURSOR();

        QTextCharFormat fmt;
        fmt.setForeground(QColor("#00ff00"));
        cursor.setBlockCharFormat(fmt);
        fmt.setForeground(QColor("#0000ff"));
        cursor.insertText("Test", fmt);

        QTest::newRow("blockcharfmt2") << QTextDocumentFragment(&doc)
                                   << QString("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; color:#00ff00;\"><span style=\" color:#0000ff;\">Test</span></p>");
    }
#endif

    {
        QTest::newRow("horizontal-ruler") << QTextDocumentFragment::fromHtml("<hr />")
                                       <<
                                          QString("EMPTYBLOCK") +
                                          QString("<hr />");
    }
    {
        QTest::newRow("horizontal-ruler-with-width") << QTextDocumentFragment::fromHtml("<hr width=\"50%\"/>")
                                                  <<
                                                      QString("EMPTYBLOCK") +
                                                      QString("<hr width=\"50%\"/>");
    }
    {
        CREATE_DOC_AND_CURSOR();

        QTextFrame *mainFrame = cursor.currentFrame();

        QTextFrameFormat ffmt;
        ffmt.setBorder(1);
        ffmt.setPosition(QTextFrameFormat::FloatRight);
        ffmt.setMargin(2);
        ffmt.setWidth(100);
        ffmt.setHeight(50);
        ffmt.setBackground(QColor("#00ff00"));
        cursor.insertFrame(ffmt);
        cursor.insertText("Hello World");
        cursor = mainFrame->lastCursorPosition();

        QTest::newRow("frame") << QTextDocumentFragment(&doc)
#if QT_VERSION >= 0x040200
                            << QString("<table border=\"1\" style=\"-qt-table-type: frame; float: right; margin-top:2px; margin-bottom:2px; margin-left:2px; margin-right:2px;\" width=\"100\" height=\"50\" bgcolor=\"#00ff00\">\n<tr>\n<td style=\"border: none;\">\n<p DEFAULTBLOCKSTYLE>Hello World</p></td></tr></table>");
#else
                            << QString("<table border=\"1\" style=\"-qt-table-type: frame; float: right; margin-top:2px; margin-bottom:2px; margin-left:2px; margin-right:2px;\" width=\"100\" height=\"50\" bgcolor=\"#00ff00\"><tr><td style=\"border: none;\"><p DEFAULTBLOCKSTYLE>Hello World</p></td></tr></table>");
#endif
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextCharFormat fmt;
        fmt.setForeground(QColor("#00ff00"));
//        fmt.setBackground(QColor("#0000ff"));
        cursor.setBlockCharFormat(fmt);

        fmt.setForeground(QBrush());
//        fmt.setBackground(QBrush());
        cursor.insertText("Test", fmt);

//        QTest::newRow("nostylebrush") << QTextDocumentFragment(&doc) << QString("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; color:#00ff00; -qt-blockcharfmt-background-color:#0000ff;\">Test</p>");
        QTest::newRow("nostylebrush") << QTextDocumentFragment(&doc) << QString("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; color:#00ff00;\">Test</p>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextTable *table = cursor.insertTable(2, 2);
        table->mergeCells(0, 0, 1, 2);
        QTextTableFormat fmt = table->format();
        QVector<QTextLength> widths;
        widths.append(QTextLength(QTextLength::FixedLength, 20));
        widths.append(QTextLength(QTextLength::FixedLength, 40));
        fmt.setColumnWidthConstraints(widths);
        table->setFormat(fmt);

        QTest::newRow("mergedtablecolwidths") << QTextDocumentFragment(&doc)
                                  << QString("<table border=\"1\" cellspacing=\"2\">"
                                             "\n<tr>\n<td colspan=\"2\"></td></tr>"
                                             "\n<tr>\n<td width=\"20\"></td>\n<td width=\"40\"></td></tr>"
                                             "</table>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextCharFormat fmt;

        cursor.insertText("Blah\nGreen yellow green");
        cursor.setPosition(0);
        cursor.setPosition(23, QTextCursor::KeepAnchor);
        fmt.setBackground(Qt::green);
        cursor.mergeCharFormat(fmt);
        cursor.clearSelection();
        cursor.setPosition(11);
        cursor.setPosition(17, QTextCursor::KeepAnchor);
        fmt.setBackground(Qt::yellow);
        cursor.mergeCharFormat(fmt);
        cursor.clearSelection();

        QTest::newRow("multiparagraph-bgcolor") << QTextDocumentFragment(&doc)
                                 << QString("<p DEFAULTBLOCKSTYLE><span style=\" background-color:#00ff00;\">Blah</span></p>\n"
                                            "<p DEFAULTBLOCKSTYLE><span style=\" background-color:#00ff00;\">Green </span>"
                                            "<span style=\" background-color:#ffff00;\">yellow</span>"
                                            "<span style=\" background-color:#00ff00;\"> green</span></p>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextBlockFormat fmt;
        fmt.setBackground(QColor("#0000ff"));
        cursor.insertBlock(fmt);

        QTextCharFormat charfmt;
        charfmt.setBackground(QColor("#0000ff"));
        cursor.insertText("Blah", charfmt);

        QTest::newRow("nospan-bgcolor") << QTextDocumentFragment(&doc)
                                 << QString("EMPTYBLOCK") +
                                    QString("<p OPENDEFAULTBLOCKSTYLE background-color:#0000ff;\">Blah</p>");
    }

    {
        CREATE_DOC_AND_CURSOR();

        QTextTable *table = cursor.insertTable(2, 2);
        QTextCharFormat fmt = table->cellAt(0, 0).format();
        fmt.setVerticalAlignment(QTextCharFormat::AlignMiddle);
        table->cellAt(0, 0).setFormat(fmt);
        fmt = table->cellAt(0, 1).format();
        fmt.setVerticalAlignment(QTextCharFormat::AlignTop);
        table->cellAt(0, 1).setFormat(fmt);
        fmt = table->cellAt(1, 0).format();
        fmt.setVerticalAlignment(QTextCharFormat::AlignBottom);
        table->cellAt(1, 0).setFormat(fmt);

        table->cellAt(0, 0).firstCursorPosition().insertText("Blah");

        QTest::newRow("table-vertical-alignment") << QTextDocumentFragment(&doc)
                                  << QString("<table border=\"1\" cellspacing=\"2\">"
                                             "\n<tr>\n<td style=\" vertical-align:middle;\">\n"
                                             "<p DEFAULTBLOCKSTYLE>Blah</p></td>"
                                             "\n<td style=\" vertical-align:top;\"></td></tr>"
                                             "\n<tr>\n<td style=\" vertical-align:bottom;\"></td>"
                                             "\n<td></td></tr>"
                                             "</table>");
    }
}

void tst_QTextDocument::toHtml()
{
    QFETCH(QTextDocumentFragment, input);
    QFETCH(QString, expectedOutput);

    cursor.insertFragment(input);

    expectedOutput.prepend(htmlHead);

    expectedOutput.replace("OPENDEFAULTBLOCKSTYLE", "style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;");
    expectedOutput.replace("DEFAULTBLOCKSTYLE", "style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"");
    expectedOutput.replace("EMPTYBLOCK", "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"></p>\n");
    if (expectedOutput.endsWith(QLatin1Char('\n')))
        expectedOutput.chop(1);
    expectedOutput.append(htmlTail);

    QString output = doc->toHtml();

    QCOMPARE(output, expectedOutput);
}

void tst_QTextDocument::setFragmentMarkersInHtmlExport()
{
    {
        CREATE_DOC_AND_CURSOR();

        cursor.insertText("Leadin");
        const int startPos = cursor.position();

        cursor.insertText("Test");
        QTextCharFormat fmt;
        fmt.setForeground(QColor("#00ff00"));
        cursor.insertText("Blah", fmt);

        const int endPos = cursor.position();
        cursor.insertText("Leadout", QTextCharFormat());

        cursor.setPosition(startPos);
        cursor.setPosition(endPos, QTextCursor::KeepAnchor);
        QTextDocumentFragment fragment(cursor);

        QString expected = htmlHead;
        expected.replace(QRegExp("<body.*>"), QString("<body>"));
        expected += QString("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><!--StartFragment-->Test<span style=\" color:#00ff00;\">Blah</span><!--EndFragment--></p>") + htmlTail;
        QCOMPARE(fragment.toHtml(), expected);
    }
    {
        CREATE_DOC_AND_CURSOR();

        cursor.insertText("Leadin");
        const int startPos = cursor.position();

        cursor.insertText("Test");

        const int endPos = cursor.position();
        cursor.insertText("Leadout", QTextCharFormat());

        cursor.setPosition(startPos);
        cursor.setPosition(endPos, QTextCursor::KeepAnchor);
        QTextDocumentFragment fragment(cursor);

        QString expected = htmlHead;
        expected.replace(QRegExp("<body.*>"), QString("<body>"));
        expected += QString("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><!--StartFragment-->Test<!--EndFragment--></p>") + htmlTail;
        QCOMPARE(fragment.toHtml(), expected);
    }
}

void tst_QTextDocument::toHtmlBodyBgColor()
{
    CREATE_DOC_AND_CURSOR();

    cursor.insertText("Blah");

    QTextFrameFormat fmt;
    fmt.setBackground(QColor("#0000ff"));
    doc.rootFrame()->setFrameFormat(fmt);

    QString expectedHtml("<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
                       "p, li { white-space: pre-wrap; }\n"
                       "</style></head>"
                       "<body style=\" font-family:'%1'; font-size:%2pt; font-weight:%3; font-style:%4;\""
                       " bgcolor=\"#0000ff\">\n"
                       "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Blah</p>"
                       "</body></html>");

    expectedHtml = expectedHtml.arg(defaultFont.family()).arg(defaultFont.pointSizeF()).arg(defaultFont.weight() * 8).arg((defaultFont.italic() ? "italic" : "normal"));

    QCOMPARE(doc.toHtml(), expectedHtml);
}

class CursorPosSignalSpy : public QObject
{
    Q_OBJECT
public:
    CursorPosSignalSpy(QTextDocument *doc)
    {
        calls = 0;
        connect(doc, SIGNAL(cursorPositionChanged(const QTextCursor &)),
                this, SLOT(cursorPositionChanged(const QTextCursor &)));
    }

    int calls;

private slots:
    void cursorPositionChanged(const QTextCursor &)
    {
        ++calls;
    }
};

void tst_QTextDocument::cursorPositionChanged()
{
    CursorPosSignalSpy spy(doc);

    cursor.insertText("Test");
    QCOMPARE(spy.calls, 1);

    spy.calls = 0;
    QTextCursor unrelatedCursor(doc);
    unrelatedCursor.insertText("Blah");
    QCOMPARE(spy.calls, 2);

    spy.calls = 0;
    cursor.insertText("Blah");
    QCOMPARE(spy.calls, 1);

    spy.calls = 0;
    cursor.movePosition(QTextCursor::PreviousCharacter);
    QCOMPARE(spy.calls, 0);
}

void tst_QTextDocument::textFrameIterator()
{
    cursor.insertTable(1, 1);

    int blockCount = 0;
    int frameCount = 0;

    for (QTextFrame::Iterator frameIt = doc->rootFrame()->begin();
         !frameIt.atEnd(); ++frameIt) {
        if (frameIt.currentFrame())
            ++frameCount;
        else if (frameIt.currentBlock().isValid())
            ++blockCount;

    }

    QEXPECT_FAIL("", "This is currently worked around in the html export but needs fixing!", Continue);
    QCOMPARE(blockCount, 0);
    QCOMPARE(frameCount, 1);
}

void tst_QTextDocument::codecForHtml()
{
    const QByteArray header("<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html;charset=utf-16\">");
    QTextCodec *c = Qt::codecForHtml(header);
    QVERIFY(c);
    QCOMPARE(c->name(), QByteArray("UTF-16"));
}

class TestSyntaxHighlighter : public QObject
{
    Q_OBJECT
public:
    inline TestSyntaxHighlighter(QTextDocument *doc) : QObject(doc), ok(false) {}

    bool ok;

private slots:
    inline void markBlockDirty(int from, int charsRemoved, int charsAdded)
    {
        Q_UNUSED(charsRemoved);
        Q_UNUSED(charsAdded);
        QTextDocument *doc = static_cast<QTextDocument *>(parent());
        QTextBlock block = doc->findBlock(from);

        QTestDocumentLayout *lout = qobject_cast<QTestDocumentLayout *>(doc->documentLayout());
        lout->called = false;

        doc->markContentsDirty(block.position(), block.length());

        ok = (lout->called == false);
    }

    inline void modifyBlockAgain(int from, int charsRemoved, int charsAdded)
    {
        Q_UNUSED(charsRemoved);
        Q_UNUSED(charsAdded);
        QTextDocument *doc = static_cast<QTextDocument *>(parent());
        QTextBlock block = doc->findBlock(from);
        QTextCursor cursor(block);

        QTestDocumentLayout *lout = qobject_cast<QTestDocumentLayout *>(doc->documentLayout());
        lout->called = false;

        cursor.insertText("Foo");

        ok = (lout->called == true);
    }
};

void tst_QTextDocument::markContentsDirty()
{
    QTestDocumentLayout *lout = new QTestDocumentLayout(doc);
    doc->setDocumentLayout(lout);
    TestSyntaxHighlighter *highlighter = new TestSyntaxHighlighter(doc);
    connect(doc, SIGNAL(contentsChange(int, int, int)),
            highlighter, SLOT(markBlockDirty(int, int, int)));

    highlighter->ok = false;
    cursor.insertText("Some dummy text blah blah");
    QVERIFY(highlighter->ok);

    disconnect(doc, SIGNAL(contentsChange(int, int, int)),
               highlighter, SLOT(markBlockDirty(int, int, int)));
    connect(doc, SIGNAL(contentsChange(int, int, int)),
            highlighter, SLOT(modifyBlockAgain(int, int, int)));
    highlighter->ok = false;
    cursor.insertText("FooBar");
    QVERIFY(highlighter->ok);

    lout->called = false;

    doc->markContentsDirty(1, 4);

    QVERIFY(lout->called);
}

void tst_QTextDocument::clonePreservesTitle()
{
    const QString title("Foobar");
    doc->setHtml("<html><head><title>" + title + "</title></head><body>Hrm</body></html>");
    QCOMPARE(doc->metaInformation(QTextDocument::DocumentTitle), title);

    QTextDocument *clone = doc->clone();
    QCOMPARE(clone->metaInformation(QTextDocument::DocumentTitle), title);
    delete clone;
}

void tst_QTextDocument::clonePreservesPageSize()
{
    QSizeF sz(100., 100.);
    doc->setPageSize(sz);
    QTextDocument *clone = doc->clone();
    QCOMPARE(clone->pageSize(), sz);
    delete clone;
}

void tst_QTextDocument::clonePreservesDefaultFont()
{
    QFont f = doc->defaultFont();
    QVERIFY(f.pointSize() != 100);
    f.setPointSize(100);
    doc->setDefaultFont(f);
    QTextDocument *clone = doc->clone();
    QCOMPARE(clone->defaultFont(), f);
    delete clone;
}

void tst_QTextDocument::clonePreservesResources()
{
    QUrl testUrl(":/foobar");
    QVariant testResource("hello world");

    doc->addResource(QTextDocument::ImageResource, testUrl, testResource);
    QTextDocument *clone = doc->clone();
    QVERIFY(clone->resource(QTextDocument::ImageResource, testUrl) == testResource);
    delete clone;
}

void tst_QTextDocument::clonePreservesUserStates()
{
    QTextCursor cursor(doc);
    cursor.insertText("bla bla bla");
    cursor.block().setUserState(1);
    cursor.insertBlock();
    cursor.insertText("foo bar");
    cursor.block().setUserState(2);
    cursor.insertBlock();
    cursor.insertText("no user state");

    QTextDocument *clone = doc->clone();
    QTextBlock b1 = doc->begin(), b2 = clone->begin();
    while (b1 != doc->end()) {
        b1 = b1.next();
        b2 = b2.next();
        QCOMPARE(b1.userState(), b2.userState());
    }
    QVERIFY(b2 == clone->end());
    delete clone;
}

void tst_QTextDocument::clonePreservesRootFrameFormat()
{
    doc->setPlainText("Hello");
    QTextFrameFormat fmt = doc->rootFrame()->frameFormat();
    fmt.setMargin(200);
    doc->rootFrame()->setFrameFormat(fmt);
    QCOMPARE(doc->rootFrame()->frameFormat().margin(), qreal(200));
    QTextDocument *copy = doc->clone();
    QCOMPARE(copy->rootFrame()->frameFormat().margin(), qreal(200));
    delete copy;
}

void tst_QTextDocument::blockCount()
{
    QCOMPARE(doc->blockCount(), 1);
    cursor.insertBlock();
    QCOMPARE(doc->blockCount(), 2);
    cursor.insertBlock();
    QCOMPARE(doc->blockCount(), 3);
    cursor.insertText("blah blah");
    QCOMPARE(doc->blockCount(), 3);
    doc->undo();
    doc->undo();
    QCOMPARE(doc->blockCount(), 2);
    doc->undo();
    QCOMPARE(doc->blockCount(), 1);
}

void tst_QTextDocument::resolvedFontInEmptyFormat()
{
    QFont font;
    font.setPointSize(42);
    doc->setDefaultFont(font);
    QTextCharFormat fmt = doc->begin().charFormat();
    QVERIFY(fmt.properties().isEmpty());
    QVERIFY(fmt.font() == font);
}

void tst_QTextDocument::defaultRootFrameMargin()
{
#if QT_VERSION >= 0x040200
    QCOMPARE(doc->rootFrame()->frameFormat().margin(), 2.0);
#else
    QCOMPARE(doc->rootFrame()->frameFormat().margin(), 4.0);
#endif
}

class TestDocument : public QTextDocument
{
public:
    inline TestDocument(const QUrl &testUrl, const QString &testString)
       : url(testUrl), string(testString), resourceLoaded(false) {}

    bool hasResourceCached();

protected:
    virtual QVariant loadResource(int type, const QUrl &name);

private:
    QUrl url;
    QString string;
    bool resourceLoaded;
};

bool TestDocument::hasResourceCached()
{
    resourceLoaded = false;
    resource(QTextDocument::ImageResource, url);
    return !resourceLoaded;
}

QVariant TestDocument::loadResource(int type, const QUrl &name)
{
    if (type == QTextDocument::ImageResource
        && name == url) {
        resourceLoaded = true;
        return string;
    }
    return QTextDocument::loadResource(type, name);
}

void tst_QTextDocument::clearResources()
{
    // regular resource for QTextDocument
    QUrl testUrl(":/foobar");
    QVariant testResource("hello world");

    // implicitly cached resource, initially loaded through TestDocument::loadResource()
    QUrl cacheUrl(":/blub");
    QString cacheResource("mah");

    TestDocument doc(cacheUrl, cacheResource);
    doc.addResource(QTextDocument::ImageResource, testUrl, testResource);

    QVERIFY(doc.resource(QTextDocument::ImageResource, testUrl) == testResource);

    doc.setPlainText("Hah");
    QVERIFY(doc.resource(QTextDocument::ImageResource, testUrl) == testResource);

    doc.setHtml("<b>Mooo</b><img src=\":/blub\"/>");
    QVERIFY(doc.resource(QTextDocument::ImageResource, testUrl) == testResource);
    QVERIFY(doc.resource(QTextDocument::ImageResource, cacheUrl) == cacheResource);

    doc.clear();
    QVERIFY(!doc.resource(QTextDocument::ImageResource, testUrl).isValid());
    QVERIFY(!doc.hasResourceCached());
    doc.clear();

    doc.setHtml("<b>Mooo</b><img src=\":/blub\"/>");
    QVERIFY(doc.resource(QTextDocument::ImageResource, cacheUrl) == cacheResource);

    doc.setPlainText("Foob");
    QVERIFY(!doc.hasResourceCached());
}

void tst_QTextDocument::setPlainText()
{
    doc->setPlainText("Hello World");
    QString s("");
    doc->setPlainText(s);
    QCOMPARE(doc->toPlainText(), s);
}

void tst_QTextDocument::deleteTextObjectsOnClear()
{
    QPointer<QTextTable> table = cursor.insertTable(2, 2);
    QVERIFY(!table.isNull());
    doc->clear();
    QVERIFY(table.isNull());
}

void tst_QTextDocument::defaultStyleSheet()
{
    const QString sheet("p { background-color: green; }");
    QVERIFY(doc->defaultStyleSheet().isEmpty());
    doc->setDefaultStyleSheet(sheet);
    QCOMPARE(doc->defaultStyleSheet(), sheet);

    cursor.insertHtml("<p>test");
    QTextBlockFormat fmt = doc->begin().blockFormat();
    QVERIFY(fmt.background().color() == QColor("green"));

    doc->clear();
    cursor.insertHtml("<p>test");
    fmt = doc->begin().blockFormat();
    QVERIFY(fmt.background().color() == QColor("green"));

    QTextDocument *clone = doc->clone();
    QCOMPARE(clone->defaultStyleSheet(), sheet);
    cursor = QTextCursor(clone);
    cursor.insertHtml("<p>test");
    fmt = clone->begin().blockFormat();
    QVERIFY(fmt.background().color() == QColor("green"));
    delete clone;

    cursor = QTextCursor(doc);
    cursor.insertHtml("<p>test");
    fmt = doc->begin().blockFormat();
    QVERIFY(fmt.background().color() == QColor("green"));

    doc->clear();
    cursor.insertHtml("<style>p { background-color: red; }</style><p>test");
    fmt = doc->begin().blockFormat();
    QVERIFY(fmt.background().color() == QColor("red"));

    doc->clear();
    doc->setDefaultStyleSheet("invalid style sheet....");
    cursor.insertHtml("<p>test");
    fmt = doc->begin().blockFormat();
    QVERIFY(fmt.background().color() != QColor("green"));
}

void tst_QTextDocument::maximumBlockCount()
{
    QCOMPARE(doc->maximumBlockCount(), 0);
    QVERIFY(doc->isUndoRedoEnabled());

    cursor.insertBlock();
    cursor.insertText("Blah");
    cursor.insertBlock();
    cursor.insertText("Foo");
    QCOMPARE(doc->blockCount(), 3);
    QCOMPARE(doc->toPlainText(), QString("\nBlah\nFoo"));

    doc->setMaximumBlockCount(1);
    QVERIFY(!doc->isUndoRedoEnabled());

    QCOMPARE(doc->blockCount(), 1);
    QCOMPARE(doc->toPlainText(), QString("Foo"));

    cursor.insertBlock();
    cursor.insertText("Hello");
    doc->setMaximumBlockCount(1);
    QCOMPARE(doc->blockCount(), 1);
    QCOMPARE(doc->toPlainText(), QString("Hello"));

    doc->setMaximumBlockCount(100);
    for (int i = 0; i < 1000; ++i) {
        cursor.insertBlock();
        cursor.insertText("Blah)");
        QVERIFY(doc->blockCount() <= 100);
    }

    cursor.movePosition(QTextCursor::End);
    QCOMPARE(cursor.blockNumber(), 99);
    QTextCharFormat fmt;
    fmt.setFontItalic(true);
    cursor.setBlockCharFormat(fmt);
    cursor.movePosition(QTextCursor::Start);
    QVERIFY(!cursor.blockCharFormat().fontItalic());

    doc->setMaximumBlockCount(1);
    QVERIFY(cursor.blockCharFormat().fontItalic());

    QTextTable *table = cursor.insertTable(2, 2);
    QCOMPARE(doc->blockCount(), 6);
    cursor.insertBlock();
    QCOMPARE(doc->blockCount(), 1);
}

void tst_QTextDocument::adjustSize()
{
    // avoid ugly tooltips like in task 125583
    QString text("Test Text");
    doc->setPlainText(text);
    doc->rootFrame()->setFrameFormat(QTextFrameFormat());
    doc->adjustSize();
    QCOMPARE(doc->size().width(), doc->idealWidth());
}

void tst_QTextDocument::initialUserData()
{
    doc->setPlainText("Hello");
    QTextBlock block = doc->begin();
    block.setUserData(new QTextBlockUserData);
    QVERIFY(block.userData());
    doc->documentLayout();
    QVERIFY(block.userData());
    doc->setDocumentLayout(new QTestDocumentLayout(doc));
    QVERIFY(!block.userData());
}

void tst_QTextDocument::html_defaultFont()
{
    QFont f;
    f.setItalic(true);
    f.setWeight(QFont::Bold);
    doc->setDefaultFont(f);
    doc->setPlainText("Test");

    QString bodyPart = QString::fromLatin1("<body style=\" font-family:'%1'; font-size:%2pt; font-weight:%3; font-style:italic;\">")
                       .arg(f.family()).arg(f.pointSizeF()).arg(f.weight() * 8);

    QString html = doc->toHtml();
    if (!html.contains(bodyPart)) {
        qDebug() << "html:" << html;
        qDebug() << "expected body:" << bodyPart;
        QVERIFY(html.contains(bodyPart));
    }

    if (html.contains("span"))
        qDebug() << "html:" << html;
    QVERIFY(!html.contains("<span style"));
}

void tst_QTextDocument::blockCountChanged()
{
    QSignalSpy spy(doc, SIGNAL(blockCountChanged(int)));

    doc->setPlainText("Foo");

    QCOMPARE(doc->blockCount(), 1);
    QCOMPARE(spy.count(), 0);

    spy.clear();

    doc->setPlainText("Foo\nBar");
    QCOMPARE(doc->blockCount(), 2);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).value(0).toInt(), 2);

    spy.clear();

    cursor.movePosition(QTextCursor::End);
    cursor.insertText("Blahblah");

    QCOMPARE(spy.count(), 0);

    cursor.insertBlock();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).value(0).toInt(), 3);

    spy.clear();
    doc->undo();

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).value(0).toInt(), 2);
}

void tst_QTextDocument::nonZeroDocumentLengthOnClear()
{
    QTestDocumentLayout *lout = new QTestDocumentLayout(doc);
    doc->setDocumentLayout(lout);

    doc->clear();
    QVERIFY(lout->called);
    QVERIFY(!lout->lastDocumentLengths.contains(0));
}

QTEST_MAIN(tst_QTextDocument)
#include "tst_qtextdocument.moc"
