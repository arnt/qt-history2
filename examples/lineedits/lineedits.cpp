/****************************************************************************
** $Id: //depot/qt/main/examples/lineedits/lineedits.cpp#3 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "lineedits.h"

#include <qlineedit.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qvalidator.h>
#include <qlabel.h>
#include <qhbox.h>

/*
 * Constructor
 *
 * Creates child widgets of the LineEdits widget
 */

LineEdits::LineEdits( QWidget *parent, const char *name )
    : QVBox( parent, name )
{
    setMargin( 10 );

    // Widget for layouting
    QHBox *row1 = new QHBox( this );
    row1->setMargin( 5 );

    // Create a Label
    (void)new QLabel( "Echo Mode: ", row1 );

    // Create a Combobox with three items...
    combo1 = new QComboBox( FALSE, row1 );
    combo1->insertItem( "Normal", -1 );
    combo1->insertItem( "Password", -1 );
    combo1->insertItem( "No Echo", -1 );
    // ...and connect the activated() SIGNAL with the slotEchoChanged() SLOT to be able
    // to react when an item is selected
    connect( combo1, SIGNAL( activated( int ) ), this, SLOT( slotEchoChanged( int ) ) );

    // insert the first LineEdit
    lined1 = new QLineEdit( this );

    // another widget which is used for layouting
    QHBox *row2 = new QHBox( this );
    row2->setMargin( 5 );

    // and the second label
    (void)new QLabel( "Validator: ", row2 );

    // A second Combobox with again three items...
    combo2 = new QComboBox( FALSE, row2 );
    combo2->insertItem( "No Validator", -1 );
    combo2->insertItem( "Integer Validator", -1 );
    combo2->insertItem( "Double Validator", -1 );
    // ...and again the activated() SIGNAL gets connected with a SLOT
    connect( combo2, SIGNAL( activated( int ) ), this, SLOT( slotValidatorChanged( int ) ) );

    // and the second LineEdit
    lined2 = new QLineEdit( this );

    // yet another widget which is used for layouting
    QHBox *row3 = new QHBox( this );
    row3->setMargin( 5 );

    // we need a label for this too
    (void)new QLabel( "Alignment: ", row3 );

    // A combo box for setting alignment
    combo3 = new QComboBox( FALSE, row3 );
    combo3->insertItem( "Left", -1 );
    combo3->insertItem( "Centered", -1 );
    combo3->insertItem( "Right", -1 );
    // ...and again the activated() SIGNAL gets connected with a SLOT
    connect( combo3, SIGNAL( activated( int ) ), this, SLOT( slotAlignmentChanged( int ) ) );

    // and the lineedit
    lined3 = new QLineEdit( this );

    // give the first LineEdit the focus at the beginning
    lined1->setFocus();
}

/*
 * SLOT slotEchoChanged( int i )
 *
 * i contains the number of the item which the user has been chosen in the
 * first Combobox. According to this value, we set the Echo-Mode for the
 * first LineEdit.
 */

void LineEdits::slotEchoChanged( int i )
{
    switch ( i ) {
    case 0:
	lined1->setEchoMode( QLineEdit::Normal );
        break;
    case 1:
	lined1->setEchoMode( QLineEdit::Password );
        break;
    case 2:
	lined1->setEchoMode( QLineEdit::NoEcho );
        break;
    }

    lined1->setFocus();
}

/*
 * SLOT slotValidatorChanged( int i )
 *
 * i contains the number of the item which the user has been chosen in the
 * second Combobox. According to this value, we set a validator for the
 * second LineEdit. A validator checks in a LineEdit each character which
 * the user enters and accepts it if it is valid, else the character gets
 * ignored and not inserted into the lineedit.
 */

void LineEdits::slotValidatorChanged( int i )
{
    switch ( i ) {
    case 0:
	lined2->setValidator( 0L );
        break;
    case 1:
	lined2->setValidator( new QIntValidator( lined2 ) );
        break;
    case 2:
	lined2->setValidator( new QDoubleValidator( -999.0, 999.0, 2,
						    lined2 ) );
        break;
    }

    lined2->setText( "" );
    lined2->setFocus();
}


/*
 * SLOT slotAlignmentChanged( int i )
 *
 * i contains the number of the item which the user has been chosen in
 * the second Combobox.  According to this value, we set an alignment
 * third LineEdit.
 */

void LineEdits::slotAlignmentChanged( int i )
{
    switch ( i ) {
    case 0:
	lined3->setAlignment( QLineEdit::AlignLeft );
        break;
    case 1:
	lined3->setAlignment( QLineEdit::AlignCenter );
        break;
    case 2:
	lined3->setAlignment( QLineEdit::AlignRight );
        break;
    }

    lined3->setFocus();
}
