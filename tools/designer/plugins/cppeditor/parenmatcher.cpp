#include "parenmatcher.h"
#include "paragdata.h"

#include "qtextedit.h"

#include "../../src/kernel/qrichtext_p.h"

ParenMatcher::ParenMatcher()
{
}

bool ParenMatcher::match( QTextCursor *cursor )
{
    QChar c( cursor->parag()->at( cursor->index() )->c );
    if ( c == '{' || c == '(' || c == '[' ) {
	return checkOpenParen( cursor );
    } else if ( cursor->index() > 0 ) {
	c = cursor->parag()->at( cursor->index() - 1 )->c;
	if ( c == '}' || c == ')' || c == ']' ) {
	    return checkClosedParen( cursor );
	}
    }

    if ( cursor->document()->hasSelection( QTextDocument::Selection1 ) ||
	 cursor->document()->hasSelection( QTextDocument::Selection2 ) ) {
	cursor->document()->removeSelection( QTextDocument::Selection1 );
	cursor->document()->removeSelection( QTextDocument::Selection2 );
	return TRUE;
    }

    return FALSE;
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
	
	    int id = QTextDocument::Selection2;
	    if ( c == '{' && closedParen.chr != '}' ||
		 c == '(' && closedParen.chr != ')' ||
		 c == '[' && closedParen.chr != ']' )
		id = QTextDocument::Selection1;
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
	
	++i;
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
	
	    int id = QTextDocument::Selection2;
	    if ( c == '}' && openParen.chr != '{' ||
		 c == ')' && openParen.chr != '(' ||
		 c == ']' && openParen.chr != '[' )
		id = QTextDocument::Selection1;
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
	
	--i;
    }

 bye:
    return FALSE;
}
