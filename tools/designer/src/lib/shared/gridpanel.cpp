/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "gridpanel_p.h"
#include "grid_p.h"

#include <QtGui/QSpinBox>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>

// Add a row consisting of widget1, widget2 and a description label to a grid.
static void addGridRow(const QString &description, QGridLayout *gridLayout, QWidget *w1,  QWidget *w2, int &row) {
    QLabel *label(new QLabel(description));
    label->setBuddy(w1);
    gridLayout->addWidget(label, row, 0);
    gridLayout->addWidget(w1, row, 1);
    gridLayout->addWidget(w2, row, 2, Qt::AlignRight);
    ++row;
}

static QSpinBox *createDeltaSpinBox()
{
    QSpinBox * rc = new QSpinBox;
    rc->setMaximum (100);
    rc->setMinimum (2);
    return rc;
}

namespace qdesigner_internal {

GridPanel::GridPanel(QWidget *parentWidget) :
    QGroupBox(parentWidget),
    m_visibleCheckBox(new QCheckBox(tr("Visible"))),
    m_snapXCheckBox(new QCheckBox(tr("Snap"))),
    m_snapYCheckBox(new QCheckBox(tr("Snap"))),
    m_deltaXSpinBox(createDeltaSpinBox()),
    m_deltaYSpinBox(createDeltaSpinBox())
{
    QGridLayout *gridLayout = new QGridLayout(this);
    int row = 0;

    gridLayout->addWidget(m_visibleCheckBox, row, 0, 1, 3);
    QPushButton *resetButton  = new QPushButton(tr("Reset"));
    connect(resetButton, SIGNAL(clicked()), this, SLOT(reset()));
    gridLayout->addWidget(resetButton, row, 2,  Qt::AlignRight);

    row++;

    addGridRow(tr("Grid-&X:"), gridLayout, m_deltaXSpinBox, m_snapXCheckBox, row);
    addGridRow(tr("Grid-&Y:"), gridLayout, m_deltaYSpinBox, m_snapYCheckBox, row);
}

void GridPanel::setGrid(const Grid &g)
{
    m_deltaXSpinBox->setValue(g.deltaX());
    m_deltaYSpinBox->setValue(g.deltaY());
    m_visibleCheckBox->setCheckState(g.visible() ? Qt::Checked : Qt::Unchecked);
    m_snapXCheckBox->setCheckState(g.snapX()  ? Qt::Checked : Qt::Unchecked);
    m_snapYCheckBox->setCheckState(g.snapY()  ? Qt::Checked : Qt::Unchecked);
}

Grid GridPanel::grid() const
{
    Grid rc;
    rc.setDeltaX(m_deltaXSpinBox->value());
    rc.setDeltaY(m_deltaYSpinBox->value());
    rc.setSnapX(m_snapXCheckBox->checkState() == Qt::Checked);
    rc.setSnapY(m_snapYCheckBox->checkState() == Qt::Checked);
    rc.setVisible(m_visibleCheckBox->checkState() == Qt::Checked);
    return rc;
}

void GridPanel::reset()
{
    setGrid(Grid());
}
}
