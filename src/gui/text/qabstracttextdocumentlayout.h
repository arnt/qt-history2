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

class QAbstractTextDocumentLayout : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QAbstractTextDocumentLayout);
    friend class QTextDocument;

public:
    struct PaintContext
    {
	QTextCursor cursor;
	QPalette palette;
	bool showCursor;
    };

    QAbstractTextDocumentLayout();

    virtual void draw(QPainter *painter, const PaintContext &context) = 0;
    virtual int hitTest(const QPoint &point, QText::HitTestAccuracy accuracy) const = 0;

    virtual void documentChange(int from, int oldLength, int length) = 0;

    virtual int numPages() const = 0;

public:
    void registerHandler(int formatType, QObject *component);
    virtual void layoutObject(QTextObject item, const QTextFormat &format);
    virtual void drawObject(QPainter *p, const QPoint &position, QTextObject item, const QTextFormat &format, QTextLayout::SelectionType selType);

    virtual void setPageSize(const QSize &size);
    QSize pageSize() const;

protected:
    void invalidate(const QRect &r);
    void invalidate(const QRegion &r);

    QTextLayout *layoutAt(int position) const;

private slots:
    void handlerDestroyed(QObject *obj);
};

class QTextObjectInterface
{
public:
    virtual void layoutObject(QTextObject object, const QTextFormat &format) = 0;
    virtual void drawObject(QPainter *painter, const QPoint &position, QTextObject object, const QTextFormat &format, QTextLayout::SelectionType selection) = 0;
};
Q_DECLARE_INTERFACE(QTextObjectInterface)

#endif
