/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSTRINGLIST_H
#define QSTRINGLIST_H

#include "qalgorithms.h"
#include "qdatastream.h"
#include "qlist.h"
#include "qregexp.h"
#include "qstring.h"
#include "qstringmatcher.h"

#ifdef QT_INCLUDE_COMPAT
#include <qvaluelist.h>
#endif

class QRegExp;

typedef QListIterator<QString> QStringListIterator;
typedef QListMutableIterator<QString> QStringListMutableIterator;

class Q_CORE_EXPORT QStringList : public QList<QString>
{
public:
    inline QStringList() { }
    inline Q_EXPLICIT QStringList(const QString &i) { append(i); }
    inline QStringList(const QStringList &l) : QList<QString>(l) { }
    inline QStringList(const QList<QString> &l) : QList<QString>(l) { }

    void sort();

    QString join(const QString &sep) const;

    QStringList find(const QString &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    QStringList find(const QRegExp &rx) const;
    QBool contains(const QString &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

    QStringList &replace(const QString &before, const QString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    QStringList &replace(const QRegExp &rx, const QString &after);

    inline QStringList operator+(const QStringList &other) const
    { QStringList n = *this; n += other; return n; }
    inline QStringList &operator<<(const QString &str)
    { append(str); return *this; }

#ifndef QT_NO_REGEXP
    int indexOf(const QRegExp &rx, int from = 0) const;
    int lastIndexOf(const QRegExp &rx, int from = -1) const;
#endif
#if !defined(Q_NO_USING_KEYWORD)
    using QList<QString>::indexOf;
    using QList<QString>::lastIndexOf;
    using QList<QString>::replace;
#else
    inline int indexOf(const QString &str, int from = 0) const
    { return QList<QString>::indexOf(str, from); }
    inline int lastIndexOf(const QString &str, int from = -1) const
    { return QList<QString>::lastIndexOf(str, from); }
    inline void replace(int i, const QString &s) { QList<QString>::replace(i, s); }
#endif
#ifdef QT_COMPAT
    static QT_COMPAT QStringList split(const QString &sep, const QString &str, bool allowEmptyEntries = false);
    static QT_COMPAT QStringList split(const QChar &sep, const QString &str, bool allowEmptyEntries = false);
    static QT_COMPAT QStringList split(const QRegExp &sep, const QString &str, bool allowEmptyEntries = false);
    inline QT_COMPAT QStringList grep(const QString &str, bool cs = true) const
        { return find(str, cs ? Qt::CaseSensitive : Qt::CaseInsensitive); }
    inline QT_COMPAT QStringList grep(const QRegExp &rx) const { return find(rx); }
    inline QT_COMPAT QStringList &gres(const QString &before, const QString &after, bool cs = true)
        { return replace(before, after, cs ? Qt::CaseSensitive : Qt::CaseInsensitive); }
    inline QT_COMPAT QStringList &gres(const QRegExp &rx, const QString &after)
        { return replace(rx, after); }
    inline Iterator QT_COMPAT fromLast() { return (isEmpty() ? end() : --end()); }
    inline ConstIterator QT_COMPAT fromLast() const { return (isEmpty() ? end() : --end()); }
#endif
};


#ifndef QT_NO_DATASTREAM
inline QDataStream &operator>>(QDataStream &in, QStringList &list)
{
    return operator>>(in, static_cast<QList<QString> &>(list));
}
inline QDataStream &operator<<(QDataStream &out, const QStringList &list)
{
    return operator<<(out, static_cast<const QList<QString> &>(list));
}
#endif // QT_NO_DATASTREAM

#endif // QSTRINGLIST_H
