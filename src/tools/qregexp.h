/****************************************************************************
** $Id: //depot/qt/main/src/tools/qregexp.h#11 $
**
** Definition of QRegExp class
**
** Created : 950126
**
** Copyright (C) 1995-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QREGEXP_H
#define QREGEXP_H

#ifndef QT_H
#include "qstring.h"
#endif // QT_H


class QRegExp
{
public:
    QRegExp();
    QRegExp( const char *, bool caseSensitive=TRUE, bool wildcard=FALSE );
    QRegExp( const QRegExp & );
   ~QRegExp();
    QRegExp    &operator=( const QRegExp & );
    QRegExp    &operator=( const char *pattern );

    bool	operator==( const QRegExp & )  const;
    bool	operator!=( const QRegExp &r ) const
					{ return !(this->operator==(r)); }

    bool	isEmpty()	const	{ return rxdata == 0; }
    bool	isValid()	const	{ return error == 0; }

    bool	caseSensitive() const	{ return cs; }
    void	setCaseSensitive( bool );

    bool	wildcard()	const	{ return wc; }
    void	setWildcard( bool );

    const char *pattern()	const	{ return (const char *)rxstring; }

    int		match( const char *str, int index=0, int *len=0 ) const;

protected:
    void	compile();
    char       *matchstr( ushort *, char *, char * ) const;

private:
    QString	rxstring;			// regular expression pattern
    ushort     *rxdata;				// compiled regexp pattern
    int		error;				// error status
    bool	cs;				// case sensitive
    bool	wc;				// wildcard
};


#endif // QREGEXP_H
