/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
    !!!!!! Warning !!!!!
    Please don't save this file in emacs. It contains utf8 text sequences emacs will
    silently convert to a series of question marks.
 */
#include <QtTest/QtTest>



#include <private/qtextengine_p.h>
#include <qtextlayout.h>

#include <qdebug.h>





//TESTED_CLASS=
//TESTED_FILES=gui/text/qtextlayout.h gui/text/qtextlayout.cpp

class tst_QTextLayout : public QObject
{
    Q_OBJECT

public:
    tst_QTextLayout();
    virtual ~tst_QTextLayout();


public slots:
    void init();
    void cleanup();
private slots:
    void getSetCheck();
    void lineBreaking();
    void simpleBoundingRect();
    void threeLineBoundingRect();
    void forcedBreaks();
    void breakAny();
    void cursorToXForInlineObjects();
    void defaultWordSeparators_data();
    void defaultWordSeparators();
    void cursorMovementInsideSpaces();
    void charWordStopOnLineSeparator();
    void xToCursorAtEndOfLine();

private:
    QFont testFont;
};

// Testing get/set functions
void tst_QTextLayout::getSetCheck()
{
    QString str("Bogus text");
    QTextLayout layout(str, testFont);
    layout.beginLayout();
    QTextEngine *engine = layout.engine();
    QTextInlineObject obj1(0, engine);
    // qreal QTextInlineObject::width()
    // void QTextInlineObject::setWidth(qreal)
    obj1.setWidth(0.0);
    QCOMPARE(0.0, obj1.width());
    obj1.setWidth(1.2);
    QVERIFY(1.0 < obj1.width());

    // qreal QTextInlineObject::ascent()
    // void QTextInlineObject::setAscent(qreal)
    obj1.setAscent(0.0);
    QCOMPARE(0.0, obj1.ascent());
    obj1.setAscent(1.2);
    QVERIFY(1.0 < obj1.ascent());

    // qreal QTextInlineObject::descent()
    // void QTextInlineObject::setDescent(qreal)
    obj1.setDescent(0.0);
    QCOMPARE(0.0, obj1.descent());
    obj1.setDescent(1.2);
    QVERIFY(1.0 < obj1.descent());

    QTextLayout obj2;
    // bool QTextLayout::cacheEnabled()
    // void QTextLayout::setCacheEnabled(bool)
    obj2.setCacheEnabled(false);
    QCOMPARE(false, obj2.cacheEnabled());
    obj2.setCacheEnabled(true);
    QCOMPARE(true, obj2.cacheEnabled());
}

extern Q_GUI_EXPORT bool qt_enable_test_font;

tst_QTextLayout::tst_QTextLayout()
{
    qt_enable_test_font = true;
}

tst_QTextLayout::~tst_QTextLayout()
{
}

void tst_QTextLayout::init()
{
    testFont = QFont();
    testFont.setFamily("__Qt__Box__Engine__");
    testFont.setPixelSize(12);
    testFont.setWeight(QFont::Normal);
#if defined(Q_WS_MAC) && QT_VERSION < 0x040200
    QSKIP("QTestFontEngine is not supported on the mac right now", SkipAll);
#endif
    QCOMPARE(QFontMetrics(testFont).width('a'), testFont.pixelSize());
}

void tst_QTextLayout::cleanup()
{
    testFont = QFont();
}


void tst_QTextLayout::lineBreaking()
{
#if defined(Q_WS_X11)
    struct Breaks {
	const char *utf8;
	uchar breaks[32];
    };
    Breaks brks[] = {
	{ "11", { false, false, 0xff } },
	{ "aa", { false, false, 0xff } },
	{ "++", { false, false, 0xff } },
	{ "--", { false, false, 0xff } },
	{ "((", { false, false, 0xff } },
	{ "))", { false, false, 0xff } },
	{ "..", { false, false, 0xff } },
	{ "\"\"", { false, false, 0xff } },
	{ "$$", { false, false, 0xff } },
	{ "!!", { false, false, 0xff } },
	{ "??", { false, false, 0xff } },
	{ ",,", { false, false, 0xff } },

	{ ")()", { false, true, false, 0xff } },
	{ "?!?", { false, false, false, 0xff } },
	{ ".,.", { false, false, false, 0xff } },
	{ "+-+", { false, false, false, 0xff } },
	{ "+=+", { false, false, false, 0xff } },
	{ "+(+", { false, false, false, 0xff } },
	{ "+)+", { false, false, false, 0xff } },

	{ "a b", { false, false, true, 0xff } },
	{ "a(b", { false, false, false, 0xff } },
	{ "a)b", { false, false, false, 0xff } },
	{ "a-b", { false, false, false, 0xff } },
	{ "a.b", { false, false, false, 0xff } },
	{ "a+b", { false, false, false, 0xff } },
	{ "a?b", { false, false, false, 0xff } },
	{ "a!b", { false, false, false, 0xff } },
	{ "a$b", { false, false, false, 0xff } },
	{ "a,b", { false, false, false, 0xff } },
	{ "a/b", { false, false, false, 0xff } },
	{ "1/2", { false, false, false, 0xff } },
	{ "./.", { false, false, false, 0xff } },
	{ ",/,", { false, false, false, 0xff } },
	{ "!/!", { false, false, false, 0xff } },
	{ "\\/\\", { false, false, false, 0xff } },
	{ "1 2", { false, false, true, 0xff } },
	{ "1(2", { false, true, false, 0xff } },
	{ "1)2", { false, false, true, 0xff } },
	{ "1-2", { false, false, false, 0xff } },
	{ "1.2", { false, false, false, 0xff } },
	{ "1+2", { false, false, false, 0xff } },
	{ "1?2", { false, false, false, 0xff } },
	{ "1!2", { false, false, false, 0xff } },
	{ "1$2", { false, false, false, 0xff } },
	{ "1,2", { false, false, false, 0xff } },
	{ "1/2", { false, false, false, 0xff } },
	{ "\330\260\331\216\331\204\331\220\331\203\331\216", { false, false, false, false, false, false, 0xff } },
	{ "\330\247\331\204\331\205 \330\247\331\204\331\205", { false, false, false, false, true, false, false, 0xff } },
	{ "1#2", { false, false, false, 0xff } },
	{ "!#!", { false, false, false, 0xff } },
	{ 0, {} }
    };
    Breaks *b = brks;
    while (b->utf8) {
	QString str = QString::fromUtf8(b->utf8);
	QTextEngine engine(str, QFont());
	const QCharAttributes *attrs = engine.attributes();
	int i;
	for (i = 0; i < (int)str.length(); ++i) {
	    QVERIFY(b->breaks[i] != 0xff);
	    if ( (bool)attrs[i].softBreak != (bool)b->breaks[i] ) {
		qDebug("test case \"%s\" failed at char %d", b->utf8, i);
		QCOMPARE( (bool)attrs[i].softBreak, (bool)b->breaks[i] );
	    }
	}
	QCOMPARE(b->breaks[i], (uchar)0xff);
	++b;
    }
#else
    QSKIP("This test can not be run on non-X11 platforms", SkipAll);
#endif
}

void tst_QTextLayout::simpleBoundingRect()
{
    /* just check if boundingRect() gives sane values. The text is not broken. */

    QString hello("hello world");

    const int width = hello.length() * testFont.pixelSize();

    QTextLayout layout(hello, testFont);
    layout.beginLayout();

    QTextLine line = layout.createLine();
    line.setLineWidth(width);
    QCOMPARE(line.textLength(), hello.length());
    QCOMPARE(layout.boundingRect(), QRectF(0, 0, width, QFontMetrics(testFont).height()));
}

void tst_QTextLayout::threeLineBoundingRect()
{
#if defined(Q_WS_MAC)
    QSKIP("QTestFontEngine on the mac does not support logclusters at the moment", SkipAll);
#endif
    /* stricter check. break text into three lines */

    QString firstWord("hello");
    QString secondWord("world");
    QString thirdWord("test");
    QString text(firstWord + ' ' + secondWord + ' ' + thirdWord);

    const int firstLineWidth = firstWord.length() * testFont.pixelSize();
    const int secondLineWidth = secondWord.length() * testFont.pixelSize();
    const int thirdLineWidth = thirdWord.length() * testFont.pixelSize();

    const int longestLine = qMax(firstLineWidth, qMax(secondLineWidth, thirdLineWidth));

    QTextLayout layout(text, testFont);
    layout.beginLayout();

    int pos = 0;
    int y = 0;
    QTextLine line = layout.createLine();
    line.setLineWidth(firstLineWidth);
    line.setPosition(QPoint(0, y));
    QCOMPARE(line.textStart(), pos);
    // + 1 for trailing space
    QCOMPARE(line.textLength(), firstWord.length() + 1);
    QCOMPARE(qRound(line.naturalTextWidth()), firstLineWidth);

    pos += line.textLength();
    y += qRound(line.ascent() + line.descent());

    line = layout.createLine();
    line.setLineWidth(secondLineWidth);
    line.setPosition(QPoint(0, y));
    // + 1 for trailing space
    QCOMPARE(line.textStart(), pos);
    QCOMPARE(line.textLength(), secondWord.length() + 1);
    QCOMPARE(qRound(line.naturalTextWidth()), secondLineWidth);

    pos += line.textLength();
    y += qRound(line.ascent() + line.descent());

    line = layout.createLine();
    line.setLineWidth(secondLineWidth);
    line.setPosition(QPoint(0, y));
    // no trailing space here!
    QCOMPARE(line.textStart(), pos);
    QCOMPARE(line.textLength(), thirdWord.length());
    QCOMPARE(qRound(line.naturalTextWidth()), thirdLineWidth);
    y += qRound(line.ascent() + line.descent());

    QCOMPARE(layout.boundingRect(), QRectF(0, 0, longestLine, y + 1));
}

void tst_QTextLayout::forcedBreaks()
{
    QString text = "A\n\nB\nC";
    text.replace('\n', QChar::LineSeparator);

    QTextLayout layout(text, testFont);

    layout.beginLayout();

    int pos = 0;

    QTextLine line = layout.createLine();
    line.setLineWidth(0x10000);
    QCOMPARE(line.textStart(), pos);
    QCOMPARE(line.textLength(),2);
    QCOMPARE(qRound(line.naturalTextWidth()),testFont.pixelSize());
    QCOMPARE((int) line.height(), testFont.pixelSize() + 1); // + 1 baseline
    QCOMPARE(line.xToCursor(0), line.textStart());
    pos += line.textLength();

    line = layout.createLine();
    line.setLineWidth(0x10000);
    QCOMPARE(line.textStart(),pos);
    QCOMPARE(line.textLength(),1);
    QCOMPARE(qRound(line.naturalTextWidth()), 0);
    QCOMPARE((int) line.height(), testFont.pixelSize() + 1); // + 1 baseline
    QCOMPARE(line.xToCursor(0), line.textStart());
    pos += line.textLength();

    line = layout.createLine();
    line.setLineWidth(0x10000);
    QCOMPARE(line.textStart(),pos);
    QCOMPARE(line.textLength(),2);
    QCOMPARE(qRound(line.naturalTextWidth()),testFont.pixelSize());
    QCOMPARE(qRound(line.height()), testFont.pixelSize() + 1); // + 1 baseline
    QCOMPARE(line.xToCursor(0), line.textStart());
    pos += line.textLength();

    line = layout.createLine();
    line.setLineWidth(0x10000);
    QCOMPARE(line.textStart(),pos);
    QCOMPARE(line.textLength(),1);
    QCOMPARE(qRound(line.naturalTextWidth()), testFont.pixelSize());
    QCOMPARE((int) line.height(), testFont.pixelSize() + 1); // + 1 baseline
    QCOMPARE(line.xToCursor(0), line.textStart());
}

void tst_QTextLayout::breakAny()
{
#if defined(Q_WS_MAC)
    QSKIP("QTestFontEngine on the mac does not support logclusters at the moment", SkipAll);
#endif
    QString text = "ABCD";

    QTextLayout layout(text, testFont);
    QTextLine line;

    QTextOption opt;
    opt.setWrapMode(QTextOption::WrapAnywhere);
    layout.setTextOption(opt);
    layout.beginLayout();

    line = layout.createLine();
    line.setLineWidth(testFont.pixelSize() * 2);
    QCOMPARE(line.textStart(), 0);
    QCOMPARE(line.textLength(), 2);

    line = layout.createLine();
    line.setLineWidth(testFont.pixelSize() * 2);
    QCOMPARE(line.textStart(), 2);
    QCOMPARE(line.textLength(), 2);

    line = layout.createLine();
    QVERIFY(!line.isValid());

    layout.endLayout();

    text = "ABCD EFGH";
    layout.setText(text);
    layout.beginLayout();

    line = layout.createLine();
    line.setLineWidth(testFont.pixelSize() * 7);
    QCOMPARE(line.textStart(), 0);
    QCOMPARE(line.textLength(), 7);

    layout.endLayout();
}

void tst_QTextLayout::cursorToXForInlineObjects()
{
    QChar ch(QChar::ObjectReplacementCharacter);
    QString text(ch);
    QTextLayout layout(text, testFont);
    layout.beginLayout();

    QTextEngine *engine = layout.engine();
    const int item = engine->findItem(0);
    engine->layoutData->items[item].width = 32;

    QTextLine line = layout.createLine();
    line.setLineWidth(0x10000);

    QCOMPARE(line.cursorToX(0), qreal(0));
    QCOMPARE(line.cursorToX(1), qreal(32));
}

void tst_QTextLayout::defaultWordSeparators_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<int>("startPos");
    QTest::addColumn<int>("endPos");

    QString separators(".,:;-<>[](){}=");
    separators += QChar(QChar::Nbsp);
    separators += QLatin1String("!?");
    separators += QLatin1Char('\t');
    for (int i = 0; i < separators.count(); ++i) {
        QTest::newRow(QString::number(i).toAscii().data())
            << QString::fromLatin1("abcd") + separators.at(i) + QString::fromLatin1("efgh")
            <<  0 << 4;
    }

    QTest::newRow("lineseparator")
            << QString::fromLatin1("abcd") + QString(QChar::LineSeparator) + QString::fromLatin1("efgh")
            << 0 << 5;
}

void tst_QTextLayout::defaultWordSeparators()
{
    QFETCH(QString, text);
    QFETCH(int, startPos);
    QFETCH(int, endPos);
    QTextLayout layout(text, testFont);

    QCOMPARE(layout.nextCursorPosition(startPos, QTextLayout::SkipWords), endPos);
    QCOMPARE(layout.previousCursorPosition(endPos, QTextLayout::SkipWords), startPos);
}

void tst_QTextLayout::cursorMovementInsideSpaces()
{
    QTextLayout layout("ABC            DEF", testFont);

    QCOMPARE(layout.previousCursorPosition(6, QTextLayout::SkipWords), 0);
    QCOMPARE(layout.nextCursorPosition(6, QTextLayout::SkipWords), 15);
}

void tst_QTextLayout::charWordStopOnLineSeparator()
{
    const QChar lineSeparator(QChar::LineSeparator);
    QString txt;
    txt.append(lineSeparator);
    txt.append(lineSeparator);
    QTextLayout layout(txt, testFont);
    QTextEngine *engine = layout.engine();
    const QCharAttributes *attrs = engine->attributes();
    QVERIFY(attrs);
    QVERIFY(attrs[1].charStop);
}

void tst_QTextLayout::xToCursorAtEndOfLine()
{
#if defined(Q_WS_MAC)
    QSKIP("QTestFontEngine on the mac does not support logclusters at the moment", SkipAll);
#endif
    QString text = "FirstLine SecondLine";
    text.replace('\n', QChar::LineSeparator);

    const qreal firstLineWidth = QString("FirstLine").length() * testFont.pixelSize();

    QTextLayout layout(text, testFont);

    layout.beginLayout();
    QTextLine line = layout.createLine();
    QVERIFY(line.isValid());
    line.setLineWidth(firstLineWidth);
    QVERIFY(layout.createLine().isValid());
    QVERIFY(!layout.createLine().isValid());
    layout.endLayout();

    line = layout.lineAt(0);
    QCOMPARE(line.xToCursor(100000), 9);
    line = layout.lineAt(1);
    QCOMPARE(line.xToCursor(100000), 20);
}

QTEST_MAIN(tst_QTextLayout)
#include "tst_qtextlayout.moc"
