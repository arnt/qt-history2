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

#include "phrasemodel.h"

static Qt::SortOrder sSortOrder = Qt::Ascending;
static int sSortColumn = 1;

void PhraseModel::removePhrases()
{
    int r = plist.count();
    if (r > 0) {
        emit rowsRemoved(QModelIndex::Null, 0, r-1);
        plist.clear();
    }
}

Phrase PhraseModel::phrase(const QModelIndex &index) const
{
    return plist.at(index.row());
}

void PhraseModel::setPhrase(const QModelIndex &indx, Phrase ph)
{
    int r = indx.row();
    
    plist[r] = ph;

    // update item in view
    QModelIndex si = QAbstractTableModel::index(r, 0);
    QModelIndex ei = QAbstractTableModel::index(r, 2);
    emit dataChanged(si, ei);
}

QModelIndex PhraseModel::addPhrase(Phrase p)
{
    int r = plist.count();
    
    plist.append(p);
    
    // update phrases as we add them
    emit rowsInserted(QModelIndex::Null, r, r);
    return QAbstractTableModel::index(r, 0);
}

void PhraseModel::removePhrase(const QModelIndex &index)
{
    int r = index.row();
    emit rowsRemoved(QModelIndex::Null, r, r);
    plist.removeAt(r);
}

bool PhraseModel::sortParameters(Qt::SortOrder &so, int &sc) const
{
    if (sortColumn == -1)
        return false;
        
    so = sortOrder;
    sc = sortColumn;

    return true;
}

void PhraseModel::resort()
{
    if (sortColumn == -1)
        return;

    sort(sortColumn, QModelIndex::Null, sortOrder);
}

QModelIndex PhraseModel::index(const Phrase phr) const
{
    int row;
    if ((row = plist.indexOf(phr)) == -1)
        return QModelIndex();

    return QAbstractTableModel::index(row,0);
}

int PhraseModel::rowCount() const
{
    return plist.count();
}

int PhraseModel::columnCount() const
{
    return 3;
}

QVariant PhraseModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((role == QAbstractItemModel::DisplayRole) && (orientation == Qt::Horizontal)) {
        switch(section) {
        case 0:
            return tr("Source phrase");
        case 1:
            return tr("Translation");
        case 2:
            return tr("Definition");
        }
    }

    return QString::null;
}

QVariant PhraseModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int column = index.column();

    if (row >= plist.count() || !index.isValid())
        return QString::null;

    Phrase phrase = plist.at(row);

    if (role == QAbstractItemModel::DisplayRole) {
        switch(column) {
        case 0: // source phrase
            return phrase.source().simplified();
        case 1: // translation
            return phrase.target().simplified();
        case 2: // definition
            return phrase.definition();
        }
    }

    return QString::null;
}

void PhraseModel::sort(int column, const QModelIndex &parent, Qt::SortOrder order)
{
    if (plist.count() <= 0)
        return;

    sortOrder = sSortOrder = order;
    sortColumn = sSortColumn = column;

    qHeapSort(plist.begin(), plist.end(), PhraseModel::compare);
    emit dataChanged(QAbstractTableModel::index(0,0), 
        QAbstractTableModel::index(plist.count()-1, 2));

    Q_UNUSED(parent);
}

bool PhraseModel::compare(const Phrase left, const Phrase right)
{
    int res;
    switch (sSortColumn) {
    case 0:
        res = QString::localeAwareCompare(left.source(), right.source());
        if ((sSortOrder == Qt::Ascending) ? (res < 0) : !(res < 0))
            return true;
        break;
    case 1:
        res = QString::localeAwareCompare(left.target(), right.target());
        if ((sSortOrder == Qt::Ascending) ? (res < 0) : !(res < 0))
            return true;
        break;
    case 2:
        // handle the shortcuts when sorting
        if ((left.shortcut() != -1) && (right.shortcut() == -1))
            return (sSortOrder == Qt::Ascending);
        else if ((left.shortcut() == -1) && (right.shortcut() != -1))
            return (sSortOrder != Qt::Ascending);
        res = QString::localeAwareCompare(left.definition(), right.definition());
        if ((sSortOrder == Qt::Ascending) ? (res < 0) : !(res < 0))
            return true;
        break;
    }

    return false;
}
