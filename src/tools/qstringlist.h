/****************************************************************************
** $Id: //depot/qt/main/src/tools/qstringlist.h#21 $
**
** Definition of QStringList class
**
** Created : 990406
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QSTRINGLIST_H
#define QSTRINGLIST_H

#ifndef QT_H
#include "qvaluelist.h"
#include "qstring.h"
#include "qregexp.h"
#endif // QT_H

class QStrList;

class Q_EXPORT QStringList : public QValueList<QString>
{
public:
    QStringList() { }
    QStringList( const QStringList& l ) : QValueList<QString>(l) { }
    QStringList( const QValueList<QString>& l ) : QValueList<QString>(l) { }
    QStringList( const QString& i ) { append(i); }
#ifndef QT_NO_CAST_ASCII
    QStringList( const char* i ) { append(i); }
#endif

    static QStringList fromStrList(const QStrList&);

    void sort();

    static QStringList split( const QString &sep, const QString &str, bool allowEmptyEntries = FALSE );
    static QStringList split(  const QChar &sep, const QString &str, bool allowEmptyEntries = FALSE );
    static QStringList split(  const QRegExp &sep, const QString &str, bool allowEmptyEntries = FALSE );
    QString join( const QString &sep ) const;

    QStringList grep( const QString &str, bool cs = TRUE ) const;
    QStringList grep( const QRegExp &expr ) const;
};

class QDataStream;

extern Q_EXPORT QDataStream &operator>>( QDataStream &, QStringList& );
extern Q_EXPORT QDataStream &operator<<( QDataStream &, const QStringList& );

#endif
