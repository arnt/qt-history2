/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qabstracttextdocumentlayout.h>
#include <qimage.h>

//TESTED_CLASS=
//TESTED_FILES=qabstracttextdocumentlayout.h

class tst_QAbstractTextDocumentLayout : public QObject
{
Q_OBJECT

public:
    tst_QAbstractTextDocumentLayout();
    virtual ~tst_QAbstractTextDocumentLayout();

private slots:
    void getSetCheck();
};

tst_QAbstractTextDocumentLayout::tst_QAbstractTextDocumentLayout()
{
}

tst_QAbstractTextDocumentLayout::~tst_QAbstractTextDocumentLayout()
{
}

class MyAbstractTextDocumentLayout : public QAbstractTextDocumentLayout
{
public:
    MyAbstractTextDocumentLayout(QTextDocument *doc) : QAbstractTextDocumentLayout(doc) {}
    void draw(QPainter *, const PaintContext &) {}
    int hitTest(const QPointF &, Qt::HitTestAccuracy) const { return 0; }
    int pageCount() const { return 0; }
    QSizeF documentSize() const { return QSizeF(); }
    QRectF frameBoundingRect(QTextFrame *) const { return QRectF(); }
    QRectF blockBoundingRect(const QTextBlock &) const { return QRectF(); }
    void documentChanged(int, int, int) {}
};

// Testing get/set functions
void tst_QAbstractTextDocumentLayout::getSetCheck()
{
    QTextDocument doc;
    MyAbstractTextDocumentLayout obj1(&doc);
    // QPaintDevice * QAbstractTextDocumentLayout::paintDevice()
    // void QAbstractTextDocumentLayout::setPaintDevice(QPaintDevice *)
    QImage *var1 = new QImage(QSize(10,10), QImage::Format_ARGB32_Premultiplied);
    obj1.setPaintDevice(var1);
    QCOMPARE(static_cast<QPaintDevice *>(var1), obj1.paintDevice());
    obj1.setPaintDevice((QPaintDevice *)0);
    QCOMPARE(static_cast<QPaintDevice *>(0), obj1.paintDevice());
    delete var1;
}

QTEST_MAIN(tst_QAbstractTextDocumentLayout)
#include "tst_qabstracttextdocumentlayout.moc"
