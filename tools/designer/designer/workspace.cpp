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
#include "propertyeditor.h"

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
    "................"
};

static const char * const file_xpm[]={
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
    "ccccccccccccc###"
};

static char * const object_xpm[] = {
    "16 16 35 1",
    "       c None",
    ".      c #004000",
    "+      c #FFFFFF",
    "@      c #000000",
    "#      c #DCDCDC",
    "$      c #585858",
    "%      c #A0A0A0",
    "&      c #FFDCA8",
    "*      c #808080",
    "=      c #FFC0C0",
    "-      c #C3C3C3",
    ";      c #004040",
    ">      c #C0C0FF",
    ",      c #404000",
    "'      c #00C000",
    ")      c #008000",
    "!      c #8080FF",
    "~      c #C05800",
    "{      c #004000",
    "]      c #303030",
    "^      c #008080",
    "/      c #808000",
    "(      c #800000",
    "_      c #FF0000",
    ":      c #C0C000",
    "<      c #C0FFC0",
    "[      c #FFFFC0",
    "}      c #800080",
    "|      c #00C0C0",
    "1      c #FF8000",
    "2      c #C00000",
    "3      c #FFA858",
    "4      c #C000C0",
    "5      c #0000FF",
    "6      c #FF00FF",
    "    %%%%***,    ",
    "    %++++++%*   ",
    "    %++++++%#*  ",
    "    %+>>%++-#+* ",
    "    %>>>!!+-%$$]",
    "    %!^!^;+++#-,",
    "    #!^$;;++###$",
    "  ===$$;;5+++&#$",
    "  ~13$]$;$/&+##$",
    "  ~64}-|<')#&&&$",
    "  ~~(('')${+###$",
    "   _2]'/)${#&=&$",
    "    %+')){{###-$",
    "    *&+&,&#&&=&$",
    "    %#+##&#-=:=$",
    "    ,****$/$*$*,"
};


static QPixmap *folderPixmap = 0;
static QPixmap *filePixmap = 0;
static QPixmap *formPixmap = 0;
static QPixmap *objectPixmap = 0;

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

WorkspaceItem::WorkspaceItem( QListViewItem *parent, QObject *o, Project *p )
    : QListViewItem( parent )
{
    init();
    object = o;
    project = p;
    t = ObjectType;
    setPixmap( 0, *objectPixmap );
    QObject::connect( p->fakeFormFor( o )->formFile(), SIGNAL( somethingChanged(FormFile*) ),
		      listView(), SLOT( update() ) );
}

WorkspaceItem::WorkspaceItem( QListViewItem *parent, FormFile* ff, Type type )
    : QListViewItem( parent )
{
    init();
    formFile = ff;
    t = type;
    if ( type ==  FormFileType ) {
	setPixmap( 0, *formPixmap );
	QObject::connect( ff, SIGNAL( somethingChanged(FormFile*) ), listView(), SLOT( update(FormFile*) ) );
	if ( formFile->supportsCodeFile() )
	    (void) new WorkspaceItem( this, formFile, FormSourceType );
    } else if ( type == FormSourceType ) {
	setPixmap( 0, *filePixmap );
    }
}


void WorkspaceItem::init()
{
    autoOpen = FALSE;
    useOddColor = FALSE;
    project = 0;
    sourceFile = 0;
    formFile = 0;
}

void WorkspaceItem::paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int align )
{
    QColorGroup g( cg );
    g.setColor( QColorGroup::Base, backgroundColor() );
    g.setColor( QColorGroup::Foreground, Qt::black );

    if ( type() == FormSourceType && !formFile->hasFormCode() ) {
	g.setColor( QColorGroup::Text, listView()->palette().disabled().color( QColorGroup::Text) );
	g.setColor( QColorGroup::HighlightedText, listView()->palette().disabled().color( QColorGroup::Text) );
    } else {
	g.setColor( QColorGroup::Text, Qt::black );
    }
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
	else if ( MainWindow::self->singleProjectMode() )
	    return project->projectName();
	return project->makeRelative( project->fileName() );
    case FormFileType:
	return formFile->formName() + ": " + formFile->fileName();
    case FormSourceType:
	return formFile->codeFile();
    case SourceFileType:
	return sourceFile->fileName();
    case ObjectType:
	return object->name();
    }

    return QString::null; // shut up compiler
}

void WorkspaceItem::fillCompletionList( QStringList& completion )
{
    switch( t ) {
    case ProjectType:
	break;
    case FormFileType:
	completion += formFile->formName();
	completion += formFile->fileName();
	break;
    case FormSourceType:
	completion += formFile->codeFile();
	break;
    case SourceFileType:
	completion += sourceFile->fileName();
	break;
    case ObjectType:
	completion += object->name();
    }
}

bool WorkspaceItem::checkCompletion( const QString& completion )
{
    switch( t ) {
    case ProjectType:
	break;
    case FormFileType:
	return  completion == formFile->formName()
		 || completion == formFile->fileName();
    case FormSourceType:
	return completion == formFile->codeFile();
    case SourceFileType:
	return completion == sourceFile->fileName();
    case ObjectType:
	return completion == object->name();
    }
    return FALSE;
}


bool WorkspaceItem::isModified() const
{
    switch( t ) {
    case ProjectType:
	return project->isModified();
    case FormFileType:
	return formFile->isModified( FormFile::WFormWindow );
    case FormSourceType:
	return formFile->isModified( FormFile::WFormCode );
    case SourceFileType:
	return sourceFile->isModified();
    case ObjectType:
	return project->fakeFormFor( object )->formFile()->isModified();
	break;
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
    bool b = useOddColor;
    if ( t == FormSourceType && parent() )
	b = ( ( WorkspaceItem*)parent() )->useOddColor;
    return b ? *backColor2 : *backColor1;
}


void WorkspaceItem::setOpen( bool b )
{
    QListViewItem::setOpen( b );
    autoOpen = FALSE;
}

void WorkspaceItem::setAutoOpen( bool b )
{
    QListViewItem::setOpen( b );
    autoOpen = b;
}

Workspace::Workspace( QWidget *parent, MainWindow *mw )
    : QListView( parent, 0, WStyle_Customize | WStyle_NormalBorder | WStyle_Title |
		 WStyle_Tool | WStyle_MinMax | WStyle_SysMenu ), mainWindow( mw ),
	project( 0 ), completionDirty( FALSE )
{
    init_colors();

    setDefaultRenameAction( Accept );
    blockNewForms = FALSE;
    bufferEdit = 0;
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
    setAllColumnsShowFocus( TRUE );
    connect( this, SIGNAL( mouseButtonClicked( int, QListViewItem *, const QPoint &, int ) ),
	     this, SLOT( itemClicked( int, QListViewItem *, const QPoint& ) ) ),
    connect( this, SIGNAL( doubleClicked( QListViewItem * ) ),
	     this, SLOT( itemDoubleClicked( QListViewItem * ) ) ),
    connect( this, SIGNAL( contextMenuRequested( QListViewItem *, const QPoint &, int ) ),
	     this, SLOT( rmbClicked( QListViewItem *, const QPoint& ) ) ),
    setHScrollBarMode( AlwaysOff );
    setVScrollBarMode( AlwaysOn );
    viewport()->setAcceptDrops( TRUE );
    setAcceptDrops( TRUE );
    setColumnWidthMode( 1, Manual );

    if ( !folderPixmap ) {
	folderPixmap = new QPixmap( folder_xpm );
	filePixmap = new QPixmap( file_xpm );
	formPixmap = new QPixmap( PixmapChooser::loadPixmap( "form.xpm", PixmapChooser::Mini ) );
	objectPixmap = new QPixmap( object_xpm );
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
	disconnect( project, SIGNAL( objectAdded(QObject*) ), this, SLOT( objectAdded(QObject*) ) );
	disconnect( project, SIGNAL( objectRemoved(QObject*) ), this, SLOT( objectRemoved(QObject*) ) );
	disconnect( project, SIGNAL( projectModified() ), this, SLOT( update() ) );
    }
    project = pro;
    connect( project, SIGNAL( sourceFileAdded(SourceFile*) ), this, SLOT( sourceFileAdded(SourceFile*) ) );
    connect( project, SIGNAL( sourceFileRemoved(SourceFile*) ), this, SLOT( sourceFileRemoved(SourceFile*) ) );
    connect( project, SIGNAL( formFileAdded(FormFile*) ), this, SLOT( formFileAdded(FormFile*) ) );
    connect( project, SIGNAL( formFileRemoved(FormFile*) ), this, SLOT( formFileRemoved(FormFile*) ) );
    connect( project, SIGNAL( destroyed(QObject*) ), this, SLOT( projectDestroyed(QObject*) ) );
    connect( project, SIGNAL( objectAdded(QObject*) ), this, SLOT( objectAdded(QObject*) ) );
    connect( project, SIGNAL( objectRemoved(QObject*) ), this, SLOT( objectRemoved(QObject*) ) );
    connect( project, SIGNAL( projectModified() ), this, SLOT( update() ) );
    clear();

    if ( bufferEdit )
	bufferEdit->clear();

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
	if ( f->isFake() )
	    continue;
	(void) new WorkspaceItem( projectItem, f );
    }

    for ( QObjectListIt objs = project->objects();
	  objs.current(); ++objs ) {
	QObject* o = objs.current();
	(void) new WorkspaceItem( projectItem, o, project );
    }

    updateColors();
    completionDirty = TRUE;
}



void Workspace::sourceFileAdded( SourceFile* sf )
{
    (void) new WorkspaceItem( projectItem, sf );
    updateColors();
}

void Workspace::sourceFileRemoved( SourceFile* sf )
{
    delete findItem( sf );
    updateColors();
}

void Workspace::formFileAdded( FormFile* ff )
{
    if ( ff->isFake() )
	return;
    (void) new WorkspaceItem( projectItem, ff );
    updateColors();
}

void Workspace::formFileRemoved( FormFile* ff )
{
    delete findItem( ff );
    updateColors();
}

void Workspace::objectAdded( QObject *o )
{
    (void) new WorkspaceItem( projectItem, o, project );
    updateColors();
}

void Workspace::objectRemoved( QObject *o )
{
    delete findItem( o );
    updateColors();
}

void Workspace::update()
{
    completionDirty = TRUE;
    triggerUpdate();
}

void Workspace::update( FormFile* ff )
{
    QListViewItem* i = findItem( ff );
    if ( i ) {
	i->repaint();
	if ( (i = i->firstChild()) )
	    i->repaint();
    }
}


void Workspace::activeFormChanged( FormWindow *fw )
{
    WorkspaceItem *i = findItem( fw->formFile() );
    if ( i ) {
	setCurrentItem( i );
	setSelected( i, TRUE );
	if ( !i->isOpen() )
	    i->setAutoOpen( TRUE );
    }

    closeAutoOpenItems();
	
}

void Workspace::activeEditorChanged( SourceEditor *se )
{
    if ( !se->object() )
	return;
	
    if ( se->formWindow() ) {
	WorkspaceItem *i = findItem( se->formWindow()->formFile() );
	if ( i && i->firstChild() ) {
	    if ( !i->isOpen() )
		i->setAutoOpen( TRUE );
	    setCurrentItem( i->firstChild() );
	    setSelected( i->firstChild(), TRUE );
	}
    } else {
	WorkspaceItem *i = findItem( se->sourceFile() );
	if ( i ) {
	    setCurrentItem( i );
	    setSelected( i, TRUE );
	}
    }

    closeAutoOpenItems();
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

WorkspaceItem *Workspace::findItem( QObject *o )
{
    QListViewItemIterator it( this );
    for ( ; it.current(); ++it ) {
	if ( ( (WorkspaceItem*)it.current() )->object == o )
	    return (WorkspaceItem*)it.current();
    }
    return 0;
}

void Workspace::closeAutoOpenItems()
{
    QListViewItemIterator it( this );
    for ( ; it.current(); ++it ) {
	WorkspaceItem* i = (WorkspaceItem*) it.current();
	WorkspaceItem* ip = (WorkspaceItem*) i->parent();
	if ( i->type() == WorkspaceItem::FormSourceType ) {
	    if ( !i->isSelected() && !ip->isSelected()
		 && ip->isAutoOpen() ) {
		ip->setAutoOpen( FALSE );
	    }
	}
    }
}


void Workspace::closeEvent( QCloseEvent *e )
{
    e->accept();
}

void Workspace::itemDoubleClicked( QListViewItem *i )
{
    if ( ( (WorkspaceItem*)i)->type()== WorkspaceItem::ProjectType )
	i->setOpen( TRUE );
}

void Workspace::itemClicked( int button, QListViewItem *i, const QPoint& )
{
    if ( !i || button != LeftButton )
	return;

    closeAutoOpenItems();

    WorkspaceItem* wi = (WorkspaceItem*)i;
    if ( wi->type() == WorkspaceItem::SourceFileType )
	mainWindow->editSource( wi->sourceFile );
    switch( wi->type() ) {
    case WorkspaceItem::ProjectType:
	break; // ### TODO
    case WorkspaceItem::FormFileType:
	wi->formFile->showFormWindow();
	break;
    case WorkspaceItem::FormSourceType:
	wi->formFile->showEditor();
	break;
    case WorkspaceItem::SourceFileType:
	mainWindow->editSource( wi->sourceFile );
	break;
    case WorkspaceItem::ObjectType:
	project->fakeFormFor( wi->object )->setFocus();
	mainWindow->propertyeditor()->setWidget( wi->object, project->fakeFormFor( wi->object ) );
	mainWindow->objectHierarchy()->setFormWindow( project->fakeFormFor( wi->object ), wi->object );
	break;
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

void Workspace::rmbClicked( QListViewItem *i, const QPoint& pos )
{
    if ( !i )
	return;
    WorkspaceItem* wi = (WorkspaceItem*)i;
    enum { OPEN_SOURCE, REMOVE_SOURCE, OPEN_FORM, REMOVE_FORM, OPEN_FORM_SOURCE };
    QPopupMenu menu( this );
    switch ( wi->type() ) {
    case WorkspaceItem::SourceFileType:
	menu.insertItem( tr( "&Open source file..." ), OPEN_SOURCE );
	menu.insertSeparator();
	menu.insertItem( PixmapChooser::loadPixmap( "editcut" ),
			 tr( "&Remove source file from project" ), REMOVE_SOURCE );
	break;
    case WorkspaceItem::FormFileType:
	menu.insertItem( tr( "&Open form..." ), OPEN_FORM );
	menu.insertSeparator();
	menu.insertItem( PixmapChooser::loadPixmap( "editcut" ),
			 tr( "&Remove form from project" ), REMOVE_FORM );
	break;
    case WorkspaceItem::FormSourceType:
	menu.insertItem( tr( "&Open form source..." ), OPEN_FORM_SOURCE );
	menu.insertSeparator();
	menu.insertItem( PixmapChooser::loadPixmap( "editcut" ),
			 tr( "&Remove form from project" ), REMOVE_FORM );
	break;
    case WorkspaceItem::ProjectType:
	MainWindow::self->popupProjectMenu( pos );
	return;
    }
    switch ( menu.exec( pos ) ) {
    case REMOVE_SOURCE:
	project->removeSourceFile( wi->sourceFile );
	break;
    case OPEN_SOURCE:
	itemClicked( LeftButton, i, pos );
	break;
    case REMOVE_FORM:
	project->removeFormFile( wi->formFile );
	break;
    case OPEN_FORM:
	itemClicked( LeftButton, i, pos );
	break;
    case OPEN_FORM_SOURCE:
	itemClicked( LeftButton, i, pos );
	break;
    }
}
	
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
    if ( !bufferEdit || !completionDirty || !MainWindow::self)
	return;
    completionDirty = FALSE;
    QStringList completion = MainWindow::self->projectFileNames();
    QListViewItemIterator it( this );
    while ( it.current() ) {
	( (WorkspaceItem*)it.current())->fillCompletionList( completion );
	++it;
    }
    completion.sort();
    bufferEdit->setCompletionList( completion );
}

void Workspace::bufferChosen( const QString &buffer )
{
    if ( bufferEdit )
	bufferEdit->setText( "" );

    if ( MainWindow::self->projectFileNames().contains( buffer ) ) {
	MainWindow::self->setCurrentProjectByFilename( buffer );
	return;
    }

    QListViewItemIterator it( this );
    while ( it.current() ) {
	if ( ( (WorkspaceItem*)it.current())->checkCompletion( buffer ) ) {
	    itemClicked( LeftButton, it.current(), QPoint() );
	    break;
	}
	++it;
    }
}

void Workspace::updateColors()
{
    QListViewItem* i = firstChild();
    if ( i )
	i = i->firstChild();
    bool b = TRUE;
    while ( i ) {
	WorkspaceItem* wi = ( WorkspaceItem*) i;
	i = i->nextSibling();
	wi->useOddColor = b;
	b = !b;
    }
}
