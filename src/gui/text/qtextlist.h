#ifndef QTEXTLIST_H
#define QTEXTLIST_H

#ifndef QT_H
#include <qshareddatapointer.h>
#include <qtextformat.h>
#include <qobject.h>
#endif // QT_H

class QTextListPrivate;
class QTextPieceTable;
class QTextCursor;

class Q_GUI_EXPORT QTextList : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTextList);
public:
    int count() const;

    inline bool isEmpty() const
    { return count() == 0; }

    QTextCursor item(int i) const;

    void setFormat(const QTextListFormat &format);
    QTextListFormat format() const;

private:
    QTextList(QTextPieceTable *table, QObject *parent);
    ~QTextList();

    friend class QTextListManager;
    friend class QTextListItem;
    QTextPieceTable *tbl;

#if defined(Q_DISABLE_COPY)
    QTextList(const QTextList &rhs);
    QTextList &operator=(const QTextList &rhs);
#endif
};

#endif // QTEXTLIST_H
