/**********************************************************************
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
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

#include "hierarchyview.h"
#include "formwindow.h"
#include "globaldefs.h"
#include "mainwindow.h"
#include "command.h"
#include "widgetfactory.h"
#include "widgetdatabase.h"
#include "pixmapchooser.h"
#include "project.h"
#include "sourceeditor.h"
#include "propertyeditor.h"
#include "editfunctionsimpl.h"
#include "listeditor.h"
#include "actiondnd.h"
#include "actioneditorimpl.h"

#include <qpalette.h>
#include <qobjectlist.h>
#include <qheader.h>
#include <qpopupmenu.h>
#include <qtabwidget.h>
#include <qwizard.h>
#include <qwidgetstack.h>
#include <qtabbar.h>
#include <qfeatures.h>
#include <qapplication.h>
#include <qtimer.h>
#include "../interfaces/languageinterface.h"
#include <qworkspace.h>
#include <qaccel.h>

#include <stdlib.h>

static const char * const folder_xpm[]={
    "16 16 6 1",
    ". c None",
    "b c #ffff00",
    "d c #000000",
    "* c #999999",
    "a c #cccccc",
    "c c #ffffff",
    "................",
    "................",
    "..*****.........",
    ".*ababa*........",
    "*abababa******..",
    "*cccccccccccc*d.",
    "*cbababababab*d.",
    "*cabababababa*d.",
    "*cbababababab*d.",
    "*cabababababa*d.",
    "*cbababababab*d.",
    "*cabababababa*d.",
    "*cbababababab*d.",
    "**************d.",
    ".dddddddddddddd.",
    "................"};

static QPixmap *folderPixmap = 0;
QListViewItem *newItem = 0;

static QPluginManager<ClassBrowserInterface> *classBrowserInterfaceManager = 0;

HierarchyItem::HierarchyItem( Type type, QListViewItem *parent,
			      const QString &txt1, const QString &txt2, const QString &txt3 )
    : QListViewItem( parent, txt1, txt2, txt3 ), typ( type )
{
}

HierarchyItem::HierarchyItem( Type type, QListView *parent,
			      const QString &txt1, const QString &txt2, const QString &txt3 )
    : QListViewItem( parent, txt1, txt2, txt3 ), typ( type )
{
}

void HierarchyItem::paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int align )
{
    QColorGroup g( cg );
    g.setColor( QColorGroup::Base, backgroundColor() );
    g.setColor( QColorGroup::Foreground, Qt::black );
    g.setColor( QColorGroup::Text, Qt::black );
    QString txt = text( 0 );
    if ( rtti() == Function &&
	 MainWindow::self->currProject()->isCpp() &&
	 ( txt == "init()" || txt == "destroy()" ) ) {
	listView()->setUpdatesEnabled( FALSE );
	if ( txt == "init()" )
	    setText( 0, txt + " " + "(Constructor)" );
	else
	    setText( 0, txt + " " + "(Destructor)" );
	QListViewItem::paintCell( p, g, column, width, align );
	setText( 0, txt );
	listView()->setUpdatesEnabled( TRUE );
    } else {
	QListViewItem::paintCell( p, g, column, width, align );
    }
    p->save();
    p->setPen( QPen( cg.dark(), 1 ) );
    if ( column == 0 )
	p->drawLine( 0, 0, 0, height() - 1 );
    if ( listView()->firstChild() != this ) {
	if ( nextSibling() != itemBelow() && itemBelow()->depth() < depth() ) {
	    int d = depth() - itemBelow()->depth();
	    p->drawLine( -listView()->treeStepSize() * d, height() - 1, 0, height() - 1 );
	}
    }
    p->drawLine( 0, height() - 1, width, height() - 1 );
    p->drawLine( width - 1, 0, width - 1, height() );
    p->restore();
}

QColor HierarchyItem::backgroundColor()
{
    updateBackColor();
    return backColor;
}

void HierarchyItem::updateBackColor()
{
    if ( listView()->firstChild() == this ) {
	backColor = *backColor1;
	return;
    }

    QListViewItemIterator it( this );
    --it;
    if ( it.current() ) {
	if ( ( ( HierarchyItem*)it.current() )->backColor == *backColor1 )
	    backColor = *backColor2;
	else
	    backColor = *backColor1;
    } else {
	backColor = *backColor1;
    }
}

void HierarchyItem::setObject( QObject *o )
{
    obj = o;
}

QObject *HierarchyItem::object() const
{
    return obj;
}

void HierarchyItem::okRename( int col )
{
    if ( newItem == this )
	newItem = 0;
    QListViewItem::okRename( col );
}

void HierarchyItem::cancelRename( int col )
{
    if ( newItem == this ) {
	newItem = 0;
	QListViewItem::cancelRename( col );
	delete this;
	return;
    }
    QListViewItem::cancelRename( col );
}




HierarchyList::HierarchyList( QWidget *parent, FormWindow *fw, bool doConnects )
    : QListView( parent ), formWindow( fw )
{
    init_colors();

    setDefaultRenameAction( Accept );
    header()->setMovingEnabled( FALSE );
    header()->setStretchEnabled( TRUE );
    normalMenu = 0;
    tabWidgetMenu = 0;
    addColumn( tr( "Name" ) );
    addColumn( tr( "Class" ) );
    QPalette p( palette() );
    p.setColor( QColorGroup::Base, QColor( *backColor2 ) );
    (void)*selectedBack; // hack
    setPalette( p );
    disconnect( header(), SIGNAL( sectionClicked( int ) ),
		this, SLOT( changeSortColumn( int ) ) );
    setSorting( -1 );
    setHScrollBarMode( AlwaysOff );
    setVScrollBarMode( AlwaysOn );
    if ( doConnects ) {
	connect( this, SIGNAL( clicked( QListViewItem * ) ),
		 this, SLOT( objectClicked( QListViewItem * ) ) );
	connect( this, SIGNAL( returnPressed( QListViewItem * ) ),
		 this, SLOT( objectClicked( QListViewItem * ) ) );
	connect( this, SIGNAL( contextMenuRequested( QListViewItem *, const QPoint&, int ) ),
		 this, SLOT( showRMBMenu( QListViewItem *, const QPoint & ) ) );
    }
    deselect = TRUE;
    setColumnWidthMode( 1, Manual );
}

void HierarchyList::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Key_Shift || e->key() == Key_Control )
	deselect = FALSE;
    else
	deselect = TRUE;
    QListView::keyPressEvent( e );
}

void HierarchyList::keyReleaseEvent( QKeyEvent *e )
{
    deselect = TRUE;
    QListView::keyReleaseEvent( e );
}

void HierarchyList::viewportMousePressEvent( QMouseEvent *e )
{
    if ( e->state() & ShiftButton || e->state() & ControlButton )
	deselect = FALSE;
    else
	deselect = TRUE;
    QListView::viewportMousePressEvent( e );
}

void HierarchyList::viewportMouseReleaseEvent( QMouseEvent *e )
{
    QListView::viewportMouseReleaseEvent( e );
}

void HierarchyList::objectClicked( QListViewItem *i )
{
    if ( !i )
	return;

    QObject *o = findObject( i );
    if ( !o )
	return;
    if ( formWindow == o ) {
	if ( deselect )
	    formWindow->clearSelection( FALSE );
	formWindow->emitShowProperties( formWindow );
	return;
    }
    if ( o->isWidgetType() ) {
	QWidget *w = (QWidget*)o;
	if ( !formWindow->widgets()->find( w ) ) {
	    if ( w->parent() && w->parent()->inherits( "QWidgetStack" ) &&
		 w->parent()->parent() &&
		 ( w->parent()->parent()->inherits( "QTabWidget" ) ||
		   w->parent()->parent()->inherits( "QWizard" ) ) ) {
		if ( w->parent()->parent()->inherits( "QTabWidget" ) )
		    ( (QTabWidget*)w->parent()->parent() )->showPage( w );
		else
		    ( (QDesignerWizard*)w->parent()->parent() )->
			setCurrentPage( ( (QDesignerWizard*)w->parent()->parent() )->
					pageNum( w ) );
		w = (QWidget*)w->parent()->parent();
		formWindow->emitUpdateProperties( formWindow->currentWidget() );
	    } else if ( w->parent() && w->parent()->inherits( "QWidgetStack" ) ) {
		( (QDesignerWidgetStack*)w->parent() )->raiseWidget( w );
		( (QDesignerWidgetStack*)w->parent() )->updateButtons();
	    } else if ( w->inherits( "QMenuBar" ) || w->inherits( "QDockWindow" ) ) {
		formWindow->setActiveObject( w );
	    } else if ( w->inherits( "QPopupMenu" ) ) {
		return; // ### we could try to find our menu bar and change the currentMenu to our index
	    } else {
		return;
	    }
	}
    } else if ( o->inherits( "QAction" ) ) {
	MainWindow::self->actioneditor()->setCurrentAction( (QAction*)o );
	deselect = TRUE;
    }

    if ( deselect )
	formWindow->clearSelection( FALSE );
    if ( o->isWidgetType() && ( (QWidget*)o )->isVisibleTo( formWindow ) )
	formWindow->selectWidget( (QWidget*)o, TRUE );
}

QObject *HierarchyList::findObject( QListViewItem *i )
{
    return ( (HierarchyItem*)i )->object();
}

QListViewItem *HierarchyList::findItem( QObject *o )
{
    QListViewItemIterator it( this );
    while ( it.current() ) {
	if ( ( (HierarchyItem*)it.current() )->object() == o )
	    return it.current();
	++it;
    }
    return 0;
}

QObject *HierarchyList::current() const
{
    if ( currentItem() )
	return ( (HierarchyItem*)currentItem() )->object();
    return 0;
}

void HierarchyList::changeNameOf( QObject *o, const QString &name )
{
    QListViewItem *item = findItem( o );
    if ( !item )
	return;
    item->setText( 0, name );
}


void HierarchyList::changeDatabaseOf( QObject *o, const QString &info )
{
#ifndef QT_NO_SQL
    if ( !formWindow->isDatabaseAware() )
	return;
    QListViewItem *item = findItem( o );
    if ( !item )
	return;
    item->setText( 2, info );
#endif
}

static QWidgetStack *lastWidgetStack = 0;

void HierarchyList::setup()
{
    if ( !formWindow || formWindow->isFake() )
	return;
    clear();
    QWidget *w = formWindow->mainContainer();
#ifndef QT_NO_SQL
    if ( formWindow->isDatabaseAware() ) {
	if ( columns() == 2 ) {
	    addColumn( tr( "Database" ) );
	    header()->resizeSection( 0, 1 );
	    header()->resizeSection( 1, 1 );
	    header()->resizeSection( 2, 1 );
	    header()->adjustHeaderSize();
	}
    } else {
	if ( columns() == 3 ) {
	    removeColumn( 2 );
	}
    }
#endif
    lastWidgetStack = 0;
    if ( w )
	insertObject( w, 0 );
    lastWidgetStack = 0;
}

void HierarchyList::setOpen( QListViewItem *i, bool b )
{
    QListView::setOpen( i, b );
}

void HierarchyList::insertObject( QObject *o, QListViewItem *parent )
{
    bool fakeMainWindow = FALSE;
    if ( o && o->inherits( "QMainWindow" ) ) {
	QObject *cw = ( (QMainWindow*)o )->centralWidget();
	if ( cw ) {
	    o = cw;
	    fakeMainWindow = TRUE;
	}
    }
    QListViewItem *item = 0;
    QString className = WidgetFactory::classNameOf( o );
    if ( o->inherits( "QLayoutWidget" ) ) {
	switch ( WidgetFactory::layoutType( (QWidget*)o ) ) {
	case WidgetFactory::HBox:
	    className = "HBox";
	    break;
	case WidgetFactory::VBox:
	    className = "VBox";
	    break;
	case WidgetFactory::Grid:
	    className = "Grid";
	    break;
	default:
	    break;
	}
    }

    QString dbInfo;
#ifndef QT_NO_SQL
    dbInfo = MetaDataBase::fakeProperty( o, "database" ).toStringList().join(".");
#endif

    QString name = o->name();
    if ( o->parent() && o->parent()->inherits( "QWidgetStack" ) &&
	 o->parent()->parent() ) {
	if ( o->parent()->parent()->inherits( "QTabWidget" ) )
	    name = ( (QTabWidget*)o->parent()->parent() )->tabLabel( (QWidget*)o );
	else if ( o->parent()->parent()->inherits( "QWizard" ) )
	    name = ( (QWizard*)o->parent()->parent() )->title( (QWidget*)o );
	else
	    name = ( (QDesignerWidgetStack*)o->parent() )->pageName();
    }

    if ( fakeMainWindow ) {
	name = o->parent()->name();
	className = "QMainWindow";
    }

    if ( !parent )
	item = new HierarchyItem( HierarchyItem::Widget, this, name, className, dbInfo );
    else
	item = new HierarchyItem( HierarchyItem::Widget, parent, name, className, dbInfo );
    if ( !parent )
	item->setPixmap( 0, PixmapChooser::loadPixmap( "form.xpm", PixmapChooser::Mini ) );
    else if ( o->inherits( "QLayoutWidget") )
	item->setPixmap( 0, PixmapChooser::loadPixmap( "layout.xpm", PixmapChooser::Small ));
    else
	item->setPixmap( 0, WidgetDatabase::iconSet(
		    WidgetDatabase::idFromClassName( WidgetFactory::classNameOf( o ) ) ).
			 pixmap( QIconSet::Small, QIconSet::Normal ) );
    if ( o->inherits( "QAction" ) )
	item->setPixmap( 0, ( (QAction*)o )->iconSet().pixmap() );

    ( (HierarchyItem*)item )->setObject( o );
    const QObjectList *l = o->children();
    if ( l ) {
	QObjectListIt it( *l );
	it.toLast();
	for ( ; it.current(); --it ) {
	    if ( !it.current()->isWidgetType() || ( (QWidget*)it.current() )->isHidden() )
		continue;
	    if (  !formWindow->widgets()->find( (QWidget*)it.current() ) ) {
		if ( it.current()->parent() &&
		     it.current()->inherits( "QWidgetStack" ) ||
		     it.current()->parent()->inherits( "QWidgetStack" ) ) {
		    QObject *obj = it.current();
		    QDesignerTabWidget *tw = 0;
		    QDesignerWizard *dw = 0;
		    if ( it.current()->parent()->inherits( "QTabWidget" ) )
			tw = (QDesignerTabWidget*)it.current()->parent();
		    if ( it.current()->parent()->inherits( "QWizard" ) )
			dw = (QDesignerWizard*)it.current()->parent();
		    QWidgetStack *stack = 0;
		    if ( dw || tw )
			stack = (QWidgetStack*)obj;
		    else
			stack = (QWidgetStack*)obj->parent();
		    if ( lastWidgetStack == stack )
			continue;
		    lastWidgetStack = stack;
		    QObjectList *l2 = stack->queryList( "QWidget", 0, TRUE, FALSE );
		    for ( obj = l2->last(); obj; obj = l2->prev() ) {
			if ( qstrcmp( obj->className(),
				      "QWidgetStackPrivate::Invisible" ) == 0 ||
			     ( tw && !tw->tabBar()->tab( stack->id( (QWidget*)obj ) ) ) ||
			     ( dw && dw->isPageRemoved( (QWidget*)obj ) ) )
			    continue;
			if ( qstrcmp( obj->name(), "designer_wizardstack_button" ) == 0 )
			    continue;
			insertObject( obj, item );
		    }
		    delete l2;
		}
		continue;
	    }
	    insertObject( it.current(), item );
	}
    }

    if ( fakeMainWindow ) {
	QObjectList *l = o->parent()->queryList( "QDesignerToolBar" );
	for ( QObject *obj = l->first(); obj; obj = l->next() )
	    insertObject( obj, item );
	delete l;
	QMenuBar *m = (QMenuBar*)o->parent()->child( 0, "QDesignerMenuBar" );
	if ( m )
	    insertObject( m, item );
    } else if ( o->inherits( "QDesignerToolBar" ) || o->inherits( "QDesignerPopupMenu" ) ) {
	QPtrList<QAction> actions;
	if ( o->inherits( "QDesignerToolBar" ) )
	    actions = ( (QDesignerToolBar*)o )->insertedActions();
	else
	    actions = ( (QDesignerPopupMenu*)o )->insertedActions();
	QPtrListIterator<QAction> it( actions );
	it.toLast();
	while ( it.current() ) {
	    QAction *a = it.current();
	    if ( a->inherits( "QDesignerAction" ) ) {
		QDesignerAction *da = (QDesignerAction*)a;
		if ( da->supportsMenu() )
		    insertObject( da, item );
		else
		    insertObject( da->widget(), item );
	    } else if ( a->inherits( "QDesignerActionGroup" ) ) {
		insertObject( a, item );
	    }
	    --it;
	}
    } else if ( o->inherits( "QDesignerActionGroup" ) && o->children() ) {
	QObjectList *l = (QObjectList*)o->children();
	for ( QObject *obj = l->last(); obj; obj = l->prev() ) {
	    if ( obj->inherits( "QDesignerAction" ) ) {
		QDesignerAction *da = (QDesignerAction*)obj;
		if ( da->supportsMenu() )
		    insertObject( da, item );
		else
		    insertObject( da->widget(), item );
	    } else if ( obj->inherits( "QDesignerActionGroup" ) ) {
		insertObject( obj, item );
	    }
	}
    } else if ( o->inherits( "QMenuBar" ) ) {
	QMenuBar *mb = (QMenuBar*)o;
	for ( int i = mb->count() -1; i >= 0; --i ) {
	    QMenuItem *md = mb->findItem( mb->idAt( i ) );
	    if ( !md || !md->popup() )
		continue;
	    insertObject( md->popup(), item );
	}
    }

    if ( item->firstChild() )
	item->setOpen( TRUE );
}

void HierarchyList::setCurrent( QObject *o )
{
    QListViewItemIterator it( this );
    while ( it.current() ) {
	if ( ( (HierarchyItem*)it.current() )->object() == o ) {
	    blockSignals( TRUE );
	    setCurrentItem( it.current() );
	    ensureItemVisible( it.current() );
	    blockSignals( FALSE );
	    return;
	}
	++it;
    }
}

void HierarchyList::showRMBMenu( QListViewItem *i, const QPoint & p )
{
    if ( !i )
	return;


    QObject *o = findObject( i );
    if ( !o )
	return;

    if ( !o->isWidgetType() ||
	 ( o != formWindow && !formWindow->widgets()->find( (QWidget*)o ) ) )
	return;

    QWidget *w = (QWidget*)o;
    if ( w->isVisibleTo( formWindow ) ) {
	if ( !w->inherits( "QTabWidget" ) && !w->inherits( "QWizard" ) ) {
	    if ( !normalMenu )
		normalMenu = formWindow->mainWindow()->setupNormalHierarchyMenu( this );
	    normalMenu->popup( p );
	} else {
	    if ( !tabWidgetMenu )
		tabWidgetMenu =
		    formWindow->mainWindow()->setupTabWidgetHierarchyMenu(
				  this, SLOT( addTabPage() ),
				  SLOT( removeTabPage() ) );
	    tabWidgetMenu->popup( p );
	}
    }
}

void HierarchyList::addTabPage()
{
    QObject *o = current();
    if ( !o || !o->isWidgetType() )
	return;
    QWidget *w = (QWidget*)o;
    if ( w->inherits( "QTabWidget" ) ) {
	QTabWidget *tw = (QTabWidget*)w;
	AddTabPageCommand *cmd = new AddTabPageCommand( tr( "Add Page to %1" ).
							arg( tw->name() ), formWindow,
							tw, "Tab" );
	formWindow->commandHistory()->addCommand( cmd );
	cmd->execute();
    } else if ( w->inherits( "QWizard" ) ) {
	QWizard *wiz = (QWizard*)formWindow->mainContainer();
	AddWizardPageCommand *cmd = new AddWizardPageCommand( tr( "Add Page to %1" ).
							      arg( wiz->name() ), formWindow,
							      wiz, "Page" );
	formWindow->commandHistory()->addCommand( cmd );
	cmd->execute();
    }
}

void HierarchyList::removeTabPage()
{
    QObject *o = current();
    if ( !o || !o->isWidgetType() )
	return;
    QWidget *w = (QWidget*)o;
    if ( w->inherits( "QTabWidget" ) ) {
	QTabWidget *tw = (QTabWidget*)w;
	if ( tw->currentPage() ) {
	    QDesignerTabWidget *dtw = (QDesignerTabWidget*)tw;
	    DeleteTabPageCommand *cmd =
		new DeleteTabPageCommand( tr( "Delete Page %1 of %2" ).
					  arg( dtw->pageTitle() ).arg( tw->name() ),
					  formWindow, tw, tw->currentPage() );
	    formWindow->commandHistory()->addCommand( cmd );
	    cmd->execute();
	}
    } else if ( w->inherits( "QWizard" ) ) {
	QWizard *wiz = (QWizard*)formWindow->mainContainer();
	if ( wiz->currentPage() ) {
	    QDesignerWizard *dw = (QDesignerWizard*)wiz;
	    DeleteWizardPageCommand *cmd =
		new DeleteWizardPageCommand( tr( "Delete Page %1 of %2" ).
					     arg( dw->pageTitle() ).arg( wiz->name() ),
					     formWindow, wiz,
					     wiz->indexOf( wiz->currentPage() ), TRUE );
	    formWindow->commandHistory()->addCommand( cmd );
	    cmd->execute();
	}
    }
}

// ------------------------------------------------------------

FormDefinitionView::FormDefinitionView( QWidget *parent, FormWindow *fw )
    : HierarchyList( parent, fw, TRUE )
{
    header()->hide();
    removeColumn( 1 );
    connect( this, SIGNAL( itemRenamed( QListViewItem *, int, const QString & ) ),
	     this, SLOT( renamed( QListViewItem * ) ) );
    popupOpen = FALSE;
    QAccel *a = new QAccel( MainWindow::self );
    a->connectItem( a->insertItem( ALT + Key_V ), this, SLOT( editVars() ) );
}

void FormDefinitionView::setup()
{
    if ( popupOpen || !formWindow )
	return;
    if ( !formWindow->project()->isCpp() )
	return;
    if ( !folderPixmap ) {
	folderPixmap = new QPixmap( folder_xpm );
    }

    clear();

    LanguageInterface *lIface = MetaDataBase::languageInterface( formWindow->project()->language() );
    if ( lIface ) {
	QStringList defs = lIface->definitions();
	for ( QStringList::Iterator dit = defs.begin(); dit != defs.end(); ++dit ) {
	    HierarchyItem *itemDef = new HierarchyItem( HierarchyItem::DefinitionParent,
							this, tr( *dit ), QString::null, QString::null );
	    itemDef->setPixmap( 0, *folderPixmap );
	    itemDef->setOpen( TRUE );
	    QStringList entries = lIface->definitionEntries( *dit, formWindow->mainWindow()->designerInterface() );
	    for ( QStringList::Iterator eit = entries.begin(); eit != entries.end(); ++eit ) {
		HierarchyItem *item = new HierarchyItem( HierarchyItem::Definition,
							 itemDef, *eit, QString::null, QString::null );
		item->setRenameEnabled( 0, TRUE );
	    }
	}
	lIface->release();
    }

    refresh( FALSE );
}

void FormDefinitionView::refresh( bool doDelete )
{
    if ( popupOpen )
	return;
    if ( !formWindow )
	return;
    if ( !formWindow->project()->isCpp() )
	return;
    QListViewItem *i = firstChild();
    while ( i ) {
	if ( doDelete && i->rtti() == HierarchyItem::SlotParent ) {
	    QListViewItem* a = i;
	    i = i->nextSibling();
	    delete a;
	    continue;
	}
	i = i->nextSibling();
    }

    i = firstChild();
    while ( i ) {
	if ( doDelete && i->rtti() == HierarchyItem::FunctParent ) {
	    QListViewItem* a = i;
	    i = i->nextSibling();
	    delete a;
	    continue;
	}
	i = i->nextSibling();
    }

    itemFunct = new HierarchyItem( HierarchyItem::FunctParent,
				   this, tr( "Functions" ), QString::null, QString::null );
    itemFunct->moveItem( i );
    itemFunct->setPixmap( 0, *folderPixmap );
    itemFunctPriv = new HierarchyItem( HierarchyItem::FunctPrivate, itemFunct,
				       tr( "private" ), QString::null, QString::null );
    itemFunctProt = new HierarchyItem( HierarchyItem::FunctProtected, itemFunct,
				       tr( "protected" ), QString::null, QString::null );
    itemFunctPubl = new HierarchyItem( HierarchyItem::FunctPublic, itemFunct,
				       tr( "public" ), QString::null, QString::null );

    if ( formWindow->project()->isCpp() ) {
	itemSlots = new HierarchyItem( HierarchyItem::SlotParent,
				       this, tr( "Slots" ), QString::null, QString::null );
	itemSlots->setPixmap( 0, *folderPixmap );
	itemPrivate = new HierarchyItem( HierarchyItem::SlotPrivate, itemSlots, tr( "private" ),
					 QString::null, QString::null );
	itemProtected = new HierarchyItem( HierarchyItem::SlotProtected, itemSlots, tr( "protected" ),
					   QString::null, QString::null );
	itemPublic = new HierarchyItem( HierarchyItem::SlotPublic, itemSlots, tr( "public" ),
					QString::null, QString::null );
    }

    QValueList<MetaDataBase::Function> functionList = MetaDataBase::functionList( formWindow );
    QValueList<MetaDataBase::Function>::Iterator it = --( functionList.end() );
    if ( !functionList.isEmpty() && itemFunct ) {
	for (;;) {
	    QListViewItem *item = 0;
	    if ( (*it).type == "slot" ) {
		if ( (*it).access == "protected" )
		    item = new HierarchyItem( HierarchyItem::Slot, itemProtected, (*it).function,
					      QString::null, QString::null );
		else if ( (*it).access == "private" )
		    item = new HierarchyItem( HierarchyItem::Slot, itemPrivate, (*it).function,
					      QString::null, QString::null );
		else // default is public
		    item = new HierarchyItem( HierarchyItem::Slot, itemPublic, (*it).function,
					      QString::null, QString::null );
	    } else {		
		if ( (*it).access == "protected" )
		    item = new HierarchyItem( HierarchyItem::Function, itemFunctProt, (*it).function,
					      QString::null, QString::null );
		else if ( (*it).access == "private" )
		    item = new HierarchyItem( HierarchyItem::Function, itemFunctPriv, (*it).function,
					      QString::null, QString::null );
		else // default is public
		    item = new HierarchyItem( HierarchyItem::Function, itemFunctPubl, (*it).function,
					      QString::null, QString::null );
	    } 		
	    item->setPixmap( 0, PixmapChooser::loadPixmap( "editslots.xpm" ) );
	    if ( it == functionList.begin() )
		break;
	    --it;
	}
    }

    itemFunct->setOpen( TRUE );
    itemFunctPriv->setOpen( TRUE );
    itemFunctProt->setOpen( TRUE );
    itemFunctPubl->setOpen( TRUE );

    if ( formWindow->project()->isCpp() ) {
        itemPrivate->setOpen( TRUE );
	itemProtected->setOpen( TRUE );
	itemPublic->setOpen( TRUE );
	itemSlots->setOpen( TRUE );
    } else {
	itemFunctProt->setVisible( FALSE );
    }
}


void FormDefinitionView::setCurrent( QWidget * )
{
}

void FormDefinitionView::objectClicked( QListViewItem *i )
{
    if ( !i )
	return;
    if ( (i->rtti() == HierarchyItem::Slot) || (i->rtti() == HierarchyItem::Function) )
	formWindow->mainWindow()->editFunction( i->text( 0 ) );
}

static HierarchyItem::Type getChildType( int type )
{
    switch ( (HierarchyItem::Type)type ) {
    case HierarchyItem::Widget:
	qWarning( "getChildType: Inserting childs dynamically to Widget or SlotParent is not allwowed!" );
	break;
    case HierarchyItem::SlotParent:
    case HierarchyItem::SlotPublic:
    case HierarchyItem::SlotProtected:
    case HierarchyItem::SlotPrivate:
    case HierarchyItem::Slot:
	return HierarchyItem::Slot;
    case HierarchyItem::DefinitionParent:
    case HierarchyItem::Definition:
	return HierarchyItem::Definition;
    case HierarchyItem::Event:
    case HierarchyItem::EventFunction:
	return HierarchyItem::Event;
    case HierarchyItem::FunctParent:
    case HierarchyItem::FunctPublic:
    case HierarchyItem::FunctProtected:
    case HierarchyItem::FunctPrivate:
    case HierarchyItem::Function:
	return HierarchyItem::Function;
    }
    return (HierarchyItem::Type)type;
}

void HierarchyList::insertEntry( QListViewItem *i, const QPixmap &pix, const QString &s )
{
    HierarchyItem *item = new HierarchyItem( getChildType( i->rtti() ), i, s,
					     QString::null, QString::null );
    if ( !pix.isNull() )
	item->setPixmap( 0, pix );
    item->setRenameEnabled( 0, TRUE );
    setCurrentItem( item );
    ensureItemVisible( item );
    qApp->processEvents();
    newItem = item;
    item->startRename( 0 );
}

void FormDefinitionView::contentsMouseDoubleClickEvent( QMouseEvent *e )
{
    QListViewItem *i = itemAt( contentsToViewport( e->pos() ) );
    if ( !i )
	return;
    if ( i->rtti() == HierarchyItem::SlotParent || i->rtti() == HierarchyItem::FunctParent )
	return;
	
    HierarchyItem::Type t = getChildType( i->rtti() );
    if ( (int)t == i->rtti() )
	i = i->parent();
    if ( formWindow->project()->isCpp() &&
	 ( i->rtti() == HierarchyItem::SlotPublic ||
	   i->rtti() == HierarchyItem::SlotProtected ||
	   i->rtti() == HierarchyItem::SlotPrivate ) ) {
	EditFunctions dlg( this, formWindow );
	QString access = "public";
	if ( i->rtti() == HierarchyItem::SlotProtected )
	    access = "protected";
	else if  ( i->rtti() == HierarchyItem::SlotPrivate )
	    access = "private";
	dlg.functionAdd( access, "slot" );
	dlg.exec();
    } else if ( formWindow->project()->isCpp() &&
		( i->rtti() == HierarchyItem::FunctPublic ||
		  i->rtti() == HierarchyItem::FunctProtected ||
		  i->rtti() == HierarchyItem::FunctPrivate ) ) {
	EditFunctions dlg( this, formWindow );
	QString access = "public";
	if ( i->rtti() == HierarchyItem::FunctProtected )
	    access = "protected";
	else if ( i->rtti() == HierarchyItem::FunctPrivate )
	    access = "private";
	dlg.functionAdd( access, "function" );	
	dlg.exec();	

    } else {
	insertEntry( i );
    }
}


void FormDefinitionView::showRMBMenu( QListViewItem *i, const QPoint &pos )
{
    if ( !i )
	return;

    if ( i->rtti() == HierarchyItem::FunctParent && !formWindow->project()->isCpp() )
	return;

    if ( i->rtti() == HierarchyItem::SlotParent ) {
	QPopupMenu menu;
	menu.insertItem( PixmapChooser::loadPixmap( "editslots" ), tr( "Edit..." ) );
	if ( menu.exec( pos ) != -1 ) {
	    EditFunctions dlg( this, formWindow );
	    dlg.functionAdd( "public", "slot" );
	    dlg.exec();
	}
	return;
    }

    if ( i->rtti() == HierarchyItem::FunctParent ) {
	QPopupMenu menu;
	menu.insertItem( PixmapChooser::loadPixmap( "editslots" ), tr( "Edit..." ) );
	if ( menu.exec( pos ) != -1 ) {
	    EditFunctions dlg( this, formWindow );
	    dlg.functionAdd( "public", "function" );
	    dlg.exec();
	}
	return;
    }

    if ( i->rtti() == HierarchyItem::Slot ) {
	QPopupMenu menu;
	const int PROPS = 1;
	const int EDIT = 2;
	const int REMOVE = 3;
	const int NEW_ITEM = 4;
	menu.insertItem( PixmapChooser::loadPixmap( "filenew" ), tr( "New" ), NEW_ITEM );
	if ( formWindow->project()->isCpp() )
	    menu.insertItem( PixmapChooser::loadPixmap( "editslots" ), tr( "Properties..." ), PROPS );
        if ( MetaDataBase::hasEditor( formWindow->project()->language() ) )
	    menu.insertItem( tr( "Goto Implementation" ), EDIT );
	menu.insertSeparator();
	menu.insertItem( PixmapChooser::loadPixmap( "editcut" ), tr( "Delete" ), REMOVE );
	popupOpen = TRUE;
	int res = menu.exec( pos );
	popupOpen = FALSE;
	if ( res == NEW_ITEM ) {
	    if ( formWindow->project()->isCpp() ) {
		EditFunctions dlg( this, formWindow );
		QString access = "public";
		if ( i->parent() && i->parent()->rtti() == HierarchyItem::SlotProtected )
		    access = "protected";
		else if  ( i->parent() && i->parent()->rtti() == HierarchyItem::SlotPrivate )
		    access = "private";
		dlg.functionAdd( access, "slot" );
		dlg.exec();
	    } else {
		insertEntry( i->parent() );
	    }
	} else if ( res == PROPS ) {
	    EditFunctions dlg( this, formWindow );
	    dlg.setCurrentFunction( MetaDataBase::normalizeFunction( i->text( 0 ) ) );
	    dlg.exec();
	} else if ( res == EDIT ) {
	    formWindow->mainWindow()->editFunction( i->text( 0 ) );
	} else if ( res == REMOVE ) {
	    MetaDataBase::removeFunction( formWindow, i->text( 0 ) );
	    EditFunctions::removeFunctionFromCode( i->text( 0 ), formWindow );
	    formWindow->mainWindow()->objectHierarchy()->updateFormDefinitionView();
	    MainWindow::self->functionsChanged();
	}
	return;
    }
    if ( i->rtti() == HierarchyItem::Function ) {
	QPopupMenu menu;
	const int PROPS = 1;
	const int EDIT = 2;
	const int REMOVE = 3;
	const int NEW_ITEM = 4;
	menu.insertItem( PixmapChooser::loadPixmap( "filenew" ), tr( "New" ), NEW_ITEM );
	if ( formWindow->project()->isCpp() )
	    menu.insertItem( PixmapChooser::loadPixmap( "editslots" ), tr( "Properties..." ), PROPS );
	if ( MetaDataBase::hasEditor( formWindow->project()->language() ) )
	    menu.insertItem( tr( "Goto Implementation" ), EDIT );
	menu.insertSeparator();
	menu.insertItem( PixmapChooser::loadPixmap( "editcut" ), tr( "Delete" ), REMOVE );
	popupOpen = TRUE;
	int res = menu.exec( pos );
	popupOpen = FALSE;
	if ( res == NEW_ITEM ) {
	    if ( formWindow->project()->isCpp() ) {
		EditFunctions dlg( this, formWindow );
		QString access = "public";
		if ( i->parent() && i->parent()->rtti() == HierarchyItem::FunctProtected )
		    access = "protected";
		else if  ( i->parent() && i->parent()->rtti() == HierarchyItem::FunctPrivate )
		    access = "private";
		dlg.functionAdd( access, "function" );
		dlg.exec();
	    } else {
		insertEntry( i->parent() );
	    }
	} else if ( res == PROPS ) {
	    EditFunctions dlg( this, formWindow );
	    dlg.setCurrentFunction( MetaDataBase::normalizeFunction( i->text( 0 ) ) );
	    dlg.exec();
	} else if ( res == EDIT ) {
	    formWindow->mainWindow()->editFunction( i->text( 0 ) );
	} else if ( res == REMOVE ) {
	    MetaDataBase::removeFunction( formWindow, i->text( 0 ) );
	    EditFunctions::removeFunctionFromCode( i->text( 0 ), formWindow );
	    formWindow->mainWindow()->objectHierarchy()->updateFormDefinitionView();
	    MainWindow::self->functionsChanged();
	}
	return;
    }

    QPopupMenu menu;
    const int NEW_ITEM = 1;
    const int DEL_ITEM = 2;
    const int EDIT_ITEM = 3;
    menu.insertItem( PixmapChooser::loadPixmap( "filenew" ), tr( "New" ), NEW_ITEM );
    if ( i->rtti() == HierarchyItem::Definition || i->rtti() == HierarchyItem::DefinitionParent ) {
	if ( i->text( 0 ) == "Class Variables" ||
	     i->parent() && i->parent()->text( 0 ) == "Class Variables" )
	    menu.insertItem( tr( "Edit...\tAlt+V" ), EDIT_ITEM );
	else
	    menu.insertItem( tr( "Edit..." ), EDIT_ITEM );
    }
    if ( i->parent() && i->rtti() != HierarchyItem::SlotPublic &&
	 i->rtti() != HierarchyItem::SlotProtected &&
	 i->rtti() != HierarchyItem::SlotPrivate &&
	 i->rtti() != HierarchyItem::FunctPublic &&
	 i->rtti() != HierarchyItem::FunctProtected &&
	 i->rtti() != HierarchyItem::FunctPrivate ) {
	menu.insertSeparator();
	menu.insertItem( PixmapChooser::loadPixmap( "editcut" ), tr( "Delete" ), DEL_ITEM );
    }
    popupOpen = TRUE;
    int res = menu.exec( pos );
    popupOpen = FALSE;
    if ( res == NEW_ITEM ) {
	HierarchyItem::Type t = getChildType( i->rtti() );
	if ( (int)t == i->rtti() )
	    i = i->parent();
	if ( formWindow->project()->isCpp() &&
	     ( i->rtti() == HierarchyItem::SlotPublic ||
	       i->rtti() == HierarchyItem::SlotProtected ||
	       i->rtti() == HierarchyItem::SlotPrivate ) ) {
	    EditFunctions dlg( this, formWindow );
	    QString access = "public";
	    if ( i->rtti() == HierarchyItem::SlotProtected )
		access = "protected";
	    else if  ( i->rtti() == HierarchyItem::SlotPrivate )
		access = "private";
	    dlg.functionAdd( access, "slot" );
	    dlg.exec();
	} else if ( formWindow->project()->isCpp() &&
	    ( i->rtti() == HierarchyItem::FunctPublic ||
	      i->rtti() == HierarchyItem::FunctProtected ||
	      i->rtti() == HierarchyItem::FunctPrivate ) ) {
	    EditFunctions dlg( this, formWindow );
	    QString access = "public";
	    if ( i->rtti() == HierarchyItem::FunctProtected )
		access = "protected";
	    else if ( i->rtti() == HierarchyItem::FunctPrivate )
		access = "private";
	    dlg.functionAdd( access, "function" );
	    dlg.exec();	
	} else {
	    insertEntry( i );
	}
    } else if ( res == DEL_ITEM ) {
	QListViewItem *p = i->parent();
	delete i;
	save( p, 0 );
    } else if ( res == EDIT_ITEM ) {
	LanguageInterface *lIface = MetaDataBase::languageInterface( formWindow->project()->language() );
	if ( !lIface )
	    return;
	if ( i->rtti() == HierarchyItem::Definition )
	    i = i->parent();
	ListEditor dia( this, 0, TRUE );
	dia.setCaption( tr( "Edit %1" ).arg( i->text( 0 ) ) );
	QStringList entries = lIface->definitionEntries( i->text( 0 ), MainWindow::self->designerInterface() );
	dia.setList( entries );
	dia.exec();
	lIface->setDefinitionEntries( i->text( 0 ), dia.items(), MainWindow::self->designerInterface() );
	refresh( TRUE );
	formWindow->commandHistory()->setModified( TRUE );
    }
}

void FormDefinitionView::renamed( QListViewItem *i )
{
    if ( newItem == i )
	newItem = 0;
    if ( !i->parent() )
	return;
    save( i->parent(), i );
}

void FormDefinitionView::save( QListViewItem *p, QListViewItem *i )
{
    if ( i && i->text( 0 ).isEmpty() ) {
	delete i;
	return;
    }
    if ( i && ( (i->rtti() == HierarchyItem::Slot) || (i->rtti() == HierarchyItem::Function) ) ) {
	MetaDataBase::addFunction( formWindow, i->text( 0 ).latin1(), "virtual", p->text( 0 ),
			           i->text( 4 ), formWindow->project()->language(), "void" );
	MainWindow::self->editFunction( i->text( 0 ).left( i->text( 0 ).find( "(" ) ),
					formWindow->project()->language(), TRUE );
	MainWindow::self->objectHierarchy()->updateFormDefinitionView();
	return;
    }

    LanguageInterface *lIface = MetaDataBase::languageInterface( formWindow->project()->language() );
    if ( !lIface )
	return;
    QStringList lst;
    i = p->firstChild();
    while ( i ) {
	lst << i->text( 0 );
	i = i->nextSibling();
    }
    lIface->setDefinitionEntries( p->text( 0 ), lst, formWindow->mainWindow()->designerInterface() );
    lIface->release();
    setup();
    formWindow->commandHistory()->setModified( TRUE );
}

void FormDefinitionView::editVars()
{
    if ( !formWindow )
	return;
    LanguageInterface *lIface = MetaDataBase::languageInterface( formWindow->project()->language() );
    if ( !lIface )
	return;
    ListEditor dia( this, 0, TRUE );
    dia.setCaption( tr( "Edit Class Variables" ) );
    QStringList entries = lIface->definitionEntries( "Class Variables", MainWindow::self->designerInterface() );
    dia.setList( entries );
    dia.exec();
    lIface->setDefinitionEntries( "Class Variables", dia.items(), MainWindow::self->designerInterface() );
    refresh( TRUE );
    formWindow->commandHistory()->setModified( TRUE );
}

// ------------------------------------------------------------

HierarchyView::HierarchyView( QWidget *parent )
    : QTabWidget( parent, 0, WStyle_Customize | WStyle_NormalBorder | WStyle_Title |
		  WStyle_Tool |WStyle_MinMax | WStyle_SysMenu )
{
    formwindow = 0;
    editor = 0;
    setIcon( PixmapChooser::loadPixmap( "logo" ) );
    listview = new HierarchyList( this, formWindow() );
    addTab( listview, tr( "Objects" ) );
    setTabToolTip( listview, tr( "List of all widgets and objects of the current form in hierarchical order" ) );
    fView = new FormDefinitionView( this, formWindow() );
    addTab( fView, tr( "Members" ) );
    setTabToolTip( fView, tr( "List of all members of the current form" ) );

    if ( !classBrowserInterfaceManager ) {
	classBrowserInterfaceManager = new QPluginManager<ClassBrowserInterface>( IID_ClassBrowser, QApplication::libraryPaths(), "/designer" );
    }

    classBrowsers = new QMap<QString, ClassBrowser>();
    QStringList langs = MetaDataBase::languages();
    for ( QStringList::Iterator it = langs.begin(); it != langs.end(); ++it ) {
	QInterfacePtr<ClassBrowserInterface> ciface = 0;
	classBrowserInterfaceManager->queryInterface( *it, &ciface );
	if ( ciface ) {
	    ClassBrowser cb( ciface->createClassBrowser( this ), ciface );
	    addTab( cb.lv, tr( "Class Declarations" ) );
	    setTabToolTip( cb.lv, tr( "List of all classes and its declarations of the current source file" ) );
	    ciface->onClick( this, SLOT( jumpTo( const QString &, const QString &, int ) ) );
	    classBrowsers->insert( *it, cb );
	    setTabEnabled( cb.lv, FALSE );
	}
    }
}

HierarchyView::~HierarchyView()
{
}

void HierarchyView::clear()
{
    listview->clear();
    fView->clear();
    for ( QMap<QString, ClassBrowser>::Iterator it = classBrowsers->begin();
	  it != classBrowsers->end(); ++it ) {
	(*it).iface->clear();
    }
}

void HierarchyView::setFormWindow( FormWindow *fw, QObject *o )
{
    bool fake = fw && qstrcmp( fw->name(), "qt_fakewindow" ) == 0;
    if ( fw == 0 || o == 0 ) {
	listview->clear();
	fView->clear();
	listview->setFormWindow( fw );
	fView->setFormWindow( fw );
	formwindow = 0;
	editor = 0;
    }

    setTabEnabled( listview, TRUE );
    setTabEnabled( fView, fw && fw->project()->isCpp() );

    if ( fw == formwindow ) {
	if ( fw ) {
	    if ( !fake )
		listview->setCurrent( (QWidget*)o );
	    else
		listview->clear();
	    if ( MainWindow::self->qWorkspace()->activeWindow() == fw )
		showPage( listview );
	    else if ( fw->project()->isCpp() )
		showPage( fView );
	    else
		showClasses( fw->formFile()->editor() );
	}
    }

    formwindow = fw;
    if ( !fake ) {
	listview->setFormWindow( fw );
    } else {
	listview->setFormWindow( 0 );
	listview->clear();
    }

    fView->setFormWindow( fw );
    if ( !fake ) {
	listview->setup();
	listview->setCurrent( (QWidget*)o );
    }
    fView->setup();

    for ( QMap<QString, ClassBrowser>::Iterator it = classBrowsers->begin();
	  it != classBrowsers->end(); ++it ) {
	(*it).iface->clear();
	setTabEnabled( (*it).lv, fw && !fw->project()->isCpp() );
    }

    if ( MainWindow::self->qWorkspace()->activeWindow() == fw )
	showPage( listview );
    else if ( fw && fw->project()->isCpp() )
	showPage( fView );
    else if ( fw )
	showClasses( fw->formFile()->editor() );

    editor = 0;
}

void HierarchyView::showClasses( SourceEditor *se )
{
    if ( !se->object() )
	return;

    lastSourceEditor = se;
    QTimer::singleShot( 100, this, SLOT( showClassesTimeout() ) );
}

void HierarchyView::showClassesTimeout()
{
    if ( !lastSourceEditor )
	return;
    SourceEditor *se = (SourceEditor*)lastSourceEditor;
    if ( !se->object() )
	return;
    if ( se->formWindow() && se->formWindow()->project()->isCpp() ) {
	setFormWindow( se->formWindow(), se->formWindow()->currentWidget() );
	MainWindow::self->propertyeditor()->setWidget( se->formWindow()->currentWidget(),
						       se->formWindow() );
	return;
    }

    setTabEnabled( listview, !!se->formWindow() && !se->formWindow()->isFake() );
    setTabEnabled( fView, FALSE );

    formwindow = 0;
    listview->setFormWindow( 0 );
    fView->setFormWindow( 0 );
    listview->clear();
    fView->clear();
    if ( !se->formWindow() )
	MainWindow::self->propertyeditor()->setWidget( 0, 0 );
    editor = se;

    for ( QMap<QString, ClassBrowser>::Iterator it = classBrowsers->begin();
	  it != classBrowsers->end(); ++it ) {
	if ( it.key() == se->project()->language() ) {
	    (*it).iface->update( se->text() );
	    setTabEnabled( (*it).lv, TRUE );
	    showPage( (*it).lv );
	} else {
	    setTabEnabled( (*it).lv, FALSE );
	    (*it).iface->clear();
	}
    }
}

void HierarchyView::updateClassBrowsers()
{
    if ( !editor )
	return;
    for ( QMap<QString, ClassBrowser>::Iterator it = classBrowsers->begin();
	  it != classBrowsers->end(); ++it ) {
	if ( it.key() == editor->project()->language() )
	    (*it).iface->update( editor->text() );
	else
	    (*it).iface->clear();
    }
}

FormWindow *HierarchyView::formWindow() const
{
    return formwindow;
}

void HierarchyView::closeEvent( QCloseEvent *e )
{
    emit hidden();
    e->accept();
}

void HierarchyView::widgetInserted( QWidget * )
{
    listview->setup();
}

void HierarchyView::widgetRemoved( QWidget * )
{
    listview->setup();
}

void HierarchyView::widgetsInserted( const QWidgetList & )
{
    listview->setup();
}

void HierarchyView::widgetsRemoved( const QWidgetList & )
{
    listview->setup();
}

void HierarchyView::namePropertyChanged( QWidget *w, const QVariant & )
{
    QWidget *w2 = w;
    if ( w->inherits( "QMainWindow" ) )
	w2 = ( (QMainWindow*)w )->centralWidget();
    listview->changeNameOf( w2, w->name() );
}


void HierarchyView::databasePropertyChanged( QWidget *w, const QStringList& info )
{
#ifndef QT_NO_SQL
    QString i = info.join( "." );
    listview->changeDatabaseOf( w, i );
#endif
}


void HierarchyView::tabsChanged( QTabWidget * )
{
    listview->setup();
}

void HierarchyView::pagesChanged( QWizard * )
{
    listview->setup();
}

void HierarchyView::rebuild()
{
    listview->setup();
}

void HierarchyView::closed( FormWindow *fw )
{
    if ( fw == formwindow ) {
	listview->clear();
	fView->clear();
    }
}

void HierarchyView::updateFormDefinitionView()
{
    fView->setup();
}

void HierarchyView::jumpTo( const QString &func, const QString &clss, int type )
{
    if ( !editor )
	return;
    if ( type == ClassBrowserInterface::Class )
	editor->setClass( func );
    else
	editor->setFunction( func, clss );
}

HierarchyView::ClassBrowser::ClassBrowser( QListView *l, ClassBrowserInterface *i )
    : lv( l ), iface( i )
{
}

HierarchyView::ClassBrowser::~ClassBrowser()
{
}
