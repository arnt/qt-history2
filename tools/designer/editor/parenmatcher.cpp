/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "parenmatcher.h"
#include "paragdata.h"

#include "qtextedit.h"
#include <qrichtext_p.h>
#include <qapplication.h>

ParenMatcher::ParenMatcher()
{
    enabled = TRUE;
}

bool ParenMatcher::match( QTextCursor *cursor )
{
    if ( !enabled )
	return FALSE;
    bool ret = FALSE;

    QChar c( cursor->parag()->at( cursor->index() )->c );
    bool ok1 = FALSE;
    bool ok2 = FALSE;
    if ( c == '{' || c == '(' || c == '[' ) {
	ok1 = checkOpenParen( cursor );
	ret = ok1 || ret;
    } else if ( cursor->index() > 0 ) {
	c = cursor->parag()->at( cursor->index() - 1 )->c;
	if ( c == '}' || c == ')' || c == ']' ) {
	    ok2 = checkClosedParen( cursor );
	    ret = ok2 || ret;
	}
    }

    return ret;
}

bool ParenMatcher::checkOpenParen( QTextCursor *cursor )
{
    if ( !cursor->parag()->extraData() )
	return FALSE;
    ParenList parenList = ( (ParagData*)cursor->parag()->extraData() )->parenList;

    Paren openParen, closedParen;
    QTextParag *closedParenParag = cursor->parag();

    int i = 0;
    int ignore = 0;
    bool foundOpen = FALSE;
    QChar c = cursor->parag()->at( cursor->index() )->c;
    while ( TRUE ) {
	if ( !foundOpen ) {
	    if ( i >= (int)parenList.count() )
		goto bye;
	    openParen = parenList[ i ];
	    if ( openParen.pos != cursor->index() ) {
		++i;
		continue;
	    } else {
		foundOpen = TRUE;
		++i;
	    }
	}
	
	if ( i >= (int)parenList.count() ) {
	    while ( TRUE ) {
		closedParenParag = closedParenParag->next();
		if ( !closedParenParag )
		    goto bye;
		if ( closedParenParag->extraData() &&
		     ( (ParagData*)closedParenParag->extraData() )->parenList.count() > 0 ) {
		    parenList = ( (ParagData*)closedParenParag->extraData() )->parenList;
		    break;
		}
	    }
	    i = 0;
	}
	
	closedParen = parenList[ i ];
	if ( closedParen.type == Paren::Open ) {
	    ignore++;
	    ++i;
	    continue;
	} else {
	    if ( ignore > 0 ) {
		ignore--;
		++i;
		continue;
	    }

	    int id = Match;
	    if ( c == '{' && closedParen.chr != '}' ||
		 c == '(' && closedParen.chr != ')' ||
		 c == '[' && closedParen.chr != ']' )
		id = Mismatch;
	    cursor->document()->setSelectionStart( id, cursor );
	    int tidx = cursor->index();
	    QTextParag *tstring = cursor->parag();
	    cursor->setParag( closedParenParag );
	    cursor->setIndex( closedParen.pos + 1 );
	    cursor->document()->setSelectionEnd( id, cursor );
	    cursor->setParag( tstring );
	    cursor->setIndex( tidx );
	    return TRUE;
	}
    }

 bye:
    return FALSE;
}

bool ParenMatcher::checkClosedParen( QTextCursor *cursor )
{
    if ( !cursor->parag()->extraData() )
	return FALSE;
    ParenList parenList = ( (ParagData*)cursor->parag()->extraData() )->parenList;

    Paren openParen, closedParen;
    QTextParag *openParenParag = cursor->parag();

    int i = parenList.count() - 1;
    int ignore = 0;
    bool foundClosed = FALSE;
    QChar c = cursor->parag()->at( cursor->index() - 1 )->c;
    while ( TRUE ) {
	if ( !foundClosed ) {
	    if ( i < 0 )
		goto bye;
	    closedParen = parenList[ i ];
	    if ( closedParen.pos != cursor->index() - 1 ) {
		--i;
		continue;
	    } else {
		foundClosed = TRUE;
		--i;
	    }
	}
	
	if ( i < 0 ) {
	    while ( TRUE ) {
		openParenParag = openParenParag->prev();
		if ( !openParenParag )
		    goto bye;
		if ( openParenParag->extraData() &&
		     ( (ParagData*)openParenParag->extraData() )->parenList.count() > 0 ) {
		    parenList = ( (ParagData*)openParenParag->extraData() )->parenList;
		    break;
		}
	    }
	    i = parenList.count() - 1;
	}
	
	openParen = parenList[ i ];
	if ( openParen.type == Paren::Closed ) {
	    ignore++;
	    --i;
	    continue;
	} else {
	    if ( ignore > 0 ) {
		ignore--;
		--i;
		continue;
	    }
	
	    int id = Match;
	    if ( c == '}' && openParen.chr != '{' ||
		 c == ')' && openParen.chr != '(' ||
		 c == ']' && openParen.chr != '[' )
		id = Mismatch;
	    cursor->document()->setSelectionStart( id, cursor );
	    int tidx = cursor->index();
	    QTextParag *tstring = cursor->parag();
	    cursor->setParag( openParenParag );
	    cursor->setIndex( openParen.pos );
	    cursor->document()->setSelectionEnd( id, cursor );
	    cursor->setParag( tstring );
	    cursor->setIndex( tidx );
	    return TRUE;
	}
    }

 bye:
    return FALSE;
}
