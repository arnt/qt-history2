/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "cppbrowser.h"
#include <private/qrichtext_p.h>
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
	Q3TextDocument *doc = curEditor->document();
	Q3TextParagraph *p = doc->firstParagraph();
	while ( p ) {
	    if ( p->string()->toString().find( txt ) != -1 ) {
		curEditor->setCursorPosition( p->paragId(), 0 );
		return;
	    }
	    p = p->next();
	}
    }

    QMainWindow *mw = qt_cast<QMainWindow*>(curEditor->topLevelWidget());
    if ( mw )
	mw->statusBar()->message( tr( "Nothing available for '%1'" ).arg( w ), 1500 );
}
