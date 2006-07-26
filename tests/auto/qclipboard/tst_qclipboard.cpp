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
};

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
    const QStringList stringArgument = QStringList() << "Test string.";
    QCOMPARE(QProcess::execute("copier/copier", stringArgument), 0);
    QCOMPARE(QProcess::execute("paster/paster", stringArgument), 0);
}

QTEST_MAIN(tst_QClipboard)

#include "tst_qclipboard.moc"
