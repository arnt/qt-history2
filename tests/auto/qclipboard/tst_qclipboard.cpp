/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtCore/QDebug>
#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#ifdef Q_WS_MAC
#include <Carbon/Carbon.h>
#endif


//TESTED_CLASS=QClipboard
//TESTED_FILES=gui/kernel/qclipboard.h gui/kernel/qclipboard.cpp

class tst_QClipboard : public QObject
{
    Q_OBJECT
private slots:

    void capabiliyFunctions();
    void modes();
    void testSignals();
    void copy_exit_paste();
    void setMimeData();

private:
    bool nativeClipboardWorking();
};


bool tst_QClipboard::nativeClipboardWorking()
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

/*
    Tests that the capability functions are implemented on all
    platforms.
*/
void tst_QClipboard::capabiliyFunctions()
{
    QClipboard * const clipboard =  QApplication::clipboard();

    clipboard->supportsSelection();
    clipboard->supportsFindBuffer();
    clipboard->ownsSelection();
    clipboard->ownsClipboard();
    clipboard->ownsFindBuffer();
}

/*
    Test that text inserted into the clipboard in different modes is
    kept separate.
*/
void tst_QClipboard::modes()
{
    QClipboard * const clipboard =  QApplication::clipboard();

    if (!nativeClipboardWorking())
        QSKIP("Native clipboard not working in this setup", SkipAll);

    const QString defaultMode = "deafult mode text;";
    clipboard->setText(defaultMode);
    QCOMPARE(clipboard->text(), defaultMode);

    if (clipboard->supportsSelection()) {
        const QString selectionMode = "selection mode text";
        clipboard->setText(selectionMode, QClipboard::Selection);
        QCOMPARE(clipboard->text(QClipboard::Selection), selectionMode);
        QCOMPARE(clipboard->text(), defaultMode);
    }

    if (clipboard->supportsFindBuffer()) {
        const QString searchMode = "find mode text";
        clipboard->setText(searchMode, QClipboard::FindBuffer);
        QCOMPARE(clipboard->text(QClipboard::FindBuffer), searchMode);
        QCOMPARE(clipboard->text(), defaultMode);
    }
}

/*
    Test that the apropriate signals are emitted when the cliboard
    contents is changed by calling the qt funcitons.
*/
void tst_QClipboard::testSignals()
{
    if (!nativeClipboardWorking())
        QSKIP("Native clipboard not working in this setup", SkipAll);

    QClipboard * const clipboard =  QApplication::clipboard();

    QSignalSpy dataChangedSpy(clipboard, SIGNAL(dataChanged()));
    QSignalSpy searchChangedSpy(clipboard, SIGNAL(findBufferChanged()));
    QSignalSpy selectionChangedSpy(clipboard, SIGNAL(selectionChanged()));

    const QString text = "clipboard text;";

    // Test the default mode signal.
    clipboard->setText(text);
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(searchChangedSpy.count(), 0);
    QCOMPARE(selectionChangedSpy.count(), 0);

    // Test the selction mode signal.
    if (clipboard->supportsSelection()) {
        clipboard->setText(text, QClipboard::Selection);
        QCOMPARE(selectionChangedSpy.count(), 1);
    } else {
        QCOMPARE(selectionChangedSpy.count(), 0);
    }
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(searchChangedSpy.count(), 0);

    // Test the search mode signal.
    if (clipboard->supportsFindBuffer()) {
        clipboard->setText(text, QClipboard::FindBuffer);
        QCOMPARE(searchChangedSpy.count(), 1);
    } else {
        QCOMPARE(searchChangedSpy.count(), 0);
    }
    QCOMPARE(dataChangedSpy.count(), 1);
}

/*
    Test that pasted text remain on the clipboard
    after a Qt application exits.
*/
void tst_QClipboard::copy_exit_paste()
{
#if defined Q_WS_X11 || defined Q_WS_QWS
    QSKIP("This test does not make sense on X11 and embedded, copied data disappears from the clipboard when the application exits ", SkipAll);
    // ### It's still possible to test copy/paste - just keep the apps running
#endif
    if (!nativeClipboardWorking())
        QSKIP("Native clipboard not working in this setup", SkipAll);
    const QStringList stringArgument = QStringList() << "Test string.";
    QCOMPARE(QProcess::execute("copier/copier", stringArgument), 0);
    QCOMPARE(QProcess::execute("paster/paster", stringArgument), 0);
}

void tst_QClipboard::setMimeData()
{
    QMimeData *mimeData = new QMimeData;

    QApplication::clipboard()->setMimeData(mimeData);
    QCOMPARE(QApplication::clipboard()->mimeData(), mimeData);

    // set it to the same data again, it shouldn't delete mimeData (and crash as a result)
    QApplication::clipboard()->setMimeData(mimeData);
    QCOMPARE(QApplication::clipboard()->mimeData(), mimeData);
    QApplication::clipboard()->clear();

    QVERIFY(QApplication::clipboard()->mimeData() != mimeData);
}

QTEST_MAIN(tst_QClipboard)

#include "tst_qclipboard.moc"
