#include "qabstractspinbox_p.h"
#include "qspinbox.h"
#include <qlineedit.h>
#include <qvalidator.h>

class QSpinBoxPrivate : public QAbstractSpinBoxPrivate
{
    Q_DECLARE_PUBLIC(QSpinBox);
public:
    QSpinBoxPrivate();
    void emitSignals();
    QCoreVariant mapTextToValue(QString *str, QValidator::State *state) const;
    QString mapValueToText(const QCoreVariant &n) const;
};

class QDoubleSpinBoxPrivate : public QAbstractSpinBoxPrivate
{
    Q_DECLARE_PUBLIC(QDoubleSpinBox);
public:
    QDoubleSpinBoxPrivate();
    void emitSignals();
    QCoreVariant mapTextToValue(QString *str, QValidator::State *state) const;
    QString mapValueToText(const QCoreVariant &n) const;

    QValidator::State validate(QString *input, int *pos, QCoreVariant *val) const;

    // variables
    int precision;
    static QString delimiter;
};

QString QDoubleSpinBoxPrivate::delimiter = "."; // ### this should probably come from qlocale in some way

#define d d_func()
#define q q_func()

/*!
    \class QSpinBox
    \brief The QSpinBox class provides a spin box widget.

    \ingroup basic
    \mainclass

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

            double mapTextToValue(const QString &text, QValidator::State *state) const
            {
		for (int i=1; i<=12; ++i) {
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
    d->minimum = QCoreVariant(0);
    d->maximum = QCoreVariant(99);
    d->singlestep = QCoreVariant(1);
    d->value = d->getZeroVariant();
}

#ifdef QT_COMPAT
QSpinBox::QSpinBox(QWidget *parent, const char *name)
    : QAbstractSpinBox(*new QSpinBoxPrivate, parent)
{
    d->minimum = QCoreVariant(0);
    d->maximum = QCoreVariant(99);
    d->singlestep = QCoreVariant(1);
    d->value = d->getZeroVariant();
    setObjectName(name);
}

QSpinBox::QSpinBox(int min, int max, int step, QWidget *parent, const char *name)
    : QAbstractSpinBox(*new QSpinBoxPrivate, parent)
{
    d->minimum = QCoreVariant(qMin(min, max));
    d->maximum = QCoreVariant(qMax(min, max));
    d->singlestep = QCoreVariant(step);
    d->value = d->minimum;
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
    d->setValue(QCoreVariant(val), EmitIfChanged);
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

    If no prefix is set, prefix() returns an empty QString.

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

    If no suffix is set, suffix() returns an empty QString.

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
    empty QString.
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
        d->singlestep = QCoreVariant(val);
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
    d->setBoundary(Minimum, QCoreVariant(min));
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
    d->setBoundary(Maximum, QCoreVariant(max));
}

/*!
    Convenience function to set minimum and maximum values with one
    function call.

    setRange(min, max);

       is analogous to:

    setMinimum(min);
    setMaximum(max);

    \sa minimum maximum
*/

void QSpinBox::setRange(int min, int max)
{
    d->setBoundary(Minimum, QCoreVariant(qMin(min,max)));
    d->setBoundary(Maximum, QCoreVariant(qMax(min,max)));
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
    return QString::number(v);
}

/*!
    This virtual function is used by the spin box whenever it needs to
    interpret text entered by the user as a value. Note that neither
    prefix() nor suffix() are included when this function is called by
    Qt. If \a state is not 0 it is set accordingly.

    Subclasses that need to display spin box values in a non-numeric
    way need to reimplement this function.

    Note that Qt handles specialValueText() separately; this function
    is only concerned with the other values.

    The default implementation tries to interpret \a text as an double
    in the standard way and returns the value. For an empty string or
    "-" (if negative values are allowed in the spinbox) it returns 0.

    \sa mapValueToText()
*/

int QSpinBox::mapTextToValue(QString *txt, QValidator::State *state) const
{
    const int t = d->maximum.toInt();
    const int b = d->minimum.toInt();
    if (txt->isEmpty() || (b < 0 && *txt == "-")) {
	if (state)
	    *state = QValidator::Intermediate;
	return 0;
    }

    bool ok = false;
    const int num = txt->toInt(&ok);
    if (state) {
        if (!ok || (num < 0 && b >= 0)) {
            *state = QValidator::Invalid;
        } else if (num >= b && num <= t) {
            *state = QValidator::Acceptable;
        } else {
            if (num >= 0) {
                *state = (num > b) ? QValidator::Invalid : QValidator::Intermediate;
            } else {
                *state = (num < b) ? QValidator::Invalid : QValidator::Intermediate;
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
    up/down buttons or pressing up/down on the keyboard to
    increase/decrease the value currently displayed. The user can also
    type the value in manually. If the value is entered directly into
    the spin box, the value will be changed and valueChanged() will be
    emitted with the new value when Enter/Return is pressed, when the
    spin box looses focus or when the spin box is deactivated (see
    QWidget::windowActivationChanged()). If tracking() is true the
    value will be changed each time the value is changed in the
    editor. The spin box supports double values but can be extended to
    use different strings with mapValueToText() and mapTextToValue().

    Every time the value changes QDoubleSpinBox emits the
    valueChanged() signals. The current value can be fetched with
    value() and set with setValue().

    Clicking the up/down buttons or using the keyboard accelerator's
    up and down arrows will increase or decrease the current value in
    steps of size lineStep(). If you want to change this behaviour you
    can reimplement the virtual function stepBy(). The minimum and
    maximum value and the step size can be set using one of the
    constructors, and can be changed later with setMinimum(),
    setMaximum() and setLineStep(). The spinbox has a default
    precision of 2 but this can be changed using setPrecision().

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
*/

/*!
    Constructs a spin box with no minimum and maximum values, a step
    value of 1.0 and a precision of 2. The value is initially set to
    0.0 . It is parented to \a parent.

    \sa minimum(), setMinimum(), maximum(), setMaximum(), lineStep(),
    setLineStep()
*/


QDoubleSpinBox::QDoubleSpinBox(QWidget *parent)
    : QAbstractSpinBox(*new QDoubleSpinBoxPrivate, parent)
{
    d->minimum = QCoreVariant(0.0);
    d->maximum = QCoreVariant(99.99);
    d->singlestep = QCoreVariant(1.0);
    d->precision = 2;
    d->value = d->getZeroVariant();
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
    QCoreVariant v(val);
    d->setValue(v, EmitIfChanged);
}
/*!
    \property QDoubleSpinBox::prefix
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

    If no prefix is set, prefix() returns an empty QString.

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
        sb->setSuffix(" km");
    \endcode

    To turn off the suffix display, set this property to an empty
    string. The default is no suffix. The suffix is not displayed for
    the minValue() if specialValueText() is set.

    If no suffix is set, suffix() returns an empty QString.

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
    empty QString.
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
    d->setBoundary(Minimum, QCoreVariant(min));
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
    d->setBoundary(Maximum, QCoreVariant(max));
    d->update();
}

/*!
    Convenience function to set minimum and maximum values with one
    function call.

    setRange(min, max);

       is analogous to:

    setMinimum(min);
    setMaximum(max);

    \sa minimum maximum
*/

void QDoubleSpinBox::setRange(double min, double max)
{
    d->setBoundary(Minimum, QCoreVariant(qMin(min,max)));
    d->setBoundary(Maximum, QCoreVariant(qMax(min,max)));
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
#ifdef QT_CHECK_RANGE
    if (d->precision != precision)
	qWarning("QDoubleSpinBox::setPrecision() %d is not a valid precision. 0-14 is allowed",
		 precision);
    // more than fifteen seems to cause problems in QLocale::doubleToString
#endif
    d->update();
}

/*!
    This virtual function is used by the spin box whenever it needs to
    display value \a v. The default implementation returns a string
    containing \a v printed using QString::number(v, 'f',
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
    return QString::number(v, 'f', d->precision);
}

/*!
    This virtual function is used by the spin box whenever it needs to
    interpret text entered by the user as a value. Note that neither
    prefix() nor suffix() are included when this function is called by
    Qt. If \a state is not 0 it is set accordingly.

    Subclasses that need to display spin box values in a non-numeric
    way need to reimplement this function.

    Note that Qt handles specialValueText() separately; this function
    is only concerned with the other values.

    The default implementation tries to interpret \a text as an double
    in the standard way and returns the value. For an empty string or
    "-" (if negative values are allowed in the spinbox) it returns 0.

    \sa mapValueToText()
*/

double QDoubleSpinBox::mapTextToValue(QString *txt, QValidator::State *state) const
{
    const double t = d->maximum.toDouble();
    const double b = d->minimum.toDouble();
    if (txt->isEmpty() || (b < 0 && *txt == "-")) {
	if (state)
	    *state = QValidator::Intermediate;
	return 0;
    }
    bool ok = false;
    const double num = txt->toDouble(&ok);
    if (state) {
        if (!ok || (num < 0 && b >= 0)) {
            *state = QValidator::Invalid;
        } else if (num >= b && num <= t) {
            *state = QValidator::Acceptable;
        } else {
            if (num >= 0) {
                *state = (num > b) ? QValidator::Invalid : QValidator::Intermediate;
            } else {
                *state = (num < b) ? QValidator::Invalid : QValidator::Intermediate;
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
    useprivate = true;
    type = QCoreVariant::Int;
    singlestep = QCoreVariant((int)1);
    maximum = minimum = QCoreVariant((int)0);
}

/*!
    \internal
    \reimp
*/

void QSpinBoxPrivate::emitSignals()
{
    QAbstractSpinBoxPrivate::emitSignals();
    emit q->valueChanged(edit->displayText());
    emit q->valueChanged(value.toInt());
}

/*!
    \internal
    \reimp
*/

QCoreVariant QSpinBoxPrivate::mapTextToValue(QString *text, QValidator::State *state) const
{
    Q_ASSERT(text);
    strip(text);
    QCoreVariant ret(q->mapTextToValue(text, state));
    return ret;
}

/*!
    \internal
    \reimp
*/

QString QSpinBoxPrivate::mapValueToText(const QCoreVariant &f) const
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
    useprivate = true;
    type = QCoreVariant::Double;
}

/*!
    \internal
    \reimp
*/

void QDoubleSpinBoxPrivate::emitSignals()
{
    pendingemit = false;
    emit q->valueChanged(edit->displayText());
    emit q->valueChanged(value.toDouble());
}

/*!
    \internal
    \reimp
*/

QValidator::State QDoubleSpinBoxPrivate::validate(QString *input, int *pos, QCoreVariant *val) const
{
    const QValidator::State ret = QAbstractSpinBoxPrivate::validate(input, pos, val);
    if (ret == QValidator::Acceptable) {
	int dot = input->indexOf(delimiter);
	if (dot != -1) {
	    if (precision == 0) {
		return QValidator::Invalid;
	    }
	    for (int i=dot+1; i<(int)input->length() - suffix.length(); ++i) {
		if ((!input->at(i).isDigit()) || i - (dot+1) >= precision) {
		    return QValidator::Invalid;
		}
	    }
	}
    }

    return ret;
}

/*!
    \internal
    \reimp
*/

QCoreVariant QDoubleSpinBoxPrivate::mapTextToValue(QString *text, QValidator::State *state) const
{
    Q_ASSERT(text);
    strip(text);
    QCoreVariant ret(q->mapTextToValue(text, state));
    return ret;
}

/*!
    \internal
    \reimp
*/

QString QDoubleSpinBoxPrivate::mapValueToText(const QCoreVariant &f) const
{
    return q->mapValueToText(f.toDouble());
}

