#include "browser.h"
#include "editor.h"
#include <qrichtext_p.h>


EditorBrowser::EditorBrowser( Editor *e )
    : curEditor( e ), oldHighlightedParag( 0 )
{
    curEditor = e;
    curEditor->viewport()->installEventFilter( this );
    curEditor->installEventFilter( this );
    QFont fn( curEditor->font() );
    fn.setUnderline( TRUE );
    highlightedFormat = new QTextFormat( fn, blue );
}

EditorBrowser::~EditorBrowser()
{
    delete highlightedFormat;
}

bool EditorBrowser::eventFilter( QObject *o, QEvent *e )
{
    if ( o->parent() && o->parent()->inherits( "Editor" ) || o->inherits( "Editor" ) ) {
	QMouseEvent *me;
	QKeyEvent *ke;
	switch ( e->type() ) {
	case QEvent::MouseMove:
	    me = (QMouseEvent*)e;
	    if ( ( me->state() & ControlButton ) == ControlButton ) {
		curEditor->viewport()->setCursor( pointingHandCursor );
		QTextCursor c( curEditor->document() );
		curEditor->placeCursor( curEditor->viewportToContents( me->pos() ), &c );
		QTextCursor from, to;
		if ( oldHighlightedParag ) {
		    oldHighlightedParag->setEndState( -1 );
		    oldHighlightedParag->format();
		    oldHighlightedParag = 0;
		}
		if ( findCursor( c, from, to ) && from.parag() == to.parag() ) {
		    // avoid collision with other selections
		    for ( int i = 0; i < curEditor->document()->numSelections(); ++i )
			curEditor->document()->removeSelection( i );
		    from.parag()->setFormat( from.index(), to.index() - from.index() + 1, highlightedFormat, FALSE );
		    lastWord = from.parag()->string()->toString().mid( from.index(), to.index() - from.index() + 1 );
		    oldHighlightedParag = from.parag();
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

bool EditorBrowser::findCursor( const QTextCursor &c, QTextCursor &from, QTextCursor &to )
{
    from = c;
    while ( from.parag()->at( from.index() )->c != ' ' && from.parag()->at( from.index() )->c != '\t'  && from.index() > 0 )
	from.gotoLeft();
    if ( from.parag()->at( from.index() )->c == ' ' || from.parag()->at( from.index() )->c == '\t' )
	from.gotoRight();
    to = c;
    while ( to.parag()->at( to.index() )->c != ' ' && to.parag()->at( to.index() )->c != '\t' &&
	    to.index() < to.parag()->length() - 1 )
	to.gotoRight();
    if ( to.parag()->at( to.index() )->c == ' ' || to.parag()->at( to.index() )->c == '\t' )
	to.gotoLeft();
    return TRUE;
}
