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

#include "tree_widget.h"

#include <QtCore/QPair>
#include <QtCore/QStack>

#include <QtGui/QApplication>
#include <QtGui/QHeaderView>
#include <QtGui/QScrollBar>
#include <QtGui/QItemDelegate>
#include <QtGui/QPainter>
#include <QtGui/QStyle>

class TreeWidgetDelegate: public QItemDelegate
{
public:
    TreeWidgetDelegate(TreeWidget *treeWidget)
        : QItemDelegate(treeWidget) {}


    virtual void paint(QPainter *painter, const QStyleOptionViewItem &opt,
                        const QModelIndex &index) const
    {
        QStyleOptionViewItem option = opt;

        option.state &= ~(QStyle::State_Selected | QStyle::State_HasFocus);

        if (opt.state & QStyle::State_Selected)
            painter->fillRect(option.rect, QColor(230, 230, 230));

        painter->drawLine(option.rect.x(), option.rect.bottom(),
                            option.rect.right(), option.rect.bottom());

        painter->drawLine(option.rect.right(), option.rect.y(),
                            option.rect.right(), option.rect.bottom());

        QItemDelegate::paint(painter, option, index);
    }

    virtual QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const
    {
        QStyleOptionViewItem option = opt;

        option.state &= ~(QStyle::State_Selected | QStyle::State_HasFocus);

        return QItemDelegate::sizeHint(option, index) + QSize(4,4);
    }
};


TreeWidget::TreeWidget(QWidget *parent)
    : QTreeWidget(parent)
{
    setItemDelegate(new TreeWidgetDelegate(this));

    setAlternatingRowColors(true);
}

TreeWidget::~TreeWidget()
{
}

void TreeWidget::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = viewOptions();
    QStyleOptionViewItem option = opt;

    if (selectionModel()->isSelected(index))
        painter->fillRect(rect, QColor(230, 230, 230));

    painter->drawLine(rect.x(), rect.bottom(),
                      rect.right(), rect.bottom());

    if (model()->hasChildren(index)) {
        static const int size = 9;
        opt.state |= QStyle::State_Children;
        opt.rect.setRect(rect.width() - (indentation() + size) / 2,
                         rect.y() + (rect.height() - size) / 2, size, size);
        if (isExpanded(index))
            opt.state |= QStyle::State_Open;
        style()->drawPrimitive(QStyle::PE_IndicatorBranch, &opt, painter, this);
    }
}
