/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "asciivalidator.h"

#include <qstring.h>

AsciiValidator::AsciiValidator( QObject * parent, const char *name )
    : QValidator( parent, name ), functionName( false )
{
}

AsciiValidator::AsciiValidator( bool funcName, QObject * parent, const char *name )
    : QValidator( parent, name ), functionName( funcName )
{
}

AsciiValidator::AsciiValidator( const QString &allow, QObject * parent, const char *name )
    : QValidator( parent, name ), functionName( false ), allowedChars( allow )
{
}

AsciiValidator::~AsciiValidator()
{
}

QValidator::State AsciiValidator::validate( QString &s, int & ) const
{
    bool inParen = false;
    bool outParen = false;
    if ( !s.isEmpty() && s[0].row() == 0 && s[0].cell() >= '0' && s[0].cell() <= '9' )
	s[0] = '_';
    for ( int i = 0, j = 0; i < (int) s.length(); i++ ) {
	uchar r = s[i].row();
	uchar c = s[i].cell();

	if ( outParen ) { // check if we have 'const' or 'volatile'
	    static const QString con = " const";
	    static const QString vol = " volatile";
	    QString mid = s.mid( j );
	    if ( !( con.startsWith( mid ) || vol.startsWith( mid ) ) )
		return QValidator::Invalid;
	}

	if ( inParen && c != ')' )
	    continue;

	if ( r == 0 && ( ( c >= '0' && c <= '9' ) ||
			 ( c >= 'a' && c <= 'z' ) ||
			 ( c >= 'A' && c <= 'Z' ) ) )
	    continue;
	
	if ( functionName ) {
	    if ( c == '(' ) {
		inParen = true;
		continue;
	    }
	    if ( c == ')' ) {
		outParen = true;
		j = i + 1;
		continue;
	    }
	}
	
	if ( allowedChars.find( s[ i ] ) != -1 )
	    continue;
	
	s[i] = '_';
    }
    return QValidator::Acceptable;
}
