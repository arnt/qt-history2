//depot/qt/main/examples/buttongroups/buttongroups.cpp#20 - integrate change 112192 (text)
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

#include "buttongroups.h"

#include <qpopupmenu.h>
#include <q3buttongroup.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qpushbutton.h>

/*
 * Constructor
 *
 * Creates all child widgets of the ButtonGroups window
 */

ButtonsGroups::ButtonsGroups(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    // Create Widgets which allow easy layouting
    QVBoxLayout *vbox = new QVBoxLayout(this, 11, 6);
    QHBoxLayout *box1 = new QHBoxLayout(vbox);
    QHBoxLayout *box2 = new QHBoxLayout(vbox);

    // ------- first group

    // Create an exclusive button group
    Q3ButtonGroup *bgrp1 = new Q3ButtonGroup(1, Qt::Horizontal, "Button Group 1 (exclusive)", this);
    box1->addWidget(bgrp1);
    bgrp1->setExclusive(true);

    // insert 3 radiobuttons
    QRadioButton *rb11 = new QRadioButton("&Radiobutton 1", bgrp1);
    rb11->setChecked(true);
    (void)new QRadioButton("R&adiobutton 2", bgrp1);
    (void)new QRadioButton("Ra&diobutton 3", bgrp1);

    // ------- second group

    // Create a non-exclusive buttongroup
    Q3ButtonGroup *bgrp2 = new Q3ButtonGroup(1, Qt::Horizontal, "Button Group 2 (non-exclusive)", this);
    box1->addWidget(bgrp2);
    bgrp2->setExclusive(false);

    // insert 3 checkboxes
    (void)new QCheckBox("&Checkbox 1", bgrp2);
    QCheckBox *cb12 = new QCheckBox("C&heckbox 2", bgrp2);
    cb12->setChecked(true);
    QCheckBox *cb13 = new QCheckBox("Triple &State Button", bgrp2);
    cb13->setTristate(true);
    cb13->setChecked(true);

    // ------------ third group

    // create a buttongroup which is exclusive for radiobuttons and non-exclusive for all other buttons
    Q3ButtonGroup *bgrp3 = new Q3ButtonGroup(1, Qt::Horizontal, "Button Group 3 (Radiobutton-exclusive)", this);
    box2->addWidget(bgrp3);
    bgrp3->setRadioButtonExclusive(true);

    // insert three radiobuttons
    rb21 = new QRadioButton("Rad&iobutton 1", bgrp3);
    rb22 = new QRadioButton("Radi&obutton 2", bgrp3);
    rb23 = new QRadioButton("Radio&button 3", bgrp3);
    rb23->setChecked(true);

    // insert a checkbox...
    state = new QCheckBox("E&nable Radiobuttons", bgrp3);
    state->setChecked(true);
    // ...and connect its SIGNAL clicked() with the SLOT slotChangeGrp3State()
    connect(state, SIGNAL(clicked()), this, SLOT(slotChangeGrp3State()));

    // ------------ fourth group

    // create a groupbox which layouts its childs in a columns
    Q3ButtonGroup *bgrp4 = new Q3ButtonGroup(1, Qt::Horizontal, "Groupbox with normal buttons", this);
    box2->addWidget(bgrp4);

    // insert four pushbuttons...
    new QPushButton("&Push Button", bgrp4);

    // now make the second one a toggle button
    QPushButton *tb2 = new QPushButton("&Toggle Button", bgrp4);
    tb2->setCheckable(true);
    tb2->setChecked(true);

    // ... and make the third one a flat button
    QPushButton *tb3 = new QPushButton("&Flat Button", bgrp4);
    tb3->setFlat(true);

    // .. and the fourth a button with a menu
    QPushButton *tb4 = new QPushButton("Popup Button", bgrp4);
    QPopupMenu *menu = new QPopupMenu(tb4);
    menu->addAction("Item1");
    menu->addAction("Item2");
    menu->addAction("Item3");
    menu->addAction("Item4");
    tb4->setMenu(menu);
}

/*
 * SLOT slotChangeGrp3State()
 *
 * enables/disables the radiobuttons of the third buttongroup
 */

void ButtonsGroups::slotChangeGrp3State()
{
    rb21->setEnabled(state->isChecked());
    rb22->setEnabled(state->isChecked());
    rb23->setEnabled(state->isChecked());
}
