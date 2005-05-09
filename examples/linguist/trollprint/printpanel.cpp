/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
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

#include "printpanel.h"

PrintPanel::PrintPanel(QWidget *parent)
    : QWidget(parent)
{
/*
    QLabel *label = new QLabel(tr("<b>TROLL PRINT</b>"));
    label->setAlignment(Qt::AlignCenter);
*/

    twoSidedGroupBox = new QGroupBox(tr("2-sided"));
    twoSidedEnabledRadio = new QRadioButton(tr("Enabled"));
    twoSidedDisabledRadio = new QRadioButton(tr("Disabled"));
    twoSidedDisabledRadio->setChecked(true);

    colorsGroupBox = new QGroupBox(tr("Colors"));
    colorsEnabledRadio = new QRadioButton(tr("Enabled"));
    colorsDisabledRadio = new QRadioButton(tr("Disabled"));
    colorsDisabledRadio->setChecked(true);

    QHBoxLayout *twoSidedLayout = new QHBoxLayout;
    twoSidedLayout->addWidget(twoSidedEnabledRadio);
    twoSidedLayout->addWidget(twoSidedDisabledRadio);
    twoSidedGroupBox->setLayout(twoSidedLayout);

    QHBoxLayout *colorsLayout = new QHBoxLayout;
    colorsLayout->addWidget(colorsEnabledRadio);
    colorsLayout->addWidget(colorsDisabledRadio);
    colorsGroupBox->setLayout(colorsLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
/*
    mainLayout->addWidget(label);
*/
    mainLayout->addWidget(twoSidedGroupBox);
    mainLayout->addWidget(colorsGroupBox);
    setLayout(mainLayout);
}
