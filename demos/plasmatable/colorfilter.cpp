/****************************************************************************
 **
 ** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
 **
 ** This file is an example program for the Qt SQL module.
 ** EDITIONS: NOLIMITS
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/

#include "colorfilter.h"
#include <qvariant.h>

ColorFilter::ColorFilter(QObject *parent)
    : QProxyModel(parent), colorfilter(0xffffffff)
{
}

ColorFilter::~ColorFilter()
{
}

QVariant ColorFilter::data(const QModelIndex &index, int role) const
{
    if (role == QAbstractItemModel::DisplayRole) {
        unsigned int color = QProxyModel::data(index, role).toInt();
        return (color & colorfilter);
    }
    return QProxyModel::data(index, role);
}
