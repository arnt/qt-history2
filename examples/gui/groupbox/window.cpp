/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "window.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QMenu>
#include <QPushButton>
#include <QPoint>
#include <QRadioButton>
#include <QVBoxLayout>

/*
    Set up a main window and populate it with four group boxes:
    an exclusive radio button group, a non-exclusive checkbox group,
    an exclusive radio button group with an enabling checkbox, and
    a group box with normal push buttons.
*/

Window::Window(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(tr("Qt Example - Group Boxes"));

    QGridLayout *gridLayout = new QGridLayout(this);

    gridLayout->addWidget(firstExclusiveGroup(), 0, 0);
    gridLayout->addWidget(nonExclusiveGroup(), 0, 1);
    gridLayout->addWidget(secondExclusiveGroup(), 1, 0);
    gridLayout->addWidget(pushButtonGroup(), 1, 1);
}

/*
    Creates a group box to display radio buttons in, and a button group
    to manage them. The button group is exclusive by default, so only
    one radio button can be checked at any given time. We check the
    first radio button to ensure that the button group contains one
    checked button.

    We use a vertical layout within the group box to present the
    buttons in the form of a vertical list.
*/

QGroupBox *Window::firstExclusiveGroup()
{
    QGroupBox *groupBox = new QGroupBox(tr("Exclusive radio buttons"), this);
    QVBoxLayout *groupLayout = new QVBoxLayout(groupBox);

    QRadioButton *radio1 = new QRadioButton(tr("&Radio button 1"), groupBox);
    QRadioButton *radio2 = new QRadioButton(tr("R&adio button 2"), groupBox);
    QRadioButton *radio3 = new QRadioButton(tr("Ra&dio button 3"), groupBox);
    
    QButtonGroup *exclusiveGroup = new QButtonGroup;
    exclusiveGroup->addButton(radio1);
    exclusiveGroup->addButton(radio2);
    exclusiveGroup->addButton(radio3);
    //exclusiveGroup->setExclusive(true);
    radio1->setChecked(true);

    groupLayout->addWidget(radio1);
    groupLayout->addWidget(radio2);
    groupLayout->addWidget(radio3);
    groupLayout->addStretch(1);

    return groupBox;
}

/*
    Creates a group box to display checkboxes in, and a button group to
    manage them. The button group is configured to be non-exclusive, so
    that each checkbox can be checked independently of the others.

    We use a vertical layout within the group box to present the
    buttons in the form of a vertical list.
*/

QGroupBox *Window::nonExclusiveGroup()
{
    QGroupBox *groupBox = new QGroupBox(tr("Non-exclusive checkboxes"), this);
    QVBoxLayout *groupLayout = new QVBoxLayout(groupBox);

    QCheckBox *checkbox1 = new QCheckBox(tr("&Checkbox 1"), groupBox);
    QCheckBox *checkbox2 = new QCheckBox(tr("C&heckbox 2"), groupBox);
    QCheckBox *tristateBox = new QCheckBox(tr("Triple &state button"), groupBox);
    tristateBox->setTristate(true);
    
    QButtonGroup *nonExclusiveGroup = new QButtonGroup;
    nonExclusiveGroup->addButton(checkbox1);
    nonExclusiveGroup->addButton(checkbox2);
    nonExclusiveGroup->addButton(tristateBox);
    nonExclusiveGroup->setExclusive(false);

    groupLayout->addWidget(checkbox1);
    groupLayout->addWidget(checkbox2);
    groupLayout->addWidget(tristateBox);
    groupLayout->addStretch(1);

    return groupBox;
}

/*
    Creates an exclusive button group and display radio buttons in a
    group box. The group box is made checkable, and is set to be initially
    unchecked - this means that the group box itself must be checked
    before any of the radio buttons inside can be checked.
*/

QGroupBox *Window::secondExclusiveGroup()
{
    QGroupBox *groupBox = new QGroupBox(tr("&Exclusive radio buttons"), this);
    groupBox->setCheckable(true);
    groupBox->setChecked(false);
    QVBoxLayout *groupLayout = new QVBoxLayout(groupBox);

    QRadioButton *radio1 = new QRadioButton(tr("Rad&io button 1"), groupBox);
    QRadioButton *radio2 = new QRadioButton(tr("Radi&o button 2"), groupBox);
    QRadioButton *radio3 = new QRadioButton(tr("Radio &button 3"), groupBox);
    
    QButtonGroup *exclusiveGroup = new QButtonGroup;
    exclusiveGroup->addButton(radio1);
    exclusiveGroup->addButton(radio2);
    exclusiveGroup->addButton(radio3);
    radio1->setChecked(true);

    groupLayout->addWidget(radio1);
    groupLayout->addWidget(radio2);
    groupLayout->addWidget(radio3);
    groupLayout->addStretch(1);

    return groupBox;
}

/*
    Creates a group box without a button group to manage its contents.

    We create three push buttons: a normal button, a flat button, and
    a button that pops up a menu when clicked.
*/

QGroupBox *Window::pushButtonGroup()
{
    QGroupBox *groupBox = new QGroupBox(tr("Push buttons"), this);
    QVBoxLayout *groupLayout = new QVBoxLayout(groupBox);

    QPushButton *pushButton = new QPushButton(tr("&Push button"), groupBox);
    QPushButton *flatButton = new QPushButton(tr("&Flat button"), groupBox);
    flatButton->setFlat(true);

    QPushButton *popupButton = new QPushButton(tr("Popup button"), groupBox);
    QMenu *menu = new QMenu(this);
    menu->addAction(tr("Item 1"));
    menu->addAction(tr("Item 2"));
    menu->addAction(tr("Item 3"));
    menu->addAction(tr("Item 4"));
    popupButton->setMenu(menu);

    groupLayout->addWidget(pushButton);
    groupLayout->addWidget(flatButton);
    groupLayout->addWidget(popupButton);
    groupLayout->addStretch(1);

    return groupBox;
}
