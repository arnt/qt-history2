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

WorkspaceItem::WorkspaceItem( QListView *parent, Project* p )
    : QListViewItem( parent )
{
    init();
    project = p;
    t = ProjectType;
    setPixmap( 0, *folderPixmap );
}

WorkspaceItem::WorkspaceItem( QListViewItem *parent, SourceFile* sf )
    : QListViewItem( parent )
{
    init();
    sourceFile = sf;
    t = SourceFileType;
    setPixmap( 0, *filePixmap );
}


void WorkspaceItem::init()
{
    project = 0;
    sourceFile = 0;
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
    case FormType:
	return "form"; //###
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
    case FormType:
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
    case FormType:
	return FALSE; //###
    case FormUiType:
	return FALSE; //###
    case FormSourceType:
	return FALSE;  //###
    case SourceFileType:
	return sourceFile->isModified();
    }
    return FALSE; // shut up compiler
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
    setSorting( -1 );
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
	disconnect( project, SIGNAL( projectModified() ), this, SLOT( update() ) );
    }
    project = pro;
    connect( project, SIGNAL( sourceFileAdded(SourceFile*) ), this, SLOT( sourceFileAdded(SourceFile*) ) );
    connect( project, SIGNAL( sourceFileRemoved(SourceFile*) ), this, SLOT( sourceFileRemoved(SourceFile*) ) );
    connect( project, SIGNAL( destroyed(QObject*) ), this, SLOT( projectDestroyed(QObject*) ) );
    connect( project, SIGNAL( projectModified() ), this, SLOT( update() ) );
    clear();
    qDebug("Workspace::setCurrentProject %s", pro->projectName().latin1() );

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

    completionDirty = TRUE;


    /*
   if ( iface && iface->supports( LanguageInterface::AdditionalFiles ) ) {
	sourceParent = new WorkspaceItem( this );
	sourceParent->setType( WorkspaceItem::Parent );
	sourceParent->setText( 0, tr( "Source Files" ) );
	sourceParent->setPixmap( 0, *folderPixmap );
	sourceParent->setOpen( TRUE );
    }

    formsParent = new WorkspaceItem( this );
    formsParent->setType( WorkspaceItem::Parent );
    formsParent->setText( 0, tr( "Forms" ) );
    formsParent->setPixmap( 0, *folderPixmap );
    formsParent->setOpen( TRUE );

    QStringList lst = project->uiFiles();
    for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
	if ( (*it).isEmpty() )
	    continue;
	WorkspaceItem *item = new WorkspaceItem( formsParent, tr( "<unknown>" ), *it, 0 );
	item->setType( WorkspaceItem::Form );
	QString className = project->formName( item->text( 1 ) );
	if ( bufferEdit )
	    bufferEdit->addCompletionEntry( item->text( 1 ) );
	if ( QFile::exists( project->makeAbsolute( item->text( 1 ) + extension ) ) ) {
	    if ( bufferEdit )
		bufferEdit->addCompletionEntry( item->text( 1 ) + extension );
	}
	if ( !className.isEmpty() ) {
	    item->setText( 0, className );
	    if ( bufferEdit )
		bufferEdit->addCompletionEntry( className );
	}
    }

    QObjectList *l = mainWindow->qWorkspace()->queryList( "FormWindow", 0, FALSE, TRUE );
    for ( QObject *o = l->first(); o; o = l->next() ) {
	if ( ( (FormWindow*)o )->project() != pro )
	    continue;
	QListViewItemIterator it( this );
	while ( it.current() ) {
	    if ( !it.current()->parent() ) {
		++it;
		continue;
	    }
	    if ( project->makeAbsolute( ( (WorkspaceItem*)it.current() )->text( 1 ) ) ==
		 project->makeAbsolute( ( (FormWindow*)o )->fileName() ) ) {
		( (WorkspaceItem*)it.current() )->setFormWindow( ( (FormWindow*)o ) );
		it.current()->setText( 0, o->name() );
		if ( bufferEdit )
		    bufferEdit->addCompletionEntry( o->name() );
	    }
	    ++it;
	}
    }

    QPtrList<FormWindow> forms = project->unnamedForms();
    for ( FormWindow *fw = forms.first(); fw; fw = forms.next() ) {
	WorkspaceItem *item = new WorkspaceItem( formsParent, fw->mainContainer()->name(), "", fw );
	if ( bufferEdit )
	    bufferEdit->addCompletionEntry( fw->mainContainer()->name() );
	item->setType( WorkspaceItem::Form );
    }

    */
    /*
      // Images do not make sense here yet until we offer an image editor
    QValueList<PixmapCollection::Pixmap> pixmaps = project->pixmapCollection()->pixmaps();
    {
	for ( QValueList<PixmapCollection::Pixmap>::Iterator it = pixmaps.begin(); it != pixmaps.end(); ++it ) {
	    WorkspaceItem *item = new WorkspaceItem( imageParent, (*it).name, "", 0 );
	    QPixmap pix( (*it).pix );
	    QImage img = pix.convertToImage();
	    img = img.smoothScale( 20, 20 );
	    pix.convertFromImage( img );
	    item->setPixmap( 0, pix );
	    item->setType( WorkspaceItem::Image );
	}
    }
    */

    /*
    if ( !iface || !iface->supports( LanguageInterface::AdditionalFiles ) )
	return;

    QPtrList<SourceFile> sources = pro->sourceFiles();
    for ( SourceFile *f = sources.first(); f; f = sources.next() ) {
	WorkspaceItem *fi = new WorkspaceItem( sourceParent, pro->makeRelative( f->fileName() ), f );
	fi->setPixmap( 0, *filePixmap );
	if ( bufferEdit )
	    bufferEdit->addCompletionEntry( pro->makeRelative( f->fileName() ) );
    }
    */
}



void Workspace::sourceFileAdded( SourceFile* sf )
{
    qDebug("source file added");
    (void) new WorkspaceItem( projectItem, sf );
}

void Workspace::sourceFileRemoved( SourceFile* sf )
{
    delete findItem( sf );
}


void Workspace::update()
{
    completionDirty = TRUE;
    // #### rebuild completion list
    triggerUpdate();
}

// void Workspace::addForm( FormWindow *fw )
// {
//     fw->setProject( project );
//     if ( blockNewForms ) {
// 	if ( currentItem() ) {
// 	    ( (WorkspaceItem*)currentItem() )->setFormWindow( fw );
// 	    ( (WorkspaceItem*)currentItem() )->setText( 0, fw->name() );
// 	}
// 	if ( project ) {
// 	    project->setFormWindow( fw->fileName(), fw );
// 	    bufferEdit->addCompletionEntry( fw->fileName() );
// 	    bufferEdit->addCompletionEntry( fw->name() );
// 	}
// 	return;
//     }

//     QString fn = project->makeRelative( fw->fileName() );
//     if ( project->hasUiFile( fn ) )
// 	return;
//     WorkspaceItem *i = new WorkspaceItem( formsParent, fw->name(), fn, 0 );
//     i->setType( WorkspaceItem::Form );
//     i->setFormWindow( fw );
//     bufferEdit->addCompletionEntry( fw->name() );
//     bufferEdit->addCompletionEntry( fw->fileName() );
//     if ( !project )
// 	return;
//     project->addUiFile( fn, fw );
// }

// void Workspace::removeFormFromProject( QListViewItem *i )
// {
//     if ( ( (WorkspaceItem*)i )->formWindow() ) {
// 	if ( !( (WorkspaceItem*)i )->formWindow()->close() )
// 	    return;
//     }
//     project->removeUiFile( ( (WorkspaceItem*)i )->text( 1 ), ( (WorkspaceItem*)i )->formWindow() );
//     delete i;
// }

// void Workspace::removeFormFromProject( const QString &file )
// {
//     QListViewItemIterator it( this );
//     while ( it.current() ) {
// 	if ( it.current()->rtti() == WorkspaceItem::Form ) {
// 	    qDebug( "   " + it.current()->text( 1 ) );
// 	    if ( it.current()->text( 1 ) == file ) {
// 		removeFormFromProject( it.current() );
// 		return;
// 	    }
// 	}
// 	++it;
//     }
// }

// void Workspace::removeSourceFromProject( const QString &file )
// {
//     QListViewItemIterator it( this );
//     while ( it.current() ) {
// 	if ( it.current()->rtti() == WorkspaceItem::Source ) {
// 	    qDebug( "   " + it.current()->text( 1 ) );
// 	    if ( it.current()->text( 0 ) == file ) {
// 		removeSourceFromProject( it.current() );
// 		return;
// 	    }
// 	}
// 	++it;
//     }
// }

// void Workspace::removeSourceFromProject( QListViewItem *i )
// {
//     SourceFile *sf = ( (WorkspaceItem*)i )->sourceFile();
//     project->removeSourceFile( sf );
//     if ( sf->editor() ) {
// 	sf->editor()->setModified( FALSE );
// 	sf->editor()->close();
//     }
//     delete i;
// }

// void Workspace::removeFormFromProject( FormWindow *fw )
// {
//     WorkspaceItem *i = findItem( fw );
//     if ( i )
// 	removeFormFromProject( i );
// }


// void Workspace::modificationChanged( bool, QObject *obj )
// {
//     if ( obj->inherits( "FormWindow" ) ) {
// 	WorkspaceItem *i = findItem( (FormWindow*)obj );
// 	if ( i )
// 	    i->repaint();
//     } else if ( obj->inherits( "SourceEditor" ) ) {
// 	WorkspaceItem *i = findItem( (SourceFile*)( (SourceEditor*)obj )->object() );
// 	if ( i )
// 	    i->repaint();
//     }
// }

// void Workspace::fileNameChanged( const QString &fn, FormWindow *fw )
// {
//     QString extension = "xx";
//     LanguageInterface *iface = MetaDataBase::languageInterface( project->language() );
//     if ( iface )
// 	extension = iface->formCodeExtension();

//     QString s = project->makeRelative( fn );
//     WorkspaceItem *i = findItem( fw );
//     if ( !i )
// 	return;
//     bufferEdit->removeCompletionEntry( i->text( 1 ) );
//     bufferEdit->removeCompletionEntry( i->text( 1 ) + extension );
//     if ( s.isEmpty() ) {
// 	i->setText( 1, tr( "(unnamed)" ) );
//     } else {
// 	i->setText( 1, s );
// 	bufferEdit->addCompletionEntry( s );
// 	if ( QFile::exists( project->makeAbsolute( s + extension ) ) )
// 	    bufferEdit->addCompletionEntry( s + extension );
//     }
//     if ( project )
// 	project->setFormWindowFileName( fw, s );
// }

void Workspace::activeFormChanged( FormWindow *fw )
{
    WorkspaceItem *i = findItem( fw );
    if ( i ) {
	setCurrentItem( i );
	setSelected( i, TRUE );
    }
}

void Workspace::activeEditorChanged( SourceEditor *se )
{
    if ( !se->object() )
	return;
    if ( se->object()->inherits( "FormWindow" ) ) {
	activeFormChanged( (FormWindow*)se->object() );
	return;
    }
    WorkspaceItem *i = findItem( (SourceFile*)se->object() );
    if ( i ) {
	setCurrentItem( i );
	setSelected( i, TRUE );
    }
}

// void Workspace::nameChanged( FormWindow *fw )
// {
//     WorkspaceItem *i = findItem( fw );
//     if ( !i )
// 	return;
//     i->setText( 0, fw->name() );
// }

WorkspaceItem *Workspace::findItem( FormWindow */*fw*/ )
{
    QListViewItemIterator it( this );
    for ( ; it.current(); ++it ) {
// 	if ( ( (WorkspaceItem*)it.current() )->formWindow() == fw )
// 	    return (WorkspaceItem*)it.current();
	
	//###
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

// void Workspace::closed( FormWindow *fw )
// {
//     WorkspaceItem *i = findItem( fw );
//     if ( i ) {
// 	if ( fw->fileName().isEmpty() ) {
// 	    delete i;
// 	} else {
// 	    i->setFormWindow( 0 );
// 	    i->repaint();
// 	}
//     }
// }

void Workspace::closeEvent( QCloseEvent *e )
{
    e->accept();
}

void Workspace::itemClicked( int button, QListViewItem *i )
{
    if ( !i || button != LeftButton )
	return;
    /*
    if ( i->rtti() == WorkspaceItem::Form ) {
	if ( ( (WorkspaceItem*)i )->formWindow() ) {
	    ( (WorkspaceItem*)i )->formWindow()->setFocus();
	} else {
	    blockNewForms = TRUE;
	    mainWindow->openFile( project->makeAbsolute( ( (WorkspaceItem*)i )->text( 1 ) ) );
	    blockNewForms = FALSE;
	}
    } else if ( i->rtti() == WorkspaceItem::Source ) {
	mainWindow->editSource( ( (WorkspaceItem*)i )->sourceFile() );
    }
    */

    WorkspaceItem* wi = (WorkspaceItem*)i;
    if ( wi->type() == WorkspaceItem::SourceFileType )
	mainWindow->editSource( wi->sourceFile );
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

/*
void Workspace::openForm( const QString &filename )
{
    QListViewItemIterator it( this );
    while ( it.current() ) {
	if ( it.current()->rtti() == WorkspaceItem::Form ) {
	    if ( it.current()->text( 1 ) == filename ) {
		setCurrentItem( it.current() );
		setSelected( it.current(), TRUE );
		blockNewForms = TRUE;
		mainWindow->openFile( project->makeAbsolute( ( (WorkspaceItem*)it.current() )->text( 1 ) ) );
		blockNewForms = FALSE;
	    }
	}
	++it;
    }
}
*/
