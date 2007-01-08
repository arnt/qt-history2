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

#include "qdesigner_propertysheet_p.h"
#include "qdesigner_utils_p.h"
#include "layoutinfo_p.h"
#include "qdesigner_widget_p.h"

#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerWidgetDataBaseInterface>

#include <QtCore/QMetaObject>
#include <QtCore/QMetaProperty>

#include <QtGui/QLayout>
#include <QtGui/QDockWidget>
#include <QtGui/QDialog>
#include <QtGui/QStackedWidget>
#include <QtGui/QToolBar>
#include <QtGui/QStatusBar>
#include <QtGui/QLabel>
#include <QtGui/QCalendarWidget>
#include <QtGui/QDialogButtonBox>
#include <qdebug.h>

namespace {

const QMetaObject *propertyIntroducedBy(const QMetaObject *meta, int index)
{
    if (index >= meta->propertyOffset())
        return meta;

    if (meta->superClass())
        return propertyIntroducedBy(meta->superClass(), index);

    return 0;
}


bool hasLayoutAttributes(QObject *object)
{
    if (!object->isWidgetType())
        return false;
    
    QWidget *w =  qobject_cast<QWidget *>(object);
    const QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(w);
    if (!fw) 
        return false;
            
    if (const QDesignerFormEditorInterface *core = fw->core()) {
        if (const QDesignerWidgetDataBaseInterface *db = core->widgetDataBase()) {
            if (db->isContainer(w))
                return true;
        }
    }
    return false;
}

}

QDesignerPropertySheet::QDesignerPropertySheet(QObject *object, QObject *parent)
    : QObject(parent),
      m_object(object),
      m_meta(object->metaObject()),
      m_canHaveLayoutAttributes( hasLayoutAttributes(object)),
      m_lastLayout(0),
      m_LastLayoutByDesigner(false)
{
    const QMetaObject *baseMeta = m_meta;

    while (baseMeta && QString::fromUtf8(baseMeta->className()).startsWith(QLatin1String("QDesigner"))) {
        baseMeta = baseMeta->superClass();
    }
    Q_ASSERT(baseMeta != 0);

    for (int index=0; index<count(); ++index) {
        const QMetaProperty p = m_meta->property(index);

        if (p.type() == QVariant::KeySequence)
            createFakeProperty(QString::fromUtf8(p.name()));
        else
            setVisible(index, false); // use the default for `real' properties

        QString pgroup = QString::fromUtf8(baseMeta->className());

        if (const QMetaObject *pmeta = propertyIntroducedBy(baseMeta, index)) {
            pgroup = QString::fromUtf8(pmeta->className());
        }

        setPropertyGroup(index, pgroup);
    }

    if (object->isWidgetType()) {
        createFakeProperty(QLatin1String("focusPolicy"));
        createFakeProperty(QLatin1String("cursor"));
        createFakeProperty(QLatin1String("toolTip"));
        createFakeProperty(QLatin1String("whatsThis"));
        createFakeProperty(QLatin1String("acceptDrops"));
        createFakeProperty(QLatin1String("dragEnabled"));
        createFakeProperty(QLatin1String("windowModality"));

        if (m_canHaveLayoutAttributes) {
            int pindex = count();
            createFakeProperty(QLatin1String("margin"), 0);
            setAttribute(pindex, true);
            setPropertyGroup(pindex, tr("Layout"));

            pindex = count();
            createFakeProperty(QLatin1String("spacing"), 0);
            setAttribute(pindex, true);
            setPropertyGroup(pindex, tr("Layout"));
        }
    }

    if (qobject_cast<QDialog*>(object)) {
        createFakeProperty(QLatin1String("modal"));
    }
    if (qobject_cast<QDockWidget*>(object)) {
        createFakeProperty(QLatin1String("floating"));
    }

    QList<QByteArray> names = object->dynamicPropertyNames();
    QListIterator<QByteArray> itName(names);
    while (itName.hasNext()) {
        QByteArray name = itName.next();
        addDynamicProperty(QString::fromLatin1(name), object->property(name));
        m_info[indexOf(QString::fromLatin1(name.constData()))].defaultDynamic = true;
    }
}

QDesignerPropertySheet::~QDesignerPropertySheet()
{
}

bool QDesignerPropertySheet::dynamicPropertiesAllowed() const
{
    return true;
}

bool QDesignerPropertySheet::canAddDynamicProperty(const QString &propName, const QVariant &value) const
{
    const int index = m_meta->indexOfProperty(propName.toUtf8());
    if (index != -1)
        return false; // property already exists and is not a dynamic one
    if (!value.isValid())
        return false; // property has invalid type
    if (m_addIndex.contains(propName)) {
        const int idx = m_addIndex.value(propName);
        if (isVisible(idx))
            return false; // dynamic property already exists
        else
            return true;
    }
    return true;
}

bool QDesignerPropertySheet::addDynamicProperty(const QString &propName, const QVariant &value)
{
    if (!canAddDynamicProperty(propName, value))
        return false;
    if (m_addIndex.contains(propName)) {
        const int idx = m_addIndex.value(propName);
        // have to be invisible, this was checked in canAddDynamicProperty() method
        setVisible(idx, true);
        m_addProperties.insert(idx, value);
        setChanged(idx, false);
        const int index = m_meta->indexOfProperty(propName.toUtf8());
        m_info[index].defaultValue = value;
        return true;
    }

    const int index = count();
    m_addIndex.insert(propName, index);
    m_addProperties.insert(index, value);
    setVisible(index, true);
    setChanged(index, false);
    m_info[index].defaultValue = value;

    setPropertyGroup(index, tr("Dynamic Properties"));
    return true;
}

bool QDesignerPropertySheet::removeDynamicProperty(const QString &propName)
{
    if (!m_addIndex.contains(propName))
        return false;
    const int index = m_addIndex[propName];

    setVisible(index, false);
    return true;
}

bool QDesignerPropertySheet::isDynamic(int index) const
{
    if (!m_addProperties.contains(index))
        return false;

    const QString pname = propertyName(index);

    if (pname == QLatin1String("margin") || pname == QLatin1String("spacing")) {
        if (m_object->isWidgetType() && m_canHaveLayoutAttributes)
            return false;
    }
    return true;
}

bool QDesignerPropertySheet::isDynamicProperty(int index) const
{
    if (m_info.value(index).defaultDynamic)
        return false;

    return isDynamic(index);
}

void QDesignerPropertySheet::createFakeProperty(const QString &propertyName, const QVariant &value)
{
    // fake properties
    const int index = m_meta->indexOfProperty(propertyName.toUtf8());
    if (index != -1) {
        setVisible(index, false);
        const QVariant v = value.isValid() ? value : metaProperty(index);
        m_fakeProperties.insert(index, v);
    } else if (value.isValid()) { // additional properties
        const int index = count();
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
    return m_meta->propertyCount() + m_addProperties.count();
}

int QDesignerPropertySheet::indexOf(const QString &name) const
{
    int index = m_meta->indexOfProperty(name.toUtf8());

    if (index == -1)
        index = m_addIndex.value(name, -1);

    return index;
}

QString QDesignerPropertySheet::propertyName(int index) const
{
    if (isAdditionalProperty(index))
        return m_addIndex.key(index);

    return QString::fromUtf8(m_meta->property(index).name());
}

QString QDesignerPropertySheet::propertyGroup(int index) const
{
    const QString g = m_info.value(index).group;

    if (!g.isEmpty())
        return g;

    if (propertyName(index).startsWith(QLatin1String("accessible")))
        return QString::fromUtf8("Accessibility");

    if (isAdditionalProperty(index))
        return QString::fromUtf8(m_meta->className());

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
        if (isFakeLayoutProperty(index)) {
            if (const QLayout *l = layout()) {
                return l->property(propertyName(index).toUtf8());
            }
        }

        return m_addProperties.value(index);
    }

    if (isFakeProperty(index)) {
        return m_fakeProperties.value(index);
    }

    return metaProperty(index);
}

QVariant QDesignerPropertySheet::metaProperty(int index) const
{
    Q_ASSERT(!isFakeProperty(index));

    const QMetaProperty p = m_meta->property(index);
    QVariant v = p.read(m_object);

    if (p.isFlagType()) {
        qdesigner_internal::FlagType e;
        e.value = v;
        const QMetaEnum me = p.enumerator();
        QString scope = QString::fromUtf8(me.scope());
        if (!scope.isEmpty())
            scope += QString::fromUtf8("::");
        for (int i=0; i<me.keyCount(); ++i) {
            const QString key = scope + QLatin1String(me.key(i));
            e.items.insert(key, me.keyToValue(key.toUtf8()));
        }

        qVariantSetValue(v, e);
    } else if (p.isEnumType()) {
        qdesigner_internal::EnumType e;
        e.value = v;
        const QMetaEnum me = p.enumerator();
        QString scope = QString::fromUtf8(me.scope());
        if (!scope.isEmpty())
            scope += QString::fromUtf8("::");
        for (int i=0; i<me.keyCount(); ++i) {
            const QString key = scope + QLatin1String(me.key(i));
            e.items.insert(key, me.keyToValue(key.toUtf8()));
            e.names.append(key);
        }

        qVariantSetValue(v, e);
    }

    return v;
}

QVariant QDesignerPropertySheet::resolvePropertyValue(const QVariant &value) const
{
    if (qVariantCanConvert<qdesigner_internal::FlagType>(value))
       return qvariant_cast<qdesigner_internal::FlagType>(value).value;
    
    if (qVariantCanConvert<qdesigner_internal::EnumType>(value))
       return qvariant_cast<qdesigner_internal::EnumType>(value).value;

    return value;
}

void QDesignerPropertySheet::setFakeProperty(int index, const QVariant &value)
{
    Q_ASSERT(isFakeProperty(index));

    QVariant &v = m_fakeProperties[index];

    if (qVariantCanConvert<qdesigner_internal::FlagType>(value) || qVariantCanConvert<qdesigner_internal::EnumType>(value)) {
        v = value;
    } else if (qVariantCanConvert<qdesigner_internal::FlagType>(v)) {
        qdesigner_internal::FlagType f = qvariant_cast<qdesigner_internal::FlagType>(v);
        f.value = value;
        qVariantSetValue(v, f);
        Q_ASSERT(f.value.type() == QVariant::Int);
    } else if (qVariantCanConvert<qdesigner_internal::EnumType>(v)) {
        qdesigner_internal::EnumType e = qvariant_cast<qdesigner_internal::EnumType>(v);
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
        if (isFakeLayoutProperty(index)) {
            if (QLayout *l = layout()) {
                l->setProperty(propertyName(index).toUtf8(), value);
                return;
            }
        }

        m_addProperties[index] = value;
    } else if (isFakeProperty(index)) {
        setFakeProperty(index, value);
    } else {
        QMetaProperty p = m_meta->property(index);
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
    if (isDynamic(index)) {
        QString propName = propertyName(index);
        QVariant oldValue = m_addProperties.value(index);
        QVariant newValue = m_info.value(index).defaultValue;
        if (oldValue == newValue)
            return true;
        m_object->setProperty(propName.toUtf8(), newValue);
        m_addProperties[index] = newValue;
        return true;
    }
    if (isAdditionalProperty(index))
        return false;
    else if (isFakeProperty(index)) {
        const QMetaProperty p = m_meta->property(index);
        const bool result = p.reset(m_object);
        m_fakeProperties[index] = p.read(m_object);
        return result;
    }

    // ### TODO: reset for fake properties.

    const QMetaProperty p = m_meta->property(index);
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

bool QDesignerPropertySheet::isFakeLayoutProperty(int index) const
{
    if (!isAdditionalProperty(index))
        return false;

    const QString pname = propertyName(index);

    if (pname == QLatin1String("sizeConstraint"))
        return true;

    if (pname == QLatin1String("margin")
            || pname == QLatin1String("spacing"))
        return m_canHaveLayoutAttributes;

    return false;
}

bool QDesignerPropertySheet::isVisible(int index) const
{
    if (isAdditionalProperty(index)) {
        if (isFakeLayoutProperty(index) && m_object->isWidgetType()) {
            return layout() != 0;
        }

        return m_info.value(index).visible;
    }

    if (isFakeProperty(index))
        return true;

    const QMetaProperty p = m_meta->property(index);
    return (p.isWritable() && p.isDesignable(m_object)) || m_info.value(index).visible;
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
        return false;

    return m_info.value(index).attribute;
}

void QDesignerPropertySheet::setAttribute(int index, bool attribute)
{
    if (!m_info.contains(index))
        m_info.insert(index, Info());

    m_info[index].attribute = attribute;
}

namespace {
    QDesignerFormEditorInterface *formEditorForWidget(QWidget *w) {
        const QDesignerFormWindowInterface *formWindow =  QDesignerFormWindowInterface::findFormWindow(w);
        if (!formWindow)
            return 0;
        return formWindow->core();
    }
}


QLayout* QDesignerPropertySheet::layout() const
{
    // Return the layout only if it is managed by designer and not one created on a custom widget.
    if (!m_object->isWidgetType() || !m_canHaveLayoutAttributes)
        return 0;
    
    QWidget *widget = qobject_cast<QWidget*>(m_object);
    QLayout *widgetLayout = widget->layout();
    if (!widgetLayout) {
        m_lastLayout = 0;
        return 0;
    }
    // Smart logic to avoid retrieving the meta DB from the widget every time.
    if (widgetLayout != m_lastLayout) {
        m_LastLayoutByDesigner = true;
        if (QDesignerFormEditorInterface *core = formEditorForWidget(widget))
            if (!qdesigner_internal::LayoutInfo::managedLayout(core ,widget))
                m_LastLayoutByDesigner = false;
        m_lastLayout = widgetLayout;
    }
    return m_LastLayoutByDesigner ? m_lastLayout : static_cast<QLayout *>(0);
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

