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

#include "tree_widget_p.h"

#include <QtCore/QPair>
#include <QtCore/QStack>

#include <QtGui/QApplication>
#include <QtGui/QHeaderView>
#include <QtGui/QScrollBar>
#include <QtGui/QItemDelegate>
#include <QtGui/QPainter>
#include <QtGui/QStyle>

QT_BEGIN_NAMESPACE

namespace {
    enum { windowsDecoSize = 9 };
}

namespace qdesigner_internal {
// -------------- TreeWidgetDelegate
TreeWidgetDelegate::TreeWidgetDelegate(QObject *parent) :
    QItemDelegate(parent)
{
}

void TreeWidgetDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    QItemDelegate::paint(painter, option, index);

    const QPen savedPen = painter->pen();
    const QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &option));
    painter->setPen(QPen(color));

    painter->drawLine(option.rect.x(), option.rect.bottom(),
                      option.rect.right(), option.rect.bottom());

    const int right = (option.direction == Qt::LeftToRight) ? option.rect.right() : option.rect.left();
    painter->drawLine(right, option.rect.y(), right, option.rect.bottom());

    painter->setPen(savedPen);
}

QSize TreeWidgetDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QItemDelegate::sizeHint(option, index) + QSize(4,4);
}
}

static inline void initializeTreeView(QTreeView *tv, qdesigner_internal::TreeWidgetDelegate *delegate)
{
    tv->setItemDelegate(delegate);
    tv->setAlternatingRowColors(true);
}

static int level(QAbstractItemModel *model, const QModelIndex &index)
{
    int result = 0;
    QModelIndex parent = model->parent(index);
    while (parent.isValid()) {
        parent = model->parent(parent);
        ++result;
    }
    return result;
}

static void drawTreeBranches(const QTreeView *iv, QStyleOptionViewItem option, QPainter *painter, const QRect &rect, const QModelIndex &index)
{
    // designer fights the style it uses. :(
    static const bool mac_style = QApplication::style()->inherits("QMacStyle");

    if (iv->model()->hasChildren(index)) {
        option.state |= QStyle::State_Children;

        const bool reverse = iv->isRightToLeft();
        const int indentation = iv->indentation();
        const int indent = level(iv->model(), index) * indentation;
        QRect primitive(reverse ? rect.left() : rect.left() + indent - 2,
                        rect.top(), indentation, rect.height());

        if (!mac_style) {
            primitive.moveLeft(reverse ? primitive.left()
                               : primitive.left() + (primitive.width() - windowsDecoSize)/2);
            primitive.moveTop(primitive.top() + (primitive.height() - windowsDecoSize)/2);
            primitive.setWidth(windowsDecoSize);
            primitive.setHeight(windowsDecoSize);
        }

        option.rect = primitive;

        if (iv->isExpanded(index))
            option.state |= QStyle::State_Open;

        iv->style()->drawPrimitive(QStyle::PE_IndicatorBranch, &option, painter, iv);
    }
    const QPen savedPen = painter->pen();
    const QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &option));
    painter->setPen(QPen(color));
    painter->drawLine(rect.x(), rect.bottom(), rect.right(), rect.bottom());
    painter->setPen(savedPen);
}

namespace qdesigner_internal {

// -- TreeView
TreeView::TreeView(QWidget *parent) :
    QTreeView(parent)
{
    initializeTreeView(this, new TreeWidgetDelegate(this));
}

TreeView::TreeView(TreeWidgetDelegate *delegate, QWidget *parent) :
    QTreeView(parent)
{
    delegate->setParent(this);
    initializeTreeView(this, delegate);
}

void TreeView::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    drawTreeBranches(this, viewOptions(), painter, rect, index);
}

//--  TreeWidget
TreeWidget::TreeWidget(QWidget *parent) :
    QTreeWidget(parent)
{
    initializeTreeView(this, new TreeWidgetDelegate(this));
}

TreeWidget::TreeWidget(TreeWidgetDelegate *delegate, QWidget *parent) :
    QTreeWidget(parent)
{
    delegate->setParent(this);
    initializeTreeView(this, delegate);
}

void TreeWidget::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    drawTreeBranches(this, viewOptions(), painter, rect, index);
}
} // namespace qdesigner_internal

QT_END_NAMESPACE
