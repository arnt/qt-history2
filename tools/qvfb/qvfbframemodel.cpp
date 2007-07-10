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

#include "qvfbframemodel.h"
#include <QPixmap>
#include <QAbstractItemDelegate>

QVFbFrameView::QVFbFrameView(QWidget *parent)
: QListView(parent)
{
    setUniformItemSizes(true);

    QObject::connect(this, SIGNAL(pressed(const QModelIndex &)), 
                     this, SLOT(mouseEnter(const QModelIndex &)));
}

void QVFbFrameView::mouseEnter(const QModelIndex &idx)
{
    QVFbFrameModel *fmodel = qobject_cast<QVFbFrameModel *>(model());
    if(fmodel) 
        emit showImage(fmodel->image(idx.row()));
}

QVFbFrameModel::QVFbFrameModel(QObject *parent)
: QAbstractListModel(parent), mMaxFrames(100)
{
}

int QVFbFrameModel::rowCount(const QModelIndex &) const
{
    return images.count();
}

QPixmap QVFbFrameModel::image(int idx) const
{
    return QPixmap(images.at(idx).second);
}

QVariant QVFbFrameModel::data(const QModelIndex &idx, int role) const
{
    if(role == Qt::DecorationRole) 
        return images.at(idx.row()).first;

    return QVariant();
}

void QVFbFrameModel::appendImage(const QImage &image, const QString &name)
{
    if (images.count() >= mMaxFrames) {
        beginRemoveRows(QModelIndex(), 0, 0);
        images.removeFirst();
        endRemoveRows();
    }
    beginInsertRows(QModelIndex(), rowCount(), rowCount() + 1);
    images.append(qMakePair(QPixmap::fromImage(image.scaledToWidth(64)), name));
    endInsertRows();
}

void QVFbFrameModel::clear()
{
    images.clear();
    reset();
}

int QVFbFrameModel::maxFrames() const
{
    return mMaxFrames;
}

void QVFbFrameModel::setMaxFrames(int size)
{
    mMaxFrames = size;
    if (images.count() > mMaxFrames) {
        int count = images.count() - mMaxFrames; 
        beginRemoveRows(QModelIndex(), 0, count);
        while(count--)
            images.removeFirst();
        endRemoveRows();
    }
}
