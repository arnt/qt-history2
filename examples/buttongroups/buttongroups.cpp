/****************************************************************************
** $Id: //depot/qt/main/examples/buttons_groups/buttons_groups.cpp#1 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "buttongroups.h"

#include <qbuttongroup.h>
#include <qhbox.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qpushbutton.h>

/*
 * Constructor
 *
 * Creates all child widgets of the ButtonGroups window
 */

ButtonsGroups::ButtonsGroups( QWidget *parent, const char *name )
    : QVBox( parent, name )
{
    // Create Widgets which allow easy layouting
    QHBox *box1 = new QHBox( this );
    QHBox *box2 = new QHBox( this );

    // ------- first group

    // Create an exclusive button group
    QButtonGroup *grp1 = new QButtonGroup( 1, QGroupBox::Horizontal, "Button Group 1 (exclusive)", box1);
    grp1->setExclusive( TRUE );

    // insert 3 radiobuttons
    QRadioButton *rb11 = new QRadioButton( "&Radiobutton 1", grp1 );
    rb11->setChecked( TRUE );
    (void)new QRadioButton( "R&adiobutton 2", grp1 );
    (void)new QRadioButton( "Ra&diobutton 3", grp1 );

    // ------- second group

    // Create a non-exclusive buttongroup
    QButtonGroup *grp2 = new QButtonGroup( 1, QGroupBox::Horizontal, "Button Group 2 (non-exclusive)", box1 );
    grp2->setExclusive( FALSE );

    // insert 3 checkboxes
    (void)new QCheckBox( "&Checkbox 1", grp2 );
    QCheckBox *cb12 = new QCheckBox( "C&heckbox 2", grp2 );
    cb12->setChecked( TRUE );
    QCheckBox *cb13 = new QCheckBox( "Tripple &State Button", grp2 );
    cb13->setTristate( TRUE );
    cb13->setChecked( TRUE );

    // ------------ third group

    // create a buttongroup which is exclusive for radiobuttons and non-exclusive for all other buttons
    QButtonGroup *grp3 = new QButtonGroup( 1, QGroupBox::Horizontal, "Button Group 3 (Radiobutton-exclusive)", box2 );
    grp3->setRadioButtonExclusive( TRUE );

    // insert three radiobuttons
    rb21 = new QRadioButton( "Rad&iobutton 1", grp3 );
    rb22 = new QRadioButton( "Radi&obutton 2", grp3 );
    rb23 = new QRadioButton( "Radio&button 3", grp3 );
    rb23->setChecked( TRUE );
    
    // insert a checkbox...
    state = new QCheckBox( "E&nable Radiobuttons", grp3 );
    state->setChecked( TRUE );
    // ...and connect its SIGNAL clicked() with the SLOT slotChangeGrp3State() 
    connect( state, SIGNAL( clicked() ), this, SLOT( slotChangeGrp3State() ) );

    // ------------ fourth group

    // create a groupbox which layouts its childs in a columns
    QGroupBox *grp4 = new QButtonGroup( 1, QGroupBox::Horizontal, "Groupbox with normal buttons", box2 );
    
    // insert two pushbuttons...
    (void)new QPushButton( "&Push Button", grp4 );
    QPushButton *tb = new QPushButton( "&Toggle Button", grp4 );
    
    // ... and make the second one a toggle button 
    tb->setToggleButton( TRUE );
    tb->setOn( TRUE );
}

/*
 * SLOT slotChangeGrp3State()
 *
 * enables/disables the radiobuttons of the third buttongroup
 */

void ButtonsGroups::slotChangeGrp3State()
{
    rb21->setEnabled( state->isChecked() );
    rb22->setEnabled( state->isChecked() );
    rb23->setEnabled( state->isChecked() );
}
