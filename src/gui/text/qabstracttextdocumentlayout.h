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

    struct PaintContext
    {
        PaintContext()
            : showCursor(false)
            {}
        QTextCursor cursor;
        QPalette palette;
        bool showCursor;
        QRectF clip;
    };

    virtual void draw(QPainter *painter, const PaintContext &context) = 0;
    virtual int hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const = 0;
    QString anchorAt(const QPointF& pos) const;

    virtual int pageCount() const = 0;
    virtual QSizeF documentSize() const = 0;

    virtual QRectF frameBoundingRect(QTextFrame *frame) const;

    void setPaintDevice(QPaintDevice *device);
    QPaintDevice *paintDevice() const;

    QTextDocument *document() const;

signals:
    void update(const QRectF & = QRectF(0., 0., 1000000000., 1000000000.));
    void documentSizeChanged(const QSizeF &newSize);
    void pageCountChanged(int newPages);

protected:
    QAbstractTextDocumentLayout(QAbstractTextDocumentLayoutPrivate &, QTextDocument *);

    virtual void documentChange(int from, int oldLength, int length) = 0;

    virtual void resizeInlineObject(QTextInlineObject item, const QTextFormat &format);
    virtual void positionInlineObject(QTextInlineObject item, const QTextFormat &format);
    virtual void drawInlineObject(QPainter *painter, const QRectF &rect, QTextInlineObject object, const QTextFormat &format);

    void registerHandler(int objectType, QObject *component);
    QTextObjectInterface *handlerForObject(int objectType) const;

    int formatIndex(int pos);
    QTextCharFormat format(int pos);

private:
    friend class QTextDocument;
    friend class QTextDocumentPrivate;
    friend class QTextEngine;
    friend class QTextLayout;
    friend class QTextLine;
    Q_PRIVATE_SLOT(d, void handlerDestroyed(QObject *obj))
};

class QTextObjectInterface
{
public:
    virtual QSizeF intrinsicSize(QTextDocument *doc, const QTextFormat &format) = 0;
    virtual void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, const QTextFormat &format) = 0;
};
Q_DECLARE_INTERFACE(QTextObjectInterface, "http://trolltech.com/Qt/QTextObjectInterface")

#endif // QABSTRACTTEXTDOCUMENTLAYOUT_H
