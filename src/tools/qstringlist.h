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

class QRegExp;

class Q_EXPORT QStringList : public QList<QString>
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

    static QStringList split(const QString &sep, const QString &str, bool allowEmptyEntries = false);
    static QStringList split(const QChar &sep, const QString &str, bool allowEmptyEntries = false);
    static QStringList split(const QRegExp &sep, const QString &str, bool allowEmptyEntries = false);
    QString join(const QString &sep) const;

    QStringList grep(const QString &str, QString::CaseSensitivity cs = QString::CaseSensitive) const;
    QStringList grep(const QRegExp &expr) const;

    QStringList &gres(const QString &before, const QString &after, QString::CaseSensitivity cs = QString::CaseSensitive);
    QStringList &gres(const QRegExp &expr, const QString &after);

#ifndef QT_NO_COMPAT
    inline QStringList grep(const QString &str, bool cs) const
	{ return grep(str, cs ? QString::CaseSensitive : QString::CaseInsensitive); }
    inline QStringList &gres(const QString &before, const QString &after, bool cs)
	{  return gres(before, after, cs ? QString::CaseSensitive : QString::CaseInsensitive); }
    Iterator fromLast() { return (isEmpty() ? end() : --end()); }
    ConstIterator fromLast() const { return (isEmpty() ? end() : --end()); }
#endif
};

#ifndef QT_NO_DATASTREAM
template <class T>
Q_EXPORT QDataStream& operator>>( QDataStream& s, QStringList& l )
{
    return operator>>(s, (QList<T>&)l);
}

template <class T>
Q_EXPORT QDataStream& operator<<( QDataStream& s, const QStringList& l )
{
    return operator<<(s, (QList<T>&)l);
}
#endif // QT_NO_DATASTREAM


#endif // QSTRINGLIST_H
