#include "cppeditor.h"
#include "syntaxhighliter_cpp.h"
#include "indent_cpp.h"
#include "cppcompletion.h"

CppEditor::CppEditor( const QString &fn, QWidget *parent, const char *name )
    : Editor( fn, parent, name )
{
    document()->setPreProcessor( new SyntaxHighlighter_CPP );
    document()->setIndent( new Indent_CPP );
    completion = new CppEditorCompletion( this );
    int i = 0;
    while ( SyntaxHighlighter_CPP::keywords[ i ] != QString::null )
	    completion->addCompletionEntry( SyntaxHighlighter_CPP::keywords[ i++ ], 0 );
}
