/**********************************************************************
**   Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
**   This file is part of Qt GUI Designer.
**
**   This file may be distributed under the terms of the GNU General
**   Public License version 2 as published by the Free Software
**   Foundation and appearing in the file COPYING included in the
**   packaging of this file. If you did not get the file, send email
**   to info@trolltech.com
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

#include "asciivalidator.h"

#include <qstring.h>

AsciiValidator::AsciiValidator( QWidget * parent, const char *name )
    : QValidator( parent, name ), functionName( FALSE )
{
}

AsciiValidator::AsciiValidator( bool funcName, QWidget * parent, const char *name )
    : QValidator( parent, name ), functionName( funcName )
{
}

AsciiValidator::AsciiValidator( const QString &allow, QWidget * parent, const char *name )
    : QValidator( parent, name ), functionName( FALSE ), allowedChars( allow )
{
}

AsciiValidator::~AsciiValidator()
{
}

QValidator::State AsciiValidator::validate( QString &s, int & ) const
{
    bool inParen = FALSE;
    for ( int i = 0; i < (int) s.length(); i++ ) {
	uchar r = s[i].row();
	uchar c = s[i].cell();
	if ( functionName && inParen ) {
	    if ( c != ')' )
		continue;
	    s.truncate( i + 1 );
	    return QValidator::Acceptable;
	}
	
	if ( r == 0 && ( ( c >= '0' && c <= '9' ) ||
			 ( c >= 'a' && c <= 'z' ) ||
			 ( c >= 'A' && c <= 'Z' ) ) )
	    continue;
	if ( functionName ) {
	    if ( c == '(' ) {
		inParen = TRUE;
		continue;
	    }
	}
	
	if ( allowedChars.find( s[ i ] ) != -1 )
	    continue;
	
	s[i] = '_';
    }
    return QValidator::Acceptable;
}
