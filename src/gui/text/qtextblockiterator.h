#ifndef QTEXTBLOCKITERATOR_H
#define QTEXTBLOCKITERATOR_H

#ifndef QT_H
#include <qglobal.h>
#endif

class QTextDocument;
class QTextDocumentPrivate;
class QTextLayout;
class QString;
class QTextBlockFormat;
class QTextCharFormat;

class QTextBlockIterator
{
    const QTextDocumentPrivate *pt;
    int n;

    friend class QTextDocumentPrivate;
    friend class QAbstractTextDocumentLayout;
    friend class QTextTablePrivate;
    friend class QTextTable;
    friend class QTextList;
    friend class QTextObject;
    friend class QTextCursor;

    inline QTextBlockIterator(const QTextDocumentPrivate *p, int b) : pt(p), n(b) {}
public:
    inline QTextBlockIterator() : pt(0), n(0) {}
    inline QTextBlockIterator(const QTextBlockIterator &o) : pt(o.pt), n(o.n) {}
    inline QTextBlockIterator &operator=(const QTextBlockIterator &o) { pt = o.pt; n = o.n; return *this; }

    int position() const;
    int length() const;

    bool contains(int position) const;

    QTextLayout *layout() const;
    QTextBlockFormat blockFormat() const;
    QTextCharFormat charFormat() const;

    QString blockText() const;

    inline bool atEnd() const { return !n; }

    inline bool operator==(const QTextBlockIterator& it) const { return pt == it.pt && n == it.n; }
    inline bool operator!=(const QTextBlockIterator& it) const { return pt != it.pt || n != it.n; }
    inline bool operator<(const QTextBlockIterator &it) const { return position() < it.position(); }

    QTextBlockIterator& operator++();
    QTextBlockIterator& operator--();

    const QTextDocument *document() const;
};

Q_DECLARE_TYPEINFO(QTextBlockIterator, Q_PRIMITIVE_TYPE);


#endif
