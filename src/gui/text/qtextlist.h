#ifndef QTEXTLIST_H
#define QTEXTLIST_H

#ifndef QT_H
#include <qtextobject.h>
#include <qobject.h>
#endif // QT_H

class QTextListPrivate;
class QTextCursor;

class Q_GUI_EXPORT QTextList : public QTextBlockGroup
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTextList)
public:
    QTextList(QTextDocument *doc);
    ~QTextList();

    int count() const;

    inline bool isEmpty() const
    { return count() == 0; }

    QTextBlock item(int i) const;

    int itemNumber(const QTextBlock &) const;
    QString itemText(const QTextBlock &) const;

    void removeItem(int i);
    void remove(const QTextBlock &);

    void setFormat(const QTextListFormat &format) { QTextObject::setFormat(format); }
    QTextListFormat format() const { return QTextObject::format().toListFormat(); }

private:
#if defined(Q_DISABLE_COPY)
    QTextList(const QTextList &rhs);
    QTextList &operator=(const QTextList &rhs);
#endif
};

#endif // QTEXTLIST_H
