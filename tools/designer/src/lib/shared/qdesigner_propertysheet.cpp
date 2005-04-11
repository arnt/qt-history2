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

#include "qdesigner_propertysheet.h"

#include <QtCore/QVariant>
#include <QMetaObject>
#include <QMetaProperty>
#include <QImage>
#include <QPixmap>
#include <QtCore/qdebug.h>

QDesignerPropertySheet::QDesignerPropertySheet(QObject *object, QObject *parent)
    : QObject(parent),
      m_object(object),
      meta(object->metaObject())
{
    // ### hack
    for (int index=0; index<count(); ++index) {
        QMetaProperty p = meta->property(index);
        setVisible(index, p.isDesignable(m_object));
    }

    // ### disable the overrided properties

    createFakeProperty("focusPolicy");
    createFakeProperty("cursor");
    createFakeProperty("toolTip");
    createFakeProperty("whatsThis");
    createFakeProperty("acceptDrops");
    createFakeProperty("dragEnabled");
}

QDesignerPropertySheet::~QDesignerPropertySheet()
{
}

void QDesignerPropertySheet::createFakeProperty(const QString &propertyName, const QVariant &value)
{
    // fake properties
    int index = meta->indexOfProperty(propertyName.toLatin1());
    if (index != -1) {
        setVisible(index, false);
        QVariant v = value.isValid() ? value : metaProperty(index);
        m_fakeProperties.insert(index, v);
    } else if (value.isValid()) { // additional properties
        int index = count();
        m_addIndex.insert(propertyName, index);
        m_addProperties.insert(index, value);
    }
}

bool QDesignerPropertySheet::isAdditionalProperty(int index) const
{
    return m_addProperties.contains(index);
}

bool QDesignerPropertySheet::isFakeProperty(int index) const
{
    // additional properties must be fake
    return (m_fakeProperties.contains(index) || isAdditionalProperty(index));
}

int QDesignerPropertySheet::count() const
{
    return meta->propertyCount() + m_addProperties.count();
}

int QDesignerPropertySheet::indexOf(const QString &name) const
{
    int index = meta->indexOfProperty(name.toLatin1());

    if (index == -1)
        index = m_addIndex.value(name, -1);

    return index;
}

QString QDesignerPropertySheet::propertyName(int index) const
{
    if (isAdditionalProperty(index))
        return m_addIndex.key(index);

    return QString::fromLatin1(meta->property(index).name());
}

QString QDesignerPropertySheet::propertyGroup(int index) const
{
    QString g = m_info.value(index).group;

    if (g.isEmpty() && propertyName(index).startsWith("accessible"))
        return QString::fromLatin1("Accessibility");

    return g;
}

void QDesignerPropertySheet::setPropertyGroup(int index, const QString &group)
{
    if (!m_info.contains(index))
        m_info.insert(index, Info());

    m_info[index].group = group;
}

QVariant QDesignerPropertySheet::property(int index) const
{
    if (isAdditionalProperty(index)) {
        return m_addProperties.value(index);
    }

    if (isFakeProperty(index)) {
        return m_fakeProperties.value(index);
    }

    QMetaProperty p = meta->property(index);
    QVariant v = p.read(m_object);

    if (p.isFlagType()) {
        FlagType e;
        e.value = v;
        QMetaEnum me = p.enumerator();
        for (int i=0; i<me.keyCount(); ++i) {
            QString k = QString::fromLatin1(me.scope());
            k += QString::fromLatin1("::");
            k += me.key(i);
            e.items.insert(k, me.keyToValue(k.toLatin1()));
        }

        qVariantSetValue(v, e);
    } else if (p.isEnumType()) {
        EnumType e;
        e.value = v;
        QMetaEnum me = p.enumerator();
        for (int i=0; i<me.keyCount(); ++i) {
            QString k = QString::fromLatin1(me.scope());
            k += QString::fromLatin1("::");
            k += me.key(i);
            e.items.insert(k, me.keyToValue(k.toLatin1()));
        }

        qVariantSetValue(v, e);
    }

    return v;
}

QVariant QDesignerPropertySheet::metaProperty(int index) const
{
    Q_ASSERT(!isFakeProperty(index));

    QMetaProperty p = meta->property(index);
    QVariant v = p.read(m_object);

    if (p.isFlagType()) {
        FlagType e;
        e.value = v;
        QMetaEnum me = p.enumerator();
        for (int i=0; i<me.keyCount(); ++i) {
            QString key;
            key += QLatin1String(me.scope());
            key += QLatin1String("::");
            key += QLatin1String(me.key(i));

            e.items.insert(key, me.keyToValue(key.toLatin1()));
        }

        qVariantSetValue(v, e);
    } else if (p.isEnumType()) {
        EnumType e;
        e.value = v;
        QMetaEnum me = p.enumerator();
        for (int i=0; i<me.keyCount(); ++i) {
            QString key;
            key += QLatin1String(me.scope());
            key += QLatin1String("::");
            key += QLatin1String(me.key(i));

            e.items.insert(key, me.keyToValue(key.toLatin1()));
        }

        qVariantSetValue(v, e);
    }

    return v;
}

QVariant QDesignerPropertySheet::resolvePropertyValue(const QVariant &value) const
{
    QVariant v;
    EnumType e;
    FlagType f;

    if (qVariantCanConvert<FlagType>(value))
        v = qvariant_cast<FlagType>(value).value;
    else if (qVariantCanConvert<EnumType>(value))
        v = qvariant_cast<EnumType>(value).value;
    else
        v = value;

    return v;
}

void QDesignerPropertySheet::setFakeProperty(int index, const QVariant &value)
{
    Q_ASSERT(isFakeProperty(index));

    QVariant &v = m_fakeProperties[index];

    if (qVariantCanConvert<FlagType>(value) || qVariantCanConvert<EnumType>(value)) {
        v = value;
    } else if (qVariantCanConvert<FlagType>(v)) {
        FlagType f = qvariant_cast<FlagType>(v);
        f.value = value;
        qVariantSetValue(v, f);
        Q_ASSERT(f.value.type() == QVariant::Int);
    } else if (qVariantCanConvert<EnumType>(v)) {
        EnumType e = qvariant_cast<EnumType>(v);
        e.value = value;
        qVariantSetValue(v, e);
        Q_ASSERT(e.value.type() == QVariant::Int);
    } else {
        v = value;
    }
}

void QDesignerPropertySheet::setProperty(int index, const QVariant &value)
{
    if (isAdditionalProperty(index)) {
        m_addProperties[index] = value;
    } else if (isFakeProperty(index)) {
        setFakeProperty(index, value);
    } else {
        QMetaProperty p = meta->property(index);
        p.write(m_object, resolvePropertyValue(value));
    }
}

bool QDesignerPropertySheet::hasReset(int index) const
{
    if (isAdditionalProperty(index))
        return m_info.value(index).reset;

    return true;
}

bool QDesignerPropertySheet::reset(int index)
{
    if (isAdditionalProperty(index))
        return false;
    else if (isFakeProperty(index)) {
        QMetaProperty p = meta->property(index);
        bool result = p.reset(m_object);
        m_fakeProperties[index] = p.read(m_object);
        return result;
    }
        
    // ### TODO: reset for fake properties.

    QMetaProperty p = meta->property(index);
    return p.reset(m_object);
}

bool QDesignerPropertySheet::isChanged(int index) const
{
    return m_info.value(index).changed;
}

void QDesignerPropertySheet::setChanged(int index, bool changed)
{
    if (!m_info.contains(index))
        m_info.insert(index, Info());

    m_info[index].changed = changed;
}

bool QDesignerPropertySheet::isVisible(int index) const
{
    if (isAdditionalProperty(index))
        return m_info.value(index).visible;

    if (isFakeProperty(index))
        return true;

    QMetaProperty p = meta->property(index);
    return p.isWritable() && m_info.value(index).visible;
}

void QDesignerPropertySheet::setVisible(int index, bool visible)
{
    if (!m_info.contains(index))
        m_info.insert(index, Info());

    m_info[index].visible = visible;
}

bool QDesignerPropertySheet::isAttribute(int index) const
{
    if (isAdditionalProperty(index))
        return m_info.value(index).attribute;

    if (isFakeProperty(index))
        return true;

    return m_info.value(index).attribute;
}

void QDesignerPropertySheet::setAttribute(int index, bool attribute)
{
    if (!m_info.contains(index))
        m_info.insert(index, Info());

    m_info[index].attribute = attribute;
}


QDesignerPropertySheetFactory::QDesignerPropertySheetFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *QDesignerPropertySheetFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid == Q_TYPEID(QDesignerPropertySheetExtension))
        return new QDesignerPropertySheet(object, parent);

    return 0;
}
