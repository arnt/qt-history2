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

#include "abstractwidgetbox.h"

/*!
    \class QDesignerWidgetBoxInterface
    \brief The QDesignerWidgetBoxInterface class provides an interface that is used to
    control Qt Designer's widget box component.
    \inmodule QtDesigner
*/

/*!
*/
QDesignerWidgetBoxInterface::QDesignerWidgetBoxInterface(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
}

/*!
*/
QDesignerWidgetBoxInterface::~QDesignerWidgetBoxInterface()
{
}

/*!
*/
int QDesignerWidgetBoxInterface::findOrInsertCategory(const QString &categoryName)
{
    int count = categoryCount();
    for (int index=0; index<count; ++index) {
        Category c = category(index);
        if (c.name() == categoryName)
            return index;
    }

    addCategory(Category(categoryName));
    return count;
}

/*!
    \fn int QDesignerWidgetBoxInterface::categoryCount() const
*/

/*!
    \fn Category QDesignerWidgetBoxInterface::category(int cat_idx) const
*/

/*!
    \fn void QDesignerWidgetBoxInterface::addCategory(const Category &cat)
*/

/*!
    \fn void QDesignerWidgetBoxInterface::removeCategory(int cat_idx)
*/

/*!
    \fn int QDesignerWidgetBoxInterface::widgetCount(int cat_idx) const
*/

/*!
    \fn Widget QDesignerWidgetBoxInterface::widget(int cat_idx, int wgt_idx) const
*/

/*!
    \fn void QDesignerWidgetBoxInterface::addWidget(int cat_idx, const Widget &wgt)
*/

/*!
    \fn void QDesignerWidgetBoxInterface::removeWidget(int cat_idx, int wgt_idx)
*/

/*!
    \fn void QDesignerWidgetBoxInterface::dropWidgets(const QList<QDesignerDnDItemInterface*> &item_list, const QPoint &global_mouse_pos)
*/

/*!
    \fn void QDesignerWidgetBoxInterface::setFileName(const QString &file_name)
*/

/*!
    \fn QString QDesignerWidgetBoxInterface::fileName() const
*/

/*!
    \fn bool QDesignerWidgetBoxInterface::load()
*/

/*!
    \fn bool QDesignerWidgetBoxInterface::save()
*/


/*!
    \class QDesignerWidgetBoxInterface::Widget
    \brief The Widget class specified a widget in Qt Designer's widget box component.
*/

/*!
    \enum QDesignerWidgetBoxInterface::Widget::Type

    \value Default
    \value Custom
*/

/*!
    \fn QDesignerWidgetBoxInterface::Widget::Widget(const QString &aname, const QString &xml, const QString &icon_name, Type atype)
*/

/*!
    \fn QString QDesignerWidgetBoxInterface::Widget::name() const
*/

/*!
    \fn void QDesignerWidgetBoxInterface::Widget::setName(const QString &aname)
*/

/*!
    \fn QString QDesignerWidgetBoxInterface::Widget::domXml() const
*/

/*!
    \fn void QDesignerWidgetBoxInterface::Widget::setDomXml(const QString &xml)
*/

/*!
    \fn QString QDesignerWidgetBoxInterface::Widget::iconName() const
*/

/*!
    \fn void QDesignerWidgetBoxInterface::Widget::setIconName(const QString &icon_name)
*/

/*!
    \fn Type QDesignerWidgetBoxInterface::Widget::type() const
*/

/*!
    \fn void QDesignerWidgetBoxInterface::Widget::setType(Type atype)
*/

/*!
    \fn bool QDesignerWidgetBoxInterface::Widget::isNull() const
*/


/*!
    \class QDesignerWidgetBoxInterface::Category
    \brief The Category class specifies a category in Qt Designer's widget box component.
*/

/*!
    \enum QDesignerWidgetBoxInterface::Category::Type

    \value Default
    \value Scratchpad
*/

/*!
    \fn QDesignerWidgetBoxInterface::Category::Category(const QString &aname, Type atype)
*/

/*!
    \fn QString QDesignerWidgetBoxInterface::Category::name() const
*/

/*!
    \fn void QDesignerWidgetBoxInterface::Category::setName(const QString &aname)
*/

/*!
    \fn int QDesignerWidgetBoxInterface::Category::widgetCount() const
*/

/*!
    \fn Widget QDesignerWidgetBoxInterface::Category::widget(int idx) const
*/

/*!
    \fn void QDesignerWidgetBoxInterface::Category::removeWidget(int idx)
*/

/*!
    \fn void QDesignerWidgetBoxInterface::Category::addWidget(const Widget &awidget)
*/

/*!
    \fn Type QDesignerWidgetBoxInterface::Category::type() const
*/

/*!
    \fn void QDesignerWidgetBoxInterface::Category::setType(Type atype)
*/

/*!
    \fn bool QDesignerWidgetBoxInterface::Category::isNull() const
*/
