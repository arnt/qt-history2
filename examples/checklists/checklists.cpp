/****************************************************************************
** $Id: //depot/qt/main/examples/checklists/checklists.cpp#1 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "checklists.h"

#include <qlistview.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qlist.h>
#include <qstring.h>
#include <qpushbutton.h>

/*
 * Constructor
 * 
 * Create all child widgets of the CheckList Widget
 */
 
CheckLists::CheckLists( QWidget *parent, const char *name )
    : QHBox( parent, name )
{
    setMargin( 5 );

    // create a widget which layouts its childs in a column
    QVBox *vbox1 = new QVBox( this );
    vbox1->setMargin( 5 );

    // First child: a Label
    (void)new QLabel( "Check some items!", vbox1 );

    // Second child: the ListView
    lv1 = new QListView( vbox1 );
    lv1->addColumn( "Items" );
    lv1->setRootIsDecorated( TRUE );

    // create a list with 4 ListViewItems which will be parent items of other ListViewItems
    QList<QListViewItem> parentList;
    parentList.setAutoDelete( FALSE );

    parentList.append( new QListViewItem( lv1, "Parent Item 1" ) );
    parentList.append( new QListViewItem( lv1, "Parent Item 2" ) );
    parentList.append( new QListViewItem( lv1, "Parent Item 3" ) );
    parentList.append( new QListViewItem( lv1, "Parent Item 4" ) );

    QListViewItem *item = 0L;
    unsigned int num;
    // go through the list of parent items...
    for ( item = parentList.first(), num = 1; item; item->setOpen( TRUE ), item = parentList.next(), num++ )
        // ...and create 5 checkable child ListViewItems for each parent item
        for ( unsigned int i = 1; i <= 5; i++ )
            (void)new QCheckListItem( item, QString( "%1. Child of Parent %2" ).arg( i ).arg( num ), QCheckListItem::CheckBox );

    // Create another widget for layouting
    QVBox *tmp = new QVBox( this );
    tmp->setMargin( 5 );
    
    // create a pushbutton
    QPushButton *copy1 = new QPushButton( "  ->  ", tmp );
    copy1->setMaximumWidth( copy1->sizeHint().width() );
    tmp->setMaximumWidth( copy1->maximumWidth() + 10 );
    // connect the SIGNAL clicked() of the pushbutton with the SLOT copy1to2()
    connect( copy1, SIGNAL( clicked() ), this, SLOT( copy1to2() ) );

    // another widget for layouting
    QVBox *vbox2 = new QVBox( this );
    vbox2->setMargin( 5 );

    // and another label
    (void)new QLabel( "Check one item!", vbox2 );

    // create the second listview
    lv2 = new QListView( vbox2 );
    lv2->addColumn( "Items" );
    lv2->setRootIsDecorated( TRUE );

    // another widget needed for layouting only
    tmp = new QVBox( this );
    tmp->setMargin( 5 );
    
    // create another pushbutton...
    QPushButton *copy2 = new QPushButton( "  ->  ", tmp );
    copy2->setMaximumWidth( copy2->sizeHint().width() );
    tmp->setMaximumWidth( copy2->maximumWidth() + 10 );
    // ...and connect its clicked() SIGNAL to the copy2to3() SLOT
    connect( copy2, SIGNAL( clicked() ), this, SLOT( copy2to3() ) );

    tmp = new QVBox( this );
    tmp->setMargin( 5 );

    // and create a label which will be at the right of the window
    label = new QLabel( "No Item yet...", tmp );
}

/*
 * SLOT copy1to2()
 *
 * Copies all checked ListViewItems from the first ListView to
 * the second one, and inserts them as Radio-ListViewItem.
 */

void CheckLists::copy1to2()
{
    // create an iterator which operates on the first ListView
    QListViewItemIterator it( lv1 );

    lv2->clear();
    
    // Insert first a controller Item into the second ListView. Always if Radio-ListViewItems
    // are inserted into a Listview, the parent item of these MUST be a controller Item!
    QCheckListItem *item = new QCheckListItem( lv2, "Controller", QCheckListItem::Controller );
    item->setOpen( TRUE );

    // iterate through the first ListView...
    for ( ; it.current(); ++it )
        // ...check state of childs, and...
        if ( it.current()->parent() )
            // ...if the item is checked...
            if ( ( (QCheckListItem*)it.current() )->isOn() )
                // ...insert a Radio-ListViewItem with the same text into the second ListView
                (void)new QCheckListItem( item, it.current()->text( 0 ), QCheckListItem::RadioButton );

    if ( item->firstChild() )
        ( ( QCheckListItem* )item->firstChild() )->setOn( TRUE );
}

/*
 * SLOT copy2to3()
 *
 * Copies the checked item of the second ListView into the
 * Label at the right.
 */

void CheckLists::copy2to3()
{
    // create an iterator which operates on the second ListView
    QListViewItemIterator it( lv2 );

    label->setText( "No Item checked" );

    // iterate through the second ListView...
    for ( ; it.current(); ++it )
        // ...check state of childs, and...
        if ( it.current()->parent() )
            // ...if the item is checked...
            if ( ( (QCheckListItem*)it.current() )->isOn() )
                // ...set the text of the item to the label 
                label->setText( it.current()->text( 0 ) );
}

