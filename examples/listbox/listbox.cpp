/****************************************************************************
** $Id: //depot/qt/main/examples/listbox/listbox.cpp#1 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "listbox.h"

#include <qlabel.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qlistbox.h>
#include <qbuttongroup.h>
#include <qlayout.h>



ListBoxDemo::ListBoxDemo()
    : QWidget( 0, 0 )
{
    QGridLayout * g = new QGridLayout( this, 2, 2 );

    g->addWidget( new QLabel( "Configuration:", this ), 0, 0 );
    g->addWidget( new QLabel( "Result:", this ), 0, 1 );

    l = new QListBox( this );
    g->addWidget( l, 1, 1 );

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
    
    v->addStretch( 100 );

    int i = 0;
    while( ++i <= 256 )
	l->insertItem( QString::fromLatin1( "Item " ) + QString::number( i ),
		       i );
    columns->setRange( 1, i );
    rows->setRange( 1, i );

    connect( columns, SIGNAL(valueChanged(int)), this, SLOT(setNumCols()) );
    connect( rows, SIGNAL(valueChanged(int)), this, SLOT(setNumRows()) );
}


ListBoxDemo::~ListBoxDemo()
{
    delete bg;
}


void ListBoxDemo::setNumRows()
{
    columns->setEnabled( TRUE );
    rows->setEnabled( FALSE );
    l->setRowMode( rows->value() );
}


void ListBoxDemo::setNumCols()
{
    columns->setEnabled( FALSE );
    rows->setEnabled( TRUE );
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
