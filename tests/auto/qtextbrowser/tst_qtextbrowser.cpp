/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qtextbrowser.h>
#include <qdatetime.h>
#include <qapplication.h>
#include <qscrollbar.h>

#include <qtextbrowser.h>

//TESTED_CLASS=
//TESTED_FILES=src/gui/widgets/qtextbrowser.h src/gui/widgets/qtextbrowser.cpp

class TestBrowser : public QTextBrowser
{
public:
    inline TestBrowser() : htmlLoadAttempts(0) {}

    virtual QVariant loadResource(int type, const QUrl &name);

    int htmlLoadAttempts;
    QUrl lastResource;
    QUrl sourceInsideLoadResource;
};

QVariant TestBrowser::loadResource(int type, const QUrl &name)
{
    if (type == QTextDocument::HtmlResource)
        htmlLoadAttempts++;
    lastResource = name;
    sourceInsideLoadResource = source();
    return QTextBrowser::loadResource(type, name);
}

class tst_QTextBrowser : public QObject
{
    Q_OBJECT
public:
    tst_QTextBrowser();
    virtual ~tst_QTextBrowser();

public slots:
    void init();
    void cleanup();

private slots:
    void noReloadOnAnchorJump();
    void bgColorOnSourceChange();
    void forwardButton();
    void viewportPositionInHistory();
    void relativeLinks();
    void anchors();
    void resourceAutoDetection();
    void forwardBackwardAvailable();
    void clearHistory();
    void sourceInsideLoadResource();
    void textInteractionFlags_vs_readOnly();
    void anchorsWithSelfBuiltHtml();
    void relativeNonLocalUrls();
    void adjacentAnchors();
    void loadResourceOnRelativeLocalFiles();
    void focusIndicator();

private:
    TestBrowser *browser;
};

tst_QTextBrowser::tst_QTextBrowser()
{
}

tst_QTextBrowser::~tst_QTextBrowser()
{
}
void tst_QTextBrowser::init()
{
#ifndef Q_OS_IRIX
    QDir::setCurrent(SRCDIR);
#endif
    browser = new TestBrowser;
    browser->show();
}

void tst_QTextBrowser::cleanup()
{
    delete browser;
    browser = 0;
}

void tst_QTextBrowser::noReloadOnAnchorJump()
{
    QUrl url("anchor.html");

    browser->htmlLoadAttempts = 0;
    browser->setSource(url);
    QCOMPARE(browser->htmlLoadAttempts, 1);
    QVERIFY(!browser->toPlainText().isEmpty());

    url.setFragment("jumphere"); // anchor.html#jumphere
    browser->setSource(url);
    QCOMPARE(browser->htmlLoadAttempts, 1);
    QVERIFY(!browser->toPlainText().isEmpty());
    QVERIFY(browser->source() == url);
}

void tst_QTextBrowser::bgColorOnSourceChange()
{
    browser->setSource(QUrl("pagewithbg.html"));
    QVERIFY(browser->document()->rootFrame()->frameFormat().hasProperty(QTextFormat::BackgroundBrush));
    QVERIFY(browser->document()->rootFrame()->frameFormat().background().color() == Qt::blue);

    browser->setSource(QUrl("pagewithoutbg.html"));
    QVERIFY(!browser->document()->rootFrame()->frameFormat().hasProperty(QTextFormat::BackgroundBrush));
}

void tst_QTextBrowser::forwardButton()
{
    QSignalSpy forwardEmissions(browser, SIGNAL(forwardAvailable(bool)));
    QSignalSpy backwardEmissions(browser, SIGNAL(backwardAvailable(bool)));

    browser->setSource(QUrl("pagewithbg.html"));

    QVERIFY(!forwardEmissions.isEmpty());
    QVariant val = forwardEmissions.takeLast()[0];
    QVERIFY(val.type() == QVariant::Bool);
    QVERIFY(val.toBool() == false);

    QVERIFY(!backwardEmissions.isEmpty());
    val = backwardEmissions.takeLast()[0];
    QVERIFY(val.type() == QVariant::Bool);
    QVERIFY(val.toBool() == false);

    browser->setSource(QUrl("anchor.html"));

    QVERIFY(!forwardEmissions.isEmpty());
    val = forwardEmissions.takeLast()[0];
    QVERIFY(val.type() == QVariant::Bool);
    QVERIFY(val.toBool() == false);

    QVERIFY(!backwardEmissions.isEmpty());
    val = backwardEmissions.takeLast()[0];
    QVERIFY(val.type() == QVariant::Bool);
    QVERIFY(val.toBool() == true);

    browser->backward();

    QVERIFY(!forwardEmissions.isEmpty());
    val = forwardEmissions.takeLast()[0];
    QVERIFY(val.type() == QVariant::Bool);
    QVERIFY(val.toBool() == true);

    QVERIFY(!backwardEmissions.isEmpty());
    val = backwardEmissions.takeLast()[0];
    QVERIFY(val.type() == QVariant::Bool);
    QVERIFY(val.toBool() == false);

    browser->setSource(QUrl("pagewithoutbg.html"));

    QVERIFY(!forwardEmissions.isEmpty());
    val = forwardEmissions.takeLast()[0];
    QVERIFY(val.type() == QVariant::Bool);
    QVERIFY(val.toBool() == false);

    QVERIFY(!backwardEmissions.isEmpty());
    val = backwardEmissions.takeLast()[0];
    QVERIFY(val.type() == QVariant::Bool);
    QVERIFY(val.toBool() == true);
}

void tst_QTextBrowser::viewportPositionInHistory()
{
    browser->setSource(QUrl("bigpage.html"));
    browser->scrollToAnchor("bottom");
    QVERIFY(browser->verticalScrollBar()->value() > 0);

    browser->setSource(QUrl("pagewithbg.html"));
    QCOMPARE(browser->verticalScrollBar()->value(), 0);

    browser->backward();
    QVERIFY(browser->verticalScrollBar()->value() > 0);
}

void tst_QTextBrowser::relativeLinks()
{
    qRegisterMetaType<QUrl>("QUrl");
    QSignalSpy sourceChangedSpy(browser, SIGNAL(sourceChanged(const QUrl &)));
    browser->setSource(QUrl("subdir/index.html"));
    QVERIFY(sourceChangedSpy.count() == 1);
    QCOMPARE(sourceChangedSpy.takeFirst()[0].toUrl(), QUrl("subdir/index.html"));
    browser->setSource(QUrl("../anchor.html"));
    QVERIFY(sourceChangedSpy.count() == 1);
    QCOMPARE(sourceChangedSpy.takeFirst()[0].toUrl(), QUrl("../anchor.html"));
    browser->setSource(QUrl("subdir/index.html"));
    QVERIFY(sourceChangedSpy.count() == 1);
    QCOMPARE(sourceChangedSpy.takeFirst()[0].toUrl(), QUrl("subdir/index.html"));
}

void tst_QTextBrowser::anchors()
{
    browser->setSource(QUrl("bigpage.html"));
    browser->setSource(QUrl("#bottom"));
    QVERIFY(browser->verticalScrollBar()->value() > 0);

    browser->setSource(QUrl("bigpage.html"));
    browser->setSource(QUrl("#id-anchor"));
    QVERIFY(browser->verticalScrollBar()->value() > 0);
}

void tst_QTextBrowser::resourceAutoDetection()
{
    QTest::ignoreMessage(QtWarningMsg, "QTextBrowser: Cannot open 'qrc:/some/resource' for reading");
    QTest::ignoreMessage(QtWarningMsg, "QTextBrowser: Cannot open 'qrc:/some/resource' for reading");
    browser->setHtml("<img src=\":/some/resource\"/>");
    QCOMPARE(browser->lastResource.toString(), QString("qrc:/some/resource"));
}

void tst_QTextBrowser::forwardBackwardAvailable()
{
    QSignalSpy backwardSpy(browser, SIGNAL(backwardAvailable(bool)));
    QSignalSpy forwardSpy(browser, SIGNAL(forwardAvailable(bool)));

    QVERIFY(!browser->isBackwardAvailable());
    QVERIFY(!browser->isForwardAvailable());

    browser->setSource(QUrl("anchor.html"));
    QVERIFY(!browser->isBackwardAvailable());
    QVERIFY(!browser->isForwardAvailable());
    QCOMPARE(backwardSpy.count(), 1);
    QVERIFY(!backwardSpy.at(0).at(0).toBool());
    QCOMPARE(forwardSpy.count(), 1);
    QVERIFY(!forwardSpy.at(0).at(0).toBool());

    backwardSpy.clear();
    forwardSpy.clear();

    browser->setSource(QUrl("bigpage.html"));
    QVERIFY(browser->isBackwardAvailable());
    QVERIFY(!browser->isForwardAvailable());
    QCOMPARE(backwardSpy.count(), 1);
    QVERIFY(backwardSpy.at(0).at(0).toBool());
    QCOMPARE(forwardSpy.count(), 1);
    QVERIFY(!forwardSpy.at(0).at(0).toBool());

    backwardSpy.clear();
    forwardSpy.clear();

    browser->setSource(QUrl("pagewithbg.html"));
    QVERIFY(browser->isBackwardAvailable());
    QVERIFY(!browser->isForwardAvailable());
    QCOMPARE(backwardSpy.count(), 1);
    QVERIFY(backwardSpy.at(0).at(0).toBool());
    QCOMPARE(forwardSpy.count(), 1);
    QVERIFY(!forwardSpy.at(0).at(0).toBool());

    backwardSpy.clear();
    forwardSpy.clear();

    browser->backward();
    QVERIFY(browser->isBackwardAvailable());
    QVERIFY(browser->isForwardAvailable());
    QCOMPARE(backwardSpy.count(), 1);
    QVERIFY(backwardSpy.at(0).at(0).toBool());
    QCOMPARE(forwardSpy.count(), 1);
    QVERIFY(forwardSpy.at(0).at(0).toBool());

    backwardSpy.clear();
    forwardSpy.clear();

    browser->backward();
    QVERIFY(!browser->isBackwardAvailable());
    QVERIFY(browser->isForwardAvailable());
    QCOMPARE(backwardSpy.count(), 1);
    QVERIFY(!backwardSpy.at(0).at(0).toBool());
    QCOMPARE(forwardSpy.count(), 1);
    QVERIFY(forwardSpy.at(0).at(0).toBool());

    backwardSpy.clear();
    forwardSpy.clear();

    browser->forward();
    QVERIFY(browser->isBackwardAvailable());
    QVERIFY(browser->isForwardAvailable());
    QCOMPARE(backwardSpy.count(), 1);
    QVERIFY(backwardSpy.at(0).at(0).toBool());
    QCOMPARE(forwardSpy.count(), 1);
    QVERIFY(forwardSpy.at(0).at(0).toBool());

    backwardSpy.clear();
    forwardSpy.clear();

    browser->forward();
    QVERIFY(browser->isBackwardAvailable());
    QVERIFY(!browser->isForwardAvailable());
    QCOMPARE(backwardSpy.count(), 1);
    QVERIFY(backwardSpy.at(0).at(0).toBool());
    QCOMPARE(forwardSpy.count(), 1);
    QVERIFY(!forwardSpy.at(0).at(0).toBool());

    backwardSpy.clear();
    forwardSpy.clear();
}

void tst_QTextBrowser::clearHistory()
{
    QSignalSpy backwardSpy(browser, SIGNAL(backwardAvailable(bool)));
    QSignalSpy forwardSpy(browser, SIGNAL(forwardAvailable(bool)));

    QVERIFY(!browser->isBackwardAvailable());
    QVERIFY(!browser->isForwardAvailable());

    browser->clearHistory();
    QVERIFY(!browser->isBackwardAvailable());
    QVERIFY(!browser->isForwardAvailable());
    QCOMPARE(backwardSpy.count(), 1);
    QVERIFY(!backwardSpy.at(0).at(0).toBool());
    QCOMPARE(forwardSpy.count(), 1);
    QVERIFY(!forwardSpy.at(0).at(0).toBool());

    backwardSpy.clear();
    forwardSpy.clear();

    browser->setSource(QUrl("anchor.html"));
    QVERIFY(!browser->isBackwardAvailable());
    QVERIFY(!browser->isForwardAvailable());
    QCOMPARE(backwardSpy.count(), 1);
    QVERIFY(!backwardSpy.at(0).at(0).toBool());
    QCOMPARE(forwardSpy.count(), 1);
    QVERIFY(!forwardSpy.at(0).at(0).toBool());

    backwardSpy.clear();
    forwardSpy.clear();

    browser->setSource(QUrl("bigpage.html"));
    QVERIFY(browser->isBackwardAvailable());
    QVERIFY(!browser->isForwardAvailable());
    QCOMPARE(backwardSpy.count(), 1);
    QVERIFY(backwardSpy.at(0).at(0).toBool());
    QCOMPARE(forwardSpy.count(), 1);
    QVERIFY(!forwardSpy.at(0).at(0).toBool());

    backwardSpy.clear();
    forwardSpy.clear();

    browser->clearHistory();
    QVERIFY(!browser->isBackwardAvailable());
    QVERIFY(!browser->isForwardAvailable());
    QCOMPARE(backwardSpy.count(), 1);
    QVERIFY(!backwardSpy.at(0).at(0).toBool());
    QCOMPARE(forwardSpy.count(), 1);
    QVERIFY(!forwardSpy.at(0).at(0).toBool());
}

void tst_QTextBrowser::sourceInsideLoadResource()
{
    QUrl url("pagewithimage.html");
    QTest::ignoreMessage(QtWarningMsg, "QTextBrowser: Cannot open 'foobar.png' for reading");
    QTest::ignoreMessage(QtWarningMsg, "QTextBrowser: Cannot open 'foobar.png' for reading");
    browser->setSource(url);
    QCOMPARE(browser->lastResource.toString(), QString("foobar.png"));
    QEXPECT_FAIL("", "This is currently not supported", Continue);
    QCOMPARE(browser->sourceInsideLoadResource.toString(), url.toString());
}

void tst_QTextBrowser::textInteractionFlags_vs_readOnly()
{
    QVERIFY(browser->isReadOnly());
    QVERIFY(browser->textInteractionFlags() == Qt::TextBrowserInteraction);
    browser->setReadOnly(true);
    QVERIFY(browser->textInteractionFlags() == Qt::TextBrowserInteraction);
    browser->setReadOnly(false);
    QVERIFY(browser->textInteractionFlags() == Qt::TextEditorInteraction);
    browser->setReadOnly(true);
    QVERIFY(browser->textInteractionFlags() == Qt::TextBrowserInteraction);
}

void tst_QTextBrowser::anchorsWithSelfBuiltHtml()
{
    browser->setHtml("<p>Hello <a href=\"#anchor\">Link</a>"
                     "<p><a name=\"anchor\"/>Blah</p>");
    QVERIFY(browser->document()->blockCount() > 1);
    browser->setSource(QUrl("#anchor"));
    QVERIFY(browser->document()->blockCount() > 1);
}

class HelpBrowser : public QTextBrowser
{
public:
    virtual QVariant loadResource(int /*type*/, const QUrl &name) {
        QString url = name.toString();
        if(url == "qhelp://docs/index.html") {
            return "index";
        } else if (url == "qhelp://docs/classes.html") {
            return "classes";
        } else if (url == "qhelp://docs/someclass.html") {
            return "someclass";
        }
        return QVariant();
    }
};

void tst_QTextBrowser::relativeNonLocalUrls()
{
    HelpBrowser browser;
    browser.setSource(QUrl("qhelp://docs/index.html"));
    QCOMPARE(browser.toPlainText(), QString("index"));
    browser.setSource(QUrl("classes.html"));
    QCOMPARE(browser.toPlainText(), QString("classes"));
    browser.setSource(QUrl("someclass.html"));
    QCOMPARE(browser.toPlainText(), QString("someclass"));
}

class HackBrowser : public QTextBrowser
{
public:
    inline bool focusTheNextChild() { return QTextBrowser::focusNextChild(); }
    inline bool focusThePreviousChild() { return QTextBrowser::focusPreviousChild(); }
};

void tst_QTextBrowser::adjacentAnchors()
{
    HackBrowser *browser = new HackBrowser;
    browser->setHtml("<a href=\"#foo\">foo</a><a href=\"#bar\">bar</a>");
    QVERIFY(browser->focusTheNextChild());
    QCOMPARE(browser->textCursor().selectedText(), QString("foo"));

    QVERIFY(browser->focusTheNextChild());
    QCOMPARE(browser->textCursor().selectedText(), QString("bar"));

    QVERIFY(!browser->focusTheNextChild());

    browser->moveCursor(QTextCursor::End);
    QVERIFY(browser->focusThePreviousChild());
    QCOMPARE(browser->textCursor().selectedText(), QString("bar"));
    QVERIFY(browser->focusThePreviousChild());
    QCOMPARE(browser->textCursor().selectedText(), QString("foo"));

    delete browser;
}

void tst_QTextBrowser::loadResourceOnRelativeLocalFiles()
{
    browser->setSource(QUrl("subdir/index.html"));
    QVERIFY(!browser->toPlainText().isEmpty());
    QVariant v = browser->loadResource(QTextDocument::HtmlResource, QUrl("../anchor.html"));
    QVERIFY(v.isValid());
    QVERIFY(v.type() == QVariant::ByteArray);
    QVERIFY(!v.toByteArray().isEmpty());
}

void tst_QTextBrowser::focusIndicator()
{
    HackBrowser *browser = new HackBrowser;
    browser->setSource(QUrl("firstpage.html"));
    QVERIFY(!browser->textCursor().hasSelection());

    browser->focusTheNextChild();

    QVERIFY(browser->textCursor().hasSelection());
    QCOMPARE(browser->textCursor().selectedText(), QString("Link to second page"));

    QTest::keyClick(browser, Qt::Key_Enter);
    QVERIFY(!browser->textCursor().hasSelection());

    browser->focusTheNextChild();

    QVERIFY(browser->textCursor().hasSelection());
    QCOMPARE(browser->textCursor().selectedText(), QString("Link to third page from second page"));

    QTest::keyClick(browser, Qt::Key_Enter);
    QVERIFY(!browser->textCursor().hasSelection());

    browser->backward();

    QVERIFY(browser->textCursor().hasSelection());
    QCOMPARE(browser->textCursor().selectedText(), QString("Link to third page from second page"));

    browser->backward();

    QVERIFY(browser->textCursor().hasSelection());
    QCOMPARE(browser->textCursor().selectedText(), QString("Link to second page"));

    browser->forward();

    QVERIFY(browser->textCursor().hasSelection());
    QCOMPARE(browser->textCursor().selectedText(), QString("Link to third page from second page"));

    browser->backward();
    browser->backward();

    QVERIFY(browser->textCursor().hasSelection());
    QCOMPARE(browser->textCursor().selectedText(), QString("Link to second page"));

    QTest::keyClick(browser, Qt::Key_Enter);
    QVERIFY(!browser->textCursor().hasSelection());

    delete browser;
}

QTEST_MAIN(tst_QTextBrowser)
#include "tst_qtextbrowser.moc"
