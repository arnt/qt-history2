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
#include "qlayout_widget_p.h"

#include <formbuilderextra_p.h>

#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerWidgetDataBaseInterface>

#include <QtCore/QMetaObject>
#include <QtCore/QMetaProperty>
#include <private/qobject_p.h>

#include <QtGui/QLayout>
#include <QtGui/QDockWidget>
#include <QtGui/QDialog>
#include <QtGui/QLabel>
#include <QtGui/QGroupBox>
#include <QtGui/QStyle>
#include <QtGui/QApplication>

static const QMetaObject *propertyIntroducedBy(const QMetaObject *meta, int index)
{
    if (index >= meta->propertyOffset())
        return meta;

    if (meta->superClass())
        return propertyIntroducedBy(meta->superClass(), index);

    return 0;
}

static QDesignerFormEditorInterface *formEditorForWidget(QWidget *w) {
    const QDesignerFormWindowInterface *formWindow =  QDesignerFormWindowInterface::findFormWindow(w);
    if (!formWindow)
        return 0;
    return formWindow->core();
}

static bool hasLayoutAttributes(QObject *object)
{
    if (!object->isWidgetType())
        return false;

    QWidget *w =  qobject_cast<QWidget *>(object);
    if (const QDesignerFormEditorInterface *core = formEditorForWidget(w)) {
        if (const QDesignerWidgetDataBaseInterface *db = core->widgetDataBase()) {
            if (db->isContainer(w))
                return true;
        }
    }
    return false;
}

// ------------ QDesignerMemberSheetPrivate
class QDesignerPropertySheetPrivate : public  QObjectPrivate {
public:
    typedef QDesignerPropertySheet::PropertyType PropertyType;
    typedef QDesignerPropertySheet::ObjectType ObjectType;

    explicit QDesignerPropertySheetPrivate(QObject *object);

    PropertyType propertyType(int index) const;
    QString transformLayoutPropertyName(int index) const;
    QLayout* layout(QDesignerPropertySheetExtension **layoutPropertySheet = 0) const;
    static ObjectType objectType(const QObject *o);

    class Info {
    public:
        Info();

        QString group;
        QVariant defaultValue;
        bool changed;
        bool visible;
        bool attribute;
        bool reset;
        bool defaultDynamic;
        PropertyType propertyType;
    };

    Info &ensureInfo(int index);

    const QMetaObject *m_meta;
    const ObjectType m_objectType;

    typedef QHash<int, Info> InfoHash;
    InfoHash m_info;
    QHash<int, QVariant> m_fakeProperties;
    QHash<int, QVariant> m_addProperties;
    QHash<QString, int> m_addIndex;

    const bool m_canHaveLayoutAttributes;

    // Variables used for caching the layout, access via layout().
    QPointer<QObject> m_object;
    mutable QPointer<QLayout> m_lastLayout;
    mutable QDesignerPropertySheetExtension *m_lastLayoutPropertySheet;
    mutable bool m_LastLayoutByDesigner;
};

QDesignerPropertySheetPrivate::Info::Info() :
    changed(false),
    visible(true),
    attribute(false),
    reset(true),
    defaultDynamic(false),
    propertyType(QDesignerPropertySheet::PropertyNone)
{
}

QDesignerPropertySheetPrivate::QDesignerPropertySheetPrivate(QObject *object) :
    m_meta(object->metaObject()),
    m_objectType(QDesignerPropertySheet::objectTypeFromObject(object)),
    m_canHaveLayoutAttributes( hasLayoutAttributes(object)),
    m_object(object),
    m_lastLayout(0),
    m_lastLayoutPropertySheet(0),
    m_LastLayoutByDesigner(false)
{
}

QLayout* QDesignerPropertySheetPrivate::layout(QDesignerPropertySheetExtension **layoutPropertySheet) const
{
    // Return the layout and its property sheet
    // only if it is managed by designer and not one created on a custom widget.
    // (attempt to cache the value as this requires some hoops).
    if (layoutPropertySheet)
        *layoutPropertySheet = 0;

    if (!m_object->isWidgetType() || !m_canHaveLayoutAttributes)
        return 0;

    QWidget *widget = qobject_cast<QWidget*>(m_object);
    QLayout *widgetLayout = qdesigner_internal::LayoutInfo::internalLayout(widget);
    if (!widgetLayout) {
        m_lastLayout = 0;
        m_lastLayoutPropertySheet = 0;
        return 0;
    }
    // Smart logic to avoid retrieving the meta DB from the widget every time.
    if (widgetLayout != m_lastLayout) {
        m_lastLayout = widgetLayout;
        m_LastLayoutByDesigner = false;
        m_lastLayoutPropertySheet = 0;
        // Is this a layout managed by designer or some layout on a custom widget?
        if (QDesignerFormEditorInterface *core = formEditorForWidget(widget)) {
            if (qdesigner_internal::LayoutInfo::managedLayout(core ,widgetLayout)) {
                m_LastLayoutByDesigner = true;
                m_lastLayoutPropertySheet = qt_extension<QDesignerPropertySheetExtension*>(core->extensionManager(), m_lastLayout);
            }
        }
    }
    if (!m_LastLayoutByDesigner)
        return 0;

    if (layoutPropertySheet)
        *layoutPropertySheet = m_lastLayoutPropertySheet;

    return  m_lastLayout;
}

QDesignerPropertySheetPrivate::Info &QDesignerPropertySheetPrivate::ensureInfo(int index)
{
    InfoHash::iterator it = m_info.find(index);
    if (it == m_info.end())
        it = m_info.insert(index, Info());
    return it.value();
}

QDesignerPropertySheet::PropertyType QDesignerPropertySheetPrivate::propertyType(int index) const
{
    const InfoHash::const_iterator it = m_info.constFind(index);
    if (it == m_info.constEnd())
        return QDesignerPropertySheet::PropertyNone;
    return it.value().propertyType;
}

QString QDesignerPropertySheetPrivate::transformLayoutPropertyName(int index) const
{
    switch (propertyType(index)) {
    case QDesignerPropertySheet::PropertyLayoutLeftMargin:
        return QLatin1String("leftMargin");
    case QDesignerPropertySheet::PropertyLayoutTopMargin:
        return QLatin1String("topMargin");
    case QDesignerPropertySheet::PropertyLayoutRightMargin:
        return QLatin1String("rightMargin");
    case QDesignerPropertySheet::PropertyLayoutBottomMargin:
        return QLatin1String("bottomMargin");
    case QDesignerPropertySheet::PropertyLayoutSpacing:
        return QLatin1String("spacing");
    case QDesignerPropertySheet::PropertyLayoutHorizontalSpacing:
        return QLatin1String("horizontalSpacing");
    case QDesignerPropertySheet::PropertyLayoutVerticalSpacing:
        return QLatin1String("verticalSpacing");
    default:
        break;
    }
    return QString();
}

// ----------- QDesignerPropertySheet

QDesignerPropertySheet::ObjectType QDesignerPropertySheet::objectTypeFromObject(const QObject *o)
{
    if (qobject_cast<const QLayout *>(o))
        return ObjectLayout;

    if (!o->isWidgetType())
        return ObjectNone;

    if (qobject_cast<const QLayoutWidget *>(o))
        return ObjectLayoutWidget;

    if (qobject_cast<const QLabel*>(o))
        return ObjectLabel;

    if (o->inherits("Q3GroupBox"))
        return ObjectQ3GroupBox;

    return ObjectNone;
}

QDesignerPropertySheet::PropertyType QDesignerPropertySheet::propertyTypeFromName(const QString &name)
{
    typedef QHash<QString, PropertyType> PropertyTypeHash;
    static PropertyTypeHash propertyTypeHash;
    if (propertyTypeHash.empty()) {
        propertyTypeHash.insert(QLatin1String("layoutLeftMargin"),        PropertyLayoutLeftMargin);
        propertyTypeHash.insert(QLatin1String("layoutTopMargin"),         PropertyLayoutTopMargin);
        propertyTypeHash.insert(QLatin1String("layoutRightMargin"),       PropertyLayoutRightMargin);
        propertyTypeHash.insert(QLatin1String("layoutBottomMargin"),      PropertyLayoutBottomMargin);
        propertyTypeHash.insert(QLatin1String("layoutSpacing"),           PropertyLayoutSpacing);
        propertyTypeHash.insert(QLatin1String("layoutHorizontalSpacing"), PropertyLayoutHorizontalSpacing);
        propertyTypeHash.insert(QLatin1String("layoutVerticalSpacing"),   PropertyLayoutVerticalSpacing);
        propertyTypeHash.insert(QLatin1String("buddy"),                   PropertyBuddy);
        propertyTypeHash.insert(QLatin1String("sizeConstraint"),          PropertySizeConstraint);
        propertyTypeHash.insert(QLatin1String("geometry"),                PropertyGeometry);
        propertyTypeHash.insert(QLatin1String("checkable"),               PropertyCheckable);
        propertyTypeHash.insert(QLatin1String("accessibleName"),          PropertyAccessibility);
        propertyTypeHash.insert(QLatin1String("accessibleDescription"),   PropertyAccessibility);
    }
    return propertyTypeHash.value(name, PropertyNone);
}

QDesignerPropertySheet::QDesignerPropertySheet(QObject *object, QObject *parent) :
    QObject(*(new QDesignerPropertySheetPrivate(object)), parent)
{
    typedef QDesignerPropertySheetPrivate::Info Info;
    Q_D(QDesignerPropertySheet);
    const QMetaObject *baseMeta = d->m_meta;

    while (baseMeta && QString::fromUtf8(baseMeta->className()).startsWith(QLatin1String("QDesigner"))) {
        baseMeta = baseMeta->superClass();
    }
    Q_ASSERT(baseMeta != 0);

    for (int index=0; index<count(); ++index) {
        const QMetaProperty p = d->m_meta->property(index);
        const QString name = QString::fromUtf8(p.name());
        if (p.type() == QVariant::KeySequence) {
            createFakeProperty(name);
        } else {
            setVisible(index, false); // use the default for `real' properties
        }

        QString pgroup = QString::fromUtf8(baseMeta->className());

        if (const QMetaObject *pmeta = propertyIntroducedBy(baseMeta, index)) {
            pgroup = QString::fromUtf8(pmeta->className());
        }

        Info &info = d->ensureInfo(index);
        info.group = pgroup;
        info.propertyType = propertyTypeFromName(name);

        if (p.type() == QVariant::Cursor) {
            info.defaultValue = p.read(d->m_object);
        }
    }

    if (object->isWidgetType()) {
        createFakeProperty(QLatin1String("focusPolicy"));
        createFakeProperty(QLatin1String("cursor"));
        createFakeProperty(QLatin1String("toolTip"));
        createFakeProperty(QLatin1String("whatsThis"));
        createFakeProperty(QLatin1String("acceptDrops"));
        createFakeProperty(QLatin1String("dragEnabled"));
        createFakeProperty(QLatin1String("windowModality"));

        if (d->m_canHaveLayoutAttributes) {
            static const QString layoutGroup = QLatin1String("Layout");
            int pindex = count();
            createFakeProperty(QLatin1String("layoutLeftMargin"), 0);
            setAttribute(pindex, true);
            setPropertyGroup(pindex, layoutGroup);

            pindex = count();
            createFakeProperty(QLatin1String("layoutTopMargin"), 0);
            setAttribute(pindex, true);
            setPropertyGroup(pindex, layoutGroup);

            pindex = count();
            createFakeProperty(QLatin1String("layoutRightMargin"), 0);
            setAttribute(pindex, true);
            setPropertyGroup(pindex, layoutGroup);

            pindex = count();
            createFakeProperty(QLatin1String("layoutBottomMargin"), 0);
            setAttribute(pindex, true);
            setPropertyGroup(pindex, layoutGroup);

            pindex = count();
            createFakeProperty(QLatin1String("layoutSpacing"), 0);
            setAttribute(pindex, true);
            setPropertyGroup(pindex, layoutGroup);

            pindex = count();
            createFakeProperty(QLatin1String("layoutHorizontalSpacing"), 0);
            setAttribute(pindex, true);
            setPropertyGroup(pindex, layoutGroup);

            pindex = count();
            createFakeProperty(QLatin1String("layoutVerticalSpacing"), 0);
            setAttribute(pindex, true);
            setPropertyGroup(pindex, layoutGroup);
        }

        if (d->m_objectType == ObjectLabel)
            createFakeProperty(QLatin1String("buddy"), QVariant(QByteArray()));
    }

    if (qobject_cast<const QDialog*>(object)) {
        createFakeProperty(QLatin1String("modal"));
    }
    if (qobject_cast<const QDockWidget*>(object)) {
        createFakeProperty(QLatin1String("floating"));
    }

    typedef QList<QByteArray> ByteArrayList;
    const ByteArrayList names = object->dynamicPropertyNames();
    if (!names.empty()) {
        const ByteArrayList::const_iterator cend =  names.constEnd();
        for (ByteArrayList::const_iterator it = names.constBegin(); it != cend; ++it) {
            const char* cName = it->constData();
            const QString name = QString::fromLatin1(cName);
            const int idx = addDynamicProperty(name, object->property(cName));
            if (idx != -1)
                d->ensureInfo(idx).defaultDynamic = true;
        }
    }
}

QDesignerPropertySheet::~QDesignerPropertySheet()
{
}

QObject *QDesignerPropertySheet::object() const
{
    Q_D(const QDesignerPropertySheet);
    return d->m_object;
}

bool QDesignerPropertySheet::dynamicPropertiesAllowed() const
{
    return true;
}

bool QDesignerPropertySheet::canAddDynamicProperty(const QString &propName) const
{
    Q_D(const QDesignerPropertySheet);
    const int index = d->m_meta->indexOfProperty(propName.toUtf8());
    if (index != -1)
        return false; // property already exists and is not a dynamic one
    if (d->m_addIndex.contains(propName)) {
        const int idx = d->m_addIndex.value(propName);
        if (isVisible(idx))
            return false; // dynamic property already exists
        else
            return true;
    }
    if (propName.startsWith(QLatin1String("_q_")))
        return false;
    return true;
}

int QDesignerPropertySheet::addDynamicProperty(const QString &propName, const QVariant &value)
{
    Q_D(QDesignerPropertySheet);
    if (!value.isValid())
        return -1; // property has invalid type
    if (!canAddDynamicProperty(propName))
        return -1;
    if (d->m_addIndex.contains(propName)) {
        const int idx = d->m_addIndex.value(propName);
        // have to be invisible, this was checked in canAddDynamicProperty() method
        setVisible(idx, true);
        d->m_addProperties.insert(idx, value);
        setChanged(idx, false);
        const int index = d->m_meta->indexOfProperty(propName.toUtf8());
        d->m_info[index].defaultValue = value;
        return idx;
    }

    const int index = count();
    d->m_addIndex.insert(propName, index);
    d->m_addProperties.insert(index, value);
    setVisible(index, true);
    setChanged(index, false);
    d->m_info[index].defaultValue = value;

    setPropertyGroup(index, tr("Dynamic Properties"));
    return index;
}

bool QDesignerPropertySheet::removeDynamicProperty(int index)
{
    Q_D(QDesignerPropertySheet);
    if (!d->m_addIndex.contains(propertyName(index)))
        return false;

    setVisible(index, false);
    return true;
}

bool QDesignerPropertySheet::isDynamic(int index) const
{
    Q_D(const QDesignerPropertySheet);
    if (!d->m_addProperties.contains(index))
        return false;

    switch (propertyType(index)) {
    case PropertyBuddy:
        if (d->m_objectType == ObjectLabel)
            return false;
        break;
    case PropertyLayoutLeftMargin:
    case PropertyLayoutTopMargin:
    case PropertyLayoutRightMargin:
    case PropertyLayoutBottomMargin:
    case PropertyLayoutSpacing:
    case PropertyLayoutHorizontalSpacing:
    case PropertyLayoutVerticalSpacing:
        if (d->m_object->isWidgetType() && d->m_canHaveLayoutAttributes)
            return false;
    default:
        break;
    }
    return true;
}

bool QDesignerPropertySheet::isDynamicProperty(int index) const
{
    Q_D(const QDesignerPropertySheet);
    if (d->m_info.value(index).defaultDynamic)
        return false;

    return isDynamic(index);
}

void QDesignerPropertySheet::createFakeProperty(const QString &propertyName, const QVariant &value)
{
    Q_D(QDesignerPropertySheet);
    // fake properties
    const int index = d->m_meta->indexOfProperty(propertyName.toUtf8());
    if (index != -1) {
        if (!d->m_meta->property(index).isDesignable())
            return;
        setVisible(index, false);
        const QVariant v = value.isValid() ? value : metaProperty(index);
        d->m_fakeProperties.insert(index, v);
    } else if (value.isValid()) { // additional properties
        const int index = count();
        d->m_addIndex.insert(propertyName, index);
        d->m_addProperties.insert(index, value);
        d->ensureInfo(index).propertyType = propertyTypeFromName(propertyName);
    }
}

bool QDesignerPropertySheet::isAdditionalProperty(int index) const
{
    Q_D(const QDesignerPropertySheet);
    return d->m_addProperties.contains(index);
}

bool QDesignerPropertySheet::isFakeProperty(int index) const
{
    Q_D(const QDesignerPropertySheet);
    // additional properties must be fake
    return (d->m_fakeProperties.contains(index) || isAdditionalProperty(index));
}

int QDesignerPropertySheet::count() const
{
    Q_D(const QDesignerPropertySheet);
    return d->m_meta->propertyCount() + d->m_addProperties.count();
}

int QDesignerPropertySheet::indexOf(const QString &name) const
{
    Q_D(const QDesignerPropertySheet);
    int index = d->m_meta->indexOfProperty(name.toUtf8());

    if (index == -1)
        index = d->m_addIndex.value(name, -1);

    return index;
}

QDesignerPropertySheet::PropertyType QDesignerPropertySheet::propertyType(int index) const
{
    Q_D(const QDesignerPropertySheet);
    return d->propertyType(index);
}

QDesignerPropertySheet::ObjectType QDesignerPropertySheet::objectType() const
{
    Q_D(const QDesignerPropertySheet);
    return d->m_objectType;
}

QString QDesignerPropertySheet::propertyName(int index) const
{
    Q_D(const QDesignerPropertySheet);
    if (isAdditionalProperty(index))
        return d->m_addIndex.key(index);

    return QString::fromUtf8(d->m_meta->property(index).name());
}

QString QDesignerPropertySheet::propertyGroup(int index) const
{
    Q_D(const QDesignerPropertySheet);
    const QString g = d->m_info.value(index).group;

    if (!g.isEmpty())
        return g;

    if (propertyType(index) == PropertyAccessibility)
        return QString::fromUtf8("Accessibility");

    if (isAdditionalProperty(index))
        return QString::fromUtf8(d->m_meta->className());

    return g;
}

void QDesignerPropertySheet::setPropertyGroup(int index, const QString &group)
{
    Q_D(QDesignerPropertySheet);
    d->ensureInfo(index).group = group;
}

QVariant QDesignerPropertySheet::property(int index) const
{
    Q_D(const QDesignerPropertySheet);
    if (isAdditionalProperty(index)) {
        if (isFakeLayoutProperty(index)) {
            QDesignerPropertySheetExtension *layoutPropertySheet;
            if (d->layout(&layoutPropertySheet) && layoutPropertySheet) {
                const QString newPropName = d->transformLayoutPropertyName(index);
                if (!newPropName.isEmpty())
                    return layoutPropertySheet->property(layoutPropertySheet->indexOf(newPropName));
            }
        }
        return d->m_addProperties.value(index);
    }

    if (isFakeProperty(index)) {
        return d->m_fakeProperties.value(index);
    }

    return metaProperty(index);
}

QVariant QDesignerPropertySheet::metaProperty(int index) const
{
    Q_D(const QDesignerPropertySheet);
    Q_ASSERT(!isFakeProperty(index));

    const QMetaProperty p = d->m_meta->property(index);
    QVariant v = p.read(d->m_object);

    static const QString doubleColon = QLatin1String("::");
    if (p.isFlagType()) {
        qdesigner_internal::FlagType e;
        e.value = v;
        const QMetaEnum me = p.enumerator();
        QString scope = QString::fromUtf8(me.scope());
        if (!scope.isEmpty())
            scope += doubleColon;
        for (int i=0; i<me.keyCount(); ++i) {
            const QString key = scope + QLatin1String(me.key(i));
            e.items.insert(key, me.keyToValue(key.toUtf8().constData()));
            e.names.append(key);
        }

        qVariantSetValue(v, e);
    } else if (p.isEnumType()) {
        qdesigner_internal::EnumType e;
        e.value = v;
        const QMetaEnum me = p.enumerator();
        QString scope = QString::fromUtf8(me.scope());
        if (!scope.isEmpty())
            scope += doubleColon;
        for (int i=0; i<me.keyCount(); ++i) {
            const QString key = scope + QLatin1String(me.key(i));
            e.items.insert(key, me.keyToValue(key.toUtf8().constData()));
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
    Q_D(QDesignerPropertySheet);
    Q_ASSERT(isFakeProperty(index));

    QVariant &v = d->m_fakeProperties[index];

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

void QDesignerPropertySheet::clearFakeProperties()
{
    Q_D(QDesignerPropertySheet);
    d->m_fakeProperties.clear();
}

// Buddy needs to be byte array, else uic won't work
static QVariant toByteArray(const QVariant &value) {
    if (value.type() == QVariant::ByteArray)
        return value;
    const QByteArray ba = value.toString().toUtf8();
    return QVariant(ba);
}

void QDesignerPropertySheet::setProperty(int index, const QVariant &value)
{
    Q_D(QDesignerPropertySheet);
    if (isAdditionalProperty(index)) {
        if (d->m_objectType == ObjectLabel && propertyType(index) == PropertyBuddy) {
            QFormBuilderExtra::applyBuddy(value.toString(), QFormBuilderExtra::BuddyApplyVisibleOnly, qobject_cast<QLabel *>(d->m_object));
            d->m_addProperties[index] = toByteArray(value);
            return;
        }

        if (isFakeLayoutProperty(index)) {
            QDesignerPropertySheetExtension *layoutPropertySheet;
            if (d->layout(&layoutPropertySheet) && layoutPropertySheet) {
                const QString newPropName = d->transformLayoutPropertyName(index);
                if (!newPropName.isEmpty())
                    layoutPropertySheet->setProperty(layoutPropertySheet->indexOf(newPropName), value);
            }
        }

        if (isDynamicProperty(index)) {
            d->m_object->setProperty(propertyName(index).toUtf8(), value);
            if (d->m_object->isWidgetType()) {
                QWidget *w = qobject_cast<QWidget *>(d->m_object);
                w->setStyleSheet(w->styleSheet());
            }
        }
        d->m_addProperties[index] = value;
    } else if (isFakeProperty(index)) {
        setFakeProperty(index, value);
    } else {
        const QMetaProperty p = d->m_meta->property(index);
        p.write(d->m_object, resolvePropertyValue(value));
        if (qobject_cast<QGroupBox *>(d->m_object) && propertyType(index) == PropertyCheckable) {
            const int idx = indexOf(QLatin1String("focusPolicy"));
            if (!isChanged(idx)) {
                qdesigner_internal::EnumType e = qVariantValue<qdesigner_internal::EnumType>(property(idx));
                if (value.toBool()) {
                    const QMetaProperty p = d->m_meta->property(idx);
                    p.write(d->m_object, Qt::NoFocus);
                    e.value = Qt::StrongFocus;
                    QVariant v;
                    qVariantSetValue(v, e);
                    setFakeProperty(idx, v);
                } else {
                    e.value = Qt::NoFocus;
                    QVariant v;
                    qVariantSetValue(v, e);
                    setFakeProperty(idx, v);
                }
            }
        }
    }
}

bool QDesignerPropertySheet::hasReset(int index) const
{
    Q_D(const QDesignerPropertySheet);
    if (isAdditionalProperty(index))
        return d->m_info.value(index).reset;

    return true;
}

bool QDesignerPropertySheet::reset(int index)
{
    Q_D(QDesignerPropertySheet);
    if (isDynamic(index)) {
        const QString propName = propertyName(index);
        const QVariant oldValue = d->m_addProperties.value(index);
        const QVariant newValue = d->m_info.value(index).defaultValue;
        if (oldValue == newValue)
            return true;
        d->m_object->setProperty(propName.toUtf8(), newValue);
        d->m_addProperties[index] = newValue;
        return true;
    } else if (!d->m_info.value(index).defaultValue.isNull()) {
        setProperty(index, d->m_info.value(index).defaultValue);
        return true;
    }
    if (isAdditionalProperty(index)) {
        const PropertyType pType = propertyType(index);

        if (d->m_objectType == ObjectLabel && pType == PropertyBuddy) {
            setProperty(index, QVariant(QByteArray()));
            return true;
        }

        if (isFakeLayoutProperty(index)) {
            int value = -1;
            switch (d->m_objectType) {
            case ObjectQ3GroupBox: {
                const QWidget *w = qobject_cast<const QWidget *>(d->m_object);
                switch (pType) {
                case PropertyLayoutLeftMargin:
                    value = w->style()->pixelMetric(QStyle::PM_LayoutLeftMargin);
                    break;
                case PropertyLayoutTopMargin:
                    value = w->style()->pixelMetric(QStyle::PM_LayoutTopMargin);
                    break;
                case PropertyLayoutRightMargin:
                    value = w->style()->pixelMetric(QStyle::PM_LayoutRightMargin);
                    break;
                case PropertyLayoutBottomMargin:
                    value = w->style()->pixelMetric(QStyle::PM_LayoutBottomMargin);
                    break;
                case PropertyLayoutSpacing:
                case PropertyLayoutHorizontalSpacing:
                case PropertyLayoutVerticalSpacing:
                    value = -1;
                    break;
                default:
                    break;
                }
            }
                break;
            case ObjectLayoutWidget:
                if (pType == PropertyLayoutLeftMargin ||
                        pType == PropertyLayoutTopMargin ||
                        pType == PropertyLayoutRightMargin ||
                        pType == PropertyLayoutBottomMargin)
                    value = 0;
                break;
            default:
                break;
            }
            setProperty(index, value);
            return true;
        }
        return false;
    } else if (isFakeProperty(index)) {
        const QMetaProperty p = d->m_meta->property(index);
        const bool result = p.reset(d->m_object);
        d->m_fakeProperties[index] = p.read(d->m_object);
        return result;
    } else if (propertyType(index) == PropertyGeometry && d->m_object->isWidgetType()) {
        if (QWidget *w = qobject_cast<QWidget*>(d->m_object)) {
            QWidget *widget = w;
            QDesignerFormWindowInterface *formWindow = QDesignerFormWindowInterface::findFormWindow(widget);
            if (qdesigner_internal::Utils::isCentralWidget(formWindow, widget) && formWindow->parentWidget())
                widget = formWindow->parentWidget();

            if (widget != w && widget->parentWidget()) {
                QApplication::processEvents();
                widget->parentWidget()->adjustSize();
            }
            QApplication::processEvents();
            widget->adjustSize();
            return true;
        }
    }

    // ### TODO: reset for fake properties.

    const QMetaProperty p = d->m_meta->property(index);
    return p.reset(d->m_object);
}

bool QDesignerPropertySheet::isChanged(int index) const
{
    Q_D(const QDesignerPropertySheet);
    if (isAdditionalProperty(index)) {
        if (isFakeLayoutProperty(index)) {
            QDesignerPropertySheetExtension *layoutPropertySheet;
            if (d->layout(&layoutPropertySheet) && layoutPropertySheet) {
                const QString newPropName = d->transformLayoutPropertyName(index);
                if (!newPropName.isEmpty())
                    return layoutPropertySheet->isChanged(layoutPropertySheet->indexOf(newPropName));
            }
        }
    }
    return d->m_info.value(index).changed;
}

void QDesignerPropertySheet::setChanged(int index, bool changed)
{
    Q_D(QDesignerPropertySheet);
    if (isAdditionalProperty(index)) {
        if (isFakeLayoutProperty(index)) {
            QDesignerPropertySheetExtension *layoutPropertySheet;
            if (d->layout(&layoutPropertySheet) && layoutPropertySheet) {
                const QString newPropName = d->transformLayoutPropertyName(index);
                if (!newPropName.isEmpty())
                    layoutPropertySheet->setChanged(layoutPropertySheet->indexOf(newPropName), changed);
            }
        }
    }
    d->ensureInfo(index).changed = changed;
}

bool QDesignerPropertySheet::isFakeLayoutProperty(int index) const
{
    Q_D(const QDesignerPropertySheet);
    if (!isAdditionalProperty(index))
        return false;

    switch (propertyType(index)) {
    case PropertySizeConstraint:
        return true;
    case PropertyLayoutLeftMargin:
    case PropertyLayoutTopMargin:
    case PropertyLayoutRightMargin:
    case PropertyLayoutBottomMargin:
    case PropertyLayoutSpacing:
    case PropertyLayoutHorizontalSpacing:
    case PropertyLayoutVerticalSpacing:
        return d->m_canHaveLayoutAttributes;
    default:
        break;
    }
    return false;
}

bool QDesignerPropertySheet::isVisible(int index) const
{
    Q_D(const QDesignerPropertySheet);
    if (isAdditionalProperty(index)) {
        if (isFakeLayoutProperty(index) && d->m_object->isWidgetType()) {
            QLayout *currentLayout = d->layout();
            if (!currentLayout)
                return false;
            switch (propertyType(index)) {
            case  PropertyLayoutSpacing:
                if (qobject_cast<const QGridLayout *>(currentLayout))
                    return false;
                break;
            case PropertyLayoutHorizontalSpacing:
            case PropertyLayoutVerticalSpacing:
                if (!qobject_cast<const QGridLayout *>(currentLayout))
                    return false;
                break;
            default:
                break;
            }
            return true;
        }
        return d->m_info.value(index).visible;
    }

    if (isFakeProperty(index))
        return true;

    const QMetaProperty p = d->m_meta->property(index);
    return (p.isWritable() && p.isDesignable(d->m_object)) || d->m_info.value(index).visible;
}

void QDesignerPropertySheet::setVisible(int index, bool visible)
{
    Q_D(QDesignerPropertySheet);
    d->ensureInfo(index).visible = visible;
}

bool QDesignerPropertySheet::isAttribute(int index) const
{
    Q_D(const QDesignerPropertySheet);
    if (isAdditionalProperty(index))
        return d->m_info.value(index).attribute;

    if (isFakeProperty(index))
        return false;

    return d->m_info.value(index).attribute;
}

void QDesignerPropertySheet::setAttribute(int index, bool attribute)
{
    Q_D(QDesignerPropertySheet);
    d->ensureInfo(index).attribute = attribute;
}

// ---------- QDesignerAbstractPropertySheetFactory
QDesignerAbstractPropertySheetFactory::QDesignerAbstractPropertySheetFactory(QExtensionManager *parent) :
    QExtensionFactory(parent),
    m_propertySheetId(Q_TYPEID(QDesignerPropertySheetExtension)),
    m_dynamicPropertySheetId(Q_TYPEID(QDesignerDynamicPropertySheetExtension))
{
}

QObject *QDesignerAbstractPropertySheetFactory::extension(QObject *object, const QString &iid) const
{
    if (!object)
        return 0;

    if (iid != m_propertySheetId && iid != m_dynamicPropertySheetId)
        return 0;

    ExtensionMap::iterator it = m_extensions.find(object);
    if (it == m_extensions.end()) {
        if (QObject *ext = createPropertySheet(object, const_cast<QDesignerAbstractPropertySheetFactory*>(this))) {
            connect(ext, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)));
            it = m_extensions.insert(object, ext);
        }
    }

    if (!m_extended.contains(object)) {
        connect(object, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)));
        m_extended.insert(object, true);
    }

    if (it == m_extensions.end())
        return 0;

    return it.value();
}

void QDesignerAbstractPropertySheetFactory::objectDestroyed(QObject *object)
{
    QMutableMapIterator<QObject*, QObject*> it(m_extensions);
    while (it.hasNext()) {
        it.next();

        QObject *o = it.key();
        if (o == object || object == it.value()) {
            it.remove();
        }
    }

    m_extended.remove(object);
}
