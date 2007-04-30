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
    void lineSeparatorFollowingTable();

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

// none of the QTextLine items in the document should intersect with the margin rect
void tst_QTextDocumentLayout::lineSeparatorFollowingTable()
{
    QString html_begin("<html><table border=1><tr><th>Column 1</th></tr><tr><td>Data</td></tr></table><br>");
    QString html_text("bla bla bla bla bla bla bla bla<br>");
    QString html_end("<table border=1><tr><th>Column 1</th></tr><tr><td>Data</td></tr></table></html>");

    QString html = html_begin;

    for (int i = 0; i < 80; ++i)
        html += html_text;

    html += html_end;

    doc->setHtml(html);

    QTextCursor cursor(doc);
    cursor.movePosition(QTextCursor::Start);

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

    // force layouting
    doc->pageCount();

    for (QTextBlock block = doc->begin(); block != doc->end(); block = block.next()) {
        QTextLayout *layout = block.layout();
        for (int i = 0; i < layout->lineCount(); ++i) {
            QTextLine line = layout->lineAt(i);
            QRectF rect = line.rect().translated(layout->position());
            QVERIFY(!rect.intersects(marginRect));
        }
    }
}

QTEST_MAIN(tst_QTextDocumentLayout)
#include "tst_qtextdocumentlayout.moc"
