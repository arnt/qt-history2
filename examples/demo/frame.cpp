/****************************************************************************
** $Id: //depot/qt/main/examples/demo/frame.cpp#2 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "frame.h"

#include <qapplication.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qaccel.h>
#include <qsplitter.h>
#include <qlistbox.h>
#include <qwidgetstack.h>
#include <qwindowsstyle.h>
#include <qmotifstyle.h>

class CategoryItem : public QListBoxPixmap {
public:
    CategoryItem( QListBox *parent, QWidget *widget,
		  const QPixmap &p, const QString &name, int id );
    virtual QString key( int, bool ) const;
    QWidget *widget() const { return _widget; }
private:
    int _id;
    QWidget *_widget;
};

CategoryItem::CategoryItem( QListBox *parent, QWidget *widget,
			    const QPixmap &p, const QString &name, int id )
    : QListBoxPixmap( parent, p, name ),
      _id( id ),
      _widget( widget )
{
};

QString CategoryItem::key( int, bool ) const
{
    QString tmp;
    return tmp.sprintf( "%03d", _id );
}

Frame::Frame( QWidget *parent, const char *name )
    : QMainWindow( parent, name )
{
    setCaption( tr( "Qt Demo Collection" ) );

    // set up the menu bar
    QMenuBar *mainMenu = menuBar();
    QPopupMenu *fileMenu = new QPopupMenu( this, "file" );
    fileMenu->insertItem( tr( "&Exit" ), this, SLOT( close() ),
			  QAccel::stringToKey( tr( "Ctrl+Q" ) ) );

    QPopupMenu *styleMenu = new QPopupMenu( this, "style" );
    styleMenu->setCheckable( TRUE );
    idWindows = styleMenu->insertItem( "Windows", this, SLOT( styleWindows() ) );
    idMotif = styleMenu->insertItem( "Motif", this, SLOT( styleMotif() ) );

    QPopupMenu *langMenu = new QPopupMenu( this, "language" );
    styleMenu->setCheckable( TRUE );
    idEnglish = langMenu->insertItem( tr( "English" ) );
    langMenu->insertItem( tr( "Mandarin" ) );

    mainMenu->insertItem( tr( "&File" ), fileMenu );
    mainMenu->insertItem( tr( "St&yle" ), styleMenu );
    mainMenu->insertItem( tr( "&Language" ), langMenu );
    mainMenu->setItemChecked( idEnglish, TRUE );

    // category chooser
    QSplitter *splitter = new QSplitter( this );
    categories = new QListBox( splitter );
    connect( categories, SIGNAL( clicked( QListBoxItem *) ),
	     SLOT( clickedCategory( QListBoxItem *) ) );

    // stack for the demo widgets
    stack = new QWidgetStack( splitter );

    setCentralWidget( splitter );
}

void Frame::addCategory( QWidget *w, const QPixmap &p, const QString &n )
{
    int i = categories->count();
    stack->addWidget( w, i );
    CategoryItem *item = new CategoryItem( categories, w, p, n, i );
    if ( !stack->visibleWidget() ) {
	categories->setCurrentItem( item );
	stack->raiseWidget( w );
    }
    if ( i < 3 && categories->height() < 3 * item->height( categories ) )
	categories->setMinimumHeight( 3 * item->height( categories ) );
}

void Frame::styleWindows()
{
    setStyle( idWindows, new QWindowsStyle() );
}

void Frame::styleMotif()
{
    setStyle( idMotif, new QMotifStyle() );
}

void Frame::setStyle( int i, QStyle *s )
{
    QApplication::setStyle( s );
    menuBar()->setItemChecked( idWindows, FALSE );
    menuBar()->setItemChecked( idMotif, FALSE );
    menuBar()->setItemChecked( i, TRUE );
}

void Frame::clickedCategory( QListBoxItem *item )
{
    if ( item ) {
	CategoryItem *c = (CategoryItem*)item;
	stack->raiseWidget( c->widget() );
    }
}
