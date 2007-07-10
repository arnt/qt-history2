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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//


#ifndef TREEWIDGET_H
#define TREEWIDGET_H

#include "shared_global_p.h"
#include <QtGui/QTreeWidget>
#include <QtGui/QItemDelegate>

namespace qdesigner_internal {

// -------------- TreeWidgetDelegate to be used for tree widgets with alternating row colors
class QDESIGNER_SHARED_EXPORT TreeWidgetDelegate: public QItemDelegate
{
    Q_OBJECT
public:
    explicit TreeWidgetDelegate(QObject *parent);
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

//  Tree view with alternating row colors as used for the object inspector
class QDESIGNER_SHARED_EXPORT TreeView: public QTreeView
{
    Q_OBJECT
public:
    explicit TreeView(QWidget *parent = 0);
    explicit TreeView(TreeWidgetDelegate *delegate, QWidget *parent = 0);

protected:
    virtual void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const;
};

//  Tree widget with alternating row colors as used for the object inspector
class QDESIGNER_SHARED_EXPORT TreeWidget: public QTreeWidget
{
    Q_OBJECT
public:
    explicit TreeWidget(QWidget *parent = 0);
    explicit TreeWidget(TreeWidgetDelegate *delegate, QWidget *parent = 0);

protected:
    virtual void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const;
};

} // namespace qdesigner_internal

#endif // TREEWIDGET_H
