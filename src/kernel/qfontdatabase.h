/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontdatabase.h#1 $
**
** Definition of the QFontDatabase class and friends
**
** Created : 981126
**
** Copyright (C) 1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QFONTDATABASE_H
#define QFONTDATABASE_H

//
//  W A R N I N G
//  -------------
//
//  These classes are under development and are currently unstable.
//
//  It is very unlikely that this code will be available in the final
//  Qt 2.0 release.  It will be available soon after then, but a number
//  of important API changes still need to be made.
//
//  Thus, it is important that you do NOT use this code in an application
//  unless you are willing for your application to be dependent on the
//  snapshot releases of Qt.
//

#ifndef QT_H
#include "qstring.h"
#include "qfont.h"
#include "qlist.h"
#endif // QT_H

class QStrList;
class QFontStylePrivate;

class Q_EXPORT QFontStyle
{
public:
    QFontStyle() {d =0;}  // Dangerous for the time being!
    QFontStyle( const QFont & ); // Dangerous for the time being!

    bool isNull() const;
    QString name() const;

    bool scalable() const;
    bool smoothlyScalable() const;

    bool italic() const;
    int  weight() const;

    const QArray<int> &pointSizes() const;
    int nSizes() const; // ### short term hack

    QFont font( int pointSize = 12 ) const;
private:
    QFontStyle( QFontStylePrivate* );

    QFontStylePrivate *d;
    friend class QFontDatabasePrivate;
    friend class QFontCharSetPrivate;
};


class QFontCharSetPrivate;
class QFontCharSet
{
public:
    QFontCharSet() {d =0;}  // Dangerous for the time being!
    bool isNull() const;
    QString name() const;
    QFont::CharSet charSet() const;

    bool scalable() const;
    bool smoothlyScalable() const;

    const QList<QFontStyle> &styles() const;
    const QStrList &styleNames() const;
    const QFontStyle &style( const QString &styleName ) const;
#if 0 
    const QFontStyle &normal() const;
    const QFontStyle &bold() const;
    const QFontStyle &italic() const;
    const QFontStyle &boldItalic() const;
#endif
private:
    QFontCharSet( QFontCharSetPrivate* );

    QFontCharSetPrivate *d;
    friend class QFontDatabasePrivate;
    friend class QFontFamilyPrivate;
    friend class QFontFamily;
    friend class QFontStylePrivate;
};


class QFontFamilyPrivate;

class Q_EXPORT QFontFamily
{
public:
    QFontFamily() {d =0;}
    QFontFamily( const QString &familyName );

    bool isNull() const;

    QString name() const;

    const QList<QFontCharSet> &charSets() const;
    const QStrList &charSetNames() const;
    const QFontCharSet &charSet( const QString &charSetName ) const;
    const QFontCharSet &charSet( QFont::CharSet ) const;

    bool supportsCharSet( QFont::CharSet cs ) const;

private:
    QFontFamily( QFontFamilyPrivate* );
    QFontFamilyPrivate *d;

    friend class QFontDatabasePrivate;
    friend class QFontStylePrivate;
};


class QFontDatabasePrivate;

class Q_EXPORT QFontDatabase
{
public:
    QFontDatabase();

    const QList<QFontFamily> &families() const;
    const QStrList &familyNames() const;
    const QFontFamily &family( const QString &familyName ) const;
private:
    QFontDatabasePrivate *d;
    friend class QFontDatabasePrivate;
};

#endif // QFONTDATABASE_H
