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

#include "treewidget.h"

#include <QApplication>
#include <QHeaderView>
#include <QStack>
#include <QScrollBar>
#include <QItemDelegate>
#include <QPair>
#include <QPainter>

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
    setOddRowColor(QColor(250, 248, 235));
    setEvenRowColor(QColor(255, 255, 255));
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
        int left = rect.width() - (indentation() + size) / 2 ;
        int top = rect.y() + (rect.height() - size) / 2;
        painter->drawLine(left + 2, top + 4, left + 6, top + 4);
        if (!isOpen(index))
            painter->drawLine(left + 4, top + 2, left + 4, top + 6);
        QPen oldPen = painter->pen();
        painter->setPen(opt.palette.dark().color());
        painter->drawRect(left, top, size - 1, size - 1);
        painter->setPen(oldPen);
    }
}
