
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

