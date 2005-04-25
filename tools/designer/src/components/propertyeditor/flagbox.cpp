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
#include <QtGui/QItemDelegate>

#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

FlagBox::FlagBox(QWidget *parent)
    : QComboBox(parent)
{
    m_model = new FlagBoxModel(this);
    setModel(m_model);

#ifdef Q_WS_MAC
    setItemDelegate(new QItemDelegate(this));
#endif

    connect(this, SIGNAL(activated(int)), this, SLOT(slotActivated(int)));
}

FlagBox::~FlagBox()
{
}

void FlagBox::slotActivated(int index)
{
    bool checked = itemData(index, Qt::CheckStateRole).toBool();

    setItemData(index, !checked, Qt::CheckStateRole);
}
