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
#include <qabstracttextdocumentlayout.h>
#include <qdebug.h>
#include <qtexttable.h>

//TESTED_CLASS=
//TESTED_FILES=gui/text/qtextdocumentlayout_p.h gui/text/qtextdocumentlayout.cpp

class tst_QTextDocumentLayout : public QObject
{
    Q_OBJECT
public:
    inline tst_QTextDocumentLayout() {}
    virtual ~tst_QTextDocumentLayout() {}

public slots:
    void init();
    void cleanup();

private slots:
    void defaultPageSizeHandling();
    void idealWidth();
    void multiPageTable();

private:
    QTextDocument *doc;
};

void tst_QTextDocumentLayout::init()
{
    doc = new QTextDocument;
}

void tst_QTextDocumentLayout::cleanup()
{
    delete doc;
    doc = 0;
}

void tst_QTextDocumentLayout::defaultPageSizeHandling()
{
    QAbstractTextDocumentLayout *layout = doc->documentLayout();
    QVERIFY(layout);

    QVERIFY(!doc->pageSize().isValid());
    QSizeF docSize = layout->documentSize();
    QVERIFY(docSize.width() > 0 && docSize.width() < 1000);
    QVERIFY(docSize.height() > 0 && docSize.height() < 1000);

    doc->setPlainText("Some text\nwith a few lines\nand not real information\nor anything otherwise useful");

    docSize = layout->documentSize();
    QVERIFY(docSize.isValid());
    QVERIFY(docSize.width() != INT_MAX);
    QVERIFY(docSize.height() != INT_MAX);
}

void tst_QTextDocumentLayout::idealWidth()
{
    doc->setPlainText("Some text\nwith a few lines\nand not real information\nor anything otherwise useful");
    doc->setTextWidth(1000);
    QCOMPARE(doc->textWidth(), qreal(1000));
    QCOMPARE(doc->size().width(), doc->textWidth());
    QVERIFY(doc->idealWidth() < doc->textWidth());
    QVERIFY(doc->idealWidth() > 0);

    QTextBlockFormat fmt;
    fmt.setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
    QTextCursor cursor(doc);
    cursor.select(QTextCursor::Document);
    cursor.mergeBlockFormat(fmt);

    QCOMPARE(doc->textWidth(), qreal(1000));
    QCOMPARE(doc->size().width(), doc->textWidth());
    QVERIFY(doc->idealWidth() < doc->textWidth());
    QVERIFY(doc->idealWidth() > 0);
}

void tst_QTextDocumentLayout::multiPageTable()
{
    QTextCursor cursor(doc);
    cursor.movePosition(QTextCursor::Start);

    QTextFrame *topFrame = cursor.currentFrame();

    QTextCharFormat format = cursor.charFormat();
    format.setFontFamily("Courier");
    QTextCharFormat boldFormat = format;
    boldFormat.setFontWeight(QFont::Bold);

    QTextTableFormat tableFormat;
    tableFormat.setBorder(1);
    QVector<QTextLength> constraints;

    constraints.clear();
    constraints << QTextLength(QTextLength::PercentageLength, 80);
    constraints << QTextLength(QTextLength::PercentageLength, 20);
    tableFormat.setColumnWidthConstraints(constraints);

    for (int i = 0; i < 10; ++i) {
        cursor.insertBlock();
        QTextTable *table = cursor.insertTable(4, 2, tableFormat);
        table->mergeCells(0, 1, 4, 1);
        QTextTableCell cell = table->cellAt(0, 1);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertText("bla bla bla bla bla bla bla bla "
                              "bla bla bla bla bla bla bla bla "
                              "bla bla bla bla bla bla bla bla "
                              "bla bla bla bla bla bla bla bla "
                              , format);

        for (int j = 0; j < 4; ++j) {
            cell = table->cellAt(j, 0);
            cell.firstCursorPosition().insertText("foo", format);
        }

        cursor = topFrame->lastCursorPosition();
    }

    const int margin = 87;
    const int pageWidth = 873;
    const int pageHeight = 1358;

    QTextFrameFormat fmt = doc->rootFrame()->frameFormat();
    fmt.setMargin(margin);
    doc->rootFrame()->setFrameFormat(fmt);

    QFont font(doc->defaultFont());
    font.setPointSize(10);
    doc->setDefaultFont(font);
    doc->setPageSize(QSizeF(pageWidth, pageHeight));

    QRectF marginRect(QPointF(0, pageHeight - margin), QSizeF(pageWidth, 2 * margin));

    QAbstractTextDocumentLayout *layout = doc->documentLayout();

    QList<QTextFrame *> childFrames = topFrame->childFrames();

    foreach (QTextFrame *tableFrame, childFrames)
        QVERIFY(!layout->frameBoundingRect(tableFrame).intersects(marginRect));
}

QTEST_MAIN(tst_QTextDocumentLayout)
#include "tst_qtextdocumentlayout.moc"
