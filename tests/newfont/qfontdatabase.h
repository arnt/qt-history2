/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontdatabase.h#11 $
**
** Definition of the QFontDatabase class
**
** Created : 981126
**
** Copyright (C) 1999-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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

#ifndef QFONTDATABASE_H
#define QFONTDATABASE_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qstring.h"
#include "qstringlist.h"
#include "qfont.h"
#include "qvaluelist.h"
#endif // QT_H


#ifndef QT_NO_FONTDATABASE

class QFontStylePrivate;
class QtFontStyle;
class QtFontFamily;
class QtFontFoundry;

class QFontDatabasePrivate;

class Q_EXPORT QFontDatabase
{
public:
    static QValueList<int> standardSizes();

    QFontDatabase();

    QStringList families() const;

    QStringList styles( const QString & ) const;

    // This will always return a list with only "Unicode"/"ISO 10646-1"
    QStringList charSets( const QString & ) const;

    QValueList<int> pointSizes( const QString &, const QString & = QString::null);

    QValueList<int> smoothSizes( const QString &, const QString &);

    QString styleString( const QFont &);

    QFont font( const QString &, const QString &, int);

    bool  isBitmapScalable( const QString &, const QString & = QString::null) const;

    bool  isSmoothlyScalable( const QString &, const QString & = QString::null) const;

    bool  isScalable( const QString &, const QString & = QString::null) const;

    bool italic( const QString &, const QString &) const;

    bool bold( const QString &, const QString &) const;

    int weight( const QString &, const QString &) const;




    // For source compatibility with < 3.0
#if 0 && !defined(QT_NO_COMPAT)

    QStringList families( bool onlyForLocale = TRUE ) const;

    QStringList styles( const QString &family,
			const QString &charSet = QString::null ) const;

    QStringList charSets( const QString &familyName,
			  bool onlyForLocale = TRUE ) const;

    QValueList<int> pointSizes( const QString &family,
				const QString &style = QString::null,
				const QString &charSet = QString::null );

    QValueList<int> smoothSizes( const QString &family,
				 const QString &style,
				 const QString &charSet = QString::null );

    QFont font( const QString familyName, const QString &style,
		int pointSize, const QString charSetName = QString::null );

    bool  isBitmapScalable( const QString &family,
			    const QString &style   = QString::null,
			    const QString &charSet = QString::null ) const;

    bool  isSmoothlyScalable( const QString &family,
			      const QString &style   = QString::null,
			      const QString &charSet = QString::null ) const;

    bool  isScalable( const QString &family,
		      const QString &style   = QString::null,
		      const QString &charSet = QString::null ) const;

    bool italic( const QString &family,
		 const QString &style,
		 const QString &charSet = QString::null ) const;

    bool bold( const QString &family,
	       const QString &style,
	       const QString &charSet = QString::null ) const;

    int weight( const QString &family,
		const QString &style,
		const QString &charSet = QString::null ) const;

#endif // QT_NO_COMPAT




    // What do we do about this?  It's not enabled for 2.x, and is obsolete in 3.x
#if 0
    QValueList<QFont::CharSet> charSets( const QString &familyName ) const;
    bool  supportsCharSet( const QString &familyName,
			   const QString &charSet ) const;
    bool  supportsCharSet( const QString &familyName,
			   QFont::CharSet charSet ) const;
#endif


    // ARGH
    static QString verboseCharSetName( const QString & );
    static QString charSetSample( const QString & );


private:
    static void createDatabase();

    friend class QtFontStyle;
    friend class QtFontFamily;
    friend class QtFontFoundry;
    friend class QFontDatabasePrivate;

    QFontDatabasePrivate *d;
};


#endif // QT_NO_FONTDATABASE

#endif // QFONTDATABASE_H
