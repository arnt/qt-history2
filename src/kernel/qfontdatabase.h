/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontdatabase.h#5 $
**
** Definition of the QFontDatabase class
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
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QFONTDATABASE_H
#define QFONTDATABASE_H

#include "qwindowdefs.h"
#ifndef _WS_WIN_

//
//  W A R N I N G
//  -------------
//
//  This class is under development and is currently unstable.
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
#include "qstringlist.h"
#include "qfont.h"
#include "qlist.h"
#include "qvaluelist.h"
#endif // QT_H

class QStringList;
class QFontStylePrivate;

class QFontDatabasePrivate;

class Q_EXPORT QFontDatabase
{
public:
    QFontDatabase();

    const QStringList &families( bool onlyForLocale = TRUE ) const;
    const QStringList &styles( const QString &family,
			       const QString &charSet = QString::null ) const;
    const QValueList<int> pointSizes( const QString &family,
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

    const QValueList<int> smoothSizes( const QString &family,
				      const QString &style,
				      const QString &charSet = QString::null );

    static const QValueList<int> standardSizes();

    bool italic( const QString &family,
		 const QString &style,
		 const QString &charSet = QString::null ) const;

    bool bold( const QString &family,
	       const QString &style,
	       const QString &charSet = QString::null ) const;

    int weight( const QString &family,
		const QString &style,
		const QString &charSet = QString::null ) const;


#if 0
    const QValueList<QFont::CharSet> charSets( const QString &familyName ) const;
    bool  supportsCharSet( const QString &familyName,
			   const QString &charSet ) const;
    bool  supportsCharSet( const QString &familyName,
			   QFont::CharSet charSet ) const;
#endif

const QStringList charSets( const QString &familyName,
			    bool onlyForLocale = TRUE ) const;



private:
    static void createDatabase();

    QFontDatabasePrivate *d;
};

#endif
#endif // QFONTDATABASE_H
