/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "actioneditorimpl.h"
#include "formwindow.h"
#include "metadatabase.h"
#include "actionlistview.h"
#include "connectiondialog.h"
#include "mainwindow.h"
#include "hierarchyview.h"
#include "formfile.h"

#include <qaction.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qtoolbutton.h>
#include <qlistview.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qevent.h>

ActionEditor::ActionEditor( QWidget* parent,  const char* name, WFlags fl )
    : ActionEditorBase( parent, name, fl ), currentAction( 0 ), formWindow( 0 ),
    explicitlyClosed(false)
{
    listActions->addColumn( tr( "Actions" ) );
    setEnabled( FALSE );
    buttonConnect->setEnabled( FALSE );

    QPopupMenu *popup = new QPopupMenu( this );
    popup->insertItem( tr( "New &Action" ), this, SLOT( newAction() ) );
    popup->insertItem( tr( "New Action &Group" ), this, SLOT( newActionGroup() ) );
    popup->insertItem( tr( "New &Dropdown Action Group" ), this, SLOT( newDropDownActionGroup() ) );
    buttonNewAction->setPopup( popup );
    buttonNewAction->setPopupDelay( 0 );

    connect( listActions, SIGNAL( insertAction() ), this, SLOT( newAction() ) );
    connect( listActions, SIGNAL( insertActionGroup() ), this, SLOT( newActionGroup() ) );
    connect( listActions, SIGNAL( insertDropDownActionGroup() ), this, SLOT( newDropDownActionGroup() ) );
    connect( listActions, SIGNAL( deleteAction() ), this, SLOT( deleteAction() ) );
    connect( listActions, SIGNAL( connectAction() ), this, SLOT( connectionsClicked() ) );
}

void ActionEditor::closeEvent( QCloseEvent *e )
{
    emit hidden();
    e->accept();
}

void ActionEditor::currentActionChanged( QListViewItem *i )
{
    buttonConnect->setEnabled( i != 0 );
    if ( !i )
	return;
    currentAction = ( (ActionItem*)i )->action();
    if ( !currentAction )
	currentAction = ( (ActionItem*)i )->actionGroup();
    if ( formWindow && currentAction )
	formWindow->setActiveObject( currentAction );
    MainWindow::self->objectHierarchy()->hierarchyList()->setCurrent( currentAction );
}

void ActionEditor::setCurrentAction( QAction *a )
{
    QListViewItemIterator it( listActions );
    while ( it.current() ) {
	if ( ( (ActionItem*)it.current() )->action() == a || ( (ActionItem*)it.current() )->actionGroup() == a ) {
	    listActions->setCurrentItem( it.current() );
	    listActions->ensureItemVisible( it.current() );
	    break;
	}
	++it;
    }
}

QAction *ActionEditor::newActionEx() // FIXME: rename. mmonsen 21112002.
{
    ActionItem *i = new ActionItem( listActions, (bool)FALSE );
    MetaDataBase::addEntry( i->action() );
    QString n = "Action";
    formWindow->unify( i->action(), n, TRUE );
    i->setText( 0, n );
    i->action()->setName( n );
    i->action()->setText( i->action()->name() );
    MetaDataBase::setPropertyChanged( i->action(), "text", TRUE );
    MetaDataBase::setPropertyChanged( i->action(), "name", TRUE );
    formWindow->actionList().append( i->action() );
    if ( formWindow->formFile() )
	formWindow->formFile()->setModified( TRUE );
    return i->action();
}

void ActionEditor::deleteAction()
{
    if ( !currentAction )
	return;

    QListViewItemIterator it( listActions );
    ActionItem *ai = 0;
    while ( it.current() ) {
	ai = (ActionItem*)it.current();
	if ( ai->action() == currentAction || ai->actionGroup() == currentAction ) {
	    removeAction( currentAction );
	    delete currentAction;
	    currentAction = 0;
	    delete it.current();
	    break;
	}
	++it;
    }
    if ( formWindow ) {
	formWindow->setActiveObject( formWindow->mainContainer() );
	if ( formWindow->formFile() )
	    formWindow->formFile()->setModified( TRUE );
    }
}

void ActionEditor::newAction()
{
    ActionItem *actionParent = (ActionItem*)listActions->selectedItem();
    if ( actionParent ) {
	if ( !qt_cast<QActionGroup*>(actionParent->actionGroup()) )
	    actionParent = (ActionItem*)actionParent->parent();
    }

    ActionItem *i = 0;
    if ( actionParent )
	i = new ActionItem( actionParent );
    else
	i = new ActionItem( listActions, (bool)FALSE );
    MetaDataBase::addEntry( i->action() );
    QString n = "Action";
    formWindow->unify( i->action(), n, TRUE );
    i->setText( 0, n );
    i->action()->setName( n );
    i->action()->setText( i->action()->name() );
    if ( actionParent && actionParent->actionGroup() &&
	 actionParent->actionGroup()->usesDropDown() ) {
	i->action()->setToggleAction( TRUE );
	MetaDataBase::setPropertyChanged( i->action(), "toggleAction", TRUE );
    }
    MetaDataBase::setPropertyChanged( i->action(), "text", TRUE );
    MetaDataBase::setPropertyChanged( i->action(), "name", TRUE );
    listActions->setCurrentItem( i );
    if ( !actionParent )
	formWindow->actionList().append( i->action() );
    if ( formWindow->formFile() )
	formWindow->formFile()->setModified( TRUE );
}

void ActionEditor::newActionGroup()
{
    ActionItem *actionParent = (ActionItem*)listActions->selectedItem();
    if ( actionParent ) {
	if ( !qt_cast<QActionGroup*>(actionParent->actionGroup()) )
	    actionParent = (ActionItem*)actionParent->parent();
    }

    ActionItem *i = 0;
    if ( actionParent )
	i = new ActionItem( actionParent, TRUE );
    else
	i = new ActionItem( listActions, TRUE );

    MetaDataBase::addEntry( i->actionGroup() );
    MetaDataBase::setPropertyChanged( i->actionGroup(), "usesDropDown", TRUE );
    QString n = "ActionGroup";
    formWindow->unify( i->action(), n, TRUE );
    i->setText( 0, n );
    i->actionGroup()->setName( n );
    i->actionGroup()->setText( i->actionGroup()->name() );
    MetaDataBase::setPropertyChanged( i->actionGroup(), "text", TRUE );
    MetaDataBase::setPropertyChanged( i->actionGroup(), "name", TRUE );
    listActions->setCurrentItem( i );
    i->setOpen( TRUE );
    if ( !actionParent )
	formWindow->actionList().append( i->actionGroup() );
    if ( formWindow->formFile() )
	formWindow->formFile()->setModified( TRUE );
}

void ActionEditor::newDropDownActionGroup()
{
    newActionGroup();
    ( (ActionItem*)listActions->currentItem() )->actionGroup()->setUsesDropDown( TRUE );
}

void ActionEditor::setFormWindow( FormWindow *fw )
{
    listActions->clear();
    formWindow = fw;
    if ( !formWindow ||
	 !qt_cast<QMainWindow*>(formWindow->mainContainer()) ) {
	setEnabled( FALSE );
    } else {
	setEnabled( TRUE );
	QList<QAction*> actions = formWindow->actionList();
	for(QList<QAction*>::Iterator it = actions.begin(); it != actions.end(); ++it) {
	    QAction *a = (*it);
	    ActionItem *i = 0;
	    if ( qt_cast<QAction*>(a->parent()) )
		continue;
	    i = new ActionItem( listActions, a );
	    i->setText( 0, a->name() );
	    i->setPixmap( 0, a->iconSet().pixmap() );
	    if ( qt_cast<QActionGroup*>(a) ) {
		insertChildActions( i );
	    }
	}
	if ( listActions->firstChild() ) {
	    listActions->setCurrentItem( listActions->firstChild() );
	    listActions->setSelected( listActions->firstChild(), TRUE );
	}
    }
}

void ActionEditor::insertChildActions( ActionItem *i )
{
    if (!i->actionGroup())
	return;
    QObjectList l = i->actionGroup()->children();
    for (int j = 0; j < l.size(); ++j) {
	QObject *o = l.at(j);
	if ( !qt_cast<QAction*>(o) )
	    continue;
	QAction *a = (QAction*)o;
	ActionItem *i2 = new ActionItem( (QListViewItem*)i, a );
	i->setOpen( TRUE );
	i2->setText( 0, a->name() );
	i2->setPixmap( 0, a->iconSet().pixmap() );
	if ( qt_cast<QActionGroup*>(a) )
	    insertChildActions( i2 );
    }
}

void ActionEditor::updateActionName( QAction *a )
{
    QListViewItemIterator it( listActions );
    while ( it.current() ) {
	if ( ( (ActionItem*)it.current() )->action() == a )
	    ( (ActionItem*)it.current() )->setText( 0, a->name() );
	else if ( ( (ActionItem*)it.current() )->actionGroup() == a )
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
	else if ( ( (ActionItem*)it.current() )->actionGroup() == a )
	    ( (ActionItem*)it.current() )->setPixmap( 0, a->iconSet().pixmap() );
	++it;
    }
}

void ActionEditor::connectionsClicked()
{
    ConnectionDialog dlg( formWindow->mainWindow() );
    dlg.setDefault( currentAction, formWindow );
    dlg.addConnection();
    dlg.exec();
}

void ActionEditor::removeAction( QAction *a )
{
    emit removing( a );

    formWindow->actionList().remove( a );
    // Remove all connections
    QList<MetaDataBase::Connection> conns = MetaDataBase::connections( formWindow, a );
    for ( QList<MetaDataBase::Connection>::Iterator it2 = conns.begin(); it2 != conns.end(); ++it2 )
	MetaDataBase::removeConnection( formWindow, (*it2).sender, (*it2).signal,
					(*it2).receiver, (*it2).slot );
    // If it's an actiongroup, do the same for its children
    QActionGroup *ag = qt_cast<QActionGroup *>(a);
    if (ag) {
	QObjectList subActions = ag->queryList( "QAction" );
	for (int i = 0; i < subActions.size(); ++i) {
	    QAction *sa = (QAction*)subActions.at(i);
	    removeAction( sa );
	}
    }
}
