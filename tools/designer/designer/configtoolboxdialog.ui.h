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

#include "mainwindow.h"
#include "widgetaction.h"
#include <qlistview.h>

void ConfigToolboxDialog::init()
{
    listViewTools->setSorting( -1 );
    listViewCommon->setSorting( -1 );

    QDict<QListViewItem> groups;
    QAction *a;
    for ( a = MainWindow::self->toolActions.last(); a;
	  a = MainWindow::self->toolActions.prev() ) {
	QString grp = ( (WidgetAction*)a )->group();
	QListViewItem *parent = groups.find( grp );
	if ( !parent ) {
	    parent = new QListViewItem( listViewTools );
	    parent->setText( 0, grp );
	    parent->setOpen( TRUE );
	    groups.insert( grp, parent );
	}
	QListViewItem *i = new QListViewItem( parent );
	i->setText( 0, a->text() );
	i->setPixmap( 0, a->iconSet().pixmap() );
    }
    for ( a = MainWindow::self->commonWidgetsPage.last(); a;
	  a = MainWindow::self->commonWidgetsPage.prev() ) {
	QListViewItem *i = new QListViewItem( listViewCommon );
	i->setText( 0, a->text() );
	i->setPixmap( 0, a->iconSet().pixmap() );
    }

}


void ConfigToolboxDialog::addTool()
{
    QAction *a = 0;
    for ( a = MainWindow::self->toolActions.last(); a;
	  a = MainWindow::self->toolActions.prev() ) {
	if ( a->text() == listViewTools->currentItem()->text( 0 ) )
	    break;
    }
    if ( !a )
	return;
    QListViewItem *i = new QListViewItem( listViewCommon, listViewCommon->lastItem() );
    i->setText( 0, a->text() );
    i->setPixmap( 0, a->iconSet().pixmap() );
    listViewCommon->setCurrentItem( i );
    listViewCommon->ensureItemVisible( i );
    MainWindow::self->commonWidgetsPage.append( a );
}


void ConfigToolboxDialog::removeTool()
{
    QListViewItem *item = listViewCommon->firstChild();
    for ( int i = 0; i < listViewCommon->childCount(); ++i ) {
	if ( item == listViewCommon->currentItem() ) {
	    delete item;
	    MainWindow::self->commonWidgetsPage.remove( i );
	    break;
	}
	item = item->itemBelow();
    }		
}


void ConfigToolboxDialog::moveToolUp()
{
    QListViewItem *item = listViewCommon->firstChild();
    for ( int i = 0; i < listViewCommon->childCount(); ++i ) {
	if ( item == listViewCommon->currentItem() ) {
	    item->itemAbove()->moveItem( item );
	    QAction *a = MainWindow::self->commonWidgetsPage.at( i );
	    MainWindow::self->commonWidgetsPage.take( i );
	    MainWindow::self->commonWidgetsPage.insert( i - 1, a );
	    currentCommonToolChanged( item );
	    break;
	}
	item = item->itemBelow();
    }		
}


void ConfigToolboxDialog::moveToolDown()
{
    QListViewItem *item = listViewCommon->firstChild();
    for ( int i = 0; i < listViewCommon->childCount(); ++i ) {
	if ( item == listViewCommon->currentItem() ) {
	    item->moveItem( item->itemBelow() );
	    QAction *a = MainWindow::self->commonWidgetsPage.at( i );
	    MainWindow::self->commonWidgetsPage.take( i );
	    MainWindow::self->commonWidgetsPage.insert( i + 1, a );
	    currentCommonToolChanged( item );
	    break;
	}
	item = item->itemBelow();
    }		
}


void ConfigToolboxDialog::currentToolChanged( QListViewItem *i )
{
    buttonAdd->setEnabled( i && i->parent() );
}


void ConfigToolboxDialog::currentCommonToolChanged( QListViewItem *i )
{
    buttonUp->setEnabled( i && i->itemAbove() );
    buttonDown->setEnabled( i && i->itemBelow() );
    buttonRemove->setEnabled( i );
}
