/****************************************************************************
**
** Definition of QStringList class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSTRINGLIST_H
#define QSTRINGLIST_H

#ifndef QT_H
#include "qalgorithms.h"
#include "qdatastream.h"
#include "qlist.h"
#include "qregexp.h"
#include "qstring.h"
#include "qstringmatcher.h"
#endif // QT_H

#ifdef QT_INCLUDE_COMPAT
#include <qvaluelist.h>
#endif

class QRegExp;

typedef QListIterator<QString> QStringListIterator;
typedef QListMutableIterator<QString> QStringListMutableIterator;

class QStringList : public QList<QString>
{
public:
    inline QStringList() { }
    inline QStringList(const QString &i) { append(i); }
#ifndef QT_NO_CAST_FROM_ASCII
    inline QStringList(const char *i) { append(i); }
#endif
    inline QStringList(const QStringList &l) : QList<QString>(l) { }
    inline QStringList(const QList<QString> &l) : QList<QString>(l) { }

    inline void sort();

#ifdef QT_COMPAT
    inline static QT_COMPAT QStringList split(const QString &sep, const QString &str, bool allowEmptyEntries = false);
    static QT_COMPAT QStringList split(const QChar &sep, const QString &str, bool allowEmptyEntries = false);
    static QT_COMPAT QStringList split(const QRegExp &sep, const QString &str, bool allowEmptyEntries = false);
#endif
    inline QString join(const QString &sep) const;

    inline QStringList find(const QString &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    inline QStringList find(const QRegExp &rx) const;
    inline QBool contains(const QString &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

#ifndef Q_QDOC
    inline void replace(int i, const QString &s) { QList<QString>::replace(i, s); }
#endif
    inline QStringList &replace(const QString &before, const QString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    inline QStringList &replace(const QRegExp &rx, const QString &after);

    inline QStringList operator+(const QStringList &other) const
    { QStringList n = *this; n += other; return n; }
    inline QStringList &operator<<(const QString &str)
    { append(str); return *this; }

#ifdef QT_COMPAT
    inline QT_COMPAT QStringList grep(const QString &str, bool cs = true) const
        { return find(str, cs ? Qt::CaseSensitive : Qt::CaseInsensitive); }
    inline QT_COMPAT QStringList grep(const QRegExp &rx) const { return find(rx); }
    inline QT_COMPAT QStringList &gres(const QString &before, const QString &after, bool cs = true)
        { return replace(before, after, cs ? Qt::CaseSensitive : Qt::CaseInsensitive); }
    inline QT_COMPAT QStringList &gres(const QRegExp &rx, const QString &after)
        { return replace(rx, after); }
    Iterator QT_COMPAT fromLast() { return (isEmpty() ? end() : --end()); }
    ConstIterator QT_COMPAT fromLast() const { return (isEmpty() ? end() : --end()); }
#endif
};

#ifdef QT_COMPAT
inline QStringList QStringList::split(const QString &sep, const QString &str,
                                      bool allowEmptyEntries)
{
    if (str.isEmpty())
        return QStringList();
    return str.split(sep, allowEmptyEntries ? QString::KeepEmptyParts
                                            : QString::SkipEmptyParts);
}

inline QStringList QStringList::split(const QChar &sep, const QString &str, bool allowEmptyEntries)
{
    if (str.isEmpty())
        return QStringList();
    return str.split(sep, allowEmptyEntries ? QString::KeepEmptyParts
                                            : QString::SkipEmptyParts);
}

inline QStringList QStringList::split(const QRegExp &sep, const QString &str,
                                      bool allowEmptyEntries)
{
    if (str.isEmpty())
        return QStringList();
    return str.split(sep, allowEmptyEntries ? QString::KeepEmptyParts
                                            : QString::SkipEmptyParts);
}
#endif

inline void QStringList::sort()
{
    qHeapSort(*this);
}

inline QStringList QStringList::find(const QString &str, Qt::CaseSensitivity cs) const
{
    QStringMatcher matcher(str, cs);
    QStringList res;
    for (int i = 0; i < size(); ++i)
        if (matcher.indexIn(at(i)) != -1)
            res << at(i);
    return res;
}

inline QBool QStringList::contains(const QString &str, Qt::CaseSensitivity cs) const
{
    QStringMatcher matcher(str, cs);
    for (int i = 0; i < size(); ++i) {
        QString string(at(i));
        if (string.length() == str.length() && matcher.indexIn(string) == 0)
            return QBool(true);
    }
    return QBool(false);
}

#ifndef QT_NO_REGEXP
inline QStringList QStringList::find(const QRegExp &rx) const
{
    QStringList res;
    for (int i = 0; i < size(); ++i)
        if (at(i).contains(rx))
            res << at(i);
    return res;
}
#endif // QT_NO_REGEXP

inline QStringList &QStringList::replace(const QString &before, const QString &after,
                                         Qt::CaseSensitivity cs)
{
    for (int i = 0; i < size(); ++i)
        (*this)[i].replace(before, after, cs);
    return *this;
}

#ifndef QT_NO_REGEXP
inline QStringList& QStringList::replace(const QRegExp &rx, const QString &after)
{
    for (int i = 0; i < size(); ++i)
        (*this)[i].replace(rx, after);
    return *this;
}
#endif // QT_NO_REGEXP

inline QString QStringList::join(const QString &sep) const
{
    QString res;
    for (int i = 0; i < size(); ++i) {
        if (i)
            res += sep;
        res += at(i);
    }
    return res;
}

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
