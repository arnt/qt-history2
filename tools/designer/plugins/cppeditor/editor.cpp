#include "editor.h"
#include "syntaxhighliter_cpp.h"
#include "indent_cpp.h"
#include "parenmatcher.h"
#include "completion.h"
#include "qfile.h"

EditorCompletion *Editor::completion = 0;

Editor::Editor( const QString &fn, QWidget *parent, const char *name )
    : QTextEdit( parent, name )
{
    if ( !fn.isEmpty() )
	load( fn );
    document()->setPreProcessor( new SyntaxHighlighter_CPP );
    document()->setIndent( new Indent_CPP );
    document()->setFormatter( new QTextFormatterBreakInWords );
    setHScrollBarMode( QScrollView::AlwaysOff );
    setVScrollBarMode( QScrollView::AlwaysOn );
    document()->setUseFormatCollection( FALSE );
    parenMatcher = new ParenMatcher;
    connect( this, SIGNAL( cursorPositionChanged( QTextCursor * ) ),
	     this, SLOT( cursorPosChanged( QTextCursor * ) ) );
    if ( !completion )
	completion = new EditorCompletion( this );
    else
	completion->addEditor( this );
}

void Editor::cursorPosChanged( QTextCursor *c )
{
    if ( parenMatcher->match( c ) )
	repaintChanged();
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

void Editor::tabify( QString &s )
{
    Indent_CPP::tabify( s );
}
