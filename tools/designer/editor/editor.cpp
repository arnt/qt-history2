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
#include "completion.h"
#include <qfile.h>
#include <qrichtext_p.h>
#include "conf.h"
#include <qapplication.h>

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
    document()->setSelectionColor( Error, red );
    document()->setSelectionColor( Step, yellow );
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

void Editor::doChangeInterval()
{
    emit intervalChanged();
    QTextEdit::doChangeInterval();
}
