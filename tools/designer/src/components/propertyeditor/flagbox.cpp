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
#include <QAbstractItemView>

FlagBox::FlagBox(QWidget *parent)
    : QComboBox(parent)
{
    m_model = new FlagBoxModel(this);
    setModel(m_model);

    setAutoHide(false);
    
    itemView()->setSelectionMode(QAbstractItemView::ExtendedSelection);
    
    connect(this, SIGNAL(activated(const QModelIndex&)),
        this, SLOT(slotActivated(const QModelIndex&)));
}

FlagBox::~FlagBox()
{
}

void FlagBox::slotActivated(const QModelIndex &index)
{
    bool checked = model()->data(index, FlagBoxModel::DecorationRole).toBool();
    model()->setData(index, !checked, FlagBoxModel::DecorationRole);
}
