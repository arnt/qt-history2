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
class QTextPieceTable;

class Q_GUI_EXPORT QAbstractTextDocumentLayout : public QObject
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

    class BlockIterator {
        const QTextPieceTable *pt;
        int block;
    public:
        BlockIterator() { pt = 0; block = 0; }
        BlockIterator(const BlockIterator &other) { pt = other.pt; block = other.block; }
        BlockIterator(const QTextPieceTable *p, int b) { pt = p; block = b; }

        BlockIterator &operator=(const BlockIterator &other) { pt = other.pt; block = other.block; return *this; }

        QTextLayout *layout() const;
        QTextBlockFormat format() const;
        int formatIndex() const;

        int position() const;
        int length() const;

        inline bool atEnd() const { return !block; }

        bool operator==(const BlockIterator& it) const { return pt == it.pt && block == it.block; }
        bool operator!=(const BlockIterator& it) const { return pt != it.pt || block != it.block; }
        bool operator<(const BlockIterator &it) const;

        BlockIterator& operator++();
        BlockIterator& operator--();
    };

    BlockIterator findBlock(int pos) const;
    BlockIterator begin() const;
    BlockIterator end() const;

    int formatIndex(int pos);
    QTextCharFormat format(int pos);

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
