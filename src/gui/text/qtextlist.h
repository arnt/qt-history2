#ifndef QTEXTLIST_H
#define QTEXTLIST_H

#include <qsharedpointer.h>
#include <qtextformat.h>

#include "qtextpiecetable_p.h"

class QTextListPrivate;
class QTextPieceTable;
class QTextCursor;

class QTextList : public QObject
{
    Q_OBJECT
public:
    ~QTextList();

    int count() const;

    bool isEmpty() const
    { return count() == 0; }

    QTextCursor item(int i) const;

    void setFormat(const QTextListFormat &format);
    QTextListFormat format() const;

private:
    QTextList(QTextPieceTable *table, QObject *parent);

    friend class QTextListManager;
    friend class QTextListItem;
    QTextPieceTable *tbl;
    Q_DECL_PRIVATE(QTextList);

#if defined(Q_DISABLE_COPY)
    QTextList(const QTextList &rhs);
    QTextList &operator=(const QTextList &rhs);
#endif
};

#endif // QTEXTLIST_H
