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

#include "editor.h"
#include "parenmatcher.h"
#include <qfile.h>
#include <qrichtext_p.h>
#include "conf.h"
#include <qapplication.h>
#include <qpopupmenu.h>
#include <qaccel.h>

Editor::Editor( const QString &fn, QWidget *parent, const char *name )
    : QTextEdit( parent, name ), hasError( FALSE )
{
    document()->setFormatter( new QTextFormatterBreakInWords );
    if ( !fn.isEmpty() )
	load( fn );
    setHScrollBarMode( QScrollView::AlwaysOff );
    setVScrollBarMode( QScrollView::AlwaysOn );
    document()->setUseFormatCollection( FALSE );
    parenMatcher = new ParenMatcher;
    connect( this, SIGNAL( cursorPositionChanged( QTextCursor * ) ),
	     this, SLOT( cursorPosChanged( QTextCursor * ) ) );
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
}

void Editor::cursorPosChanged( QTextCursor *c )
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
    QTextParag *p = document()->paragAt( line );
    if ( !p )
	return;
    QTextCursor c( document() );
    c.setParag( p );
    c.setIndex( 0 );
    document()->removeSelection( Error );
    document()->setSelectionStart( Error, &c );
    c.gotoLineEnd();
    document()->setSelectionEnd( Error, &c );
    hasError = TRUE;
    viewport()->repaint( FALSE );
}

void Editor::setStepSelection( int line )
{
    QTextParag *p = document()->paragAt( line );
    if ( !p )
	return;
    QTextCursor c( document() );
    c.setParag( p );
    c.setIndex( 0 );
    document()->removeSelection( Step );
    document()->setSelectionStart( Step, &c );
    c.gotoLineEnd();
    document()->setSelectionEnd( Step, &c );
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
    QTextParag *start = document()->selectionStart( QTextDocument::Standard );
    QTextParag *end = document()->selectionEnd( QTextDocument::Standard );
    if ( !start || !end )
	start = end = textCursor()->parag();
    while ( start ) {
	if ( start == end && textCursor()->index() == 0 )
	    break;
	start->insert( 0, "//" );
	if ( start == end )
	    break;
	start = start->next();
    }
    document()->removeSelection( QTextDocument::Standard );
    repaintChanged();
    setModified( TRUE );
}

void Editor::uncommentSelection()
{
    QTextParag *start = document()->selectionStart( QTextDocument::Standard );
    QTextParag *end = document()->selectionEnd( QTextDocument::Standard );
    if ( !start || !end )
	start = end = textCursor()->parag();
    while ( start ) {
	if ( start == end && textCursor()->index() == 0 )
	    break;
	while ( start->at( 0 )->c == '/' )
	    start->remove( 0, 1 );
	if ( start == end )
	    break;
	start = start->next();
    }
    document()->removeSelection( QTextDocument::Standard );
    repaintChanged();
    setModified( TRUE );
}

QPopupMenu *Editor::createPopupMenu()
{
    QPopupMenu *menu = QTextEdit::createPopupMenu();
    menu->insertSeparator();
    menu->insertItem( tr( "C&omment Code\tALT+C" ), this, SLOT( commentSelection() ) );
    menu->insertItem( tr( "Unco&mment Code\tALT+U" ), this, SLOT( uncommentSelection() ) );
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
