/****************************************************************************
** $Id$
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "main.h"

const char* red_icon[]={
"16 16 2 1",
"r c red",
". c None",
"................",
"................",
"..rrrrrrrrrrrr..",
"..rrrrrrrrrrrr..",
"..rrrrrrrrrrrr..",
"..rrr......rrr..",
"..rrr......rrr..",
"..rrr......rrr..",
"..rrr......rrr..",
"..rrr......rrr..",
"..rrr......rrr..",
"..rrrrrrrrrrrr..",
"..rrrrrrrrrrrr..",
"..rrrrrrrrrrrr..",
"................",
"................"};

const char* blue_icon[]={
"16 16 2 1",
"b c blue",
". c None",
"................",
"................",
"..bbbbbbbbbbbb..",
"..bbbbbbbbbbbb..",
"..bbbbbbbbbbbb..",
"..bbb......bbb..",
"..bbb......bbb..",
"..bbb......bbb..",
"..bbb......bbb..",
"..bbb......bbb..",
"..bbb......bbb..",
"..bbbbbbbbbbbb..",
"..bbbbbbbbbbbb..",
"..bbbbbbbbbbbb..",
"................",
"................"};

const char* green_icon[]={
"16 16 2 1",
"g c green",
". c None",
"................",
"................",
"..gggggggggggg..",
"..gggggggggggg..",
"..gggggggggggg..",
"..ggg......ggg..",
"..ggg......ggg..",
"..ggg......ggg..",
"..ggg......ggg..",
"..ggg......ggg..",
"..ggg......ggg..",
"..gggggggggggg..",
"..gggggggggggg..",
"..gggggggggggg..",
"................",
"................"};


DDListBox::DDListBox( QWidget * parent = 0, const char * name = 0, WFlags f = 0 ) :
    QListBox( parent, name, f )
{
    setAcceptDrops( TRUE );
}


void DDListBox::dragEnterEvent( QDragEnterEvent *evt )
{
    if ( QTextDrag::canDecode( evt ) ) 
	evt->accept();
}


void DDListBox::dropEvent( QDropEvent *evt )
{
    QString text;

    if ( QTextDrag::decode( evt, text ) ) 
	insertItem( text );
}


void DDIconView::startDrag()
{
    QDragObject *d = new QTextDrag( currentItem()->text(), this );
    d->dragCopy(); // do NOT delete d.
}


int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    // Create and show the widgets
    QSplitter *split = new QSplitter();
    DDIconView *iv   = new DDIconView( split );
    (void)	       new DDListBox( split );
    app.setMainWidget( split );
    split->resize( 600, 400 );
    split->show();

    // Populate the QIconView with icons
    (void) new QIconViewItem( iv, "Red",   QPixmap( red_icon ) );
    (void) new QIconViewItem( iv, "Green", QPixmap( green_icon ) );
    (void) new QIconViewItem( iv, "Blue",  QPixmap( blue_icon ) );

    return app.exec();
}


