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

#include <qobject.h>
#include <qtextlayout.h>
#include <qtextdocument.h>
#include <qtextcursor.h>
#include <qpalette.h>

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
            : showCursor(false), textColorFromPalette(false),
              imStart(0), imEnd(0), imSelectionStart(0), imSelectionEnd(0)
            {}
        QTextCursor cursor;
        QPalette palette;
        bool showCursor;
        bool textColorFromPalette;
        QRect rect;
        int imStart, imEnd, imSelectionStart, imSelectionEnd;
    };

    QAbstractTextDocumentLayout(QTextDocument *doc);

    virtual void draw(QPainter *painter, const PaintContext &context) = 0;
    virtual int hitTest(const QPoint &point, Qt::HitTestAccuracy accuracy) const = 0;

    virtual void documentChange(int from, int oldLength, int length) = 0;

    virtual int numPages() const = 0;

    void registerHandler(int objectType, QObject *component);
    QTextObjectInterface *handlerForObject(int objectType) const;

    virtual void setSize(QTextInlineObject item, const QTextFormat &format);
    virtual void layoutObject(QTextInlineObject item, const QTextFormat &format);
    virtual void drawObject(QPainter *painter, const QRect &rect, QTextInlineObject object, const QTextFormat &format,
                            QTextLayout::SelectionType selection);

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

    const QTextDocument *document() const;
signals:
    void update(const QRect & = QRect(0, 0, 0x10000000, 0x10000000));

private:
    Q_PRIVATE_SLOT(d, void handlerDestroyed(QObject *obj))
};

class QTextObjectInterface
{
public:
    virtual QSize intrinsicSize(const QTextDocument *doc, const QTextFormat &format) = 0;
    virtual void drawObject(QPainter *painter, const QRect &rect, const QTextDocument *doc, const QTextFormat &format) = 0;
};
Q_DECLARE_INTERFACE(QTextObjectInterface, "http://trolltech.com/Qt/QTextObjectInterface")

#endif
