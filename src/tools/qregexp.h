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
#endif // QT_H


class Q_EXPORT QRegExp
{
public:
    QRegExp();
    QRegExp( const QString &, bool caseSensitive=TRUE, bool wildcard=FALSE );
    QRegExp( const QRegExp & );
   ~QRegExp();
    QRegExp    &operator=( const QRegExp & );
    QRegExp    &operator=( const QString &pattern );

    bool	operator==( const QRegExp & )  const;
    bool	operator!=( const QRegExp &r ) const
					{ return !(this->operator==(r)); }

    bool	isEmpty()	const	{ return rxdata == 0; }
    bool	isValid()	const	{ return error == 0; }

    bool	caseSensitive() const	{ return cs; }
    void	setCaseSensitive( bool );

    bool	wildcard()	const	{ return wc; }
    void	setWildcard( bool );

    QString	pattern()	const	{ return rxstring; }
    // ### in Qt 3.0, provide a real implementation
    void	setPattern( const QString& pattern )
					{ operator=( pattern ); }

    int		match( const QString &str, int index=0, int *len=0,
		       bool indexIsStart = TRUE ) const;
    int		find( const QString& str, int index )
					{ return match( str, index ); }

protected:
    void	compile();
    const QChar *matchstr( uint *, const QChar *, uint, const QChar * ) const;

private:
    QString	rxstring;			// regular expression pattern
    uint	*rxdata;			// compiled regexp pattern
    int		error;				// error status
    bool	cs;				// case sensitive
    bool	wc;				// wildcard
};


#endif // QREGEXP_H
