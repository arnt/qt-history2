/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "listbox.h"

#include <qlabel.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qlistbox.h>
#include <qbuttongroup.h>
#include <qlayout.h>
#include <qpushbutton.h>


ListBoxDemo::ListBoxDemo()
    : QWidget( 0, 0 )
{
    QGridLayout * g = new QGridLayout( this, 2, 2, 6 );

    g->addWidget( new QLabel( "<b>Configuration:</b>", this ), 0, 0 );
    g->addWidget( new QLabel( "<b>Result:</b>", this ), 0, 1 );

    l = new QListBox( this );
    g->addWidget( l, 1, 1 );
    l->setFocusPolicy( QWidget::StrongFocus );

    QVBoxLayout * v = new QVBoxLayout;
    g->addLayout( v, 1, 0 );

    QRadioButton * b;
    bg = new QButtonGroup( 0 );

    b = new QRadioButton( "Fixed number of columns,\n"
                          "as many rows as needed.",
                          this );
    bg->insert( b );
    v->addWidget( b );
    b->setChecked( TRUE );
    connect( b, SIGNAL(clicked()), this, SLOT(setNumCols()) );
    QHBoxLayout * h = new QHBoxLayout;
    v->addLayout( h );
    h->addSpacing( 30 );
    h->addSpacing( 100 );
    h->addWidget( new QLabel( "Columns:", this ) );
    columns = new QSpinBox( this );
    h->addWidget( columns );

    v->addSpacing( 12 );

    b = new QRadioButton( "As many columns as fit on-screen,\n"
                          "as many rows as needed.",
                          this );
    bg->insert( b );
    v->addWidget( b );
    connect( b, SIGNAL(clicked()), this, SLOT(setColsByWidth()) );

    v->addSpacing( 12 );

    b = new QRadioButton( "Fixed number of rows,\n"
                          "as many columns as needed.",
                          this );
    bg->insert( b );
    v->addWidget( b );
    connect( b, SIGNAL(clicked()), this, SLOT(setNumRows()) );
    h = new QHBoxLayout;
    v->addLayout( h );
    h->addSpacing( 30 );
    h->addSpacing( 100 );
    h->addWidget( new QLabel( "Rows:", this ) );
    rows = new QSpinBox( this );
    rows->setEnabled( FALSE );
    h->addWidget( rows );

    v->addSpacing( 12 );

    b = new QRadioButton( "As many rows as fit on-screen,\n"
                          "as many columns as needed.",
                          this );
    bg->insert( b );
    v->addWidget( b );
    connect( b, SIGNAL(clicked()), this, SLOT(setRowsByHeight()) );

    v->addSpacing( 12 );

    QCheckBox * cb = new QCheckBox( "Variable-height rows", this );
    cb->setChecked( TRUE );
    connect( cb, SIGNAL(toggled(bool)), this, SLOT(setVariableHeight(bool)) );
    v->addWidget( cb );
    v->addSpacing( 6 );

    cb = new QCheckBox( "Variable-width columns", this );
    connect( cb, SIGNAL(toggled(bool)), this, SLOT(setVariableWidth(bool)) );
    v->addWidget( cb );

    cb = new QCheckBox( "Multiselection", this );
    connect( cb, SIGNAL(toggled(bool)), this, SLOT(setMultiSelection(bool)) );
    v->addWidget( cb );

    QPushButton *pb = new QPushButton( "Sort ascending", this );
    connect( pb, SIGNAL( clicked() ), this, SLOT( sortAscending() ) );
    v->addWidget( pb );

    pb = new QPushButton( "Sort descending", this );
    connect( pb, SIGNAL( clicked() ), this, SLOT( sortDescending() ) );
    v->addWidget( pb );

    v->addStretch( 100 );

    int i = 0;
    while( ++i <= 256 )
        l->insertItem( QString::fromLatin1( "Item " ) + QString::number( i ),
                       i );
    columns->setRange( 1, 256 );
    columns->setValue( 1 );
    rows->setRange( 1, 256 );
    rows->setValue( 256 );

    connect( columns, SIGNAL(valueChanged(int)), this, SLOT(setNumCols()) );
    connect( rows, SIGNAL(valueChanged(int)), this, SLOT(setNumRows()) );

//     connect( l, SIGNAL( highlighted( int ) ),
// 	     this, SLOT( highlighted( int ) ) );

//     connect( l, SIGNAL( selected( int ) ),
// 	     this, SLOT( selected( int ) ) );
    
    connect( l, SIGNAL( highlighted( const QString & ) ),
	     this, SLOT( highlighted( const QString & ) ) );
    
    connect( l, SIGNAL( selected( const QString & ) ),
	     this, SLOT( selected( const QString & ) ) );
    
//     connect( l, SIGNAL( highlighted( QListBoxItem * ) ),
// 	     this, SLOT( highlighted( QListBoxItem * ) ) );
    
//     connect( l, SIGNAL( selected( QListBoxItem * ) ),
// 	     this, SLOT( selected( QListBoxItem * ) ) );
    
    connect( l, SIGNAL( selectionChanged() ),
	     this, SLOT( selectionChanged() ) );
    
    connect( l, SIGNAL( selectionChanged( QListBoxItem * ) ),
	     this, SLOT( selectionChanged( QListBoxItem * ) ) );
    
    connect( l, SIGNAL( currentChanged( QListBoxItem * ) ),
	     this, SLOT( currentChanged( QListBoxItem * ) ) );
    
//     connect( l, SIGNAL( clicked( QListBoxItem * ) ),
// 	     this, SLOT( clicked( QListBoxItem * ) ) );
    
//     connect( l, SIGNAL( pressed( QListBoxItem * ) ),
// 	     this, SLOT( pressed( QListBoxItem * ) ) );
    
//     connect( l, SIGNAL( doubleClicked( QListBoxItem * ) ),
// 	     this, SLOT( doubleClicked( QListBoxItem * ) ) );
    
//     connect( l, SIGNAL( returnPressed( QListBoxItem * ) ),
// 	     this, SLOT( returnPressed( QListBoxItem * ) ) );
    
//     connect( l, SIGNAL( rightButtonClicked( QListBoxItem *, const QPoint & ) ),
// 	     this, SLOT( rightButtonClicked( QListBoxItem *, const QPoint & ) ) );
    
//     connect( l, SIGNAL( rightButtonPressed( QListBoxItem *, const QPoint & ) ),
// 	     this, SLOT( rightButtonPressed( QListBoxItem *, const QPoint & ) ) );
    
//     connect( l, SIGNAL( mouseButtonClicked( int, QListBoxItem *, const QPoint & ) ),
// 	     this, SLOT( mouseButtonClicked( int, QListBoxItem *, const QPoint & ) ) );
    
//     connect( l, SIGNAL( mouseButtonPressed( int, QListBoxItem *, const QPoint & ) ),
// 	     this, SLOT( mouseButtonPressed( int, QListBoxItem *, const QPoint & ) ) );

//     connect( l, SIGNAL( onItem( QListBoxItem * ) ),
// 	     this, SLOT( onItem( QListBoxItem * ) ) );

//     connect( l, SIGNAL( onViewport() ),
// 	     this, SLOT( onViewport() ) );
}


ListBoxDemo::~ListBoxDemo()
{
    delete bg;
}


void ListBoxDemo::setNumRows()
{
    columns->setEnabled( FALSE );
    rows->setEnabled( TRUE );
    l->setRowMode( rows->value() );
}


void ListBoxDemo::setNumCols()
{
    columns->setEnabled( TRUE );
    rows->setEnabled( FALSE );
    l->setColumnMode( columns->value() );
}


void ListBoxDemo::setRowsByHeight()
{
    columns->setEnabled( FALSE );
    rows->setEnabled( FALSE );
    l->setRowMode( QListBox::FitToHeight );
}


void ListBoxDemo::setColsByWidth()
{
    columns->setEnabled( FALSE );
    rows->setEnabled( FALSE );
    l->setColumnMode( QListBox::FitToWidth );
}


void ListBoxDemo::setVariableWidth( bool b )
{
    l->setVariableWidth( b );
}


void ListBoxDemo::setVariableHeight( bool b )
{
    l->setVariableHeight( b );
}

void ListBoxDemo::setMultiSelection( bool b )
{
    l->clearSelection();
    l->setSelectionMode( b ? QListBox::Extended : QListBox::NoSelection );
}

void ListBoxDemo::sortAscending()
{
    l->setSelected( 5, TRUE );
    //l->sort( TRUE );
}

void ListBoxDemo::sortDescending()
{
    //l->setCurrentItem( 3 );
    //l->sort( FALSE );
    delete l->firstItem();
}
