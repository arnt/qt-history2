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

class Q_GUI_EXPORT QTextList : public QTextFormatGroup
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTextList);
public:
    int count() const;

    inline bool isEmpty() const
    { return count() == 0; }

    QTextBlockIterator item(int i) const;

    int itemNumber(const QTextBlockIterator &) const;
    QString itemText(const QTextBlockIterator &) const;


    void setFormat(const QTextListFormat &format) { setCommonFormat(format); }
    QTextListFormat format() const { return commonFormat().toListFormat(); }

private:
    QTextList();
    ~QTextList();

    friend class QTextFormatCollection;

#if defined(Q_DISABLE_COPY)
    QTextList(const QTextList &rhs);
    QTextList &operator=(const QTextList &rhs);
#endif
};

#endif // QTEXTLIST_H
