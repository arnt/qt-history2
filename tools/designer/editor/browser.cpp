/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "browser.h"
#include "editor.h"
#include <private/qrichtext_p.h>
#include <qcursor.h>
#include <qevent.h>

EditorBrowser::EditorBrowser( Editor *e )
    : curEditor( e ), oldHighlightedParag( 0 )
{
    curEditor = e;

    // ### disabled for now
//     curEditor->viewport()->installEventFilter( this );
//     curEditor->installEventFilter( this );

    QFont fn( curEditor->font() );
    fn.setUnderline( TRUE );
    highlightedFormat = new Q3TextFormat( fn, blue );
}

EditorBrowser::~EditorBrowser()
{
    delete highlightedFormat;
}

bool EditorBrowser::eventFilter( QObject *o, QEvent *e )
{
    if ( qt_cast<Editor*>(o->parent()) || qt_cast<Editor*>(o) ) {
	QMouseEvent *me;
	QKeyEvent *ke;
	switch ( e->type() ) {
	case QEvent::MouseMove:
	    me = (QMouseEvent*)e;
	    if ( ( me->state() & ControlButton ) == ControlButton ) {
		curEditor->viewport()->setCursor( pointingHandCursor );
		Q3TextCursor c( curEditor->document() );
		curEditor->placeCursor( curEditor->viewportToContents( me->pos() ), &c );
		Q3TextCursor from, to;
		if ( oldHighlightedParag ) {
		    oldHighlightedParag->setEndState( -1 );
		    oldHighlightedParag->format();
		    oldHighlightedParag = 0;
		}
		if ( findCursor( c, from, to ) && from.paragraph() == to.paragraph() ) {
		    // avoid collision with other selections
		    for ( int i = 0; i < curEditor->document()->numSelections(); ++i )
			curEditor->document()->removeSelection( i );
		    from.paragraph()->setFormat( from.index(), to.index() - from.index() + 1, highlightedFormat, FALSE );
		    lastWord = from.paragraph()->string()->toString().mid( from.index(), to.index() - from.index() + 1 );
		    oldHighlightedParag = from.paragraph();
		} else {
		    lastWord = "";
		}
		curEditor->repaintChanged();
		return TRUE;
	    }
	    break;
	case QEvent::MouseButtonPress: {
	    bool killEvent = !lastWord.isEmpty();
	    if ( !lastWord.isEmpty() )
		showHelp( lastWord );
	    lastWord = "";
	    curEditor->viewport()->setCursor( ibeamCursor );
	    if ( oldHighlightedParag ) {
		oldHighlightedParag->setEndState( -1 );
		oldHighlightedParag->format();
		curEditor->repaintChanged();
		oldHighlightedParag = 0;
	    }
	    if ( killEvent )
		return TRUE;
	} break;
	case QEvent::KeyRelease:
	    lastWord = "";
	    ke = (QKeyEvent*)e;
	    if ( ke->key() == Key_Control ) {
		curEditor->viewport()->setCursor( ibeamCursor );
		if ( oldHighlightedParag ) {
		    oldHighlightedParag->setEndState( -1 );
		    oldHighlightedParag->format();
		    curEditor->repaintChanged();
		    oldHighlightedParag = 0;
		}
	    }
	default:
	    break;
	}
    }
    return FALSE;
}

void EditorBrowser::setCurrentEdior( Editor *e )
{
    curEditor = e;
    curEditor->installEventFilter( this );
}

void EditorBrowser::addEditor( Editor *e )
{
    e->installEventFilter( this );
}

bool EditorBrowser::findCursor( const Q3TextCursor &c, Q3TextCursor &from, Q3TextCursor &to )
{
    from = c;
    while ( from.paragraph()->at( from.index() )->c != ' ' && from.paragraph()->at( from.index() )->c != '\t'  && from.index() > 0 )
	from.gotoLeft();
    if ( from.paragraph()->at( from.index() )->c == ' ' || from.paragraph()->at( from.index() )->c == '\t' )
	from.gotoRight();
    to = c;
    while ( to.paragraph()->at( to.index() )->c != ' ' && to.paragraph()->at( to.index() )->c != '\t' &&
	    to.index() < to.paragraph()->length() - 1 )
	to.gotoRight();
    if ( to.paragraph()->at( to.index() )->c == ' ' || to.paragraph()->at( to.index() )->c == '\t' )
	to.gotoLeft();
    return TRUE;
}
