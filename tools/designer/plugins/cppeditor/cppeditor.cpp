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

#include "cppeditor.h"
#include "syntaxhighliter_cpp.h"
#include "indent_cpp.h"
#include "cppcompletion.h"
#include "cppbrowser.h"
#include <parenmatcher.h>
#include <qsettings.h>
#include <qpopupmenu.h>
#include <qinputdialog.h>
#include <designerinterface.h>

CppEditor::CppEditor( const QString &fn, QWidget *parent, const char *name, DesignerInterface *i )
    : Editor( fn, parent, name ), dIface( i )
{
    dIface->addRef();
    document()->setPreProcessor( new SyntaxHighlighter_CPP );
    document()->setIndent( new Indent_CPP );
    completion = new CppEditorCompletion( this );
    browser = new CppEditorBrowser( this );
    int j = 0;
    while ( SyntaxHighlighter_CPP::keywords[ j ] != QString::null )
	    completion->addCompletionEntry( SyntaxHighlighter_CPP::keywords[ j++ ], 0, FALSE );
    configChanged();
}

CppEditor::~CppEditor()
{
    dIface->release();
}

void CppEditor::configChanged()
{
    QMap<QString, ConfigStyle> styles = Config::readStyles( "/Trolltech/CppEditor/" );
    config()->styles = styles;
    ( (SyntaxHighlighter_CPP*)document()->preProcessor() )->updateStyles( config()->styles );
    document()->setTabStops( ( (SyntaxHighlighter_CPP*)document()->preProcessor() )->format( QTextPreProcessor::Standard )->width( 'x' ) * 8 );

    completion->setEnabled( Config::completion( "/Trolltech/CppEditor/" ) );
    parenMatcher->setEnabled( Config::parenMatching( "/Trolltech/CppEditor/" ) );
    if ( Config::wordWrap( "/Trolltech/CppEditor/" ) ) {
	if ( hScrollBarMode() != AlwaysOff ) {
	    document()->setFormatter( new QTextFormatterBreakInWords );
	    setHScrollBarMode( AlwaysOff );
	}
    } else {
	if ( hScrollBarMode() != AlwaysOn ) {
	    QTextFormatterBreakWords *f = new QTextFormatterBreakWords;
	    f->setWrapEnabled( FALSE );
	    document()->setFormatter( f );
	    setHScrollBarMode( AlwaysOn );
	}
    }

    Editor::configChanged();
}

QPopupMenu *CppEditor::createPopupMenu()
{
    QPopupMenu *m = Editor::createPopupMenu();
    m->insertSeparator();
    m->insertItem( tr( "Add Include File (in Declaration)..." ), this, SLOT( addInclDecl() ) );
    m->insertItem( tr( "Add Include File (in Implementation)..." ), this, SLOT( addInclImpl() ) );
    m->insertItem( tr( "Add Forward Declaration..." ), this, SLOT( addForward() ) );
    m->insertItem( tr( "Add Class Variable..." ), this, SLOT( addVar() ) );
    return m;
}

void CppEditor::addInclDecl()
{
    QString s = QInputDialog::getText( tr( "Add Include File (In Declaration)" ),
				       tr( "You should input that in the form <b>&lt;include.h&gt;</b> or <b>\"include.h\"</b>" ) );
    if ( s.isEmpty() )
	return;
    DesignerFormWindow *form = dIface->currentForm();
    QStringList lst = form->declarationIncludes();
    lst << s;
    form->setDeclarationIncludes( lst );
}

void CppEditor::addInclImpl()
{
    QString s = QInputDialog::getText( tr( "Add Include File (In Implementation)" ),
				       tr( "You should input that in the form <b>&lt;include.h&gt;</b> or <b>\"include.h\"</b>" ) );
    if ( s.isEmpty() )
	return;
    DesignerFormWindow *form = dIface->currentForm();
    QStringList lst = form->implementationIncludes();
    lst << s;
    form->setImplementationIncludes( lst );
}

void CppEditor::addForward()
{
    QString s = QInputDialog::getText( tr( "Add Forward Declaration" ),
				       tr( "You should input that in the form <b>ClassName;</b>" ) );
    DesignerFormWindow *form = dIface->currentForm();
    QStringList lst = form->forwardDeclarations();
    lst << s;
    form->setForwardDeclarations( lst );
}

void CppEditor::addVar()
{
    QString s = QInputDialog::getText( tr( "Add Class Variable" ),
				       tr( "You should input that in the form <b>type var;</b>" ) );
    DesignerFormWindow *form = dIface->currentForm();
    QStringList lst = form->variables();
    lst << s;
    form->setVariables( lst );
}
