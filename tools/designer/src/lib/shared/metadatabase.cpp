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

#include "metadatabase.h"

// sdk
#include <abstractformeditor.h>
#include <abstractwidgetfactory.h>

// Qt
#include <QtCore/qalgorithms.h>
#include <QtCore/qdebug.h>

MetaDataBaseItem::MetaDataBaseItem(QObject *object)
    : m_object(object),
      m_spacing(0),
      m_margin(0),
      m_enabled(true)
{
}

MetaDataBaseItem::~MetaDataBaseItem()
{
}

QString MetaDataBaseItem::name() const
{
    Q_ASSERT(m_object);
    return m_object->objectName();
}

void MetaDataBaseItem::setName(const QString &name)
{
    Q_ASSERT(m_object);
    m_object->setObjectName(name);
}

QString MetaDataBaseItem::author() const
{
    return m_author;
}

void MetaDataBaseItem::setAuthor(const QString &author)
{
    m_author = author;
}

QString MetaDataBaseItem::comment() const
{
    return m_comment;
}

void MetaDataBaseItem::setComment(const QString &comment)
{
    m_comment = comment;
}

QCursor MetaDataBaseItem::cursor() const
{
    return m_cursor;
}

void MetaDataBaseItem::setCursor(const QCursor &cursor)
{
    m_cursor = cursor;
}

QList<QWidget*> MetaDataBaseItem::tabOrder() const
{
    return m_tabOrder;
}

void MetaDataBaseItem::setTabOrder(const QList<QWidget*> &tabOrder)
{
    m_tabOrder = tabOrder;
}

int MetaDataBaseItem::spacing() const
{
    return m_spacing;
}

void MetaDataBaseItem::setSpacing(int spacing)
{
    m_spacing = spacing;
}

int MetaDataBaseItem::margin() const
{
    return m_margin;
}

void MetaDataBaseItem::setMargin(int margin)
{
    m_margin = margin;
}

bool MetaDataBaseItem::enabled() const
{
    return m_enabled;
}

void MetaDataBaseItem::setEnabled(bool b)
{
    m_enabled = b;
}

// -----------------------------------------------------
MetaDataBase::MetaDataBase(AbstractFormEditor *core, QObject *parent)
    : AbstractMetaDataBase(parent),
      m_core(core)
{
}

MetaDataBase::~MetaDataBase()
{
    qDeleteAll(m_items);
}

AbstractMetaDataBaseItem *MetaDataBase::item(QObject *object) const
{
    MetaDataBaseItem *i = m_items.value(object);
    if (i == 0 || !i->enabled())
        return 0;
    return i;
}

void MetaDataBase::add(QObject *object)
{
    MetaDataBaseItem *i = m_items.value(object);
    if (i != 0 && !i->enabled()) {
        i->setEnabled(true);
        return;
    }

    m_items.insert(object, new MetaDataBaseItem(object));
    connect(object, SIGNAL(destroyed(QObject*)),
        this, SLOT(slotDestroyed(QObject*)));

    core()->widgetFactory()->initialize(object);

    emit changed();
}

void MetaDataBase::remove(QObject *object)
{
    Q_ASSERT(object);

    if (MetaDataBaseItem *item = m_items.value(object)) {
        item->setEnabled(false);
        emit changed();
    }
}

QList<QObject*> MetaDataBase::objects() const
{
    QList<QObject*> result;

    ItemMap::const_iterator it = m_items.begin();
    for (; it != m_items.end(); ++it) {
        if (it.value()->enabled())
            result.append(it.key());
    }

    return result;
}

AbstractFormEditor *MetaDataBase::core() const
{
    return m_core;
}

void MetaDataBase::slotDestroyed(QObject *object)
{
    if (m_items.contains(object)) {
        MetaDataBaseItem *item = m_items.value(object);
        delete item;
        m_items.remove(object);
    }
}
