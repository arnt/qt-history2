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

#include "flagbox_p.h"
#include <QtGui/QAbstractItemView>
#include <QtCore/qdebug.h>

FlagBox::FlagBox(QWidget *parent)
    : QComboBox(parent)
{
    m_model = new FlagBoxModel(this);
    setModel(m_model);

    connect(this, SIGNAL(activated(int)), this, SLOT(slotActivated(int)));
}

FlagBox::~FlagBox()
{
}

void FlagBox::slotActivated(int index)
{
    bool checked = itemData(index, FlagBoxModel::DecorationRole).toBool();

    setItemData(index, !checked, FlagBoxModel::DecorationRole);
}
