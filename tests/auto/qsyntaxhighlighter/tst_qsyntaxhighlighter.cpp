/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QTextDocument>
#include <QTextLayout>
#include <QDebug>
#include <QAbstractTextDocumentLayout>
#include <QSyntaxHighlighter>

//TESTED_CLASS=
//TESTED_FILES=gui/text/qsyntaxhighlighter.h gui/text/qsyntaxhighlighter.cpp
//
class QTestDocumentLayout : public QAbstractTextDocumentLayout
{
    Q_OBJECT
public:
    inline QTestDocumentLayout(QTextDocument *doc)
        : QAbstractTextDocumentLayout(doc), documentChangedCalled(false) {}

        virtual void draw(QPainter *, const QAbstractTextDocumentLayout::PaintContext &)  {}

        virtual int hitTest(const QPointF &, Qt::HitTestAccuracy ) const { return 0; }

        virtual void documentChanged(int, int, int) { documentChangedCalled = true; }

        virtual int pageCount() const { return 1; }

        virtual QSizeF documentSize() const { return QSize(); }

        virtual QRectF frameBoundingRect(QTextFrame *) const { return QRectF(); }
        virtual QRectF blockBoundingRect(const QTextBlock &) const { return QRectF(); }

        bool documentChangedCalled;
};

class tst_QSyntaxHighlighter : public QObject
{
    Q_OBJECT
public:
    inline tst_QSyntaxHighlighter() {}

public slots:
    void init();
    void cleanup();

private slots:
    void basic();
    void basicTwo();
    void removeFormatsOnDelete();
    void emptyBlocks();
    void setCharFormat();
    void highlightOnInit();
    void stopHighlightingWhenStateDoesNotChange();
    void unindent();
    void highlightToEndOfDocument();
    void highlightToEndOfDocument2();
    void preservePreeditArea();
    void task108530();
    void avoidUnnecessaryRehighlight();

private:
    QTextDocument *doc;
    QTestDocumentLayout *lout;
    QTextCursor cursor;
};

void tst_QSyntaxHighlighter::init()
{
    doc = new QTextDocument;
    lout = new QTestDocumentLayout(doc);
    doc->setDocumentLayout(lout);
    cursor = QTextCursor(doc);
}

void tst_QSyntaxHighlighter::cleanup()
{
    delete doc;
    doc = 0;
}

class TestHighlighter : public QSyntaxHighlighter
{
public:
    inline TestHighlighter(const QList<QTextLayout::FormatRange> &fmts, QTextDocument *parent)
        : QSyntaxHighlighter(parent), formats(fmts), highlighted(false), callCount(0) {}
        inline TestHighlighter(QTextDocument *parent)
            : QSyntaxHighlighter(parent), highlighted(false), callCount(0) {}

            virtual void highlightBlock(const QString &text)
            {
                for (int i = 0; i < formats.count(); ++i) {
                    const QTextLayout::FormatRange &range = formats.at(i);
                    setFormat(range.start, range.length, range.format);
                }
                highlighted = true;
                highlightedText += text;
                ++callCount;
            }

            QList<QTextLayout::FormatRange> formats;
            bool highlighted;
            int callCount;
            QString highlightedText;
};

static bool operator==(const QTextLayout::FormatRange &lhs, const QTextLayout::FormatRange &rhs)
{
    return lhs.start == rhs.start
        && lhs.length == rhs.length
        && lhs.format == rhs.format;
}

void tst_QSyntaxHighlighter::basic()
{
    QList<QTextLayout::FormatRange> formats;
    QTextLayout::FormatRange range;
    range.start = 0;
    range.length = 2;
    range.format.setForeground(Qt::blue);
    formats.append(range);

    range.start = 4;
    range.length = 2;
    range.format.setFontItalic(true);
    formats.append(range);

    range.start = 9;
    range.length = 2;
    range.format.setFontUnderline(true);
    formats.append(range);

    TestHighlighter *hl = new TestHighlighter(formats, doc);

    lout->documentChangedCalled = false;
    doc->setPlainText("Hello World");
    QVERIFY(hl->highlighted);
    QVERIFY(lout->documentChangedCalled);

    QVERIFY(doc->begin().layout()->additionalFormats() == formats);
}

class CommentTestHighlighter : public QSyntaxHighlighter
{
public:
    inline CommentTestHighlighter(QTextDocument *parent)
        : QSyntaxHighlighter(parent), highlighted(false) {}

        inline void reset()
        {
            highlighted = false;
        }

        virtual void highlightBlock(const QString &text)
        {
            QTextCharFormat commentFormat;
            commentFormat.setForeground(Qt::darkGreen);
            commentFormat.setFontWeight(QFont::StyleItalic);
            commentFormat.setFontFixedPitch(true);
            int textLength = text.length();
                
            if (text.startsWith(QLatin1Char(';'))){
                // The entire line is a comment
                setFormat(0, textLength, commentFormat);
                highlighted = true;
            }
        }
        bool highlighted;
};


void tst_QSyntaxHighlighter::basicTwo()
{
    // Done for task 104409
    CommentTestHighlighter *hl = new CommentTestHighlighter(doc);
    doc->setPlainText("; a test");
    QVERIFY(hl->highlighted);
    QVERIFY(lout->documentChangedCalled);
}

void tst_QSyntaxHighlighter::removeFormatsOnDelete()
{
    QList<QTextLayout::FormatRange> formats;
    QTextLayout::FormatRange range;
    range.start = 0;
    range.length = 9;
    range.format.setForeground(Qt::blue);
    formats.append(range);

    TestHighlighter *hl = new TestHighlighter(formats, doc);

    lout->documentChangedCalled = false;
    doc->setPlainText("Hello World");
    QVERIFY(hl->highlighted);
    QVERIFY(lout->documentChangedCalled);

    lout->documentChangedCalled = false;
    QVERIFY(!doc->begin().layout()->additionalFormats().isEmpty());
    delete hl;
    QVERIFY(doc->begin().layout()->additionalFormats().isEmpty());
    QVERIFY(lout->documentChangedCalled);
}

void tst_QSyntaxHighlighter::emptyBlocks()
{
    TestHighlighter *hl = new TestHighlighter(doc);

    cursor.insertText("Foo");
    cursor.insertBlock();
    cursor.insertBlock();
    hl->highlighted = false;
    cursor.insertBlock();
    QVERIFY(hl->highlighted);
}

void tst_QSyntaxHighlighter::setCharFormat()
{
    TestHighlighter *hl = new TestHighlighter(doc);

    cursor.insertText("FooBar");
    cursor.insertBlock();
    cursor.insertText("Blah");
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    QTextCharFormat fmt;
    fmt.setFontItalic(true);
    hl->highlighted = false;
    hl->callCount = 0;
    cursor.mergeCharFormat(fmt);
    QVERIFY(hl->highlighted);
    QCOMPARE(hl->callCount, 2);
}

void tst_QSyntaxHighlighter::highlightOnInit()
{
    cursor.insertText("Hello");
    cursor.insertBlock();
    cursor.insertText("World");

    TestHighlighter *hl = new TestHighlighter(doc);
    QCoreApplication::instance()->processEvents();
    QVERIFY(hl->highlighted);
}

class StateTestHighlighter : public QSyntaxHighlighter
{
public:
    inline StateTestHighlighter(QTextDocument *parent)
        : QSyntaxHighlighter(parent), state(0), highlighted(false) {}

        inline void reset()
        {
            highlighted = false;
            state = 0;
        }

        virtual void highlightBlock(const QString &text)
        {
            highlighted = true;
            if (text == QLatin1String("changestate"))
                setCurrentBlockState(state++);
        }

        int state;
        bool highlighted;
};

void tst_QSyntaxHighlighter::stopHighlightingWhenStateDoesNotChange()
{
    cursor.insertText("state");
    cursor.insertBlock();
    cursor.insertText("changestate");
    cursor.insertBlock();
    cursor.insertText("keepstate");
    cursor.insertBlock();
    cursor.insertText("changestate");
    cursor.insertBlock();
    cursor.insertText("changestate");

    StateTestHighlighter *hl = new StateTestHighlighter(doc);
    QCoreApplication::instance()->processEvents();
    QVERIFY(hl->highlighted);

    hl->reset();

    // turn the text of the first block into 'changestate'
    cursor.movePosition(QTextCursor::Start);
    cursor.insertText("change");

    // verify that we highlighted only to the 'keepstate' block,
    // not beyond
    QCOMPARE(hl->state, 2);
}

void tst_QSyntaxHighlighter::unindent()
{
    const QString spaces("    ");
    const QString text("Foobar");
    QString plainText;
    for (int i = 0; i < 5; ++i) {
        cursor.insertText(spaces + text);
        cursor.insertBlock();

        plainText += spaces;
        plainText += text;
        plainText += QLatin1Char('\n');
    }
    QCOMPARE(doc->toPlainText(), plainText);

    TestHighlighter *hl = new TestHighlighter(doc);
    hl->callCount = 0;

    cursor.movePosition(QTextCursor::Start);
    cursor.beginEditBlock();

    plainText.clear();
    for (int i = 0; i < 5; ++i) {
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 4);
        cursor.removeSelectedText();
        cursor.movePosition(QTextCursor::NextBlock);

        plainText += text;
        plainText += QLatin1Char('\n');
    }

    cursor.endEditBlock();
    QCOMPARE(doc->toPlainText(), plainText);
    QCOMPARE(hl->callCount, 5);
}

void tst_QSyntaxHighlighter::highlightToEndOfDocument()
{
    TestHighlighter *hl = new TestHighlighter(doc);
    hl->callCount = 0;

    cursor.movePosition(QTextCursor::Start);
    cursor.beginEditBlock();

    cursor.insertText("Hello");
    cursor.insertBlock();
    cursor.insertBlock();
    cursor.insertText("World");
    cursor.insertBlock();

    cursor.endEditBlock();

    QCOMPARE(hl->callCount, 4);
}

void tst_QSyntaxHighlighter::highlightToEndOfDocument2()
{
    TestHighlighter *hl = new TestHighlighter(doc);
    hl->callCount = 0;

    cursor.movePosition(QTextCursor::End);
    cursor.beginEditBlock();
    QTextBlockFormat fmt;
    fmt.setAlignment(Qt::AlignLeft);
    cursor.setBlockFormat(fmt);
    cursor.insertText("Three\nLines\nHere");
    cursor.endEditBlock();

    QCOMPARE(hl->callCount, 3);
}

void tst_QSyntaxHighlighter::preservePreeditArea()
{
    QList<QTextLayout::FormatRange> formats;
    QTextLayout::FormatRange range;
    range.start = 0;
    range.length = 8;
    range.format.setForeground(Qt::blue);
    formats << range;
    range.start = 9;
    range.length = 1;
    range.format.setForeground(Qt::red);
    formats << range;
    TestHighlighter *hl = new TestHighlighter(formats, doc);

    doc->setPlainText("Hello World");
    cursor.movePosition(QTextCursor::Start);

    QTextLayout *layout = cursor.block().layout();

    layout->setPreeditArea(5, QString("foo"));
    range.start = 5;
    range.length = 3;
    range.format.setFontUnderline(true);
    formats.clear();
    formats << range;

    hl->callCount = 0;

    cursor.beginEditBlock();
    layout->setAdditionalFormats(formats);
    cursor.endEditBlock();

    QCOMPARE(hl->callCount, 1);

    formats = layout->additionalFormats();
    QCOMPARE(formats.count(), 3);

    range = formats.at(0);

    QCOMPARE(range.start, 5);
    QCOMPARE(range.length, 3);
    QVERIFY(range.format.fontUnderline());

    range = formats.at(1);
    QCOMPARE(range.start, 0);
    QCOMPARE(range.length, 8 + 3);

    range = formats.at(2);
    QCOMPARE(range.start, 9 + 3);
    QCOMPARE(range.length, 1);
}

void tst_QSyntaxHighlighter::task108530()
{
    TestHighlighter *hl = new TestHighlighter(doc);
    
    cursor.insertText("test");
    hl->callCount = 0;
    hl->highlightedText.clear();
    cursor.movePosition(QTextCursor::Start);
    cursor.insertBlock();
    
    QCOMPARE(hl->highlightedText, QString("test"));
    QCOMPARE(hl->callCount, 2);
}

void tst_QSyntaxHighlighter::avoidUnnecessaryRehighlight()
{
    TestHighlighter *hl = new TestHighlighter(doc);
    QVERIFY(!hl->highlighted);

    doc->setPlainText("Hello World");
    QVERIFY(hl->highlighted);

    hl->highlighted = false;
    QApplication::processEvents();
    QVERIFY(!hl->highlighted);
}

QTEST_MAIN(tst_QSyntaxHighlighter)
#include "tst_qsyntaxhighlighter.moc"
