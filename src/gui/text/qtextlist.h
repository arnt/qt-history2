#ifndef QTEXTLIST_H
#define QTEXTLIST_H

#ifndef QT_H
#include <qshareddata.h>
#include <qtextformat.h>
#include <qobject.h>
#endif // QT_H

class QTextListPrivate;
class QTextPieceTable;
class QTextCursor;

class Q_GUI_EXPORT QTextList : public QTextBlockGroup
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTextList)
public:
    int count() const;

    inline bool isEmpty() const
    { return count() == 0; }

    QTextBlockIterator item(int i) const;

    int itemNumber(const QTextBlockIterator &) const;
    QString itemText(const QTextBlockIterator &) const;

    void removeItem(int i);

    void setFormat(const QTextListFormat &format) { QTextObject::setFormat(format); }
    QTextListFormat format() const { return QTextObject::format().toListFormat(); }

private:
    QTextList(QObject *parent);
    ~QTextList();

    friend class QTextPieceTable;

#if defined(Q_DISABLE_COPY)
    QTextList(const QTextList &rhs);
    QTextList &operator=(const QTextList &rhs);
#endif
};

#endif // QTEXTLIST_H
