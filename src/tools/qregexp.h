/****************************************************************************
** $Id: //depot/qt/main/src/tools/qregexp.h#18 $
**
** Definition of QRegExp class
**
** Created : 950126
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the tools
** module and therefore may only be used if the tools module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QREGEXP_H
#define QREGEXP_H

#ifndef QT_H
#include "qstring.h"
#include "qstringlist.h"
#endif

class QRegExpEngine;
struct QRegExpPrivate;

class Q_EXPORT QRegExp
{
public:
    QRegExp();
    QRegExp( const QString& pattern, bool caseSensitive = TRUE,
	     bool wildcard = FALSE, bool minimal = FALSE );
    QRegExp( const QRegExp& rx );
    ~QRegExp();
    QRegExp& operator=( const QRegExp& rx );

    bool operator==( const QRegExp& rx ) const;
    bool operator!=( const QRegExp& rx ) const { return !operator==( rx ); }

    bool isValid() const;
    QString pattern() const;
    void setPattern( const QString& pattern );
    bool caseSensitive() const;
    void setCaseSensitive( bool sensitive );
#ifndef QT_NO_REGEXP_WILDCARD
    bool wildcard() const;
    void setWildcard( bool wildcard );
#endif
    bool minimal() const;
    void setMinimal( bool minimal );

    bool match( const QString& str );
    bool match( const QString& str ) const;
#if defined(QT_OBSOLETE)
    int match( const QString& str, int index, int *len = 0,
	       bool indexIsStart = TRUE );
#endif
    int search( const QString& str, int start = 0 );
    int search( const QString& str, int start = 0 ) const;
    int searchRev( const QString& str, int start = -1 );
    int searchRev( const QString& str, int start = -1 ) const;
    int matchedLength();
#ifndef QT_NO_REGEXP_CAPTURE
    QString capturedText( int subexpression = 0 );
    QStringList capturedTexts();
#endif

private:
    void compile( bool caseSensitive );

    QRegExpEngine *eng;
    QRegExpPrivate *priv;
};

#endif // QREGEXP_H
