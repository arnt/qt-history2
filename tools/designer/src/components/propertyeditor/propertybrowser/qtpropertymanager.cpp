#include "qtpropertymanager.h"
#include "qtcursordatabase_p.h"
#include <QDateTime>
#include <QLocale>
#include <QMap>
#include <QIcon>
#include <QMetaEnum>
#include <QFontDatabase>
#include <QPainter>
#include <QLabel>

class QtMetaEnumWrapper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QSizePolicy::Policy policy READ policy)
public:
    QSizePolicy::Policy policy() const { return QSizePolicy::Ignored; }
private:
    QtMetaEnumWrapper(QObject *parent) : QObject(parent) {}
};

class QtMetaEnumProvider
{
public:
    QtMetaEnumProvider();

    QStringList policyEnumNames() const { return m_policyEnumNames; }
    QStringList languageEnumNames() const { return m_languageEnumNames; }
    QStringList countryEnumNames(QLocale::Language language) const { return m_countryEnumNames.value(language); }

    QSizePolicy::Policy indexToSizePolicy(int index) const;
    int sizePolicyToIndex(QSizePolicy::Policy policy) const;

    void indexToLocale(int languageIndex, int countryIndex, QLocale::Language *language, QLocale::Country *country) const;
    void localeToIndex(QLocale::Language language, QLocale::Country country, int *languageIndex, int *countryIndex) const;

private:
    void initLocale();

    QStringList m_policyEnumNames;
    QStringList m_languageEnumNames;
    QMap<QLocale::Language, QStringList> m_countryEnumNames;
    QMap<int, QLocale::Language> m_indexToLanguage;
    QMap<QLocale::Language, int> m_languageToIndex;
    QMap<int, QMap<int, QLocale::Country> > m_indexToCountry;
    QMap<QLocale::Language, QMap<QLocale::Country, int> > m_countryToIndex;
    QMetaEnum m_policyEnum;
};

#if QT_VERSION < 0x040300

static QList<QLocale::Country> countriesForLanguage(QLocale::Language language)
{
    QList<QLocale::Country> countries;
    QLocale::Country country = QLocale::AnyCountry;
    while (country <= QLocale::LastCountry) {
        QLocale locale(language, country);
        if (locale.language() == language && !countries.contains(locale.country()))
            countries << locale.country();
        country = (QLocale::Country)((uint)country + 1); // ++country
    }
    return countries;
}

#endif

static QList<QLocale::Country> sortCountries(const QList<QLocale::Country> &countries)
{
    QMultiMap<QString, QLocale::Country> nameToCountry;
    QListIterator<QLocale::Country> itCountry(countries);
    while (itCountry.hasNext()) {
        QLocale::Country country = itCountry.next();
        nameToCountry.insert(QLocale::countryToString(country), country);
    }
    return nameToCountry.values();
}

void QtMetaEnumProvider::initLocale()
{
    QMultiMap<QString, QLocale::Language> nameToLanguage;
    QLocale::Language language = QLocale::C;
    while (language <= QLocale::LastLanguage) {
        QLocale locale(language);
        if (locale.language() == language)
            nameToLanguage.insert(QLocale::languageToString(language), language);
        language = (QLocale::Language)((uint)language + 1); // ++language
    }

    const QLocale system = QLocale::system();
    if (!nameToLanguage.contains(QLocale::languageToString(system.language())))
        nameToLanguage.insert(QLocale::languageToString(system.language()), system.language());

    QList<QLocale::Language> languages = nameToLanguage.values();
    QListIterator<QLocale::Language> itLang(languages);
    while (itLang.hasNext()) {
        QLocale::Language language = itLang.next();
        QList<QLocale::Country> countries;
#if QT_VERSION < 0x040300
        countries = countriesForLanguage(language);
#else
        countries = QLocale::countriesForLanguage(language);
#endif
        if (countries.isEmpty() && language == system.language())
            countries << system.country();

        if (!countries.isEmpty() && !m_languageToIndex.contains(language)) {
            countries = sortCountries(countries);
            int langIdx = m_languageEnumNames.count();
            m_indexToLanguage[langIdx] = language;
            m_languageToIndex[language] = langIdx;
            QStringList countryNames;
            QListIterator<QLocale::Country> it(countries);
            int countryIdx = 0;
            while (it.hasNext()) {
                QLocale::Country country = it.next();
                countryNames << QLocale::countryToString(country);
                m_indexToCountry[langIdx][countryIdx] = country;
                m_countryToIndex[language][country] = countryIdx;
                ++countryIdx;
            }
            m_languageEnumNames << QLocale::languageToString(language);
            m_countryEnumNames[language] = countryNames;
        }
    }
}

QtMetaEnumProvider::QtMetaEnumProvider()
{
    QMetaProperty p;

    p = QtMetaEnumWrapper::staticMetaObject.property(
                QtMetaEnumWrapper::staticMetaObject.propertyOffset() + 0);
    m_policyEnum = p.enumerator();
    for (int i = 0; i < m_policyEnum.keyCount(); i++)
        m_policyEnumNames << QLatin1String(m_policyEnum.key(i));

    initLocale();
}

QSizePolicy::Policy QtMetaEnumProvider::indexToSizePolicy(int index) const
{
    return (QSizePolicy::Policy)(m_policyEnum.value(index));
}

int QtMetaEnumProvider::sizePolicyToIndex(QSizePolicy::Policy policy) const
{
    for (int i = 0; i < m_policyEnum.keyCount(); i++)
        if (indexToSizePolicy(i) == policy)
            return i;
    return -1;
}

void QtMetaEnumProvider::indexToLocale(int languageIndex, int countryIndex, QLocale::Language *language, QLocale::Country *country) const
{
    QLocale::Language l = QLocale::C;
    QLocale::Country c = QLocale::AnyCountry;
    if (m_indexToLanguage.contains(languageIndex)) {
        l = m_indexToLanguage[languageIndex];
        if (m_indexToCountry.contains(languageIndex) && m_indexToCountry[languageIndex].contains(countryIndex))
            c = m_indexToCountry[languageIndex][countryIndex];
    }
    if (language)
        *language = l;
    if (country)
        *country = c;
}

void QtMetaEnumProvider::localeToIndex(QLocale::Language language, QLocale::Country country, int *languageIndex, int *countryIndex) const
{
    int l = -1;
    int c = -1;
    if (m_languageToIndex.contains(language)) {
        l = m_languageToIndex[language];
        if (m_countryToIndex.contains(language) && m_countryToIndex[language].contains(country))
            c = m_countryToIndex[language][country];
    }

    if (languageIndex)
        *languageIndex = l;
    if (countryIndex)
        *countryIndex = c;
}

Q_GLOBAL_STATIC(QtMetaEnumProvider, metaEnumProvider)

// QtGroupPropertyManager

/*!
    \class QtGroupPropertyManager

    \brief The QtGroupPropertyManager provides and manages group properties.

    This class is intended to provide a grouping element without any value.

    \sa QtAbstractPropertyManager
*/

/*!
    Creates a manager with the given \a parent.
*/
QtGroupPropertyManager::QtGroupPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{

}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtGroupPropertyManager::~QtGroupPropertyManager()
{

}

/*!
    \reimp
*/
bool QtGroupPropertyManager::hasValue(const QtProperty *property) const
{
    Q_UNUSED(property)
    return false;
}

/*!
    \reimp
*/
void QtGroupPropertyManager::initializeProperty(QtProperty *property)
{
    Q_UNUSED(property)
}

/*!
    \reimp
*/
void QtGroupPropertyManager::uninitializeProperty(QtProperty *property)
{
    Q_UNUSED(property)
}

// QtIntPropertyManager

class QtIntPropertyManagerPrivate
{
    QtIntPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtIntPropertyManager)
public:

    struct Data
    {
        Data() : val(0), min(-INT_MAX), max(INT_MAX) {}
        int val;
        int min;
        int max;
    };

    QMap<const QtProperty *, Data> m_values;
};

/*!
    \class QtIntPropertyManager

    \brief The QtIntPropertyManager provides and manages int properties.

    An int property has a current value, and a range specifying the
    valid values. The range is defined by a minimum and a maximum
    value.

    The property's value and range can be retrieved using the value(),
    minimum() and maximum() functions, and can be set using the
    setValue(), setMinimum() and setMaximum() slots. Alternatively,
    the range can be defined in one go using the setRange() slot.

    In addition, QtIntPropertyManager provides the valueChanged() signal which
    is emitted whenever a property created by this manager changes,
    and the rangeChanged() signal which is emitted whenever such a
    property changes its range of valid values.

    \sa QtAbstractPropertyManager, QtSpinBoxFactory, QtSliderFactory, QtScrollBarFactory
*/

/*!
    \fn void QtIntPropertyManager::valueChanged(QtProperty *property, int value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the new
    \a value as parameters.

    \sa setValue()
*/

/*!
    \fn void QtIntPropertyManager::rangeChanged(QtProperty *property, int minimum, int maximum)

    This signal is emitted whenever a property created by this manager
    changes its range of valid values, passing a pointer to the
    \a property and the new \a minimum and \a maximum values.

    \sa setRange()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtIntPropertyManager::QtIntPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtIntPropertyManagerPrivate;
    d_ptr->q_ptr = this;
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtIntPropertyManager::~QtIntPropertyManager()
{
    delete d_ptr;
}

/*!
    Returns the given \a property's value.

    If the given property is not managed by this manager, this
    function returns 0.

    \sa setValue()
*/
int QtIntPropertyManager::value(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return 0;
    return d_ptr->m_values[property].val;
}

/*!
    Returns the given \a property's minimum value.

    \sa setMinimum(), maximum(), setRange()
*/
int QtIntPropertyManager::minimum(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return 0;
    return d_ptr->m_values[property].min;
}

/*!
    Returns the given \a property's maximum value.

    \sa setMaximum(), minimum(), setRange()
*/
int QtIntPropertyManager::maximum(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return 0;
    return d_ptr->m_values[property].max;
}

/*!
    \reimp
*/
QString QtIntPropertyManager::valueText(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QString();
    int v = value(property);
    return QString::number(v);
}

/*!
    \fn void QtIntPropertyManager::setValue(QtProperty *property, int value)

    Sets the value of the given \a property to \a value.

    If the specified \a value is not valid according to the given \a
    property's range, the \a value is adjusted to the nearest valid
    value within the range.

    \sa value(), setRange(), valueChanged()
*/
void QtIntPropertyManager::setValue(QtProperty *property, int val)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtIntPropertyManagerPrivate::Data data = d_ptr->m_values[property];

    if (data.val == val)
        return;

    int oldVal = data.val;

    if (val < data.min)
        data.val = data.min;
    else if (val > data.max)
        data.val = data.max;
    else
        data.val = val;

    if (data.val == oldVal)
        return;

    d_ptr->m_values[property] = data;

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    Sets the minimum value for the given \a property to \a min.

    When setting the minimum value, the maximum and current values are
    adjusted if necessary (ensuring that the range remains valid and
    that the current value is within the range).

    \sa minimum(), setRange(), rangeChanged()
*/
void QtIntPropertyManager::setMinimum(QtProperty *property, int min)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtIntPropertyManagerPrivate::Data data = d_ptr->m_values[property];

    if (data.min == min)
        return;

    int oldVal = data.val;

    data.min = min;
    if (data.max < data.min)
        data.max = data.min;

    if (data.val < data.min)
        data.val = data.min;

    d_ptr->m_values[property] = data;

    emit rangeChanged(property, data.min, data.max);

    if (data.val == oldVal)
        return;

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    Sets the maximum value for the given \a property to \a max.

    When setting maximum value, the minimum and current values are
    adjusted if necessary (ensuring that the range remains valid and
    that the current value is within the range).

    \sa maximum(), setRange(), rangeChanged()
*/
void QtIntPropertyManager::setMaximum(QtProperty *property, int max)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtIntPropertyManagerPrivate::Data data = d_ptr->m_values[property];

    if (data.max == max)
        return;

    int oldVal = data.val;

    data.max = max;
    if (data.min > data.max)
        data.min = data.max;

    if (data.val > data.max)
        data.val = data.max;

    d_ptr->m_values[property] = data;

    emit rangeChanged(property, data.min, data.max);

    if (data.val == oldVal)
        return;

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    \fn void QtIntPropertyManager::setRange(QtProperty *property, int minimum, int maximum)

    Sets the range of valid values.

    This is a convenience function defining the range of valid values
    in one go; setting the \a minimum and \a maximum values for the
    given \a property with a single function call.

    When setting a new range, the current value is adjusted if
    necessary (ensuring that the value remains within range).

    \sa setMinimum(), setMaximum(), rangeChanged()
*/
void QtIntPropertyManager::setRange(QtProperty *property, int min, int max)
{
    if (min > max) {
        int temp = min;
        min = max;
        max = temp;
    }

    if (!d_ptr->m_values.contains(property))
        return;

    QtIntPropertyManagerPrivate::Data data = d_ptr->m_values[property];

    if (data.min == min && data.max == max)
        return;

    int oldVal = data.val;

    data.min = min;
    data.max = max;

    if (data.val < data.min)
        data.val = data.min;
    else if (data.val > data.max)
        data.val = data.max;

    d_ptr->m_values[property] = data;

    emit rangeChanged(property, data.min, data.max);

    if (data.val == oldVal)
        return;

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    \reimp
*/
void QtIntPropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QtIntPropertyManagerPrivate::Data();
}

/*!
    \reimp
*/
void QtIntPropertyManager::uninitializeProperty(QtProperty *property)
{
    d_ptr->m_values.remove(property);
}

// QtDoublePropertyManager

class QtDoublePropertyManagerPrivate
{
    QtDoublePropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtDoublePropertyManager)
public:

    struct Data
    {
        Data() : val(0), min(-INT_MAX), max(INT_MAX), decimals(2) {}
        double val;
        double min;
        double max;
        int decimals;
    };

    QMap<const QtProperty *, Data> m_values;
};

/*!
    \class QtDoublePropertyManager

    \brief The QtDoublePropertyManager provides and manages double properties.

    A double property has a current value, and a range specifying the
    valid values. The range is defined by a minimum and a maximum
    value.

    The property's value and range can be retrieved using the value(),
    minimum() and maximum() functions, and can be set using the
    setValue(), setMinimum() and setMaximum() slots.
    Alternatively, the range can be defined in one go using the
    setRange() slot.

    In addition, QtDoublePropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes, and the rangeChanged() signal which is emitted whenever
    such a property changes its range of valid values.

    \sa QtAbstractPropertyManager, QtDoubleSpinBoxFactory
*/

/*!
    \fn void QtDoublePropertyManager::valueChanged(QtProperty *property, double value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the new
    \a value as parameters.

    \sa setValue()
*/

/*!
    \fn void QtDoublePropertyManager::rangeChanged(QtProperty *property, double minimum, double maximum)

    This signal is emitted whenever a property created by this manager
    changes its range of valid values, passing a pointer to the
    \a property and the new \a minimum and \a maximum values

    \sa setRange()
*/

/*!
    \fn void QtDoublePropertyManager::decimalsChanged(QtProperty *property, int prec)

    This signal is emitted whenever a property created by this manager
    changes its precision of value, passing a pointer to the
    \a property and the new \a prec value

    \sa setDecimals()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtDoublePropertyManager::QtDoublePropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtDoublePropertyManagerPrivate;
    d_ptr->q_ptr = this;
}

/*!
    Destroys  this manager, and all the properties it has created.
*/
QtDoublePropertyManager::~QtDoublePropertyManager()
{
    delete d_ptr;
}

/*!
    Returns the given \a property's value.

    If the given property is not managed by this manager, this
    function returns 0.

    \sa setValue()
*/
double QtDoublePropertyManager::value(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return 0.0;
    return d_ptr->m_values[property].val;
}

/*!
    Returns the given \a property's minimum value.

    \sa maximum(), setRange()
*/
double QtDoublePropertyManager::minimum(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return 0.0;
    return d_ptr->m_values[property].min;
}

/*!
    Returns the given \a property's maximum value.

    \sa minimum(), setRange()
*/
double QtDoublePropertyManager::maximum(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return 0.0;
    return d_ptr->m_values[property].max;
}

/*!
    Returns the given \a property's precision, in decimals.

    \sa setDecimals()
*/
int QtDoublePropertyManager::decimals(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return 0;
    return d_ptr->m_values[property].decimals;
}

/*!
    \reimp
*/
QString QtDoublePropertyManager::valueText(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QString();
    return QString::number(value(property), 'f', decimals(property));
}

/*!
    \fn void QtDoublePropertyManager::setValue(QtProperty *property, double value)

    Sets the value of the given \a property to \a value.

    If the specified \a value is not valid according to the given
    \a property's range, the \a value is adjusted to the nearest valid value
    within the range.

    \sa value(), setRange(), valueChanged()
*/
void QtDoublePropertyManager::setValue(QtProperty *property, double val)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtDoublePropertyManagerPrivate::Data data = d_ptr->m_values[property];

    if (data.val == val)
        return;

    double oldVal = data.val;

    if (val < data.min)
        data.val = data.min;
    else if (val > data.max)
        data.val = data.max;
    else
        data.val = val;

    if (data.val == oldVal)
        return;

    d_ptr->m_values[property] = data;

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    \fn void QtDoublePropertyManager::setDecimals(QtProperty *property, int prec)

    Sets the precision of the given \a property to \a prec.

    The valid decimal range is 0-13. The default is 2.

    \sa decimals()
*/
void QtDoublePropertyManager::setDecimals(QtProperty *property, int prec)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtDoublePropertyManagerPrivate::Data data = d_ptr->m_values[property];

    if (prec > 13)
        prec = 13;
    else if (prec < 0)
        prec = 0;

    if (data.decimals == prec)
        return;

    data.decimals = prec;

    d_ptr->m_values[property] = data;

    emit decimalsChanged(property, data.decimals);
}

/*!
    Sets the minimum value for the given \a property to \a min.

    When setting the minimum value, the maximum and current values are
    adjusted if necessary (ensuring that the range remains valid and
    that the current value is within in the range).

    \sa minimum(), setRange(), rangeChanged()
*/
void QtDoublePropertyManager::setMinimum(QtProperty *property, double min)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtDoublePropertyManagerPrivate::Data data = d_ptr->m_values[property];

    if (data.min == min)
        return;

    double oldVal = data.val;

    data.min = min;
    if (data.max < data.min)
        data.max = data.min;

    if (data.val < data.min)
        data.val = data.min;

    d_ptr->m_values[property] = data;

    emit rangeChanged(property, data.min, data.max);

    if (data.val == oldVal)
        return;

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    Sets the maximum value for the given \a property to \a max.

    When setting the maximum value, the minimum and current values are
    adjusted if necessary (ensuring that the range remains valid and
    that the current value is within in the range).

    \sa maximum(), setRange(), rangeChanged()
*/
void QtDoublePropertyManager::setMaximum(QtProperty *property, double max)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtDoublePropertyManagerPrivate::Data data = d_ptr->m_values[property];

    if (data.max == max)
        return;

    double oldVal = data.val;

    data.max = max;
    if (data.min > data.max)
        data.min = data.max;

    if (data.val > data.max)
        data.val = data.max;

    d_ptr->m_values[property] = data;

    emit rangeChanged(property, data.min, data.max);

    if (data.val == oldVal)
        return;

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    \fn void QtDoublePropertyManager::setRange(QtProperty *property, double minimum, double maximum)

    Sets the range of valid values.

    This is a convenience function defining the range of valid values
    in one go; setting the \a minimum and \a maximum values for the
    given \a property with a single function call.

    When setting a new range, the current value is adjusted if
    necessary (ensuring that the value remains within range).

    \sa setMinimum(), setMaximum(), rangeChanged()
*/
void QtDoublePropertyManager::setRange(QtProperty *property, double min, double max)
{
    if (min > max) {
        double temp = min;
        min = max;
        max = temp;
    }

    if (!d_ptr->m_values.contains(property))
        return;

    QtDoublePropertyManagerPrivate::Data data = d_ptr->m_values[property];

    if (data.min == min && data.max == max)
        return;

    double oldVal = data.val;

    data.min = min;
    data.max = max;

    if (data.val < data.min)
        data.val = data.min;
    else if (data.val > data.max)
        data.val = data.max;

    d_ptr->m_values[property] = data;

    emit rangeChanged(property, data.min, data.max);

    if (data.val == oldVal)
        return;

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    \reimp
*/
void QtDoublePropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QtDoublePropertyManagerPrivate::Data();
}

/*!
    \reimp
*/
void QtDoublePropertyManager::uninitializeProperty(QtProperty *property)
{
    d_ptr->m_values.remove(property);
}

// QtStringPropertyManager

class QtStringPropertyManagerPrivate
{
    QtStringPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtStringPropertyManager)
public:

    struct Data
    {
        Data()
        {
            regExp.setPatternSyntax(QRegExp::Wildcard);
            regExp.setPattern("*");
        }
        QString val;
        QRegExp regExp;
    };

    QMap<const QtProperty *, Data> m_values;
};

/*!
    \class QtStringPropertyManager

    \brief The QtStringPropertyManager provides and manages QString properties.

    A string property's value can be retrieved using the value()
    function, and set using the setValue() slot.

    The current value can be checked against a regular expression. To
    set the regular expression use the setRegExp() slot, use the
    regExp() function to retrieve the currently set expression.

    In addition, QtStringPropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes, and the regExpChanged() signal which is emitted whenever
    such a property changes its currently set regular expression.

    \sa QtAbstractPropertyManager, QtLineEditFactory
*/

/*!
    \fn void QtStringPropertyManager::valueChanged(QtProperty *property, const QString &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the
    new \a value as parameters.

    \sa setValue()
*/

/*!
    \fn void QtStringPropertyManager::regExpChanged(QtProperty *property, const QRegExp &regExp)

    This signal is emitted whenever a property created by this manager
    changes its currenlty set regular expression, passing a pointer to
    the \a property and the new \a regExp as parameters.

    \sa setRegExp()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtStringPropertyManager::QtStringPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtStringPropertyManagerPrivate;
    d_ptr->q_ptr = this;
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtStringPropertyManager::~QtStringPropertyManager()
{
    delete d_ptr;
}

/*!
    Returns the given \a property's value.

    If the given property is not managed by this manager, this
    function returns an empty string.

    \sa setValue()
*/
QString QtStringPropertyManager::value(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QString();
    return d_ptr->m_values[property].val;
}

/*!
    Returns the given \a property's currently set regular expression.

    If the given \a property is not managed by this manager, this
    function returns an empty expression.

    \sa setRegExp()
*/
QRegExp QtStringPropertyManager::regExp(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QRegExp();
    return d_ptr->m_values[property].regExp;
}

/*!
    \reimp
*/
QString QtStringPropertyManager::valueText(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QString();
    return value(property);
}

/*!
    \fn void QtStringPropertyManager::setValue(QtProperty *property, const QString &value)

    Sets the value of the given \a property to \a value.

    If the specified \a value doesn't match the given \a property's
    regular expression, this function does nothing.

    \sa value(), setRegExp(), valueChanged()
*/
void QtStringPropertyManager::setValue(QtProperty *property, const QString &val)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtStringPropertyManagerPrivate::Data data = d_ptr->m_values[property];

    if (data.val == val)
        return;

    if (data.regExp.isValid() && !data.regExp.exactMatch(val))
        return;

    data.val = val;

    d_ptr->m_values[property] = data;

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    Sets the regular expression of the given \a property to \a regExp.

    \sa regExp(), setValue(), regExpChanged()
*/
void QtStringPropertyManager::setRegExp(QtProperty *property, const QRegExp &regExp)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtStringPropertyManagerPrivate::Data data = d_ptr->m_values[property];

    if (data.regExp == regExp)
        return;

    data.regExp = regExp;

    d_ptr->m_values[property] = data;

    emit regExpChanged(property, data.regExp);
}

/*!
    \reimp
*/
void QtStringPropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QtStringPropertyManagerPrivate::Data();
}

/*!
    \reimp
*/
void QtStringPropertyManager::uninitializeProperty(QtProperty *property)
{
    d_ptr->m_values.remove(property);
}

// QtBoolPropertyManager

class QtBoolPropertyManagerPrivate
{
    QtBoolPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtBoolPropertyManager)
public:

    QMap<const QtProperty *, bool> m_values;
};

/*!
    \class QtBoolPropertyManager

    \brief The QtBoolPropertyManager class provides and manages boolean properties.

    The property's value can be retrieved using the value() function,
    and set using the setValue() slot.

    In addition, QtBoolPropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes.

    \sa QtAbstractPropertyManager, QtCheckBoxFactory
*/

/*!
    \fn void QtBoolPropertyManager::valueChanged(QtProperty *property, bool value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the
    new \a value as parameters.
*/

/*!
    Creates a manager with the given \a parent.
*/
QtBoolPropertyManager::QtBoolPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtBoolPropertyManagerPrivate;
    d_ptr->q_ptr = this;
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtBoolPropertyManager::~QtBoolPropertyManager()
{
    delete d_ptr;
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by \e this manager, this
    function returns false.

    \sa setValue()
*/
bool QtBoolPropertyManager::value(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return false;
    return d_ptr->m_values[property];
}

/*!
    \reimp
*/
QString QtBoolPropertyManager::valueText(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QString();
    bool v = value(property);
    if (v)
        return tr("True");
    return tr("False");
}

/*!
    \reimp
*/
QIcon QtBoolPropertyManager::valueIcon(const QtProperty *) const
{
    return QIcon();
}

/*!
    \fn void QtBoolPropertyManager::setValue(QtProperty *property, bool value)

    Sets the value of the given \a property to \a value.

    \sa value()
*/
void QtBoolPropertyManager::setValue(QtProperty *property, bool val)
{
    if (!d_ptr->m_values.contains(property))
        return;

    bool v = value(property);
    if (v == val)
        return;
    d_ptr->m_values[property] = val;
    emit propertyChanged(property);
    emit valueChanged(property, val);
}

/*!
    \reimp
*/
void QtBoolPropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = false;
}

/*!
    \reimp
*/
void QtBoolPropertyManager::uninitializeProperty(QtProperty *property)
{
    d_ptr->m_values.remove(property);
}

// QtDatePropertyManager

class QtDatePropertyManagerPrivate
{
    QtDatePropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtDatePropertyManager)
public:

    struct Data
    {
        Data() : val(QDate::currentDate()), min(QDate(1752, 9, 14)),
                max(QDate(7999, 12, 31)) {}
        QDate val;
        QDate min;
        QDate max;
    };

    QString m_format;

    QMap<const QtProperty *, Data> m_values;
};

/*!
    \class QtDatePropertyManager

    \brief The QtDatePropertyManager provides and manages QDate properties.

    A date property has a current value, and a range specifying the
    valid dates. The range is defined by a minimum and a maximum
    value.

    The property's values can be retrieved using the minimum(),
    maximum() and value() functions, and can be set using the
    setMinimum(), setMaximum() and setValue() slots. Alternatively,
    the range can be defined in one go using the setRange() slot.

    In addition, QtDatePropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes, and the rangeChanged() signal which is emitted whenever
    such a property changes its range of valid dates.

    \sa QtAbstractPropertyManager, QtDateEditFactory, QtDateTimePropertyManager
*/

/*!
    \fn void QtDatePropertyManager::valueChanged(QtProperty *property, const QDate &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the new
    \a value as parameters.

    \sa setValue()
*/

/*!
    \fn void QtDatePropertyManager::rangeChanged(QtProperty *property, const QDate &minimum, const QDate &maximum)

    This signal is emitted whenever a property created by this manager
    changes its range of valid dates, passing a pointer to the \a
    property and the new \a minimum and \a maximum dates.

    \sa setRange()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtDatePropertyManager::QtDatePropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtDatePropertyManagerPrivate;
    d_ptr->q_ptr = this;

    QLocale loc;
    d_ptr->m_format = loc.dateFormat(QLocale::ShortFormat);
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtDatePropertyManager::~QtDatePropertyManager()
{
    delete d_ptr;
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by \e this manager, this
    function returns an invalid date.

    \sa setValue()
*/
QDate QtDatePropertyManager::value(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QDate();
    return d_ptr->m_values[property].val;
}

/*!
    Returns the given \a  property's  minimum date.

    \sa maximum(), setRange()
*/
QDate QtDatePropertyManager::minimum(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QDate();
    return d_ptr->m_values[property].min;
}

/*!
    Returns the given \a property's maximum date.

    \sa minimum(), setRange()
*/
QDate QtDatePropertyManager::maximum(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QDate();
    return d_ptr->m_values[property].max;
}

/*!
    \reimp
*/
QString QtDatePropertyManager::valueText(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QString();
    return value(property).toString(d_ptr->m_format);
}

/*!
    \fn void QtDatePropertyManager::setValue(QtProperty *property, const QDate &value)

    Sets the value of the given \a property to \a value.

    If the specified \a value is not a valid date according to the
    given \a property's range, the value is adjusted to the nearest
    valid value within the range.

    \sa value(), setRange(), valueChanged()
*/
void QtDatePropertyManager::setValue(QtProperty *property, const QDate &val)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtDatePropertyManagerPrivate::Data data = d_ptr->m_values[property];

    if (data.val == val)
        return;

    QDate oldVal = data.val;

    if (val < data.min)
        data.val = data.min;
    else if (val > data.max)
        data.val = data.max;
    else
        data.val = val;

    if (data.val == oldVal)
        return;

    d_ptr->m_values[property] = data;

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    Sets the minimum value for the given \a property to \a min.

    When setting the minimum value, the maximum and current values are
    adjusted if necessary (ensuring that the range remains valid and
    that the current value is within in the range).

    \sa minimum(), setRange()
*/
void QtDatePropertyManager::setMinimum(QtProperty *property, const QDate &min)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtDatePropertyManagerPrivate::Data data = d_ptr->m_values[property];

    if (data.min == min)
        return;

    QDate oldVal = data.val;

    data.min = min;
    if (data.max < data.min)
        data.max = data.min;

    if (data.val < data.min)
        data.val = data.min;

    d_ptr->m_values[property] = data;

    emit rangeChanged(property, data.min, data.max);

    if (data.val == oldVal)
        return;

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    Sets the maximum value for the given \a property to \a max.

    When setting the maximum value, the minimum and current
    values are adjusted if necessary (ensuring that the range remains
    valid and that the current value is within in the range).

    \sa maximum(), setRange()
*/
void QtDatePropertyManager::setMaximum(QtProperty *property, const QDate &max)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtDatePropertyManagerPrivate::Data data = d_ptr->m_values[property];

    if (data.max == max)
        return;

    QDate oldVal = data.val;

    data.max = max;
    if (data.min > data.max)
        data.min = data.max;

    if (data.val > data.max)
        data.val = data.max;

    d_ptr->m_values[property] = data;

    emit rangeChanged(property, data.min, data.max);

    if (data.val == oldVal)
        return;

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    \fn void QtDatePropertyManager::setRange(QtProperty *property, const QDate &minimum, const QDate &maximum)

    Sets the range of valid dates.

    This is a convenience function defining the range of valid dates
    in one go; setting the \a minimum and \a maximum values for the
    given \a property with a single function call.

    When setting a new date range, the current value is adjusted if
    necessary (ensuring that the value remains in date range).

    \sa setMinimum(), setMaximum(), rangeChanged()
*/
void QtDatePropertyManager::setRange(QtProperty *property, const QDate &min, const QDate &max)
{
    QDate fromDate = min;
    QDate toDate = max;
    if (min > max) {
        QDate fromDate = max;
        QDate toDate = min;
    }

    if (!d_ptr->m_values.contains(property))
        return;

    QtDatePropertyManagerPrivate::Data data = d_ptr->m_values[property];

    if (data.min == fromDate && data.max == toDate)
        return;

    QDate oldVal = data.val;

    data.min = fromDate;
    data.max = toDate;

    if (data.val < data.min)
        data.val = data.min;
    else if (data.val > data.max)
        data.val = data.max;

    d_ptr->m_values[property] = data;

    emit rangeChanged(property, data.min, data.max);

    if (data.val == oldVal)
        return;

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    \reimp
*/
void QtDatePropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QtDatePropertyManagerPrivate::Data();
}

/*!
    \reimp
*/
void QtDatePropertyManager::uninitializeProperty(QtProperty *property)
{
    d_ptr->m_values.remove(property);
}

// QtTimePropertyManager

class QtTimePropertyManagerPrivate
{
    QtTimePropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtTimePropertyManager)
public:

    QString m_format;

    QMap<const QtProperty *, QTime> m_values;
};

/*!
    \class QtTimePropertyManager

    \brief The QtTimePropertyManager provides and manages QTime properties.

    A time property's value can be retrieved using the value()
    function, and set using the setValue() slot.

    In addition, QtTimePropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes.

    \sa QtAbstractPropertyManager, QtTimeEditFactory
*/

/*!
    \fn void QtTimePropertyManager::valueChanged(QtProperty *property, const QTime &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the
    new \a value as parameters.

    \sa setValue()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtTimePropertyManager::QtTimePropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtTimePropertyManagerPrivate;
    d_ptr->q_ptr = this;

    QLocale loc;
    d_ptr->m_format = loc.timeFormat(QLocale::ShortFormat);
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtTimePropertyManager::~QtTimePropertyManager()
{
    delete d_ptr;
}

/*!
    Returns the given \a property's value.

    If the given property is not managed by this manager, this
    function returns an invalid time object.

    \sa setValue()
*/
QTime QtTimePropertyManager::value(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QTime();
    return d_ptr->m_values[property];
}

/*!
    \reimp
*/
QString QtTimePropertyManager::valueText(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QString();
    return value(property).toString(d_ptr->m_format);
}

/*!
    \fn void QtTimePropertyManager::setValue(QtProperty *property, const QTime &value)

    Sets the value of the given \a property to \a value.

    \sa value(), valueChanged()
*/
void QtTimePropertyManager::setValue(QtProperty *property, const QTime &val)
{
    if (!d_ptr->m_values.contains(property))
        return;

    if (d_ptr->m_values[property] == val)
        return;

    d_ptr->m_values[property] = val;

    emit propertyChanged(property);
    emit valueChanged(property, val);
}

/*!
    \reimp
*/
void QtTimePropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QTime::currentTime();
}

/*!
    \reimp
*/
void QtTimePropertyManager::uninitializeProperty(QtProperty *property)
{
    d_ptr->m_values.remove(property);
}

// QtDateTimePropertyManager

class QtDateTimePropertyManagerPrivate
{
    QtDateTimePropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtDateTimePropertyManager)
public:

    QString m_format;

    QMap<const QtProperty *, QDateTime> m_values;
};

/*! \class QtDateTimePropertyManager

    \brief The QtDateTimePropertyManager provides and manages QDateTime properties.

    A date and time property has a current value which can be
    retrieved using the value() function, and set using the setValue()
    slot. In addition, QtDateTimePropertyManager provides the
    valueChanged() signal which is emitted whenever a property created
    by this manager changes.

    \sa QtAbstractPropertyManager, QtDateTimeEditFactory, QtDatePropertyManager
*/

/*!
    \fn void QtDateTimePropertyManager::valueChanged(QtProperty *property, const QDateTime &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the new
    \a value as parameters.
*/

/*!
    Creates a manager with the given \a parent.
*/
QtDateTimePropertyManager::QtDateTimePropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtDateTimePropertyManagerPrivate;
    d_ptr->q_ptr = this;

    QLocale loc;
    d_ptr->m_format = loc.dateFormat(QLocale::ShortFormat) +
            QLatin1String(" ") + loc.timeFormat(QLocale::ShortFormat);
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtDateTimePropertyManager::~QtDateTimePropertyManager()
{
    delete d_ptr;
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by this manager, this
    function returns an invalid QDateTime object.

    \sa setValue()
*/
QDateTime QtDateTimePropertyManager::value(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QDateTime();
    return d_ptr->m_values[property];
}

/*!
    \reimp
*/
QString QtDateTimePropertyManager::valueText(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QString();
    return value(property).toString(d_ptr->m_format);
}

/*!
    \fn void QtDateTimePropertyManager::setValue(QtProperty *property, const QDateTime &value)

    Sets the value of the given \a property to \a value.

    \sa value(), valueChanged()
*/
void QtDateTimePropertyManager::setValue(QtProperty *property, const QDateTime &val)
{
    if (!d_ptr->m_values.contains(property))
        return;

    if (d_ptr->m_values[property] == val)
        return;

    d_ptr->m_values[property] = val;

    emit propertyChanged(property);
    emit valueChanged(property, val);
}

/*!
    \reimp
*/
void QtDateTimePropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QDateTime::currentDateTime();
}

/*!
    \reimp
*/
void QtDateTimePropertyManager::uninitializeProperty(QtProperty *property)
{
    d_ptr->m_values.remove(property);
}

// QtKeySequencePropertyManager

class QtKeySequencePropertyManagerPrivate
{
    QtKeySequencePropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtKeySequencePropertyManager)
public:

    QString m_format;

    QMap<const QtProperty *, QKeySequence> m_values;
};

/*! \class QtKeySequencePropertyManager

    \brief The QtKeySequencePropertyManager provides and manages QKeySequence properties.

    A key sequence's value can be retrieved using the value()
    function, and set using the setValue() slot.

    In addition, QtKeySequencePropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes.

    \sa QtAbstractPropertyManager
*/

/*!
    \fn void QtKeySequencePropertyManager::valueChanged(QtProperty *property, const QKeySequence &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the new
    \a value as parameters.
*/

/*!
    Creates a manager with the given \a parent.
*/
QtKeySequencePropertyManager::QtKeySequencePropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtKeySequencePropertyManagerPrivate;
    d_ptr->q_ptr = this;
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtKeySequencePropertyManager::~QtKeySequencePropertyManager()
{
    delete d_ptr;
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by this manager, this
    function returns an empty QKeySequence object.

    \sa setValue()
*/
QKeySequence QtKeySequencePropertyManager::value(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QKeySequence();
    return d_ptr->m_values[property];
}

/*!
    \reimp
*/
QString QtKeySequencePropertyManager::valueText(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QString();
    return value(property).toString(QKeySequence::NativeText);
}

/*!
    \fn void QtKeySequencePropertyManager::setValue(QtProperty *property, const QKeySequence &value)

    Sets the value of the given \a property to \a value.

    \sa value(), valueChanged()
*/
void QtKeySequencePropertyManager::setValue(QtProperty *property, const QKeySequence &val)
{
    if (!d_ptr->m_values.contains(property))
        return;

    if (d_ptr->m_values[property] == val)
        return;

    d_ptr->m_values[property] = val;

    emit propertyChanged(property);
    emit valueChanged(property, val);
}

/*!
    \reimp
*/
void QtKeySequencePropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QKeySequence();
}

/*!
    \reimp
*/
void QtKeySequencePropertyManager::uninitializeProperty(QtProperty *property)
{
    d_ptr->m_values.remove(property);
}

// QtLocalePropertyManager

class QtLocalePropertyManagerPrivate
{
    QtLocalePropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtLocalePropertyManager)
public:

    QtLocalePropertyManagerPrivate();

    void slotEnumChanged(QtProperty *property, int value);
    void slotPropertyDestroyed(QtProperty *property);

    QMap<const QtProperty *, QLocale> m_values;

    QtEnumPropertyManager *m_enumPropertyManager;

    QMap<const QtProperty *, QtProperty *> m_propertyToLanguage;
    QMap<const QtProperty *, QtProperty *> m_propertyToCountry;

    QMap<const QtProperty *, QtProperty *> m_languageToProperty;
    QMap<const QtProperty *, QtProperty *> m_countryToProperty;
};

QtLocalePropertyManagerPrivate::QtLocalePropertyManagerPrivate()
{
}

void QtLocalePropertyManagerPrivate::slotEnumChanged(QtProperty *property, int value)
{
    if (m_languageToProperty.contains(property)) {
        QtProperty *prop = m_languageToProperty[property];
        QLocale loc = m_values[prop];
        QLocale::Language newLanguage = loc.language();
        QLocale::Country newCountry = loc.country();
        metaEnumProvider()->indexToLocale(value, 0, &newLanguage, 0);
        QLocale newLoc(newLanguage, newCountry);
        q_ptr->setValue(prop, newLoc);
    } else if (m_countryToProperty.contains(property)) {
        QtProperty *prop = m_countryToProperty[property];
        QLocale loc = m_values[prop];
        QLocale::Language newLanguage = loc.language();
        QLocale::Country newCountry = loc.country();
        metaEnumProvider()->indexToLocale(m_enumPropertyManager->value(m_propertyToLanguage.value(prop)), value, &newLanguage, &newCountry);
        QLocale newLoc(newLanguage, newCountry);
        q_ptr->setValue(prop, newLoc);
    }
}

void QtLocalePropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
    if (m_languageToProperty.contains(property)) {
        QtProperty *subProp = m_languageToProperty[property];
        m_propertyToLanguage[subProp] = 0;
        m_languageToProperty.remove(property);
    } else if (m_countryToProperty.contains(property)) {
        QtProperty *subProp = m_countryToProperty[property];
        m_propertyToCountry[subProp] = 0;
        m_countryToProperty.remove(property);
    }
}

/*!
    \class QtLocalePropertyManager

    \brief The QtLocalePropertyManager provides and manages QLocale properties.

    A locale property has nested \e language and \e country
    subproperties. The top-level property's value can be retrieved
    using the value() function, and set using the setValue() slot.

    The subproperties are created by QtEnumPropertyManager object.
    These submanager can be retrieved using the subEnumPropertyManager()
    function. In order to provide editing widgets for the subproperties
    in a property browser widget, this manager must be associated with editor factory.

    In addition, QtLocalePropertyManager provides the valueChanged()
    signal which is emitted whenever a property created by this
    manager changes.

    \sa QtAbstractPropertyManager, QtEnumPropertyManager
*/

/*!
    \fn void QtLocalePropertyManager::valueChanged(QtProperty *property, const QLocale &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the
    new \a value as parameters.

    \sa setValue()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtLocalePropertyManager::QtLocalePropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtLocalePropertyManagerPrivate;
    d_ptr->q_ptr = this;

    d_ptr->m_enumPropertyManager = new QtEnumPropertyManager(this);
    connect(d_ptr->m_enumPropertyManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotEnumChanged(QtProperty *, int)));

    connect(d_ptr->m_enumPropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtLocalePropertyManager::~QtLocalePropertyManager()
{
    delete d_ptr;
}

/*!
    Returns the manager that creates the nested \e language
    and \e country subproperties.

    In order to provide editing widgets for the mentioned subproperties
    in a property browser widget, this manager must be associated with
    an editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtEnumPropertyManager *QtLocalePropertyManager::subEnumPropertyManager() const
{
    return d_ptr->m_enumPropertyManager;
}

/*!
    Returns the given \a property's value.

    If the given property is not managed by this manager, this
    function returns the default locale.

    \sa setValue()
*/
QLocale QtLocalePropertyManager::value(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QLocale();
    return d_ptr->m_values[property];
}

/*!
    \reimp
*/
QString QtLocalePropertyManager::valueText(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QString();

    QLocale loc = d_ptr->m_values[property];

    int langIdx = 0;
    int countryIdx = 0;
    metaEnumProvider()->localeToIndex(loc.language(), loc.country(), &langIdx, &countryIdx);
    QString str = tr("%1, %2")
            .arg(metaEnumProvider()->languageEnumNames().at(langIdx))
            .arg(metaEnumProvider()->countryEnumNames(loc.language()).at(countryIdx));
    return str;
}

/*!
    \fn void QtLocalePropertyManager::setValue(QtProperty *property, const QLocale &value)

    Sets the value of the given \a property to \a value. Nested
    properties are updated automatically.

    \sa value(), valueChanged()
*/
void QtLocalePropertyManager::setValue(QtProperty *property, const QLocale &val)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QLocale loc = d_ptr->m_values[property];
    if (loc == val)
        return;

    d_ptr->m_values[property] = val;

    int langIdx = 0;
    int countryIdx = 0;
    metaEnumProvider()->localeToIndex(val.language(), val.country(), &langIdx, &countryIdx);
    if (loc.language() != val.language()) {
        d_ptr->m_enumPropertyManager->setValue(d_ptr->m_propertyToLanguage.value(property), langIdx);
        d_ptr->m_enumPropertyManager->setEnumNames(d_ptr->m_propertyToCountry.value(property),
                    metaEnumProvider()->countryEnumNames(val.language()));
    }
    d_ptr->m_enumPropertyManager->setValue(d_ptr->m_propertyToCountry.value(property), countryIdx);

    emit propertyChanged(property);
    emit valueChanged(property, val);
}

/*!
    \reimp
*/
void QtLocalePropertyManager::initializeProperty(QtProperty *property)
{
    QLocale val;
    d_ptr->m_values[property] = val;

    int langIdx = 0;
    int countryIdx = 0;
    metaEnumProvider()->localeToIndex(val.language(), val.country(), &langIdx, &countryIdx);

    QtProperty *languageProp = d_ptr->m_enumPropertyManager->addProperty();
    languageProp->setPropertyName(tr("Language"));
    d_ptr->m_enumPropertyManager->setEnumNames(languageProp, metaEnumProvider()->languageEnumNames());
    d_ptr->m_enumPropertyManager->setValue(languageProp, langIdx);
    d_ptr->m_propertyToLanguage[property] = languageProp;
    d_ptr->m_languageToProperty[languageProp] = property;
    property->addSubProperty(languageProp);

    QtProperty *countryProp = d_ptr->m_enumPropertyManager->addProperty();
    countryProp->setPropertyName(tr("Country"));
    d_ptr->m_enumPropertyManager->setEnumNames(countryProp, metaEnumProvider()->countryEnumNames(val.language()));
    d_ptr->m_enumPropertyManager->setValue(countryProp, countryIdx);
    d_ptr->m_propertyToCountry[property] = countryProp;
    d_ptr->m_countryToProperty[countryProp] = property;
    property->addSubProperty(countryProp);
}

/*!
    \reimp
*/
void QtLocalePropertyManager::uninitializeProperty(QtProperty *property)
{
    QtProperty *languageProp = d_ptr->m_propertyToLanguage[property];
    if (languageProp) {
        d_ptr->m_languageToProperty.remove(languageProp);
        delete languageProp;
    }
    d_ptr->m_propertyToLanguage.remove(property);

    QtProperty *countryProp = d_ptr->m_propertyToCountry[property];
    if (countryProp) {
        d_ptr->m_countryToProperty.remove(countryProp);
        delete countryProp;
    }
    d_ptr->m_propertyToCountry.remove(property);

    d_ptr->m_values.remove(property);
}

// QtPointPropertyManager

class QtPointPropertyManagerPrivate
{
    QtPointPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtPointPropertyManager)
public:

    void slotIntChanged(QtProperty *property, int value);
    void slotPropertyDestroyed(QtProperty *property);

    QMap<const QtProperty *, QPoint> m_values;

    QtIntPropertyManager *m_intPropertyManager;

    QMap<const QtProperty *, QtProperty *> m_propertyToX;
    QMap<const QtProperty *, QtProperty *> m_propertyToY;

    QMap<const QtProperty *, QtProperty *> m_xToProperty;
    QMap<const QtProperty *, QtProperty *> m_yToProperty;
};

void QtPointPropertyManagerPrivate::slotIntChanged(QtProperty *property, int value)
{
    if (m_xToProperty.contains(property)) {
        QtProperty *prop = m_xToProperty[property];
        QPoint p = m_values[prop];
        p.setX(value);
        q_ptr->setValue(prop, p);
    } else if (m_yToProperty.contains(property)) {
        QtProperty *prop = m_yToProperty[property];
        QPoint p = m_values[prop];
        p.setY(value);
        q_ptr->setValue(prop, p);
    }
}

void QtPointPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
    if (m_xToProperty.contains(property)) {
        QtProperty *pointProp = m_xToProperty[property];
        m_propertyToX[pointProp] = 0;
        m_xToProperty.remove(property);
    } else if (m_yToProperty.contains(property)) {
        QtProperty *pointProp = m_yToProperty[property];
        m_propertyToY[pointProp] = 0;
        m_yToProperty.remove(property);
    }
}

/*! \class QtPointPropertyManager

    \brief The QtPointPropertyManager provides and manages QPoint properties.

    A point property has nested \e x and \e y subproperties. The
    top-level property's value can be retrieved using the value()
    function, and set using the setValue() slot.

    The subproperties are created by a QtIntPropertyManager object. This
    manager can be retrieved using the subIntPropertyManager() function. In
    order to provide editing widgets for the subproperties in a
    property browser widget, this manager must be associated with an
    editor factory.

    In addition, QtPointPropertyManager provides the valueChanged() signal which
    is emitted whenever a property created by this manager changes.

    \sa QtAbstractPropertyManager, QtIntPropertyManager, QtPointFPropertyManager
*/

/*!
    \fn void QtPointPropertyManager::valueChanged(QtProperty *property, const QPoint &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the
    new \a value as parameters.

    \sa setValue()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtPointPropertyManager::QtPointPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtPointPropertyManagerPrivate;
    d_ptr->q_ptr = this;

    d_ptr->m_intPropertyManager = new QtIntPropertyManager(this);
    connect(d_ptr->m_intPropertyManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotIntChanged(QtProperty *, int)));
    connect(d_ptr->m_intPropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtPointPropertyManager::~QtPointPropertyManager()
{
    delete d_ptr;
}

/*!
    Returns the manager that creates the nested \e x and \e y
    subproperties.

    In order to provide editing widgets for the subproperties in a
    property browser widget, this manager must be associated with an
    editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtIntPropertyManager *QtPointPropertyManager::subIntPropertyManager() const
{
    return d_ptr->m_intPropertyManager;
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by this manager, this
    function returns a point with coordinates (0, 0).

    \sa setValue()
*/
QPoint QtPointPropertyManager::value(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QPoint();
    return d_ptr->m_values[property];
}

/*!
    \reimp
*/
QString QtPointPropertyManager::valueText(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QString();
    QPoint v = value(property);
    return QString(tr("(%1, %2)").arg(QString::number(v.x()))
                                 .arg(QString::number(v.y())));
}

/*!
    \fn void QtPointPropertyManager::setValue(QtProperty *property, const QPoint &value)

    Sets the value of the given \a property to \a value. Nested
    properties are updated automatically.

    \sa value(), valueChanged()
*/
void QtPointPropertyManager::setValue(QtProperty *property, const QPoint &val)
{
    if (!d_ptr->m_values.contains(property))
        return;

    if (d_ptr->m_values[property] == val)
        return;

    d_ptr->m_values[property] = val;
    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToX[property], val.x());
    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToY[property], val.y());

    emit propertyChanged(property);
    emit valueChanged(property, val);
}

/*!
    \reimp
*/
void QtPointPropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QPoint(0, 0);

    QtProperty *xProp = d_ptr->m_intPropertyManager->addProperty();
    xProp->setPropertyName(tr("X"));
    d_ptr->m_intPropertyManager->setValue(xProp, 0);
    d_ptr->m_propertyToX[property] = xProp;
    d_ptr->m_xToProperty[xProp] = property;
    property->addSubProperty(xProp);

    QtProperty *yProp = d_ptr->m_intPropertyManager->addProperty();
    yProp->setPropertyName(tr("Y"));
    d_ptr->m_intPropertyManager->setValue(yProp, 0);
    d_ptr->m_propertyToY[property] = yProp;
    d_ptr->m_yToProperty[yProp] = property;
    property->addSubProperty(yProp);
}

/*!
    \reimp
*/
void QtPointPropertyManager::uninitializeProperty(QtProperty *property)
{
    QtProperty *xProp = d_ptr->m_propertyToX[property];
    if (xProp) {
        d_ptr->m_xToProperty.remove(xProp);
        delete xProp;
    }
    d_ptr->m_propertyToX.remove(property);

    QtProperty *yProp = d_ptr->m_propertyToY[property];
    if (yProp) {
        d_ptr->m_yToProperty.remove(yProp);
        delete yProp;
    }
    d_ptr->m_propertyToY.remove(property);

    d_ptr->m_values.remove(property);
}

// QtPointFPropertyManager

class QtPointFPropertyManagerPrivate
{
    QtPointFPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtPointFPropertyManager)
public:

    struct Data
    {
        Data() : decimals(2) {}
        QPointF val;
        int decimals;
    };

    void slotDoubleChanged(QtProperty *property, double value);
    void slotPropertyDestroyed(QtProperty *property);

    QMap<const QtProperty *, Data> m_values;

    QtDoublePropertyManager *m_doublePropertyManager;

    QMap<const QtProperty *, QtProperty *> m_propertyToX;
    QMap<const QtProperty *, QtProperty *> m_propertyToY;

    QMap<const QtProperty *, QtProperty *> m_xToProperty;
    QMap<const QtProperty *, QtProperty *> m_yToProperty;
};

void QtPointFPropertyManagerPrivate::slotDoubleChanged(QtProperty *property, double value)
{
    if (m_xToProperty.contains(property)) {
        QtProperty *prop = m_xToProperty[property];
        QPointF p = m_values[prop].val;
        p.setX(value);
        q_ptr->setValue(prop, p);
    } else if (m_yToProperty.contains(property)) {
        QtProperty *prop = m_yToProperty[property];
        QPointF p = m_values[prop].val;
        p.setY(value);
        q_ptr->setValue(prop, p);
    }
}

void QtPointFPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
    if (m_xToProperty.contains(property)) {
        QtProperty *pointProp = m_xToProperty[property];
        m_propertyToX[pointProp] = 0;
        m_xToProperty.remove(property);
    } else if (m_yToProperty.contains(property)) {
        QtProperty *pointProp = m_yToProperty[property];
        m_propertyToY[pointProp] = 0;
        m_yToProperty.remove(property);
    }
}

/*! \class QtPointFPropertyManager

    \brief The QtPointFPropertyManager provides and manages QPointF properties.

    A point property has nested \e x and \e y subproperties. The
    top-level property's value can be retrieved using the value()
    function, and set using the setValue() slot.

    The subproperties are created by a QtDoublePropertyManager object. This
    manager can be retrieved using the subDoublePropertyManager() function. In
    order to provide editing widgets for the subproperties in a
    property browser widget, this manager must be associated with an
    editor factory.

    In addition, QtPointFPropertyManager provides the valueChanged() signal which
    is emitted whenever a property created by this manager changes.

    \sa QtAbstractPropertyManager, QtDoublePropertyManager, QtPointPropertyManager
*/

/*!
    \fn void QtPointFPropertyManager::valueChanged(QtProperty *property, const QPointF &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the
    new \a value as parameters.

    \sa setValue()
*/

/*!
    \fn void QtPointFPropertyManager::decimalsChanged(QtProperty *property, int prec)

    This signal is emitted whenever a property created by this manager
    changes its precision of value, passing a pointer to the
    \a property and the new \a prec value

    \sa setDecimals()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtPointFPropertyManager::QtPointFPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtPointFPropertyManagerPrivate;
    d_ptr->q_ptr = this;

    d_ptr->m_doublePropertyManager = new QtDoublePropertyManager(this);
    connect(d_ptr->m_doublePropertyManager, SIGNAL(valueChanged(QtProperty *, double)),
                this, SLOT(slotDoubleChanged(QtProperty *, double)));
    connect(d_ptr->m_doublePropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtPointFPropertyManager::~QtPointFPropertyManager()
{
    delete d_ptr;
}

/*!
    Returns the manager that creates the nested \e x and \e y
    subproperties.

    In order to provide editing widgets for the subproperties in a
    property browser widget, this manager must be associated with an
    editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtDoublePropertyManager *QtPointFPropertyManager::subDoublePropertyManager() const
{
    return d_ptr->m_doublePropertyManager;
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by this manager, this
    function returns a point with coordinates (0, 0).

    \sa setValue()
*/
QPointF QtPointFPropertyManager::value(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QPointF();
    return d_ptr->m_values[property].val;
}

/*!
    Returns the given \a property's precision, in decimals.

    \sa setDecimals()
*/
int QtPointFPropertyManager::decimals(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return 0;
    return d_ptr->m_values[property].decimals;
}

/*!
    \reimp
*/
QString QtPointFPropertyManager::valueText(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QString();
    QPointF v = value(property);
    return QString(tr("(%1, %2)").arg(QString::number(v.x(), 'f', decimals(property)))
                                 .arg(QString::number(v.y(), 'f', decimals(property))));
}

/*!
    \fn void QtPointFPropertyManager::setValue(QtProperty *property, const QPointF &value)

    Sets the value of the given \a property to \a value. Nested
    properties are updated automatically.

    \sa value(), valueChanged()
*/
void QtPointFPropertyManager::setValue(QtProperty *property, const QPointF &val)
{
    if (!d_ptr->m_values.contains(property))
        return;

    if (d_ptr->m_values[property].val == val)
        return;

    d_ptr->m_values[property].val = val;
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToX[property], val.x());
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToY[property], val.y());

    emit propertyChanged(property);
    emit valueChanged(property, val);
}

/*!
    \fn void QtPointFPropertyManager::setDecimals(QtProperty *property, int prec)

    Sets the precision of the given \a property to \a prec.

    The valid decimal range is 0-13. The default is 2.

    \sa decimals()
*/
void QtPointFPropertyManager::setDecimals(QtProperty *property, int prec)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtPointFPropertyManagerPrivate::Data data = d_ptr->m_values[property];

    if (prec > 13)
        prec = 13;
    else if (prec < 0)
        prec = 0;

    if (data.decimals == prec)
        return;

    data.decimals = prec;
    d_ptr->m_doublePropertyManager->setDecimals(d_ptr->m_propertyToX[property], prec);
    d_ptr->m_doublePropertyManager->setDecimals(d_ptr->m_propertyToY[property], prec);

    d_ptr->m_values[property] = data;

    emit decimalsChanged(property, data.decimals);
}

/*!
    \reimp
*/
void QtPointFPropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QtPointFPropertyManagerPrivate::Data();

    QtProperty *xProp = d_ptr->m_doublePropertyManager->addProperty();
    xProp->setPropertyName(tr("X"));
    d_ptr->m_doublePropertyManager->setDecimals(xProp, decimals(property));
    d_ptr->m_doublePropertyManager->setValue(xProp, 0);
    d_ptr->m_propertyToX[property] = xProp;
    d_ptr->m_xToProperty[xProp] = property;
    property->addSubProperty(xProp);

    QtProperty *yProp = d_ptr->m_doublePropertyManager->addProperty();
    yProp->setPropertyName(tr("Y"));
    d_ptr->m_doublePropertyManager->setDecimals(yProp, decimals(property));
    d_ptr->m_doublePropertyManager->setValue(yProp, 0);
    d_ptr->m_propertyToY[property] = yProp;
    d_ptr->m_yToProperty[yProp] = property;
    property->addSubProperty(yProp);
}

/*!
    \reimp
*/
void QtPointFPropertyManager::uninitializeProperty(QtProperty *property)
{
    QtProperty *xProp = d_ptr->m_propertyToX[property];
    if (xProp) {
        d_ptr->m_xToProperty.remove(xProp);
        delete xProp;
    }
    d_ptr->m_propertyToX.remove(property);

    QtProperty *yProp = d_ptr->m_propertyToY[property];
    if (yProp) {
        d_ptr->m_yToProperty.remove(yProp);
        delete yProp;
    }
    d_ptr->m_propertyToY.remove(property);

    d_ptr->m_values.remove(property);
}

// QtSizePropertyManager

class QtSizePropertyManagerPrivate
{
    QtSizePropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtSizePropertyManager)
public:

    void slotIntChanged(QtProperty *property, int value);
    void slotPropertyDestroyed(QtProperty *property);
    void setRange(QtProperty *property,
                const QSize &min, const QSize &max, const QSize &val);

    struct Data
    {
        Data() : val(QSize(0, 0)), min(QSize(0, 0)), max(QSize(INT_MAX, INT_MAX)) {}
        QSize val;
        QSize min;
        QSize max;
    };

    QMap<const QtProperty *, Data> m_values;

    QtIntPropertyManager *m_intPropertyManager;

    QMap<const QtProperty *, QtProperty *> m_propertyToW;
    QMap<const QtProperty *, QtProperty *> m_propertyToH;

    QMap<const QtProperty *, QtProperty *> m_wToProperty;
    QMap<const QtProperty *, QtProperty *> m_hToProperty;
};

void QtSizePropertyManagerPrivate::slotIntChanged(QtProperty *property, int value)
{
    if (m_wToProperty.contains(property)) {
        QtProperty *prop = m_wToProperty[property];
        QSize s = m_values[prop].val;
        s.setWidth(value);
        q_ptr->setValue(prop, s);
    } else if (m_hToProperty.contains(property)) {
        QtProperty *prop = m_hToProperty[property];
        QSize s = m_values[prop].val;
        s.setHeight(value);
        q_ptr->setValue(prop, s);
    }
}

void QtSizePropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
    if (m_wToProperty.contains(property)) {
        QtProperty *pointProp = m_wToProperty[property];
        m_propertyToW[pointProp] = 0;
        m_wToProperty.remove(property);
    } else if (m_hToProperty.contains(property)) {
        QtProperty *pointProp = m_hToProperty[property];
        m_propertyToH[pointProp] = 0;
        m_hToProperty.remove(property);
    }
}

void QtSizePropertyManagerPrivate::setRange(QtProperty *property,
                const QSize &min, const QSize &max, const QSize &val)
{
    m_intPropertyManager->setRange(m_propertyToW[property], min.width(), max.width());
    m_intPropertyManager->setValue(m_propertyToW[property], val.width());
    m_intPropertyManager->setRange(m_propertyToH[property], min.height(), max.height());
    m_intPropertyManager->setValue(m_propertyToH[property], val.height());
}

/*!
    \class QtSizePropertyManager

    \brief The QtSizePropertyManager provides and manages QSize properties.

    A size property has nested \e width and \e height
    subproperties. The top-level property's value can be retrieved
    using the value() function, and set using the setValue() slot.

    The subproperties are created by a QtIntPropertyManager object. This
    manager can be retrieved using the subIntPropertyManager() function. In
    order to provide editing widgets for the subproperties in a
    property browser widget, this manager must be associated with an
    editor factory.

    A size property also has a range of valid values defined by a
    minimum size and a maximum size. These sizes can be retrieved
    using the minimum() and the maximum() functions, and set using the
    setMinimum() and setMaximum() slots. Alternatively, the range can
    be defined in one go using the setRange() slot.

    In addition, QtSizePropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes, and the rangeChanged() signal which is emitted whenever
    such a property changes its range of valid sizes.

    \sa QtAbstractPropertyManager, QtIntPropertyManager, QtSizeFPropertyManager
*/

/*!
    \fn void QtSizePropertyManager::valueChanged(QtProperty *property, const QSize &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the new
    \a value as parameters.

    \sa setValue()
*/

/*!
    \fn void QtSizePropertyManager::rangeChanged(QtProperty *property, const QSize &minimum, const QSize &maximum)

    This signal is emitted whenever a property created by this manager
    changes its range of valid sizes, passing a pointer to the \a
    property and the new \a minimum and \a maximum sizes.

    \sa setRange()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtSizePropertyManager::QtSizePropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtSizePropertyManagerPrivate;
    d_ptr->q_ptr = this;

    d_ptr->m_intPropertyManager = new QtIntPropertyManager(this);
    connect(d_ptr->m_intPropertyManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotIntChanged(QtProperty *, int)));
    connect(d_ptr->m_intPropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtSizePropertyManager::~QtSizePropertyManager()
{
    delete d_ptr;
}

/*!
    Returns the manager that creates the nested \e width and \e height
    subproperties.

    In order to provide editing widgets for the \e width and \e height
    properties in a property browser widget, this manager must be
    associated with an editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtIntPropertyManager *QtSizePropertyManager::subIntPropertyManager() const
{
    return d_ptr->m_intPropertyManager;
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by this manager, this
    function returns an invalid size

    \sa setValue()
*/
QSize QtSizePropertyManager::value(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QSize();
    return d_ptr->m_values[property].val;
}

/*!
    Returns the given \a property's minimum size value.

    \sa setMinimum(), maximum(), setRange()
*/
QSize QtSizePropertyManager::minimum(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QSize();
    return d_ptr->m_values[property].min;
}

/*!
    Returns the given \a property's maximum size value.

    \sa setMaximum(), minimum(), setRange()
*/
QSize QtSizePropertyManager::maximum(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QSize();
    return d_ptr->m_values[property].max;
}

/*!
    \reimp
*/
QString QtSizePropertyManager::valueText(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QString();
    QSize v = value(property);
    return QString(tr("%1 x %2").arg(QString::number(v.width()))
                                .arg(QString::number(v.height())));
}

/*!
    \fn void QtSizePropertyManager::setValue(QtProperty *property, const QSize &value)

    Sets the value of the given \a property to \a value.

    If the specified \a value is not valid according to the given \a
    property's size range, the \a value is adjusted to the nearest
    valid value within the size range.

    \sa value(), setRange(), valueChanged()
*/
void QtSizePropertyManager::setValue(QtProperty *property, const QSize &val)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtSizePropertyManagerPrivate::Data data = d_ptr->m_values[property];

    QSize newSize = val;
    if (data.min.width() > newSize.width())
        newSize.setWidth(data.min.width());
    else if (data.max.width() < newSize.width())
        newSize.setWidth(data.max.width());
    if (data.min.height() > newSize.height())
        newSize.setHeight(data.min.height());
    else if (data.max.height() < newSize.height())
        newSize.setHeight(data.max.height());

    if (data.val == newSize)
        return;

    data.val = newSize;

    d_ptr->m_values[property] = data;
    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToW[property], newSize.width());
    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToH[property], newSize.height());

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    Sets the minimum size value for the given \a property to \a min.

    When setting the minimum size value, the maximum and current
    values are adjusted if necessary (ensuring that the size range
    remains valid and that the current value is within the range).

    \sa minimum(), setRange(), rangeChanged()
*/
void QtSizePropertyManager::setMinimum(QtProperty *property, const QSize &min)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtSizePropertyManagerPrivate::Data data = d_ptr->m_values[property];

    QSize oldVal = data.val;

    data.min = min;
    if (data.max.width() < data.min.width())
        data.max.setWidth(data.min.width());
    if (data.max.height() < data.min.height())
        data.max.setHeight(data.min.height());

    if (data.val.width() < data.min.width())
        data.val.setWidth(data.min.width());
    if (data.val.height() < data.min.height())
        data.val.setHeight(data.min.height());

    d_ptr->m_values[property] = data;

    emit rangeChanged(property, data.min, data.max);

    d_ptr->setRange(property, data.min, data.max, data.val);

    if (data.val == oldVal)
        return;

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    Sets the maximum size value for the given \a property to \a max.

    When setting the maximum size value, the minimum and current
    values are adjusted if necessary (ensuring that the size range
    remains valid and that the current value is within the range).

    \sa maximum(), setRange(), rangeChanged()
*/
void QtSizePropertyManager::setMaximum(QtProperty *property, const QSize &max)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtSizePropertyManagerPrivate::Data data = d_ptr->m_values[property];

    QSize oldVal = data.val;

    data.max = max;
    if (data.min.width() > data.max.width())
        data.min.setWidth(data.max.width());
    if (data.min.height() > data.max.height())
        data.min.setHeight(data.max.height());

    if (data.val.width() > data.max.width())
        data.val.setWidth(data.max.width());
    if (data.val.height() > data.max.height())
        data.val.setHeight(data.max.height());

    d_ptr->m_values[property] = data;

    emit rangeChanged(property, data.min, data.max);

    d_ptr->setRange(property, data.min, data.max, data.val);

    if (data.val == oldVal)
        return;

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    \fn void QtSizePropertyManager::setRange(QtProperty *property, const QSize &minimum, const QSize &maximum)

    Sets the range of valid values.

    This is a convenience function defining the range of valid values
    in one go; setting the \a minimum and \a maximum values for the
    given \a property with a single function call.

    When setting a new range, the current value is adjusted if
    necessary (ensuring that the value remains within the range).

    \sa  setMinimum(), setMaximum(), rangeChanged()
*/
void QtSizePropertyManager::setRange(QtProperty *property, const QSize &min, const QSize &max)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QSize fromSize = min;
    QSize toSize = max;
    if (fromSize.width() > toSize.width()) {
        fromSize.setWidth(max.width());
        toSize.setWidth(min.width());
    }
    if (fromSize.height() > toSize.height()) {
        fromSize.setHeight(max.height());
        toSize.setHeight(min.height());
    }

    QtSizePropertyManagerPrivate::Data data = d_ptr->m_values[property];

    if (data.min == fromSize && data.max == toSize)
        return;

    QSize oldVal = data.val;

    data.min = min;
    data.max = max;

    if (data.val.width() < data.min.width())
        data.val.setWidth(data.min.width());
    else if (data.val.width() > data.max.width())
        data.val.setWidth(data.max.width());
    if (data.val.height() < data.min.height())
        data.val.setHeight(data.min.height());
    else if (data.val.height() > data.max.height())
        data.val.setHeight(data.max.height());

    d_ptr->m_values[property] = data;

    emit rangeChanged(property, data.min, data.max);

    d_ptr->setRange(property, data.min, data.max, data.val);

    if (data.val == oldVal)
        return;

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    \reimp
*/
void QtSizePropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QtSizePropertyManagerPrivate::Data();

    QtProperty *wProp = d_ptr->m_intPropertyManager->addProperty();
    wProp->setPropertyName(tr("Width"));
    d_ptr->m_intPropertyManager->setValue(wProp, 0);
    d_ptr->m_intPropertyManager->setMinimum(wProp, 0);
    d_ptr->m_propertyToW[property] = wProp;
    d_ptr->m_wToProperty[wProp] = property;
    property->addSubProperty(wProp);

    QtProperty *hProp = d_ptr->m_intPropertyManager->addProperty();
    hProp->setPropertyName(tr("Height"));
    d_ptr->m_intPropertyManager->setValue(hProp, 0);
    d_ptr->m_intPropertyManager->setMinimum(hProp, 0);
    d_ptr->m_propertyToH[property] = hProp;
    d_ptr->m_hToProperty[hProp] = property;
    property->addSubProperty(hProp);
}

/*!
    \reimp
*/
void QtSizePropertyManager::uninitializeProperty(QtProperty *property)
{
    QtProperty *wProp = d_ptr->m_propertyToW[property];
    if (wProp) {
        d_ptr->m_wToProperty.remove(wProp);
        delete wProp;
    }
    d_ptr->m_propertyToW.remove(property);

    QtProperty *hProp = d_ptr->m_propertyToH[property];
    if (hProp) {
        d_ptr->m_hToProperty.remove(hProp);
        delete hProp;
    }
    d_ptr->m_propertyToH.remove(property);

    d_ptr->m_values.remove(property);
}

// QtSizeFPropertyManager

class QtSizeFPropertyManagerPrivate
{
    QtSizeFPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtSizeFPropertyManager)
public:

    void slotDoubleChanged(QtProperty *property, double value);
    void slotPropertyDestroyed(QtProperty *property);
    void setRange(QtProperty *property,
                const QSizeF &min, const QSizeF &max, const QSizeF &val);

    struct Data
    {
        Data() : val(QSizeF(0, 0)), min(QSizeF(0, 0)), max(QSizeF(INT_MAX, INT_MAX)), decimals(2) {}
        QSizeF val;
        QSizeF min;
        QSizeF max;
        int decimals;
    };

    QMap<const QtProperty *, Data> m_values;

    QtDoublePropertyManager *m_doublePropertyManager;

    QMap<const QtProperty *, QtProperty *> m_propertyToW;
    QMap<const QtProperty *, QtProperty *> m_propertyToH;

    QMap<const QtProperty *, QtProperty *> m_wToProperty;
    QMap<const QtProperty *, QtProperty *> m_hToProperty;
};

void QtSizeFPropertyManagerPrivate::slotDoubleChanged(QtProperty *property, double value)
{
    if (m_wToProperty.contains(property)) {
        QtProperty *prop = m_wToProperty[property];
        QSizeF s = m_values[prop].val;
        s.setWidth(value);
        q_ptr->setValue(prop, s);
    } else if (m_hToProperty.contains(property)) {
        QtProperty *prop = m_hToProperty[property];
        QSizeF s = m_values[prop].val;
        s.setHeight(value);
        q_ptr->setValue(prop, s);
    }
}

void QtSizeFPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
    if (m_wToProperty.contains(property)) {
        QtProperty *pointProp = m_wToProperty[property];
        m_propertyToW[pointProp] = 0;
        m_wToProperty.remove(property);
    } else if (m_hToProperty.contains(property)) {
        QtProperty *pointProp = m_hToProperty[property];
        m_propertyToH[pointProp] = 0;
        m_hToProperty.remove(property);
    }
}

void QtSizeFPropertyManagerPrivate::setRange(QtProperty *property,
                const QSizeF &min, const QSizeF &max, const QSizeF &val)
{
    m_doublePropertyManager->setRange(m_propertyToW[property], min.width(), max.width());
    m_doublePropertyManager->setValue(m_propertyToW[property], val.width());
    m_doublePropertyManager->setRange(m_propertyToH[property], min.height(), max.height());
    m_doublePropertyManager->setValue(m_propertyToH[property], val.height());
}

/*!
    \class QtSizeFPropertyManager

    \brief The QtSizeFPropertyManager provides and manages QSizeF properties.

    A size property has nested \e width and \e height
    subproperties. The top-level property's value can be retrieved
    using the value() function, and set using the setValue() slot.

    The subproperties are created by a QtDoublePropertyManager object. This
    manager can be retrieved using the subDoublePropertyManager() function. In
    order to provide editing widgets for the subproperties in a
    property browser widget, this manager must be associated with an
    editor factory.

    A size property also has a range of valid values defined by a
    minimum size and a maximum size. These sizes can be retrieved
    using the minimum() and the maximum() functions, and set using the
    setMinimum() and setMaximum() slots. Alternatively, the range can
    be defined in one go using the setRange() slot.

    In addition, QtSizeFPropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes, and the rangeChanged() signal which is emitted whenever
    such a property changes its range of valid sizes.

    \sa QtAbstractPropertyManager, QtDoublePropertyManager, QtSizePropertyManager
*/

/*!
    \fn void QtSizeFPropertyManager::valueChanged(QtProperty *property, const QSizeF &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the new
    \a value as parameters.

    \sa setValue()
*/

/*!
    \fn void QtSizeFPropertyManager::rangeChanged(QtProperty *property, const QSizeF &minimum, const QSizeF &maximum)

    This signal is emitted whenever a property created by this manager
    changes its range of valid sizes, passing a pointer to the \a
    property and the new \a minimum and \a maximum sizes.

    \sa setRange()
*/

/*!
    \fn void QtSizeFPropertyManager::decimalsChanged(QtProperty *property, int prec)

    This signal is emitted whenever a property created by this manager
    changes its precision of value, passing a pointer to the
    \a property and the new \a prec value

    \sa setDecimals()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtSizeFPropertyManager::QtSizeFPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtSizeFPropertyManagerPrivate;
    d_ptr->q_ptr = this;

    d_ptr->m_doublePropertyManager = new QtDoublePropertyManager(this);
    connect(d_ptr->m_doublePropertyManager, SIGNAL(valueChanged(QtProperty *, double)),
                this, SLOT(slotDoubleChanged(QtProperty *, double)));
    connect(d_ptr->m_doublePropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtSizeFPropertyManager::~QtSizeFPropertyManager()
{
    delete d_ptr;
}

/*!
    Returns the manager that creates the nested \e width and \e height
    subproperties.

    In order to provide editing widgets for the \e width and \e height
    properties in a property browser widget, this manager must be
    associated with an editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtDoublePropertyManager *QtSizeFPropertyManager::subDoublePropertyManager() const
{
    return d_ptr->m_doublePropertyManager;
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by this manager, this
    function returns an invalid size

    \sa setValue()
*/
QSizeF QtSizeFPropertyManager::value(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QSizeF();
    return d_ptr->m_values[property].val;
}

/*!
    Returns the given \a property's precision, in decimals.

    \sa setDecimals()
*/
int QtSizeFPropertyManager::decimals(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return 0;
    return d_ptr->m_values[property].decimals;
}

/*!
    Returns the given \a property's minimum size value.

    \sa setMinimum(), maximum(), setRange()
*/
QSizeF QtSizeFPropertyManager::minimum(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QSizeF();
    return d_ptr->m_values[property].min;
}

/*!
    Returns the given \a property's maximum size value.

    \sa setMaximum(), minimum(), setRange()
*/
QSizeF QtSizeFPropertyManager::maximum(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QSizeF();
    return d_ptr->m_values[property].max;
}

/*!
    \reimp
*/
QString QtSizeFPropertyManager::valueText(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QString();
    QSizeF v = value(property);
    return QString(tr("%1 x %2").arg(QString::number(v.width(), 'f', decimals(property)))
                                .arg(QString::number(v.height(), 'f', decimals(property))));
}

/*!
    \fn void QtSizeFPropertyManager::setValue(QtProperty *property, const QSizeF &value)

    Sets the value of the given \a property to \a value.

    If the specified \a value is not valid according to the given \a
    property's size range, the \a value is adjusted to the nearest
    valid value within the size range.

    \sa value(), setRange(), valueChanged()
*/
void QtSizeFPropertyManager::setValue(QtProperty *property, const QSizeF &val)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtSizeFPropertyManagerPrivate::Data data = d_ptr->m_values[property];

    QSizeF newSize = val;
    if (data.min.width() > newSize.width())
        newSize.setWidth(data.min.width());
    else if (data.max.width() < newSize.width())
        newSize.setWidth(data.max.width());
    if (data.min.height() > newSize.height())
        newSize.setHeight(data.min.height());
    else if (data.max.height() < newSize.height())
        newSize.setHeight(data.max.height());

    if (data.val == newSize)
        return;

    data.val = newSize;

    d_ptr->m_values[property] = data;
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToW[property], newSize.width());
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToH[property], newSize.height());

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    \fn void QtSizeFPropertyManager::setDecimals(QtProperty *property, int prec)

    Sets the precision of the given \a property to \a prec.

    The valid decimal range is 0-13. The default is 2.

    \sa decimals()
*/
void QtSizeFPropertyManager::setDecimals(QtProperty *property, int prec)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtSizeFPropertyManagerPrivate::Data data = d_ptr->m_values[property];

    if (prec > 13)
        prec = 13;
    else if (prec < 0)
        prec = 0;

    if (data.decimals == prec)
        return;

    data.decimals = prec;
    d_ptr->m_doublePropertyManager->setDecimals(d_ptr->m_propertyToW[property], prec);
    d_ptr->m_doublePropertyManager->setDecimals(d_ptr->m_propertyToH[property], prec);

    d_ptr->m_values[property] = data;

    emit decimalsChanged(property, data.decimals);
}

/*!
    Sets the minimum size value for the given \a property to \a min.

    When setting the minimum size value, the maximum and current
    values are adjusted if necessary (ensuring that the size range
    remains valid and that the current value is within the range).

    \sa minimum(), setRange(), rangeChanged()
*/
void QtSizeFPropertyManager::setMinimum(QtProperty *property, const QSizeF &min)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtSizeFPropertyManagerPrivate::Data data = d_ptr->m_values[property];

    QSizeF oldVal = data.val;

    data.min = min;
    if (data.max.width() < data.min.width())
        data.max.setWidth(data.min.width());
    if (data.max.height() < data.min.height())
        data.max.setHeight(data.min.height());

    if (data.val.width() < data.min.width())
        data.val.setWidth(data.min.width());
    if (data.val.height() < data.min.height())
        data.val.setHeight(data.min.height());

    d_ptr->m_values[property] = data;

    emit rangeChanged(property, data.min, data.max);

    d_ptr->setRange(property, data.min, data.max, data.val);

    if (data.val == oldVal)
        return;

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    Sets the maximum size value for the given \a property to \a max.

    When setting the maximum size value, the minimum and current
    values are adjusted if necessary (ensuring that the size range
    remains valid and that the current value is within the range).

    \sa maximum(), setRange(), rangeChanged()
*/
void QtSizeFPropertyManager::setMaximum(QtProperty *property, const QSizeF &max)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtSizeFPropertyManagerPrivate::Data data = d_ptr->m_values[property];

    QSizeF oldVal = data.val;

    data.max = max;
    if (data.min.width() > data.max.width())
        data.min.setWidth(data.max.width());
    if (data.min.height() > data.max.height())
        data.min.setHeight(data.max.height());

    if (data.val.width() > data.max.width())
        data.val.setWidth(data.max.width());
    if (data.val.height() > data.max.height())
        data.val.setHeight(data.max.height());

    d_ptr->m_values[property] = data;

    emit rangeChanged(property, data.min, data.max);

    d_ptr->setRange(property, data.min, data.max, data.val);

    if (data.val == oldVal)
        return;

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    \fn void QtSizeFPropertyManager::setRange(QtProperty *property, const QSizeF &minimum, const QSizeF &maximum)

    Sets the range of valid values.

    This is a convenience function defining the range of valid values
    in one go; setting the \a minimum and \a maximum values for the
    given \a property with a single function call.

    When setting a new range, the current value is adjusted if
    necessary (ensuring that the value remains within the range).

    \sa  setMinimum(), setMaximum(), rangeChanged()
*/
void QtSizeFPropertyManager::setRange(QtProperty *property, const QSizeF &min, const QSizeF &max)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QSizeF fromSize = min;
    QSizeF toSize = max;
    if (fromSize.width() > toSize.width()) {
        fromSize.setWidth(max.width());
        toSize.setWidth(min.width());
    }
    if (fromSize.height() > toSize.height()) {
        fromSize.setHeight(max.height());
        toSize.setHeight(min.height());
    }

    QtSizeFPropertyManagerPrivate::Data data = d_ptr->m_values[property];

    if (data.min == fromSize && data.max == toSize)
        return;

    QSizeF oldVal = data.val;

    data.min = min;
    data.max = max;

    if (data.val.width() < data.min.width())
        data.val.setWidth(data.min.width());
    else if (data.val.width() > data.max.width())
        data.val.setWidth(data.max.width());
    if (data.val.height() < data.min.height())
        data.val.setHeight(data.min.height());
    else if (data.val.height() > data.max.height())
        data.val.setHeight(data.max.height());

    d_ptr->m_values[property] = data;

    emit rangeChanged(property, data.min, data.max);

    d_ptr->setRange(property, data.min, data.max, data.val);

    if (data.val == oldVal)
        return;

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    \reimp
*/
void QtSizeFPropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QtSizeFPropertyManagerPrivate::Data();

    QtProperty *wProp = d_ptr->m_doublePropertyManager->addProperty();
    wProp->setPropertyName(tr("Width"));
    d_ptr->m_doublePropertyManager->setDecimals(wProp, decimals(property));
    d_ptr->m_doublePropertyManager->setValue(wProp, 0);
    d_ptr->m_doublePropertyManager->setMinimum(wProp, 0);
    d_ptr->m_propertyToW[property] = wProp;
    d_ptr->m_wToProperty[wProp] = property;
    property->addSubProperty(wProp);

    QtProperty *hProp = d_ptr->m_doublePropertyManager->addProperty();
    hProp->setPropertyName(tr("Height"));
    d_ptr->m_doublePropertyManager->setDecimals(hProp, decimals(property));
    d_ptr->m_doublePropertyManager->setValue(hProp, 0);
    d_ptr->m_doublePropertyManager->setMinimum(hProp, 0);
    d_ptr->m_propertyToH[property] = hProp;
    d_ptr->m_hToProperty[hProp] = property;
    property->addSubProperty(hProp);
}

/*!
    \reimp
*/
void QtSizeFPropertyManager::uninitializeProperty(QtProperty *property)
{
    QtProperty *wProp = d_ptr->m_propertyToW[property];
    if (wProp) {
        d_ptr->m_wToProperty.remove(wProp);
        delete wProp;
    }
    d_ptr->m_propertyToW.remove(property);

    QtProperty *hProp = d_ptr->m_propertyToH[property];
    if (hProp) {
        d_ptr->m_hToProperty.remove(hProp);
        delete hProp;
    }
    d_ptr->m_propertyToH.remove(property);

    d_ptr->m_values.remove(property);
}

// QtRectPropertyManager

class QtRectPropertyManagerPrivate
{
    QtRectPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtRectPropertyManager)
public:

    void slotIntChanged(QtProperty *property, int value);
    void slotPropertyDestroyed(QtProperty *property);
    void setConstraint(QtProperty *property, const QRect &constraint, const QRect &val);

    struct Data
    {
        Data() : val(0, 0, 0, 0), constraint(QRect(0, 0, INT_MAX, INT_MAX)) {}
        QRect val;
        QRect constraint;
    };

    QMap<const QtProperty *, Data> m_values;

    QtIntPropertyManager *m_intPropertyManager;

    QMap<const QtProperty *, QtProperty *> m_propertyToX;
    QMap<const QtProperty *, QtProperty *> m_propertyToY;
    QMap<const QtProperty *, QtProperty *> m_propertyToW;
    QMap<const QtProperty *, QtProperty *> m_propertyToH;

    QMap<const QtProperty *, QtProperty *> m_xToProperty;
    QMap<const QtProperty *, QtProperty *> m_yToProperty;
    QMap<const QtProperty *, QtProperty *> m_wToProperty;
    QMap<const QtProperty *, QtProperty *> m_hToProperty;
};

void QtRectPropertyManagerPrivate::slotIntChanged(QtProperty *property, int value)
{
    if (m_xToProperty.contains(property)) {
        QtProperty *prop = m_xToProperty[property];
        QRect r = m_values[prop].val;
        r.moveLeft(value);
        q_ptr->setValue(prop, r);
    } else if (m_yToProperty.contains(property)) {
        QtProperty *prop = m_yToProperty[property];
        QRect r = m_values[prop].val;
        r.moveTop(value);
        q_ptr->setValue(prop, r);
    } else if (m_wToProperty.contains(property)) {
        QtProperty *prop = m_wToProperty[property];
        Data data = m_values[prop];
        QRect r = data.val;
        r.setWidth(value);
        if (data.constraint.x() + data.constraint.width() < r.x() + r.width()) {
            r.moveLeft(data.constraint.left() + data.constraint.width() - r.width());
        }
        q_ptr->setValue(prop, r);
    } else if (m_hToProperty.contains(property)) {
        QtProperty *prop = m_hToProperty[property];
        Data data = m_values[prop];
        QRect r = data.val;
        r.setHeight(value);
        if (data.constraint.y() + data.constraint.height() < r.y() + r.height()) {
            r.moveTop(data.constraint.top() + data.constraint.height() - r.height());
        }
        q_ptr->setValue(prop, r);
    }
}

void QtRectPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
    if (m_xToProperty.contains(property)) {
        QtProperty *pointProp = m_xToProperty[property];
        m_propertyToX[pointProp] = 0;
        m_xToProperty.remove(property);
    } else if (m_yToProperty.contains(property)) {
        QtProperty *pointProp = m_yToProperty[property];
        m_propertyToY[pointProp] = 0;
        m_yToProperty.remove(property);
    } else if (m_wToProperty.contains(property)) {
        QtProperty *pointProp = m_wToProperty[property];
        m_propertyToW[pointProp] = 0;
        m_wToProperty.remove(property);
    } else if (m_hToProperty.contains(property)) {
        QtProperty *pointProp = m_hToProperty[property];
        m_propertyToH[pointProp] = 0;
        m_hToProperty.remove(property);
    }
}

void QtRectPropertyManagerPrivate::setConstraint(QtProperty *property,
            const QRect &constraint, const QRect &val)
{
    m_intPropertyManager->setRange(m_propertyToX[property], constraint.left(),
                    constraint.left() + constraint.width());
    m_intPropertyManager->setRange(m_propertyToY[property], constraint.top(),
                    constraint.top() + constraint.height());
    m_intPropertyManager->setRange(m_propertyToW[property], 0, constraint.width());
    m_intPropertyManager->setRange(m_propertyToH[property], 0, constraint.height());

    m_intPropertyManager->setValue(m_propertyToX[property], val.x());
    m_intPropertyManager->setValue(m_propertyToY[property], val.y());
    m_intPropertyManager->setValue(m_propertyToW[property], val.width());
    m_intPropertyManager->setValue(m_propertyToH[property], val.height());
}

/*!
    \class QtRectPropertyManager

    \brief The QtRectPropertyManager provides and manages QRect properties.

    A rectangle property has nested \e x, \e y, \e width and \e height
    subproperties. The top-level property's value can be retrieved
    using the value() function, and set using the setValue() slot.

    The subproperties are created by a QtIntPropertyManager object. This
    manager can be retrieved using the subIntPropertyManager() function. In
    order to provide editing widgets for the subproperties in a
    property browser widget, this manager must be associated with an
    editor factory.

    A rectangle property also has a constraint rectangle which can be
    retrieved using the constraint() function, and set using the
    setConstraint() slot.

    In addition, QtRectPropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes, and the constraintChanged() signal which is emitted
    whenever such a property changes its constraint rectangle.

    \sa QtAbstractPropertyManager, QtIntPropertyManager, QtRectFPropertyManager
*/

/*!
    \fn void QtRectPropertyManager::valueChanged(QtProperty *property, const QRect &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the new
    \a value as parameters.

    \sa setValue()
*/

/*!
    \fn void QtRectPropertyManager::constraintChanged(QtProperty *property, const QRect &constraint)

    This signal is emitted whenever property changes its constraint
    rectangle, passing a pointer to the \a property and the new \a
    constraint rectangle as parameters.

    \sa setConstraint()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtRectPropertyManager::QtRectPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtRectPropertyManagerPrivate;
    d_ptr->q_ptr = this;

    d_ptr->m_intPropertyManager = new QtIntPropertyManager(this);
    connect(d_ptr->m_intPropertyManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotIntChanged(QtProperty *, int)));
    connect(d_ptr->m_intPropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtRectPropertyManager::~QtRectPropertyManager()
{
    delete d_ptr;
}

/*!
    Returns the manager that creates the nested \e x, \e y, \e width
    and \e height subproperties.

    In order to provide editing widgets for the mentioned
    subproperties in a property browser widget, this manager must be
    associated with an editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtIntPropertyManager *QtRectPropertyManager::subIntPropertyManager() const
{
    return d_ptr->m_intPropertyManager;
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by this manager, this
    function returns an invalid rectangle.

    \sa setValue(), constraint()
*/
QRect QtRectPropertyManager::value(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QRect();
    return d_ptr->m_values[property].val;
}

/*!
    Returns the given \a property's constraining rectangle.

    \sa value(), setConstraint()
*/
QRect QtRectPropertyManager::constraint(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QRect();
    return d_ptr->m_values[property].constraint;
}

/*!
    \reimp
*/
QString QtRectPropertyManager::valueText(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QString();
    QRect v = value(property);
    return QString(tr("[(%1, %2), %3 x %4]").arg(QString::number(v.x()))
                                .arg(QString::number(v.y()))
                                .arg(QString::number(v.width()))
                                .arg(QString::number(v.height())));
}

/*!
    \fn void QtRectPropertyManager::setValue(QtProperty *property, const QRect &value)

    Sets the value of the given \a property to \a value. Nested
    properties are updated automatically.

    If the specified \a value is not inside the given \a property's
    constraining rectangle, the value is adjusted accordingly to fit
    within the constraint.

    \sa value(), setConstraint(), valueChanged()
*/
void QtRectPropertyManager::setValue(QtProperty *property, const QRect &val)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtRectPropertyManagerPrivate::Data data = d_ptr->m_values[property];

    QRect newRect = val.normalized();
    if (!data.constraint.contains(newRect)) {
        QRect r1 = data.constraint;
        QRect r2 = newRect;
        newRect.setLeft(qMax(r1.left(), r2.left()));
        newRect.setRight(qMin(r1.right(), r2.right()));
        newRect.setTop(qMax(r1.top(), r2.top()));
        newRect.setBottom(qMin(r1.bottom(), r2.bottom()));
        if (newRect.width() < 0 || newRect.height() < 0)
            return;
    }

    if (data.val == newRect)
        return;

    data.val = newRect;

    d_ptr->m_values[property] = data;
    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToX[property], newRect.x());
    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToY[property], newRect.y());
    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToW[property], newRect.width());
    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToH[property], newRect.height());

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    Sets the given \a property's constraining rectangle to \a
    constraint.

    When setting the constraint, the current value is adjusted if
    necessary (ensuring that the current rectangle value is inside the
    constraint).

    \sa setValue(), constraint(), constraintChanged()
*/
void QtRectPropertyManager::setConstraint(QtProperty *property, const QRect &constraint)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtRectPropertyManagerPrivate::Data data = d_ptr->m_values[property];

    QRect newConstraint = constraint.normalized();
    if (data.constraint == newConstraint)
        return;

    QRect oldVal = data.val;

    data.constraint = newConstraint;

    if (!data.constraint.contains(oldVal)) {
        QRect r1 = data.constraint;
        QRect r2 = data.val;
        if (r2.width() > r1.width())
            r2.setWidth(r1.width());
        if (r2.height() > r1.height())
            r2.setHeight(r1.height());
        if (r2.left() < r1.left())
            r2.moveLeft(r1.left());
        else if (r2.right() > r1.right())
            r2.moveRight(r1.right());
        if (r2.top() < r1.top())
            r2.moveTop(r1.top());
        else if (r2.bottom() > r1.bottom())
            r2.moveBottom(r1.bottom());

        data.val = r2;
    }

    d_ptr->m_values[property] = data;

    emit constraintChanged(property, data.constraint);

    d_ptr->setConstraint(property, data.constraint, data.val);

    if (data.val == oldVal)
        return;

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    \reimp
*/
void QtRectPropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QtRectPropertyManagerPrivate::Data();

    QtProperty *xProp = d_ptr->m_intPropertyManager->addProperty();
    xProp->setPropertyName(tr("X"));
    d_ptr->m_intPropertyManager->setValue(xProp, 0);
    d_ptr->m_intPropertyManager->setMinimum(xProp, 0);
    d_ptr->m_propertyToX[property] = xProp;
    d_ptr->m_xToProperty[xProp] = property;
    property->addSubProperty(xProp);

    QtProperty *yProp = d_ptr->m_intPropertyManager->addProperty();
    yProp->setPropertyName(tr("Y"));
    d_ptr->m_intPropertyManager->setValue(yProp, 0);
    d_ptr->m_intPropertyManager->setMinimum(yProp, 0);
    d_ptr->m_propertyToY[property] = yProp;
    d_ptr->m_yToProperty[yProp] = property;
    property->addSubProperty(yProp);

    QtProperty *wProp = d_ptr->m_intPropertyManager->addProperty();
    wProp->setPropertyName(tr("Width"));
    d_ptr->m_intPropertyManager->setValue(wProp, 0);
    d_ptr->m_intPropertyManager->setMinimum(wProp, 0);
    d_ptr->m_propertyToW[property] = wProp;
    d_ptr->m_wToProperty[wProp] = property;
    property->addSubProperty(wProp);

    QtProperty *hProp = d_ptr->m_intPropertyManager->addProperty();
    hProp->setPropertyName(tr("Height"));
    d_ptr->m_intPropertyManager->setValue(hProp, 0);
    d_ptr->m_intPropertyManager->setMinimum(hProp, 0);
    d_ptr->m_propertyToH[property] = hProp;
    d_ptr->m_hToProperty[hProp] = property;
    property->addSubProperty(hProp);
}

/*!
    \reimp
*/
void QtRectPropertyManager::uninitializeProperty(QtProperty *property)
{
    QtProperty *xProp = d_ptr->m_propertyToX[property];
    if (xProp) {
        d_ptr->m_xToProperty.remove(xProp);
        delete xProp;
    }
    d_ptr->m_propertyToX.remove(property);

    QtProperty *yProp = d_ptr->m_propertyToY[property];
    if (yProp) {
        d_ptr->m_yToProperty.remove(yProp);
        delete yProp;
    }
    d_ptr->m_propertyToY.remove(property);

    QtProperty *wProp = d_ptr->m_propertyToW[property];
    if (wProp) {
        d_ptr->m_wToProperty.remove(wProp);
        delete wProp;
    }
    d_ptr->m_propertyToW.remove(property);

    QtProperty *hProp = d_ptr->m_propertyToH[property];
    if (hProp) {
        d_ptr->m_hToProperty.remove(hProp);
        delete hProp;
    }
    d_ptr->m_propertyToH.remove(property);

    d_ptr->m_values.remove(property);
}

// QtRectFPropertyManager

class QtRectFPropertyManagerPrivate
{
    QtRectFPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtRectFPropertyManager)
public:

    void slotDoubleChanged(QtProperty *property, double value);
    void slotPropertyDestroyed(QtProperty *property);
    void setConstraint(QtProperty *property, const QRectF &constraint, const QRectF &val);

    struct Data
    {
        Data() : val(0, 0, 0, 0), constraint(QRectF(0, 0, INT_MAX, INT_MAX)), decimals(2) {}
        QRectF val;
        QRectF constraint;
        int decimals;
    };

    QMap<const QtProperty *, Data> m_values;

    QtDoublePropertyManager *m_doublePropertyManager;

    QMap<const QtProperty *, QtProperty *> m_propertyToX;
    QMap<const QtProperty *, QtProperty *> m_propertyToY;
    QMap<const QtProperty *, QtProperty *> m_propertyToW;
    QMap<const QtProperty *, QtProperty *> m_propertyToH;

    QMap<const QtProperty *, QtProperty *> m_xToProperty;
    QMap<const QtProperty *, QtProperty *> m_yToProperty;
    QMap<const QtProperty *, QtProperty *> m_wToProperty;
    QMap<const QtProperty *, QtProperty *> m_hToProperty;
};

void QtRectFPropertyManagerPrivate::slotDoubleChanged(QtProperty *property, double value)
{
    if (m_xToProperty.contains(property)) {
        QtProperty *prop = m_xToProperty[property];
        QRectF r = m_values[prop].val;
        r.moveLeft(value);
        q_ptr->setValue(prop, r);
    } else if (m_yToProperty.contains(property)) {
        QtProperty *prop = m_yToProperty[property];
        QRectF r = m_values[prop].val;
        r.moveTop(value);
        q_ptr->setValue(prop, r);
    } else if (m_wToProperty.contains(property)) {
        QtProperty *prop = m_wToProperty[property];
        Data data = m_values[prop];
        QRectF r = data.val;
        r.setWidth(value);
        if (data.constraint.x() + data.constraint.width() < r.x() + r.width()) {
            r.moveLeft(data.constraint.left() + data.constraint.width() - r.width());
        }
        q_ptr->setValue(prop, r);
    } else if (m_hToProperty.contains(property)) {
        QtProperty *prop = m_hToProperty[property];
        Data data = m_values[prop];
        QRectF r = data.val;
        r.setHeight(value);
        if (data.constraint.y() + data.constraint.height() < r.y() + r.height()) {
            r.moveTop(data.constraint.top() + data.constraint.height() - r.height());
        }
        q_ptr->setValue(prop, r);
    }
}

void QtRectFPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
    if (m_xToProperty.contains(property)) {
        QtProperty *pointProp = m_xToProperty[property];
        m_propertyToX[pointProp] = 0;
        m_xToProperty.remove(property);
    } else if (m_yToProperty.contains(property)) {
        QtProperty *pointProp = m_yToProperty[property];
        m_propertyToY[pointProp] = 0;
        m_yToProperty.remove(property);
    } else if (m_wToProperty.contains(property)) {
        QtProperty *pointProp = m_wToProperty[property];
        m_propertyToW[pointProp] = 0;
        m_wToProperty.remove(property);
    } else if (m_hToProperty.contains(property)) {
        QtProperty *pointProp = m_hToProperty[property];
        m_propertyToH[pointProp] = 0;
        m_hToProperty.remove(property);
    }
}

void QtRectFPropertyManagerPrivate::setConstraint(QtProperty *property,
            const QRectF &constraint, const QRectF &val)
{
    m_doublePropertyManager->setRange(m_propertyToX[property], constraint.left(),
                    constraint.left() + constraint.width());
    m_doublePropertyManager->setRange(m_propertyToY[property], constraint.top(),
                    constraint.top() + constraint.height());
    m_doublePropertyManager->setRange(m_propertyToW[property], 0, constraint.width());
    m_doublePropertyManager->setRange(m_propertyToH[property], 0, constraint.height());

    m_doublePropertyManager->setValue(m_propertyToX[property], val.x());
    m_doublePropertyManager->setValue(m_propertyToY[property], val.y());
    m_doublePropertyManager->setValue(m_propertyToW[property], val.width());
    m_doublePropertyManager->setValue(m_propertyToH[property], val.height());
}

/*!
    \class QtRectFPropertyManager

    \brief The QtRectFPropertyManager provides and manages QRectF properties.

    A rectangle property has nested \e x, \e y, \e width and \e height
    subproperties. The top-level property's value can be retrieved
    using the value() function, and set using the setValue() slot.

    The subproperties are created by a QtDoublePropertyManager object. This
    manager can be retrieved using the subDoublePropertyManager() function. In
    order to provide editing widgets for the subproperties in a
    property browser widget, this manager must be associated with an
    editor factory.

    A rectangle property also has a constraint rectangle which can be
    retrieved using the constraint() function, and set using the
    setConstraint() slot.

    In addition, QtRectFPropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes, and the constraintChanged() signal which is emitted
    whenever such a property changes its constraint rectangle.

    \sa QtAbstractPropertyManager, QtDoublePropertyManager, QtRectPropertyManager
*/

/*!
    \fn void QtRectFPropertyManager::valueChanged(QtProperty *property, const QRectF &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the new
    \a value as parameters.

    \sa setValue()
*/

/*!
    \fn void QtRectFPropertyManager::constraintChanged(QtProperty *property, const QRectF &constraint)

    This signal is emitted whenever property changes its constraint
    rectangle, passing a pointer to the \a property and the new \a
    constraint rectangle as parameters.

    \sa setConstraint()
*/

/*!
    \fn void QtRectFPropertyManager::decimalsChanged(QtProperty *property, int prec)

    This signal is emitted whenever a property created by this manager
    changes its precision of value, passing a pointer to the
    \a property and the new \a prec value

    \sa setDecimals()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtRectFPropertyManager::QtRectFPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtRectFPropertyManagerPrivate;
    d_ptr->q_ptr = this;

    d_ptr->m_doublePropertyManager = new QtDoublePropertyManager(this);
    connect(d_ptr->m_doublePropertyManager, SIGNAL(valueChanged(QtProperty *, double)),
                this, SLOT(slotDoubleChanged(QtProperty *, double)));
    connect(d_ptr->m_doublePropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtRectFPropertyManager::~QtRectFPropertyManager()
{
    delete d_ptr;
}

/*!
    Returns the manager that creates the nested \e x, \e y, \e width
    and \e height subproperties.

    In order to provide editing widgets for the mentioned
    subproperties in a property browser widget, this manager must be
    associated with an editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtDoublePropertyManager *QtRectFPropertyManager::subDoublePropertyManager() const
{
    return d_ptr->m_doublePropertyManager;
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by this manager, this
    function returns an invalid rectangle.

    \sa setValue(), constraint()
*/
QRectF QtRectFPropertyManager::value(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QRectF();
    return d_ptr->m_values[property].val;
}

/*!
    Returns the given \a property's precision, in decimals.

    \sa setDecimals()
*/
int QtRectFPropertyManager::decimals(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return 0;
    return d_ptr->m_values[property].decimals;
}

/*!
    Returns the given \a property's constraining rectangle.

    \sa value(), setConstraint()
*/
QRectF QtRectFPropertyManager::constraint(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QRectF();
    return d_ptr->m_values[property].constraint;
}

/*!
    \reimp
*/
QString QtRectFPropertyManager::valueText(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QString();
    QRectF v = value(property);
    return QString(tr("[(%1, %2), %3 x %4]").arg(QString::number(v.x(), 'f', decimals(property)))
                                .arg(QString::number(v.y(), 'f', decimals(property)))
                                .arg(QString::number(v.width(), 'f', decimals(property)))
                                .arg(QString::number(v.height(), 'f', decimals(property))));
}

/*!
    \fn void QtRectFPropertyManager::setValue(QtProperty *property, const QRectF &value)

    Sets the value of the given \a property to \a value. Nested
    properties are updated automatically.

    If the specified \a value is not inside the given \a property's
    constraining rectangle, the value is adjusted accordingly to fit
    within the constraint.

    \sa value(), setConstraint(), valueChanged()
*/
void QtRectFPropertyManager::setValue(QtProperty *property, const QRectF &val)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtRectFPropertyManagerPrivate::Data data = d_ptr->m_values[property];

    QRectF newRect = val.normalized();
    if (!data.constraint.contains(newRect)) {
        QRectF r1 = data.constraint;
        QRectF r2 = newRect;
        newRect.setLeft(qMax(r1.left(), r2.left()));
        newRect.setRight(qMin(r1.right(), r2.right()));
        newRect.setTop(qMax(r1.top(), r2.top()));
        newRect.setBottom(qMin(r1.bottom(), r2.bottom()));
        if (newRect.width() < 0 || newRect.height() < 0)
            return;
    }

    if (data.val == newRect)
        return;

    data.val = newRect;

    d_ptr->m_values[property] = data;
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToX[property], newRect.x());
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToY[property], newRect.y());
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToW[property], newRect.width());
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToH[property], newRect.height());

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    Sets the given \a property's constraining rectangle to \a
    constraint.

    When setting the constraint, the current value is adjusted if
    necessary (ensuring that the current rectangle value is inside the
    constraint).

    \sa setValue(), constraint(), constraintChanged()
*/
void QtRectFPropertyManager::setConstraint(QtProperty *property, const QRectF &constraint)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtRectFPropertyManagerPrivate::Data data = d_ptr->m_values[property];

    QRectF newConstraint = constraint.normalized();
    if (data.constraint == newConstraint)
        return;

    QRectF oldVal = data.val;

    data.constraint = newConstraint;

    if (!data.constraint.contains(oldVal)) {
        QRectF r1 = data.constraint;
        QRectF r2 = data.val;
        if (r2.width() > r1.width())
            r2.setWidth(r1.width());
        if (r2.height() > r1.height())
            r2.setHeight(r1.height());
        if (r2.left() < r1.left())
            r2.moveLeft(r1.left());
        else if (r2.right() > r1.right())
            r2.moveRight(r1.right());
        if (r2.top() < r1.top())
            r2.moveTop(r1.top());
        else if (r2.bottom() > r1.bottom())
            r2.moveBottom(r1.bottom());

        data.val = r2;
    }

    d_ptr->m_values[property] = data;

    emit constraintChanged(property, data.constraint);

    d_ptr->setConstraint(property, data.constraint, data.val);

    if (data.val == oldVal)
        return;

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    \fn void QtRectFPropertyManager::setDecimals(QtProperty *property, int prec)

    Sets the precision of the given \a property to \a prec.

    The valid decimal range is 0-13. The default is 2.

    \sa decimals()
*/
void QtRectFPropertyManager::setDecimals(QtProperty *property, int prec)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtRectFPropertyManagerPrivate::Data data = d_ptr->m_values[property];

    if (prec > 13)
        prec = 13;
    else if (prec < 0)
        prec = 0;

    if (data.decimals == prec)
        return;

    data.decimals = prec;
    d_ptr->m_doublePropertyManager->setDecimals(d_ptr->m_propertyToX[property], prec);
    d_ptr->m_doublePropertyManager->setDecimals(d_ptr->m_propertyToY[property], prec);
    d_ptr->m_doublePropertyManager->setDecimals(d_ptr->m_propertyToW[property], prec);
    d_ptr->m_doublePropertyManager->setDecimals(d_ptr->m_propertyToH[property], prec);

    d_ptr->m_values[property] = data;

    emit decimalsChanged(property, data.decimals);
}

/*!
    \reimp
*/
void QtRectFPropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QtRectFPropertyManagerPrivate::Data();

    QtProperty *xProp = d_ptr->m_doublePropertyManager->addProperty();
    xProp->setPropertyName(tr("X"));
    d_ptr->m_doublePropertyManager->setDecimals(xProp, decimals(property));
    d_ptr->m_doublePropertyManager->setValue(xProp, 0);
    d_ptr->m_doublePropertyManager->setMinimum(xProp, 0);
    d_ptr->m_propertyToX[property] = xProp;
    d_ptr->m_xToProperty[xProp] = property;
    property->addSubProperty(xProp);

    QtProperty *yProp = d_ptr->m_doublePropertyManager->addProperty();
    yProp->setPropertyName(tr("Y"));
    d_ptr->m_doublePropertyManager->setDecimals(yProp, decimals(property));
    d_ptr->m_doublePropertyManager->setValue(yProp, 0);
    d_ptr->m_doublePropertyManager->setMinimum(yProp, 0);
    d_ptr->m_propertyToY[property] = yProp;
    d_ptr->m_yToProperty[yProp] = property;
    property->addSubProperty(yProp);

    QtProperty *wProp = d_ptr->m_doublePropertyManager->addProperty();
    wProp->setPropertyName(tr("Width"));
    d_ptr->m_doublePropertyManager->setDecimals(wProp, decimals(property));
    d_ptr->m_doublePropertyManager->setValue(wProp, 0);
    d_ptr->m_doublePropertyManager->setMinimum(wProp, 0);
    d_ptr->m_propertyToW[property] = wProp;
    d_ptr->m_wToProperty[wProp] = property;
    property->addSubProperty(wProp);

    QtProperty *hProp = d_ptr->m_doublePropertyManager->addProperty();
    hProp->setPropertyName(tr("Height"));
    d_ptr->m_doublePropertyManager->setDecimals(hProp, decimals(property));
    d_ptr->m_doublePropertyManager->setValue(hProp, 0);
    d_ptr->m_doublePropertyManager->setMinimum(hProp, 0);
    d_ptr->m_propertyToH[property] = hProp;
    d_ptr->m_hToProperty[hProp] = property;
    property->addSubProperty(hProp);
}

/*!
    \reimp
*/
void QtRectFPropertyManager::uninitializeProperty(QtProperty *property)
{
    QtProperty *xProp = d_ptr->m_propertyToX[property];
    if (xProp) {
        d_ptr->m_xToProperty.remove(xProp);
        delete xProp;
    }
    d_ptr->m_propertyToX.remove(property);

    QtProperty *yProp = d_ptr->m_propertyToY[property];
    if (yProp) {
        d_ptr->m_yToProperty.remove(yProp);
        delete yProp;
    }
    d_ptr->m_propertyToY.remove(property);

    QtProperty *wProp = d_ptr->m_propertyToW[property];
    if (wProp) {
        d_ptr->m_wToProperty.remove(wProp);
        delete wProp;
    }
    d_ptr->m_propertyToW.remove(property);

    QtProperty *hProp = d_ptr->m_propertyToH[property];
    if (hProp) {
        d_ptr->m_hToProperty.remove(hProp);
        delete hProp;
    }
    d_ptr->m_propertyToH.remove(property);

    d_ptr->m_values.remove(property);
}

// QtEnumPropertyManager

class QtEnumPropertyManagerPrivate
{
    QtEnumPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtEnumPropertyManager)
public:

    struct Data
    {
        Data() : val(-1) {}
        int val;
        QStringList enumNames;
        QMap<int, QIcon> enumIcons;
    };

    QMap<const QtProperty *, Data> m_values;
};

/*!
    \class QtEnumPropertyManager

    \brief The QtEnumPropertyManager provides and manages enum properties.

    Each enum property has an associated list of enum names which can
    be retrieved using the enumNames() function, and set using the
    corresponding setEnumNames() function. An enum property's value is
    represented by an index in this list, and can be retrieved and set
    using the value() and setValue() slots respectively.

    In addition, QtEnumPropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes, and the enumNamesChanged() signal which is emitted
    whenever the list of enum names is altered.

    \sa QtAbstractPropertyManager, QtEnumEditorFactory
*/

/*!
    \fn void QtEnumPropertyManager::valueChanged(QtProperty *property, int value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the new
    \a value as parameters.

    \sa setValue()
*/

/*!
    \fn void QtEnumPropertyManager::enumNamesChanged(QtProperty *property, const QStringList &names)

    This signal is emitted whenever a property created by this manager
    changes its enum names, passing a pointer to the \a property and
    the new \a names as parameters.

    \sa setEnumNames()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtEnumPropertyManager::QtEnumPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtEnumPropertyManagerPrivate;
    d_ptr->q_ptr = this;
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtEnumPropertyManager::~QtEnumPropertyManager()
{
    delete d_ptr;
}

/*!
    Returns the given \a property's value which is an index in the
    list returned by enumNames()

    If the given property is not managed by this manager, this
    function returns -1.

    \sa enumNames(), setValue()
*/
int QtEnumPropertyManager::value(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return -1;
    return d_ptr->m_values[property].val;
}

/*!
    Returns the given \a property's list of enum names.

    \sa value(), setEnumNames()
*/
QStringList QtEnumPropertyManager::enumNames(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QStringList();
    return d_ptr->m_values[property].enumNames;
}

/*!
    Returns the given \a property's map of enum values to their icons.

    \sa value(), setEnumIcons()
*/
QMap<int, QIcon> QtEnumPropertyManager::enumIcons(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QMap<int, QIcon>();
    return d_ptr->m_values[property].enumIcons;
}

/*!
    \reimp
*/
QString QtEnumPropertyManager::valueText(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QString();

    QtEnumPropertyManagerPrivate::Data data = d_ptr->m_values[property];

    int v = data.val;
    if (v >= 0 && v < data.enumNames.count())
        return data.enumNames.at(v);
    return QString();
}

/*!
    \reimp
*/
QIcon QtEnumPropertyManager::valueIcon(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QIcon();

    QtEnumPropertyManagerPrivate::Data data = d_ptr->m_values[property];

    int v = data.val;
    return data.enumIcons.value(v);
}

/*!
    \fn void QtEnumPropertyManager::setValue(QtProperty *property, int value)

    Sets the value of the given  \a property to \a value.

    The specified \a value must be less than the size of the given \a
    property's enumNames() list, and larger than (or equal to) 0.

    \sa value(), valueChanged()
*/
void QtEnumPropertyManager::setValue(QtProperty *property, int val)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtEnumPropertyManagerPrivate::Data data = d_ptr->m_values[property];

    if (val >= data.enumNames.count())
        return;

    if (val < 0 && data.enumNames.count() > 0)
        return;

    if (val < 0)
        val = -1;

    if (data.val == val)
        return;

    data.val = val;

    d_ptr->m_values[property] = data;

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    Sets the given \a property's list of enum names to \a
    enumNames. The \a property's current value is reset to 0
    indicating the first item of the list.

    If the specified \a enumNames list is empty, the \a property's
    current value is set to -1.

    \sa enumNames(), enumNamesChanged()
*/
void QtEnumPropertyManager::setEnumNames(QtProperty *property, const QStringList &enumNames)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtEnumPropertyManagerPrivate::Data data = d_ptr->m_values[property];

    if (data.enumNames == enumNames)
        return;

    data.enumNames = enumNames;

    data.val = -1;

    if (enumNames.count() > 0)
        data.val = 0;

    d_ptr->m_values[property] = data;

    emit enumNamesChanged(property, data.enumNames);

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    Sets the given \a property's map of enum values to their icons to \a
    enumIcons.

    Each enum value can have associated icon. This association is represented with passed \a enumIcons map.

    \sa enumNames(), enumNamesChanged()
*/
void QtEnumPropertyManager::setEnumIcons(QtProperty *property, const QMap<int, QIcon> &enumIcons)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtEnumPropertyManagerPrivate::Data data = d_ptr->m_values[property];

    //if (data.enumIcons == enumIcons)
    //    return;

    data.enumIcons = enumIcons;

    d_ptr->m_values[property] = data;

    emit enumIconsChanged(property, data.enumIcons);

    emit propertyChanged(property);
}

/*!
    \reimp
*/
void QtEnumPropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QtEnumPropertyManagerPrivate::Data();
}

/*!
    \reimp
*/
void QtEnumPropertyManager::uninitializeProperty(QtProperty *property)
{
    d_ptr->m_values.remove(property);
}

// QtFlagPropertyManager

class QtFlagPropertyManagerPrivate
{
    QtFlagPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtFlagPropertyManager)
public:

    void slotBoolChanged(QtProperty *property, bool value);
    void slotPropertyDestroyed(QtProperty *property);

    struct Data
    {
        Data() : val(-1) {}
        int val;
        QStringList flagNames;
    };

    QMap<const QtProperty *, Data> m_values;

    QtBoolPropertyManager *m_boolPropertyManager;

    QMap<const QtProperty *, QList<QtProperty *> > m_propertyToFlags;

    QMap<const QtProperty *, QtProperty *> m_flagToProperty;
};

void QtFlagPropertyManagerPrivate::slotBoolChanged(QtProperty *property, bool value)
{
    if (!m_flagToProperty.contains(property))
        return;

    QtProperty *prop = m_flagToProperty[property];
    QListIterator<QtProperty *> itProp(m_propertyToFlags[prop]);
    int level = 0;
    while (itProp.hasNext()) {
        QtProperty *p = itProp.next();
        if (p == property) {
            int v = m_values[prop].val;
            if (value) {
                v |= (1 << level);
            } else {
                v &= ~(1 << level);
            }
            q_ptr->setValue(prop, v);
            return;
        }
        level++;
    }
}

void QtFlagPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
    if (!m_flagToProperty.contains(property))
        return;

    QtProperty *flagProperty = m_flagToProperty[property];
    m_propertyToFlags[flagProperty].replace(m_propertyToFlags[flagProperty].indexOf(property), 0);
    m_flagToProperty.remove(property);
}

/*!
    \class QtFlagPropertyManager

    \brief The QtFlagPropertyManager provides and manages flag properties.

    Each flag property has an associated list of flag names which can
    be retrieved using the flagNames() function, and set using the
    corresponding setFlagNames() function.

    The flag manager provides properties with nested boolean
    subproperties representing each flag, i.e. a flag property's value
    is the binary combination of the subproperties' values. A
    property's value can be retrieved and set using the value() and
    setValue() slots respectively.

    The subproperties are created by a QtBoolPropertyManager object. This
    manager can be retrieved using the subBoolPropertyManager() function. In
    order to provide editing widgets for the subproperties in a
    property browser widget, this manager must be associated with an
    editor factory.

    In addition, QtFlagPropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes, and the flagNamesChanged() signal which is emitted
    whenever the list of flag names is altered.

    \sa QtAbstractPropertyManager, QtBoolPropertyManager
*/

/*!
    \fn void QtFlagPropertyManager::valueChanged(QtProperty *property, int value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a  property and the new
    \a value as parameters.

    \sa setValue()
*/

/*!
    \fn void QtFlagPropertyManager::flagNamesChanged(QtProperty *property, const QStringList &names)

    This signal is emitted whenever a property created by this manager
    changes its flag names, passing a pointer to the \a property and the
    new \a names as parameters.

    \sa setFlagNames()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtFlagPropertyManager::QtFlagPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtFlagPropertyManagerPrivate;
    d_ptr->q_ptr = this;

    d_ptr->m_boolPropertyManager = new QtBoolPropertyManager(this);
    connect(d_ptr->m_boolPropertyManager, SIGNAL(valueChanged(QtProperty *, bool)),
                this, SLOT(slotBoolChanged(QtProperty *, bool)));
    connect(d_ptr->m_boolPropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtFlagPropertyManager::~QtFlagPropertyManager()
{
    delete d_ptr;
}

/*!
    Returns the manager that produces the nested boolean subproperties
    representing each flag.

    In order to provide editing widgets for the subproperties in a
    property browser widget, this manager must be associated with an
    editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtBoolPropertyManager *QtFlagPropertyManager::subBoolPropertyManager() const
{
    return d_ptr->m_boolPropertyManager;
}

/*!
    Returns the given \a property's value.

    If the given property is not managed by this manager, this
    function returns 0.

    \sa flagNames(), setValue()
*/
int QtFlagPropertyManager::value(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return 0;
    return d_ptr->m_values[property].val;
}

/*!
    Returns the given \a property's list of flag names.

    \sa value(), setFlagNames()
*/
QStringList QtFlagPropertyManager::flagNames(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QStringList();
    return d_ptr->m_values[property].flagNames;
}

/*!
    \reimp
*/
QString QtFlagPropertyManager::valueText(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QString();

    QtFlagPropertyManagerPrivate::Data data = d_ptr->m_values[property];

    QString str;
    QStringListIterator itFlag(data.flagNames);
    int level = 0;
    while (itFlag.hasNext()) {
        QString flagName = itFlag.next();
        if (data.val & (1 << level)) {
            if (str.isNull())
                str = flagName;
            else
                str += QLatin1String("|") + flagName;
        }

        level++;
    }
    return str;
}

/*!
    \fn void QtFlagPropertyManager::setValue(QtProperty *property, int value)

    Sets the value of the given \a property to \a value. Nested
    properties are updated automatically.

    The specified \a value must be less than the binary combination of
    the property's flagNames() list size (i.e. less than 2\sup n,
    where \c n is the size of the list) and larger than (or equal to)
    0.

    \sa value(), valueChanged()
*/
void QtFlagPropertyManager::setValue(QtProperty *property, int val)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtFlagPropertyManagerPrivate::Data data = d_ptr->m_values[property];

    if (data.val == val)
        return;

    if (val > (1 << data.flagNames.count()) - 1)
        return;

    if (val < 0)
        return;

    data.val = val;

    d_ptr->m_values[property] = data;

    QListIterator<QtProperty *> itProp(d_ptr->m_propertyToFlags[property]);
    int level = 0;
    while (itProp.hasNext()) {
        QtProperty *prop = itProp.next();
        if (prop)
            d_ptr->m_boolPropertyManager->setValue(prop, val & (1 << level));
        level++;
    }

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    Sets the given \a property's list of flag names to \a flagNames. The
    property's current value is reset to 0 indicating the first item
    of the list.

    \sa flagNames(), flagNamesChanged()
*/
void QtFlagPropertyManager::setFlagNames(QtProperty *property, const QStringList &flagNames)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QtFlagPropertyManagerPrivate::Data data = d_ptr->m_values[property];

    if (data.flagNames == flagNames)
        return;

    data.flagNames = flagNames;
    data.val = 0;

    d_ptr->m_values[property] = data;

    QListIterator<QtProperty *> itProp(d_ptr->m_propertyToFlags[property]);
    while (itProp.hasNext()) {
        QtProperty *prop = itProp.next();
        if (prop) {
            delete prop;
            d_ptr->m_flagToProperty.remove(prop);
        }
    }
    d_ptr->m_propertyToFlags[property].clear();

    QStringListIterator itFlag(flagNames);
    while (itFlag.hasNext()) {
        QString flagName = itFlag.next();
        QtProperty *prop = d_ptr->m_boolPropertyManager->addProperty();
        prop->setPropertyName(flagName);
        property->addSubProperty(prop);
        d_ptr->m_propertyToFlags[property].append(prop);
        d_ptr->m_flagToProperty[prop] = property;
    }

    emit flagNamesChanged(property, data.flagNames);

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    \reimp
*/
void QtFlagPropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QtFlagPropertyManagerPrivate::Data();

    d_ptr->m_propertyToFlags[property] = QList<QtProperty *>();
}

/*!
    \reimp
*/
void QtFlagPropertyManager::uninitializeProperty(QtProperty *property)
{
    QListIterator<QtProperty *> itProp(d_ptr->m_propertyToFlags[property]);
    while (itProp.hasNext()) {
        QtProperty *prop = itProp.next();
        if (prop) {
            delete prop;
            d_ptr->m_flagToProperty.remove(prop);
        }
    }
    d_ptr->m_propertyToFlags.remove(property);

    d_ptr->m_values.remove(property);
}

// QtSizePolicyPropertyManager

class QtSizePolicyPropertyManagerPrivate
{
    QtSizePolicyPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtSizePolicyPropertyManager)
public:

    QtSizePolicyPropertyManagerPrivate();

    void slotIntChanged(QtProperty *property, int value);
    void slotEnumChanged(QtProperty *property, int value);
    void slotPropertyDestroyed(QtProperty *property);

    QMap<const QtProperty *, QSizePolicy> m_values;

    QtIntPropertyManager *m_intPropertyManager;
    QtEnumPropertyManager *m_enumPropertyManager;

    QMap<const QtProperty *, QtProperty *> m_propertyToHPolicy;
    QMap<const QtProperty *, QtProperty *> m_propertyToVPolicy;
    QMap<const QtProperty *, QtProperty *> m_propertyToHStretch;
    QMap<const QtProperty *, QtProperty *> m_propertyToVStretch;

    QMap<const QtProperty *, QtProperty *> m_hPolicyToProperty;
    QMap<const QtProperty *, QtProperty *> m_vPolicyToProperty;
    QMap<const QtProperty *, QtProperty *> m_hStretchToProperty;
    QMap<const QtProperty *, QtProperty *> m_vStretchToProperty;
};

QtSizePolicyPropertyManagerPrivate::QtSizePolicyPropertyManagerPrivate()
{
}

void QtSizePolicyPropertyManagerPrivate::slotIntChanged(QtProperty *property, int value)
{
    if (m_hStretchToProperty.contains(property)) {
        QtProperty *prop = m_hStretchToProperty[property];
        QSizePolicy sp = m_values[prop];
        sp.setHorizontalStretch(value);
        q_ptr->setValue(prop, sp);
    } else if (m_vStretchToProperty.contains(property)) {
        QtProperty *prop = m_vStretchToProperty[property];
        QSizePolicy sp = m_values[prop];
        sp.setVerticalStretch(value);
        q_ptr->setValue(prop, sp);
    }
}

void QtSizePolicyPropertyManagerPrivate::slotEnumChanged(QtProperty *property, int value)
{
    if (m_hPolicyToProperty.contains(property)) {
        QtProperty *prop = m_hPolicyToProperty[property];
        QSizePolicy sp = m_values[prop];
        sp.setHorizontalPolicy(metaEnumProvider()->indexToSizePolicy(value));
        q_ptr->setValue(prop, sp);
    } else if (m_vPolicyToProperty.contains(property)) {
        QtProperty *prop = m_vPolicyToProperty[property];
        QSizePolicy sp = m_values[prop];
        sp.setVerticalPolicy(metaEnumProvider()->indexToSizePolicy(value));
        q_ptr->setValue(prop, sp);
    }
}

void QtSizePolicyPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
    if (m_hStretchToProperty.contains(property)) {
        QtProperty *pointProp = m_hStretchToProperty[property];
        m_propertyToHStretch[pointProp] = 0;
        m_hStretchToProperty.remove(property);
    } else if (m_vStretchToProperty.contains(property)) {
        QtProperty *pointProp = m_vStretchToProperty[property];
        m_propertyToVStretch[pointProp] = 0;
        m_vStretchToProperty.remove(property);
    } else if (m_hPolicyToProperty.contains(property)) {
        QtProperty *pointProp = m_hPolicyToProperty[property];
        m_propertyToHPolicy[pointProp] = 0;
        m_hPolicyToProperty.remove(property);
    } else if (m_vPolicyToProperty.contains(property)) {
        QtProperty *pointProp = m_vPolicyToProperty[property];
        m_propertyToVPolicy[pointProp] = 0;
        m_vPolicyToProperty.remove(property);
    }
}

/*!
    \class QtSizePolicyPropertyManager

    \brief The QtSizePolicyPropertyManager provides and manages QSizePolicy properties.

    A size policy property has nested \e horizontalPolicy, \e
    verticalPolicy, \e horizontalStretch and \e verticalStretch
    subproperties. The top-level property's value can be retrieved
    using the value() function, and set using the setValue() slot.

    The subproperties are created by QtIntPropertyManager and QtEnumPropertyManager
    objects. These managers can be retrieved using the subIntPropertyManager()
    and subEnumPropertyManager() functions respectively. In order to provide
    editing widgets for the subproperties in a property browser widget,
    these managers must be associated with editor factories.

    In addition, QtSizePolicyPropertyManager provides the valueChanged()
    signal which is emitted whenever a property created by this
    manager changes.

    \sa QtAbstractPropertyManager, QtIntPropertyManager, QtEnumPropertyManager
*/

/*!
    \fn void QtSizePolicyPropertyManager::valueChanged(QtProperty *property, const QSizePolicy &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the
    new \a value as parameters.

    \sa setValue()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtSizePolicyPropertyManager::QtSizePolicyPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtSizePolicyPropertyManagerPrivate;
    d_ptr->q_ptr = this;

    d_ptr->m_intPropertyManager = new QtIntPropertyManager(this);
    connect(d_ptr->m_intPropertyManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotIntChanged(QtProperty *, int)));
    d_ptr->m_enumPropertyManager = new QtEnumPropertyManager(this);
    connect(d_ptr->m_enumPropertyManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotEnumChanged(QtProperty *, int)));

    connect(d_ptr->m_intPropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
    connect(d_ptr->m_enumPropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtSizePolicyPropertyManager::~QtSizePolicyPropertyManager()
{
    delete d_ptr;
}

/*!
    Returns the manager that creates the nested \e horizontalStretch
    and \e verticalStretch subproperties.

    In order to provide editing widgets for the mentioned subproperties
    in a property browser widget, this manager must be associated with
    an editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtIntPropertyManager *QtSizePolicyPropertyManager::subIntPropertyManager() const
{
    return d_ptr->m_intPropertyManager;
}

/*!
    Returns the manager that creates the nested \e horizontalPolicy
    and \e verticalPolicy subproperties.

    In order to provide editing widgets for the mentioned subproperties
    in a property browser widget, this manager must be associated with
    an editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtEnumPropertyManager *QtSizePolicyPropertyManager::subEnumPropertyManager() const
{
    return d_ptr->m_enumPropertyManager;
}

/*!
    Returns the given \a property's value.

    If the given property is not managed by this manager, this
    function returns the default size policy.

    \sa setValue()
*/
QSizePolicy QtSizePolicyPropertyManager::value(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QSizePolicy();
    return d_ptr->m_values[property];
}

/*!
    \reimp
*/
QString QtSizePolicyPropertyManager::valueText(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QString();

    QSizePolicy sp = d_ptr->m_values[property];

    QString str = tr("[%1, %2, %3, %4]")
            .arg(metaEnumProvider()->policyEnumNames().at(metaEnumProvider()->sizePolicyToIndex(sp.horizontalPolicy())))
            .arg(metaEnumProvider()->policyEnumNames().at(metaEnumProvider()->sizePolicyToIndex(sp.verticalPolicy())))
            .arg(sp.horizontalStretch())
            .arg(sp.verticalStretch());
    return str;
}

/*!
    \fn void QtSizePolicyPropertyManager::setValue(QtProperty *property, const QSizePolicy &value)

    Sets the value of the given \a property to \a value. Nested
    properties are updated automatically.

    \sa value(), valueChanged()
*/
void QtSizePolicyPropertyManager::setValue(QtProperty *property, const QSizePolicy &val)
{
    if (!d_ptr->m_values.contains(property))
        return;

    if (d_ptr->m_values[property] == val)
        return;

    d_ptr->m_values[property] = val;

    d_ptr->m_enumPropertyManager->setValue(d_ptr->m_propertyToHPolicy[property],
                metaEnumProvider()->sizePolicyToIndex(val.horizontalPolicy()));
    d_ptr->m_enumPropertyManager->setValue(d_ptr->m_propertyToVPolicy[property],
                metaEnumProvider()->sizePolicyToIndex(val.verticalPolicy()));
    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToHStretch[property],
                val.horizontalStretch());
    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToVStretch[property],
                val.verticalStretch());

    emit propertyChanged(property);
    emit valueChanged(property, val);
}

/*!
    \reimp
*/
void QtSizePolicyPropertyManager::initializeProperty(QtProperty *property)
{
    QSizePolicy val;
    d_ptr->m_values[property] = val;

    QtProperty *hPolicyProp = d_ptr->m_enumPropertyManager->addProperty();
    hPolicyProp->setPropertyName(tr("Horizontal Policy"));
    d_ptr->m_enumPropertyManager->setEnumNames(hPolicyProp, metaEnumProvider()->policyEnumNames());
    d_ptr->m_enumPropertyManager->setValue(hPolicyProp,
                metaEnumProvider()->sizePolicyToIndex(val.horizontalPolicy()));
    d_ptr->m_propertyToHPolicy[property] = hPolicyProp;
    d_ptr->m_hPolicyToProperty[hPolicyProp] = property;
    property->addSubProperty(hPolicyProp);

    QtProperty *vPolicyProp = d_ptr->m_enumPropertyManager->addProperty();
    vPolicyProp->setPropertyName(tr("Vertical Policy"));
    d_ptr->m_enumPropertyManager->setEnumNames(vPolicyProp, metaEnumProvider()->policyEnumNames());
    d_ptr->m_enumPropertyManager->setValue(vPolicyProp,
                metaEnumProvider()->sizePolicyToIndex(val.verticalPolicy()));
    d_ptr->m_propertyToVPolicy[property] = vPolicyProp;
    d_ptr->m_vPolicyToProperty[vPolicyProp] = property;
    property->addSubProperty(vPolicyProp);

    QtProperty *hStretchProp = d_ptr->m_intPropertyManager->addProperty();
    hStretchProp->setPropertyName(tr("Horizontal Stretch"));
    d_ptr->m_intPropertyManager->setValue(hStretchProp, val.horizontalStretch());
    d_ptr->m_intPropertyManager->setRange(hStretchProp, 0, 0xff);
    d_ptr->m_propertyToHStretch[property] = hStretchProp;
    d_ptr->m_hStretchToProperty[hStretchProp] = property;
    property->addSubProperty(hStretchProp);

    QtProperty *vStretchProp = d_ptr->m_intPropertyManager->addProperty();
    vStretchProp->setPropertyName(tr("Vertical Stretch"));
    d_ptr->m_intPropertyManager->setValue(vStretchProp, val.verticalStretch());
    d_ptr->m_intPropertyManager->setRange(vStretchProp, 0, 0xff);
    d_ptr->m_propertyToVStretch[property] = vStretchProp;
    d_ptr->m_vStretchToProperty[vStretchProp] = property;
    property->addSubProperty(vStretchProp);

}

/*!
    \reimp
*/
void QtSizePolicyPropertyManager::uninitializeProperty(QtProperty *property)
{
    QtProperty *hPolicyProp = d_ptr->m_propertyToHPolicy[property];
    if (hPolicyProp) {
        d_ptr->m_hPolicyToProperty.remove(hPolicyProp);
        delete hPolicyProp;
    }
    d_ptr->m_propertyToHPolicy.remove(property);

    QtProperty *vPolicyProp = d_ptr->m_propertyToVPolicy[property];
    if (vPolicyProp) {
        d_ptr->m_vPolicyToProperty.remove(vPolicyProp);
        delete vPolicyProp;
    }
    d_ptr->m_propertyToVPolicy.remove(property);

    QtProperty *hStretchProp = d_ptr->m_propertyToHStretch[property];
    if (hStretchProp) {
        d_ptr->m_hStretchToProperty.remove(hStretchProp);
        delete hStretchProp;
    }
    d_ptr->m_propertyToHStretch.remove(property);

    QtProperty *vStretchProp = d_ptr->m_propertyToVStretch[property];
    if (vStretchProp) {
        d_ptr->m_vStretchToProperty.remove(vStretchProp);
        delete vStretchProp;
    }
    d_ptr->m_propertyToVStretch.remove(property);

    d_ptr->m_values.remove(property);
}

// QtFontPropertyManager

Q_GLOBAL_STATIC(QFontDatabase, fontDatabase)

class QtFontPropertyManagerPrivate
{
    QtFontPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtFontPropertyManager)
public:

    QtFontPropertyManagerPrivate();

    void slotIntChanged(QtProperty *property, int value);
    void slotEnumChanged(QtProperty *property, int value);
    void slotBoolChanged(QtProperty *property, bool value);
    void slotPropertyDestroyed(QtProperty *property);

    QMap<const QtProperty *, QFont> m_values;

    QStringList m_familyNames;

    QtIntPropertyManager *m_intPropertyManager;
    QtEnumPropertyManager *m_enumPropertyManager;
    QtBoolPropertyManager *m_boolPropertyManager;

    QMap<const QtProperty *, QtProperty *> m_propertyToFamily;
    QMap<const QtProperty *, QtProperty *> m_propertyToPointSize;
    QMap<const QtProperty *, QtProperty *> m_propertyToBold;
    QMap<const QtProperty *, QtProperty *> m_propertyToItalic;
    QMap<const QtProperty *, QtProperty *> m_propertyToUnderline;
    QMap<const QtProperty *, QtProperty *> m_propertyToStrikeOut;
    QMap<const QtProperty *, QtProperty *> m_propertyToKerning;

    QMap<const QtProperty *, QtProperty *> m_familyToProperty;
    QMap<const QtProperty *, QtProperty *> m_pointSizeToProperty;
    QMap<const QtProperty *, QtProperty *> m_boldToProperty;
    QMap<const QtProperty *, QtProperty *> m_italicToProperty;
    QMap<const QtProperty *, QtProperty *> m_underlineToProperty;
    QMap<const QtProperty *, QtProperty *> m_strikeOutToProperty;
    QMap<const QtProperty *, QtProperty *> m_kerningToProperty;
};

QtFontPropertyManagerPrivate::QtFontPropertyManagerPrivate()
{
    m_familyNames = fontDatabase()->families();
}

void QtFontPropertyManagerPrivate::slotIntChanged(QtProperty *property, int value)
{
    if (m_pointSizeToProperty.contains(property)) {
        QtProperty *prop = m_pointSizeToProperty[property];
        QFont f = m_values[prop];
        f.setPointSize(value);
        q_ptr->setValue(prop, f);
    }
}

void QtFontPropertyManagerPrivate::slotEnumChanged(QtProperty *property, int value)
{
    if (m_familyToProperty.contains(property)) {
        QtProperty *prop = m_familyToProperty[property];
        QFont f = m_values[prop];
        f.setFamily(m_familyNames.at(value));
        q_ptr->setValue(prop, f);
    }
}

void QtFontPropertyManagerPrivate::slotBoolChanged(QtProperty *property, bool value)
{
    if (m_boldToProperty.contains(property)) {
        QtProperty *prop = m_boldToProperty[property];
        QFont f = m_values[prop];
        f.setBold(value);
        q_ptr->setValue(prop, f);
    } else if (m_italicToProperty.contains(property)) {
        QtProperty *prop = m_italicToProperty[property];
        QFont f = m_values[prop];
        f.setItalic(value);
        q_ptr->setValue(prop, f);
    } else if (m_underlineToProperty.contains(property)) {
        QtProperty *prop = m_underlineToProperty[property];
        QFont f = m_values[prop];
        f.setUnderline(value);
        q_ptr->setValue(prop, f);
    } else if (m_strikeOutToProperty.contains(property)) {
        QtProperty *prop = m_strikeOutToProperty[property];
        QFont f = m_values[prop];
        f.setStrikeOut(value);
        q_ptr->setValue(prop, f);
    } else if (m_kerningToProperty.contains(property)) {
        QtProperty *prop = m_kerningToProperty[property];
        QFont f = m_values[prop];
        f.setKerning(value);
        q_ptr->setValue(prop, f);
    }
}

void QtFontPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
    if (m_pointSizeToProperty.contains(property)) {
        QtProperty *pointProp = m_pointSizeToProperty[property];
        m_propertyToPointSize[pointProp] = 0;
        m_pointSizeToProperty.remove(property);
    } else if (m_familyToProperty.contains(property)) {
        QtProperty *pointProp = m_familyToProperty[property];
        m_propertyToFamily[pointProp] = 0;
        m_familyToProperty.remove(property);
    } else if (m_boldToProperty.contains(property)) {
        QtProperty *pointProp = m_boldToProperty[property];
        m_propertyToBold[pointProp] = 0;
        m_boldToProperty.remove(property);
    } else if (m_italicToProperty.contains(property)) {
        QtProperty *pointProp = m_italicToProperty[property];
        m_propertyToItalic[pointProp] = 0;
        m_italicToProperty.remove(property);
    } else if (m_underlineToProperty.contains(property)) {
        QtProperty *pointProp = m_underlineToProperty[property];
        m_propertyToUnderline[pointProp] = 0;
        m_underlineToProperty.remove(property);
    } else if (m_strikeOutToProperty.contains(property)) {
        QtProperty *pointProp = m_strikeOutToProperty[property];
        m_propertyToStrikeOut[pointProp] = 0;
        m_strikeOutToProperty.remove(property);
    } else if (m_kerningToProperty.contains(property)) {
        QtProperty *pointProp = m_kerningToProperty[property];
        m_propertyToKerning[pointProp] = 0;
        m_kerningToProperty.remove(property);
    }
}

/*!
    \class QtFontPropertyManager

    \brief The QtFontPropertyManager provides and manages QFont properties.

    A font property has nested \e family, \e pointSize, \e bold, \e
    italic, \e underline, \e strikeOut and \e kerning subproperties. The top-level
    property's value can be retrieved using the value() function, and
    set using the setValue() slot.

    The subproperties are created by QtIntPropertyManager, QtEnumPropertyManager and
    QtBoolPropertyManager objects. These managers can be retrieved using the
    corresponding subIntPropertyManager(), subEnumPropertyManager() and
    subBoolPropertyManager() functions. In order to provide editing widgets
    for the subproperties in a property browser widget, these managers
    must be associated with editor factories.

    In addition, QtFontPropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes.

    \sa QtAbstractPropertyManager, QtEnumPropertyManager, QtIntPropertyManager, QtBoolPropertyManager
*/

/*!
    \fn void QtFontPropertyManager::valueChanged(QtProperty *property, const QFont &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the
    new \a value as parameters.

    \sa setValue()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtFontPropertyManager::QtFontPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtFontPropertyManagerPrivate;
    d_ptr->q_ptr = this;

    d_ptr->m_intPropertyManager = new QtIntPropertyManager(this);
    connect(d_ptr->m_intPropertyManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotIntChanged(QtProperty *, int)));
    d_ptr->m_enumPropertyManager = new QtEnumPropertyManager(this);
    connect(d_ptr->m_enumPropertyManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotEnumChanged(QtProperty *, int)));
    d_ptr->m_boolPropertyManager = new QtBoolPropertyManager(this);
    connect(d_ptr->m_boolPropertyManager, SIGNAL(valueChanged(QtProperty *, bool)),
                this, SLOT(slotBoolChanged(QtProperty *, bool)));

    connect(d_ptr->m_intPropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
    connect(d_ptr->m_enumPropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
    connect(d_ptr->m_boolPropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtFontPropertyManager::~QtFontPropertyManager()
{
    delete d_ptr;
}

/*!
    Returns the manager that creates the \e pointSize subproperty.

    In order to provide editing widgets for the \e pointSize property
    in a property browser widget, this manager must be associated
    with an editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtIntPropertyManager *QtFontPropertyManager::subIntPropertyManager() const
{
    return d_ptr->m_intPropertyManager;
}

/*!
    Returns the manager that create the \e family subproperty.

    In order to provide editing widgets for the \e family property
    in a property browser widget, this manager must be associated
    with an editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtEnumPropertyManager *QtFontPropertyManager::subEnumPropertyManager() const
{
    return d_ptr->m_enumPropertyManager;
}

/*!
    Returns the manager that creates the  \e bold, \e italic, \e underline,
    \e strikeOut and \e kerning subproperties.

    In order to provide editing widgets for the mentioned properties
    in a property browser widget, this manager must be associated with
    an editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtBoolPropertyManager *QtFontPropertyManager::subBoolPropertyManager() const
{
    return d_ptr->m_boolPropertyManager;
}

/*!
    Returns the given \a property's value.

    If the given property is not managed by this manager, this
    function returns a font object that uses the application's default
    font.

    \sa setValue()
*/
QFont QtFontPropertyManager::value(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QFont();
    return d_ptr->m_values[property];
}

/*!
    \reimp
*/
QString QtFontPropertyManager::valueText(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QString();

    QFont f = d_ptr->m_values[property];

    QString str = tr("[%1, %2]").arg(f.family()).arg(f.pointSize());
    return str;
}

/*!
    \reimp
*/
QIcon QtFontPropertyManager::valueIcon(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QIcon();

    QFont f = d_ptr->m_values[property];

    QImage img(16, 16, QImage::Format_ARGB32_Premultiplied);
    img.fill(0);
    QPainter p(&img);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setRenderHint(QPainter::Antialiasing, true);
    f.setPointSize(13);
    p.setFont(f);
    QTextOption t;
    t.setAlignment(Qt::AlignCenter);
    p.drawText(QRect(0, 0, 16, 16), QLatin1String("A"), t);
    //p.drawRect(QRect(0, 0, 15, 15));
    QPixmap pix = QPixmap::fromImage(img);
    return QIcon(pix);
}

/*!
    \fn void QtFontPropertyManager::setValue(QtProperty *property, const QFont &value)

    Sets the value of the given \a property to \a value. Nested
    properties are updated automatically.

    \sa value(), valueChanged()
*/
void QtFontPropertyManager::setValue(QtProperty *property, const QFont &val)
{
    if (!d_ptr->m_values.contains(property))
        return;

    QFont oldVal = d_ptr->m_values[property];
    if (oldVal == val && oldVal.resolve() == val.resolve())
        return;

    d_ptr->m_values[property] = val;

    int idx = d_ptr->m_familyNames.indexOf(val.family());
    if (idx == -1)
        idx = 0;
    d_ptr->m_enumPropertyManager->setValue(d_ptr->m_propertyToFamily[property], idx);
    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToPointSize[property], val.pointSize());
    d_ptr->m_boolPropertyManager->setValue(d_ptr->m_propertyToBold[property], val.bold());
    d_ptr->m_boolPropertyManager->setValue(d_ptr->m_propertyToItalic[property], val.italic());
    d_ptr->m_boolPropertyManager->setValue(d_ptr->m_propertyToUnderline[property], val.underline());
    d_ptr->m_boolPropertyManager->setValue(d_ptr->m_propertyToStrikeOut[property], val.strikeOut());
    d_ptr->m_boolPropertyManager->setValue(d_ptr->m_propertyToKerning[property], val.kerning());

    emit propertyChanged(property);
    emit valueChanged(property, val);
}

/*!
    \reimp
*/
void QtFontPropertyManager::initializeProperty(QtProperty *property)
{
    QFont val;
    d_ptr->m_values[property] = val;

    QtProperty *familyProp = d_ptr->m_enumPropertyManager->addProperty();
    familyProp->setPropertyName(tr("Family"));
    d_ptr->m_enumPropertyManager->setEnumNames(familyProp, d_ptr->m_familyNames);
    int idx = d_ptr->m_familyNames.indexOf(val.family());
    if (idx == -1)
        idx = 0;
    d_ptr->m_enumPropertyManager->setValue(familyProp, idx);
    d_ptr->m_propertyToFamily[property] = familyProp;
    d_ptr->m_familyToProperty[familyProp] = property;
    property->addSubProperty(familyProp);

    QtProperty *pointSizeProp = d_ptr->m_intPropertyManager->addProperty();
    pointSizeProp->setPropertyName(tr("Point Size"));
    d_ptr->m_intPropertyManager->setValue(pointSizeProp, val.pointSize());
    d_ptr->m_intPropertyManager->setMinimum(pointSizeProp, 1);
    d_ptr->m_propertyToPointSize[property] = pointSizeProp;
    d_ptr->m_pointSizeToProperty[pointSizeProp] = property;
    property->addSubProperty(pointSizeProp);

    QtProperty *boldProp = d_ptr->m_boolPropertyManager->addProperty();
    boldProp->setPropertyName(tr("Bold"));
    d_ptr->m_boolPropertyManager->setValue(boldProp, val.bold());
    d_ptr->m_propertyToBold[property] = boldProp;
    d_ptr->m_boldToProperty[boldProp] = property;
    property->addSubProperty(boldProp);

    QtProperty *italicProp = d_ptr->m_boolPropertyManager->addProperty();
    italicProp->setPropertyName(tr("Italic"));
    d_ptr->m_boolPropertyManager->setValue(italicProp, val.italic());
    d_ptr->m_propertyToItalic[property] = italicProp;
    d_ptr->m_italicToProperty[italicProp] = property;
    property->addSubProperty(italicProp);

    QtProperty *underlineProp = d_ptr->m_boolPropertyManager->addProperty();
    underlineProp->setPropertyName(tr("Underline"));
    d_ptr->m_boolPropertyManager->setValue(underlineProp, val.underline());
    d_ptr->m_propertyToUnderline[property] = underlineProp;
    d_ptr->m_underlineToProperty[underlineProp] = property;
    property->addSubProperty(underlineProp);

    QtProperty *strikeOutProp = d_ptr->m_boolPropertyManager->addProperty();
    strikeOutProp->setPropertyName(tr("Strikeout"));
    d_ptr->m_boolPropertyManager->setValue(strikeOutProp, val.strikeOut());
    d_ptr->m_propertyToStrikeOut[property] = strikeOutProp;
    d_ptr->m_strikeOutToProperty[strikeOutProp] = property;
    property->addSubProperty(strikeOutProp);

    QtProperty *kerningProp = d_ptr->m_boolPropertyManager->addProperty();
    kerningProp->setPropertyName(tr("Kerning"));
    d_ptr->m_boolPropertyManager->setValue(kerningProp, val.kerning());
    d_ptr->m_propertyToKerning[property] = kerningProp;
    d_ptr->m_kerningToProperty[kerningProp] = property;
    property->addSubProperty(kerningProp);
}

/*!
    \reimp
*/
void QtFontPropertyManager::uninitializeProperty(QtProperty *property)
{
    QtProperty *familyProp = d_ptr->m_propertyToFamily[property];
    if (familyProp) {
        d_ptr->m_familyToProperty.remove(familyProp);
        delete familyProp;
    }
    d_ptr->m_propertyToFamily.remove(property);

    QtProperty *pointSizeProp = d_ptr->m_propertyToPointSize[property];
    if (pointSizeProp) {
        d_ptr->m_pointSizeToProperty.remove(pointSizeProp);
        delete pointSizeProp;
    }
    d_ptr->m_propertyToPointSize.remove(property);

    QtProperty *boldProp = d_ptr->m_propertyToBold[property];
    if (boldProp) {
        d_ptr->m_boldToProperty.remove(boldProp);
        delete boldProp;
    }
    d_ptr->m_propertyToBold.remove(property);

    QtProperty *italicProp = d_ptr->m_propertyToItalic[property];
    if (italicProp) {
        d_ptr->m_italicToProperty.remove(italicProp);
        delete italicProp;
    }
    d_ptr->m_propertyToItalic.remove(property);

    QtProperty *underlineProp = d_ptr->m_propertyToUnderline[property];
    if (underlineProp) {
        d_ptr->m_underlineToProperty.remove(underlineProp);
        delete underlineProp;
    }
    d_ptr->m_propertyToUnderline.remove(property);

    QtProperty *strikeOutProp = d_ptr->m_propertyToStrikeOut[property];
    if (strikeOutProp) {
        d_ptr->m_strikeOutToProperty.remove(strikeOutProp);
        delete strikeOutProp;
    }
    d_ptr->m_propertyToStrikeOut.remove(property);

    QtProperty *kerningProp = d_ptr->m_propertyToKerning[property];
    if (kerningProp) {
        d_ptr->m_kerningToProperty.remove(kerningProp);
        delete kerningProp;
    }
    d_ptr->m_propertyToKerning.remove(property);

    d_ptr->m_values.remove(property);
}

// QtColorPropertyManager

class QtColorPropertyManagerPrivate
{
    QtColorPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtColorPropertyManager)
public:

    void slotIntChanged(QtProperty *property, int value);
    void slotPropertyDestroyed(QtProperty *property);

    QMap<const QtProperty *, QColor> m_values;

    QtIntPropertyManager *m_intPropertyManager;

    QMap<const QtProperty *, QtProperty *> m_propertyToR;
    QMap<const QtProperty *, QtProperty *> m_propertyToG;
    QMap<const QtProperty *, QtProperty *> m_propertyToB;
    QMap<const QtProperty *, QtProperty *> m_propertyToA;

    QMap<const QtProperty *, QtProperty *> m_rToProperty;
    QMap<const QtProperty *, QtProperty *> m_gToProperty;
    QMap<const QtProperty *, QtProperty *> m_bToProperty;
    QMap<const QtProperty *, QtProperty *> m_aToProperty;
};

void QtColorPropertyManagerPrivate::slotIntChanged(QtProperty *property, int value)
{
    if (m_rToProperty.contains(property)) {
        QtProperty *prop = m_rToProperty[property];
        QColor c = m_values[prop];
        c.setRed(value);
        q_ptr->setValue(prop, c);
    } else if (m_gToProperty.contains(property)) {
        QtProperty *prop = m_gToProperty[property];
        QColor c = m_values[prop];
        c.setGreen(value);
        q_ptr->setValue(prop, c);
    } else if (m_bToProperty.contains(property)) {
        QtProperty *prop = m_bToProperty[property];
        QColor c = m_values[prop];
        c.setBlue(value);
        q_ptr->setValue(prop, c);
    } else if (m_aToProperty.contains(property)) {
        QtProperty *prop = m_aToProperty[property];
        QColor c = m_values[prop];
        c.setAlpha(value);
        q_ptr->setValue(prop, c);
    }
}

void QtColorPropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
    if (m_rToProperty.contains(property)) {
        QtProperty *pointProp = m_rToProperty[property];
        m_propertyToR[pointProp] = 0;
        m_rToProperty.remove(property);
    } else if (m_gToProperty.contains(property)) {
        QtProperty *pointProp = m_gToProperty[property];
        m_propertyToG[pointProp] = 0;
        m_gToProperty.remove(property);
    } else if (m_bToProperty.contains(property)) {
        QtProperty *pointProp = m_bToProperty[property];
        m_propertyToB[pointProp] = 0;
        m_bToProperty.remove(property);
    } else if (m_aToProperty.contains(property)) {
        QtProperty *pointProp = m_aToProperty[property];
        m_propertyToA[pointProp] = 0;
        m_aToProperty.remove(property);
    }
}

/*!
    \class QtColorPropertyManager

    \brief The QtColorPropertyManager provides and manages QColor properties.

    A color property has nested \e red, \e green and \e blue
    subproperties. The top-level property's value can be retrieved
    using the value() function, and set using the setValue() slot.

    The subproperties are created by a QtIntPropertyManager object. This
    manager can be retrieved using the subIntPropertyManager() function.  In
    order to provide editing widgets for the subproperties in a
    property browser widget, this manager must be associated with an
    editor factory.

    In addition, QtColorPropertyManager provides the valueChanged() signal
    which is emitted whenever a property created by this manager
    changes.

    \sa QtAbstractPropertyManager, QtAbstractPropertyBrowser, QtIntPropertyManager
*/

/*!
    \fn void QtColorPropertyManager::valueChanged(QtProperty *property, const QColor &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the new
    \a value as parameters.

    \sa setValue()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtColorPropertyManager::QtColorPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtColorPropertyManagerPrivate;
    d_ptr->q_ptr = this;

    d_ptr->m_intPropertyManager = new QtIntPropertyManager(this);
    connect(d_ptr->m_intPropertyManager, SIGNAL(valueChanged(QtProperty *, int)),
                this, SLOT(slotIntChanged(QtProperty *, int)));

    connect(d_ptr->m_intPropertyManager, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtColorPropertyManager::~QtColorPropertyManager()
{
    delete d_ptr;
}

/*!
    Returns the manager that produces the nested \e red, \e green and
    \e blue subproperties.

    In order to provide editing widgets for the subproperties in a
    property browser widget, this manager must be associated with an
    editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtIntPropertyManager *QtColorPropertyManager::subIntPropertyManager() const
{
    return d_ptr->m_intPropertyManager;
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by \e this manager, this
    function returns an invalid color.

    \sa setValue()
*/
QColor QtColorPropertyManager::value(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QColor();
    return d_ptr->m_values[property];
}

/*!
    \reimp
*/
QString QtColorPropertyManager::valueText(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QString();

    QColor c = d_ptr->m_values[property];

    QString str = tr("[%1, %2, %3] (%4)").arg(QString::number(c.red()))
                                            .arg(QString::number(c.green()))
                                            .arg(QString::number(c.blue()))
                                            .arg(QString::number(c.alpha()));
    return str;
}

/*!
    \reimp
*/
QIcon QtColorPropertyManager::valueIcon(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QIcon();

    QColor c = d_ptr->m_values[property];

    QImage img(16, 16, QImage::Format_ARGB32);

    QPainter painter(&img);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(0, 0, img.width(), img.height(), c);
    painter.fillRect(img.width() / 4, img.height() / 4,
                     img.width() / 2, img.height() / 2,
                     QColor(c.rgb()));
    painter.end();

    QPixmap pix = QPixmap::fromImage(img);
    return QIcon(pix);
}

/*!
    \fn void QtColorPropertyManager::setValue(QtProperty *property, const QColor &value)

    Sets the value of the given \a property to \a value.  Nested
    properties are updated automatically.

    \sa value(), valueChanged()
*/
void QtColorPropertyManager::setValue(QtProperty *property, const QColor &val)
{
    if (!d_ptr->m_values.contains(property))
        return;

    if (d_ptr->m_values[property] == val)
        return;

    d_ptr->m_values[property] = val;

    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToR[property], val.red());
    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToG[property], val.green());
    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToB[property], val.blue());
    d_ptr->m_intPropertyManager->setValue(d_ptr->m_propertyToA[property], val.alpha());

    emit propertyChanged(property);
    emit valueChanged(property, val);
}

/*!
    \reimp
*/
void QtColorPropertyManager::initializeProperty(QtProperty *property)
{
    QColor val;
    d_ptr->m_values[property] = val;

    QtProperty *rProp = d_ptr->m_intPropertyManager->addProperty();
    rProp->setPropertyName(tr("Red"));
    d_ptr->m_intPropertyManager->setValue(rProp, val.red());
    d_ptr->m_intPropertyManager->setRange(rProp, 0, 0xFF);
    d_ptr->m_propertyToR[property] = rProp;
    d_ptr->m_rToProperty[rProp] = property;
    property->addSubProperty(rProp);

    QtProperty *gProp = d_ptr->m_intPropertyManager->addProperty();
    gProp->setPropertyName(tr("Green"));
    d_ptr->m_intPropertyManager->setValue(gProp, val.green());
    d_ptr->m_intPropertyManager->setRange(gProp, 0, 0xFF);
    d_ptr->m_propertyToG[property] = gProp;
    d_ptr->m_gToProperty[gProp] = property;
    property->addSubProperty(gProp);

    QtProperty *bProp = d_ptr->m_intPropertyManager->addProperty();
    bProp->setPropertyName(tr("Blue"));
    d_ptr->m_intPropertyManager->setValue(bProp, val.blue());
    d_ptr->m_intPropertyManager->setRange(bProp, 0, 0xFF);
    d_ptr->m_propertyToB[property] = bProp;
    d_ptr->m_bToProperty[bProp] = property;
    property->addSubProperty(bProp);

    QtProperty *aProp = d_ptr->m_intPropertyManager->addProperty();
    aProp->setPropertyName(tr("Alpha"));
    d_ptr->m_intPropertyManager->setValue(aProp, val.alpha());
    d_ptr->m_intPropertyManager->setRange(aProp, 0, 0xFF);
    d_ptr->m_propertyToA[property] = aProp;
    d_ptr->m_aToProperty[aProp] = property;
    property->addSubProperty(aProp);
}

/*!
    \reimp
*/
void QtColorPropertyManager::uninitializeProperty(QtProperty *property)
{
    QtProperty *rProp = d_ptr->m_propertyToR[property];
    if (rProp) {
        d_ptr->m_rToProperty.remove(rProp);
        delete rProp;
    }
    d_ptr->m_propertyToR.remove(property);

    QtProperty *gProp = d_ptr->m_propertyToG[property];
    if (gProp) {
        d_ptr->m_gToProperty.remove(gProp);
        delete gProp;
    }
    d_ptr->m_propertyToG.remove(property);

    QtProperty *bProp = d_ptr->m_propertyToB[property];
    if (bProp) {
        d_ptr->m_bToProperty.remove(bProp);
        delete bProp;
    }
    d_ptr->m_propertyToB.remove(property);

    QtProperty *aProp = d_ptr->m_propertyToA[property];
    if (aProp) {
        d_ptr->m_aToProperty.remove(aProp);
        delete aProp;
    }
    d_ptr->m_propertyToA.remove(property);

    d_ptr->m_values.remove(property);
}

// QtCursorPropertyManager

Q_GLOBAL_STATIC(QtCursorDatabase, cursorDatabase)

class QtCursorPropertyManagerPrivate
{
    QtCursorPropertyManager *q_ptr;
    Q_DECLARE_PUBLIC(QtCursorPropertyManager)
public:
    QMap<const QtProperty *, QCursor> m_values;
};

/*!
    \class QtCursorPropertyManager

    \brief The QtCursorPropertyManager provides and manages QCursor properties.

    A cursor property has a current value which can be
    retrieved using the value() function, and set using the setValue()
    slot. In addition, QtCursorPropertyManager provides the
    valueChanged() signal which is emitted whenever a property created
    by this manager changes.

    \sa QtAbstractPropertyManager
*/

/*!
    \fn void QtCursorPropertyManager::valueChanged(QtProperty *property, const QCursor &value)

    This signal is emitted whenever a property created by this manager
    changes its value, passing a pointer to the \a property and the new
    \a value as parameters.

    \sa setValue()
*/

/*!
    Creates a manager with the given \a parent.
*/
QtCursorPropertyManager::QtCursorPropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtCursorPropertyManagerPrivate;
    d_ptr->q_ptr = this;
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtCursorPropertyManager::~QtCursorPropertyManager()
{
    delete d_ptr;
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by this manager, this
    function returns a default QCursor object.

    \sa setValue()
*/
QCursor QtCursorPropertyManager::value(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QCursor();
    return d_ptr->m_values[property];
}

/*!
    \reimp
*/
QString QtCursorPropertyManager::valueText(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QString();

    return cursorDatabase()->cursorToShapeName(d_ptr->m_values[property]);
}

/*!
    \reimp
*/
QIcon QtCursorPropertyManager::valueIcon(const QtProperty *property) const
{
    if (!d_ptr->m_values.contains(property))
        return QIcon();

    return cursorDatabase()->cursorToShapeIcon(d_ptr->m_values[property]);
}

/*!
    \fn void QtCursorPropertyManager::setValue(QtProperty *property, const QCursor &value)

    Sets the value of the given \a property to \a value.

    \sa value(), valueChanged()
*/
void QtCursorPropertyManager::setValue(QtProperty *property, const QCursor &value)
{
    if (!d_ptr->m_values.contains(property))
        return;

    if (d_ptr->m_values[property].shape() == value.shape() && value.shape() != Qt::BitmapCursor)
        return;

    d_ptr->m_values[property] = value;

    emit propertyChanged(property);
    emit valueChanged(property, value);
}

/*!
    \reimp
*/
void QtCursorPropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QCursor();
}

/*!
    \reimp
*/
void QtCursorPropertyManager::uninitializeProperty(QtProperty *property)
{
    d_ptr->m_values.remove(property);
}

#include "moc_qtpropertymanager.cpp"
#include "qtpropertymanager.moc"
