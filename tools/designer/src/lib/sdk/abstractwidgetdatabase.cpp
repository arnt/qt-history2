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

AbstractWidgetDataBase::AbstractWidgetDataBase(QObject *parent)
    : QObject(parent)
{
}

AbstractWidgetDataBase::~AbstractWidgetDataBase()
{
    qDeleteAll(m_items);
}

int AbstractWidgetDataBase::count() const
{
    return m_items.count();
}

AbstractWidgetDataBaseItem *AbstractWidgetDataBase::item(int index) const
{
    return index != -1 ? m_items.at(index) : 0;
}

int AbstractWidgetDataBase::indexOf(AbstractWidgetDataBaseItem *item) const
{
    return m_items.indexOf(item);
}

void AbstractWidgetDataBase::insert(int index, AbstractWidgetDataBaseItem *item)
{
    m_items.insert(index, item);
}

void AbstractWidgetDataBase::append(AbstractWidgetDataBaseItem *item)
{
    m_items.append(item);
}

AbstractFormEditor *AbstractWidgetDataBase::core() const
{
    return 0;
}

int AbstractWidgetDataBase::indexOfClassName(const QString &name, bool) const
{
    for (int i=0; i<count(); ++i) {
        AbstractWidgetDataBaseItem *entry = item(i);
        if (entry->name() == name)
            return i;
    }

    return -1;
}

int AbstractWidgetDataBase::indexOfObject(QObject *object, bool) const
{
    if (!object)
        return -1;

    QString className = QString::fromUtf8(object->metaObject()->className());         
    return indexOfClassName(className);
}

bool AbstractWidgetDataBase::isContainer(QObject *object, bool resolveName) const
{
    if (AbstractWidgetDataBaseItem *i = item(indexOfObject(object, resolveName)))
        return i->isContainer();
    return false;
}

bool AbstractWidgetDataBase::isForm(QObject *object, bool resolveName) const
{
    if (AbstractWidgetDataBaseItem *i = item(indexOfObject(object, resolveName)))
        return i->isForm();
    return false;
}

bool AbstractWidgetDataBase::isCustom(QObject *object, bool resolveName) const
{
    if (AbstractWidgetDataBaseItem *i = item(indexOfObject(object, resolveName)))
        return i->isCustom();
    return false;
}
