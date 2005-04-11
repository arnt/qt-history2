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

#include "abstractwidgetdatabase.h"
#include <QtCore/qdebug.h>
#include <qalgorithms.h>

QDesignerWidgetDataBaseInterface::QDesignerWidgetDataBaseInterface(QObject *parent)
    : QObject(parent)
{
}

QDesignerWidgetDataBaseInterface::~QDesignerWidgetDataBaseInterface()
{
    qDeleteAll(m_items);
}

int QDesignerWidgetDataBaseInterface::count() const
{
    return m_items.count();
}

QDesignerWidgetDataBaseItemInterface *QDesignerWidgetDataBaseInterface::item(int index) const
{
    return index != -1 ? m_items.at(index) : 0;
}

int QDesignerWidgetDataBaseInterface::indexOf(QDesignerWidgetDataBaseItemInterface *item) const
{
    return m_items.indexOf(item);
}

void QDesignerWidgetDataBaseInterface::insert(int index, QDesignerWidgetDataBaseItemInterface *item)
{
    m_items.insert(index, item);
}

void QDesignerWidgetDataBaseInterface::append(QDesignerWidgetDataBaseItemInterface *item)
{
    m_items.append(item);
}

QDesignerFormEditorInterface *QDesignerWidgetDataBaseInterface::core() const
{
    return 0;
}

int QDesignerWidgetDataBaseInterface::indexOfClassName(const QString &name, bool) const
{
    for (int i=0; i<count(); ++i) {
        QDesignerWidgetDataBaseItemInterface *entry = item(i);
        if (entry->name() == name)
            return i;
    }

    return -1;
}

int QDesignerWidgetDataBaseInterface::indexOfObject(QObject *object, bool) const
{
    if (!object)
        return -1;

    QString className = QString::fromUtf8(object->metaObject()->className());         
    return indexOfClassName(className);
}

bool QDesignerWidgetDataBaseInterface::isContainer(QObject *object, bool resolveName) const
{
    if (QDesignerWidgetDataBaseItemInterface *i = item(indexOfObject(object, resolveName)))
        return i->isContainer();
    return false;
}

bool QDesignerWidgetDataBaseInterface::isForm(QObject *object, bool resolveName) const
{
    if (QDesignerWidgetDataBaseItemInterface *i = item(indexOfObject(object, resolveName)))
        return i->isForm();
    return false;
}

bool QDesignerWidgetDataBaseInterface::isCustom(QObject *object, bool resolveName) const
{
    if (QDesignerWidgetDataBaseItemInterface *i = item(indexOfObject(object, resolveName)))
        return i->isCustom();
    return false;
}
