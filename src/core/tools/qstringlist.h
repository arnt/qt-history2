/****************************************************************************
**
** Definition of QStringList class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
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
#include "qlist.h"
#include "qstring.h"
#endif // QT_H

#ifdef QT_INCLUDE_COMPAT
#include <qvaluelist.h>
#endif

class QRegExp;

class Q_CORE_EXPORT QStringList : public QList<QString>
{
public:
    inline QStringList() { }
    inline QStringList(const QStringList &l) : QList<QString>(l) { }
    inline QStringList(const QList<QString> &l) : QList<QString>(l) { }
    inline QStringList(const QString &i) { append(i); }
#ifndef QT_NO_CAST_FROM_ASCII
    inline QStringList(const char *i) { append(i); }
#endif

    void sort();

#ifdef QT_COMPAT
    inline static QT_COMPAT QStringList split(const QString &sep, const QString &str, bool allowEmptyEntries = false);
    static QT_COMPAT QStringList split(const QChar &sep, const QString &str, bool allowEmptyEntries = false);
    static QT_COMPAT QStringList split(const QRegExp &sep, const QString &str, bool allowEmptyEntries = false);
#endif
    QString join(const QString &sep) const;

    QStringList find(const QString &str, QString::CaseSensitivity cs = QString::CaseSensitive) const;
    QStringList find(const QRegExp &rx) const;

    QStringList &replace(const QString &before, const QString &after, QString::CaseSensitivity cs = QString::CaseSensitive);
    QStringList &replace(const QRegExp &rx, const QString &after);

#ifdef QT_COMPAT
    inline QT_COMPAT QStringList grep(const QString &str, bool cs = true) const
	{ return find(str, cs ? QString::CaseSensitive : QString::CaseInsensitive); }
    inline QT_COMPAT QStringList grep(const QRegExp &rx) const { return find(rx); }
    inline QT_COMPAT QStringList &gres(const QString &before, const QString &after, bool cs = true)
	{ return replace(before, after, cs ? QString::CaseSensitive : QString::CaseInsensitive); }
    inline QT_COMPAT QStringList &gres(const QRegExp &rx, const QString &after)
	{ return replace(rx, after); }
    Iterator QT_COMPAT fromLast() { return (isEmpty() ? end() : --end()); }
    ConstIterator QT_COMPAT fromLast() const { return (isEmpty() ? end() : --end()); }
#endif
};

#ifdef QT_COMPAT
inline
QStringList QStringList::split(const QString &sep, const QString &str, bool allowEmptyEntries)
{
    QStringList s = str.split(sep);
    if (!allowEmptyEntries)
	s.remove(QString());
    return s;
}

inline
QStringList QStringList::split(const QChar &sep, const QString &str, bool allowEmptyEntries)
{
    QStringList s = str.split(sep);
    if (!allowEmptyEntries)
	s.remove(QString());
    return s;
}
inline
QStringList QStringList::split(const QRegExp &sep, const QString &str, bool allowEmptyEntries)
{
    QStringList s = str.split(sep);
    if (!allowEmptyEntries)
	s.remove(QString());
    return s;
}
#endif

#ifndef QT_NO_DATASTREAM
template <class T>
Q_CORE_EXPORT QDataStream& operator>>( QDataStream& s, QStringList& l );

template <class T>
Q_CORE_EXPORT QDataStream& operator<<( QDataStream& s, const QStringList& l );
#endif // QT_NO_DATASTREAM

#endif // QSTRINGLIST_H
