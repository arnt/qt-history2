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

#include <qvariant.h>  // HP-UX compiler needs this here
#include "workspace.h"
#include "formwindow.h"
#include "mainwindow.h"
#include "pixmapchooser.h"
#include "globaldefs.h"
#include "command.h"
#include "project.h"
#include "pixmapcollection.h"
#include "sourcefile.h"
#include "sourceeditor.h"

#include <qheader.h>
#include <qdragobject.h>
#include <qfileinfo.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qpen.h>
#include <qobjectlist.h>
#include <qworkspace.h>
#include <qpopupmenu.h>
#include <qtextstream.h>
#include "qcompletionedit.h"

static const char * folder_xpm[]={
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

static const char* file_xpm[]={
    "16 16 5 1",
    ". c #7f7f7f",
    "# c None",
    "c c #000000",
    "b c #bfbfbf",
    "a c #ffffff",
    "################",
    "..........######",
    ".aaaaaaaab.#####",
    ".aaaaaaaaba.####",
    ".aaaaaaaacccc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".bbbbbbbbbbbc###",
    "ccccccccccccc###"};

static QPixmap *folderPixmap = 0;
static QPixmap *filePixmap = 0;
static QPixmap* formPixmap = 0;

WorkspaceItem::WorkspaceItem( QListView *parent, Project* p )
    : QListViewItem( parent )
{
    init();
    project = p;
    t = ProjectType;
    setPixmap( 0, *folderPixmap );
    setExpandable( FALSE );
}

WorkspaceItem::WorkspaceItem( QListViewItem *parent, SourceFile* sf )
    : QListViewItem( parent )
{
    init();
    sourceFile = sf;
    t = SourceFileType;
    setPixmap( 0, *filePixmap );
}

WorkspaceItem::WorkspaceItem( QListViewItem *parent, FormFile* ff )
    : QListViewItem( parent )
{
    init();
    formFile = ff;
    t = FormFileType;
    setPixmap( 0, *formPixmap );
    QObject::connect( ff, SIGNAL( somethingChanged(FormFile*) ), listView(), SLOT( update(FormFile*) ) );
}


void WorkspaceItem::init()
{
    project = 0;
    sourceFile = 0;
    formFile = 0;
}

void WorkspaceItem::paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int align )
{
    QColorGroup g( cg );
    g.setColor( QColorGroup::Base, backgroundColor() );
    g.setColor( QColorGroup::Foreground, Qt::black );
    g.setColor( QColorGroup::Text, Qt::black );
    p->save();

    if ( isModified() ) {
	QFont f = p->font();
	f.setBold( TRUE );
	p->setFont( f );
    }

    QListViewItem::paintCell( p, g, column, width, align );
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

QString WorkspaceItem::text( int column ) const
{
    if ( column != 0 )
	return QListViewItem::text( column );
    switch( t ) {
    case ProjectType:
	if ( project->isDummy() )
	    return Project::tr("<No Project>" );
	return project->fileName();
    case FormFileType:
	return formFile->formName();
    case FormUiType:
	return "form ui"; //###
    case FormSourceType:
	return "form source"; //###
    case SourceFileType:
	return sourceFile->fileName();
    }

    return QString::null; // shut up compiler
}

void WorkspaceItem::fillCompletionList( QStringList& completion )
{
    switch( t ) {
    case ProjectType:
	break;
    case FormFileType:
	break; //###
    case FormUiType:
	break;//###
    case FormSourceType:
	break;	//###
    case SourceFileType:
	completion += sourceFile->fileName();
    }
}

bool WorkspaceItem::isModified() const
{
    switch( t ) {
    case ProjectType:
	return project->isModified();
    case FormFileType:
	return formFile->isModified();
    case FormUiType:
	return FALSE; //###
    case FormSourceType:
	return FALSE;  //###
    case SourceFileType:
	return sourceFile->isModified();
    }
    return FALSE; // shut up compiler
}

QString WorkspaceItem::key( int column, bool ) const
{
    QString key = text( column );
    if ( t == FormFileType )
	key.prepend( "0" );
    else 
	key.prepend( "a" );
    return key;
}

QColor WorkspaceItem::backgroundColor()
{
    updateBackColor();
    return backColor;
}

void WorkspaceItem::updateBackColor()
{
    if ( listView()->firstChild() == this ) {
	backColor = *backColor1;
	return;
    }

    QListViewItemIterator it( this );
    --it;
    if ( it.current() ) {
	if ( ( ( WorkspaceItem*)it.current() )->backColor == *backColor1 )
	    backColor = *backColor2;
	else
	    backColor = *backColor1;
    } else {
	backColor == *backColor1;
    }
}

Workspace::Workspace( QWidget *parent, MainWindow *mw )
    : QListView( parent, 0, WStyle_Customize | WStyle_NormalBorder | WStyle_Title |
		 WStyle_Tool | WStyle_MinMax | WStyle_SysMenu ), mainWindow( mw ),
	project( 0 ), completionDirty( FALSE )
{
    init_colors();

    blockNewForms = FALSE;
    bufferEdit = 0;
//     header()->setMovingEnabled( FALSE );
    header()->setStretchEnabled( TRUE );
    header()->hide();
    setSorting( 0 );
    setResizePolicy( QScrollView::Manual );
    setIcon( PixmapChooser::loadPixmap( "logo" ) );
    QPalette p( palette() );
    p.setColor( QColorGroup::Base, QColor( *backColor2 ) );
    (void)*selectedBack; // hack
    setPalette( p );
    addColumn( tr( "Files" ) );
//     addColumn( "" );
    setAllColumnsShowFocus( TRUE );
    connect( this, SIGNAL( mouseButtonClicked( int, QListViewItem *, const QPoint &, int ) ),
	     this, SLOT( itemClicked( int, QListViewItem * ) ) ),
    connect( this, SIGNAL( contextMenuRequested( QListViewItem *, const QPoint &, int ) ),
	     this, SLOT( rmbClicked( QListViewItem * ) ) ),
    setHScrollBarMode( AlwaysOff );
    setVScrollBarMode( AlwaysOn );
    viewport()->setAcceptDrops( TRUE );
    setAcceptDrops( TRUE );
    setColumnWidthMode( 1, Manual );
//     setRootIsDecorated( TRUE );

    if ( !folderPixmap ) {
	folderPixmap = new QPixmap( folder_xpm );
	filePixmap = new QPixmap( file_xpm );
	formPixmap = new QPixmap( PixmapChooser::loadPixmap( "form.xpm", PixmapChooser::Mini ) );
    }
}


void Workspace::projectDestroyed( QObject* o )
{
    if ( o == project ) {
	project = 0;
	clear();
    }
}

void Workspace::setCurrentProject( Project *pro )
{
    if ( project == pro )
	return;
    if ( project ) {
	disconnect( project, SIGNAL( sourceFileAdded(SourceFile*) ), this, SLOT( sourceFileAdded(SourceFile*) ) );
	disconnect( project, SIGNAL( sourceFileRemoved(SourceFile*) ), this, SLOT( sourceFileRemoved(SourceFile*) ) );
	disconnect( project, SIGNAL( formFileAdded(FormFile*) ), this, SLOT( formFileAdded(FormFile*) ) );
	disconnect( project, SIGNAL( formFileRemoved(FormFile*) ), this, SLOT( formFileRemoved(FormFile*) ) );
	disconnect( project, SIGNAL( projectModified() ), this, SLOT( update() ) );
    }
    project = pro;
    connect( project, SIGNAL( sourceFileAdded(SourceFile*) ), this, SLOT( sourceFileAdded(SourceFile*) ) );
    connect( project, SIGNAL( sourceFileRemoved(SourceFile*) ), this, SLOT( sourceFileRemoved(SourceFile*) ) );
    connect( project, SIGNAL( formFileAdded(FormFile*) ), this, SLOT( formFileAdded(FormFile*) ) );
    connect( project, SIGNAL( formFileRemoved(FormFile*) ), this, SLOT( formFileRemoved(FormFile*) ) );
    connect( project, SIGNAL( destroyed(QObject*) ), this, SLOT( projectDestroyed(QObject*) ) );
    connect( project, SIGNAL( projectModified() ), this, SLOT( update() ) );
    clear();

    if ( bufferEdit )
	bufferEdit->clear()
;
    LanguageInterface *iface = MetaDataBase::languageInterface( pro->language() );
    QString extension = "xx";
    if ( iface )
	extension = iface->formCodeExtension();

    projectItem = new WorkspaceItem( this, project );



    projectItem->setOpen( TRUE );

    for ( QPtrListIterator<SourceFile> sources = project->sourceFiles();
	  sources.current(); ++sources ) {
	SourceFile* f = sources.current();
	(void) new WorkspaceItem( projectItem, f );
    }

    for ( QPtrListIterator<FormFile> forms = project->formFiles();
	  forms.current(); ++forms ) {
	FormFile* f = forms.current();
	(void) new WorkspaceItem( projectItem, f );
    }

    completionDirty = TRUE;
}



void Workspace::sourceFileAdded( SourceFile* sf )
{
    (void) new WorkspaceItem( projectItem, sf );
}

void Workspace::sourceFileRemoved( SourceFile* sf )
{
    delete findItem( sf );
}

void Workspace::formFileAdded( FormFile* ff )
{
    (void) new WorkspaceItem( projectItem, ff );
}

void Workspace::formFileRemoved( FormFile* ff )
{
    delete findItem( ff );
}


void Workspace::update()
{
    qDebug("Workspace::update ");
    completionDirty = TRUE;
    // #### rebuild completion list
    triggerUpdate();
}

void Workspace::update( FormFile* ff )
{
    qDebug("Workspace::update form file");
    QListViewItem* i = findItem( ff );
    if ( i )
	i->repaint();
}


void Workspace::activeFormChanged( FormWindow *fw )
{
    WorkspaceItem *i = findItem( fw->formFile() );
    if ( i ) {
	setCurrentItem( i );
	setSelected( i, TRUE );
    }
}

void Workspace::activeEditorChanged( SourceEditor *se )
{
    if ( !se->object() )
	return;
    if ( se->formWindow() ) {
	activeFormChanged( se->formWindow() );
	return;
    }
    WorkspaceItem *i = findItem( se->sourceFile() );
    if ( i ) {
	setCurrentItem( i );
	setSelected( i, TRUE );
    }
}

WorkspaceItem *Workspace::findItem( FormFile* ff)
{
    QListViewItemIterator it( this );
    for ( ; it.current(); ++it ) {
	if ( ( (WorkspaceItem*)it.current() )->formFile == ff )
	    return (WorkspaceItem*)it.current();
    }
    return 0;
}

WorkspaceItem *Workspace::findItem( SourceFile *sf )
{
    QListViewItemIterator it( this );
    for ( ; it.current(); ++it ) {
	if ( ( (WorkspaceItem*)it.current() )->sourceFile == sf )
	    return (WorkspaceItem*)it.current();
    }
    return 0;
}

void Workspace::closeEvent( QCloseEvent *e )
{
    e->accept();
}

void Workspace::itemClicked( int button, QListViewItem *i )
{
    if ( !i || button != LeftButton )
	return;

    WorkspaceItem* wi = (WorkspaceItem*)i;
    if ( wi->type() == WorkspaceItem::SourceFileType )
	mainWindow->editSource( wi->sourceFile );
    switch( wi->type() ) {
    case WorkspaceItem::ProjectType:
	break; // ### TODO
    case WorkspaceItem::FormFileType:
	wi->formFile->showFormWindow();
	break;
    case WorkspaceItem::FormUiType:
	break; // ###
    case WorkspaceItem::FormSourceType:
	break; // ###
    case WorkspaceItem::SourceFileType:
	mainWindow->editSource( wi->sourceFile );
	break;
    }
}

void Workspace::bufferChosen( const QString &buffer )
{
    if ( bufferEdit )
	bufferEdit->setText( "" );
    QListViewItemIterator it( this );
    QListViewItem *res = 0;
    QString extension = "xx";
    LanguageInterface *iface = MetaDataBase::languageInterface( project->language() );
    if ( iface )
	extension = iface->formCodeExtension();
    bool formCode = buffer.right( extension.length() + 2 ) == QString( "ui" + extension );
    while ( it.current() ) {
	if ( !formCode &&
	     ( it.current()->text( 0 ) == buffer || it.current()->text( 1 ) == buffer ) ||
	     formCode && ( it.current()->text( 1 ) + extension ) == buffer ) {
	    res = it.current();
	    break;
	}
	++it;
    }

    if ( res ) {
	setCurrentItem( res );
	itemClicked( LeftButton, res );
	if ( formCode )
	    MainWindow::self->editSource();
    }
}

void Workspace::contentsDropEvent( QDropEvent *e )
{
    if ( !QUriDrag::canDecode( e ) ) {
	e->ignore();
    } else {
	QStringList files;
	QUriDrag::decodeLocalFiles( e, files );
	if ( !files.isEmpty() ) {
	    for ( QStringList::Iterator it = files.begin(); it != files.end(); ++it ) {
		QString fn = *it;
		mainWindow->fileOpen( "", "", fn );
	    }
	}
    }
}

void Workspace::contentsDragEnterEvent( QDragEnterEvent *e )
{
    if ( !QUriDrag::canDecode( e ) )
	e->ignore();
    else
	e->accept();
}

void Workspace::contentsDragMoveEvent( QDragMoveEvent *e )
{
    if ( !QUriDrag::canDecode( e ) )
	e->ignore();
    else
	e->accept();
}

void Workspace::rmbClicked( QListViewItem *i )
{
    if ( !i )
	return;
    QPopupMenu menu( this );
    WorkspaceItem* wi = (WorkspaceItem*)i;
    if ( wi->type() == WorkspaceItem::SourceFileType ) {
 	const int OPEN_SOURCE = menu.insertItem( tr( "&Open source file..." ) );
 	int REMOVE_SOURCE = -2;
	menu.insertSeparator();
	REMOVE_SOURCE = menu.insertItem( tr( "&Remove source file from project" ) );
	int id = menu.exec( QCursor::pos() );
	if ( id == -1 )
	    return;
	if ( id == REMOVE_SOURCE ) {
	    project->removeSourceFile( wi->sourceFile );
	} else if ( id == OPEN_SOURCE ) {
	    itemClicked( LeftButton, i );
	}
    }


//     if ( i->rtti() == WorkspaceItem::Form || i->text( 0 ) == tr( "Forms" ) ) {
// 	QPopupMenu menu( this );

// 	const int OPEN_FORM = menu.insertItem( tr( "&Open form" ) );
// 	int REMOVE_FORM = -2;
// 	if ( i->rtti() == WorkspaceItem::Form ) {
// 	    menu.insertSeparator();
// 	    REMOVE_FORM = menu.insertItem( tr( "&Remove form from project" ) );
// 	}
// 	int id = menu.exec( QCursor::pos() );

// 	if ( id == -1 )
// 	    return;

// 	if ( id == REMOVE_FORM ) {
// 	    removeFormFromProject( i );
// 	} else if ( id == OPEN_FORM ) {
// 	    itemClicked( LeftButton, i );
// 	}
//     } else if ( i->rtti() == WorkspaceItem::Source || i->text( 0 ) == tr("Source Files") ) {
// 	QPopupMenu menu( this );

// 	const int OPEN_SOURCE = menu.insertItem( tr( "&Open source file..." ) );
// 	int REMOVE_SOURCE = -2;
// 	if ( i->rtti() == WorkspaceItem::Source ) {
// 	    menu.insertSeparator();
// 	    REMOVE_SOURCE = menu.insertItem( tr( "&Remove source file from project" ) );
// 	}
// 	int id = menu.exec( QCursor::pos() );

// 	if ( id == -1 )
// 	    return;

// 	if ( id == REMOVE_SOURCE ) {
// 	    removeSourceFromProject( i );
// 	} else if ( id == OPEN_SOURCE ) {
// 	    itemClicked( LeftButton, i );
// 	}
//     }
}

// void Workspace::formNameChanged( FormWindow *fw )
// {
//     QListViewItemIterator it( this );
//     while ( it.current() ) {
// 	if ( it.current()->rtti() == WorkspaceItem::Form ) {
// 	    WorkspaceItem *i = (WorkspaceItem*)it.current();
// 	    if ( i->formWindow() == fw ) {
// 		bufferEdit->removeCompletionEntry( i->text( 0 ) );
// 		i->setText( 0, fw->name() );
// 		bufferEdit->addCompletionEntry( i->text( 0 ) );
// 		break;
// 	    }
// 	}
// 	++it;
//     }
// }

bool Workspace::eventFilter( QObject *o, QEvent * e )
{
    if ( o ==bufferEdit )
	updateBufferEdit();
    return QListView::eventFilter( o, e );
}

void Workspace::setBufferEdit( QCompletionEdit *edit )
{
    bufferEdit = edit;
    connect( bufferEdit, SIGNAL( chosen( const QString & ) ),
	     this, SLOT( bufferChosen( const QString & ) ) );
    bufferEdit->installEventFilter( this );
}

void Workspace::updateBufferEdit()
{
    if ( !bufferEdit || !completionDirty )
	return;
    completionDirty = FALSE;
    QStringList completion;
     QListViewItemIterator it( this );
     while ( it.current() ) {
	 ( (WorkspaceItem*)it.current())->fillCompletionList( completion );
	 ++it;
     }
     completion.sort();
     bufferEdit->setCompletionList( completion );
}

