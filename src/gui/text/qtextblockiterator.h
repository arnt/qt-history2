#ifndef QTEXTBLOCKITERATOR_H
#define QTEXTBLOCKITERATOR_H

#ifndef QT_H
#include <qglobal.h>
#endif

class QTextPieceTable;
class QTextLayout;
class QString;
class QTextBlockFormat;
class QTextCharFormat;

class QTextBlockIterator
{
    const QTextPieceTable *pt;
    int n;

    friend class QTextPieceTable;
    friend class QAbstractTextDocumentLayout;
    friend class QTextTablePrivate;
    friend class QTextFormatGroup;

    inline QTextBlockIterator(const QTextPieceTable *p, int b) : pt(p), n(b) {}
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

    // ######
    const QTextPieceTable *pieceTable() const { return pt; }
};

Q_DECLARE_TYPEINFO(QTextBlockIterator, Q_PRIMITIVE_TYPE);


#endif
