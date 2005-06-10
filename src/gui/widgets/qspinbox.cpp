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

#include <private/qabstractspinbox_p.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <qlocale.h>
#include <qvalidator.h>
#include <qdebug.h>
//#define QSPINBOX_QSBDEBUG
#ifdef QSPINBOX_QSBDEBUG
#  define QSBDEBUG qDebug
#else
#  define QSBDEBUG if (false) qDebug
#endif

static const char dot = '.';
static bool isIntermediateValueHelper(qint64 num, qint64 min, qint64 max, qint64 *match = 0);

class QSpinBoxPrivate : public QAbstractSpinBoxPrivate
{
    Q_DECLARE_PUBLIC(QSpinBox)
public:
    QSpinBoxPrivate();
    void emitSignals(EmitPolicy ep, const QVariant &);

    virtual QVariant valueFromText(const QString &n) const;
    virtual QString textFromValue(const QVariant &n) const;
    QVariant validateAndInterpret(QString &input, int &pos,
                                  QValidator::State &state) const;
    bool isIntermediateValue(const QString &str) const;
    QChar thousand;
};

class QDoubleSpinBoxPrivate : public QAbstractSpinBoxPrivate
{
    Q_DECLARE_PUBLIC(QDoubleSpinBox)
public:
    QDoubleSpinBoxPrivate();
    void emitSignals(EmitPolicy ep, const QVariant &);
    bool isIntermediateValue(const QString &str) const;
    int findDelimiter(const QString &str, int index = 0) const;

    virtual QVariant valueFromText(const QString &n) const;
    virtual QString textFromValue(const QVariant &n) const;
    QVariant validateAndInterpret(QString &input, int &pos,
                                  QValidator::State &state) const;
    // variables
    int decimals;
    QChar delimiter, thousand;
};


/*!
    \class QSpinBox
    \brief The QSpinBox class provides a spin box widget.

    \ingroup basic
    \mainclass

    QSpinBox is designed to handle integers and discrete sets of
    values (e.g., month names); use QDoubleSpinBox for floating point
    values.

    QSpinBox allows the user to choose a value by clicking the up/down
    buttons or pressing up/down on the keyboard to increase/decrease
    the value currently displayed. The user can also type the value in
    manually. If the value is entered directly into the spin box, the
    value will be changed and valueChanged() will be emitted with the
    new value when Enter/Return is pressed, when the spin box looses
    focus or when the spin box is deactivated (see
    QWidget::windowActivationChanged()). The spin box supports integer
    values but can be extended to use different strings with
    validate(), textFromValue() and valueFromText().

    Every time the value changes QSpinBox emits the valueChanged()
    signals. The current value can be fetched with value() and set
    with setValue().

    Clicking the up/down buttons or using the keyboard accelerator's
    up and down arrows will increase or decrease the current value in
    steps of size singleStep(). If you want to change this behaviour you
    can reimplement the virtual function stepBy(). The minimum and
    maximum value and the step size can be set using one of the
    constructors, and can be changed later with setMinimum(),
    setMaximum() and setSingleStep().

    Most spin boxes are directional, but QSpinBox can also operate as
    a circular spin box, i.e. if the range is 0-99 and the current
    value is 99, clicking "up" will give 0 if wrapping() is set to
    true. Use setWrapping() if you want circular behavior.

    The displayed value can be prepended and appended with arbitrary
    strings indicating, for example, currency or the unit of
    measurement. See setPrefix() and setSuffix(). The text in the spin
    box is retrieved with text() (which includes any prefix() and
    suffix()), or with cleanText() (which has no prefix(), no suffix()
    and no leading or trailing whitespace).

    It is often desirable to give the user a special (often default)
    choice in addition to the range of numeric values. See
    setSpecialValueText() for how to do this with QSpinBox.

    \inlineimage macintosh-spinbox.png Screenshot in Macintosh style
    \inlineimage windows-spinbox.png Screenshot in Windows style

    \section1 Subclassing QSpinBox

    If using prefix(), suffix(), and specialValueText() don't provide
    enough control, you subclass QSpinBox and reimplement
    valueFromText() and textFromValue(). For example, here's the code
    for a custom spin box that allows the user to enter icon sizes
    (e.g., "32 x 32"):

    \quotefromfile widgets/icons/iconsizespinbox.cpp
    \skipto ::valueFromText
    \printuntil /^\}$/
    \skipto ::textFromValue
    \printuntil /^\}$/

    See the \l{widgets/icons}{Icons} example for the full source
    code.

    \sa QDoubleSpinBox, QSlider, {fowler}{GUI Design Handbook: Slider}
*/

/*!
    \fn void QSpinBox::valueChanged(int i)

    This signal is emitted whenever the spin box's value is changed.
    The new value's integer value is passed in \a i.
*/

/*!
    \fn void QSpinBox::valueChanged(const QString &text)

    \overload

    The new value is passed literally in \a text with no prefix() or
    suffix().
*/

/*!
    Constructs a spin box with no minimum and maximum values, a step
    value of 1. The value is initially set to 0. It is parented to \a
    parent.

    \sa setMinimum(), setMaximum(), setSingleStep()
*/

QSpinBox::QSpinBox(QWidget *parent)
    : QAbstractSpinBox(*new QSpinBoxPrivate, parent)
{
}

#ifdef QT3_SUPPORT
/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QSpinBox::QSpinBox(QWidget *parent, const char *name)
    : QAbstractSpinBox(*new QSpinBoxPrivate, parent)
{
    setObjectName(name);
}

/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QSpinBox::QSpinBox(int min, int max, int step, QWidget *parent, const char *name)
    : QAbstractSpinBox(*new QSpinBoxPrivate, parent)
{
    Q_D(QSpinBox);
    d->minimum = QVariant(qMin<int>(min, max));
    d->maximum = QVariant(qMax<int>(min, max));
    d->singleStep = QVariant(step);
    setObjectName(name);
}

#endif

/*!
    \property QSpinBox::value
    \brief the value of the spin box

    setValue() will emit valueChanged() if the new value is different
    from the old one.
*/

int QSpinBox::value() const
{
    Q_D(const QSpinBox);
    return d->value.toInt();
}

void QSpinBox::setValue(int val)
{
    Q_D(QSpinBox);
    d->setValue(QVariant(val), EmitIfChanged);
}

/*!
    \property QSpinBox::prefix
    \brief the spin box's prefix

    The prefix is prepended to the start of the displayed value.
    Typical use is to display a unit of measurement or a currency
    symbol. For example:

    \code
        sb->setPrefix("$");
    \endcode

    To turn off the prefix display, set this property to an empty
    string. The default is no prefix. The prefix is not displayed when
    value() == minimum() and specialValueText() is set.

    If no prefix is set, prefix() returns an empty string.

    \sa suffix(), setSuffix(), specialValueText(), setSpecialValueText()
*/

QString QSpinBox::prefix() const
{
    Q_D(const QSpinBox);
    return d->prefix;
}

void QSpinBox::setPrefix(const QString &p)
{
    Q_D(QSpinBox);

    d->prefix = p;
    d->update();
}

/*!
    \property QSpinBox::suffix
    \brief the suffix of the spin box

    The suffix is appended to the end of the displayed value. Typical
    use is to display a unit of measurement or a currency symbol. For
    example:

    \code
        sb->setSuffix(" km");
    \endcode

    To turn off the suffix display, set this property to an empty
    string. The default is no suffix. The suffix is not displayed for
    the minimum() if specialValueText() is set.

    If no suffix is set, suffix() returns an empty string.

    \sa prefix(), setPrefix(), specialValueText(), setSpecialValueText()
*/

QString QSpinBox::suffix() const
{
    Q_D(const QSpinBox);

    return d->suffix;
}

void QSpinBox::setSuffix(const QString &s)
{
    Q_D(QSpinBox);

    d->suffix = s;
    d->update();
}

/*!
    \property QSpinBox::cleanText

    \brief the text of the spin box excluding any prefix, suffix,
    or leading or trailing whitespace.

    \sa text, QSpinBox::prefix, QSpinBox::suffix
*/

QString QSpinBox::cleanText() const
{
    Q_D(const QSpinBox);

    if (d->dirty)
        d->updateEdit();

    return d->stripped(d->edit->displayText());
}


/*!
    \property QSpinBox::singleStep
    \brief the step value

    When the user uses the arrows to change the spin box's value the
    value will be incremented/decremented by the amount of the
    singleStep. The default value is 1. Setting a singleStep value of
    less than 0 does nothing.
*/

int QSpinBox::singleStep() const
{
    Q_D(const QSpinBox);

    return d->singleStep.toInt();
}

void QSpinBox::setSingleStep(int val)
{
    Q_D(QSpinBox);
    if (val >= 0) {
        d->singleStep = QVariant(val);
        d->update();
    }
}

/*!
    \property QSpinBox::minimum

    \brief the minimum value of the spin box

    When setting this property the \l maximum is adjusted
    if necessary to ensure that the range remains valid.

    The default minimum value is 0.

    \sa setRange()  specialValueText
*/

int QSpinBox::minimum() const
{
    Q_D(const QSpinBox);

    return d->minimum.toInt();
}

void QSpinBox::setMinimum(int min)
{
    Q_D(QSpinBox);
    const QVariant m(min);
    d->setRange(m, qMax(d->maximum, m));
}

/*!
    \property QSpinBox::maximum

    \brief the maximum value of the spin box

    When setting this property the \l minimum is adjusted
    if necessary, to ensure that the range remains valid.

    The default maximum value is 99.

    \sa setRange() specialValueText

*/

int QSpinBox::maximum() const
{
    Q_D(const QSpinBox);

    return d->maximum.toInt();
}

void QSpinBox::setMaximum(int max)
{
    Q_D(QSpinBox);
    const QVariant m(max);
    d->setRange(qMin(d->minimum, m), m);
}

/*!
    Convenience function to set the minimum, \a min, and maximum, \a
    max, values with a single function call.

    \code
    setRange(min, max);
    \endcode
    is equivalent to:
    \code
    setMinimum(min);
    setMaximum(max);
    \endcode

    \sa minimum maximum
*/

void QSpinBox::setRange(int min, int max)
{
    Q_D(QSpinBox);
    d->setRange(QVariant(min), QVariant(max));
}

/*!
    \fn QString QSpinBox::textFromValue(int v) const

    This virtual function is used by the spin box whenever it needs to
    display value \a v. The default implementation returns a string
    containing \a v printed in the standard way using
    QLocale().toString(v). Reimplementations may return anything. (See
    the example in the detailed description.)

    Note that Qt does not call this function for specialValueText()
    and that neither prefix() nor suffix() should be included in the
    return value.

    If you reimplement this, you may also need to reimplement
    valueFromText() and validate()

    \sa valueFromText(), validate()
*/

QString QSpinBox::textFromValue(int v) const
{
    return QString::number(v);
}

/*!
    \fn int QSpinBox::valueFromText(const QString &text) const

    This virtual function is used by the spin box whenever it needs to
    interpret text entered by the user as a value. Note that neither
    prefix() nor suffix() are included when this function is called by
    Qt.

    Subclasses that need to display spin box values in a non-numeric
    way need to reimplement this function.

    Note that Qt handles specialValueText() separately; this function
    is only concerned with the other values.

    The default implementation tries to interpret \a text as an integer
    using QString::toInt() and returns the value.

    \sa textFromValue(), validate()
*/

int QSpinBox::valueFromText(const QString &text) const
{
    Q_D(const QSpinBox);

    QString copy = text;
    int pos = d->edit->cursorPosition();
    QValidator::State state = QValidator::Acceptable;
    return d->validateAndInterpret(copy, pos, state).toInt();
}

/*!
  \reimp
*/
QValidator::State QSpinBox::validate(QString &text, int &pos) const
{
    Q_D(const QSpinBox);

    QValidator::State state;
    d->validateAndInterpret(text, pos, state);
    return state;
}


/*!
  \reimp
*/
void QSpinBox::fixup(QString &input) const
{
    Q_D(const QSpinBox);

    input.remove(d->thousand);
}


// --- QDoubleSpinBox ---

/*!
    \class QDoubleSpinBox
    \brief The QDoubleSpinBox class provides a spin box widget that
    takes doubles.

    \ingroup basic
    \mainclass

    QDoubleSpinBox allows the user to choose a value by clicking the
    up and down buttons or by pressing Up or Down on the keyboard to
    increase or decrease the value currently displayed. The user can
    also type the value in manually. If the value is entered directly
    into the spin box, the value will be changed and valueChanged()
    will be emitted with the new value when Enter or Return is
    pressed, when the spin box loses focus or when the spin box is
    deactivated (see QWidget::windowActivationChanged()). The spin box
    supports double values but can be extended to use different
    strings with validate(), textFromValue() and valueFromText().

    Every time the value changes QDoubleSpinBox emits the
    valueChanged() signal. The current value can be fetched with
    value() and set with setValue().

    Clicking the up and down buttons or using the keyboard accelerator's
    Up and Down arrows will increase or decrease the current value in
    steps of size singleStep(). If you want to change this behavior you
    can reimplement the virtual function stepBy(). The minimum and
    maximum value and the step size can be set using one of the
    constructors, and can be changed later with setMinimum(),
    setMaximum() and setSingleStep(). The spinbox has a default
    precision of 2 decimal places but this can be changed using
    setDecimals().

    Most spin boxes are directional, but QDoubleSpinBox can also
    operate as a circular spin box, i.e. if the range is 0.0-99.9 and
    the current value is 99.9, clicking "up" will give 0 if wrapping()
    is set to true. Use setWrapping() if you want circular behavior.

    The displayed value can be prepended and appended with arbitrary
    strings indicating, for example, currency or the unit of
    measurement. See setPrefix() and setSuffix(). The text in the spin
    box is retrieved with text() (which includes any prefix() and
    suffix()), or with cleanText() (which has no prefix(), no suffix()
    and no leading or trailing whitespace).

    It is often desirable to give the user a special (often default)
    choice in addition to the range of numeric values. See
    setSpecialValueText() for how to do this with QDoubleSpinBox.

    \sa QSpinBox, QSlider, {fowler}{GUI Design Handbook: Slider}
*/

/*!
    \fn void QDoubleSpinBox::valueChanged(double d);

    This signal is emitted whenever the spin box's value is changed.
    The new value is passed in \a d.
*/

/*!
    \fn void QDoubleSpinBox::valueChanged(const QString &text);

    \overload

    The new value is passed literally in \a text with no prefix() or
    suffix().
*/

/*!
    Constructs a spin box with no minimum and maximum values, a step
    value of 1.0 and a precision of 2 decimal places. The value is
    initially set to 0.00. The spin box has the given \a parent.

    \sa setMinimum(), setMaximum(), setSingleStep()
*/
QDoubleSpinBox::QDoubleSpinBox(QWidget *parent)
    : QAbstractSpinBox(*new QDoubleSpinBoxPrivate, parent)
{
}

/*!
    \property QDoubleSpinBox::value
    \brief the value of the spin box

    setValue() will emit valueChanged() if the new value is different
    from the old one.
*/
double QDoubleSpinBox::value() const
{
    Q_D(const QDoubleSpinBox);

    return d->value.toDouble();
}

void QDoubleSpinBox::setValue(double val)
{
    Q_D(QDoubleSpinBox);
    QVariant v(val);
    d->setValue(v, EmitIfChanged);
}
/*!
    \property QDoubleSpinBox::prefix
    \brief the spin box's prefix

    The prefix is prepended to the start of the displayed value.
    Typical use is to display a unit of measurement or a currency
    symbol. For example:

    \code
        spinbox->setPrefix("$");
    \endcode

    To turn off the prefix display, set this property to an empty
    string. The default is no prefix. The prefix is not displayed when
    value() == minimum() and specialValueText() is set.

    If no prefix is set, prefix() returns an empty string.

    \sa suffix(), setSuffix(), specialValueText(), setSpecialValueText()
*/

QString QDoubleSpinBox::prefix() const
{
    Q_D(const QDoubleSpinBox);

    return d->prefix;
}

void QDoubleSpinBox::setPrefix(const QString &p)
{
    Q_D(QDoubleSpinBox);

    d->prefix = p;
    d->update();
}

/*!
    \property QDoubleSpinBox::suffix
    \brief the suffix of the spin box

    The suffix is appended to the end of the displayed value. Typical
    use is to display a unit of measurement or a currency symbol. For
    example:

    \code
        spinbox->setSuffix(" km");
    \endcode

    To turn off the suffix display, set this property to an empty
    string. The default is no suffix. The suffix is not displayed for
    the minimum() if specialValueText() is set.

    If no suffix is set, suffix() returns an empty string.

    \sa prefix(), setPrefix(), specialValueText(), setSpecialValueText()
*/

QString QDoubleSpinBox::suffix() const
{
    Q_D(const QDoubleSpinBox);

    return d->suffix;
}

void QDoubleSpinBox::setSuffix(const QString &s)
{
    Q_D(QDoubleSpinBox);

    d->suffix = s;
    d->update();
}

/*!
    \property QDoubleSpinBox::cleanText

    \brief the text of the spin box excluding any prefix, suffix,
    or leading or trailing whitespace.

    \sa text, QDoubleSpinBox::prefix, QDoubleSpinBox::suffix
*/

QString QDoubleSpinBox::cleanText() const
{
    Q_D(const QDoubleSpinBox);

    if (d->dirty)
        d->updateEdit();

    return d->stripped(d->edit->displayText());
}

/*!
    \property QDoubleSpinBox::singleStep
    \brief the step value

    When the user uses the arrows to change the spin box's value the
    value will be incremented/decremented by the amount of the
    singleStep. The default value is 1.0. Setting a singleStep value
    of less than 0 does nothing.
*/
double QDoubleSpinBox::singleStep() const
{
    Q_D(const QDoubleSpinBox);

    return d->singleStep.toDouble();
}

void QDoubleSpinBox::setSingleStep(double val)
{
    Q_D(QDoubleSpinBox);

    if (val >= 0) {
        d->singleStep = val;
        d->update();
    }
}

/*!
    \property QDoubleSpinBox::minimum

    \brief the minimum value of the spin box

    When setting this property the \l maximum is adjusted
    if necessary to ensure that the range remains valid.

    The default minimum value is 0.0.

    \sa setRange() specialValueText
*/

double QDoubleSpinBox::minimum() const
{
    Q_D(const QDoubleSpinBox);

    return d->minimum.toDouble();
}

void QDoubleSpinBox::setMinimum(double min)
{
    Q_D(QDoubleSpinBox);
    const QVariant m(min);
    d->setRange(m, qMax(d->maximum, m));
}

/*!
    \property QDoubleSpinBox::maximum

    \brief the maximum value of the spin box

    When setting this property the \l minimum is adjusted
    if necessary, to ensure that the range remains valid.

    The default maximum value is 99.99.

    \sa setRange()
*/

double QDoubleSpinBox::maximum() const
{
    Q_D(const QDoubleSpinBox);

    return d->maximum.toDouble();
}

void QDoubleSpinBox::setMaximum(double max)
{
    Q_D(QDoubleSpinBox);
    const QVariant m(max);
    d->setRange(qMin(d->minimum, m), m);
}

/*!
    Convenience function to set the minimum, \a min, and maximum, \a
    max, values with a single function call.

    \code
    setRange(min, max);
    \endcode
    is equivalent to:
    \code
    setMinimum(min);
    setMaximum(max);
    \endcode

    \sa minimum maximum
*/

void QDoubleSpinBox::setRange(double min, double max)
{
    Q_D(QDoubleSpinBox);
    d->setRange(QVariant(min), QVariant(max));
}

/*!
     \property QDoubleSpinBox::decimals

     \brief the precision of the spin box, in decimals

     Sets how many decimals you want to display when displaying double
     values. Valid ranges for decimals is 0-14. The default is 2.


*/

int QDoubleSpinBox::decimals() const
{
    Q_D(const QDoubleSpinBox);

    return d->decimals;
}

void QDoubleSpinBox::setDecimals(int decimals)
{
    Q_D(QDoubleSpinBox);

    d->decimals = qMin<int>(qMax<int>(0, decimals), 14);
    if (d->decimals != decimals)
        qWarning("QDoubleSpinBox::setDecimals() %d is not a valid precision. 0-14 is allowed",
                 decimals);
    // more than fifteen seems to cause problems in QLocale::doubleToString
    d->update();
}

/*!
    \fn QString QDoubleSpinBox::textFromValue(double v) const

    This virtual function is used by the spin box whenever it needs to
    display value \a v. The default implementation returns a string
    containing \a v printed using QLocale().toString(v, QLatin1Char('f'),
    decimals()). Reimplementations may return anything.

    Note that Qt does not call this function for specialValueText()
    and that neither prefix() nor suffix() should be included in the
    return value.

    If you reimplement this, you may also need to reimplement
    valueFromText().

    \sa valueFromText()
*/


QString QDoubleSpinBox::textFromValue(double v) const
{
    Q_D(const QDoubleSpinBox);

    return QString::number(v, 'f', d->decimals);
}

/*!
    This virtual function is used by the spin box whenever it needs to
    interpret \a text entered by the user as a value. Note that neither
    prefix() nor suffix() are included when this function is called by
    Qt.

    Subclasses that need to display spin box values in a non-numeric
    way need to reimplement this function.

    Note that Qt handles specialValueText() separately; this function
    is only concerned with the other values.

    \sa textFromValue(), validate()
*/
double QDoubleSpinBox::valueFromText(const QString &text) const
{
    Q_D(const QDoubleSpinBox);

    QString copy = text;
    int pos = d->edit->cursorPosition();
    QValidator::State state = QValidator::Acceptable;
    return d->validateAndInterpret(copy, pos, state).toDouble();
}

/*!
  \reimp
*/
QValidator::State QDoubleSpinBox::validate(QString &text, int &pos) const
{
    Q_D(const QDoubleSpinBox);

    QValidator::State state;
    d->validateAndInterpret(text, pos, state);
    return state;
}


/*!
  \reimp
*/
void QDoubleSpinBox::fixup(QString &input) const
{
    Q_D(const QDoubleSpinBox);

    if (d->thousand != dot && d->delimiter != dot && input.count(dot) == 1)
        input.replace(dot, d->delimiter);

    input.remove(d->thousand);
}

// --- QSpinBoxPrivate ---

/*!
    \internal
    Constructs a QSpinBoxPrivate object
*/

QSpinBoxPrivate::QSpinBoxPrivate()
{
    minimum = QVariant((int)0);
    maximum = QVariant((int)99);
    value = minimum;
    singleStep = QVariant((int)1);
    type = QVariant::Int;
    const QString str = QLocale().toString(4567);
    if (str.size() == 5) {
        thousand = QChar(str.at(1));
        if (thousand.isSpace())
            thousand = QLatin1Char(' '); // to avoid problems with 0xA0
    }

}

/*!
    \internal
    \reimp
*/

void QSpinBoxPrivate::emitSignals(EmitPolicy ep, const QVariant &old)
{
    Q_Q(QSpinBox);
    if (ep != NeverEmit) {
        pendingEmit = false;
        if (ep == AlwaysEmit || value != old) {
            emit q->valueChanged(edit->displayText());
            emit q->valueChanged(value.toInt());
        }
    }
}

/*!
    \internal
    \reimp
*/

QString QSpinBoxPrivate::textFromValue(const QVariant &f) const
{
    Q_Q(const QSpinBox);
    return q->textFromValue(f.toInt());
}
/*!
    \internal
    \reimp
*/

QVariant QSpinBoxPrivate::valueFromText(const QString &f) const
{
    Q_Q(const QSpinBox);

    return QVariant(q->valueFromText(f));
}


/*!
  \internal Return true if str can become a number which is between min
  and max or false if this is not possible.
  */

bool QSpinBoxPrivate::isIntermediateValue(const QString &str) const
{
    const int num = QLocale().toInt(str, 0, 10);
    const int min = minimum.toInt();
    const int max = maximum.toInt();

    int numDigits = 0;
    int digits[10];
    int tmp = num;
    if (tmp == 0) {
        numDigits = 1;
        digits[0] = 0;
    } else {
        tmp = num;
        for (int i=0; tmp != 0; ++i) {
            digits[numDigits++] = qAbs(tmp % 10);
            tmp /= 10;
        }
    }

    int failures = 0;
    for (int number=min; /*number<=max*/; ++number) {
        tmp = number;
        for (int i=0; tmp != 0;) {
            if (digits[i] == qAbs(tmp % 10)) {
                if (++i == numDigits)
                    return true;
            }
            tmp /= 10;
        }
        if (failures++ == 500000) //upper bound
            return true;
        if (number == max) // needed for INT_MAX
            break;
    }
    return false;
}

/*!
    \internal Multi purpose function that parses input, sets state to
    the appropriate state and returns the value it will be interpreted
    as.
*/

QVariant QSpinBoxPrivate::validateAndInterpret(QString &input, int &,
                                               QValidator::State &state) const
{
    if (cachedText == input && !input.isEmpty()) {
        state = cachedState;
        QSBDEBUG() << "cachedText was" << "'" + cachedText + "'" << "state was "
                   << state << " and value was " << cachedValue;

        return cachedValue;
    }
    const int max = maximum.toInt();
    const int min = minimum.toInt();

    QString copy = stripped(input);
    QSBDEBUG() << "input" << input << "copy" << copy;
    state = QValidator::Acceptable;
    int num = min;

    if (max != min && (copy.isEmpty()
                       || (min < 0 && copy == QLatin1String("-"))
                       || (min >= 0 && copy == QLatin1String("+")))) {
        state = QValidator::Intermediate;
        QSBDEBUG() << __FILE__ << __LINE__<< "num is set to" << num;
    } else {
        bool ok = false;
        bool removedThousand = false;
        num = QLocale().toInt(copy, &ok, 10);
        if (!ok && copy.contains(thousand) && (max >= 1000 || min <= -1000)) {
            copy.remove(thousand);
            removedThousand = true;
            num = QLocale().toInt(copy, &ok, 10);
        }
        QSBDEBUG() << __FILE__ << __LINE__<< "num is set to" << num;
        if (!ok) {
            state = QValidator::Invalid;
        } else if (num >= min && num <= max) {
            state = removedThousand ? QValidator::Intermediate : QValidator::Acceptable;
        } else if (max == min) {
            state = QValidator::Invalid;
        } else {
            if ((num >= 0 && num > max) || (num < 0 && num < min)) {
                state = QValidator::Invalid;
                QSBDEBUG() << __FILE__ << __LINE__<< "state is set to Invalid";
            } else {
                state = isIntermediateValue(copy) ? QValidator::Intermediate : QValidator::Invalid;
                QSBDEBUG() << __FILE__ << __LINE__<< "state is set to "
                           << (state == QValidator::Intermediate ? "Intermediate" : "Acceptable");
            }
        }
    }
    if (state != QValidator::Acceptable)
        num = max > 0 ? min : max;
    cachedText = input;
    cachedState = state;
    cachedValue = QVariant((int)num);

    QSBDEBUG() << "cachedText is set to '" << cachedText << "' state is set to "
               << state << " and value is set to " << cachedValue;
    return cachedValue;
}

// --- QDoubleSpinBoxPrivate ---

/*!
    \internal
    Constructs a QSpinBoxPrivate object
*/

QDoubleSpinBoxPrivate::QDoubleSpinBoxPrivate()
{
    minimum = QVariant(0.0);
    maximum = QVariant(99.99);
    value = minimum;
    singleStep = QVariant(1.0);
    decimals = 2;
    type = QVariant::Double;
    const QString str = QLocale().toString(4567.1);
    if (str.size() == 6) {
        delimiter = str.at(4);
        thousand = QChar((ushort)0);
    } else if (str.size() == 7) {
        thousand = str.at(1);
        if (thousand.isSpace())
            thousand = QLatin1Char(' '); // to avoid problems with 0xA0
        delimiter = str.at(5);
    }
    Q_ASSERT(!delimiter.isNull());
}

/*!
    \internal
    \reimp
*/

void QDoubleSpinBoxPrivate::emitSignals(EmitPolicy ep, const QVariant &old)
{
    Q_Q(QDoubleSpinBox);
    if (ep != NeverEmit) {
        pendingEmit = false;
        if (ep == AlwaysEmit || value != old) {
            emit q->valueChanged(edit->displayText());
            emit q->valueChanged(value.toDouble());
        }
    }
}


bool QDoubleSpinBoxPrivate::isIntermediateValue(const QString &str) const
{
    QSBDEBUG() << "input is" << str << minimum << maximum;
    qint64 dec = 1;
    for (int i=0; i<decimals; ++i)
        dec *= 10;

    // I know QString::number() uses CLocale so I use dot
    const QString minstr = QString::number(minimum.toDouble(), 'f', decimals);
    qint64 min_left = minstr.left(minstr.indexOf(dot)).toLongLong();
    qint64 min_right = minstr.mid(minstr.indexOf(dot) + 1).toLongLong();

    const QString maxstr = QString::number(maximum.toDouble(), 'f', decimals);
    qint64 max_left = maxstr.left(maxstr.indexOf(dot)).toLongLong();
    qint64 max_right = maxstr.mid(maxstr.indexOf(dot) + 1).toLongLong();

    const int dot = str.indexOf(delimiter);
    const bool negative = maximum.toDouble() < 0;
    qint64 left = 0, right = 0;
    bool doleft = true;
    bool doright = true;
    if (dot == -1) {
        left = str.toLongLong();
        doright = false;
    } else if (dot == 0 || (dot == 1 && str.at(0) == '+')) {
        if (negative) {
            QSBDEBUG() << __FILE__ << __LINE__ << "returns false";
            return false;
        }
        doleft = false;
        right = str.mid(dot + 1).toLongLong();
    } else if (dot == 1 && str.at(0) == '-') {
        if (!negative) {
            QSBDEBUG() << __FILE__ << __LINE__ << "returns false";
            return false;
        }
        doleft = false;
        right = str.mid(dot + 1).toLongLong();
    } else {
        left = str.left(dot).toLongLong();
        if (dot == str.size() - 1) {
            doright = false;
        } else {
            right = str.mid(dot + 1).toLongLong();
        }
    }
    if ((left >= 0 && max_left < 0 && !str.startsWith('-')) || (left < 0 && min_left >= 0)) {
        QSBDEBUG("returns false 0");
        return false;
    }

    qint64 match = min_left;
    if (doleft && !isIntermediateValueHelper(left, min_left, max_left, &match)) {
        QSBDEBUG() << __FILE__ << __LINE__ << "returns false";
        return false;
    }
    if (doright) {
        QSBDEBUG("match %lld min_left %lld max_left %lld", match, min_left, max_left);
        if (!doleft) {
            if (min_left == max_left) {
                const bool ret = isIntermediateValueHelper(qAbs(left),
                                                           negative ? max_right : min_right,
                                                           negative ? min_right : max_right);
                QSBDEBUG() << __FILE__ << __LINE__ << "returns" << ret;
                return ret;
            } else if (qAbs(max_left - min_left) == 1) {
                const bool ret = isIntermediateValueHelper(qAbs(left), min_right, negative ? 0 : dec)
                                 || isIntermediateValueHelper(qAbs(left), negative ? dec : 0, max_right);
                QSBDEBUG() << __FILE__ << __LINE__ << "returns" << ret;
                return ret;
            } else {
                const bool ret = isIntermediateValueHelper(qAbs(left), 0, dec);
                QSBDEBUG() << __FILE__ << __LINE__ << "returns" << ret;
                return ret;
            }
        }
        if (match != min_left) {
            min_right = negative ? dec : 0;
        }
        if (match != max_left) {
            max_right = negative ? 0 : dec;
        }
        qint64 tmpl = negative ? max_right : min_right;
        qint64 tmpr = negative ? min_right : max_right;
        const bool ret = isIntermediateValueHelper(right, tmpl, tmpr);
        QSBDEBUG() << __FILE__ << __LINE__ << "returns" << ret;
        return ret;
    }
    QSBDEBUG() << __FILE__ << __LINE__ << "returns true";
    return true;
}


/*
  \internal Tries to find the decimal separator. If it can't find it
  and the thousand delimiter is != '.' it will try to find a '.';
*/

int QDoubleSpinBoxPrivate::findDelimiter(const QString &str, int index) const
{
    int dotindex = str.indexOf(delimiter, index);
    if (dotindex == -1 && thousand != dot && delimiter != dot)
        dotindex = str.indexOf(dot, index);
    return dotindex;
}

/*!
    \internal
    \reimp
*/
QVariant QDoubleSpinBoxPrivate::valueFromText(const QString &f) const
{
    Q_Q(const QDoubleSpinBox);
    return QVariant(q->valueFromText(f));
}

/*!
    \internal Multi purpose function that parses input, sets state to
    the appropriate state and returns the value it will be interpreted
    as.
*/

QVariant QDoubleSpinBoxPrivate::validateAndInterpret(QString &input, int &,
                                                     QValidator::State &state) const
{
    if (cachedText == input && !input.isEmpty()) {
        state = cachedState;
        QSBDEBUG() << "cachedText was" << "'" + cachedText + "'" << "state was "
                   << state << " and value was " << cachedValue;
        return cachedValue;
    }
    const double max = maximum.toDouble();
    const double min = minimum.toDouble();
    QString copy = stripped(input);
    QSBDEBUG() << "input" << input << "copy" << copy;
    int len = copy.size();
    double num = min;
    const bool plus = max >= 0;
    const bool minus = min <= 0;

    switch (len) {
    case 0:
        state = max != min ? QValidator::Intermediate : QValidator::Invalid;
        goto end;
    case 1:
        if (copy.at(0) == delimiter
            || (thousand != dot && copy.at(0) == dot)
            || (plus && copy.at(0) == QLatin1Char('+'))
            || (minus && copy.at(0) == QLatin1Char('-'))) {
            state = QValidator::Intermediate;
            goto end;
        }
        break;
    case 2:
        if ((copy.at(1) == delimiter || (thousand != dot && copy.at(1) == dot))
            && ((plus && copy.at(0) == QLatin1Char('+')) || (minus && copy.at(0) == QLatin1Char('-')))) {
            state = QValidator::Intermediate;
            goto end;
        }
        break;
    default: break;
    }

    if (copy.at(0).isSpace() || copy.at(0) == thousand) {
        QSBDEBUG() << __FILE__ << __LINE__<< "state is set to Invalid";
        state = QValidator::Invalid;
        goto end;
    } else if (len > 1) {
        const int dec = findDelimiter(copy);
        if (dec != -1) {
            if (copy.size() - dec > decimals + 1) {
                QSBDEBUG() << __FILE__ << __LINE__<< "state is set to Invalid";
                state = QValidator::Invalid;
                goto end;
            }
            for (int i=dec + 1; i<copy.size(); ++i) {
                if (copy.at(i).isSpace() || copy.at(i) == thousand) {
                    QSBDEBUG() << __FILE__ << __LINE__<< "state is set to Invalid";
                    state = QValidator::Invalid;
                    goto end;
                }
            }
        } else {
            const QChar &last = copy.at(len - 1);
            const QChar &secondLast = copy.at(len - 2);
            if ((last == thousand || last.isSpace())
                && (secondLast == thousand || secondLast.isSpace())) {
                state = QValidator::Invalid;
                QSBDEBUG() << __FILE__ << __LINE__<< "state is set to Invalid";
                goto end;
            } else if (last.isSpace() && (!thousand.isSpace() || secondLast.isSpace())) {
                state = QValidator::Invalid;
                QSBDEBUG() << __FILE__ << __LINE__<< "state is set to Invalid";
                goto end;
            }
        }
    }

    {
        bool ok = false;
        QLocale loc;
        num = loc.toDouble(copy, &ok);
        bool notAcceptable = false;

        if (!ok) {
            bool tryAgain = false;
            if (thousand != dot && delimiter != dot && copy.count(dot) == 1) {
                copy.replace(dot, delimiter);
                tryAgain = true;
            }

            if (thousand.isPrint()) {
                if (max < 1000 && min > -1000 && copy.contains(thousand)) {
                    state = QValidator::Invalid;
                    QSBDEBUG() << __FILE__ << __LINE__<< "state is set to Invalid";
                    goto end;
                }

                const int len = copy.size();
                for (int i=0; i<len- 1; ++i) {
                    if (copy.at(i) == thousand && copy.at(i + 1) == thousand) {
                        QSBDEBUG() << __FILE__ << __LINE__<< "state is set to Invalid";
                        state = QValidator::Invalid;
                        goto end;
                    }
                }

                copy.remove(thousand);
                tryAgain = tryAgain || len != copy.size();
            }

            if (tryAgain) {
                num = loc.toDouble(copy, &ok);
                if (!ok) {
                    state = QValidator::Invalid;
                    QSBDEBUG() << __FILE__ << __LINE__<< "state is set to Invalid";
                    goto end;
                }
                notAcceptable = true;
            }
        }

        if (!ok) {
            state = QValidator::Invalid;
            QSBDEBUG() << __FILE__ << __LINE__<< "state is set to Invalid";
        } else if (num >= min && num <= max) {
            state = notAcceptable ? QValidator::Intermediate : QValidator::Acceptable;
            QSBDEBUG() << __FILE__ << __LINE__<< "state is set to "
                       << (state == QValidator::Intermediate ? "Intermediate" : "Acceptable");
        } else if (max == min) { // when max and min is the same the only non-Invalid input is max (or min)
            state = QValidator::Invalid;
            QSBDEBUG() << __FILE__ << __LINE__<< "state is set to Invalid";
        } else {
            if ((num >= 0 && num > max) || (num < 0 && num < min)) {
                state = QValidator::Invalid;
                QSBDEBUG() << __FILE__ << __LINE__<< "state is set to Invalid";
            } else {
                state = isIntermediateValue(copy) ? QValidator::Intermediate : QValidator::Invalid;
                QSBDEBUG() << __FILE__ << __LINE__<< "state is set to "
                           << (state == QValidator::Intermediate ? "Intermediate" : "Acceptable");
            }
        }
    }

end:
    if (state != QValidator::Acceptable) {
        num = max > 0 ? min : max;
    }


    cachedText = input;
    cachedState = state;
    cachedValue = QVariant(num);
    return QVariant(num);
}

/*
    \internal
    \reimp
*/

QString QDoubleSpinBoxPrivate::textFromValue(const QVariant &f) const
{
    Q_Q(const QDoubleSpinBox);
    return q->textFromValue(f.toDouble());
}

/*!
    \fn void QSpinBox::setLineStep(int step)

    Use setSingleStep() instead.
*/

/*!
    \fn void QSpinBox::setMaxValue(int val)

    Use setMaximum() instead.
*/

/*!
    \fn void QSpinBox::setMinValue(int val)

    Use setMinimum() instead.
*/

/*!
    \fn int QSpinBox::maxValue() const

    Use maximum() instead.
*/

/*!
    \fn int QSpinBox::minValue() const

    Use minimum() instead.
*/

/*!
    \internal Returns whether \a str is a string which value cannot be
    parsed but still might turn into something valid.
*/

static bool isIntermediateValueHelper(qint64 num, qint64 min, qint64 max, qint64 *match)
{
    QSBDEBUG("%lld %lld %lld", num, min, max);

    if (num >= min && num <= max) {
        if (match)
            *match = num;
        QSBDEBUG("returns true 0");
        return true;
    }
    qint64 tmp = num;

    int numDigits = 0;
    int digits[10];
    if (tmp == 0) {
        numDigits = 1;
        digits[0] = 0;
    } else {
        tmp = qAbs(num);
        for (int i=0; tmp > 0; ++i) {
            digits[numDigits++] = tmp % 10;
            tmp /= 10;
        }
    }

    int failures = 0;
    qint64 number;
    for (number=max; number>=min; --number) {
        tmp = qAbs(number);
        for (int i=0; tmp > 0;) {
            if (digits[i] == (tmp % 10)) {
                if (++i == numDigits) {
                    if (match)
                        *match = number;
                    QSBDEBUG("returns true 1");
                    return true;
                }
            }
            tmp /= 10;
        }
        if (failures++ == 500000) { //upper bound
            if (match)
                *match = num;
            QSBDEBUG("returns true 2");
            return true;
        }
    }
    QSBDEBUG("returns false");
    return false;
}
