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

#ifndef IMAGEMODEL_H
#define IMAGEMODEL_H

#include <qabstractitemmodel.h>
#include <qvector.h>
#include <qimage.h>

class ImageModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ImageModel(const QString &fileName, QObject *parent = 0);
    ~ImageModel();

    int rowCount() const;
    int columnCount() const;

    QVariant data(const QModelIndex &index, int role = QAbstractItemModel::DisplayRole) const;
    bool setData(const QModelIndex &index, int role, const QVariant &value);

    QAbstractItemModel::ItemFlags flags(const QModelIndex &index) const;

protected:
    QImage image;
};

#endif // IMAGEMODEL_H
