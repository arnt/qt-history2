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

#include "actioneditorimpl.h"
#include "formwindow.h"
#include "metadatabase.h"
#include "actionlistview.h"
#include "connectioneditorimpl.h"
#include "mainwindow.h"

#include <qaction.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qtoolbutton.h>
#include <qlistview.h>
#include <qcheckbox.h>
#include <qpushbutton.h>

ActionEditor::ActionEditor( QWidget* parent,  const char* name, WFlags fl )
    : ActionEditorBase( parent, name, fl ), currentAction( 0 ), formWindow( 0 )
{
    listActions->addColumn( tr( "Actions" ) );
    setEnabled( FALSE );
    buttonConnect->setEnabled( FALSE );
}

void ActionEditor::closeEvent( QCloseEvent *e )
{
    emit hidden();
    e->accept();
}

void ActionEditor::currentActionChanged( QListViewItem *i )
{
    buttonConnect->setEnabled( (bool)i );
    if ( !i )
	return;
    currentAction = ( (ActionItem*)i )->action();
    if ( formWindow )
	formWindow->setActiveObject( currentAction );
}

void ActionEditor::deleteAction()
{
    if ( !currentAction )
	return;

    QListViewItemIterator it( listActions );
    while ( it.current() ) {
	if ( ( (ActionItem*)it.current() )->action() == currentAction ) {
	    delete it.current();
	    break;
	}
	++it;
    }

    delete currentAction;
    currentAction = 0;
}

void ActionEditor::newAction()
{
    ActionItem *i = new ActionItem( listActions, FALSE );
    MetaDataBase::addEntry( i->action() );
    i->setText( 0, tr( "Action" ) );
    i->action()->setName( tr( "Action" ) );
    i->action()->setText( tr( "Action" ) );
    listActions->setCurrentItem( i );
    formWindow->actionList().append( i->action() );
}

void ActionEditor::setFormWindow( FormWindow *fw )
{
    listActions->clear();
    formWindow = fw;
    if ( !formWindow ||
	 !formWindow->mainContainer() ||
	 !formWindow->mainContainer()->inherits( "QMainWindow" ) ) {
	setEnabled( FALSE );
    } else {
	setEnabled( TRUE );
	for ( QDesignerAction *a = formWindow->actionList().first(); a; a = formWindow->actionList().next() ) { // ### handle action grpups/childs
	    ActionItem *i = new ActionItem( listActions, a );
	    i->setText( 0, a->name() );
	    i->setPixmap( 0, a->iconSet().pixmap() );
	}
	if ( listActions->firstChild() ) {
	    listActions->setCurrentItem( listActions->firstChild() );
	    listActions->setSelected( listActions->firstChild(), TRUE );
	}
    }
}

void ActionEditor::updateActionName( QAction *a )
{
    QListViewItemIterator it( listActions );
    while ( it.current() ) {
	if ( ( (ActionItem*)it.current() )->action() == a )
	    ( (ActionItem*)it.current() )->setText( 0, a->name() );
	++it;
    }
}

void ActionEditor::updateActionIcon( QAction *a )
{
    QListViewItemIterator it( listActions );
    while ( it.current() ) {
	if ( ( (ActionItem*)it.current() )->action() == a )
	    ( (ActionItem*)it.current() )->setPixmap( 0, a->iconSet().pixmap() );
	++it;
    }
}

void ActionEditor::connectionsClicked()
{
    ConnectionEditor editor( formWindow->mainWindow(), currentAction, formWindow, formWindow );
    editor.exec();
}
