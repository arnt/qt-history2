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

class QRect;
class QRegion;
class QAbstractTextDocumentLayoutPrivate;
class QTextBlock;
class QTextObjectInterface;
class QTextFrame;

class Q_GUI_EXPORT QAbstractTextDocumentLayout : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QAbstractTextDocumentLayout)
    friend class QTextDocument;

public:
    struct PaintContext
    {
        PaintContext()
            : showCursor(false)
            {}
        QTextCursor cursor;
        QPalette palette;
        bool showCursor;
        QRect rect;
    };

    explicit QAbstractTextDocumentLayout(QTextDocument *doc);

    virtual void draw(QPainter *painter, const PaintContext &context) = 0;
    virtual int hitTest(const QPoint &point, Qt::HitTestAccuracy accuracy) const = 0;

    virtual void documentChange(int from, int oldLength, int length) = 0;

    virtual int numPages() const = 0;

    void registerHandler(int objectType, QObject *component);
    QTextObjectInterface *handlerForObject(int objectType) const;

    virtual void setSize(QTextInlineObject item, const QTextFormat &format);
    virtual void layoutObject(QTextInlineObject item, const QTextFormat &format);
    virtual void drawObject(QPainter *painter, const QRectF &rect, QTextInlineObject object, const QTextFormat &format);

    virtual void setPageSize(const QSize &size) = 0;
    virtual QSize pageSize() const = 0;

    virtual QSize sizeUsed() const;

    QString anchorAt(const QPoint& pos) const;

    virtual QRect frameBoundingRect(QTextFrame *frame) const;

    void setDefaultFont(const QFont &font);
    QFont defaultFont() const;

protected:
    QAbstractTextDocumentLayout(QAbstractTextDocumentLayoutPrivate &, QTextDocument *);

    int formatIndex(int pos);
    QTextCharFormat format(int pos);

    QTextDocument *document() const;
signals:
    void update(const QRect & = QRect(0, 0, 0x10000000, 0x10000000));

private:
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
