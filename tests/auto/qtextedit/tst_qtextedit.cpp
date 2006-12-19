/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>


#include <qtextedit.h>
#include <qtextcursor.h>
#include <qtextlist.h>
#include <qdebug.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qtextbrowser.h>
#include <private/qtextcontrol_p.h>
#include <qscrollbar.h>
#include <qtextobject.h>

#include <qabstracttextdocumentlayout.h>
#include <qtextdocumentfragment.h>

//Used in copyAvailable
typedef QPair<Qt::Key, Qt::KeyboardModifier> keyPairType;
typedef QList<keyPairType> pairListType;
Q_DECLARE_METATYPE(pairListType);
Q_DECLARE_METATYPE(keyPairType);
Q_DECLARE_METATYPE(QList<bool>);

#ifdef Q_WS_MAC
#include <Carbon/Carbon.h>
#endif

class QTextEdit;

//TESTED_CLASS=
//TESTED_FILES=gui/widgets/qtextedit.h gui/widgets/qtextedit.cpp

class tst_QTextEdit : public QObject
{
    Q_OBJECT
public:
    tst_QTextEdit();

public slots:
    void init();
    void cleanup();
private slots:
    void getSetCheck();
    void inlineAttributesOnInsert();
    void inlineAttributesOnSelection();
    void inlineAttributeSymmetry();
    void inlineAttributeSymmetryWithSelection();
    void autoBulletList1();
    void autoBulletList2();
    void preserveCharFormatAfterNewline();
    void clearMustNotChangeClipboard();
    void clearMustNotResetRootFrameMarginToDefault();
    void clearShouldPreserveTheCurrentCharFormat();
    void paragSeparatorOnPlaintextAppend();
    void layoutingLoop();
    void selectAllSetsNotSelection();
    void asciiTab();
    void setDocument();
    void mergeCurrentCharFormat();
    void mergeCurrentBlockCharFormat();
    void emptyAppend();
    void appendOnEmptyDocumentShouldReuseInitialParagraph();
#ifdef QT3_SUPPORT
    void textSemantics();
#endif
    void cursorPositionChanged();
    void setTextCursor();
    void undoAvailableAfterPaste();
    void appendShouldUseCurrentFormat();
    void appendShouldNotTouchTheSelection();
    void backspace();
    void undoRedo();
    void preserveCharFormatInAppend();
    void copyAndSelectAllInReadonly();
    void ctrlAltInput();
    void noPropertiesOnDefaultTextEditCharFormat();
    void setPlainTextShouldUseCurrentCharFormat();
    void setPlainTextShouldEmitTextChangedOnce();
    void overwriteMode();
    void shiftDownInLineLastShouldSelectToEnd_data();
    void shiftDownInLineLastShouldSelectToEnd();
    void undoRedoShouldRepositionTextEditCursor();
    void lineWrapModes();
    void mouseCursorShape();
    void implicitClear();
    void undoRedoAfterSetContent();
    void numPadKeyNavigation();
    void moveCursor();
    void mimeDataReimplementations();
    void ctrlEnterShouldInsertLineSeparator();
    void selectWordsFromStringsContainingSeparators_data();
    void selectWordsFromStringsContainingSeparators();
    void canPaste();
    void copyAvailable_data();
    void copyAvailable();
    void ensureCursorVisibleOnInitialShow();
    void setHtmlInsideResizeEvent();
    void colorfulAppend();
    void ensureVisibleWithRtl();
    void preserveCharFormatAfterSetPlainText();
    void extraSelections();
    void adjustScrollbars();

private:
    void createSelection();
    int blockCount() const;
    bool nativeClipboardWorking();

    QTextEdit *ed;
    qreal rootFrameMargin;
};

bool tst_QTextEdit::nativeClipboardWorking()
{
#ifdef Q_WS_MAC
    PasteboardRef pasteboard;
    OSStatus status = PasteboardCreate(0, &pasteboard);
    if (status == noErr)
        CFRelease(pasteboard);
    return status == noErr;
#endif
    return true;
}

// Testing get/set functions
void tst_QTextEdit::getSetCheck()
{
    QTextEdit obj1;
    // QTextDocument * QTextEdit::document()
    // void QTextEdit::setDocument(QTextDocument *)
    QTextDocument *var1 = new QTextDocument;
    obj1.setDocument(var1);
    QCOMPARE(var1, obj1.document());
    obj1.setDocument((QTextDocument *)0);
    QVERIFY(var1 != obj1.document()); // QTextEdit creates a new document when setting 0
    QVERIFY((QTextDocument *)0 != obj1.document());
    delete var1;

    // AutoFormatting QTextEdit::autoFormatting()
    // void QTextEdit::setAutoFormatting(AutoFormatting)
    obj1.setAutoFormatting(QTextEdit::AutoFormatting(QTextEdit::AutoNone));
    QCOMPARE(QTextEdit::AutoFormatting(QTextEdit::AutoNone), obj1.autoFormatting());
    obj1.setAutoFormatting(QTextEdit::AutoFormatting(QTextEdit::AutoBulletList));
    QCOMPARE(QTextEdit::AutoFormatting(QTextEdit::AutoBulletList), obj1.autoFormatting());
    obj1.setAutoFormatting(QTextEdit::AutoFormatting(QTextEdit::AutoAll));
    QCOMPARE(QTextEdit::AutoFormatting(QTextEdit::AutoAll), obj1.autoFormatting());

    // bool QTextEdit::tabChangesFocus()
    // void QTextEdit::setTabChangesFocus(bool)
    obj1.setTabChangesFocus(false);
    QCOMPARE(false, obj1.tabChangesFocus());
    obj1.setTabChangesFocus(true);
    QCOMPARE(true, obj1.tabChangesFocus());

    // LineWrapMode QTextEdit::lineWrapMode()
    // void QTextEdit::setLineWrapMode(LineWrapMode)
    obj1.setLineWrapMode(QTextEdit::LineWrapMode(QTextEdit::NoWrap));
    QCOMPARE(QTextEdit::LineWrapMode(QTextEdit::NoWrap), obj1.lineWrapMode());
    obj1.setLineWrapMode(QTextEdit::LineWrapMode(QTextEdit::WidgetWidth));
    QCOMPARE(QTextEdit::LineWrapMode(QTextEdit::WidgetWidth), obj1.lineWrapMode());
    obj1.setLineWrapMode(QTextEdit::LineWrapMode(QTextEdit::FixedPixelWidth));
    QCOMPARE(QTextEdit::LineWrapMode(QTextEdit::FixedPixelWidth), obj1.lineWrapMode());
    obj1.setLineWrapMode(QTextEdit::LineWrapMode(QTextEdit::FixedColumnWidth));
    QCOMPARE(QTextEdit::LineWrapMode(QTextEdit::FixedColumnWidth), obj1.lineWrapMode());

    // int QTextEdit::lineWrapColumnOrWidth()
    // void QTextEdit::setLineWrapColumnOrWidth(int)
    obj1.setLineWrapColumnOrWidth(0);
    QCOMPARE(0, obj1.lineWrapColumnOrWidth());
    obj1.setLineWrapColumnOrWidth(INT_MIN);
    QCOMPARE(INT_MIN, obj1.lineWrapColumnOrWidth());
    obj1.setLineWrapColumnOrWidth(INT_MAX);
    QCOMPARE(INT_MAX, obj1.lineWrapColumnOrWidth());

    // bool QTextEdit::overwriteMode()
    // void QTextEdit::setOverwriteMode(bool)
    obj1.setOverwriteMode(false);
    QCOMPARE(false, obj1.overwriteMode());
    obj1.setOverwriteMode(true);
    QCOMPARE(true, obj1.overwriteMode());

    // int QTextEdit::tabStopWidth()
    // void QTextEdit::setTabStopWidth(int)
    obj1.setTabStopWidth(0);
    QCOMPARE(0, obj1.tabStopWidth());
    obj1.setTabStopWidth(INT_MIN);
    QCOMPARE(0, obj1.tabStopWidth()); // Makes no sense to set a negative tabstop value
    obj1.setTabStopWidth(INT_MAX);
    QCOMPARE(INT_MAX, obj1.tabStopWidth());

    // bool QTextEdit::acceptRichText()
    // void QTextEdit::setAcceptRichText(bool)
    obj1.setAcceptRichText(false);
    QCOMPARE(false, obj1.acceptRichText());
    obj1.setAcceptRichText(true);
    QCOMPARE(true, obj1.acceptRichText());

    // qreal QTextEdit::fontPointSize()
    // void QTextEdit::setFontPointSize(qreal)
    obj1.setFontPointSize(1.1);
    QCOMPARE(1.1, obj1.fontPointSize());
    // we currently Q_ASSERT_X in QFont::setPointSizeF for that
    //obj1.setFontPointSize(0.0);
    //QCOMPARE(1.1, obj1.fontPointSize()); // Should not accept 0.0 => keep old

    // int QTextEdit::fontWeight()
    // void QTextEdit::setFontWeight(int)
    obj1.setFontWeight(1);
    QCOMPARE(1, obj1.fontWeight()); // Range<1, 99>
    obj1.setFontWeight(99);
    QCOMPARE(99, obj1.fontWeight()); // Range<1, 99>
    /* Q_ASSERT_X in qfont.cpp
    obj1.setFontWeight(INT_MIN);
    QCOMPARE(1, obj1.fontWeight()); // Range<1, 99>
    obj1.setFontWeight(INT_MAX);
    QCOMPARE(99, obj1.fontWeight()); // Range<1, 99>
    */

    // bool QTextEdit::fontUnderline()
    // void QTextEdit::setFontUnderline(bool)
    obj1.setFontUnderline(false);
    QCOMPARE(false, obj1.fontUnderline());
    obj1.setFontUnderline(true);
    QCOMPARE(true, obj1.fontUnderline());

    // bool QTextEdit::fontItalic()
    // void QTextEdit::setFontItalic(bool)
    obj1.setFontItalic(false);
    QCOMPARE(false, obj1.fontItalic());
    obj1.setFontItalic(true);
    QCOMPARE(true, obj1.fontItalic());
}

class QtTestDocumentLayout : public QAbstractTextDocumentLayout
{
    Q_OBJECT
public:
    inline QtTestDocumentLayout(QTextEdit *edit, QTextDocument *doc, int &itCount)
        : QAbstractTextDocumentLayout(doc), useBiggerSize(false), ed(edit), iterationCounter(itCount) {}

    virtual void draw(QPainter *, const QAbstractTextDocumentLayout::PaintContext &)  {}

    virtual int hitTest(const QPointF &, Qt::HitTestAccuracy ) const { return 0; }

    virtual void documentChanged(int, int, int) {}

    virtual int pageCount() const { return 1; }

    virtual QSizeF documentSize() const { return usedSize; }

    virtual QRectF frameBoundingRect(QTextFrame *) const { return QRectF(); }
    virtual QRectF blockBoundingRect(const QTextBlock &) const { return QRectF(); }

    bool useBiggerSize;
    QSize usedSize;

    QTextEdit *ed;

    int &iterationCounter;
};

tst_QTextEdit::tst_QTextEdit()
{}

void tst_QTextEdit::init()
{
    ed = new QTextEdit(0);
    rootFrameMargin = 2.0;
#if QT_VERSION < 0x040200
    rootFrameMargin = 4.0;
#endif
}

void tst_QTextEdit::cleanup()
{
    delete ed;
    ed = 0;
}

void tst_QTextEdit::inlineAttributesOnInsert()
{
    QVERIFY(ed->textCursor().charFormat().foreground().color() != Qt::blue);

    ed->setTextColor(Qt::blue);
    QTest::keyClick(ed, Qt::Key_A);

    QVERIFY(ed->textCursor().charFormat().foreground().color() == Qt::blue);
}

void tst_QTextEdit::inlineAttributesOnSelection()
{
    createSelection();

    ed->setFontItalic(true);

    QVERIFY(ed->textCursor().charFormat().fontItalic());
}

void tst_QTextEdit::inlineAttributeSymmetry()
{
    ed->setFontPointSize(42.0);
    QCOMPARE(double(ed->fontPointSize()), 42.0);

    ed->setFontFamily("Test");
    QCOMPARE(ed->fontFamily(), QString("Test"));

    ed->setFontWeight(QFont::Bold);
    QCOMPARE((int)ed->fontWeight(), (int)QFont::Bold);

    ed->setFontUnderline(true);
    QCOMPARE(ed->fontUnderline(), true);

    ed->setFontItalic(true);
    QCOMPARE(ed->fontItalic(), true);

    ed->setTextColor(Qt::blue);
    QCOMPARE(ed->textColor(), QColor(Qt::blue));

    ed->setAlignment(Qt::AlignRight);
    QCOMPARE((int)ed->alignment(), (int)Qt::AlignRight);
}

void tst_QTextEdit::inlineAttributeSymmetryWithSelection()
{
    createSelection();

    inlineAttributeSymmetry();
}

void tst_QTextEdit::autoBulletList1()
{
    ed->setAutoFormatting(QTextEdit::AutoBulletList);

    QTest::keyClick(ed, Qt::Key_Return);
    QTest::keyClicks(ed, "*This should become a list");

    QVERIFY(ed->textCursor().currentList());
    QVERIFY(ed->textCursor().currentList()->format().style() == QTextListFormat::ListDisc);
}

void tst_QTextEdit::autoBulletList2()
{
    ed->setAutoFormatting(QTextEdit::AutoNone);
    QTest::keyClick(ed, Qt::Key_Return);
    QTest::keyClicks(ed, "*This should NOT become a list");

    QVERIFY(!ed->textCursor().currentList());
}

void tst_QTextEdit::preserveCharFormatAfterNewline()
{
    ed->setTextColor(Qt::blue);
    QTest::keyClicks(ed, "Hello");

    QTest::keyClick(ed, Qt::Key_Return);

    QCOMPARE(ed->textColor(), QColor(Qt::blue));
}

void tst_QTextEdit::createSelection()
{
    QTest::keyClicks(ed, "Hello World");
    /* go to start */
#ifndef Q_WS_MAC
    QTest::keyClick(ed, Qt::Key_Home, Qt::ControlModifier);
#else
    QTest::keyClick(ed, Qt::Key_Home);
#endif
    QCOMPARE(ed->textCursor().position(), 0);
    /* select until end of text */
#ifndef Q_WS_MAC
    QTest::keyClick(ed, Qt::Key_End, Qt::ControlModifier | Qt::ShiftModifier);
#else
    QTest::keyClick(ed, Qt::Key_End, Qt::ShiftModifier);
#endif
    QCOMPARE(ed->textCursor().position(), 11);
}

void tst_QTextEdit::clearMustNotChangeClipboard()
{
    if (!nativeClipboardWorking())
        QSKIP("Clipboard not working with cron-started unit tests", SkipAll);
    ed->textCursor().insertText("Hello World");
    QString txt("This is different text");
    QApplication::clipboard()->setText(txt);
    ed->clear();
    QCOMPARE(QApplication::clipboard()->text(), txt);
}

void tst_QTextEdit::clearMustNotResetRootFrameMarginToDefault()
{
    QCOMPARE(ed->document()->rootFrame()->frameFormat().margin(), rootFrameMargin);
    ed->clear();
    QCOMPARE(ed->document()->rootFrame()->frameFormat().margin(), rootFrameMargin);
}

void tst_QTextEdit::clearShouldPreserveTheCurrentCharFormat()
{
    ed->setFontUnderline(true);
    QVERIFY(ed->fontUnderline());
    ed->clear();
    QVERIFY(ed->fontUnderline());
}

void tst_QTextEdit::paragSeparatorOnPlaintextAppend()
{
    ed->append("Hello\nWorld");
    int cnt = 0;
    QTextBlock blk = ed->document()->begin();
    while (blk.isValid()) {
        ++cnt;
        blk = blk.next();
    }
    QCOMPARE(cnt, 2);
}

void tst_QTextEdit::layoutingLoop()
{
    QPointer<QTextEdit> ed = new QTextEdit(0);
    // this is a testcase for an ugly layouting problem, causing an infinite loop.
    // QTextEdit's resizeEvent has a long comment about what and why it can happen.

    int callsToSetPageSize = 0;

    QTextDocument *doc = new QTextDocument;
    QtTestDocumentLayout *lout = new QtTestDocumentLayout(ed, doc, callsToSetPageSize);
    doc->setDocumentLayout(lout);
    ed->setDocument(doc);

    ed->show();
    ed->resize(100, 100);

    qApp->processEvents();
    delete doc;
    delete ed;

    // ###### should need less!
    QVERIFY(callsToSetPageSize < 10);
}

void tst_QTextEdit::selectAllSetsNotSelection()
{
    if (!QApplication::clipboard()->supportsSelection()) {
        QSKIP("Test only relevant for systems with selection", SkipAll);
        return;
    }

    QApplication::clipboard()->setText(QString("foobar"), QClipboard::Selection);
    QVERIFY(QApplication::clipboard()->text(QClipboard::Selection) == QString("foobar"));

    ed->insertPlainText("Hello World");
    ed->selectAll();

    QCOMPARE(QApplication::clipboard()->text(QClipboard::Selection), QString::fromAscii("foobar"));
}

void tst_QTextEdit::asciiTab()
{
    QTextEdit edit;
    edit.setPlainText("\t");
    edit.show();
    qApp->processEvents();
    QCOMPARE(edit.toPlainText().at(0), QChar('\t'));
}

void tst_QTextEdit::setDocument()
{
    QTextDocument *document = new QTextDocument(ed);
    QTextCursor(document).insertText("Test");
    ed->setDocument(document);
    QCOMPARE(ed->toPlainText(), QString("Test"));
}

void tst_QTextEdit::mergeCurrentCharFormat()
{
    ed->setPlainText("Hello Test World");
    QTextCursor cursor = ed->textCursor();
    cursor.setPosition(7);
    ed->setTextCursor(cursor);

    QTextCharFormat mod;
    mod.setFontItalic(true);
    mod.setForeground(Qt::red);
    ed->mergeCurrentCharFormat(mod);

    cursor.movePosition(QTextCursor::Right);
    cursor.movePosition(QTextCursor::Right);
    // do NOT select the current word under the cursor, /JUST/
    // call mergeCharFormat on the cursor
    QVERIFY(!cursor.charFormat().fontItalic());
    QVERIFY(cursor.charFormat().foreground().color() != Qt::red);
}

void tst_QTextEdit::mergeCurrentBlockCharFormat()
{
    ed->setPlainText("FirstLine\n\nThirdLine");
    QTextCursor cursor = ed->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::Down);

    // make sure we're in the empty second line
    QVERIFY(cursor.atBlockStart());
    QVERIFY(cursor.atBlockEnd());

    ed->setTextCursor(cursor);

    QTextCharFormat mod;
    mod.setForeground(Qt::red);
    ed->mergeCurrentCharFormat(mod);

    QVERIFY(cursor.blockCharFormat().foreground().color() != Qt::red);
    cursor.movePosition(QTextCursor::Up);
    QVERIFY(cursor.blockCharFormat().foreground().color() != Qt::red);
    cursor.movePosition(QTextCursor::Down);
    cursor.movePosition(QTextCursor::Down);
    QVERIFY(cursor.blockCharFormat().foreground().color() != Qt::red);
}

int tst_QTextEdit::blockCount() const
{
    int blocks = 0;
    for (QTextBlock block = ed->document()->begin(); block.isValid(); block = block.next())
        ++blocks;
    return blocks;
}

// Supporter issue #56783
void tst_QTextEdit::emptyAppend()
{
    ed->append("Blah");
    QCOMPARE(blockCount(), 1);
    ed->append(QString::null);
    QCOMPARE(blockCount(), 2);
    ed->append(QString("   "));
    QCOMPARE(blockCount(), 3);
}

void tst_QTextEdit::appendOnEmptyDocumentShouldReuseInitialParagraph()
{
    QCOMPARE(blockCount(), 1);
    ed->append("Blah");
    QCOMPARE(blockCount(), 1);
}

#ifdef QT3_SUPPORT
void tst_QTextEdit::textSemantics()
{
    ed->setTextFormat(Qt::AutoText);

    ed->setPlainText("Hello World");
    QVERIFY(!Qt::mightBeRichText(ed->text()));

    ed->setHtml("<b>Hey</b>");
    QVERIFY(Qt::mightBeRichText(ed->text()));
}
#endif

class CursorPositionChangedRecorder : public QObject
{
    Q_OBJECT
public:
    inline CursorPositionChangedRecorder(QTextEdit *ed)
        : editor(ed)
    {
        connect(editor, SIGNAL(cursorPositionChanged()), this, SLOT(recordCursorPos()));
    }

    QList<int> cursorPositions;

private slots:
    void recordCursorPos()
    {
        cursorPositions.append(editor->textCursor().position());
    }

private:
    QTextEdit *editor;
};

void tst_QTextEdit::cursorPositionChanged()
{
    QSignalSpy spy(ed, SIGNAL(cursorPositionChanged()));

    spy.clear();
    QTest::keyClick(ed, Qt::Key_A);
    QCOMPARE(spy.count(), 1);

    QTextCursor cursor = ed->textCursor();
    cursor.movePosition(QTextCursor::Start);
    ed->setTextCursor(cursor);
    cursor.movePosition(QTextCursor::End);
    spy.clear();
    cursor.insertText("Test");
    QCOMPARE(spy.count(), 0);

    cursor.movePosition(QTextCursor::End);
    ed->setTextCursor(cursor);
    cursor.movePosition(QTextCursor::Start);
    spy.clear();
    cursor.insertText("Test");
    QCOMPARE(spy.count(), 1);

    spy.clear();
    QTest::keyClick(ed, Qt::Key_Left);
    QCOMPARE(spy.count(), 1);

    CursorPositionChangedRecorder spy2(ed);
    QVERIFY(ed->textCursor().position() > 0);
    ed->setPlainText("Hello World");
    QCOMPARE(spy2.cursorPositions.count(), 1);
    QCOMPARE(spy2.cursorPositions.at(0), 0);
    QCOMPARE(ed->textCursor().position(), 0);
}

void tst_QTextEdit::setTextCursor()
{
    QSignalSpy spy(ed, SIGNAL(cursorPositionChanged()));

    ed->setPlainText("Test");
    QTextCursor cursor = ed->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::NextCharacter);

    spy.clear();

    ed->setTextCursor(cursor);
    QCOMPARE(spy.count(), 1);
}

void tst_QTextEdit::undoAvailableAfterPaste()
{
    if (!nativeClipboardWorking())
        QSKIP("Clipboard not working with cron-started unit tests", SkipAll);

    QSignalSpy spy(ed->document(), SIGNAL(undoAvailable(bool)));

    const QString txt("Test");
    QApplication::clipboard()->setText(txt);
    ed->paste();
    QVERIFY(spy.count() >= 1);
    QCOMPARE(ed->toPlainText(), txt);
}

void tst_QTextEdit::appendShouldUseCurrentFormat()
{
    ed->textCursor().insertText("A");
    QTextCharFormat fmt;
    fmt.setForeground(Qt::blue);
    fmt.setFontItalic(true);
    ed->setCurrentCharFormat(fmt);
    ed->append("Hello");

    QTextCursor cursor(ed->document());

    QVERIFY(cursor.movePosition(QTextCursor::NextCharacter));
    QVERIFY(cursor.charFormat().foreground().color() != Qt::blue);
    QVERIFY(!cursor.charFormat().fontItalic());

    QVERIFY(cursor.movePosition(QTextCursor::NextBlock));

    {
        QTextCursor tmp = cursor;
        tmp.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        QCOMPARE(tmp.selectedText(), QString::fromLatin1("Hello"));
    }

    QVERIFY(cursor.movePosition(QTextCursor::NextCharacter));
    QVERIFY(cursor.charFormat().foreground().color() == Qt::blue);
    QVERIFY(cursor.charFormat().fontItalic());
}

void tst_QTextEdit::appendShouldNotTouchTheSelection()
{
    QTextCursor cursor(ed->document());
    QTextCharFormat fmt;
    fmt.setForeground(Qt::blue);
    cursor.insertText("H", fmt);
    fmt.setForeground(Qt::red);
    cursor.insertText("ey", fmt);

    cursor.insertText("some random text inbetween");

    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    QCOMPARE(cursor.charFormat().foreground().color(), QColor(Qt::blue));
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    QCOMPARE(cursor.charFormat().foreground().color(), QColor(Qt::red));
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    QCOMPARE(cursor.charFormat().foreground().color(), QColor(Qt::red));
    QCOMPARE(cursor.selectedText(), QString("Hey"));

    ed->setTextCursor(cursor);
    QVERIFY(ed->textCursor().hasSelection());

    ed->append("<b>Some Bold Text</b>");
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::NextCharacter);
    QCOMPARE(cursor.charFormat().foreground().color(), QColor(Qt::blue));
}

void tst_QTextEdit::backspace()
{
    QTextCursor cursor = ed->textCursor();

    QTextListFormat listFmt;
    listFmt.setStyle(QTextListFormat::ListDisc);
    listFmt.setIndent(1);
    cursor.insertList(listFmt);
    cursor.insertText("A");

    ed->setTextCursor(cursor);

    // delete 'A'
    QTest::keyClick(ed, Qt::Key_Backspace);
    QVERIFY(ed->textCursor().currentList());
    // delete list
    QTest::keyClick(ed, Qt::Key_Backspace);
    QVERIFY(!ed->textCursor().currentList());
    QCOMPARE(ed->textCursor().blockFormat().indent(), 1);
    // outdent paragraph
    QTest::keyClick(ed, Qt::Key_Backspace);
    QCOMPARE(ed->textCursor().blockFormat().indent(), 0);
}

void tst_QTextEdit::undoRedo()
{
    ed->clear();
    QTest::keyClicks(ed, "abc d");
    QCOMPARE(ed->text(), QString("abc d"));
    ed->undo();
    QCOMPARE(ed->text(), QString());
    ed->redo();
    QCOMPARE(ed->text(), QString("abc d"));
#ifdef Q_WS_WIN
    // shortcut for undo
    QTest::keyClick(ed, Qt::Key_Backspace, Qt::AltModifier);
    QCOMPARE(ed->text(), QString());
    // shortcut for redo
    QTest::keyClick(ed, Qt::Key_Backspace, Qt::ShiftModifier|Qt::AltModifier);
    QCOMPARE(ed->text(), QString("abc d"));
#endif
}

// Task #70465
void tst_QTextEdit::preserveCharFormatInAppend()
{
    ed->append("First para");
    ed->append("<b>Second para</b>");
    ed->append("third para");

    QTextCursor cursor(ed->textCursor());

    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::NextCharacter);
    QCOMPARE(cursor.charFormat().fontWeight(), (int)QFont::Normal);
    QCOMPARE(cursor.block().text(), QString("First para"));

    cursor.movePosition(QTextCursor::NextBlock);
    cursor.movePosition(QTextCursor::NextCharacter);
    QCOMPARE(cursor.charFormat().fontWeight(), (int)QFont::Bold);
    QCOMPARE(cursor.block().text(), QString("Second para"));

    cursor.movePosition(QTextCursor::NextBlock);
    cursor.movePosition(QTextCursor::NextCharacter);
    QCOMPARE(cursor.charFormat().fontWeight(), (int)QFont::Normal);
    QCOMPARE(cursor.block().text(), QString("third para"));
}

void tst_QTextEdit::copyAndSelectAllInReadonly()
{
    if (!nativeClipboardWorking())
        QSKIP("Clipboard not working with cron-started unit tests", SkipAll);

    ed->setReadOnly(true);
    ed->setPlainText("Hello World");

    QTextCursor cursor = ed->textCursor();
    cursor.clearSelection();
    ed->setTextCursor(cursor);
    QVERIFY(!ed->textCursor().hasSelection());

    QCOMPARE(ed->toPlainText(), QString("Hello World"));

    // shouldn't do anything
    QTest::keyClick(ed, Qt::Key_A);

    QCOMPARE(ed->toPlainText(), QString("Hello World"));

    QTest::keyClick(ed, Qt::Key_A, Qt::ControlModifier);

    QVERIFY(ed->textCursor().hasSelection());

    QApplication::clipboard()->setText(QString());
    QVERIFY(QApplication::clipboard()->text().isEmpty());

    QTest::keyClick(ed, Qt::Key_C, Qt::ControlModifier);
    QCOMPARE(QApplication::clipboard()->text(), QString("Hello World"));
}

void tst_QTextEdit::ctrlAltInput()
{
    QTest::keyClick(ed, Qt::Key_At, Qt::ControlModifier | Qt::AltModifier);
    QCOMPARE(ed->toPlainText(), QString("@"));
}

void tst_QTextEdit::noPropertiesOnDefaultTextEditCharFormat()
{
    // there should be no properties set on the default/initial char format
    // on a text edit. Font properties instead should be taken from the
    // widget's font (in sync with defaultFont property in document) and the
    // foreground color should be taken from the palette.
    QCOMPARE(ed->currentCharFormat().properties().count(), 0);
}

void tst_QTextEdit::setPlainTextShouldUseCurrentCharFormat()
{
    ed->setFontUnderline(true);
    ed->setPlainText("Hello World");
    QTextCursor cursor(ed->document());
    cursor.movePosition(QTextCursor::NextCharacter);
    QCOMPARE(cursor.charFormat().fontUnderline(), true);
}

void tst_QTextEdit::setPlainTextShouldEmitTextChangedOnce()
{
    QSignalSpy spy(ed, SIGNAL(textChanged()));
    ed->setPlainText("Yankee Doodle");
    QCOMPARE(spy.count(), 1);
    ed->setPlainText("");
    QCOMPARE(spy.count(), 2);
}

void tst_QTextEdit::overwriteMode()
{
    QVERIFY(!ed->overwriteMode());
    QTest::keyClicks(ed, "Some first text");

    QCOMPARE(ed->toPlainText(), QString("Some first text"));

    ed->setOverwriteMode(true);

    QTextCursor cursor = ed->textCursor();
    cursor.setPosition(5);
    ed->setTextCursor(cursor);

    QTest::keyClicks(ed, "shiny");
    QCOMPARE(ed->toPlainText(), QString("Some shiny text"));

    cursor.movePosition(QTextCursor::End);
    ed->setTextCursor(cursor);

    QTest::keyClick(ed, Qt::Key_Enter);

    ed->setOverwriteMode(false);
    QTest::keyClicks(ed, "Second paragraph");

    QCOMPARE(blockCount(), 2);

    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::EndOfBlock);

    QCOMPARE(cursor.position(), 15);
    ed->setTextCursor(cursor);

    ed->setOverwriteMode(true);

    QTest::keyClicks(ed, " blah");

    QCOMPARE(blockCount(), 2);

    QTextBlock block = ed->document()->begin();
    QCOMPARE(block.text(), QString("Some shiny text blah"));
    block = block.next();
    QCOMPARE(block.text(), QString("Second paragraph"));
}

void tst_QTextEdit::shiftDownInLineLastShouldSelectToEnd_data()
{
    // shift cursor-down in the last line should select to the end of the document

    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("totalLineCount");

    QTest::newRow("1") << QString("Foo\nBar") << 2;
    QTest::newRow("2") << QString("Foo\nBar") + QChar(QChar::LineSeparator) + QString("Baz") << 3;
}

void tst_QTextEdit::shiftDownInLineLastShouldSelectToEnd()
{
    QFETCH(QString, input);
    QFETCH(int, totalLineCount);

    ed->setPlainText(input);
    ed->show();

    // ensure we're layouted
    ed->document()->documentLayout()->documentSize();

    QCOMPARE(blockCount(), 2);

    int lineCount = 0;
    for (QTextBlock block = ed->document()->begin(); block.isValid(); block = block.next())
        lineCount += block.layout()->lineCount();
    QCOMPARE(lineCount, totalLineCount);

    QTextCursor cursor = ed->textCursor();
    cursor.movePosition(QTextCursor::Start);
    ed->setTextCursor(cursor);

    for (int i = 0; i < lineCount; ++i) {
        QTest::keyClick(ed, Qt::Key_Down, Qt::ShiftModifier);
    }

    input.replace(QLatin1Char('\n'), QChar(QChar::ParagraphSeparator));
    QCOMPARE(ed->textCursor().selectedText(), input);
    QVERIFY(ed->textCursor().atEnd());

    // also test that without shift modifier the cursor does not move to the end
    // for Key_Down in the last line
    cursor.movePosition(QTextCursor::Start);
    ed->setTextCursor(cursor);
    for (int i = 0; i < lineCount; ++i) {
        QTest::keyClick(ed, Qt::Key_Down);
    }
    QVERIFY(!ed->textCursor().atEnd());
}

void tst_QTextEdit::undoRedoShouldRepositionTextEditCursor()
{
    ed->setPlainText("five\nlines\nin\nthis\ntextedit");
    QTextCursor cursor = ed->textCursor();
    cursor.movePosition(QTextCursor::Start);

    ed->setUndoRedoEnabled(false);
    ed->setUndoRedoEnabled(true);

    QVERIFY(!ed->isUndoAvailable());
    QVERIFY(!ed->isRedoAvailable());

    cursor.insertText("Blah");

    QVERIFY(ed->isUndoAvailable());
    QVERIFY(!ed->isRedoAvailable());

    cursor.movePosition(QTextCursor::End);
    ed->setTextCursor(cursor);

    QVERIFY(QMetaObject::invokeMethod(ed, "undo"));

    QVERIFY(!ed->isUndoAvailable());
    QVERIFY(ed->isRedoAvailable());

    QCOMPARE(ed->textCursor().position(), 0);

    cursor.movePosition(QTextCursor::End);
    ed->setTextCursor(cursor);

    QVERIFY(QMetaObject::invokeMethod(ed, "redo"));

    QVERIFY(ed->isUndoAvailable());
    QVERIFY(!ed->isRedoAvailable());

    QCOMPARE(ed->textCursor().position(), 4);
}

void tst_QTextEdit::lineWrapModes()
{
    ed->setLineWrapMode(QTextEdit::NoWrap);
    QCOMPARE(ed->document()->pageSize().width(), qreal(-1));

    ed->setLineWrapColumnOrWidth(10);
    ed->setLineWrapMode(QTextEdit::FixedColumnWidth);
    QVERIFY(qIsNull(ed->document()->pageSize().width()));

    ed->setLineWrapColumnOrWidth(1000);
    ed->setLineWrapMode(QTextEdit::FixedPixelWidth);
    QCOMPARE(ed->document()->pageSize().width(), qreal(1000));
}

void tst_QTextEdit::mouseCursorShape()
{
    // always show an IBeamCursor, see change 170146
    QVERIFY(!ed->isReadOnly());
    QVERIFY(ed->viewport()->cursor().shape() == Qt::IBeamCursor);

    ed->setReadOnly(true);
    QVERIFY(ed->viewport()->cursor().shape() == Qt::IBeamCursor);

    ed->setPlainText("Foo");
    QVERIFY(ed->viewport()->cursor().shape() == Qt::IBeamCursor);
}

void tst_QTextEdit::implicitClear()
{
    // test that QTextEdit::setHtml, etc. avoid calling clear() but instead call
    // QTextDocument::setHtml/etc. instead, which also clear the contents and
    // cached resource but preserve manually added resources. setHtml on a textedit
    // should behave the same as on a document with respect to that.
    // see also clearResources() autotest in qtextdocument

    // regular resource for QTextDocument
    QUrl testUrl(":/foobar");
    QVariant testResource("hello world");

    ed->document()->addResource(QTextDocument::ImageResource, testUrl, testResource);
    QVERIFY(ed->document()->resource(QTextDocument::ImageResource, testUrl) == testResource);

    ed->setPlainText("Blah");
    QVERIFY(ed->document()->resource(QTextDocument::ImageResource, testUrl) == testResource);

    ed->setPlainText("<b>Blah</b>");
    QVERIFY(ed->document()->resource(QTextDocument::ImageResource, testUrl) == testResource);

    ed->clear();
    QVERIFY(!ed->document()->resource(QTextDocument::ImageResource, testUrl).isValid());
    QVERIFY(ed->toPlainText().isEmpty());
}

void tst_QTextEdit::copyAvailable_data()
{
    QTest::addColumn<pairListType>("keystrokes");
    QTest::addColumn<QList<bool> >("copyAvailable");
    QTest::addColumn<QString>("function");

    pairListType keystrokes;
    QList<bool> copyAvailable;

    keystrokes << qMakePair(Qt::Key_B, Qt::NoModifier) <<  qMakePair(Qt::Key_B, Qt::NoModifier)
               << qMakePair(Qt::Key_Left, Qt::ShiftModifier);
    copyAvailable << true ;
    QTest::newRow(QString("Case1 B,B, <- + shift | signals: true").toLatin1())
        << keystrokes << copyAvailable << QString();

    keystrokes.clear();
    copyAvailable.clear();

    keystrokes << qMakePair(Qt::Key_T, Qt::NoModifier) << qMakePair(Qt::Key_A, Qt::NoModifier)
               <<  qMakePair(Qt::Key_A, Qt::NoModifier) << qMakePair(Qt::Key_Left, Qt::ShiftModifier);
    copyAvailable << true << false;
    QTest::newRow(QString("Case2 T,A,A, <- + shift, cut() | signals: true, false").toLatin1())
        << keystrokes << copyAvailable << QString("cut");

    keystrokes.clear();
    copyAvailable.clear();

    keystrokes << qMakePair(Qt::Key_T, Qt::NoModifier) << qMakePair(Qt::Key_A, Qt::NoModifier)
               <<  qMakePair(Qt::Key_A, Qt::NoModifier) << qMakePair(Qt::Key_Left, Qt::ShiftModifier)
               << qMakePair(Qt::Key_Left, Qt::ShiftModifier)  << qMakePair(Qt::Key_Left, Qt::ShiftModifier);
    copyAvailable << true;
    QTest::newRow(QString("Case3 T,A,A, <- + shift, <- + shift, <- + shift, copy() | signals: true").toLatin1())
        << keystrokes << copyAvailable << QString("copy");

    keystrokes.clear();
    copyAvailable.clear();

    keystrokes << qMakePair(Qt::Key_T, Qt::NoModifier) << qMakePair(Qt::Key_A, Qt::NoModifier)
               <<  qMakePair(Qt::Key_A, Qt::NoModifier) << qMakePair(Qt::Key_Left, Qt::ShiftModifier)
               << qMakePair(Qt::Key_Left, Qt::ShiftModifier)  << qMakePair(Qt::Key_Left, Qt::ShiftModifier)
               << qMakePair(Qt::Key_X, Qt::ControlModifier);
    copyAvailable << true << false;
    QTest::newRow(QString("Case4 T,A,A, <- + shift, <- + shift, <- + shift, ctrl + x, paste() | signals: true, false").toLatin1())
        << keystrokes << copyAvailable << QString("paste");

    keystrokes.clear();
    copyAvailable.clear();

    keystrokes << qMakePair(Qt::Key_B, Qt::NoModifier) <<  qMakePair(Qt::Key_B, Qt::NoModifier)
               << qMakePair(Qt::Key_Left, Qt::ShiftModifier) << qMakePair(Qt::Key_Left, Qt::NoModifier);
    copyAvailable << true << false;
    QTest::newRow(QString("Case5 B,B, <- + shift, <- | signals: true, false").toLatin1())
        << keystrokes << copyAvailable << QString();

    keystrokes.clear();
    copyAvailable.clear();

    keystrokes << qMakePair(Qt::Key_B, Qt::NoModifier) <<  qMakePair(Qt::Key_A, Qt::NoModifier)
               << qMakePair(Qt::Key_Left, Qt::ShiftModifier) << qMakePair(Qt::Key_Left, Qt::NoModifier)
               << qMakePair(Qt::Key_Right, Qt::ShiftModifier);
    copyAvailable << true << false << true << false;
    QTest::newRow(QString("Case6 B,A, <- + shift, ->, <- + shift | signals: true, false, true, false").toLatin1())
        << keystrokes << copyAvailable << QString("cut");

    keystrokes.clear();
    copyAvailable.clear();

    keystrokes << qMakePair(Qt::Key_T, Qt::NoModifier) << qMakePair(Qt::Key_A, Qt::NoModifier)
               <<  qMakePair(Qt::Key_A, Qt::NoModifier) << qMakePair(Qt::Key_Left, Qt::ShiftModifier)
               << qMakePair(Qt::Key_Left, Qt::ShiftModifier)  << qMakePair(Qt::Key_Left, Qt::ShiftModifier)
               << qMakePair(Qt::Key_X, Qt::ControlModifier);
    copyAvailable << true << false << true;
    QTest::newRow(QString("Case7 T,A,A, <- + shift, <- + shift, <- + shift, ctrl + x, undo() | signals: true, false, true").toLatin1())
        << keystrokes << copyAvailable << QString("undo");
}

//Tests the copyAvailable slot for several cases
void tst_QTextEdit::copyAvailable()
{
    QFETCH(pairListType,keystrokes);
    QFETCH(QList<bool>, copyAvailable);
    QFETCH(QString, function);

    ed->clear();
    QApplication::clipboard()->clear();
    QVERIFY(!ed->canPaste());
    QSignalSpy spyCopyAvailabe(ed, SIGNAL(copyAvailable(bool)));

    //Execute Keystrokes
    foreach(keyPairType keyPair, keystrokes) {
        QTest::keyClick(ed, keyPair.first, keyPair.second );
    }

    //Execute ed->"function"
    if (function == "cut")
        ed->cut();
    else if (function == "copy")
        ed->copy();
    else if (function == "paste")
        ed->paste();
    else if (function == "undo")
        ed->paste();
    else if (function == "redo")
        ed->paste();

    //Compare spied signals
    QEXPECT_FAIL("Case7 T,A,A, <- + shift, <- + shift, <- + shift, ctrl + x, undo() | signals: true, false, true",
        "Wrong undo selection behaviour. Should be fixed in some future release. (See task: 132482)", Abort);
    QCOMPARE(spyCopyAvailabe.count(), copyAvailable.count());
    for (int i=0;i<spyCopyAvailabe.count(); i++) {
        QVariant variantSpyCopyAvailable = spyCopyAvailabe.at(i).at(0);
        QVERIFY2(variantSpyCopyAvailable.toBool() == copyAvailable.at(i), QString("Spied singnal: %1").arg(i).toLatin1());
    }
}

void tst_QTextEdit::undoRedoAfterSetContent()
{
    QVERIFY(!ed->document()->isUndoAvailable());
    QVERIFY(!ed->document()->isRedoAvailable());
    ed->setPlainText("Foobar");
    QVERIFY(!ed->document()->isUndoAvailable());
    QVERIFY(!ed->document()->isRedoAvailable());
    ed->setHtml("<p>bleh</p>");
    QVERIFY(!ed->document()->isUndoAvailable());
    QVERIFY(!ed->document()->isRedoAvailable());
}

void tst_QTextEdit::numPadKeyNavigation()
{
    ed->setText("Hello World");
    QCOMPARE(ed->textCursor().position(), 0);
    QTest::keyClick(ed, Qt::Key_Right, Qt::KeypadModifier);
    QCOMPARE(ed->textCursor().position(), 1);
}

void tst_QTextEdit::moveCursor()
{
    ed->setText("Test");

    QSignalSpy cursorMovedSpy(ed, SIGNAL(cursorPositionChanged()));

    QCOMPARE(ed->textCursor().position(), 0);
    ed->moveCursor(QTextCursor::NextCharacter);
    QCOMPARE(ed->textCursor().position(), 1);
    QCOMPARE(cursorMovedSpy.count(), 1);
    ed->moveCursor(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    QCOMPARE(ed->textCursor().position(), 2);
    QCOMPARE(cursorMovedSpy.count(), 2);
    QCOMPARE(ed->textCursor().selectedText(), QString("e"));
}

class MyTextEdit : public QTextEdit
{
public:
    inline MyTextEdit()
        : createMimeDataCallCount(0),
          canInsertCallCount(0),
          insertCallCount(0)
    {}

    mutable int createMimeDataCallCount;
    mutable int canInsertCallCount;
    mutable int insertCallCount;

    virtual QMimeData *createMimeDataFromSelection() const {
        createMimeDataCallCount++;
        return QTextEdit::createMimeDataFromSelection();
    }
    virtual bool canInsertFromMimeData(const QMimeData *source) const {
        canInsertCallCount++;
        return QTextEdit::canInsertFromMimeData(source);
    }
    virtual void insertFromMimeData(const QMimeData *source) {
        insertCallCount++;
        QTextEdit::insertFromMimeData(source);
    }

};

void tst_QTextEdit::mimeDataReimplementations()
{
    MyTextEdit ed;
    ed.setPlainText("Hello World");

    QCOMPARE(ed.createMimeDataCallCount, 0);
    QCOMPARE(ed.canInsertCallCount, 0);
    QCOMPARE(ed.insertCallCount, 0);

    ed.selectAll();

    QCOMPARE(ed.createMimeDataCallCount, 0);
    QCOMPARE(ed.canInsertCallCount, 0);
    QCOMPARE(ed.insertCallCount, 0);

    ed.copy();

    QCOMPARE(ed.createMimeDataCallCount, 1);
    QCOMPARE(ed.canInsertCallCount, 0);
    QCOMPARE(ed.insertCallCount, 0);

    QTextControl *control = qFindChild<QTextControl *>(&ed);
    QVERIFY(control);

    control->canInsertFromMimeData(QApplication::clipboard()->mimeData());

    QCOMPARE(ed.createMimeDataCallCount, 1);
    QCOMPARE(ed.canInsertCallCount, 1);
    QCOMPARE(ed.insertCallCount, 0);

    ed.paste();

    QCOMPARE(ed.createMimeDataCallCount, 1);
    QCOMPARE(ed.canInsertCallCount, 1);
    QCOMPARE(ed.insertCallCount, 1);
}

void tst_QTextEdit::ctrlEnterShouldInsertLineSeparator()
{
    QTest::keyClick(ed, Qt::Key_A);
    QTest::keyClick(ed, Qt::Key_Enter, Qt::ControlModifier);
    QTest::keyClick(ed, Qt::Key_B);
    QString expected;
    expected += 'a';
    expected += QChar::LineSeparator;
    expected += 'b';
    QCOMPARE(ed->textCursor().block().text(), expected);
}

void tst_QTextEdit::selectWordsFromStringsContainingSeparators_data()
{
    QTest::addColumn<QString>("testString");
    QTest::addColumn<QString>("selectedWord");

    QStringList wordSeparators;
    wordSeparators << "." << "," << "?" << "!" << ":" << ";" << "-" << "<" << ">" << "["
                   << "]" << "(" << ")" << "{" << "}" << "=" << "\t"<< QString(QChar::Nbsp);

    foreach (QString s, wordSeparators)
        QTest::newRow("separator: " + s) << QString("foo") + s + QString("bar") << QString("foo");
}

void tst_QTextEdit::selectWordsFromStringsContainingSeparators()
{
    QFETCH(QString, testString);
    QFETCH(QString, selectedWord);
    ed->setText(testString);
    QTextCursor cursor = ed->textCursor();
    cursor.movePosition(QTextCursor::StartOfLine);
    cursor.select(QTextCursor::WordUnderCursor);
    QVERIFY(cursor.hasSelection());
    QCOMPARE(cursor.selection().toPlainText(), selectedWord);
    cursor.clearSelection();
}

void tst_QTextEdit::canPaste()
{
    if (!nativeClipboardWorking())
        QSKIP("Clipboard not working with cron-started unit tests", SkipAll);

    QApplication::clipboard()->setText(QString());
    QVERIFY(!ed->canPaste());
    QApplication::clipboard()->setText("Test");
    QVERIFY(ed->canPaste());
    ed->setTextInteractionFlags(Qt::NoTextInteraction);
    QVERIFY(!ed->canPaste());
}

void tst_QTextEdit::ensureCursorVisibleOnInitialShow()
{
    QString manyPagesOfPlainText;
    for (int i = 0; i < 800; ++i)
        manyPagesOfPlainText += QLatin1String("Blah blah blah blah blah blah\n");

    ed->setPlainText(manyPagesOfPlainText);
    QCOMPARE(ed->textCursor().position(), 0);

    ed->moveCursor(QTextCursor::End);
    ed->show();
    QVERIFY(ed->verticalScrollBar()->value() > 10);

    ed->moveCursor(QTextCursor::Start);
    QVERIFY(ed->verticalScrollBar()->value() < 10);
    ed->hide();
    ed->verticalScrollBar()->setValue(ed->verticalScrollBar()->maximum());
    ed->show();
    QCOMPARE(ed->verticalScrollBar()->value(), ed->verticalScrollBar()->maximum());
}

class TestEdit : public QTextEdit
{
public:
    TestEdit() : resizeEventCalled(false) {}

    bool resizeEventCalled;

protected:
    virtual void resizeEvent(QResizeEvent *e)
    {
        QTextEdit::resizeEvent(e);
        setHtml("<img src=qtextbrowser-resizeevent.png width=" + QString::number(size().width()) + "><br>Size is " + QString::number(size().width()) + " x " + QString::number(size().height()));
        resizeEventCalled = true;
    }
};

void tst_QTextEdit::setHtmlInsideResizeEvent()
{
    TestEdit edit;
    edit.show();
    edit.resize(800, 600);
    QVERIFY(edit.resizeEventCalled);
}

void tst_QTextEdit::colorfulAppend()
{
    ed->setTextColor(Qt::red);
    ed->append("Red");
    ed->setTextColor(Qt::blue);
    ed->append("Blue");
    ed->setTextColor(Qt::green);
    ed->append("Green");

    QCOMPARE(ed->document()->blockCount(), 3);
    QTextBlock block = ed->document()->begin();
    QCOMPARE(block.begin().fragment().text(), QString("Red"));
    QVERIFY(block.begin().fragment().charFormat().foreground().color() == Qt::red);
    block = block.next();
    QCOMPARE(block.begin().fragment().text(), QString("Blue"));
    QVERIFY(block.begin().fragment().charFormat().foreground().color() == Qt::blue);
    block = block.next();
    QCOMPARE(block.begin().fragment().text(), QString("Green"));
    QVERIFY(block.begin().fragment().charFormat().foreground().color() == Qt::green);
}

void tst_QTextEdit::ensureVisibleWithRtl()
{
    ed->setLayoutDirection(Qt::RightToLeft);
    ed->setLineWrapMode(QTextEdit::NoWrap);
    QString txt(500, QChar(QLatin1Char('a')));
    QCOMPARE(txt.length(), 500);
    ed->setPlainText(txt);
    ed->resize(100, 100);
    ed->show();

    qApp->processEvents();

    QVERIFY(ed->horizontalScrollBar()->maximum() > 0);

    ed->moveCursor(QTextCursor::Start);
    QCOMPARE(ed->horizontalScrollBar()->value(), ed->horizontalScrollBar()->maximum());
    ed->moveCursor(QTextCursor::End);
    QCOMPARE(ed->horizontalScrollBar()->value(), 0);
    ed->moveCursor(QTextCursor::Start);
    QCOMPARE(ed->horizontalScrollBar()->value(), ed->horizontalScrollBar()->maximum());
    ed->moveCursor(QTextCursor::End);
    QCOMPARE(ed->horizontalScrollBar()->value(), 0);
}

void tst_QTextEdit::preserveCharFormatAfterSetPlainText()
{
    ed->setTextColor(Qt::blue);
    ed->setPlainText("This is blue");
    ed->append("This should still be blue");
    QTextBlock block = ed->document()->begin();
    block = block.next();
    QCOMPARE(block.text(), QString("This should still be blue"));
    QVERIFY(block.begin().fragment().charFormat().foreground().color() == QColor(Qt::blue));
}

void tst_QTextEdit::extraSelections()
{
    ed->setPlainText("Hello World");

    QTextCursor c = ed->textCursor();
    c.movePosition(QTextCursor::Start);
    c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    const int endPos = c.position();

    QTextEdit::ExtraSelection sel;
    sel.cursor = c;
    ed->setExtraSelections(QList<QTextEdit::ExtraSelection>() << sel);

    c.movePosition(QTextCursor::Start);
    c.movePosition(QTextCursor::NextWord);
    const int wordPos = c.position();
    c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    sel.cursor = c;
    ed->setExtraSelections(QList<QTextEdit::ExtraSelection>() << sel);

    QList<QTextEdit::ExtraSelection> selections = ed->extraSelections();
    QCOMPARE(selections.count(), 1);
    QCOMPARE(selections.at(0).cursor.position(), endPos);
    QCOMPARE(selections.at(0).cursor.anchor(), wordPos);
}

void tst_QTextEdit::adjustScrollbars()
{
    QFont ff(ed->font());
    ff.setFamily("Tahoma");
    ff.setPointSize(11);
    ed->setFont(ff);
    ed->setMinimumSize(140, 100);
    ed->setMaximumSize(140, 100);
    ed->show();
    QLatin1String txt("\nabc def ghi jkl mno pqr stu vwx");
    ed->setText(txt + txt + txt);

    QVERIFY(ed->verticalScrollBar()->maximum() > 0);

    ed->moveCursor(QTextCursor::End);
    int oldMaximum = ed->verticalScrollBar()->maximum();
    QTextCursor cursor = ed->textCursor();
    cursor.insertText(QLatin1String("\n"));
    cursor.deletePreviousChar();
    QCOMPARE(ed->verticalScrollBar()->maximum(), oldMaximum);
}

QTEST_MAIN(tst_QTextEdit)
#include "tst_qtextedit.moc"
