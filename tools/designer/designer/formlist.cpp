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
#include "formlist.h"
#include "formwindow.h"
#include "mainwindow.h"
#include "pixmapchooser.h"
#include "globaldefs.h"
#include "command.h"
#include "project.h"

#include <qheader.h>
#include <qdragobject.h>
#include <qfileinfo.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qpen.h>
#include <qobjectlist.h>
#include <qworkspace.h>

static bool blockNewForms = FALSE;

FormListItem::FormListItem( QListView *parent, const QString &form, const QString &file, FormWindow *fw )
    : QListViewItem( parent, form, file, "" ), formwindow( fw )
{
    setPixmap( 0, PixmapChooser::loadPixmap( "form.xpm", PixmapChooser::Mini ) );
}

void FormListItem::paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int align )
{
    QColorGroup g( cg );
    g.setColor( QColorGroup::Base, backgroundColor() );
    g.setColor( QColorGroup::Foreground, Qt::black );
    g.setColor( QColorGroup::Text, Qt::black );
    p->save();

    if ( formWindow() && formWindow()->commandHistory()->isModified() ) {
	QFont f = p->font();
	f.setBold( TRUE );
	p->setFont( f );
    }
	
    QListViewItem::paintCell( p, g, column, width, align );
    p->setPen( QPen( cg.dark(), 1 ) );
    if ( column == 0 )
	p->drawLine( 0, 0, 0, height() - 1 );
    p->drawLine( 0, height() - 1, width, height() - 1 );
    p->drawLine( width - 1, 0, width - 1, height() );
    p->restore();
}

QColor FormListItem::backgroundColor()
{
    updateBackColor();
    return backColor;
}

void FormListItem::updateBackColor()
{
    if ( listView()->firstChild() == this ) {
 	backColor = backColor1;
	return;
    }

    QListViewItemIterator it( this );
    --it;
    if ( it.current() ) {
	if ( ( ( FormListItem*)it.current() )->backColor == backColor1 )
	    backColor = backColor2;
	else
	    backColor = backColor1;
    } else {
	backColor == backColor1;
    }
}

FormList::FormList( QWidget *parent, MainWindow *mw, Project *pro )
    : QListView( parent, 0, WStyle_Customize | WStyle_NormalBorder | WStyle_Title |
		 WStyle_Tool | WStyle_MinMax | WStyle_SysMenu ), mainWindow( mw ),
	project( pro )
{
    header()->setMovingEnabled( FALSE );
    setResizePolicy( QScrollView::Manual );
    setIcon( PixmapChooser::loadPixmap( "logo" ) );
    QPalette p( palette() );
    p.setColor( QColorGroup::Base, QColor( backColor2 ) );
    setPalette( p );
    addColumn( tr( "Form" ) );
    addColumn( tr( "Filename" ) );
    addColumn( tr( "Changed" ) );
    setAllColumnsShowFocus( TRUE );
    connect( header(), SIGNAL( sizeChange( int, int, int ) ),
	     this, SLOT( updateHeader() ) );
    connect( this, SIGNAL( clicked( QListViewItem * ) ),
	     this, SLOT( itemClicked( QListViewItem * ) ) ),
    setHScrollBarMode( AlwaysOff );
    viewport()->setAcceptDrops( TRUE );
    setAcceptDrops( TRUE );
}

void FormList::setProject( Project *pro )
{
    project = pro;
    clear();

    QStringList lst = project->uiFiles();
    for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
	(void)new FormListItem( this, tr( "<unknown>" ), *it, 0 );
    }

    QObjectList *l = mainWindow->workSpace()->queryList( "FormWindow", 0, FALSE, TRUE );
    for ( QObject *o = l->first(); o; o = l->next() ) {
	if ( ( (FormWindow*)o )->project() != pro )
	    continue;
	QListViewItemIterator it( this );
	while ( it.current() ) {
	    if ( project->makeAbsolute( ( (FormListItem*)it.current() )->text( 1 ) ) ==
		 project->makeAbsolute( ( (FormWindow*)o )->fileName() ) )
		( (FormListItem*)it.current() )->setFormWindow( ( (FormWindow*)o ) );
	    ++it;
	}
    }

    updateHeader();
}

void FormList::addForm( FormWindow *fw )
{
    fw->setProject( project );
    if ( blockNewForms ) {
	if ( currentItem() ) {
	    ( (FormListItem*)currentItem() )->setFormWindow( fw );
	    ( (FormListItem*)currentItem() )->setText( 0, fw->name() );
	}
	if ( project )
	    project->setFormWindow( fw->fileName(), fw );
	return;
    }
    
    QString fn = project->makeRelative( fw->fileName() );
    FormListItem *i = new FormListItem( this, fw->name(), fn, 0 );
    i->setFormWindow( fw );
    if ( !project )
	return;
    project->addUiFile( fn, fw );
    updateHeader();
}

void FormList::modificationChanged( bool m, FormWindow *fw )
{
    FormListItem *i = findItem( fw );
    if ( i ) {
    	QString s;
    	if ( m )
      	s = tr( "*" );
      else
        s = tr( "" );
    	i->setText( 2, s );
    }
}

void FormList::fileNameChanged( const QString &fn, FormWindow *fw )
{
    QString s = project->makeRelative( fn );
    FormListItem *i = findItem( fw );
    if ( !i )
	return;
    if ( s.isEmpty() )
	i->setText( 1, tr( "(unnamed)" ) );
    else
	i->setText( 1, s );
    if ( project )
	project->setFormWindowFileName( fw, s );
}

void FormList::activeFormChanged( FormWindow *fw )
{
    FormListItem *i = findItem( fw );
    if ( i ) {
	setCurrentItem( i );
	setSelected( i, TRUE );
    }
}

void FormList::nameChanged( FormWindow *fw )
{
    FormListItem *i = findItem( fw );
    if ( !i )
	return;
    i->setText( 0, fw->name() );
}

FormListItem *FormList::findItem( FormWindow *fw )
{
    QListViewItemIterator it( this );
    for ( ; it.current(); ++it ) {
	if ( ( (FormListItem*)it.current() )->formWindow() == fw )
	    return (FormListItem*)it.current();
    }
    return 0;
}


void FormList::closed( FormWindow *fw )
{
    FormListItem *i = findItem( fw );
    if ( i ) {
	i->setFormWindow( 0 );
	i->repaint();
    }
}

void FormList::resizeEvent( QResizeEvent *e )
{
    QListView::resizeEvent( e );
    QSize vs = viewportSize( 0, contentsHeight() );

    int ns = vs.width() - header()->sectionSize( 0 ) - header()->sectionSize( 2 );
    if ( ns < 16 )
	ns = 16;
	
    header()->resizeSection( 1, ns );
    header()->repaint( FALSE );

    viewport()->repaint( FALSE );
}

void FormList::updateHeader()
{
    QSize s( header()->sectionPos( 2 ) + header()->sectionSize( 1 ), height() );
    QResizeEvent e( s, size() );
    resizeEvent( &e );
    viewport()->repaint( s.width(), 0, width() - s.width(), height(), FALSE );
}

void FormList::closeEvent( QCloseEvent *e )
{
    emit hidden();
    e->accept();
}

void FormList::itemClicked( QListViewItem *i )
{
    if ( !i )
	return;
    if ( ( (FormListItem*)i )->formWindow() ) {
	( (FormListItem*)i )->formWindow()->setFocus();
    } else {
	blockNewForms = TRUE;
	mainWindow->openFile( project->makeAbsolute( ( (FormListItem*)i )->text( 1 ) ) );
	blockNewForms = FALSE;
    }
}

void FormList::contentsDropEvent( QDropEvent *e )
{
    if ( !QUriDrag::canDecode( e ) ) {
	e->ignore();
    } else {
	QStringList files;
	QUriDrag::decodeLocalFiles( e, files );
	if ( !files.isEmpty() ) {
	    for ( QStringList::Iterator it = files.begin(); it != files.end(); ++it ) {
		QString fn = *it;
		if ( QFileInfo( fn ).extension().lower() == "ui" )
		    mainWindow->openFile( fn );
	    }
	}
    }
}

void FormList::contentsDragEnterEvent( QDragEnterEvent *e )
{
    if ( !QUriDrag::canDecode( e ) )
	e->ignore();
    else
	e->accept();
}

void FormList::contentsDragMoveEvent( QDragMoveEvent *e )
{
    if ( !QUriDrag::canDecode( e ) )
	e->ignore();
    else
	e->accept();
}
