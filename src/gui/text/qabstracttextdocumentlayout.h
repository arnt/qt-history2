/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QABSTRACTTEXTDOCUMENTLAYOUT_H
#define QABSTRACTTEXTDOCUMENTLAYOUT_H

#include <QtCore/qobject.h>
#include <QtGui/qtextlayout.h>
#include <QtGui/qtextdocument.h>
#include <QtGui/qtextcursor.h>
#include <QtGui/qpalette.h>

QT_MODULE(Gui)

class QAbstractTextDocumentLayoutPrivate;
class QTextBlock;
class QTextObjectInterface;
class QTextFrame;

class Q_GUI_EXPORT QAbstractTextDocumentLayout : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QAbstractTextDocumentLayout)

public:
    explicit QAbstractTextDocumentLayout(QTextDocument *doc);
    ~QAbstractTextDocumentLayout();

    struct Selection
    {
        QTextCursor cursor;
        QTextCharFormat format;
    };

    struct PaintContext
    {
        PaintContext()
            : cursorPosition(-1)
            {}
        int cursorPosition;
        QPalette palette;
        QRectF clip;
        QVector<Selection> selections;
    };

    virtual void draw(QPainter *painter, const PaintContext &context) = 0;
    virtual int hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const = 0;
    QString anchorAt(const QPointF& pos) const;

    virtual int pageCount() const = 0;
    virtual QSizeF documentSize() const = 0;

    int dynamicPageCount() const;
    QSizeF dynamicDocumentSize() const;
    void ensureLayouted(qreal y);

    virtual QRectF frameBoundingRect(QTextFrame *frame) const = 0;
    virtual QRectF blockBoundingRect(const QTextBlock &block) const = 0;

    void setPaintDevice(QPaintDevice *device);
    QPaintDevice *paintDevice() const;

    QTextDocument *document() const;

    void registerHandler(int objectType, QObject *component);
    QTextObjectInterface *handlerForObject(int objectType) const;

Q_SIGNALS:
    void update(const QRectF & = QRectF(0., 0., 1000000000., 1000000000.));
    void documentSizeChanged(const QSizeF &newSize);
    void pageCountChanged(int newPages);

protected:
    QAbstractTextDocumentLayout(QAbstractTextDocumentLayoutPrivate &, QTextDocument *);

    virtual void documentChanged(int from, int charsRemoved, int charsAdded) = 0;

    virtual void resizeInlineObject(QTextInlineObject item, int posInDocument, const QTextFormat &format);
    virtual void positionInlineObject(QTextInlineObject item, int posInDocument, const QTextFormat &format);
    virtual void drawInlineObject(QPainter *painter, const QRectF &rect, QTextInlineObject object, int posInDocument, const QTextFormat &format);

    int formatIndex(int pos);
    QTextCharFormat format(int pos);

private:
    friend class QTextDocument;
    friend class QTextDocumentPrivate;
    friend class QTextEngine;
    friend class QTextLayout;
    friend class QTextLine;
    Q_PRIVATE_SLOT(d_func(), void handlerDestroyed(QObject *obj))
    Q_PRIVATE_SLOT(d_func(), int dynamicPageCountSlot())
    Q_PRIVATE_SLOT(d_func(), QSizeF dynamicDocumentSizeSlot())
};

class QTextObjectInterface
{
public:
    virtual ~QTextObjectInterface() {}
    virtual QSizeF intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format) = 0;
    virtual void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format) = 0;
};
Q_DECLARE_INTERFACE(QTextObjectInterface, "com.trolltech.Qt.QTextObjectInterface")

#endif // QABSTRACTTEXTDOCUMENTLAYOUT_H
