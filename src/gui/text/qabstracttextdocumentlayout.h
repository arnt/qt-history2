#ifndef QABSTRACTTEXTDOCUMENTLAYOUT_H
#define QABSTRACTTEXTDOCUMENTLAYOUT_H

#ifndef QT_H
#include <qobject.h>
#include <qtextlayout.h>
#include <qtextdocument.h>
#include <qtextcursor.h>
#include <qpalette.h>
#endif

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
        PaintContext() { showCursor = false; textColorFromPalette = false; }
        QTextCursor cursor;
        QPalette palette;
        bool showCursor;
        bool textColorFromPalette;
    };

    QAbstractTextDocumentLayout(QTextDocument *doc);

    virtual void draw(QPainter *painter, const PaintContext &context) = 0;
    virtual int hitTest(const QPoint &point, QText::HitTestAccuracy accuracy) const = 0;

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

    QString anchorAt(const QPoint& pos) const;

protected:
    QAbstractTextDocumentLayout(QAbstractTextDocumentLayoutPrivate &, QTextDocument *);

    int formatIndex(int pos);
    QTextCharFormat format(int pos);

    const QTextDocument *document() const;
signals:
    void update(const QRect & = QRect(0, 0, 0x10000000, 0x10000000));

private:
    Q_PRIVATE_SLOT(void handlerDestroyed(QObject *obj));
};

class QTextObjectInterface
{
public:
    virtual QSize intrinsicSize(const QTextFormat &format) = 0;
    virtual void drawObject(QPainter *painter, const QRect &rect, const QTextFormat &format) = 0;
};
Q_DECLARE_INTERFACE(QTextObjectInterface, "http://trolltech.com/Qt/QTextObjectInterface")

#endif
