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

#include "qwidgetbaseitem.h"
#include <qcolor.h>
#include <qpainter.h>
#include <qapplication.h>

QString QWidgetCellItem::text() const
{
    return retrieve(QAbstractItemModel::DisplayRole).toString();
}

void QWidgetCellItem::setText(const QString &text)
{
    store(QAbstractItemModel::DisplayRole, text);
}

QIconSet QWidgetCellItem::icon() const
{
    return retrieve(QAbstractItemModel::DecorationRole).toIcon();
}

void QWidgetCellItem::setIcon(const QIconSet &icon)
{
    store(QAbstractItemModel::DisplayRole, icon);
}

/*!
    Returns the status tip text.

    \sa setStatusTip() whatsThis() toolTip()
*/
QString QWidgetCellItem::statusTip() const
{
    return retrieve(QAbstractItemModel::StatusTipRole).toString();
}

/*!
    Sets the status tip text to \a statusTip.

    \sa statusTip() setWhatsThis() setToolTip()
*/
void QWidgetCellItem::setStatusTip(const QString &statusTip)
{
    store(QAbstractItemModel::StatusTipRole, statusTip);
}

/*!
    Returns the tool tip text.

    \sa setToolTip() whatsThis() statusTip()
*/
QString QWidgetCellItem::toolTip() const
{
    return retrieve(QAbstractItemModel::ToolTipRole).toString();
}

/*!
    Sets the tool tip text to \a toolTip.

    \sa toolTip() setWhatsThis() setStatusTip()
*/
void QWidgetCellItem::setToolTip(const QString &toolTip)
{
    store(QAbstractItemModel::ToolTipRole, toolTip);
}

/*!
    Returns the What's This text.

    \sa setWhatsThis() toolTip() statusTip()
*/
QString QWidgetCellItem::whatsThis() const
{
    return retrieve(QAbstractItemModel::WhatsThisRole).toString();
}

/*!
    Sets the What's This text to \a whatsThis.

    \sa whatsThis() setToolTip() setStatusTip()
*/
void QWidgetCellItem::setWhatsThis(const QString &whatsThis)
{
    store(QAbstractItemModel::WhatsThisRole, whatsThis);
}

/*!
    Returns the text font.

    \sa setFont() textColor()
*/
QFont QWidgetCellItem::font() const
{
    QVariant value = retrieve(FontRole);
    return value.isValid() ? value.toFont() : QApplication::font();
}

/*!
    Sets the \a font for the specified.

    \sa font() setTextColor()
*/
void QWidgetCellItem::setFont(const QFont &font)
{
    store(FontRole, font);
}

/*!
    Returns the background color.

    \sa setBackgroundColor() textColor()
*/
QColor QWidgetCellItem::backgroundColor() const
{
    QVariant value = retrieve(BackgroundColorRole);
    return value.isValid() ? value.toColor() : QColor();
}

/*!
    Sets the background \a color.

    \sa backgroundColor() setTextColor()
*/
void QWidgetCellItem::setBackgroundColor(const QColor &color)
{
    store(BackgroundColorRole, color);
}

/*!
    Returns the text color.

    \sa setTextColor() backgroundColor()
*/
QColor QWidgetCellItem::textColor() const
{
    QVariant value = retrieve(TextColorRole);
    return value.isValid() ? value.toColor() : QColor();
}

/*!
    Sets the text \a color.

    \sa textColor() setBackgroundColor()
*/
void QWidgetCellItem::setTextColor(const QColor &color)
{
    store(TextColorRole, color);
}

QVariant QWidgetCellItem::data(int role) const
{
    role = (role == QAbstractItemModel::EditRole ? QAbstractItemModel::DisplayRole : role);
    switch (role) {
    case QAbstractItemModel::DisplayRole:
        return text();
    case QAbstractItemModel::DecorationRole:
        return icon();
    case QAbstractItemModel::StatusTipRole:
        return statusTip();
    case QAbstractItemModel::ToolTipRole:
        return toolTip();
    case QAbstractItemModel::WhatsThisRole:
        return whatsThis();
    case QWidgetBaseItem::FontRole:
        return font();
    case QWidgetBaseItem::BackgroundColorRole:
        return backgroundColor();
    case QWidgetBaseItem::TextColorRole:
        return textColor();
    }
    return QVariant();
}

void QWidgetCellItem::setData(int role, const QVariant &value)
{
    role = (role == QAbstractItemModel::EditRole ? QAbstractItemModel::DisplayRole : role);
    switch (role) {
    case QAbstractItemModel::DisplayRole:
        setText(value.toString());
        break;
    case QAbstractItemModel::DecorationRole:
        setIcon(value.toIconSet());
        break;
    case QAbstractItemModel::StatusTipRole:
        setStatusTip(value.toString());
        break;
    case QAbstractItemModel::ToolTipRole:
        setToolTip(value.toString());
        break;
    case QAbstractItemModel::WhatsThisRole:
        setWhatsThis(value.toString());
        break;
    case QWidgetBaseItem::FontRole:
        setFont(value.toFont());
        break;
    case QWidgetBaseItem::BackgroundColorRole:
        setBackgroundColor(value.toColor());
        break;
    case QWidgetBaseItem::TextColorRole:
        setTextColor(value.toColor());
        break;
    }
}

void QWidgetCellItem::store(int role, const QVariant &value)
{
    role = (role == QAbstractItemModel::EditRole ? QAbstractItemModel::DisplayRole : role);
    for (int i = 0; i < values.count(); ++i) {
        if (values.at(i).role == role) {
            values[i].value = value;
            return;
        }
    }
    values.append(QWidgetBaseItem::Data(role, value));
}

QVariant QWidgetCellItem::retrieve(int role) const
{
    role = (role == QAbstractItemModel::EditRole ? QAbstractItemModel::DisplayRole : role);
    for (int i = 0; i < values.count(); ++i)
        if (values.at(i).role == role)
            return values.at(i).value;
    return QVariant();
}

void QWidgetBaseItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                    const QAbstractItemModel *model, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    // enabled
    QWidgetBaseItem *item = static_cast<QWidgetBaseItem*>(index.data());
    if (!item->isEnabled()) // FIXME: this will crash when used with other models
        opt.state &= ~QStyle::Style_Enabled;
    // set font
    QVariant value = model->data(index, QWidgetBaseItem::FontRole);
    if (value.isValid())
        opt.font = value.toFont();
    // set text color
    value = model->data(index, QWidgetBaseItem::TextColorRole);
    if (value.isValid() && value.toColor().isValid())
        opt.palette.setColor(QPalette::Text, value.toColor());
    // draw the background color
    value = model->data(index, QWidgetBaseItem::BackgroundColorRole);
    if (value.isValid() && value.toColor().isValid())
        painter->fillRect(option.rect, value.toColor());
    // draw the item
    QItemDelegate::paint(painter, opt, model, index);
}

QSize QWidgetBaseItemDelegate::sizeHint(const QFontMetrics &/*fontMetrics*/,
                                        const QStyleOptionViewItem &option,
                                        const QAbstractItemModel *model,
                                        const QModelIndex &index) const
{
    QVariant value = model->data(index, QWidgetBaseItem::FontRole);
    QFont fnt = value.isValid() ? value.toFont() : option.font;
    return QItemDelegate::sizeHint(QFontMetrics(fnt), option, model, index);
}
