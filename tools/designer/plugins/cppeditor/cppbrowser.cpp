#include "cppbrowser.h"
#include <qrichtext_p.h>
#include <qprocess.h>

CppEditorBrowser::CppEditorBrowser( Editor *e )
    : EditorBrowser( e )
{
}

bool CppEditorBrowser::findCursor( const QTextCursor &c, QTextCursor &from, QTextCursor &to )
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

void CppEditorBrowser::showHelp( const QString &w )
{
    QString word( w );
    if ( word[ 0 ] == 'Q' ) {
	if ( word[ (int)word.length() - 1 ] == '&' ||
	     word[ (int)word.length() - 1 ] == '*' )
	    word.remove( word.length() - 1, 1 );
	word = word.lower() + ".html";
	QStringList lst;
	lst << "assistant" << word;
	QProcess proc( lst );
	proc.start();
    }
}
