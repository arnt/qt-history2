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

#include <QtGui>

#include "window.h"

Window::Window(QWidget *parent)
    : QWidget(parent)
{
    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(createFirstExclusiveGroup(), 0, 0);
    grid->addWidget(createSecondExclusiveGroup(), 1, 0);
    grid->addWidget(createNonExclusiveGroup(), 0, 1);
    grid->addWidget(createPushButtonGroup(), 1, 1);

    setWindowTitle(tr("Group Box"));
    resize(480, 320);
}

QGroupBox *Window::createFirstExclusiveGroup()
{
    QGroupBox *groupBox = new QGroupBox(tr("Exclusive Radio Buttons"), this);

    QRadioButton *radio1 = new QRadioButton(tr("&Radio button 1"), groupBox);
    QRadioButton *radio2 = new QRadioButton(tr("R&adio button 2"), groupBox);
    QRadioButton *radio3 = new QRadioButton(tr("Ra&dio button 3"), groupBox);

    radio1->setChecked(true);

    QVBoxLayout *vbox = new QVBoxLayout(groupBox);
    vbox->addWidget(radio1);
    vbox->addWidget(radio2);
    vbox->addWidget(radio3);
    vbox->addStretch(1);

    return groupBox;
}

QGroupBox *Window::createSecondExclusiveGroup()
{
    QGroupBox *groupBox = new QGroupBox(tr("&Exclusive Radio Buttons"), this);
    groupBox->setCheckable(true);
    groupBox->setChecked(false);

    QRadioButton *radio1 = new QRadioButton(tr("Rad&io button 1"), groupBox);
    QRadioButton *radio2 = new QRadioButton(tr("Radi&o button 2"), groupBox);
    QRadioButton *radio3 = new QRadioButton(tr("Radio &button 3"), groupBox);

#if 1
    QButtonGroup *exclusiveGroup = new QButtonGroup;
    exclusiveGroup->addButton(radio1);
    exclusiveGroup->addButton(radio2);
    exclusiveGroup->addButton(radio3);
    radio1->setChecked(true);
#endif

    QVBoxLayout *vbox = new QVBoxLayout(groupBox);
    vbox->addWidget(radio1);
    vbox->addWidget(radio2);
    vbox->addWidget(radio3);
    vbox->addStretch(1);

    return groupBox;
}

QGroupBox *Window::createNonExclusiveGroup()
{
    QGroupBox *groupBox = new QGroupBox(tr("Non-Exclusive Checkboxes"), this);

    QCheckBox *checkbox1 = new QCheckBox(tr("&Checkbox 1"), groupBox);
    QCheckBox *checkbox2 = new QCheckBox(tr("C&heckbox 2"), groupBox);
    QCheckBox *tristateBox = new QCheckBox(tr("Tri-&state button"), groupBox);
    tristateBox->setTristate(true);
    tristateBox->setState(QCheckBox::NoChange);
    
    QVBoxLayout *vbox = new QVBoxLayout(groupBox);
    vbox->addWidget(checkbox1);
    vbox->addWidget(checkbox2);
    vbox->addWidget(tristateBox);
    vbox->addStretch(1);

    return groupBox;
}

QGroupBox *Window::createPushButtonGroup()
{
    QGroupBox *groupBox = new QGroupBox(tr("Push Buttons"), this);

    QPushButton *pushButton = new QPushButton(tr("&Normal Button"), groupBox);
    QPushButton *flatButton = new QPushButton(tr("&Flat Button"), groupBox);
    flatButton->setFlat(true);

    QPushButton *popupButton = new QPushButton(tr("&Popup Button"), groupBox);
    QMenu *menu = new QMenu(this);
    menu->addAction(tr("Item 1"));
    menu->addAction(tr("Item 2"));
    menu->addAction(tr("Item 3"));
    menu->addAction(tr("Item 4"));
    popupButton->setMenu(menu);

    QVBoxLayout *vbox = new QVBoxLayout(groupBox);
    vbox->addWidget(pushButton);
    vbox->addWidget(flatButton);
    vbox->addWidget(popupButton);
    vbox->addStretch(1);

    return groupBox;
}
