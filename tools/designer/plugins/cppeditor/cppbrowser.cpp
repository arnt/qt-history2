#include "cppbrowser.h"
#include <qrichtext_p.h>
#include <qprocess.h>
#include <qmainwindow.h>
#include <qstatusbar.h>
#include <editor.h>

CppEditorBrowser::CppEditorBrowser( Editor *e )
    : EditorBrowser( e )
{
}

void CppEditorBrowser::showHelp( const QString &w )
{
    QString word( w );
    if ( word[ 0 ] == 'Q' ) {
	if ( word[ (int)word.length() - 1 ] == '&' ||
	     word[ (int)word.length() - 1 ] == '*' )
	    word.remove( word.length() - 1, 1 );
	word = word.lower() + ".html";
	QStringList lst;
	lst << "assistant" << "-file" << word;
	QProcess proc( lst );
	proc.start();
	return;
    }

    if ( word.find( '(' ) != -1 ) {
	QString txt = "::" + word.left( word.find( '(' ) );
	QTextDocument *doc = curEditor->document();
	QTextParag *p = doc->firstParag();
	while ( p ) {
	    if ( p->string()->toString().find( txt ) != -1 ) {
		curEditor->setCursorPosition( p->paragId(), 0 );
		return;
	    }
	    p = p->next();
	}
    }

    QWidget *wid = curEditor->topLevelWidget();
    if ( wid->inherits( "QMainWindow" ) )
	( (QMainWindow*)wid )->statusBar()->message( tr( "Nothing available for '%1'" ).arg( w ), 1500 );
}
