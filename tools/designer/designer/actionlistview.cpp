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

#include "actiondnd.h"
#include "actionlistview.h"
#include <qheader.h>
#include <qcstring.h>

ActionListView::ActionListView( QWidget *parent, const char *name )
    : QListView( parent, name )
{
    setShowSortIndicator( TRUE );
    setResizeMode( LastColumn );
    setRootIsDecorated( TRUE );
    connect( this, SIGNAL( contextMenuRequested( QListViewItem *, const QPoint &, int ) ),
	     this, SLOT( rmbMenu( QListViewItem *, const QPoint & ) ) );
}

ActionItem::ActionItem( QListView *lv, QAction *ac )
    : QListViewItem( lv ), a( 0 ), g( 0 )
{
    g = qt_cast<QDesignerActionGroup*>(ac);
    if ( !g )
	a = qt_cast<QDesignerAction*>(ac);
    setDragEnabled( TRUE );
}

ActionItem::ActionItem( QListViewItem *i, QAction *ac )
    : QListViewItem( i ), a( 0 ), g( 0 )
{
    g = qt_cast<QDesignerActionGroup*>(ac);
    if ( !g )
	a = qt_cast<QDesignerAction*>(ac);
    setDragEnabled( TRUE );
    moveToEnd();
}

void ActionItem::moveToEnd()
{
    QListViewItem *i = this;
    while ( i->nextSibling() )
	i = i->nextSibling();
    if ( i != this )
	moveItem( i );
}

QDragObject *ActionListView::dragObject()
{
    ActionItem *i = (ActionItem*)currentItem();
    if ( !i )
	return 0;
    QStoredDrag *drag = 0;
    if ( i->action() ) {
	drag = new ActionDrag( i->action(), viewport() );
	drag->setPixmap( i->action()->iconSet().pixmap() );
    } else {
	drag = new ActionDrag( i->actionGroup(), viewport() );
	drag->setPixmap( i->actionGroup()->iconSet().pixmap() );
    }
    return drag;
}

void ActionListView::rmbMenu( QListViewItem *i, const QPoint &p )
{
    QPopupMenu *popup = new QPopupMenu( this );
    popup->insertItem( tr( "New &Action" ), 0 );
    popup->insertItem( tr( "New Action &Group" ), 1 );
    popup->insertItem( tr( "New &Dropdown Action Group" ), 2 );
    if ( i ) {
	popup->insertSeparator();
	popup->insertItem( tr( "&Connect Action..." ), 3 );
	popup->insertSeparator();
	popup->insertItem( tr( "Delete Action" ), 4 );
    }
    int res = popup->exec( p );
    if ( res == 0 )
	emit insertAction();
    else if ( res == 1 )
	emit insertActionGroup();
    else if ( res == 2 )
	emit insertDropDownActionGroup();
    else if ( res == 3 )
	emit connectAction();
    else if ( res == 4 )
	emit deleteAction();
}
