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

class QSpinBoxPrivate : public QAbstractSpinBoxPrivate
{
    Q_DECLARE_PUBLIC(QSpinBox)
public:
    QSpinBoxPrivate();
    void emitSignals();
    QVariant mapTextToValue(QString *str, QValidator::State *state) const;
    QString mapValueToText(const QVariant &n) const;
};

static const QLatin1Char dot('.');
class QDoubleSpinBoxPrivate : public QAbstractSpinBoxPrivate
{
    Q_DECLARE_PUBLIC(QDoubleSpinBox)
public:
    QDoubleSpinBoxPrivate();
    void emitSignals();
    bool checkIntermediate(const QString &str) const;
    QVariant mapTextToValue(QString *str, QValidator::State *state) const;
    QString mapValueToText(const QVariant &n) const;

    QValidator::State validate(QString *input, int *pos, QVariant *val) const;
    void fixup(QString &input) const;

    int findDelimiter(const QString &str, int index = 0) const;


    // variables
    int precision;
    QChar delimiter, thousand;

};

#define d d_func()
#define q q_func()

/*!
    \class QSpinBox
    \brief The QSpinBox class provides a spin box widget.

    \ingroup basic
    \mainclass

    QSpinBox is designed to handle integers and discrete sets of
    values (such as \l{#monthexample}{month names}); use QDoubleSpinBox
    for floating point values.

    QSpinBox allows the user to choose a value by clicking the
    up/down buttons or pressing up/down on the keyboard to
    increase/decrease the value currently displayed. The user can also
    type the value in manually. If the value is entered directly into
    the spin box, the value will be changed and valueChanged() will be
    emitted with the new value when Enter/Return is pressed, when the
    spin box looses focus or when the spin box is deactivated (see
    QWidget::windowActivationChanged()) If tracking() is true the
    value will be changed each time the value is changed in the
    editor. The spin box supports integer values but can be extended
    to use different strings with mapValueToText() and
    mapTextToValue().

    Every time the value changes QSpinBox emits the valueChanged()
    signals. The current value can be fetched with value() and set
    with setValue().

    Clicking the up/down buttons or using the keyboard accelerator's
    up and down arrows will increase or decrease the current value in
    steps of size lineStep(). If you want to change this behaviour you
    can reimplement the virtual function stepBy(). The minimum and
    maximum value and the step size can be set using one of the
    constructors, and can be changed later with setMinimum(),
    setMaximum() and setLineStep().

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

    If using prefix(), suffix() and specialValueText() don't provide
    enough control, you can ignore them and subclass QSpinBox
    instead.

    Here's an example QSpinBox subclass that provides a month picker.

    \target monthexample
    \code
    class monthSpin : public QSpinBox
    {
        Q_OBJECT
    public:
        monthSpin(QWidget *parent = 0)
            : QSpinBox(0, 12, 1, parent)
        {
            setSpecialValueText("None");
            setValue(0);
        }

        QString mapValueToText(int v) const
        {
            return QDate::longMonthName(v);
        }

        int mapTextToValue(const QString &text, QValidator::State *state) const
        {
            for (int i = 1; i <= 12; ++i) {
                if (text == QDate::longMonthName(i) || text == QDate::shortMonthName(i)) {
                    if (state)
                        *state = QValidator::Acceptable;
                    return i;
                }
            }
            if (state)
                *state = QValidator::Invalid;
            return 0;
        }

        void keyPressEvent(QKeyEvent *e)
        {
            if (e->key() == Qt::Key_P) {
                stepBy(1);
                e->accept();
            } else if (e->key() == Qt::Key_N) {
                stepBy(-1);
                e->accept();
            } else {
                QSpinBox::keyPressEvent(e);
            }
        }
    \endcode

    \inlineimage macintosh-spinbox.png Screenshot in Macintosh style
    \inlineimage windows-spinbox.png Screenshot in Windows style

    \sa QDoubleSpinBox QSlider
    \link guibooks.html#fowler GUI Design Handbook: Slider\endlink
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

    \sa minimum(), setMinimum(), maximum(), setMaximum(), lineStep(),
    setLineStep()
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
    d->minimum = QVariant(qMin(min, max));
    d->maximum = QVariant(qMax(min, max));
    d->singlestep = QVariant(step);
    setObjectName(name);
}

#endif

/*!
    \property QSpinBox::value
    \brief the value of the spin box

    setValue() will emit valueChanged() if the new value is different
    from the old one.

    \sa setValue()
*/

int QSpinBox::value() const
{
    return d->value.toInt();
}

void QSpinBox::setValue(int val)
{
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
    return d->prefix;
}

void QSpinBox::setPrefix(const QString &p)
{
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
    the minValue() if specialValueText() is set.

    If no suffix is set, suffix() returns an empty string.

    \sa prefix(), setPrefix(), specialValueText(), setSpecialValueText()
*/

QString QSpinBox::suffix() const
{
    return d->suffix;
}

void QSpinBox::setSuffix(const QString &s)
{
    d->suffix = s;
    d->update();
}

/*!
    \property QSpinBox::specialValueText
    \brief the special-value text

    If set, the spin box will display this text instead of a numeric
    value whenever the current value is equal to minimum(). Typical use
    is to indicate that this choice has a special (default) meaning.

    For example, if your spin box allows the user to choose the margin
    width in a print dialog and your application is able to
    automatically choose a good margin width, you can set up the spin
    box like this:

    \code
        QSpinBox marginBox(-1, 20, 1, parent);
        marginBox.setSuffix(" mm");
        marginBox.setSpecialValueText("Auto");
    \endcode

    The user will then be able to choose a margin width from 0-20
    millimeters or select "Auto" to leave it to the application to
    choose. Your code must then interpret the spin box value of -1 as
    the user requesting automatic margin width.

    All values are displayed with the prefix() and suffix() (if set),
    \e except for the special value, which only shows the special
    value text.

    To turn off the special-value text display, call this function
    with an empty string. The default is no special-value text, i.e.
    the numeric value is shown as usual.

    If no special-value text is set, specialValueText() returns an
    empty string.
*/

QString QSpinBox::specialValueText() const
{
    return d->specialvaluetext;
}

void QSpinBox::setSpecialValueText(const QString &s)
{
    d->specialvaluetext = s;
    d->update();
}


/*!
    \property QSpinBox::singleStep
    \brief the step value

    When the user uses the arrows to change the spin box's value the
    value will be incremented/decremented by the amount of the
    singleStep. The default value is 1. Setting a singleStep value of
    less than 0 does nothing.

    \sa setSingleStep()
*/

int QSpinBox::singleStep() const
{
    return d->singlestep.toInt();
}

void QSpinBox::setSingleStep(int val)
{
    if (val >= 0) {
        d->singlestep = QVariant(val);
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
    return d->minimum.toInt();
}

void QSpinBox::setMinimum(int min)
{
    d->setBoundary(Minimum, QVariant(min));
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
    return d->maximum.toInt();
}

void QSpinBox::setMaximum(int max)
{
    d->setBoundary(Maximum, QVariant(max));
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
    d->setBoundary(Minimum, QVariant(min));
    d->setBoundary(Maximum, QVariant(max));
}

/*!
    This virtual function is used by the spin box whenever it needs to
    display value \a v. The default implementation returns a string
    containing \a v printed in the standard way. Reimplementations may
    return anything. (See the example in the detailed description.)

    Note that Qt does not call this function for specialValueText()
    and that neither prefix() nor suffix() should be included in the
    return value.

    If you reimplement this, you may also need to reimplement
    mapTextToValue().

    \sa mapTextToValue()
*/

QString QSpinBox::mapValueToText(int v) const
{
    return QLocale().toString(v);
}

/*!
    \fn int QSpinBox::mapTextToValue(QString *text, QValidator::State *state) const

    This virtual function is used by the spin box whenever it needs to
    interpret text entered by the user as a value. Note that neither
    prefix() nor suffix() are included when this function is called by
    Qt. If \a state is not 0 it is set accordingly.

    Subclasses that need to display spin box values in a non-numeric
    way need to reimplement this function.

    Note that Qt handles specialValueText() separately; this function
    is only concerned with the other values.

    The default implementation tries to interpret \a text as an integer
    using QString::toInt() and returns the value. When this function
    sets *state to Intermediate it will return the minimum value.

    \warning
    If \a state or \a text is 0 this function does nothing.

    \sa mapValueToText()
*/

int QSpinBox::mapTextToValue(QString *txt, QValidator::State *state) const
{
    const int t = d->maximum.toInt();
    const int b = d->minimum.toInt();
    if (!txt || !state) {
        if (state)
            *state = QValidator::Invalid;
        return b;
    }

    QString copy = *txt;
    d->strip(&copy);

    if (copy.isEmpty() || (b <= 0 && *txt == QLatin1String("-")) || (t >= 0 && copy == QLatin1String("+"))) {
        *state = QValidator::Intermediate;
	return b;
    }

    bool ok = false;
    int num = QLocale().toInt(copy, &ok, 10);
    if (!ok || (num < 0 && b >= 0)) {
        *state = QValidator::Invalid;
    } else if (num >= b && num <= t) {
        *state = QValidator::Acceptable;
    } else {
        if (num >= 0) {
            if (num > b) {
                *state = QValidator::Invalid;
            } else {
                *state = QValidator::Intermediate;
                num = b;
            }
        } else {
            if (num < b) {
                *state = QValidator::Invalid;
            } else {
                *state = QValidator::Intermediate;
                num = b;
            }
        }
    }

    return num;
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
    deactivated (see QWidget::windowActivationChanged()). If
    tracking() is true the value will be changed each time the value
    is changed in the editor. The spin box supports double values but
    can be extended to use different strings with mapValueToText() and
    mapTextToValue().

    Every time the value changes QDoubleSpinBox emits the
    valueChanged() signal. The current value can be fetched with
    value() and set with setValue().

    Clicking the up and down buttons or using the keyboard accelerator's
    Up and Down arrows will increase or decrease the current value in
    steps of size lineStep(). If you want to change this behavior you
    can reimplement the virtual function stepBy(). The minimum and
    maximum value and the step size can be set using one of the
    constructors, and can be changed later with setMinimum(),
    setMaximum() and setLineStep(). The spinbox has a default
    precision of 2 decimal places but this can be changed using
    setPrecision().

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

    \sa QSpinBox QSlider
    \link guibooks.html#fowler GUI Design Handbook: Slider\endlink
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

    \sa minimum(), setMinimum(), maximum(), setMaximum(), lineStep(),
    setLineStep()
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

    \sa setValue()
*/

double QDoubleSpinBox::value() const
{
    return d->value.toDouble();
}

void QDoubleSpinBox::setValue(double val)
{
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
    return d->prefix;
}

void QDoubleSpinBox::setPrefix(const QString &p)
{
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
    the minValue() if specialValueText() is set.

    If no suffix is set, suffix() returns an empty string.

    \sa prefix(), setPrefix(), specialValueText(), setSpecialValueText()
*/

QString QDoubleSpinBox::suffix() const
{
    return d->suffix;
}

void QDoubleSpinBox::setSuffix(const QString &s)
{
    d->suffix = s;
    d->update();
}

/*!
    \property QDoubleSpinBox::specialValueText
    \brief the special-value text

    If set, the spin box will display this text instead of a numeric
    value whenever the current value is equal to minimum(). Typical use
    is to indicate that this choice has a special (default) meaning.

    For example, if your spin box allows the user to choose the margin
    width in a print dialog and your application is able to
    automatically choose a good margin width, you can set up the spin
    box like this:

    \code
        QDoubleSpinBox marginBox(-1.0, 20.0, 1.0, 1, parent);
        marginBox.setSuffix(" mm");
        marginBox.setSpecialValueText("Auto");
    \endcode

    The user will then be able to choose a margin width from 0.0-20.0
    millimeters or select "Auto" to leave it to the application to
    choose. Your code must then interpret the spin box value of -1 as
    the user requesting automatic margin width.

    All values are displayed with the prefix() and suffix() (if set),
    \e except for the special value, which only shows the special
    value text.

    To turn off the special-value text display, call this function
    with an empty string. The default is no special-value text, i.e.
    the numeric value is shown as usual.

    If no special-value text is set, specialValueText() returns an
    empty string.
*/

QString QDoubleSpinBox::specialValueText() const
{
    return d->specialvaluetext;
}

void QDoubleSpinBox::setSpecialValueText(const QString &s)
{
    d->specialvaluetext = s;
    d->update();
}

/*!
    \property QDoubleSpinBox::singleStep
    \brief the step value

    When the user uses the arrows to change the spin box's value the
    value will be incremented/decremented by the amount of the
    singleStep. The default value is 1.0. Setting a singleStep value
    of less than 0 does nothing.

    \sa setSingleStep()
*/


double QDoubleSpinBox::singleStep() const
{
    return d->singlestep.toDouble();
}

void QDoubleSpinBox::setSingleStep(double val)
{
    if (val >= 0) {
        d->singlestep = val;
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
    return d->minimum.toDouble();
}

void QDoubleSpinBox::setMinimum(double min)
{
    d->setBoundary(Minimum, QVariant(min));
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
    return d->maximum.toDouble();
}

void QDoubleSpinBox::setMaximum(double max)
{
    d->setBoundary(Maximum, QVariant(max));
    d->update();
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
    d->setBoundary(Minimum, QVariant(min));
    d->setBoundary(Maximum, QVariant(max));
}

/*!
     \property QDoubleSpinBox::precision

     \brief the precision of the spin box

     The precision sets how many decimals you want to display when
     displaying double values. Valid ranges for decimals is 0-14.


*/

int QDoubleSpinBox::precision() const
{
    return d->precision;
}

void QDoubleSpinBox::setPrecision(int precision)
{
    d->precision = qMin(qMax(0, precision), 14);
    if (d->precision != precision)
	qWarning("QDoubleSpinBox::setPrecision() %d is not a valid precision. 0-14 is allowed",
		 precision);
    // more than fifteen seems to cause problems in QLocale::doubleToString
    d->update();
}

/*!
    This virtual function is used by the spin box whenever it needs to
    display value \a v. The default implementation returns a string
    containing \a v printed using QString::number(v, QLatin1Char('f'),
    precision()). Reimplementations may return anything.

    Note that Qt does not call this function for specialValueText()
    and that neither prefix() nor suffix() should be included in the
    return value.

    If you reimplement this, you may also need to reimplement
    mapTextToValue().

    \sa mapTextToValue()
*/


QString QDoubleSpinBox::mapValueToText(double v) const
{
    return QLocale().toString(v, 'f', d->precision);
}

/*!
    \fn double QDoubleSpinBox::mapTextToValue(QString *text, QValidator::State *state) const

    This virtual function is used by the spin box whenever it needs to
    interpret text entered by the user as a value. Note that neither
    prefix() nor suffix() are included when this function is called by
    Qt. If \a state is not 0 it is set accordingly.

    Subclasses that need to display spin box values in a non-numeric
    way need to reimplement this function.

    Note that Qt handles specialValueText() separately; this function
    is only concerned with the other values.

    The default implementation tries to interpret \a text as an double
    using QString::toDouble() and returns the value. When this function
    sets *state to Intermediate it will return the minimum value.

    \warning
    If \a state or \a text is 0 this function does nothing.

    \sa mapValueToText()
*/

double QDoubleSpinBox::mapTextToValue(QString *txt, QValidator::State *state) const
{
    const double t = d->maximum.toDouble();
    const double b = d->minimum.toDouble();
    if (!txt || !state) {
        if (state)
            *state = QValidator::Invalid;
        return b;
    }
    QString copy = *txt;
    d->strip(&copy);
    int len = copy.size();

    if (d->checkIntermediate(copy)) {
        *state = QValidator::Intermediate;
        return b;
    } else if (copy.at(0).isSpace() || copy.at(0) == d->thousand) {
        *state = QValidator::Invalid;
        return b;
    } else if (len > 1) {
        const int dec = d->findDelimiter(copy);
        if (dec != -1) {
            for (int i=dec + 1; i<copy.size(); ++i) {
                if (copy.at(i).isSpace() || copy.at(i) == d->thousand) {
                    *state = QValidator::Invalid;
                    return b;
                }
            }
        } else {
            const QChar &last = copy.at(len - 1);
            const QChar &secondLast = copy.at(len - 2);
            if ((last == d->thousand || last.isSpace()) && (secondLast == d->thousand || secondLast.isSpace())) {
                *state = QValidator::Invalid;
                return b;
            } else if (last.isSpace() && (!d->thousand.isSpace() || secondLast.isSpace())) {
                *state = QValidator::Invalid;
                return b;
            }
        }
    }

    bool ok = false;
    QLocale loc;
    double num = loc.toDouble(copy, &ok);
    bool notAcceptable = false;

//    qDebug() << "num" << num << "copy" << copy << "ok" << ok;

    if (!ok) {
        bool tryAgain = false;
        if (d->thousand != dot && d->delimiter != dot && copy.count(dot) == 1) {
            copy.replace(dot, d->delimiter);
            tryAgain = true;
        }

        if (d->thousand.isPrint()) {
            const int len = copy.size();
            for (int i=0; i<len- 1; ++i) {
                if (copy.at(i) == d->thousand && copy.at(i + 1) == d->thousand) {
                    *state = QValidator::Invalid;
                    return b;
                }
            }

            copy.remove(d->thousand);
            tryAgain = tryAgain || len != copy.size();
        }

        if (tryAgain) {
            num = loc.toDouble(copy, &ok);
            if (!ok) {
                *state = QValidator::Invalid;
                return b;
            }
            notAcceptable = true;
        }
    }

    if (!ok || (num < 0 && b >= 0)) {
        *state = QValidator::Invalid;
    } else if (num >= b && num <= t) {
        *state = notAcceptable ? QValidator::Intermediate : QValidator::Acceptable;

    } else {
        if (num >= 0) {
            if (num > b) {
                *state = QValidator::Invalid;
            } else {
                *state = QValidator::Intermediate;
                num = b;
            }
        } else {
            if (num < b) {
                *state = QValidator::Invalid;
            } else {
                *state = QValidator::Intermediate;
                num = b;
            }
        }
    }
    return num;
}


// --- QSpinBoxPrivate ---

/*!
    \internal
    Constructs a QSpinBoxPrivate object
*/

QSpinBoxPrivate::QSpinBoxPrivate()
{
    minimum = QVariant((int)0);
    maximum = QVariant((int)100);
    value = minimum;
    singlestep = QVariant((int)1);
    type = QVariant::Int;
}

/*!
    \internal
    \reimp
*/

void QSpinBoxPrivate::emitSignals()
{
    pendingemit = false;
    if (slider)
        updateSlider();
    emit q->valueChanged(edit->displayText());
    emit q->valueChanged(value.toInt());
}

/*!
    \internal
    \reimp
*/

QVariant QSpinBoxPrivate::mapTextToValue(QString *text, QValidator::State *state) const
{
    return q->mapTextToValue(text, state);
}

/*!
    \internal
    \reimp
*/

QString QSpinBoxPrivate::mapValueToText(const QVariant &f) const
{
    return q->mapValueToText(f.toInt());
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
    singlestep = QVariant(1.0);
    precision = 2;
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

void QDoubleSpinBoxPrivate::emitSignals()
{
    pendingemit = false;
    if (slider)
        updateSlider();
    emit q->valueChanged(edit->displayText());
    emit q->valueChanged(value.toDouble());
}
/*!
    \internal
    Returns whether \a str is a string which value cannot be parsed but still might turn into something valid.
*/

bool QDoubleSpinBoxPrivate::checkIntermediate(const QString &str) const
{
    const bool plus = d->maximum.toDouble() >= 0;
    const bool minus = d->minimum.toDouble() <= 0;
    switch (str.size()) {
    case 0: return true;
    case 1:
        if (str.at(0) == delimiter
            || (thousand != dot && str.at(0) == dot)
            || (plus && str.at(0) == QLatin1Char('+'))
            || (minus && str.at(0) == QLatin1Char('-'))) {
            return true;
        }
        return false;
    case 2:
        if ((str.at(1) == delimiter || (thousand != dot && str.at(1) == dot))
            && ((plus && str.at(0) == QLatin1Char('+')) || (minus && str.at(0) == QLatin1Char('-'))))
            return true;
        break;
    default: break;
    }

    return false;
}

void QDoubleSpinBoxPrivate::fixup(QString &input) const
{
    if (d->thousand != dot && d->delimiter != dot && input.count(dot) == 1)
        input.replace(dot, d->delimiter);

    input.remove(d->thousand);
}

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

QValidator::State QDoubleSpinBoxPrivate::validate(QString *input, int *pos, QVariant *val) const
{
    const QValidator::State ret = QAbstractSpinBoxPrivate::validate(input, pos, val);
    QString copy = *input;
    d->strip(&copy);

    if (ret != QValidator::Invalid) {
        int dotindex = findDelimiter(copy);
	if (dotindex != -1) {
	    if (precision == 0) {
                return QValidator::Invalid;
            }
            int digits = 0;
	    for (int i=dotindex+1; i<(int)copy.length(); ++i) {
		if (copy.at(i).isDigit() && ++digits > precision)
                    return QValidator::Invalid;
	    }
	}
    }


//     qDebug() << "input was: " << copy << " Input is: " << *input << " Val is: "
//              << (val ? val->toString() : QString()) << " ret is: " << ret;
    return ret;
}

/*!
    \internal
    \reimp
*/

QVariant QDoubleSpinBoxPrivate::mapTextToValue(QString *text, QValidator::State *state) const
{
    return q->mapTextToValue(text, state);
}

/*!
    \internal
    \reimp
*/

QString QDoubleSpinBoxPrivate::mapValueToText(const QVariant &f) const
{
    return q->mapValueToText(f.toDouble());
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
