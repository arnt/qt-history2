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

#include "listwidget.h"
#include <QWidget>
#include <QListWidget>
#include <QMouseEvent>
#include <QModelIndex>
#include <QMap>
#include <QVariant>
#include <QString>
#include <QStringList>
#include <QDebug>

ListWidget::ListWidget(QWidget *parent)
    : QListWidget(parent), moving(false)
{
}

void ListWidget::mousePressEvent(QMouseEvent *e)
{
    QListWidget::mousePressEvent(e);
    if (e->button() == Qt::LeftButton) {
        QModelIndex index = indexAt(e->pos());
        if (index.isValid()) {
            moving = true;
            movedIndex = index;
        }
    }
}

void ListWidget::swapData(const QModelIndex &from, const QModelIndex &to)
{
    const QMap<int, QVariant> itemDataFrom = model()->itemData(from);
    model()->setItemData(from, model()->itemData(to));
    model()->setItemData(to, itemDataFrom);
    if (currentIndex() == from) {
        setCurrentIndex(to);
    } else if (currentIndex() == to) {
        setCurrentIndex(from);
    }
    emit changed();
}


void ListWidget::mouseMoveEvent(QMouseEvent *e)
{
    QListWidget::mouseMoveEvent(e);
    if (moving) {
        QModelIndex to = indexAt(e->pos());
        if (to.isValid() && to != movedIndex) {
            swapData(movedIndex, to);
            movedIndex = to;
        }
    }
}
void ListWidget::mouseReleaseEvent(QMouseEvent *e)
{
    QListWidget::mouseReleaseEvent(e);
    moving = false;
}
bool ListWidget::isUpEnabled() const
{
    const QModelIndex index = currentIndex();
    return index.isValid() && index.row() > 0;
}
bool ListWidget::isDownEnabled() const
{
    const QModelIndex index = currentIndex();
    return index.isValid() && index.row() + 1 < count();
}
void ListWidget::moveCurrentUp()
{
    const QModelIndex index = currentIndex();
    if (index.row() > 0)
        swapData(index, model()->index(index.row() - 1, 0));
}
void ListWidget::moveCurrentDown()
{
    const QModelIndex index = currentIndex();
    if (index.row() + 1 < count())
        swapData(index, model()->index(index.row() + 1, 0));
}
QStringList ListWidget::items() const
{
    QStringList ret;
    for (int i=0; i<count(); ++i) {
        ret << item(i)->text();
    }
    return ret;
}
void ListWidget::setItems(const QStringList &items)
{
    clear();
    addItems(items);
}
