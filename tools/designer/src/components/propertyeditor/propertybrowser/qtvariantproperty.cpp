#include "qtvariantproperty.h"
#include "qtpropertymanager.h"
#include "qteditorfactory.h"
#include <QVariant>
#include <QIcon>
#include <QDate>
#include <QLocale>

class QtEnumPropertyType
{
};

Q_DECLARE_METATYPE(QtEnumPropertyType)

class QtFlagPropertyType
{
};

Q_DECLARE_METATYPE(QtFlagPropertyType)

class QtGroupPropertyType
{
};

Q_DECLARE_METATYPE(QtGroupPropertyType)

/*!
    Returns the type id for an enum property.

    Note that the property's value type can be retrieved using the
    valueType() function (which is QVariant::Int for the enum property
    type).

    \sa propertyType(), valueType()
*/
int QtVariantPropertyManager::enumTypeId()
{
    return qMetaTypeId<QtEnumPropertyType>();
}

/*!
    Returns the type id for a flag property.

    Note that the property's value type can be retrieved using the
    valueType() function (which is QVariant::Int for the flag property
    type).

    \sa propertyType(), valueType()
*/
int QtVariantPropertyManager::flagTypeId()
{
    return qMetaTypeId<QtFlagPropertyType>();
}

/*!
    Returns the type id for a group property.

    Note that the property's value type can be retrieved using the
    valueType() function (which is QVariant::Invalid for the group
    property type, since it doesn't provide any value).

    \sa propertyType(), valueType()
*/
int QtVariantPropertyManager::groupTypeId()
{
    return qMetaTypeId<QtGroupPropertyType>();
}

/*!
    Returns the type id for a icon map attribute.

    Note that the property's attribute type can be retrieved using the
    attributeType() function.

    \sa attributeType(), QtEnumPropertyManager::enumIcons()
*/
int QtVariantPropertyManager::iconMapTypeId()
{
    return qMetaTypeId<QtIconMap>();
}

typedef QMap<QtVariantProperty *, QtProperty *> PropertyMap;
Q_GLOBAL_STATIC(PropertyMap, propertyToWrappedProperty)

static QtProperty *wrappedProperty(QtProperty *property)
{
    QMap<QtVariantProperty *, QtProperty *>::ConstIterator it =
            propertyToWrappedProperty()->constBegin();
    while (it != propertyToWrappedProperty()->constEnd()) {
        QtVariantProperty *varProp = it.key();
        if (property == varProp) {
            return it.value();
        }
        it++;
    }
    return 0;
}

class QtVariantPropertyPrivate
{
    QtVariantProperty *q_ptr;
public:
    QtVariantPropertyManager *manager;
};

/*!
    \class QtVariantProperty

    \brief The QtVariantProperty class is a convenience class handling
    QVariant based properties.

    QtVariantProperty provides additional API: A property's type,
    value type, attribute values and current value can easily be
    retrieved using the propertyType(), valueType(), attributeValue()
    and value() functions respectively. In addition, the attribute
    values and the current value can be set using the corresponding
    setValue() and setAttribute() functions.

    For example, instead of writing:

    \code
        QtVariantPropertyManager *variantPropertyManager;
        QtProperty *property;

        variantPropertyManager->setValue(property, 10);
    \endcode

    you can write:

    \code
        QtVariantPropertyManager *variantPropertyManager;
        QtVariantProperty *property;

        property->setValue(10);
    \endcode

    QtVariantProperty instances can only be created by the
    QtVariantPropertyManager class.

    \sa QtProperty, QtVariantPropertyManager, QtVariantEditorFactory
*/

/*!
    Creates a variant property using the given \a manager.

    Do not use this constructor to create variant property instances;
    use the QtVariantPropertyManager::addProperty() function
    instead.  This constructor is used internally by the
    QtVariantPropertyManager::createProperty() function.

    \sa QtVariantPropertyManager
*/
QtVariantProperty::QtVariantProperty(QtVariantPropertyManager *manager)
    : QtProperty(manager)
{
    d_ptr = new QtVariantPropertyPrivate;
    d_ptr->manager = manager;
}

/*!
    Destroys this property.

    \sa QtProperty::~QtProperty()
*/
QtVariantProperty::~QtVariantProperty()
{
    delete d_ptr;
}

/*!
    Returns the property's current value.

    \sa valueType(), setValue()
*/
QVariant QtVariantProperty::value() const
{
    return d_ptr->manager->value(this);
}

/*!
    Returns this property's value for the specified \a attribute.

    QtVariantPropertyManager provides a couple of related functions:
    \l{QtVariantPropertyManager::attributes()}{attributes()} and
    \l{QtVariantPropertyManager::attributeType()}{attributeType()}.

    \sa setAttribute()
*/
QVariant QtVariantProperty::attributeValue(const QString &attribute) const
{
    return d_ptr->manager->attributeValue(this, attribute);
}

/*!
    Returns the type of this property's value.

    \sa propertyType()
*/
int QtVariantProperty::valueType() const
{
    return d_ptr->manager->valueType(this);
}

/*!
    Returns this property's type.

    QtVariantPropertyManager provides several related functions:
    \l{QtVariantPropertyManager::enumTypeId()}{enumTypeId()},
    \l{QtVariantPropertyManager::flagTypeId()}{flagTypeId()} and
    \l{QtVariantPropertyManager::groupTypeId()}{groupTypeId()}.

    \sa valueType()
*/
int QtVariantProperty::propertyType() const
{
    return d_ptr->manager->propertyType(this);
}

/*!
    Sets the value of this property to \a value.

    The specified \a value must be of the type returned by
    valueType(), or of a type that can be converted to valueType()
    using the QVariant::canConvert() function; otherwise this function
    does nothing.

    \sa value()
*/
void QtVariantProperty::setValue(const QVariant &value)
{
    d_ptr->manager->setValue(this, value);
}

/*!
    Sets the \a attribute of property to \a value.

    QtVariantPropertyManager provides the related
    \l{QtVariantPropertyManager::setAttribute()}{setAttribute()}
    function.

    \sa attributeValue()
*/
void QtVariantProperty::setAttribute(const QString &attribute, const QVariant &value)
{
    d_ptr->manager->setAttribute(this, attribute, value);
}

class QtVariantPropertyManagerPrivate
{
    QtVariantPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtVariantPropertyManager)
public:

    bool m_creatingProperty;
    bool m_creatingSubProperties;
    bool m_destroyingSubProperties;
    int m_propertyType;

    void slotValueChanged(QtProperty *property, int val);
    void slotRangeChanged(QtProperty *property, int min, int max);
    void slotValueChanged(QtProperty *property, double val);
    void slotRangeChanged(QtProperty *property, double min, double max);
    void slotDecimalsChanged(QtProperty *property, int prec);
    void slotValueChanged(QtProperty *property, bool val);
    void slotValueChanged(QtProperty *property, const QString &val);
    void slotRegExpChanged(QtProperty *property, const QRegExp &regExp);
    void slotValueChanged(QtProperty *property, const QDate &val);
    void slotRangeChanged(QtProperty *property, const QDate &min, const QDate &max);
    void slotValueChanged(QtProperty *property, const QTime &val);
    void slotValueChanged(QtProperty *property, const QDateTime &val);
    void slotValueChanged(QtProperty *property, const QKeySequence &val);
    void slotValueChanged(QtProperty *property, const QLocale &val);
    void slotValueChanged(QtProperty *property, const QPoint &val);
    void slotValueChanged(QtProperty *property, const QPointF &val);
    void slotValueChanged(QtProperty *property, const QSize &val);
    void slotRangeChanged(QtProperty *property, const QSize &min, const QSize &max);
    void slotValueChanged(QtProperty *property, const QSizeF &val);
    void slotRangeChanged(QtProperty *property, const QSizeF &min, const QSizeF &max);
    void slotValueChanged(QtProperty *property, const QRect &val);
    void slotConstraintChanged(QtProperty *property, const QRect &val);
    void slotValueChanged(QtProperty *property, const QRectF &val);
    void slotConstraintChanged(QtProperty *property, const QRectF &val);
    void slotValueChanged(QtProperty *property, const QColor &val);
    void slotEnumChanged(QtProperty *property, int val);
    void slotEnumNamesChanged(QtProperty *property, const QStringList &enumNames);
    void slotEnumIconsChanged(QtProperty *property, const QMap<int, QIcon> &enumIcons);
    void slotValueChanged(QtProperty *property, const QSizePolicy &val);
    void slotValueChanged(QtProperty *property, const QFont &val);
    void slotValueChanged(QtProperty *property, const QCursor &val);
    void slotFlagChanged(QtProperty *property, int val);
    void slotFlagNamesChanged(QtProperty *property, const QStringList &flagNames);
    void slotPropertyInserted(QtProperty *property, QtProperty *parent, QtProperty *after);
    void slotPropertyRemoved(QtProperty *property, QtProperty *parent);

    void valueChanged(QtProperty *property, const QVariant &val);

    int internalPropertyToType(QtProperty *property) const;
    QtVariantProperty *createSubProperty(QtVariantProperty *parent, QtVariantProperty *after,
            QtProperty *internal);
    void removeSubProperty(QtVariantProperty *property);

    QMap<int, QtAbstractPropertyManager *> m_typeToPropertyManager;
    QMap<int, QMap<QString, int> > m_typeToAttributeToAttributeType;

    QMap<QtVariantProperty *, int> m_propertyToType;
    QMap<int, int> m_typeToValueType;
    QMap<QtProperty *, QtVariantProperty *> m_internalToProperty;
};

int QtVariantPropertyManagerPrivate::internalPropertyToType(QtProperty *property) const
{
    int type = 0;
    QtAbstractPropertyManager *internPropertyManager = property->propertyManager();
    if (qobject_cast<QtIntPropertyManager *>(internPropertyManager))
        type = QVariant::Int;
    else if (qobject_cast<QtEnumPropertyManager *>(internPropertyManager))
        type = QtVariantPropertyManager::enumTypeId();
    else if (qobject_cast<QtBoolPropertyManager *>(internPropertyManager))
        type = QVariant::Bool;
    else if (qobject_cast<QtDoublePropertyManager *>(internPropertyManager))
        type = QVariant::Double;
    return type;
}

QtVariantProperty *QtVariantPropertyManagerPrivate::createSubProperty(QtVariantProperty *parent,
            QtVariantProperty *after, QtProperty *internal)
{
    int type = internalPropertyToType(internal);
    if (!type)
        return 0;

    bool wasCreatingSubProperties = m_creatingSubProperties;
    m_creatingSubProperties = true;

    QtVariantProperty *varChild = q_ptr->addProperty(type, internal->propertyName());

    m_creatingSubProperties = wasCreatingSubProperties;

    varChild->setPropertyName(internal->propertyName());
    varChild->setToolTip(internal->toolTip());
    varChild->setStatusTip(internal->statusTip());
    varChild->setWhatsThis(internal->whatsThis());

    parent->insertSubProperty(varChild, after);

    m_internalToProperty[internal] = varChild;
    (*propertyToWrappedProperty())[varChild] = internal;
    return varChild;
}

void QtVariantPropertyManagerPrivate::removeSubProperty(QtVariantProperty *property)
{
    QtProperty *internChild = wrappedProperty(property);
    bool wasDestroyingSubProperties = m_destroyingSubProperties;
    m_destroyingSubProperties = true;
    delete property;
    m_destroyingSubProperties = wasDestroyingSubProperties;
    m_internalToProperty.remove(internChild);
    propertyToWrappedProperty()->remove(property);
}

void QtVariantPropertyManagerPrivate::slotPropertyInserted(QtProperty *property,
            QtProperty *parent, QtProperty *after)
{
    if (m_creatingProperty)
        return;

    if (!m_internalToProperty.contains(parent))
        return;

    if (after && !m_internalToProperty.contains(after))
        return;

    QtVariantProperty *varParent = m_internalToProperty[parent];
    QtVariantProperty *varAfter = after ? m_internalToProperty[after] : 0;

    createSubProperty(varParent, varAfter, property);
}

void QtVariantPropertyManagerPrivate::slotPropertyRemoved(QtProperty *property, QtProperty *parent)
{
    Q_UNUSED(parent)
    if (!m_internalToProperty.contains(property))
        return;

    QtVariantProperty *varProperty = m_internalToProperty[property];

    removeSubProperty(varProperty);
}

void QtVariantPropertyManagerPrivate::valueChanged(QtProperty *property, const QVariant &val)
{
    if (!m_internalToProperty.contains(property))
        return;
    QtVariantProperty *varProp = m_internalToProperty[property];
    emit q_ptr->valueChanged(varProp, val);
    emit q_ptr->propertyChanged(varProp);
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, int val)
{
    valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotRangeChanged(QtProperty *property, int min, int max)
{
    if (!m_internalToProperty.contains(property))
        return;
    QtVariantProperty *varProp = m_internalToProperty[property];
    emit q_ptr->attributeChanged(varProp, QLatin1String("minimum"), QVariant(min));
    emit q_ptr->attributeChanged(varProp, QLatin1String("maximum"), QVariant(max));
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, double val)
{
    valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotRangeChanged(QtProperty *property, double min, double max)
{
    if (!m_internalToProperty.contains(property))
        return;
    QtVariantProperty *varProp = m_internalToProperty[property];
    emit q_ptr->attributeChanged(varProp, QLatin1String("minimum"), QVariant(min));
    emit q_ptr->attributeChanged(varProp, QLatin1String("maximum"), QVariant(max));
}

void QtVariantPropertyManagerPrivate::slotDecimalsChanged(QtProperty *property, int prec)
{
    if (!m_internalToProperty.contains(property))
        return;
    QtVariantProperty *varProp = m_internalToProperty[property];
    emit q_ptr->attributeChanged(varProp, QLatin1String("decimals"), QVariant(prec));
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, bool val)
{
    valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QString &val)
{
    valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotRegExpChanged(QtProperty *property, const QRegExp &regExp)
{
    if (!m_internalToProperty.contains(property))
        return;
    QtVariantProperty *varProp = m_internalToProperty[property];
    emit q_ptr->attributeChanged(varProp, QLatin1String("regExp"), QVariant(regExp));
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QDate &val)
{
    valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotRangeChanged(QtProperty *property, const QDate &min, const QDate &max)
{
    if (!m_internalToProperty.contains(property))
        return;
    QtVariantProperty *varProp = m_internalToProperty[property];
    emit q_ptr->attributeChanged(varProp, QLatin1String("minimum"), QVariant(min));
    emit q_ptr->attributeChanged(varProp, QLatin1String("maximum"), QVariant(max));
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QTime &val)
{
    valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QDateTime &val)
{
    valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QKeySequence &val)
{
    QVariant v;
    qVariantSetValue(v, val);
    valueChanged(property, v);
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QLocale &val)
{
    valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QPoint &val)
{
    valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QPointF &val)
{
    valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QSize &val)
{
    valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotRangeChanged(QtProperty *property, const QSize &min, const QSize &max)
{
    if (!m_internalToProperty.contains(property))
        return;
    QtVariantProperty *varProp = m_internalToProperty[property];
    emit q_ptr->attributeChanged(varProp, QLatin1String("minimum"), QVariant(min));
    emit q_ptr->attributeChanged(varProp, QLatin1String("maximum"), QVariant(max));
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QSizeF &val)
{
    valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotRangeChanged(QtProperty *property, const QSizeF &min, const QSizeF &max)
{
    if (!m_internalToProperty.contains(property))
        return;
    QtVariantProperty *varProp = m_internalToProperty[property];
    emit q_ptr->attributeChanged(varProp, QLatin1String("minimum"), QVariant(min));
    emit q_ptr->attributeChanged(varProp, QLatin1String("maximum"), QVariant(max));
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QRect &val)
{
    valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotConstraintChanged(QtProperty *property, const QRect &constraint)
{
    if (!m_internalToProperty.contains(property))
        return;
    QtVariantProperty *varProp = m_internalToProperty[property];
    emit q_ptr->attributeChanged(varProp, QLatin1String("constraint"), QVariant(constraint));
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QRectF &val)
{
    valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotConstraintChanged(QtProperty *property, const QRectF &constraint)
{
    if (!m_internalToProperty.contains(property))
        return;
    QtVariantProperty *varProp = m_internalToProperty[property];
    emit q_ptr->attributeChanged(varProp, QLatin1String("constraint"), QVariant(constraint));
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QColor &val)
{
    valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotEnumNamesChanged(QtProperty *property, const QStringList &enumNames)
{
    if (!m_internalToProperty.contains(property))
        return;
    QtVariantProperty *varProp = m_internalToProperty[property];
    emit q_ptr->attributeChanged(varProp, QLatin1String("enumNames"), QVariant(enumNames));
}

void QtVariantPropertyManagerPrivate::slotEnumIconsChanged(QtProperty *property, const QMap<int, QIcon> &enumIcons)
{
    if (!m_internalToProperty.contains(property))
        return;
    QtVariantProperty *varProp = m_internalToProperty[property];
    QVariant v;
    qVariantSetValue(v, enumIcons);
    emit q_ptr->attributeChanged(varProp, QLatin1String("enumIcons"), v);
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QSizePolicy &val)
{
    valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QFont &val)
{
    valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotValueChanged(QtProperty *property, const QCursor &val)
{
    valueChanged(property, QVariant(val));
}

void QtVariantPropertyManagerPrivate::slotFlagNamesChanged(QtProperty *property, const QStringList &flagNames)
{
    if (!m_internalToProperty.contains(property))
        return;
    QtVariantProperty *varProp = m_internalToProperty[property];
    emit q_ptr->attributeChanged(varProp, QLatin1String("flagNames"), QVariant(flagNames));
}

/*!
    \class QtVariantPropertyManager

    \brief The QtVariantPropertyManager class provides and manages QVariant based properties.

    QtVariantPropertyManager provides the addProperty() function which
    creates QtVariantProperty objects. The QtVariantProperty class is
    a convenience class handling QVariant based properties inheriting
    QtProperty. A QtProperty object created by a
    QtVariantPropertyManager instance can be converted into a
    QtVariantProperty object using the variantProperty() function.

    The property's value can be retrieved using the value(), and set
    using the setValue() slot. In addition the property's type, and
    the type of its value, can be retrieved using the propertyType()
    and valueType() functions respectively.

    A property's type is a QVariant::Type enumerator value, and
    usually a property's type is the same as its value type. But for
    some properties the types differ, for example for enums, flags and
    group types in which case QtVariantPropertyManager provides the
    enumTypeId(), flagTypeId() and groupTypeId() functions,
    respectively, to identify their property type (the value types are
    QVariant::Int for the enum and flag types, and QVariant::Invalid
    for the group type).

    Use the isPropertyTypeSupported() function to check if a particular
    property type is supported. The currently supported property types
    are:

    \table
    \header
        \o Property Type
        \o Property Type Id
    \row
        \o int
        \o QVariant::Int
    \row
        \o double
        \o QVariant::Double
    \row
        \o bool
        \o QVariant::Bool
    \row
        \o QString
        \o QVariant::String
    \row
        \o QDate
        \o QVariant::Date
    \row
        \o QTime
        \o QVariant::Time
    \row
        \o QDateTime
        \o QVariant::DateTime
    \row
        \o QKeySequence
        \o QVariant::KeySequence
    \row
        \o QLocale
        \o QVariant::Locale
    \row
        \o QPoint
        \o QVariant::Point
    \row
        \o QPointF
        \o QVariant::PointF
    \row
        \o QSize
        \o QVariant::Size
    \row
        \o QSizeF
        \o QVariant::SizeF
    \row
        \o QRect
        \o QVariant::Rect
    \row
        \o QRectF
        \o QVariant::RectF
    \row
        \o QColor
        \o QVariant::Color
    \row
        \o QSizePolicy
        \o QVariant::SizePolicy
    \row
        \o QFont
        \o QVariant::Font
    \row
        \o QCursor
        \o QVariant::Cursor
    \row
        \o enum
        \o enumTypeId()
    \row
        \o flag
        \o flagTypeId()
    \row
        \o group
        \o groupTypeId()
    \endtable

    Each property type can provide additional attributes,
    e.g. QVariant::Int and QVariant::Double provides minimum and
    maximum values. The currently supported attributes are:

    \table
    \header
        \o Property Type
        \o Attribute Name
        \o Attribute Type
    \row
        \o \c int
        \o minimum
        \o QVariant::Int
    \row
        \o
        \o maximum
        \o QVariant::Int
    \row
        \o \c double
        \o minimum
        \o QVariant::Double
    \row
        \o
        \o maximum
        \o QVariant::Double
    \row
        \o
        \o decimals
        \o QVariant::Int
    \row
        \o QString
        \o regExp
        \o QVariant::RegExp
    \row
        \o QDate
        \o minimum
        \o QVariant::Date
    \row
        \o
        \o maximum
        \o QVariant::Date
    \row
        \o QPointF
        \o decimals
        \o QVariant::Int
    \row
        \o QSize
        \o minimum
        \o QVariant::Size
    \row
        \o
        \o maximum
        \o QVariant::Size
    \row
        \o QSizeF
        \o minimum
        \o QVariant::SizeF
    \row
        \o
        \o maximum
        \o QVariant::SizeF
    \row
        \o
        \o decimals
        \o QVariant::Int
    \row
        \o QRect
        \o constraint
        \o QVariant::Rect
    \row
        \o QRect
        \o constraint
        \o QVariant::RectF
    \row
        \o
        \o decimals
        \o QVariant::Int
    \row
        \o \c enum
        \o enumNames
        \o QVariant::StringList
    \row
        \o
        \o enumIcons
        \o iconMapTypeId()
    \row
        \o \c flag
        \o flagNames
        \o QVariant::StringList
    \endtable

    The attributes for a given property type can be retrieved using
    the attributes() function. Each attribute has a value type which
    can be retrieved using the attributeType() function, and a value
    accessible through the attributeValue() function. In addition, the
    value can be set using the setAttribute() slot.

    QtVariantManager also provides the valueChanged() signal which is
    emitted whenever a property created by this manager change, and
    the attributeChanged() signal which is emitted whenever an
    attribute of such a property changes.

    \sa QtVariantProperty, QtVariantEditorFactory
*/

/*!
    \fn void QtVariantPropertyManager::valueChanged(QtProperty *property, const QVariant &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the
    new \a value as parameters.

    \sa setValue()
*/

/*!
    \fn void QtVariantPropertyManager::attributeChanged(QtProperty *property,
                const QString &attribute, const QVariant &value)

    This signal is emitted whenever an attribute of a property created
    by this manager changes its value, passing a pointer to the \a
    property, the \a attribute and the new \a value as parameters.

    \sa setAttribute()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtVariantPropertyManager::QtVariantPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtVariantPropertyManagerPrivate;
    d_ptr->q_ptr = this;

    d_ptr->m_creatingProperty = false;
    d_ptr->m_creatingSubProperties = false;
    d_ptr->m_destroyingSubProperties = false;
    d_ptr->m_propertyType = 0;

    // IntPropertyManager
    QtIntPropertyManager *intPropertyManager = new QtIntPropertyManager(this);
    d_ptr->m_typeToPropertyManager[QVariant::Int] = intPropertyManager;
    d_ptr->m_typeToAttributeToAttributeType[QVariant::Int][QLatin1String("minimum")] = QVariant::Int;
    d_ptr->m_typeToAttributeToAttributeType[QVariant::Int][QLatin1String("maximum")] = QVariant::Int;
    d_ptr->m_typeToValueType[QVariant::Int] = QVariant::Int;
    connect(intPropertyManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotValueChanged(QtProperty *, int)));
    connect(intPropertyManager, SIGNAL(rangeChanged(QtProperty *, int, int)),
                this, SLOT(slotRangeChanged(QtProperty *, int, int)));
    // DoublePropertyManager
    QtDoublePropertyManager *doublePropertyManager = new QtDoublePropertyManager(this);
    d_ptr->m_typeToPropertyManager[QVariant::Double] = doublePropertyManager;
    d_ptr->m_typeToAttributeToAttributeType[QVariant::Double][QLatin1String("minimum")] =
            QVariant::Double;
    d_ptr->m_typeToAttributeToAttributeType[QVariant::Double][QLatin1String("maximum")] =
            QVariant::Double;
    d_ptr->m_typeToAttributeToAttributeType[QVariant::Double][QLatin1String("decimals")] =
            QVariant::Int;
    d_ptr->m_typeToValueType[QVariant::Double] = QVariant::Double;
    connect(doublePropertyManager, SIGNAL(valueChanged(QtProperty *, double)),
                this, SLOT(slotValueChanged(QtProperty *, double)));
    connect(doublePropertyManager, SIGNAL(rangeChanged(QtProperty *, double, double)),
                this, SLOT(slotRangeChanged(QtProperty *, double, double)));
    connect(doublePropertyManager, SIGNAL(decimalsChanged(QtProperty *, int)),
                this, SLOT(slotDecimalsChanged(QtProperty *, int)));
    // BoolPropertyManager
    QtBoolPropertyManager *boolPropertyManager = new QtBoolPropertyManager(this);
    d_ptr->m_typeToPropertyManager[QVariant::Bool] = boolPropertyManager;
    d_ptr->m_typeToValueType[QVariant::Bool] = QVariant::Bool;
    connect(boolPropertyManager, SIGNAL(valueChanged(QtProperty *, bool)),
                this, SLOT(slotValueChanged(QtProperty *, bool)));
    // StringPropertyManager
    QtStringPropertyManager *stringPropertyManager = new QtStringPropertyManager(this);
    d_ptr->m_typeToPropertyManager[QVariant::String] = stringPropertyManager;
    d_ptr->m_typeToValueType[QVariant::String] = QVariant::String;
    d_ptr->m_typeToAttributeToAttributeType[QVariant::String][QLatin1String("regExp")] =
            QVariant::RegExp;
    connect(stringPropertyManager, SIGNAL(valueChanged(QtProperty *, const QString &)),
                this, SLOT(slotValueChanged(QtProperty *, const QString &)));
    connect(stringPropertyManager, SIGNAL(regExpChanged(QtProperty *, const QRegExp &)),
                this, SLOT(slotRegExpChanged(QtProperty *, const QRegExp &)));
    // DatePropertyManager
    QtDatePropertyManager *datePropertyManager = new QtDatePropertyManager(this);
    d_ptr->m_typeToPropertyManager[QVariant::Date] = datePropertyManager;
    d_ptr->m_typeToValueType[QVariant::Date] = QVariant::Date;
    d_ptr->m_typeToAttributeToAttributeType[QVariant::Date][QLatin1String("minimum")] =
            QVariant::Date;
    d_ptr->m_typeToAttributeToAttributeType[QVariant::Date][QLatin1String("maximum")] =
            QVariant::Date;
    connect(datePropertyManager, SIGNAL(valueChanged(QtProperty *, const QDate &)),
                this, SLOT(slotValueChanged(QtProperty *, const QDate &)));
    connect(datePropertyManager, SIGNAL(rangeChanged(QtProperty *, const QDate &, const QDate &)),
                this, SLOT(slotRangeChanged(QtProperty *, const QDate &, const QDate &)));
    // TimePropertyManager
    QtTimePropertyManager *timePropertyManager = new QtTimePropertyManager(this);
    d_ptr->m_typeToPropertyManager[QVariant::Time] = timePropertyManager;
    d_ptr->m_typeToValueType[QVariant::Time] = QVariant::Time;
    connect(timePropertyManager, SIGNAL(valueChanged(QtProperty *, const QTime &)),
                this, SLOT(slotValueChanged(QtProperty *, const QTime &)));
    // DateTimePropertyManager
    QtDateTimePropertyManager *dateTimePropertyManager = new QtDateTimePropertyManager(this);
    d_ptr->m_typeToPropertyManager[QVariant::DateTime] = dateTimePropertyManager;
    d_ptr->m_typeToValueType[QVariant::DateTime] = QVariant::DateTime;
    connect(dateTimePropertyManager, SIGNAL(valueChanged(QtProperty *, const QDateTime &)),
                this, SLOT(slotValueChanged(QtProperty *, const QDateTime &)));
    // KeySequencePropertyManager
    QtKeySequencePropertyManager *keySequencePropertyManager = new QtKeySequencePropertyManager(this);
    d_ptr->m_typeToPropertyManager[QVariant::KeySequence] = keySequencePropertyManager;
    d_ptr->m_typeToValueType[QVariant::KeySequence] = QVariant::KeySequence;
    connect(keySequencePropertyManager, SIGNAL(valueChanged(QtProperty *, const QKeySequence &)),
                this, SLOT(slotValueChanged(QtProperty *, const QKeySequence &)));
    // LocalePropertyManager
    QtLocalePropertyManager *localePropertyManager = new QtLocalePropertyManager(this);
    d_ptr->m_typeToPropertyManager[QVariant::Locale] = localePropertyManager;
    d_ptr->m_typeToValueType[QVariant::Locale] = QVariant::Locale;
    connect(localePropertyManager, SIGNAL(valueChanged(QtProperty *, const QLocale &)),
                this, SLOT(slotValueChanged(QtProperty *, const QLocale &)));
    connect(localePropertyManager->subEnumPropertyManager(), SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotValueChanged(QtProperty *, int)));
    connect(localePropertyManager, SIGNAL(propertyInserted(QtProperty *, QtProperty *, QtProperty *)),
                this, SLOT(slotPropertyInserted(QtProperty *, QtProperty *, QtProperty *)));
    connect(localePropertyManager, SIGNAL(propertyRemoved(QtProperty *, QtProperty *)),
                this, SLOT(slotPropertyRemoved(QtProperty *, QtProperty *)));
    // PointPropertyManager
    QtPointPropertyManager *pointPropertyManager = new QtPointPropertyManager(this);
    d_ptr->m_typeToPropertyManager[QVariant::Point] = pointPropertyManager;
    d_ptr->m_typeToValueType[QVariant::Point] = QVariant::Point;
    connect(pointPropertyManager, SIGNAL(valueChanged(QtProperty *, const QPoint &)),
                this, SLOT(slotValueChanged(QtProperty *, const QPoint &)));
    connect(pointPropertyManager->subIntPropertyManager(), SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotValueChanged(QtProperty *, int)));
    connect(pointPropertyManager, SIGNAL(propertyInserted(QtProperty *, QtProperty *, QtProperty *)),
                this, SLOT(slotPropertyInserted(QtProperty *, QtProperty *, QtProperty *)));
    connect(pointPropertyManager, SIGNAL(propertyRemoved(QtProperty *, QtProperty *)),
                this, SLOT(slotPropertyRemoved(QtProperty *, QtProperty *)));
    // PointFPropertyManager
    QtPointFPropertyManager *pointFPropertyManager = new QtPointFPropertyManager(this);
    d_ptr->m_typeToPropertyManager[QVariant::PointF] = pointFPropertyManager;
    d_ptr->m_typeToValueType[QVariant::PointF] = QVariant::PointF;
    d_ptr->m_typeToAttributeToAttributeType[QVariant::PointF][QLatin1String("decimals")] =
            QVariant::Int;
    connect(pointFPropertyManager, SIGNAL(valueChanged(QtProperty *, const QPointF &)),
                this, SLOT(slotValueChanged(QtProperty *, const QPointF &)));
    connect(pointFPropertyManager, SIGNAL(decimalsChanged(QtProperty *, int)),
                this, SLOT(slotDecimalsChanged(QtProperty *, int)));
    connect(pointFPropertyManager->subDoublePropertyManager(), SIGNAL(valueChanged(QtProperty *, double)),
                this, SLOT(slotValueChanged(QtProperty *, double)));
    connect(pointFPropertyManager, SIGNAL(propertyInserted(QtProperty *, QtProperty *, QtProperty *)),
                this, SLOT(slotPropertyInserted(QtProperty *, QtProperty *, QtProperty *)));
    connect(pointFPropertyManager, SIGNAL(propertyRemoved(QtProperty *, QtProperty *)),
                this, SLOT(slotPropertyRemoved(QtProperty *, QtProperty *)));
    // SizePropertyManager
    QtSizePropertyManager *sizePropertyManager = new QtSizePropertyManager(this);
    d_ptr->m_typeToPropertyManager[QVariant::Size] = sizePropertyManager;
    d_ptr->m_typeToValueType[QVariant::Size] = QVariant::Size;
    d_ptr->m_typeToAttributeToAttributeType[QVariant::Size][QLatin1String("minimum")] =
            QVariant::Size;
    d_ptr->m_typeToAttributeToAttributeType[QVariant::Size][QLatin1String("maximum")] =
            QVariant::Size;
    connect(sizePropertyManager, SIGNAL(valueChanged(QtProperty *, const QSize &)),
                this, SLOT(slotValueChanged(QtProperty *, const QSize &)));
    connect(sizePropertyManager, SIGNAL(rangeChanged(QtProperty *, const QSize &, const QSize &)),
                this, SLOT(slotRangeChanged(QtProperty *, const QSize &, const QSize &)));
    connect(sizePropertyManager->subIntPropertyManager(), SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotValueChanged(QtProperty *, int)));
    connect(sizePropertyManager->subIntPropertyManager(), SIGNAL(rangeChanged(QtProperty *, int, int)),
                this, SLOT(slotRangeChanged(QtProperty *, int, int)));
    connect(sizePropertyManager, SIGNAL(propertyInserted(QtProperty *, QtProperty *, QtProperty *)),
                this, SLOT(slotPropertyInserted(QtProperty *, QtProperty *, QtProperty *)));
    connect(sizePropertyManager, SIGNAL(propertyRemoved(QtProperty *, QtProperty *)),
                this, SLOT(slotPropertyRemoved(QtProperty *, QtProperty *)));
    // SizeFPropertyManager
    QtSizeFPropertyManager *sizeFPropertyManager = new QtSizeFPropertyManager(this);
    d_ptr->m_typeToPropertyManager[QVariant::SizeF] = sizeFPropertyManager;
    d_ptr->m_typeToValueType[QVariant::SizeF] = QVariant::SizeF;
    d_ptr->m_typeToAttributeToAttributeType[QVariant::SizeF][QLatin1String("minimum")] =
            QVariant::SizeF;
    d_ptr->m_typeToAttributeToAttributeType[QVariant::SizeF][QLatin1String("maximum")] =
            QVariant::SizeF;
    d_ptr->m_typeToAttributeToAttributeType[QVariant::SizeF][QLatin1String("decimals")] =
            QVariant::Int;
    connect(sizeFPropertyManager, SIGNAL(valueChanged(QtProperty *, const QSizeF &)),
                this, SLOT(slotValueChanged(QtProperty *, const QSizeF &)));
    connect(sizeFPropertyManager, SIGNAL(rangeChanged(QtProperty *, const QSizeF &, const QSizeF &)),
                this, SLOT(slotRangeChanged(QtProperty *, const QSizeF &, const QSizeF &)));
    connect(sizeFPropertyManager, SIGNAL(decimalsChanged(QtProperty *, int)),
                this, SLOT(slotDecimalsChanged(QtProperty *, int)));
    connect(sizeFPropertyManager->subDoublePropertyManager(), SIGNAL(valueChanged(QtProperty *, double)),
                this, SLOT(slotValueChanged(QtProperty *, double)));
    connect(sizeFPropertyManager->subDoublePropertyManager(), SIGNAL(rangeChanged(QtProperty *, double, double)),
                this, SLOT(slotRangeChanged(QtProperty *, double, double)));
    connect(sizeFPropertyManager, SIGNAL(propertyInserted(QtProperty *, QtProperty *, QtProperty *)),
                this, SLOT(slotPropertyInserted(QtProperty *, QtProperty *, QtProperty *)));
    connect(sizeFPropertyManager, SIGNAL(propertyRemoved(QtProperty *, QtProperty *)),
                this, SLOT(slotPropertyRemoved(QtProperty *, QtProperty *)));
    // RectPropertyManager
    QtRectPropertyManager *rectPropertyManager = new QtRectPropertyManager(this);
    d_ptr->m_typeToPropertyManager[QVariant::Rect] = rectPropertyManager;
    d_ptr->m_typeToValueType[QVariant::Rect] = QVariant::Rect;
    d_ptr->m_typeToAttributeToAttributeType[QVariant::Rect][QLatin1String("constraint")] =
            QVariant::Rect;
    connect(rectPropertyManager, SIGNAL(valueChanged(QtProperty *, const QRect &)),
                this, SLOT(slotValueChanged(QtProperty *, const QRect &)));
    connect(rectPropertyManager, SIGNAL(constraintChanged(QtProperty *, const QRect &)),
                this, SLOT(slotConstraintChanged(QtProperty *, const QRect &)));
    connect(rectPropertyManager->subIntPropertyManager(), SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotValueChanged(QtProperty *, int)));
    connect(rectPropertyManager->subIntPropertyManager(), SIGNAL(rangeChanged(QtProperty *, int, int)),
                this, SLOT(slotRangeChanged(QtProperty *, int, int)));
    connect(rectPropertyManager, SIGNAL(propertyInserted(QtProperty *, QtProperty *, QtProperty *)),
                this, SLOT(slotPropertyInserted(QtProperty *, QtProperty *, QtProperty *)));
    connect(rectPropertyManager, SIGNAL(propertyRemoved(QtProperty *, QtProperty *)),
                this, SLOT(slotPropertyRemoved(QtProperty *, QtProperty *)));
    // RectFPropertyManager
    QtRectFPropertyManager *rectFPropertyManager = new QtRectFPropertyManager(this);
    d_ptr->m_typeToPropertyManager[QVariant::RectF] = rectFPropertyManager;
    d_ptr->m_typeToValueType[QVariant::RectF] = QVariant::RectF;
    d_ptr->m_typeToAttributeToAttributeType[QVariant::RectF][QLatin1String("constraint")] =
            QVariant::RectF;
    d_ptr->m_typeToAttributeToAttributeType[QVariant::RectF][QLatin1String("decimals")] =
            QVariant::Int;
    connect(rectFPropertyManager, SIGNAL(valueChanged(QtProperty *, const QRectF &)),
                this, SLOT(slotValueChanged(QtProperty *, const QRectF &)));
    connect(rectFPropertyManager, SIGNAL(constraintChanged(QtProperty *, const QRectF &)),
                this, SLOT(slotConstraintChanged(QtProperty *, const QRectF &)));
    connect(rectFPropertyManager, SIGNAL(decimalsChanged(QtProperty *, int)),
                this, SLOT(slotDecimalsChanged(QtProperty *, int)));
    connect(rectFPropertyManager->subDoublePropertyManager(), SIGNAL(valueChanged(QtProperty *, double)),
                this, SLOT(slotValueChanged(QtProperty *, double)));
    connect(rectFPropertyManager->subDoublePropertyManager(), SIGNAL(rangeChanged(QtProperty *, double, double)),
                this, SLOT(slotRangeChanged(QtProperty *, double, double)));
    connect(rectFPropertyManager, SIGNAL(propertyInserted(QtProperty *, QtProperty *, QtProperty *)),
                this, SLOT(slotPropertyInserted(QtProperty *, QtProperty *, QtProperty *)));
    connect(rectFPropertyManager, SIGNAL(propertyRemoved(QtProperty *, QtProperty *)),
                this, SLOT(slotPropertyRemoved(QtProperty *, QtProperty *)));
    // ColorPropertyManager
    QtColorPropertyManager *colorPropertyManager = new QtColorPropertyManager(this);
    d_ptr->m_typeToPropertyManager[QVariant::Color] = colorPropertyManager;
    d_ptr->m_typeToValueType[QVariant::Color] = QVariant::Color;
    connect(colorPropertyManager, SIGNAL(valueChanged(QtProperty *, const QColor &)),
                this, SLOT(slotValueChanged(QtProperty *, const QColor &)));
    connect(colorPropertyManager->subIntPropertyManager(), SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotValueChanged(QtProperty *, int)));
    connect(colorPropertyManager, SIGNAL(propertyInserted(QtProperty *, QtProperty *, QtProperty *)),
                this, SLOT(slotPropertyInserted(QtProperty *, QtProperty *, QtProperty *)));
    connect(colorPropertyManager, SIGNAL(propertyRemoved(QtProperty *, QtProperty *)),
                this, SLOT(slotPropertyRemoved(QtProperty *, QtProperty *)));
    // EnumPropertyManager
    int enumId = enumTypeId();
    QtEnumPropertyManager *enumPropertyManager = new QtEnumPropertyManager(this);
    d_ptr->m_typeToPropertyManager[enumId] = enumPropertyManager;
    d_ptr->m_typeToValueType[enumId] = QVariant::Int;
    d_ptr->m_typeToAttributeToAttributeType[enumId][QLatin1String("enumNames")] =
            QVariant::StringList;
    d_ptr->m_typeToAttributeToAttributeType[enumId][QLatin1String("enumIcons")] =
            iconMapTypeId();
    connect(enumPropertyManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotValueChanged(QtProperty *, int)));
    connect(enumPropertyManager, SIGNAL(enumNamesChanged(QtProperty *, const QStringList &)),
                this, SLOT(slotEnumNamesChanged(QtProperty *, const QStringList &)));
    connect(enumPropertyManager, SIGNAL(enumIconsChanged(QtProperty *, const QMap<int, QIcon> &)),
                this, SLOT(slotEnumIconsChanged(QtProperty *, const QMap<int, QIcon> &)));
    // SizePolicyPropertyManager
    QtSizePolicyPropertyManager *sizePolicyPropertyManager = new QtSizePolicyPropertyManager(this);
    d_ptr->m_typeToPropertyManager[QVariant::SizePolicy] = sizePolicyPropertyManager;
    d_ptr->m_typeToValueType[QVariant::SizePolicy] = QVariant::SizePolicy;
    connect(sizePolicyPropertyManager, SIGNAL(valueChanged(QtProperty *, const QSizePolicy &)),
                this, SLOT(slotValueChanged(QtProperty *, const QSizePolicy &)));
    connect(sizePolicyPropertyManager->subIntPropertyManager(), SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotValueChanged(QtProperty *, int)));
    connect(sizePolicyPropertyManager->subIntPropertyManager(), SIGNAL(rangeChanged(QtProperty *, int, int)),
                this, SLOT(slotRangeChanged(QtProperty *, int, int)));
    connect(sizePolicyPropertyManager->subEnumPropertyManager(), SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotValueChanged(QtProperty *, int)));
    connect(sizePolicyPropertyManager->subEnumPropertyManager(),
                SIGNAL(enumNamesChanged(QtProperty *, const QStringList &)),
                this, SLOT(slotEnumNamesChanged(QtProperty *, const QStringList &)));
    connect(sizePolicyPropertyManager, SIGNAL(propertyInserted(QtProperty *, QtProperty *, QtProperty *)),
                this, SLOT(slotPropertyInserted(QtProperty *, QtProperty *, QtProperty *)));
    connect(sizePolicyPropertyManager, SIGNAL(propertyRemoved(QtProperty *, QtProperty *)),
                this, SLOT(slotPropertyRemoved(QtProperty *, QtProperty *)));
    // FontPropertyManager
    QtFontPropertyManager *fontPropertyManager = new QtFontPropertyManager(this);
    d_ptr->m_typeToPropertyManager[QVariant::Font] = fontPropertyManager;
    d_ptr->m_typeToValueType[QVariant::Font] = QVariant::Font;
    connect(fontPropertyManager, SIGNAL(valueChanged(QtProperty *, const QFont &)),
                this, SLOT(slotValueChanged(QtProperty *, const QFont &)));
    connect(fontPropertyManager->subIntPropertyManager(), SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotValueChanged(QtProperty *, int)));
    connect(fontPropertyManager->subIntPropertyManager(), SIGNAL(rangeChanged(QtProperty *, int, int)),
                this, SLOT(slotRangeChanged(QtProperty *, int, int)));
    connect(fontPropertyManager->subEnumPropertyManager(), SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotValueChanged(QtProperty *, int)));
    connect(fontPropertyManager->subEnumPropertyManager(),
                SIGNAL(enumNamesChanged(QtProperty *, const QStringList &)),
                this, SLOT(slotEnumNamesChanged(QtProperty *, const QStringList &)));
    connect(fontPropertyManager->subBoolPropertyManager(), SIGNAL(valueChanged(QtProperty *, bool)),
                this, SLOT(slotValueChanged(QtProperty *, bool)));
    connect(fontPropertyManager, SIGNAL(propertyInserted(QtProperty *, QtProperty *, QtProperty *)),
                this, SLOT(slotPropertyInserted(QtProperty *, QtProperty *, QtProperty *)));
    connect(fontPropertyManager, SIGNAL(propertyRemoved(QtProperty *, QtProperty *)),
                this, SLOT(slotPropertyRemoved(QtProperty *, QtProperty *)));
    // CursorPropertyManager
    QtCursorPropertyManager *cursorPropertyManager = new QtCursorPropertyManager(this);
    d_ptr->m_typeToPropertyManager[QVariant::Cursor] = cursorPropertyManager;
    d_ptr->m_typeToValueType[QVariant::Cursor] = QVariant::Cursor;
    connect(cursorPropertyManager, SIGNAL(valueChanged(QtProperty *, const QCursor &)),
                this, SLOT(slotValueChanged(QtProperty *, const QCursor &)));
    // FlagPropertyManager
    int flagId = flagTypeId();
    QtFlagPropertyManager *flagPropertyManager = new QtFlagPropertyManager(this);
    d_ptr->m_typeToPropertyManager[flagId] = flagPropertyManager;
    d_ptr->m_typeToValueType[flagId] = QVariant::Int;
    d_ptr->m_typeToAttributeToAttributeType[flagId][QLatin1String("flagNames")] =
            QVariant::StringList;
    connect(flagPropertyManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotValueChanged(QtProperty *, int)));
    connect(flagPropertyManager, SIGNAL(flagNamesChanged(QtProperty *, const QStringList &)),
                this, SLOT(slotFlagNamesChanged(QtProperty *, const QStringList &)));
    connect(flagPropertyManager->subBoolPropertyManager(), SIGNAL(valueChanged(QtProperty *, bool)),
                this, SLOT(slotValueChanged(QtProperty *, bool)));
    connect(flagPropertyManager, SIGNAL(propertyInserted(QtProperty *, QtProperty *, QtProperty *)),
                this, SLOT(slotPropertyInserted(QtProperty *, QtProperty *, QtProperty *)));
    connect(flagPropertyManager, SIGNAL(propertyRemoved(QtProperty *, QtProperty *)),
                this, SLOT(slotPropertyRemoved(QtProperty *, QtProperty *)));
    // FlagPropertyManager
    int groupId = groupTypeId();
    QtGroupPropertyManager *groupPropertyManager = new QtGroupPropertyManager(this);
    d_ptr->m_typeToPropertyManager[groupId] = groupPropertyManager;
    d_ptr->m_typeToValueType[groupId] = QVariant::Invalid;
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtVariantPropertyManager::~QtVariantPropertyManager()
{
    delete d_ptr;
}

/*!
    Returns the given \a property converted into a QtVariantProperty.

    If the \a property was not created by this variant manager, the
    function returns 0.

    \sa createProperty()
*/
QtVariantProperty *QtVariantPropertyManager::variantProperty(const QtProperty *property) const
{
    QMap<QtVariantProperty *, int>::ConstIterator it =
            d_ptr->m_propertyToType.constBegin();
    while (it != d_ptr->m_propertyToType.constEnd()) {
        QtVariantProperty *varProp = it.key();
        if (property == varProp) {
            return varProp;
        }
        it++;
    }
    return 0;
}

/*!
    Returns true if the given \a propertyType is supported by this
    variant manager; otherwise false.

    \sa propertyType()
*/
bool QtVariantPropertyManager::isPropertyTypeSupported(int propertyType) const
{
    if (d_ptr->m_typeToValueType.contains(propertyType))
        return true;
    return false;
}

/*!
   Creates and returns a variant property of the given \a propertyType
   with the given \a name.

   If the specified \a propertyType is not supported by this variant
   manager, this function returns 0.

   Do not use the inherited
   QtAbstractPropertyManager::addProperty() function to create a
   variant property (that function will always return 0 since it will
   not be clear what type the property should have).

    \sa isPropertyTypeSupported()
*/
QtVariantProperty *QtVariantPropertyManager::addProperty(int propertyType, const QString &name)
{
    if (!isPropertyTypeSupported(propertyType))
        return 0;

    bool wasCreating = d_ptr->m_creatingProperty;
    d_ptr->m_creatingProperty = true;
    d_ptr->m_propertyType = propertyType;
    QtProperty *property = QtAbstractPropertyManager::addProperty(name);
    d_ptr->m_creatingProperty = wasCreating;
    d_ptr->m_propertyType = 0;

    if (!property)
        return 0;

    return variantProperty(property);
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by this manager, this
    function returns an invalid variant.

    \sa setValue()
*/
QVariant QtVariantPropertyManager::value(const QtProperty *property) const
{
    QMap<QtVariantProperty *, QtProperty *>::ConstIterator it =
            propertyToWrappedProperty()->constBegin();
    while (it != propertyToWrappedProperty()->constEnd()) {
        QtVariantProperty *varProp = it.key();
        if (property == varProp) {
            QtProperty *internProp = it.value();

            QtAbstractPropertyManager *manager = internProp->propertyManager();
            if (QtIntPropertyManager *intManager = qobject_cast<QtIntPropertyManager *>(manager)) {
                return intManager->value(internProp);
            } else if (QtDoublePropertyManager *doubleManager = qobject_cast<QtDoublePropertyManager *>(manager)) {
                return doubleManager->value(internProp);
            } else if (QtBoolPropertyManager *boolManager = qobject_cast<QtBoolPropertyManager *>(manager)) {
                return boolManager->value(internProp);
            } else if (QtStringPropertyManager *stringManager = qobject_cast<QtStringPropertyManager *>(manager)) {
                return stringManager->value(internProp);
            } else if (QtDatePropertyManager *dateManager = qobject_cast<QtDatePropertyManager *>(manager)) {
                return dateManager->value(internProp);
            } else if (QtTimePropertyManager *timeManager = qobject_cast<QtTimePropertyManager *>(manager)) {
                return timeManager->value(internProp);
            } else if (QtDateTimePropertyManager *dateTimeManager = qobject_cast<QtDateTimePropertyManager *>(manager)) {
                return dateTimeManager->value(internProp);
            } else if (QtKeySequencePropertyManager *keySequenceManager = qobject_cast<QtKeySequencePropertyManager *>(manager)) {
                return keySequenceManager->value(internProp);
            } else if (QtLocalePropertyManager *localeManager = qobject_cast<QtLocalePropertyManager *>(manager)) {
                return localeManager->value(internProp);
            } else if (QtPointPropertyManager *pointManager = qobject_cast<QtPointPropertyManager *>(manager)) {
                return pointManager->value(internProp);
            } else if (QtPointFPropertyManager *pointFManager = qobject_cast<QtPointFPropertyManager *>(manager)) {
                return pointFManager->value(internProp);
            } else if (QtSizePropertyManager *sizeManager = qobject_cast<QtSizePropertyManager *>(manager)) {
                return sizeManager->value(internProp);
            } else if (QtSizeFPropertyManager *sizeFManager = qobject_cast<QtSizeFPropertyManager *>(manager)) {
                return sizeFManager->value(internProp);
            } else if (QtRectPropertyManager *rectManager = qobject_cast<QtRectPropertyManager *>(manager)) {
                return rectManager->value(internProp);
            } else if (QtRectFPropertyManager *rectFManager = qobject_cast<QtRectFPropertyManager *>(manager)) {
                return rectFManager->value(internProp);
            } else if (QtColorPropertyManager *colorManager = qobject_cast<QtColorPropertyManager *>(manager)) {
                return colorManager->value(internProp);
            } else if (QtEnumPropertyManager *enumManager = qobject_cast<QtEnumPropertyManager *>(manager)) {
                return enumManager->value(internProp);
            } else if (QtSizePolicyPropertyManager *sizePolicyManager =
                            qobject_cast<QtSizePolicyPropertyManager *>(manager)) {
                return sizePolicyManager->value(internProp);
            } else if (QtFontPropertyManager *fontManager = qobject_cast<QtFontPropertyManager *>(manager)) {
                return fontManager->value(internProp);
            } else if (QtCursorPropertyManager *cursorManager = qobject_cast<QtCursorPropertyManager *>(manager)) {
                return cursorManager->value(internProp);
            } else if (QtFlagPropertyManager *flagManager = qobject_cast<QtFlagPropertyManager *>(manager)) {
                return flagManager->value(internProp);
            }
        }
        it++;
    }
    return QVariant();
}

/*!
    Returns the given \a property's value type.

    \sa propertyType()
*/
int QtVariantPropertyManager::valueType(const QtProperty *property) const
{
    int propType = propertyType(property);
    return valueType(propType);
}

/*!
    \overload

    Returns the value type associated with the given \a propertyType.
*/
int QtVariantPropertyManager::valueType(int propertyType) const
{
    if (d_ptr->m_typeToValueType.contains(propertyType))
        return d_ptr->m_typeToValueType[propertyType];
    return 0;
}

/*!
    Returns the given \a property's type.

    \sa valueType()
*/
int QtVariantPropertyManager::propertyType(const QtProperty *property) const
{
    QMap<QtVariantProperty *, int>::ConstIterator it = d_ptr->m_propertyToType.constBegin();
    while (it != d_ptr->m_propertyToType.constEnd()) {
        if (it.key() == property)
            return it.value();
        it++;
    }
    return 0;
}

/*!
    Returns the given \a property's value for the specified \a
    attribute

    If the given \a property was not created by \e this manager, or if
    the specified \a attribute does not exist, this function returns
    an invalid variant.

    \sa attributes(), attributeType(), setAttribute()
*/
QVariant QtVariantPropertyManager::attributeValue(const QtProperty *property, const QString &attribute)
{
    int propType = propertyType(property);
    if (!propType)
        return QVariant();

    QMap<int, QMap<QString, int> >::ConstIterator it =
            d_ptr->m_typeToAttributeToAttributeType.find(propType);
    if (it == d_ptr->m_typeToAttributeToAttributeType.constEnd())
        return QVariant();

    QMap<QString, int> attributes = it.value();
    QMap<QString, int>::ConstIterator itAttr = attributes.find(attribute);
    if (itAttr == attributes.constEnd())
        return QVariant();

    QMap<QtVariantProperty *, QtProperty *>::ConstIterator itVar =
            propertyToWrappedProperty()->constBegin();
    while (itVar != propertyToWrappedProperty()->constEnd()) {
        QtVariantProperty *varProp = itVar.key();
        if (property == varProp) {
            QtProperty *internProp = itVar.value();

            QtAbstractPropertyManager *manager = internProp->propertyManager();
            if (QtIntPropertyManager *intManager = qobject_cast<QtIntPropertyManager *>(manager)) {
                if (attribute == QLatin1String("maximum"))
                    return intManager->maximum(internProp);
                if (attribute == QLatin1String("minimum"))
                    return intManager->minimum(internProp);
                return QVariant();
            } else if (QtDoublePropertyManager *doubleManager = qobject_cast<QtDoublePropertyManager *>(manager)) {
                if (attribute == QLatin1String("maximum"))
                    return doubleManager->maximum(internProp);
                if (attribute == QLatin1String("minimum"))
                    return doubleManager->minimum(internProp);
                if (attribute == QLatin1String("decimals"))
                    return doubleManager->decimals(internProp);
                return QVariant();
            } else if (QtStringPropertyManager *stringManager = qobject_cast<QtStringPropertyManager *>(manager)) {
                if (attribute == QLatin1String("regExp"))
                    return stringManager->regExp(internProp);
                return QVariant();
            } else if (QtDatePropertyManager *dateManager = qobject_cast<QtDatePropertyManager *>(manager)) {
                if (attribute == QLatin1String("maximum"))
                    return dateManager->maximum(internProp);
                if (attribute == QLatin1String("minimum"))
                    return dateManager->minimum(internProp);
                return QVariant();
            } else if (QtPointFPropertyManager *pointFManager = qobject_cast<QtPointFPropertyManager *>(manager)) {
                if (attribute == QLatin1String("decimals"))
                    return pointFManager->decimals(internProp);
                return QVariant();
            } else if (QtSizePropertyManager *sizeManager = qobject_cast<QtSizePropertyManager *>(manager)) {
                if (attribute == QLatin1String("maximum"))
                    return sizeManager->maximum(internProp);
                if (attribute == QLatin1String("minimum"))
                    return sizeManager->minimum(internProp);
                return QVariant();
            } else if (QtSizeFPropertyManager *sizeFManager = qobject_cast<QtSizeFPropertyManager *>(manager)) {
                if (attribute == QLatin1String("maximum"))
                    return sizeFManager->maximum(internProp);
                if (attribute == QLatin1String("minimum"))
                    return sizeFManager->minimum(internProp);
                if (attribute == QLatin1String("decimals"))
                    return sizeFManager->decimals(internProp);
                return QVariant();
            } else if (QtRectPropertyManager *rectManager = qobject_cast<QtRectPropertyManager *>(manager)) {
                if (attribute == QLatin1String("constraint"))
                    return rectManager->constraint(internProp);
                return QVariant();
            } else if (QtRectFPropertyManager *rectFManager = qobject_cast<QtRectFPropertyManager *>(manager)) {
                if (attribute == QLatin1String("constraint"))
                    return rectFManager->constraint(internProp);
                if (attribute == QLatin1String("decimals"))
                    return rectFManager->decimals(internProp);
                return QVariant();
            } else if (QtEnumPropertyManager *enumManager = qobject_cast<QtEnumPropertyManager *>(manager)) {
                if (attribute == QLatin1String("enumNames"))
                    return enumManager->enumNames(internProp);
                if (attribute == QLatin1String("enumIcons")) {
                    QVariant v;
                    qVariantSetValue(v, enumManager->enumIcons(internProp));
                    return v;
                }
                return QVariant();
            } else if (QtFlagPropertyManager *flagManager = qobject_cast<QtFlagPropertyManager *>(manager)) {
                if (attribute == QLatin1String("flagNames"))
                    return flagManager->flagNames(internProp);
                return QVariant();
            }
        }
        itVar++;
    }
    return QVariant();
}

/*!
    Returns a list of the given \a propertyType 's attributes.

    \sa attributeValue(), attributeType()
*/
QStringList QtVariantPropertyManager::attributes(int propertyType) const
{
    QMap<int, QMap<QString, int> >::ConstIterator it =
            d_ptr->m_typeToAttributeToAttributeType.find(propertyType);
    if (it == d_ptr->m_typeToAttributeToAttributeType.constEnd())
        return QStringList();
    return it.value().keys();
}

/*!
    Returns the type of the specified \a attribute of the given \a
    propertyType.

    If the given \a propertyType is not supported by \e this manager,
    or if the given \a propertyType does not possess the specified \a
    attribute, this function returns QVariant::Invalid.

    \sa attributes(), valueType()
*/
int QtVariantPropertyManager::attributeType(int propertyType, const QString &attribute) const
{
    QMap<int, QMap<QString, int> >::ConstIterator it =
            d_ptr->m_typeToAttributeToAttributeType.find(propertyType);
    if (it == d_ptr->m_typeToAttributeToAttributeType.constEnd())
        return 0;

    QMap<QString, int> attributes = it.value();
    QMap<QString, int>::ConstIterator itAttr = attributes.find(attribute);
    if (itAttr == attributes.constEnd())
        return 0;
    return itAttr.value();
}

/*!
    \fn void QtVariantPropertyManager::setValue(QtProperty *property, const QVariant &value)

    Sets the value of the given \a property to \a value.

    The specified \a value must be of a type returned by valueType(),
    or of type that can be converted to valueType() using the
    QVariant::canConvert() funciton, otherwise this function does
    nothing.

    \sa value(), QtVariantProperty::setValue(), valueChanged()
*/
void QtVariantPropertyManager::setValue(QtProperty *property, const QVariant &val)
{
    int propType = val.userType();
    if (!propType)
        return;

    int valType = valueType(property);

    if (propType != valType && !val.canConvert((QVariant::Type)valType))
        return;

    QMap<QtVariantProperty *, QtProperty *>::ConstIterator it =
            propertyToWrappedProperty()->constBegin();
    while (it != propertyToWrappedProperty()->constEnd()) {
        QtVariantProperty *varProp = it.key();
        if (property == varProp) {
            QtProperty *internProp = it.value();

            QtAbstractPropertyManager *manager = internProp->propertyManager();
            if (QtIntPropertyManager *intManager = qobject_cast<QtIntPropertyManager *>(manager)) {
                intManager->setValue(internProp, qVariantValue<int>(val));
                return;
            } else if (QtDoublePropertyManager *doubleManager = qobject_cast<QtDoublePropertyManager *>(manager)) {
                doubleManager->setValue(internProp, qVariantValue<double>(val));
                return;
            } else if (QtBoolPropertyManager *boolManager = qobject_cast<QtBoolPropertyManager *>(manager)) {
                boolManager->setValue(internProp, qVariantValue<bool>(val));
                return;
            } else if (QtStringPropertyManager *stringManager = qobject_cast<QtStringPropertyManager *>(manager)) {
                stringManager->setValue(internProp, qVariantValue<QString>(val));
                return;
            } else if (QtDatePropertyManager *dateManager = qobject_cast<QtDatePropertyManager *>(manager)) {
                dateManager->setValue(internProp, qVariantValue<QDate>(val));
                return;
            } else if (QtTimePropertyManager *timeManager = qobject_cast<QtTimePropertyManager *>(manager)) {
                timeManager->setValue(internProp, qVariantValue<QTime>(val));
                return;
            } else if (QtDateTimePropertyManager *dateTimeManager = qobject_cast<QtDateTimePropertyManager *>(manager)) {
                dateTimeManager->setValue(internProp, qVariantValue<QDateTime>(val));
                return;
            } else if (QtKeySequencePropertyManager *keySequenceManager = qobject_cast<QtKeySequencePropertyManager *>(manager)) {
                keySequenceManager->setValue(internProp, qVariantValue<QKeySequence>(val));
                return;
            } else if (QtLocalePropertyManager *localeManager = qobject_cast<QtLocalePropertyManager *>(manager)) {
                localeManager->setValue(internProp, qVariantValue<QLocale>(val));
                return;
            } else if (QtPointPropertyManager *pointManager = qobject_cast<QtPointPropertyManager *>(manager)) {
                pointManager->setValue(internProp, qVariantValue<QPoint>(val));
                return;
            } else if (QtPointFPropertyManager *pointFManager = qobject_cast<QtPointFPropertyManager *>(manager)) {
                pointFManager->setValue(internProp, qVariantValue<QPointF>(val));
                return;
            } else if (QtSizePropertyManager *sizeManager = qobject_cast<QtSizePropertyManager *>(manager)) {
                sizeManager->setValue(internProp, qVariantValue<QSize>(val));
                return;
            } else if (QtSizeFPropertyManager *sizeFManager = qobject_cast<QtSizeFPropertyManager *>(manager)) {
                sizeFManager->setValue(internProp, qVariantValue<QSizeF>(val));
                return;
            } else if (QtRectPropertyManager *rectManager = qobject_cast<QtRectPropertyManager *>(manager)) {
                rectManager->setValue(internProp, qVariantValue<QRect>(val));
                return;
            } else if (QtRectFPropertyManager *rectFManager = qobject_cast<QtRectFPropertyManager *>(manager)) {
                rectFManager->setValue(internProp, qVariantValue<QRectF>(val));
                return;
            } else if (QtColorPropertyManager *colorManager = qobject_cast<QtColorPropertyManager *>(manager)) {
                colorManager->setValue(internProp, qVariantValue<QColor>(val));
                return;
            } else if (QtEnumPropertyManager *enumManager = qobject_cast<QtEnumPropertyManager *>(manager)) {
                enumManager->setValue(internProp, qVariantValue<int>(val));
                return;
            } else if (QtSizePolicyPropertyManager *sizePolicyManager =
                            qobject_cast<QtSizePolicyPropertyManager *>(manager)) {
                sizePolicyManager->setValue(internProp, qVariantValue<QSizePolicy>(val));
                return;
            } else if (QtFontPropertyManager *fontManager = qobject_cast<QtFontPropertyManager *>(manager)) {
                fontManager->setValue(internProp, qVariantValue<QFont>(val));
                return;
            } else if (QtCursorPropertyManager *cursorManager = qobject_cast<QtCursorPropertyManager *>(manager)) {
                cursorManager->setValue(internProp, qVariantValue<QCursor>(val));
                return;
            } else if (QtFlagPropertyManager *flagManager = qobject_cast<QtFlagPropertyManager *>(manager)) {
                flagManager->setValue(internProp, qVariantValue<int>(val));
                return;
            }
        }
        it++;
    }
}

/*!
    Sets the value of the specified \a attribute of the given \a
    property, to \a value.

    The new \a value's type must be of the type returned by
    attributeType(), or of a type that can be converted to
    attributeType() using the QVariant::canConvert() function,
    otherwise this function does nothing.

    \sa attributeValue(), QtVariantProperty::setAttribute(), attributeChanged()
*/
void QtVariantPropertyManager::setAttribute(QtProperty *property,
        const QString &attribute, const QVariant &value)
{
    QVariant oldAttr = attributeValue(property, attribute);
    if (!oldAttr.isValid())
        return;

    int attrType = value.userType();
    if (!attrType)
        return;

    if (attrType != attributeType(propertyType(property), attribute) &&
                !value.canConvert((QVariant::Type)attrType))
        return;

    QMap<QtVariantProperty *, QtProperty *>::ConstIterator it =
            propertyToWrappedProperty()->constBegin();
    while (it != propertyToWrappedProperty()->constEnd()) {
        QtVariantProperty *varProp = it.key();
        if (property == varProp) {
            QtProperty *internProp = it.value();

            QtAbstractPropertyManager *manager = internProp->propertyManager();
            if (QtIntPropertyManager *intManager = qobject_cast<QtIntPropertyManager *>(manager)) {
                if (attribute == QLatin1String("maximum"))
                    intManager->setMaximum(internProp, qVariantValue<int>(value));
                else if (attribute == QLatin1String("minimum"))
                    intManager->setMinimum(internProp, qVariantValue<int>(value));
                return;
            } else if (QtDoublePropertyManager *doubleManager = qobject_cast<QtDoublePropertyManager *>(manager)) {
                if (attribute == QLatin1String("maximum"))
                    doubleManager->setMaximum(internProp, qVariantValue<double>(value));
                if (attribute == QLatin1String("minimum"))
                    doubleManager->setMinimum(internProp, qVariantValue<double>(value));
                if (attribute == QLatin1String("decimals"))
                    doubleManager->setDecimals(internProp, qVariantValue<int>(value));
                return;
            } else if (QtStringPropertyManager *stringManager = qobject_cast<QtStringPropertyManager *>(manager)) {
                if (attribute == QLatin1String("regExp"))
                    stringManager->setRegExp(internProp, qVariantValue<QRegExp>(value));
                return;
            } else if (QtDatePropertyManager *dateManager = qobject_cast<QtDatePropertyManager *>(manager)) {
                if (attribute == QLatin1String("maximum"))
                    dateManager->setMaximum(internProp, qVariantValue<QDate>(value));
                if (attribute == QLatin1String("minimum"))
                    dateManager->setMinimum(internProp, qVariantValue<QDate>(value));
                return;
            } else if (QtPointFPropertyManager *pointFManager = qobject_cast<QtPointFPropertyManager *>(manager)) {
                if (attribute == QLatin1String("decimals"))
                    pointFManager->setDecimals(internProp, qVariantValue<int>(value));
                return;
            } else if (QtSizePropertyManager *sizeManager = qobject_cast<QtSizePropertyManager *>(manager)) {
                if (attribute == QLatin1String("maximum"))
                    sizeManager->setMaximum(internProp, qVariantValue<QSize>(value));
                if (attribute == QLatin1String("minimum"))
                    sizeManager->setMinimum(internProp, qVariantValue<QSize>(value));
                return;
            } else if (QtSizeFPropertyManager *sizeFManager = qobject_cast<QtSizeFPropertyManager *>(manager)) {
                if (attribute == QLatin1String("maximum"))
                    sizeFManager->setMaximum(internProp, qVariantValue<QSizeF>(value));
                if (attribute == QLatin1String("minimum"))
                    sizeFManager->setMinimum(internProp, qVariantValue<QSizeF>(value));
                if (attribute == QLatin1String("decimals"))
                    sizeFManager->setDecimals(internProp, qVariantValue<int>(value));
                return;
            } else if (QtRectPropertyManager *rectManager = qobject_cast<QtRectPropertyManager *>(manager)) {
                if (attribute == QLatin1String("constraint"))
                    rectManager->setConstraint(internProp, qVariantValue<QRect>(value));
                return;
            } else if (QtRectFPropertyManager *rectFManager = qobject_cast<QtRectFPropertyManager *>(manager)) {
                if (attribute == QLatin1String("constraint"))
                    rectFManager->setConstraint(internProp, qVariantValue<QRectF>(value));
                if (attribute == QLatin1String("decimals"))
                    rectFManager->setDecimals(internProp, qVariantValue<int>(value));
                return;
            } else if (QtEnumPropertyManager *enumManager = qobject_cast<QtEnumPropertyManager *>(manager)) {
                if (attribute == QLatin1String("enumNames"))
                    enumManager->setEnumNames(internProp, qVariantValue<QStringList>(value));
                if (attribute == QLatin1String("enumIcons"))
                    enumManager->setEnumIcons(internProp, qVariantValue<QtIconMap>(value));
                return;
            } else if (QtFlagPropertyManager *flagManager = qobject_cast<QtFlagPropertyManager *>(manager)) {
                if (attribute == QLatin1String("flagNames"))
                    flagManager->setFlagNames(internProp, qVariantValue<QStringList>(value));
                return;
            }
        }
        it++;
    }
}

/*!
    \reimp
*/
bool QtVariantPropertyManager::hasValue(const QtProperty *property) const
{
    if (propertyType(property) == groupTypeId())
        return false;
    return true;
}

/*!
    \reimp
*/
QString QtVariantPropertyManager::valueText(const QtProperty *property) const
{
    QMap<QtVariantProperty *, QtProperty *>::ConstIterator it =
            propertyToWrappedProperty()->constBegin();
    while (it != propertyToWrappedProperty()->constEnd()) {
        QtVariantProperty *varProp = it.key();
        if (property == varProp) {
            QtProperty *internProp = it.value();
            return internProp->valueText();
        }
        it++;
    }
    return QString();
}

/*!
    \reimp
*/
QIcon QtVariantPropertyManager::valueIcon(const QtProperty *property) const
{
    QMap<QtVariantProperty *, QtProperty *>::ConstIterator it =
            propertyToWrappedProperty()->constBegin();
    while (it != propertyToWrappedProperty()->constEnd()) {
        QtVariantProperty *varProp = it.key();
        if (property == varProp) {
            QtProperty *internProp = it.value();
            return internProp->valueIcon();
        }
        it++;
    }
    return QIcon();
}

/*!
    \reimp
*/
void QtVariantPropertyManager::initializeProperty(QtProperty *property)
{
    QtVariantProperty *varProp = variantProperty(property);
    if (!varProp)
        return;

    QMap<int, QtAbstractPropertyManager *>::ConstIterator it =
            d_ptr->m_typeToPropertyManager.find(d_ptr->m_propertyType);
    if (it != d_ptr->m_typeToPropertyManager.constEnd()) {
        QtProperty *internProp = 0;
        if (!d_ptr->m_creatingSubProperties) {
            QtAbstractPropertyManager *manager = it.value();
            internProp = manager->addProperty();
            d_ptr->m_internalToProperty[internProp] = varProp;
        }
        (*propertyToWrappedProperty())[varProp] = internProp;
        if (internProp) {
            QList<QtProperty *> children = internProp->subProperties();
            QListIterator<QtProperty *> itChild(children);
            QtVariantProperty *lastProperty = 0;
            while (itChild.hasNext()) {
                QtVariantProperty *prop = d_ptr->createSubProperty(varProp, lastProperty, itChild.next());
                lastProperty = prop ? prop : lastProperty;
            }
        }
    }
}

/*!
    \reimp
*/
void QtVariantPropertyManager::uninitializeProperty(QtProperty *property)
{
    QtVariantProperty *varProp = variantProperty(property);
    if (!varProp)
        return;

    QMap<QtVariantProperty *, QtProperty *>::ConstIterator it =
            propertyToWrappedProperty()->find(varProp);
    if (it != propertyToWrappedProperty()->constEnd()) {
        QtProperty *internProp = it.value();
        if (internProp) {
            d_ptr->m_internalToProperty.remove(internProp);
            if (!d_ptr->m_destroyingSubProperties) {
                delete internProp;
            }
        }
        propertyToWrappedProperty()->remove(varProp);
    }
    d_ptr->m_propertyToType.remove(varProp);
}

/*!
    \reimp
*/
QtProperty *QtVariantPropertyManager::createProperty()
{
    if (!d_ptr->m_creatingProperty)
        return 0;

    QtVariantProperty *property = new QtVariantProperty(this);
    d_ptr->m_propertyToType[property] = d_ptr->m_propertyType;

    return property;
}

/////////////////////////////

class QtVariantEditorFactoryPrivate
{
    QtVariantEditorFactory *q_ptr;
    Q_DECLARE_PUBLIC(QtVariantEditorFactory)
public:

    QtSpinBoxFactory        *m_spinBoxFactory;
    QtDoubleSpinBoxFactory  *m_doubleSpinBoxFactory;
    QtCheckBoxFactory       *m_checkBoxFactory;
    QtLineEditFactory       *m_lineEditFactory;
    QtDateEditFactory       *m_dateEditFactory;
    QtTimeEditFactory       *m_timeEditFactory;
    QtDateTimeEditFactory   *m_dateTimeEditFactory;
    QtKeySequenceEditorFactory *m_keySequenceEditorFactory;
    QtEnumEditorFactory     *m_comboBoxFactory;
    QtCursorEditorFactory   *m_cursorEditorFactory;

    QMap<QtAbstractEditorFactoryBase *, int> m_factoryToType;
    QMap<int, QtAbstractEditorFactoryBase *> m_typeToFactory;
};

/*!
    \class QtVariantEditorFactory

    \brief The QtVariantEditorFactory class provides widgets for properties
    created by QtVariantPropertyManager objects.

    The variant factory provides the following widgets for the
    specified property types:

    \table
    \header
        \o Property Type
        \o Widget
    \row
        \o \c int
        \o QSpinBox
    \row
        \o \c double
        \o QDoubleSpinBox
    \row
        \o \c bool
        \o QCheckBox
    \row
        \o QString
        \o QLineEdit
    \row
        \o QDate
        \o QDateEdit
    \row
        \o QTime
        \o QTimeEdit
    \row
        \o QDateTime
        \o QDateTimeEdit
    \row
        \o QKeySequence
        \o customized editor
    \row
        \o \c enum
        \o QComboBox
    \row
        \o QCursor
        \o QComboBox
    \endtable

    Note that QtVariantPropertyManager supports several additional property
    types for which the QtVariantEditorFactory class does not provide
    editing widgets, e.g. QPoint and QSize. To provide widgets for other
    types using the variant approach, derive from the QtVariantEditorFactory
    class.

    \sa QtAbstractEditorFactory, QtVariantPropertyManager
*/

/*!
    Creates a factory with the given \a parent.
*/
QtVariantEditorFactory::QtVariantEditorFactory(QObject *parent)
    : QtAbstractEditorFactory<QtVariantPropertyManager>(parent)
{
    d_ptr = new QtVariantEditorFactoryPrivate();
    d_ptr->q_ptr = this;

    d_ptr->m_spinBoxFactory = new QtSpinBoxFactory(this);
    d_ptr->m_factoryToType[d_ptr->m_spinBoxFactory] = QVariant::Int;
    d_ptr->m_typeToFactory[QVariant::Int] = d_ptr->m_spinBoxFactory;

    d_ptr->m_doubleSpinBoxFactory = new QtDoubleSpinBoxFactory(this);
    d_ptr->m_factoryToType[d_ptr->m_doubleSpinBoxFactory] = QVariant::Double;
    d_ptr->m_typeToFactory[QVariant::Double] = d_ptr->m_doubleSpinBoxFactory;

    d_ptr->m_checkBoxFactory = new QtCheckBoxFactory(this);
    d_ptr->m_factoryToType[d_ptr->m_checkBoxFactory] = QVariant::Bool;
    d_ptr->m_typeToFactory[QVariant::Bool] = d_ptr->m_checkBoxFactory;

    d_ptr->m_lineEditFactory = new QtLineEditFactory(this);
    d_ptr->m_factoryToType[d_ptr->m_lineEditFactory] = QVariant::String;
    d_ptr->m_typeToFactory[QVariant::String] = d_ptr->m_lineEditFactory;

    d_ptr->m_dateEditFactory = new QtDateEditFactory(this);
    d_ptr->m_factoryToType[d_ptr->m_dateEditFactory] = QVariant::Date;
    d_ptr->m_typeToFactory[QVariant::Date] = d_ptr->m_dateEditFactory;

    d_ptr->m_timeEditFactory = new QtTimeEditFactory(this);
    d_ptr->m_factoryToType[d_ptr->m_timeEditFactory] = QVariant::Time;
    d_ptr->m_typeToFactory[QVariant::Time] = d_ptr->m_timeEditFactory;

    d_ptr->m_dateTimeEditFactory = new QtDateTimeEditFactory(this);
    d_ptr->m_factoryToType[d_ptr->m_dateTimeEditFactory] = QVariant::DateTime;
    d_ptr->m_typeToFactory[QVariant::DateTime] = d_ptr->m_dateTimeEditFactory;

    d_ptr->m_keySequenceEditorFactory = new QtKeySequenceEditorFactory(this);
    d_ptr->m_factoryToType[d_ptr->m_keySequenceEditorFactory] = QVariant::KeySequence;
    d_ptr->m_typeToFactory[QVariant::KeySequence] = d_ptr->m_keySequenceEditorFactory;

    d_ptr->m_cursorEditorFactory = new QtCursorEditorFactory(this);
    d_ptr->m_factoryToType[d_ptr->m_cursorEditorFactory] = QVariant::Cursor;
    d_ptr->m_typeToFactory[QVariant::Cursor] = d_ptr->m_cursorEditorFactory;

    d_ptr->m_comboBoxFactory = new QtEnumEditorFactory(this);
    int enumId = QtVariantPropertyManager::enumTypeId();
    d_ptr->m_factoryToType[d_ptr->m_comboBoxFactory] = enumId;
    d_ptr->m_typeToFactory[enumId] = d_ptr->m_comboBoxFactory;
}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtVariantEditorFactory::~QtVariantEditorFactory()
{
    delete d_ptr;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtVariantEditorFactory::connectPropertyManager(QtVariantPropertyManager *manager)
{
    QList<QtIntPropertyManager *> intPropertyManagers = qFindChildren<QtIntPropertyManager *>(manager);
    QListIterator<QtIntPropertyManager *> itInt(intPropertyManagers);
    while (itInt.hasNext())
        d_ptr->m_spinBoxFactory->addPropertyManager(itInt.next());

    QList<QtDoublePropertyManager *> doublePropertyManagers = qFindChildren<QtDoublePropertyManager *>(manager);
    QListIterator<QtDoublePropertyManager *> itDouble(doublePropertyManagers);
    while (itDouble.hasNext())
        d_ptr->m_doubleSpinBoxFactory->addPropertyManager(itDouble.next());

    QList<QtBoolPropertyManager *> boolPropertyManagers = qFindChildren<QtBoolPropertyManager *>(manager);
    QListIterator<QtBoolPropertyManager *> itBool(boolPropertyManagers);
    while (itBool.hasNext())
        d_ptr->m_checkBoxFactory->addPropertyManager(itBool.next());

    QList<QtStringPropertyManager *> stringPropertyManagers = qFindChildren<QtStringPropertyManager *>(manager);
    QListIterator<QtStringPropertyManager *> itString(stringPropertyManagers);
    while (itString.hasNext())
        d_ptr->m_lineEditFactory->addPropertyManager(itString.next());

    QList<QtDatePropertyManager *> datePropertyManagers = qFindChildren<QtDatePropertyManager *>(manager);
    QListIterator<QtDatePropertyManager *> itDate(datePropertyManagers);
    while (itDate.hasNext())
        d_ptr->m_dateEditFactory->addPropertyManager(itDate.next());

    QList<QtTimePropertyManager *> timePropertyManagers = qFindChildren<QtTimePropertyManager *>(manager);
    QListIterator<QtTimePropertyManager *> itTime(timePropertyManagers);
    while (itTime.hasNext())
        d_ptr->m_timeEditFactory->addPropertyManager(itTime.next());

    QList<QtDateTimePropertyManager *> dateTimePropertyManagers = qFindChildren<QtDateTimePropertyManager *>(manager);
    QListIterator<QtDateTimePropertyManager *> itDateTime(dateTimePropertyManagers);
    while (itDateTime.hasNext())
        d_ptr->m_dateTimeEditFactory->addPropertyManager(itDateTime.next());

    QList<QtKeySequencePropertyManager *> keySequencePropertyManagers = qFindChildren<QtKeySequencePropertyManager *>(manager);
    QListIterator<QtKeySequencePropertyManager *> itKeySequence(keySequencePropertyManagers);
    while (itKeySequence.hasNext())
        d_ptr->m_keySequenceEditorFactory->addPropertyManager(itKeySequence.next());

    QList<QtLocalePropertyManager *> localePropertyManagers = qFindChildren<QtLocalePropertyManager *>(manager);
    QListIterator<QtLocalePropertyManager *> itLocale(localePropertyManagers);
    while (itLocale.hasNext())
        d_ptr->m_comboBoxFactory->addPropertyManager(itLocale.next()->subEnumPropertyManager());

    QList<QtPointPropertyManager *> pointPropertyManagers = qFindChildren<QtPointPropertyManager *>(manager);
    QListIterator<QtPointPropertyManager *> itPoint(pointPropertyManagers);
    while (itPoint.hasNext())
        d_ptr->m_spinBoxFactory->addPropertyManager(itPoint.next()->subIntPropertyManager());

    QList<QtPointFPropertyManager *> pointFPropertyManagers = qFindChildren<QtPointFPropertyManager *>(manager);
    QListIterator<QtPointFPropertyManager *> itPointF(pointFPropertyManagers);
    while (itPointF.hasNext())
        d_ptr->m_doubleSpinBoxFactory->addPropertyManager(itPointF.next()->subDoublePropertyManager());

    QList<QtSizePropertyManager *> sizePropertyManagers = qFindChildren<QtSizePropertyManager *>(manager);
    QListIterator<QtSizePropertyManager *> itSize(sizePropertyManagers);
    while (itSize.hasNext())
        d_ptr->m_spinBoxFactory->addPropertyManager(itSize.next()->subIntPropertyManager());

    QList<QtSizeFPropertyManager *> sizeFPropertyManagers = qFindChildren<QtSizeFPropertyManager *>(manager);
    QListIterator<QtSizeFPropertyManager *> itSizeF(sizeFPropertyManagers);
    while (itSizeF.hasNext())
        d_ptr->m_doubleSpinBoxFactory->addPropertyManager(itSizeF.next()->subDoublePropertyManager());

    QList<QtRectPropertyManager *> rectPropertyManagers = qFindChildren<QtRectPropertyManager *>(manager);
    QListIterator<QtRectPropertyManager *> itRect(rectPropertyManagers);
    while (itRect.hasNext())
        d_ptr->m_spinBoxFactory->addPropertyManager(itRect.next()->subIntPropertyManager());

    QList<QtRectFPropertyManager *> rectFPropertyManagers = qFindChildren<QtRectFPropertyManager *>(manager);
    QListIterator<QtRectFPropertyManager *> itRectF(rectFPropertyManagers);
    while (itRectF.hasNext())
        d_ptr->m_doubleSpinBoxFactory->addPropertyManager(itRectF.next()->subDoublePropertyManager());

    QList<QtColorPropertyManager *> colorPropertyManagers = qFindChildren<QtColorPropertyManager *>(manager);
    QListIterator<QtColorPropertyManager *> itColor(colorPropertyManagers);
    while (itColor.hasNext())
        d_ptr->m_spinBoxFactory->addPropertyManager(itColor.next()->subIntPropertyManager());

    QList<QtEnumPropertyManager *> enumPropertyManagers = qFindChildren<QtEnumPropertyManager *>(manager);
    QListIterator<QtEnumPropertyManager *> itEnum(enumPropertyManagers);
    while (itEnum.hasNext())
        d_ptr->m_comboBoxFactory->addPropertyManager(itEnum.next());

    QList<QtSizePolicyPropertyManager *> sizePolicyPropertyManagers = qFindChildren<QtSizePolicyPropertyManager *>(manager);
    QListIterator<QtSizePolicyPropertyManager *> itSizePolicy(sizePolicyPropertyManagers);
    while (itSizePolicy.hasNext()) {
        QtSizePolicyPropertyManager *manager = itSizePolicy.next();
        d_ptr->m_spinBoxFactory->addPropertyManager(manager->subIntPropertyManager());
        d_ptr->m_comboBoxFactory->addPropertyManager(manager->subEnumPropertyManager());
    }

    QList<QtFontPropertyManager *> fontPropertyManagers = qFindChildren<QtFontPropertyManager *>(manager);
    QListIterator<QtFontPropertyManager *> itFont(fontPropertyManagers);
    while (itFont.hasNext()) {
        QtFontPropertyManager *manager = itFont.next();
        d_ptr->m_spinBoxFactory->addPropertyManager(manager->subIntPropertyManager());
        d_ptr->m_comboBoxFactory->addPropertyManager(manager->subEnumPropertyManager());
        d_ptr->m_checkBoxFactory->addPropertyManager(manager->subBoolPropertyManager());
    }

    QList<QtCursorPropertyManager *> cursorPropertyManagers = qFindChildren<QtCursorPropertyManager *>(manager);
    QListIterator<QtCursorPropertyManager *> itCursor(cursorPropertyManagers);
    while (itCursor.hasNext())
        d_ptr->m_cursorEditorFactory->addPropertyManager(itCursor.next());

    QList<QtFlagPropertyManager *> flagPropertyManagers = qFindChildren<QtFlagPropertyManager *>(manager);
    QListIterator<QtFlagPropertyManager *> itFlag(flagPropertyManagers);
    while (itFlag.hasNext())
        d_ptr->m_checkBoxFactory->addPropertyManager(itFlag.next()->subBoolPropertyManager());
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtVariantEditorFactory::createEditor(QtVariantPropertyManager *manager, QtProperty *property,
        QWidget *parent)
{
    int propType = manager->propertyType(property);
    if (!d_ptr->m_typeToFactory.contains(propType))
        return 0;
    return d_ptr->m_typeToFactory[propType]->createEditor(wrappedProperty(property), parent);
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtVariantEditorFactory::disconnectPropertyManager(QtVariantPropertyManager *manager)
{
    QList<QtIntPropertyManager *> intPropertyManagers = qFindChildren<QtIntPropertyManager *>(manager);
    QListIterator<QtIntPropertyManager *> itInt(intPropertyManagers);
    while (itInt.hasNext())
        d_ptr->m_spinBoxFactory->removePropertyManager(itInt.next());

    QList<QtDoublePropertyManager *> doublePropertyManagers = qFindChildren<QtDoublePropertyManager *>(manager);
    QListIterator<QtDoublePropertyManager *> itDouble(doublePropertyManagers);
    while (itDouble.hasNext())
        d_ptr->m_doubleSpinBoxFactory->removePropertyManager(itDouble.next());

    QList<QtBoolPropertyManager *> boolPropertyManagers = qFindChildren<QtBoolPropertyManager *>(manager);
    QListIterator<QtBoolPropertyManager *> itBool(boolPropertyManagers);
    while (itBool.hasNext())
        d_ptr->m_checkBoxFactory->removePropertyManager(itBool.next());

    QList<QtStringPropertyManager *> stringPropertyManagers = qFindChildren<QtStringPropertyManager *>(manager);
    QListIterator<QtStringPropertyManager *> itString(stringPropertyManagers);
    while (itString.hasNext())
        d_ptr->m_lineEditFactory->removePropertyManager(itString.next());

    QList<QtDatePropertyManager *> datePropertyManagers = qFindChildren<QtDatePropertyManager *>(manager);
    QListIterator<QtDatePropertyManager *> itDate(datePropertyManagers);
    while (itDate.hasNext())
        d_ptr->m_dateEditFactory->removePropertyManager(itDate.next());

    QList<QtTimePropertyManager *> timePropertyManagers = qFindChildren<QtTimePropertyManager *>(manager);
    QListIterator<QtTimePropertyManager *> itTime(timePropertyManagers);
    while (itTime.hasNext())
        d_ptr->m_timeEditFactory->removePropertyManager(itTime.next());

    QList<QtDateTimePropertyManager *> dateTimePropertyManagers = qFindChildren<QtDateTimePropertyManager *>(manager);
    QListIterator<QtDateTimePropertyManager *> itDateTime(dateTimePropertyManagers);
    while (itDateTime.hasNext())
        d_ptr->m_dateTimeEditFactory->removePropertyManager(itDateTime.next());

    QList<QtKeySequencePropertyManager *> keySequencePropertyManagers = qFindChildren<QtKeySequencePropertyManager *>(manager);
    QListIterator<QtKeySequencePropertyManager *> itKeySequence(keySequencePropertyManagers);
    while (itKeySequence.hasNext())
        d_ptr->m_keySequenceEditorFactory->removePropertyManager(itKeySequence.next());

    QList<QtLocalePropertyManager *> localePropertyManagers = qFindChildren<QtLocalePropertyManager *>(manager);
    QListIterator<QtLocalePropertyManager *> itLocale(localePropertyManagers);
    while (itLocale.hasNext())
        d_ptr->m_comboBoxFactory->removePropertyManager(itLocale.next()->subEnumPropertyManager());

    QList<QtPointPropertyManager *> pointPropertyManagers = qFindChildren<QtPointPropertyManager *>(manager);
    QListIterator<QtPointPropertyManager *> itPoint(pointPropertyManagers);
    while (itPoint.hasNext())
        d_ptr->m_spinBoxFactory->removePropertyManager(itPoint.next()->subIntPropertyManager());

    QList<QtPointFPropertyManager *> pointFPropertyManagers = qFindChildren<QtPointFPropertyManager *>(manager);
    QListIterator<QtPointFPropertyManager *> itPointF(pointFPropertyManagers);
    while (itPointF.hasNext())
        d_ptr->m_doubleSpinBoxFactory->removePropertyManager(itPointF.next()->subDoublePropertyManager());

    QList<QtSizePropertyManager *> sizePropertyManagers = qFindChildren<QtSizePropertyManager *>(manager);
    QListIterator<QtSizePropertyManager *> itSize(sizePropertyManagers);
    while (itSize.hasNext())
        d_ptr->m_spinBoxFactory->removePropertyManager(itSize.next()->subIntPropertyManager());

    QList<QtSizeFPropertyManager *> sizeFPropertyManagers = qFindChildren<QtSizeFPropertyManager *>(manager);
    QListIterator<QtSizeFPropertyManager *> itSizeF(sizeFPropertyManagers);
    while (itSizeF.hasNext())
        d_ptr->m_doubleSpinBoxFactory->removePropertyManager(itSizeF.next()->subDoublePropertyManager());

    QList<QtRectPropertyManager *> rectPropertyManagers = qFindChildren<QtRectPropertyManager *>(manager);
    QListIterator<QtRectPropertyManager *> itRect(rectPropertyManagers);
    while (itRect.hasNext())
        d_ptr->m_spinBoxFactory->removePropertyManager(itRect.next()->subIntPropertyManager());

    QList<QtRectFPropertyManager *> rectFPropertyManagers = qFindChildren<QtRectFPropertyManager *>(manager);
    QListIterator<QtRectFPropertyManager *> itRectF(rectFPropertyManagers);
    while (itRectF.hasNext())
        d_ptr->m_doubleSpinBoxFactory->removePropertyManager(itRectF.next()->subDoublePropertyManager());

    QList<QtColorPropertyManager *> colorPropertyManagers = qFindChildren<QtColorPropertyManager *>(manager);
    QListIterator<QtColorPropertyManager *> itColor(colorPropertyManagers);
    while (itColor.hasNext())
        d_ptr->m_spinBoxFactory->removePropertyManager(itColor.next()->subIntPropertyManager());

    QList<QtEnumPropertyManager *> enumPropertyManagers = qFindChildren<QtEnumPropertyManager *>(manager);
    QListIterator<QtEnumPropertyManager *> itEnum(enumPropertyManagers);
    while (itEnum.hasNext())
        d_ptr->m_comboBoxFactory->removePropertyManager(itEnum.next());

    QList<QtSizePolicyPropertyManager *> sizePolicyPropertyManagers = qFindChildren<QtSizePolicyPropertyManager *>(manager);
    QListIterator<QtSizePolicyPropertyManager *> itSizePolicy(sizePolicyPropertyManagers);
    while (itSizePolicy.hasNext()) {
        QtSizePolicyPropertyManager *manager = itSizePolicy.next();
        d_ptr->m_spinBoxFactory->removePropertyManager(manager->subIntPropertyManager());
        d_ptr->m_comboBoxFactory->removePropertyManager(manager->subEnumPropertyManager());
    }

    QList<QtFontPropertyManager *> fontPropertyManagers = qFindChildren<QtFontPropertyManager *>(manager);
    QListIterator<QtFontPropertyManager *> itFont(fontPropertyManagers);
    while (itFont.hasNext()) {
        QtFontPropertyManager *manager = itFont.next();
        d_ptr->m_spinBoxFactory->removePropertyManager(manager->subIntPropertyManager());
        d_ptr->m_comboBoxFactory->removePropertyManager(manager->subEnumPropertyManager());
        d_ptr->m_checkBoxFactory->removePropertyManager(manager->subBoolPropertyManager());
    }

    QList<QtCursorPropertyManager *> cursorPropertyManagers = qFindChildren<QtCursorPropertyManager *>(manager);
    QListIterator<QtCursorPropertyManager *> itCursor(cursorPropertyManagers);
    while (itCursor.hasNext())
        d_ptr->m_cursorEditorFactory->removePropertyManager(itCursor.next());

    QList<QtFlagPropertyManager *> flagPropertyManagers = qFindChildren<QtFlagPropertyManager *>(manager);
    QListIterator<QtFlagPropertyManager *> itFlag(flagPropertyManagers);
    while (itFlag.hasNext())
        d_ptr->m_checkBoxFactory->removePropertyManager(itFlag.next()->subBoolPropertyManager());
}

#include "moc_qtvariantproperty.cpp"
