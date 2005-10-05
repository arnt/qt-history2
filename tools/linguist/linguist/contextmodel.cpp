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

#include "contextmodel.h"
#include "messagemodel.h"
#include "trwindow.h"

static Qt::SortOrder sSortOrder = Qt::AscendingOrder;
static int sSortColumn = 1;

ContextItem::ContextItem(QString c)
: sortColumn(-1), com(""), ctxt(c)
{
    unfinishedCount = 0;
    dangerCount   = 0;
    obsoleteCount = 0;
}

ContextItem::~ContextItem()
{
    // delete all the message items
    for (int i=0; i<msgItemList.count(); ++i)
    {
        delete msgItemList[i];
    }
}

void ContextItem::appendToComment(const QString& x)
{
    if (!com.isEmpty())
        com += QString("\n\n");
    com += x;
}

MessageItem *ContextItem::messageItem(int i)
{
    if ((i >= msgItemList.count()) || (i < 0))
        return 0;

    return msgItemList[i];
}

bool ContextItem::sortParameters(Qt::SortOrder &so, int &sc) const
{
    if (sortColumn == -1)
        return false;

    so = sortOrder;
    sc = sortColumn;

    return true;
}

void ContextItem::sortMessages(int column, Qt::SortOrder order)
{
    sortOrder = sSortOrder = order;
    sortColumn = sSortColumn = column;

    qSort(msgItemList.begin(), msgItemList.end(), ContextItem::compare);
}

bool ContextItem::compare(const MessageItem *left, const MessageItem *right)
{
    int res, nleft, nright;
    if (sSortColumn == 0) {
        nleft = left->danger() + left->finished() + left->translation().isEmpty();
        nright = right->danger() + right->finished() + right->translation().isEmpty();
        if ((sSortOrder == Qt::AscendingOrder) ? (nleft < nright) : !(nleft < nright))
            return true;
    }
    else if (sSortColumn == 1) {
        res = QString::localeAwareCompare(left->sourceText().remove('&'),
            right->sourceText().remove('&'));
        if ((sSortOrder == Qt::AscendingOrder) ? (res < 0) : !(res < 0))
            return true;
    }
    else if (sSortColumn == 2) {
        res = QString::localeAwareCompare(left->translation().remove('&'),
            right->translation().remove('&'));
        if ((sSortOrder == Qt::AscendingOrder) ? (res < 0) : !(res < 0))
            return true;
    }

    return false;
}

ContextModel::ContextModel(QObject *parent)
: QAbstractTableModel(parent), sortColumn(-1)
{

}

ContextItem *ContextModel::contextItem(const QModelIndex &indx) const
{
    if (indx.isValid())
        return cntxtList.at(indx.row());

    return 0;
}

bool ContextModel::sortParameters(Qt::SortOrder &so, int &sc) const
{
    if (sortColumn == -1)
        return false;

    so = sortOrder;
    sc = sortColumn;

    return true;
}

void ContextModel::updateItem(QModelIndex indx)
{
    QModelIndex strtindx = createIndex(indx.row(), 0);
    QModelIndex endindx = createIndex(indx.row(), 2);

    emit dataChanged(strtindx, endindx);
}

void ContextModel::clearContextList()
{
    int r = cntxtList.count();

    if (r <= 0) // no items
        return;

    for (int i=0; i<r; ++i)
        delete cntxtList[i];

    cntxtList.clear();

    reset();
}

// since we don't add or remove single rows, update all at once...
void ContextModel::updateAll()
{
    reset();
}

int ContextModel::rowCount(const QModelIndex &) const
{
    return cntxtList.count();
}

int ContextModel::columnCount(const QModelIndex &) const
{
    return 3;
}

QVariant ContextModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((role == Qt::DisplayRole) && (orientation == Qt::Horizontal)) {
        switch(section)    {
        case 0:
            return tr("Done");
        case 1:
            return tr("Context");
        case 2:
            return tr("Items");
        }

        return "Error";
    }

    return QVariant();
}

QVariant ContextModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int column = index.column();

    if (row >= cntxtList.count() || !index.isValid())
        return QVariant();

    ContextItem *cntxtItem = cntxtList.at(row);

    if (role == Qt::DisplayRole) {
        switch(column) {
        case 0: // done
            return QVariant();
        case 1: // context
            return cntxtItem->context().simplified();
        case 2: // items
            QString s;
            int itemCount = cntxtItem->messageItemsInList();
            int obsoleteCount = cntxtItem->obsolete();
            int unfinishedCount = cntxtItem->unfinished();
            s.sprintf("%d/%d", itemCount - unfinishedCount - obsoleteCount,
                itemCount - obsoleteCount);
            return s;
        }
    }
    else if ((role == Qt::DecorationRole) && (column == 0)) {
        if (cntxtItem->isContextObsolete())
            return qVariantFromValue(*TrWindow::pxObsolete);
        else if (cntxtItem->finished())
            return qVariantFromValue(*TrWindow::pxOn);
        else
            return qVariantFromValue(*TrWindow::pxOff);
    }

    return QVariant();
}

void ContextModel::sort(int column, Qt::SortOrder order)
{
    if (cntxtList.count() <= 0)
        return;

    sortOrder = sSortOrder = order;
    sortColumn = sSortColumn = column;

    qSort(cntxtList.begin(), cntxtList.end(), ContextModel::compare);
    emit dataChanged(index(0,0), index(cntxtList.count()-1, 2));
}

bool ContextModel::compare(const ContextItem *left, const ContextItem *right)
{
    int res;
    int nleft, nright;
    switch (sSortColumn)
    {
    case 0:
        nleft = left->isContextObsolete() + left->finished();
        nright = right->isContextObsolete() + right->finished();

        if ((sSortOrder == Qt::AscendingOrder) ? (nleft < nright) : !(nleft < nright))
            return true;
        break;
    case 1:
        res = QString::localeAwareCompare(left->context(), right->context());
        if ((sSortOrder == Qt::AscendingOrder) ? (res < 0) : !(res < 0))
            return true;
        break;
    case 2:
        nleft = left->messageItemsInList() - left->unfinished() - left->obsolete();
        nright = right->messageItemsInList() - right->unfinished() - right->obsolete();
        if ((sSortOrder == Qt::AscendingOrder) ? (nleft < nright) : !(nleft < nright))
            return true;
        break;
    }

    return false;
}
