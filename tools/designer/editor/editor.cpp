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

#include "editor.h"
#include "parenmatcher.h"
#include <qfile.h>
#include <qtextedit.h>
#include "conf.h"
#include <private/qrichtext_p.h>
#include <qapplication.h>
#include <qpopupmenu.h>
#include <qaccel.h>
#include <qcstring.h>
#include <qevent.h>

Editor::Editor( const QString &fn, QWidget *parent, const char *name )
    : QTextEdit( parent, name ), hasError( FALSE )
{
    document()->setFormatter( new Q3TextFormatterBreakInWords );
    if ( !fn.isEmpty() )
	load( fn );
    setHScrollBarMode( QScrollView::AlwaysOff );
    setVScrollBarMode( QScrollView::AlwaysOn );
    document()->setUseFormatCollection( FALSE );
    parenMatcher = new ParenMatcher;
    connect( this, SIGNAL( cursorPositionChanged( Q3TextCursor * ) ),
	     this, SLOT( cursorPosChanged( Q3TextCursor * ) ) );
    cfg = new Config;
    document()->addSelection( Error );
    document()->addSelection( Step );
    document()->setSelectionColor( Error, red );
    document()->setSelectionColor( Step, yellow );
    document()->setInvertSelectionText( Error, FALSE );
    document()->setInvertSelectionText( Step, FALSE );
    document()->addSelection( ParenMatcher::Match );
    document()->addSelection( ParenMatcher::Mismatch );
    document()->setSelectionColor( ParenMatcher::Match, QColor( 204, 232, 195 ) );
    document()->setSelectionColor( ParenMatcher::Mismatch, Qt::magenta );
    document()->setInvertSelectionText( ParenMatcher::Match, FALSE );
    document()->setInvertSelectionText( ParenMatcher::Mismatch, FALSE );

    accelComment = new QAccel( this );
    accelComment->connectItem( accelComment->insertItem( ALT + Key_C ),
			       this, SLOT( commentSelection() ) );
    accelUncomment = new QAccel( this );
    accelUncomment->connectItem( accelUncomment->insertItem( ALT + Key_U ),
				 this, SLOT( uncommentSelection() ) );
    editable = TRUE;
}

Editor::~Editor()
{
    delete cfg;
    delete parenMatcher;
}

void Editor::cursorPosChanged( Q3TextCursor *c )
{
    if ( parenMatcher->match( c ) )
	repaintChanged();
    if ( hasError ) {
	emit clearErrorMarker();
	hasError = FALSE;
    }
}

void Editor::load( const QString &fn )
{
    filename = fn;
    QFile f( filename );
    if ( !f.open( IO_ReadOnly ) )
	return;
    QCString txt;
    txt.resize( f.size() );
    f.readBlock( txt.data(), f.size() );
    QString s( QString::fromLatin1( txt ) );
    setText( s );
}

void Editor::save( const QString &fn )
{
    if ( !filename.isEmpty() )
	filename = fn;
}

void Editor::configChanged()
{
    document()->invalidate();
    viewport()->repaint( FALSE );
}

void Editor::setErrorSelection( int line )
{
    Q3TextParagraph *p = document()->paragAt( line );
    if ( !p )
	return;
    Q3TextCursor c( document() );
    c.setParagraph( p );
    c.setIndex( 0 );
    document()->removeSelection( Error );
    document()->setSelectionStart( Error, c );
    c.gotoLineEnd();
    document()->setSelectionEnd( Error, c );
    hasError = TRUE;
    viewport()->repaint( FALSE );
}

void Editor::setStepSelection( int line )
{
    Q3TextParagraph *p = document()->paragAt( line );
    if ( !p )
	return;
    Q3TextCursor c( document() );
    c.setParagraph( p );
    c.setIndex( 0 );
    document()->removeSelection( Step );
    document()->setSelectionStart( Step, c );
    c.gotoLineEnd();
    document()->setSelectionEnd( Step, c );
    viewport()->repaint( FALSE );
}

void Editor::clearStepSelection()
{
    document()->removeSelection( Step );
    viewport()->repaint( FALSE );
}

void Editor::doChangeInterval()
{
    emit intervalChanged();
    QTextEdit::doChangeInterval();
}

void Editor::commentSelection()
{
    Q3TextParagraph *start = document()->selectionStartCursor( Q3TextDocument::Standard ).paragraph();
    Q3TextParagraph *end = document()->selectionEndCursor( Q3TextDocument::Standard ).paragraph();
    if ( !start || !end )
	start = end = textCursor()->paragraph();
    while ( start ) {
	if ( start == end && textCursor()->index() == 0 )
	    break;
	start->insert( 0, "//" );
	if ( start == end )
	    break;
	start = start->next();
    }
    document()->removeSelection( Q3TextDocument::Standard );
    repaintChanged();
    setModified( TRUE );
}

void Editor::uncommentSelection()
{
    Q3TextParagraph *start = document()->selectionStartCursor( Q3TextDocument::Standard ).paragraph();
    Q3TextParagraph *end = document()->selectionEndCursor( Q3TextDocument::Standard ).paragraph();
    if ( !start || !end )
	start = end = textCursor()->paragraph();
    while ( start ) {
	if ( start == end && textCursor()->index() == 0 )
	    break;
	while ( start->at( 0 )->c == '/' )
	    start->remove( 0, 1 );
	if ( start == end )
	    break;
	start = start->next();
    }
    document()->removeSelection( Q3TextDocument::Standard );
    repaintChanged();
    setModified( TRUE );
}

QPopupMenu *Editor::createPopupMenu( const QPoint &p )
{
    QPopupMenu *menu = QTextEdit::createPopupMenu( p );
    menu->insertSeparator();
    menu->insertItem( tr( "C&omment Code\tAlt+C" ), this, SLOT( commentSelection() ) );
    menu->insertItem( tr( "Unco&mment Code\tAlt+U" ), this, SLOT( uncommentSelection() ) );
    return menu;
}

bool Editor::eventFilter( QObject *o, QEvent *e )
{
    if ( ( e->type() == QEvent::FocusIn || e->type() == QEvent::FocusOut ) &&
	 ( o == this || o == viewport() ) ) {
	accelUncomment->setEnabled( e->type() == QEvent::FocusIn );
	accelComment->setEnabled( e->type() == QEvent::FocusIn );
    }
    return QTextEdit::eventFilter( o, e );
}

void Editor::doKeyboardAction( KeyboardAction action )
{
    if ( !editable )
	return;
    QTextEdit::doKeyboardAction( action );
}

void Editor::keyPressEvent( QKeyEvent *e )
{
    if ( editable ) {
	QTextEdit::keyPressEvent( e );
	return;
    }

    switch ( e->key() ) {
    case Key_Left:
    case Key_Right:
    case Key_Up:
    case Key_Down:
    case Key_Home:
    case Key_End:
    case Key_Prior:
    case Key_Next:
    case Key_Direction_L:
    case Key_Direction_R:
	QTextEdit::keyPressEvent( e );
	break;
    default:
	e->accept();
	break;
    }
}
