/****************************************************************************
** $Id: //depot/qt/main/examples/demo/frame.cpp#7 $
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
#include <qpainter.h>
#include <qwidgetstack.h>
#include <qwindowsstyle.h>
#include <qmotifstyle.h>
#include <qmotifplusstyle.h>
#include <qplatinumstyle.h>
#include <qsgistyle.h>

class CategoryItem : public QListBoxItem {
public:
    CategoryItem( QListBox *parent, QWidget *widget,
		  const QPixmap &p, const QString &name, int id );
    CategoryItem(QListBox *parent, QWidget *widget,
		  const QPixmap &p1, const QPixmap &p2, const QString &name, int id );
    virtual QString key( int, bool ) const;
    virtual int height( const QListBox * ) const;
    virtual int width( const QListBox * )  const;


    QWidget *widget() const { return _widget; }
protected:
    virtual void paint( QPainter * );
private:
    int _id;
    QWidget *_widget;
    QPixmap pm_Unsel;
    QPixmap pm_Sel;
};

CategoryItem::CategoryItem( QListBox *parent, QWidget *widget,
			    const QPixmap &p, const QString &name, int id )
    : QListBoxItem( parent ),
      _id( id ),
      _widget( widget ),
      pm_Unsel( p ),
      pm_Sel( p )

{
    setText( name );
}

CategoryItem::CategoryItem( QListBox * parent, QWidget *widget, const QPixmap &p1, const QPixmap &p2,
			    const QString &name, int id )
    : QListBoxItem( parent ),
      _id( id),
      _widget( widget ),
      pm_Unsel( p1 ),
      pm_Sel( p2 )
{
    setText( name );
}



QString CategoryItem::key( int, bool ) const
{
    QString tmp;
    return tmp.sprintf( "%03d", _id );
}

int CategoryItem::height( const QListBox * ) const
{
    return 100;
}

int CategoryItem::width( const QListBox * )  const
{
    return 150;
}

void CategoryItem::paint( QPainter *p )
{
    int w = width( listBox() );
    int tx = (w-p->fontMetrics().boundingRect(text()).width())/2;
    p->drawText( tx, 80, text() );
    if ( selected() )
	p->drawPixmap( (w-pm_Sel.width())/2, 10, pm_Sel );
    else
    	p->drawPixmap( (w-pm_Unsel.width())/2, 10, pm_Unsel );
}

Frame::Frame( QWidget *parent, const char *name )
    : QMainWindow( parent, name )
{
    title = tr( "Qt Demo Collection" );
    setCaption( title );

    // set up the menu bar
    QMenuBar *mainMenu = menuBar();
    QPopupMenu *fileMenu = new QPopupMenu( this, "file" );
    fileMenu->insertItem( tr( "&Exit" ), this, SLOT( close() ),
			  QAccel::stringToKey( tr( "Ctrl+Q" ) ) );

    QPopupMenu *styleMenu = new QPopupMenu( this, "style" );
    styleMenu->setCheckable( TRUE );
    idWindows = styleMenu->insertItem( "Windows", this,
				       SLOT( styleWindows() ) );
    idMotif = styleMenu->insertItem( "Motif", this, SLOT( styleMotif() ) );
    idMotifPlus = styleMenu->insertItem( "Motif Plus", this,
					 SLOT( styleMotifPlus() ) );
    idPlatinum = styleMenu->insertItem( "Platinum", this,
					SLOT( stylePlatinum() ) );
    idSGI = styleMenu->insertItem( "SGI", this, SLOT( styleSGI() ) );

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
    QFont f = categories->font();
    f.setWeight( QFont::Bold );
    categories->setFont( f );

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
	clickedCategory( item );
    }
    if ( i < 3 && categories->height() < 3 * item->height( categories ) )
	categories->setMinimumHeight( 3 * item->height( categories ) );
}

void Frame::addCategory( QWidget *w, const QPixmap &p1, const QPixmap &p2, const QString &n )
{
    int i = categories->count();
    stack->addWidget( w, i );
    CategoryItem *item = new CategoryItem( categories, w, p1, p2, n, i );
    if ( !stack->visibleWidget() ) {
	categories->setCurrentItem( item );
	clickedCategory( item );
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

void Frame::styleMotifPlus()
{
    setStyle( idMotifPlus, new QMotifPlusStyle() );
}

void Frame::stylePlatinum()
{
    setStyle( idPlatinum, new QPlatinumStyle() );
}

void Frame::styleSGI()
{
    setStyle( idSGI, new QSGIStyle() );
}

void Frame::setStyle( int i, QStyle *s )
{
    QApplication::setStyle( s );
    menuBar()->setItemChecked( idWindows, FALSE );
    menuBar()->setItemChecked( idMotif, FALSE );
    menuBar()->setItemChecked( idMotifPlus, FALSE );
    menuBar()->setItemChecked( idPlatinum, FALSE );
    menuBar()->setItemChecked( idSGI, FALSE );
    menuBar()->setItemChecked( i, TRUE );
}

void Frame::clickedCategory( QListBoxItem *item )
{
    if ( item ) {
	CategoryItem *c = (CategoryItem*)item;
	topLevelWidget()->setCaption( title + " - " + item->text() );
	stack->raiseWidget( c->widget() );
    }
}

