#include <qapplication.h>
#include <qfile.h>
#include "qtextedit.h"
#include "qtexteditintern_h.cpp"
#include "qcppsyntaxhighlighter.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );

    QString fn = "qtextedit.cpp";
    if ( argc > 1 )
	fn = argv[ 1 ];
    if ( !QFile::exists( fn ) )
	fn = "qtextedit.cpp";
#if 1 // change this to #if 0 if you want to have a C++ editor (syntax-highlighte, etc.)
    QTextEditDocument *d = new QTextEditDocument( fn, FALSE );
    QTextEdit ed( 0, d );
#else
    QTextEditDocument *d = new QTextEditDocument( fn, TRUE );
    d->setSyntaxHighlighter( new QCppSyntaxHighlighter( d ) );
    d->setIndent( new QCppIndent( d ) );
    d->setParenCheckingEnabled( TRUE );
    d->setCompletionEnabled( TRUE );
    QTextEdit ed( 0, d );
    d->setFormatter( new QTextEditFormatterBreakInWords( d ) );
#endif

    a.setMainWidget( &ed );
    ed.resize( 600, 800 );
    ed.show();

    return a.exec();
}

