/****************************************************************************
** $Id: $
**
** Definition of QStringList class
**
** Created : 990406
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

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
