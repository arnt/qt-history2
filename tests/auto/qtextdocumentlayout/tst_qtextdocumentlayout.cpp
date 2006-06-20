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
#if QT_VERSION >= 0x040200
    void defaultPageSizeHandling();
#endif

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

#if QT_VERSION >= 0x040200
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
#endif

QTEST_MAIN(tst_QTextDocumentLayout)
#include "tst_qtextdocumentlayout.moc"
