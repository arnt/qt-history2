
#include "default_propertysheet.h"

#include <QVariant>
#include <QMetaObject>
#include <QMetaProperty>
#include <qdebug.h>

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

    // fake properties
    int focusPolicyIndex = -1;
    if ((focusPolicyIndex = meta->indexOfProperty("focusPolicy")) != -1) {
        QVariant focusPolicy = metaProperty(focusPolicyIndex);
        setVisible(focusPolicyIndex, false);
        m_fakeProperties.append(qMakePair(QString::fromLatin1("focusPolicy"), focusPolicy));
    }
    
}

QDesignerPropertySheet::~QDesignerPropertySheet()
{
}

bool QDesignerPropertySheet::isFakeProperty(int index) const
{
    return !(index < meta->propertyCount());
}

int QDesignerPropertySheet::count() const
{
    return meta->propertyCount() + m_fakeProperties.count();
}

int QDesignerPropertySheet::indexOf(const QString &name) const
{
    int index = -1;
    
    for (int i=0; i<m_fakeProperties.count(); ++i) {
        if (m_fakeProperties.at(i).first == name) {
            index = meta->propertyCount() + i;
            break;
        }
    }
    
    if (index == -1)
        index = meta->indexOfProperty(name.toLatin1());
    
    return index;
}

QString QDesignerPropertySheet::propertyName(int index) const
{
    if (isFakeProperty(index))
        return m_fakeProperties.at(index - meta->propertyCount()).first;
        
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
    if (isFakeProperty(index)) {
        return m_fakeProperties.at(index - meta->propertyCount()).second;
    }
    
    QMetaProperty p = meta->property(index);
    QVariant v = p.read(m_object);

    if (p.isFlagType()) {
        FlagType e;
        e.value = v;
        QMetaEnum me = p.enumerator();
        for (int i=0; i<me.keyCount(); ++i) {
            const char *k = me.key(i);
            e.items.insert(QString::fromLatin1(k), me.keyToValue(k));
        }

        qVariantSet(v, e, "FlagType");
    } else if (p.isEnumType()) {
        EnumType e;
        e.value = v;
        QMetaEnum me = p.enumerator();
        for (int i=0; i<me.keyCount(); ++i) {
            const char *k = me.key(i);
            e.items.insert(QString::fromLatin1(k), me.keyToValue(k));
        }

        qVariantSet(v, e, "EnumType");
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
            const char *k = me.key(i);
            e.items.insert(QString::fromLatin1(k), me.keyToValue(k));
        }

        qVariantSet(v, e, "FlagType");
    } else if (p.isEnumType()) {
        EnumType e;
        e.value = v;
        QMetaEnum me = p.enumerator();
        for (int i=0; i<me.keyCount(); ++i) {
            const char *k = me.key(i);
            e.items.insert(QString::fromLatin1(k), me.keyToValue(k));
        }

        qVariantSet(v, e, "EnumType");
    }
    
    return v;
}

QVariant QDesignerPropertySheet::resolvePropertyValue(const QVariant &value) const
{
    QVariant v;
    EnumType e;
    FlagType f;
    
    if (qVariantGet(value, f, "FlagType"))
        v = f.value;
    else if (qVariantGet(value, e, "EnumType"))
        v = e.value;
    else
        v = value;
    
    return v;
}

void QDesignerPropertySheet::setFakeProperty(int index, const QVariant &value)
{
    Q_ASSERT(isFakeProperty(index));
    Q_ASSERT(propertyName(index) == m_fakeProperties[index - meta->propertyCount()].first);

    FlagType f;
    EnumType e;

    QVariant &v = m_fakeProperties[index - meta->propertyCount()].second;
    
    if (qVariantGet(value, f, "FlagType") || qVariantGet(value, e, "EnumType")) {
        v = value;
    } else if (qVariantGet(v, f, "FlagType")) {
        f.value = value;
        qVariantSet(v, f, "FlagType");
        Q_ASSERT(f.value.type() == QVariant::Int);
    } else if (qVariantGet(v, e, "EnumType")) {
        e.value = value;
        qVariantSet(v, e, "EnumType");
        Q_ASSERT(e.value.type() == QVariant::Int);
    } else {
        v = value;
    }
}

void QDesignerPropertySheet::setProperty(int index, const QVariant &value)
{
    if (isFakeProperty(index)) {
        setFakeProperty(index, value);
    } else {
        QMetaProperty p = meta->property(index);
        p.write(m_object, resolvePropertyValue(value));
    }
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

QDesignerPropertySheetFactory::QDesignerPropertySheetFactory(QExtensionManager *parent)
    : DefaultExtensionFactory(parent)
{
}

QObject *QDesignerPropertySheetFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid == Q_TYPEID(IPropertySheet))
        return new QDesignerPropertySheet(object, parent);

    return 0;
}

