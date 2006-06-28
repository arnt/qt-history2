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
    QDir::setCurrent(SRCDIR);
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
    QUrl url = QUrl::fromLocalFile("anchor.html");

    browser->htmlLoadAttempts = 0;
    browser->setSource(url);
    QCOMPARE(browser->htmlLoadAttempts, 1);
    QVERIFY(!browser->toPlainText().isEmpty());

    url.setFragment("jumphere");
    browser->setSource(url);
    QCOMPARE(browser->htmlLoadAttempts, 1);
    QVERIFY(!browser->toPlainText().isEmpty());
    QVERIFY(browser->source() == url);
}

void tst_QTextBrowser::bgColorOnSourceChange()
{
    browser->setSource(QUrl::fromLocalFile("pagewithbg.html"));
    QVERIFY(browser->document()->rootFrame()->frameFormat().hasProperty(QTextFormat::BackgroundBrush));
    QVERIFY(browser->document()->rootFrame()->frameFormat().background().color() == Qt::blue);

    browser->setSource(QUrl::fromLocalFile("pagewithoutbg.html"));
    QVERIFY(!browser->document()->rootFrame()->frameFormat().hasProperty(QTextFormat::BackgroundBrush));
}

void tst_QTextBrowser::forwardButton()
{
    QSignalSpy forwardEmissions(browser, SIGNAL(forwardAvailable(bool)));
    QSignalSpy backwardEmissions(browser, SIGNAL(backwardAvailable(bool)));

    browser->setSource(QUrl::fromLocalFile("pagewithbg.html"));

    QVERIFY(!forwardEmissions.isEmpty());
    QVariant val = forwardEmissions.takeLast()[0];
    QVERIFY(val.type() == QVariant::Bool);
    QVERIFY(val.toBool() == false);

    QVERIFY(!backwardEmissions.isEmpty());
    val = backwardEmissions.takeLast()[0];
    QVERIFY(val.type() == QVariant::Bool);
    QVERIFY(val.toBool() == false);

    browser->setSource(QUrl::fromLocalFile("anchor.html"));

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

    browser->setSource(QUrl::fromLocalFile("pagewithoutbg.html"));

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
    browser->setSource(QUrl::fromLocalFile("bigpage.html"));
    browser->scrollToAnchor("bottom");
    QVERIFY(browser->verticalScrollBar()->value() > 0);

    browser->setSource(QUrl::fromLocalFile("pagewithbg.html"));
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
    browser->setSource(QUrl::fromLocalFile("bigpage.html"));
    browser->setSource(QUrl("#bottom"));
    QVERIFY(browser->verticalScrollBar()->value() > 0);
}

void tst_QTextBrowser::resourceAutoDetection()
{
    browser->setHtml("<img src=\":/some/resource\"/>");
    QCOMPARE(browser->lastResource.toString(), QString("qrc:/some/resource"));
}

void tst_QTextBrowser::forwardBackwardAvailable()
{
    QSignalSpy backwardSpy(browser, SIGNAL(backwardAvailable(bool)));
    QSignalSpy forwardSpy(browser, SIGNAL(forwardAvailable(bool)));

    QVERIFY(!browser->isBackwardAvailable());
    QVERIFY(!browser->isForwardAvailable());

    browser->setSource(QUrl::fromLocalFile("anchor.html"));
    QVERIFY(!browser->isBackwardAvailable());
    QVERIFY(!browser->isForwardAvailable());
    QCOMPARE(backwardSpy.count(), 1);
    QVERIFY(!backwardSpy.at(0).at(0).toBool());
    QCOMPARE(forwardSpy.count(), 1);
    QVERIFY(!forwardSpy.at(0).at(0).toBool());

    backwardSpy.clear();
    forwardSpy.clear();

    browser->setSource(QUrl::fromLocalFile("bigpage.html"));
    QVERIFY(browser->isBackwardAvailable());
    QVERIFY(!browser->isForwardAvailable());
    QCOMPARE(backwardSpy.count(), 1);
    QVERIFY(backwardSpy.at(0).at(0).toBool());
    QCOMPARE(forwardSpy.count(), 1);
    QVERIFY(!forwardSpy.at(0).at(0).toBool());

    backwardSpy.clear();
    forwardSpy.clear();

    browser->setSource(QUrl::fromLocalFile("pagewithbg.html"));
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

    browser->setSource(QUrl::fromLocalFile("anchor.html"));
    QVERIFY(!browser->isBackwardAvailable());
    QVERIFY(!browser->isForwardAvailable());
    QCOMPARE(backwardSpy.count(), 1);
    QVERIFY(!backwardSpy.at(0).at(0).toBool());
    QCOMPARE(forwardSpy.count(), 1);
    QVERIFY(!forwardSpy.at(0).at(0).toBool());

    backwardSpy.clear();
    forwardSpy.clear();

    browser->setSource(QUrl::fromLocalFile("bigpage.html"));
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
    browser->setSource(url);
    QCOMPARE(browser->lastResource.toString(), QString("foobar.png"));
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

QTEST_MAIN(tst_QTextBrowser)
#include "tst_qtextbrowser.moc"
