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
#include <qabstractspinbox.h>
#include <qapplication.h>
#include <qdatetimeedit.h>
#include <qlineedit.h>
#include <qevent.h>
#include <math.h>

#ifdef Q_WS_MAC
#include <private/qt_mac_p.h>
extern QString qt_mac_from_pascal_string(const Str255); //qglobal.cpp
#endif

//#define QDATETIMEEDIT_QDTEDEBUG
#ifdef QDATETIMEEDIT_QDTEDEBUG
#  define QDTEDEBUG qDebug() << QString("%1:%2").arg(__FILE__).arg(__LINE__)
#  define QDTEDEBUGN qDebug
#else
#  define QDTEDEBUG if (false) qDebug()
#  define QDTEDEBUGN if (false) qDebug
#endif
#include <qdebug.h>
static const QChar space = QLatin1Char(' ');

class QDateTimeEditPrivate : public QAbstractSpinBoxPrivate
{
    Q_DECLARE_PUBLIC(QDateTimeEdit)
public:
    enum Section {
	NoSection = 0x0000,
	AmPmSection = 0x0001,
	MSecSection = 0x0002,
	SecondSection = 0x0004,
	MinuteSection = 0x0008,
	HourSection = 0x0010,
        TimeSectionMask = (AmPmSection|MSecSection|SecondSection|MinuteSection|HourSection),
	Internal = 0x8000,
	AmPmLowerCaseSection = AmPmSection|Internal,
	DaySection = 0x0100,
	MonthSection = 0x0200,
	YearSection = 0x0400,
	MonthShortNameSection = MonthSection|Internal,
	YearTwoDigitsSection = YearSection|Internal,
        DateSectionMask = (DaySection|MonthSection|YearSection),
        FirstSection = 0x1000|Internal,
        LastSection = 0x2000|Internal
    }; // duplicated from qdatetimeedit.h

    struct SectionNode {
	Section section;
	int pos;
    };

    QDateTimeEditPrivate();

    void readLocaleSettings();

    void calculateSizeHints() const;
    void emitSignals(EmitPolicy ep, const QVariant &old);
    QString textFromValue(const QVariant &f) const;
    QVariant valueFromText(const QString &f) const;
    QVariant validateAndInterpret(QString &input, int &, QValidator::State &state) const;
    void editorCursorPositionChanged(int lastpos, int newpos);

    QStyleOptionSpinBox styleOption() const;
    QVariant valueForPosition(int pos) const;

    void clearSection(Section s);

    static int sectionSize(Section s);
    int sectionPos(Section s) const;

    SectionNode sectionNode(Section t) const;
    QVariant stepBy(Section s, int steps, bool test = false) const;
    QString sectionText(const QString &text, Section s) const;
    int getDigit(const QVariant &dt, Section s) const;
    void setDigit(QVariant &t, Section s, int newval) const;
    int sectionValue(Section s, QString &txt, QValidator::State &state) const;
    int absoluteMax(Section s) const;
    int absoluteMin(Section s) const;
    Section sectionAt(int index) const;
    Section closestSection(int index, bool forward) const;
    SectionNode nextPrevSection(Section current, bool forward) const;
    bool addSection(QList<SectionNode> &list, Section ds, int pos);
    bool parseFormat(const QString &format);
    void setSelected(Section s, bool forward = false);
    QValidator::State checkIntermediate(const QDateTime &dt, const QString &str) const;
    static int findMonth(const QString &str1, int index = 1);
    static int maxChange(QDateTimeEditPrivate::Section s);
    static int potentialValue(const QString &str, int min, int max, Section s);
    static int potentialValueHelper(const QString &str, int min, int max, int length);
    static int multiplier(Section s);

    static QString sectionName(int s);
    static QString stateName(int s);

    QString displayFormat;
    QString defaultDateFormat, defaultTimeFormat, defaultDateTimeFormat;
    QString escapedFormat;
    QList<SectionNode> sections;
    SectionNode first, last;
    QStringList separators;
    QDateTimeEdit::Sections display;
    mutable int cachedday;
    mutable Section currentsection;
    Section oldcurrentsection;
    mutable QVariant cached;
    mutable QString cachedText;
};

#define d d_func()
#define q q_func()

// --- QDateTimeEdit ---

/*!
    \class QDateTimeEdit qdatetimeedit.h
    \brief The QDateTimeEdit class provides a widget for editing dates and times.

    \ingroup basic
    \mainclass

    QDateTimeEdit allows the user to edit dates by using the keyboard or
    the arrow keys to increase and decrease date and time values. The
    arrow keys can be used to move from section to section within the
    QDateTimeEdit box. Dates and times appear in accordance with the
    format set; see setDisplayFormat().

    \code
    QDateTimeEdit *dateEdit = new QDateTimeEdit(QDate::currentDate());
    dateEdit->setMinimumDate(QDate::currentDate().addDays(-365));
    dateEdit->setMaximumDate(QDate::currentDate().addDays(365));
    dateEdit->setDisplayFormat("yyyy.MM.dd");
    \endcode

    Here we've created a new QDateTimeEdit object initialized with
    today's date, and restricted the valid date range to today plus or
    minus 365 days. We've set the order to month, day, year. If the
    auto advance property is true (as we've set it here) when the user
    completes a section of the date, e.g. enters two digits for the
    month, they are automatically taken to the next section.

    The maximum and minimum values for a date value in the date editor
    default to the maximum and minimum values for a QDate. You can
    change this by calling setMinimumDate(), setMaximumDate(),
    setMinimumTime(), and setMaximumTime().
*/

/*!
    \enum QDateTimeEdit::Section

    \value NoSection
    \value AmPmSection
    \value MSecSection
    \value SecondSection
    \value MinuteSection
    \value HourSection
    \value DaySection
    \value MonthSection
    \value YearSection
    \omitvalue DateSections_Mask
    \omitvalue TimeSections_Mask
*/

/*!
    \fn void QDateTimeEdit::dateTimeChanged(const QDateTime &datetime)

    This signal is emitted whenever the date or time is changed. The
    new date and time is passed in \a datetime.
*/

/*!
    \fn void QDateTimeEdit::timeChanged(const QTime &time)

    This signal is emitted whenever the time is changed. The new time
    is passed in \a time.
*/

/*!
    \fn void QDateTimeEdit::dateChanged(const QDate &date)

    This signal is emitted whenever the date is changed. The new date
    is passed in \a date.
*/


/*!
    Constructs an empty date time editor with a \a parent.
*/

QDateTimeEdit::QDateTimeEdit(QWidget *parent)
    : QAbstractSpinBox(*new QDateTimeEditPrivate, parent)
{
    d->minimum = QVariant(DATETIME_MIN);
    d->maximum = QVariant(DATETIME_MAX);
    d->value = QVariant(QDateTime(DATE_INITIAL, TIME_MIN));
    setDisplayFormat(d->defaultDateTimeFormat);
    if (d->displayFormat.isEmpty()) {
        d->defaultDateTimeFormat = QLatin1String("MM/dd/yy hh:mm:ss");
        setDisplayFormat(d->defaultDateTimeFormat);
    }
}

/*!
    Constructs an empty date time editor with a \a parent. The value
    is set to \a datetime.
*/

QDateTimeEdit::QDateTimeEdit(const QDateTime &datetime, QWidget *parent)
    : QAbstractSpinBox(*new QDateTimeEditPrivate, parent)
{
    d->minimum = QVariant(DATETIME_MIN);
    d->maximum = QVariant(DATETIME_MAX);
    d->value = datetime.isValid() ? QVariant(datetime) : QVariant(QDateTime(DATE_INITIAL, TIME_MIN));
    setDisplayFormat(d->defaultDateTimeFormat);
    if (d->displayFormat.isEmpty()) {
        d->defaultDateTimeFormat = QLatin1String("MM/dd/yy hh:mm:ss");
        setDisplayFormat(d->defaultDateTimeFormat);
    }
}

/*!
    \fn QDateTimeEdit::QDateTimeEdit(const QDate &date, QWidget *parent)

    Constructs an empty date time editor with a \a parent.
    The value is set to \a date.
*/

QDateTimeEdit::QDateTimeEdit(const QDate &date, QWidget *parent)
    : QAbstractSpinBox(*new QDateTimeEditPrivate, parent)
{
    d->minimum = QVariant(DATETIME_MIN);
    d->maximum = QVariant(DATETIME_MAX);
    d->value = QVariant(QDateTime(date.isValid() ? date : DATE_INITIAL, TIME_MIN));
    setDisplayFormat(d->defaultDateFormat);
    if (d->displayFormat.isEmpty()) {
        d->defaultDateFormat = QLatin1String("MM/dd/yy");
        setDisplayFormat(d->defaultDateFormat);
    }
}

/*!
    \fn QDateTimeEdit::QDateTimeEdit(const QTime &time, QWidget *parent)

    Constructs an empty date time editor with a \a parent.
    The value is set to \a time.
*/

QDateTimeEdit::QDateTimeEdit(const QTime &time, QWidget *parent)
    : QAbstractSpinBox(*new QDateTimeEditPrivate, parent)
{
    d->minimum = QVariant(DATETIME_MIN);
    d->maximum = QVariant(DATETIME_MAX);
    d->value = QVariant(QDateTime(DATE_INITIAL, time.isValid() ? time : TIME_MIN));
    setDisplayFormat(d->defaultTimeFormat);
    if (d->displayFormat.isEmpty()) {
        d->defaultDateFormat = QLatin1String("hh:mm:ss");
        setDisplayFormat(d->defaultTimeFormat);
    }
}

QDateTime QDateTimeEdit::dateTime() const
{
    return d->value.toDateTime();
}

void QDateTimeEdit::setDateTime(const QDateTime &datetime)
{
    if (datetime.isValid()) {
        d->cachedday = -1;
        d->setValue(QVariant(datetime), EmitIfChanged);
    }
}

/*!
    \property QDateTimeEdit::date
    \brief the QDate that is set in the QDateTimeEdit

    \sa setDate()
*/

QDate QDateTimeEdit::date() const
{
    return d->value.toDate();
}

void QDateTimeEdit::setDate(const QDate &date)
{
    if (date.isValid()) {
        d->cachedday = -1;
        d->setValue(QVariant(QDateTime(date, d->value.toTime())), EmitIfChanged);
    }
}

/*!
    \property QDateTimeEdit::time
    \brief the QTime that is set in the QDateTimeEdit

    \sa setTime()
*/

QTime QDateTimeEdit::time() const
{
    return d->value.toTime();
}

void QDateTimeEdit::setTime(const QTime &time)
{
    if (time.isValid()) {
        d->cachedday = -1;
        d->setValue(QVariant(QDateTime(d->value.toDate(), time)), EmitIfChanged);
    }
}

/*!
    \property QDateTimeEdit::dateTime
    \brief the QDateTime that is set in the QDateTimeEdit

    \sa setDateTime()
*/

/*!
    \property QDateTimeEdit::minimumDate

    \brief the minimum date of the date time edit

    When setting this property the \l QDateTimeEdit::maximumDate is
    adjusted if necessary, to ensure that the range remains valid.
    If the date is not a valid QDate object, this function does
    nothing.

    The default minimum value can be restored with clearMinimum().

    \sa setMinimumDate(), maximumDate(), setMaximumDate(),
    clearMinimumDate(), setMinimumTime(), maximumTime(), setMaximumTime(),
    clearMinimum(), setTimeRange(), setDateRange(), QDate::isValid()
*/

QDate QDateTimeEdit::minimumDate() const
{
    return d->minimum.toDate();
}

void QDateTimeEdit::setMinimumDate(const QDate &min)
{
    if (min.isValid()) {
        d->setBoundary(Minimum, QVariant(QDateTime(min, d->minimum.toTime())));
    }
}

void QDateTimeEdit::clearMinimumDate()
{
    d->setBoundary(Minimum, QVariant(QDateTime(DATE_MIN, d->minimum.toTime())));
}

/*!
    \property QDateTimeEdit::maximumDate

    \brief the maximum date of the date time edit

    When setting this property the \l QDateTimeEdit::minimumDate is
    adjusted if necessary to ensure that the range remains valid.
    If the date is not a valid QDate object, this function does
    nothing.

    The default minimum value can be restored with clearMinimumDate().

    \sa setMinimumDate(), maximumDate(), setMaximumDate(), clearMinimumDate(),
    setMinimumTime(), maximumTime(), setMaximumTime(), clearMinimumTime(),
    setTimeRange(), setDateRange(), QDate::isValid()
*/

QDate QDateTimeEdit::maximumDate() const
{
    return d->maximum.toDate();
}

void QDateTimeEdit::setMaximumDate(const QDate &max)
{
    if (max.isValid())
        d->setBoundary(Maximum, QVariant(QDateTime(max, d->maximum.toTime())));
}

void QDateTimeEdit::clearMaximumDate()
{
    d->setBoundary(Maximum, QVariant(QDateTime(DATE_MAX, d->maximum.toTime())));
}

/*!
    \property QDateTimeEdit::minimumTime

    \brief the minimum time of the date time edit

    When setting this property the \l QDateTimeEdit::maximumTime is
    adjusted if necessary, to ensure that the range remains valid.
    If the time is not a valid QTime object, this function does
    nothing.

    The default minimum value can be restored with clearMinimumTime().

    \sa setMinimumTime(), maximumTime(), setMaximumTime(),
    clearMinimumTime(), setMinimumDate(), maximumDate(), setMaximumDate(),
    clearMinimumDate(), setTimeRange(), setDateRange(), QTime::isValid()
*/

QTime QDateTimeEdit::minimumTime() const
{
    return d->minimum.toTime();
}

void QDateTimeEdit::setMinimumTime(const QTime &min)
{
    if (min.isValid())
        d->setBoundary(Minimum, QVariant(QDateTime(d->minimum.toDate(), min)));
}

void QDateTimeEdit::clearMinimumTime()
{
    d->setBoundary(Minimum, QVariant(QDateTime(d->minimum.toDate(), TIME_MIN)));
}

/*!
    \property QDateTimeEdit::maximumTime

    \brief the maximum time of the date time edit

    When setting this property the \l QDateTimeEdit::maximumTime is
    adjusted if necessary to ensure that the range remains valid.
    If the time is not a valid QTime object, this function does
    nothing.

    The default minimum value can be restored with clearMinimumDate().

    \sa setMinimumDate(), maximumDate(), setMaximumDate(),
    clearMinimumDate(), setMinimumTime(), maximumTime(),
    setMaximumTime(), clearMinimumTime(), setTimeRange(),
    setDateRange(), QTime::isValid()
*/

QTime QDateTimeEdit::maximumTime() const
{
    return d->maximum.toTime();
}

void QDateTimeEdit::setMaximumTime(const QTime &max)
{
    if (max.isValid())
        d->setBoundary(Maximum, QVariant(QDateTime(d->maximum.toDate(), max)));
}

void QDateTimeEdit::clearMaximumTime()
{
    d->setBoundary(Maximum, QVariant(QDateTime(d->maximum.toDate(), TIME_MAX)));
}

/*!
    Convenience function to set minimum and maximum date with one
    function call.

    \code
    setDateRange(min, max);
    \endcode

       is analogous to:

    \code
    setMinimumDate(min);
    setMaximumDate(max);
    \endcode

    If either \a min or \a max are not valid, this function does
    nothing.

    \sa setMinimumDate(), maximumDate(), setMaximumDate(),
    clearMinimumDate(), setMinimumTime(), maximumTime(),
    setMaximumTime(), clearMinimumTime(), QDate::isValid()
*/

void QDateTimeEdit::setDateRange(const QDate &min, const QDate &max)
{
    if (min.isValid() && max.isValid()) {
        d->setBoundary(Minimum, QVariant(QDateTime(min, d->minimum.toTime())));
        d->setBoundary(Maximum, QVariant(QDateTime(max, d->maximum.toTime())));
    }
}

/*!
    Convenience function to set minimum and maximum time with one
    function call.

    \code
    setTimeRange(min, max);
    \endcode

       is analogous to:

    \code
    setMinimumTime(min);
    setMaximumTime(max);
    \endcode

    If either \a min or \a max are not valid, this function does
    nothing.

    \sa setMinimumDate(), maximumDate(), setMaximumDate(),
    clearMinimumDate(), setMinimumTime(), maximumTime(),
    setMaximumTime(), clearMinimumTime(), QTime::isValid()
*/

void QDateTimeEdit::setTimeRange(const QTime &min, const QTime &max)
{
    if (min.isValid() && max.isValid()) {
        d->setBoundary(Minimum, QVariant(QDateTime(d->minimum.toDate(), min)));
        d->setBoundary(Maximum, QVariant(QDateTime(d->maximum.toDate(), max)));
    }
}

/*!
    \property QDateTimeEdit::displayedSections

    \brief the currently displayed fields of the date time edit

    Returns a bit set of the displayed sections for this format.
    \a setDisplayFormat(), displayFormat()
*/

QDateTimeEdit::Sections QDateTimeEdit::displayedSections() const
{
    return d->display;
}

/*!
    \property QDateTimeEdit::currentSection

    \brief the current section of the spinbox
    \a setCurrentSection()
*/

QDateTimeEdit::Section QDateTimeEdit::currentSection() const
{
    switch (d->currentsection) {
    case QDateTimeEditPrivate::NoSection:
    case QDateTimeEditPrivate::FirstSection:
    case QDateTimeEditPrivate::LastSection:
        return QDateTimeEdit::NoSection;
    default:
        return (QDateTimeEdit::Section)(d->currentsection & (~QDateTimeEditPrivate::Internal));
    }
}

void QDateTimeEdit::setCurrentSection(Section section)
{
    const QDateTimeEditPrivate::Section s = (QDateTimeEditPrivate::Section)section;
    if (s != QDateTimeEditPrivate::NoSection)
        d->edit->setCursorPosition(d->sectionNode(s).pos);
}

/*!
    \fn QString QDateTimeEdit::sectionText(Section section) const

    Returns the text from the given \a section.

    \sa cleanText(), currentSection()
*/

QString QDateTimeEdit::sectionText(Section s) const
{
    return d->sectionText(d->edit->displayText(), (QDateTimeEditPrivate::Section)s);
}

/*!
    \property QDateTimeEdit::displayFormat

    \brief the format used to display the time/date of the date time edit

    This format is a subset of the format described in QDateTime::toString()

    These expressions may be used:

    \table
    \header \i Expression \i Output
    \row \i hh
         \i the hour with a leading zero (00 to 23 or 01 to 12 if AM/PM display)
    \row \i mm \i the minute with a leading zero (00 to 59)
    \row \i ss \i the second whith a leading zero (00 to 59)
    \row \i zzz \i the milliseconds with leading zeroes (000 to 999)
    \row \i AP
         \i use AM/PM display. \e AP will be replaced by either "AM" or "PM".
    \row \i ap
         \i use am/pm display. \e ap will be replaced by either "am" or "pm".
    \row \i dd \i the day as number with a leading zero (01 to 31)
    \row \i MM \i the month as number with a leading zero (01 to 12)
    \row \i MMM
         \i the abbreviated localized month name (e.g. 'Jan' to 'Dec').
            Uses QDate::shortMonthName().
    \row \i yy \i the year as two digit number (00 to 99)
    \row \i yyyy \i the year as four digit number (1752 to 8000)
    \endtable

    All other input characters or sequence of characters that are
    enclosed in singlequotes will be treated as text and can be used
    as delimiters.

    Example format strings (assuming that the QDate is the
    20<sup><small>th</small></sup> July 1969):
    \table
    \header \i Format \i Result
    \row \i dd.MM.yyyy    \i 20.07.1969
    \row \i MMM d yy \i Jul 20 69
    \endtable

    If you specify an invalid format the format will not be set.

    Multiple instances of the same field is not allowed. E.g.

    setDisplayFormat("yyyy.MM.yy"); // not allowed

    a format with no valid fields is not allowed either. E.g.

    setDisplayFormat("s.M.y"); // not allowed

    \sa QDateTime::toString(), setDisplayFormat(), displayedSections()

    \warning Since QDateTimeEdit internally always operates on a
    QDateTime changing the format can change the minimum[Time|Date]s
    and the current[Time|Date]. E.g.

    \code
        QDateTimeEdit edit(0); // format is "yyyy.MM.dd hh:mm:ss"
        edit.setMinimumDate(QDate(2000, 1, 1));
        edit.setMaximumDate(QDate(2003, 1, 1));
        edit.setDateTime(QDateTime(QDate(2002, 5, 5), QTime(10, 10, 10)));
        edit.setDisplayFormat("hh:mm:ss");

        // edit can no longer display dates. This means that the
        // minimum and maximum date will be set to the current date,
        // e.g. 2002, 5, 5.
    \endcode
*/

QString QDateTimeEdit::displayFormat() const
{
    return d->displayFormat;
}

void QDateTimeEdit::setDisplayFormat(const QString &format)
{
    if (d->parseFormat(format)) {
        d->cached = QVariant();
        if ((d->display & QDateTimeEditPrivate::TimeSectionMask) == 0
            || (d->display & QDateTimeEditPrivate::DateSectionMask) == 0) {
            if ((d->display & QDateTimeEditPrivate::TimeSectionMask) != 0) {
                setDateRange(d->value.toDate(), d->value.toDate());
            } else { // It must be date
                setTimeRange(TIME_MIN, TIME_MAX);
            }
        }
        d->sizehintdirty = true;
	d->update();
        d->edit->setCursorPosition(0);
        d->editorCursorPositionChanged(-1, 0);
    }
}

/*!
    \reimp
*/

void QDateTimeEdit::clear()
{
    d->clearSection(d->currentsection);
}

/*!
    This virtual function is used by the date time edit whenever it
    needs to interpret text entered by the user as a value. The user's
    text is passed in \a txt and the validator's state in \a state.

    \sa mapDateTimeToText()
*/

/*!
    \reimp
*/

void QDateTimeEdit::keyPressEvent(QKeyEvent *e)
{
    const QDateTimeEditPrivate::Section oldCurrent = d->currentsection;
    bool select = true;

    if ((e->key() == Qt::Key_Backspace || (e->key() == Qt::Key_H && e->key() & Qt::ControlModifier))
	    && !d->edit->hasSelectedText()) {
	const int pos = d->edit->cursorPosition();
	if (pos <= d->separators.first().size()) {
	    e->accept();
	    return;
	}
	select = false;
	const QDateTimeEditPrivate::Section s = d->sectionAt(pos);
	const QDateTimeEditPrivate::Section closest = d->closestSection(pos - 1, false);
	QDTEDEBUG << "found those two" << d->sectionName(s)<< d->sectionName(closest);
	if (s == QDateTimeEditPrivate::LastSection
	    || (s != QDateTimeEditPrivate::NoSection && pos == d->sectionPos(s))) {
	    QString copy = d->edit->displayText();
	    int cursorCopy = pos;
	    if (validate(copy, cursorCopy) != QValidator::Acceptable) {
		d->refresh(EmitIfChanged);
	    }
	    d->ignorecursorpositionchanged = true;
	    d->edit->setCursorPosition(d->sectionPos(closest) + d->sectionSize(closest));
	    d->currentsection = closest;
	    d->ignorecursorpositionchanged = false;
	}
    }

    switch((Qt::Key)e->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        d->refresh(AlwaysEmit);
        d->setSelected(d->currentsection);
        return;

    case Qt::Key_Left:
    case Qt::Key_Right:
        if (!(e->modifiers() & Qt::ControlModifier)) {
            const int selsize = d->edit->selectedText().size();
            if (selsize == 0 || selsize != d->sectionSize(d->currentsection))
                break;
            select = false;
        }
    case Qt::Key_Backtab:
    case Qt::Key_Tab: {
        const QDateTimeEditPrivate::SectionNode newSection =
            d->nextPrevSection(d->currentsection,
                               (e->key() == Qt::Key_Right ||
                                (e->key() == Qt::Key_Tab && !(e->modifiers() & Qt::ShiftModifier))));
        if (select) {
            d->setSelected(newSection.section);
        } else {
            d->edit->setCursorPosition(e->key() == Qt::Key_Right ? newSection.pos : d->sectionPos(d->currentsection));
        }
        if (!select)
            d->edit->deselect();
        e->accept();
        return; }
    default:
        select = !e->text().isEmpty() && e->text().at(0).isPrint();
        break;
    }

    QAbstractSpinBox::keyPressEvent(e);
    if (select && d->currentsection != oldCurrent)
        d->setSelected(d->currentsection);
}

/*!
    \reimp
*/

void QDateTimeEdit::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::ActivationChange) {
        QString *frm = 0;
        if (d->displayFormat == d->defaultTimeFormat) {
            frm = &d->defaultTimeFormat;
        } else if (d->displayFormat == d->defaultDateFormat) {
            frm = &d->defaultDateFormat;
        } else if (d->displayFormat == d->defaultDateTimeFormat) {
            frm = &d->defaultTimeFormat;
        }

        if (frm) {
            d->readLocaleSettings();
            if (d->displayFormat != *frm)
                setDisplayFormat(*frm);
        }
    }
    QAbstractSpinBox::changeEvent(e);
}

/*!
    \reimp
*/

void QDateTimeEdit::wheelEvent(QWheelEvent *e)
{
    const QDateTimeEditPrivate::Section s = d->sectionAt(qMax<int>(0, d->edit->cursorPositionAt(e->pos()) - 1));
    if (s != d->currentsection)
        d->edit->setCursorPosition(d->sectionNode(s).pos);
    switch (s) {
    case QDateTimeEditPrivate::NoSection:
    case QDateTimeEditPrivate::FirstSection:
    case QDateTimeEditPrivate::LastSection:
        break;
    default:
        QAbstractSpinBox::wheelEvent(e);
        break;
    }
}
/*!
    \reimp
*/

void QDateTimeEdit::focusInEvent(QFocusEvent *e)
{
    QAbstractSpinBox::focusInEvent(e);
    QDateTimeEditPrivate::Section s;
    switch(e->reason()) {
    case Qt::ShortcutFocusReason:
    case Qt::TabFocusReason: s = d->sections.first().section; break;
    case Qt::BacktabFocusReason: s = d->sections.at(d->sections.size() - 1).section; break;
    default: return;
    }

    d->setSelected(s);
}

/*!
    \reimp
*/

bool QDateTimeEdit::focusNextPrevChild(bool next)
{
    const QDateTimeEditPrivate::Section newSection = d->nextPrevSection(d->currentsection, next).section;
    switch (newSection) {
    case QDateTimeEditPrivate::NoSection:
    case QDateTimeEditPrivate::FirstSection:
    case QDateTimeEditPrivate::LastSection:
        break;
    default:
        if (newSection != QDateTimeEditPrivate::NoSection) {
            d->currentsection = newSection;
            d->setSelected(newSection);
            return true;
        }
    }
    return QAbstractSpinBox::focusNextPrevChild(next);
}

/*!
    \reimp
*/

void QDateTimeEdit::stepBy(int steps)
{
    const QDateTimeEditPrivate::Section s = d->currentsection;
    d->setValue(d->stepBy(s, steps, false), EmitIfChanged);
    d->setSelected(s);
}

/*!
    This virtual function is used by the date time edit whenever it
    needs to display \a dateTime.

    If you reimplement this, you may also need to reimplement
    valueFromText() and validate().

    \sa valueFromText(), validate()
*/

QString QDateTimeEdit::textFromDateTime(const QDateTime &dateTime) const
{
    QVariant var(dateTime);
    if (var == d->cached) {
	QDTEDEBUG << "cached and var is the same so returning cachedText" << dateTime << d->cachedText;
        return d->cachedText;
    }
    QString ret = d->escapedFormat;
    for (int i=0; i<d->sections.size(); ++i) {
	int l = d->sectionSize(d->sections.at(i).section);
	int pos = d->sections.at(i).pos;
	QDTEDEBUG << "FOOOOO now at " << d->sectionName(d->sections.at(i).section);
	switch (d->sections.at(i).section) {
	    case QDateTimeEditPrivate::AmPmSection:
	    case QDateTimeEditPrivate::AmPmLowerCaseSection: {
		QString input = var.toTime().hour() > 11 ? tr("pm") : tr("am");
		ret.replace(pos, l,
			d->sections.at(i).section == QDateTimeEditPrivate::AmPmSection // ### upper lower needs thinking in terms of tr()
			? input.toUpper() : input); }
		break;
	    case QDateTimeEditPrivate::MonthShortNameSection:
		ret.replace(pos, l, QDate::shortMonthName(var.toDate().month()));
		break;
	    case QDateTimeEditPrivate::YearTwoDigitsSection:
		ret.replace(pos, l,
			QString::number(d->getDigit(var, QDateTimeEditPrivate::YearTwoDigitsSection) - 2000)
			.rightJustified(l, QLatin1Char('0'), true));
		break;
	    default:
		ret.replace(pos, l, QString::number(d->getDigit(var, d->sections.at(i).section)).
		       rightJustified(l, QLatin1Char('0')));
		break;
	}
    }
    d->cached = var;
    d->cachedText = ret;
    QDTEDEBUG << "setting cached to" << d->cached << " and cachedText to" << d->cachedText;
    return ret;
}


/*!
    \fn QDateTime QSpinBox::dateTimeFromText(const QString &text) const

    This virtual function is used by the datetime edit whenever it
    needs to interpret text entered by the user as a value.

    \sa textFromValue(), validate()
*/

QDateTime QDateTimeEdit::dateTimeFromText(const QString &text) const
{
    QString copy = text;
    int pos = d->edit->cursorPosition();
    QValidator::State state = QValidator::Acceptable;
    return d->validateAndInterpret(copy, pos, state).toDateTime();
}

/*!
    \reimp
*/

QValidator::State QDateTimeEdit::validate(QString &text, int &pos) const
{
    QValidator::State state;
    d->validateAndInterpret(text, pos, state);
    return state;
}

/*!
    \reimp
*/

QDateTimeEdit::StepEnabled QDateTimeEdit::stepEnabled() const
{
    if (!style()->styleHint(QStyle::SH_SpinControls_DisableOnBounds))
        return StepEnabled(StepUpEnabled) | StepDownEnabled;
    switch (d->currentsection) {
    case QDateTimeEditPrivate::NoSection:
    case QDateTimeEditPrivate::FirstSection:
    case QDateTimeEditPrivate::LastSection: return 0;
    default: break;
    }
    if (d->wrapping)
        return (StepEnabled)(StepUpEnabled | StepDownEnabled);

    QAbstractSpinBox::StepEnabled ret = 0;

    QVariant v = d->stepBy(d->currentsection, 1, true);
    if (v != d->value) {
	ret |= QAbstractSpinBox::StepUpEnabled;
    }
    v = d->stepBy(d->currentsection, -1, true);
    if (v != d->value) {
	ret |= QAbstractSpinBox::StepDownEnabled;
    }

    return ret;
}


/*!
    Constructs an empty time editor with a \a parent.
*/


QTimeEdit::QTimeEdit(QWidget *parent)
    : QDateTimeEdit(TIME_MIN, parent)
{
}

/*!
    Constructs an empty time editor with a \a parent. The time is set
    to \a time.
*/

QTimeEdit::QTimeEdit(const QTime &t, QWidget *parent)
    : QDateTimeEdit(t, parent)
{
}

/*!
    Constructs an empty date editor with a \a parent.
*/

QDateEdit::QDateEdit(QWidget *parent)
    : QDateTimeEdit(DATE_INITIAL, parent)
{
}

/*!
    Constructs an empty date editor with a \a parent. The date is set
    to \a date.
*/

QDateEdit::QDateEdit(const QDate &date, QWidget *parent)
    : QDateTimeEdit(date, parent)
{
}


// --- QDateTimeEditPrivate ---

/*!
    \internal
    Constructs a QDateTimeEditPrivate object
*/

QDateTimeEditPrivate::QDateTimeEditPrivate()
{
    type = QVariant::DateTime;
    display = (QDateTimeEdit::Sections)0;
    cachedday = -1;
    currentsection = oldcurrentsection = NoSection;
    first.section = FirstSection;
    first.pos = 0;
    last.section = LastSection;
    last.pos = -1;
    readLocaleSettings();
}

/*!
    \internal
    \reimp
*/

void QDateTimeEditPrivate::emitSignals(EmitPolicy ep, const QVariant &old)
{
    if (ep == NeverEmit) {
	return;
    }
    pendingemit = false;

    const bool dodate = value.toDate().isValid() && (display & DateSectionMask);
    const bool datechanged = (ep == AlwaysEmit || old.toDate() != value.toDate());
    const bool dotime = value.toTime().isValid() && (display & TimeSectionMask);
    const bool timechanged = (ep == AlwaysEmit || old.toTime() != value.toTime());

    if (dodate && dotime && (datechanged || timechanged))
	emit q->dateTimeChanged(value.toDateTime());
    if (dodate && datechanged)
        emit q->dateChanged(value.toDate());
    if (dotime && timechanged)
        emit q->timeChanged(value.toTime());
}

/*!
    \internal
    \reimp
*/

void QDateTimeEditPrivate::editorCursorPositionChanged(int oldpos, int newpos)
{
    if (ignorecursorpositionchanged)
        return;
    ignorecursorpositionchanged = true;
    Section s = sectionAt(newpos);
    const Section old = oldcurrentsection;
    oldcurrentsection = sectionAt(oldpos);
    int c = newpos;

    if (!d->dragging) {
        const int selstart = d->edit->selectionStart();
        const Section selSection = sectionAt(selstart);
        const int l = sectionSize(selSection);

        if (s == NoSection) {
            if (l > 0 && selstart == sectionPos(selSection) && d->edit->selectedText().size() == l) {
                s = selSection;
                setSelected(selSection, true);
                c = -1;
            } else {
                const SectionNode &sn = sectionNode(closestSection(newpos, oldpos < newpos));
                c = sn.pos + (oldpos < newpos ? 0 : qMax<int>(0, sectionSize(sn.section) - 1));
                edit->setCursorPosition(c);
                s = sn.section;
            }
        }
    }
    QDTEDEBUGN("oldpos %d newpos %d", oldpos, newpos);
    QDTEDEBUGN("(%s)currentsection = %s (%s)oldcurrentsection = %s",
          sectionName(currentsection).toLatin1().constData(),
          sectionName(s).toLatin1().constData(),
          sectionName(old).toLatin1().constData(),
          sectionName(oldcurrentsection).toLatin1().constData());
    if (currentsection != s) {
        QString tmp = edit->displayText();
        int pos = d->edit->cursorPosition();
        if (q->validate(tmp, pos) != QValidator::Acceptable) {
            refresh(EmitIfChanged);
            if (c == -1) {
                setSelected(s, true);
            } else {
                edit->setCursorPosition(pos);
            } // ### should this set the text to tmp if changed?
        }
        updateSpinBox();
    }
    currentsection = s;
    ignorecursorpositionchanged = false;

}

#ifdef Q_WS_MAC
static QString macParseDateLocale(QVariant::Type type)
{
    CFGregorianDate macGDate;
    macGDate.year = 99;
    macGDate.month = 11;
    macGDate.day = 20; // <---- Should be 22, but seems something is wrong.
    macGDate.hour = 10;
    macGDate.minute = 34;
    macGDate.second = 56.0;
    QCFType<CFDateRef> myDate = CFDateCreate(0, CFGregorianDateGetAbsoluteTime(macGDate, QCFType<CFTimeZoneRef>
                                                                               (CFTimeZoneCopySystem())));
    switch (type) {
    case QVariant::Date: {
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
        if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
            QCFType<CFLocaleRef> mylocale = CFLocaleCopyCurrent();
            QCFType<CFDateFormatterRef> myFormatter = CFDateFormatterCreate(kCFAllocatorDefault,
                                                                            mylocale, kCFDateFormatterShortStyle,
                                                                            kCFDateFormatterNoStyle);
            return QCFString(CFDateFormatterCreateStringWithDate(0, myFormatter, myDate));

        } else
#endif
        {
            Handle intlHandle = GetIntlResource(0);
            LongDateTime oldDate;
            UCConvertCFAbsoluteTimeToLongDateTime(CFGregorianDateGetAbsoluteTime(macGDate, 0),
                                                  &oldDate);
            Str255 pString;
            LongDateString(&oldDate, shortDate, pString, intlHandle);
            return qt_mac_from_pascal_string(pString);
        }
    }
    case QVariant::DateTime: {
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
        if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
            QCFType<CFLocaleRef> mylocale = CFLocaleCopyCurrent();
            QCFType<CFDateFormatterRef> myFormatter = CFDateFormatterCreate(kCFAllocatorDefault,
                                                                            mylocale, kCFDateFormatterShortStyle,
                                                                            kCFDateFormatterMediumStyle);
            return QCFString(CFDateFormatterCreateStringWithDate(0, myFormatter, myDate));

        } else
#endif
        {
            Handle intlHandle = GetIntlResource(0);
            LongDateTime oldDate;
            UCConvertCFAbsoluteTimeToLongDateTime(CFGregorianDateGetAbsoluteTime(macGDate, 0),
                                                  &oldDate);
            Str255 pString;
            LongDateString(&oldDate, shortDate, pString, intlHandle);
            QString final = qt_mac_from_pascal_string(pString);
            LongTimeString(&oldDate, true, pString, intlHandle);
            return final + space + qt_mac_from_pascal_string(pString);
        }
    }
    default: return QString();
    }
}
#endif

/*!
  \internal

   Try to get the format from the local settings
*/
void QDateTimeEditPrivate::readLocaleSettings()
{
    // Time
    QString str = QTime(10, 34, 56).toString(Qt::LocalDate);
    int index = str.indexOf(QLatin1String("10"));
    if (index != -1)
        str.replace(index, 2, QLatin1String("hh"));

    index = str.indexOf(QLatin1String("34"));
    if (index != -1)
        str.replace(index, 2, QLatin1String("mm"));

    index = str.indexOf(QLatin1String("56"));
    if (index != -1)
        str.replace(index, 2, QLatin1String("ss"));

    index = str.indexOf(QLatin1String("pm"), 0, Qt::CaseInsensitive);
    if (index != -1)
        str.replace(index, 2, str.at(index) == QLatin1Char('p') ? QLatin1String("ap") : QLatin1String("AP"));

    defaultTimeFormat = str;

    // Date

    const QDate date(1999, 11, 22);
    const QString shortMonthName = QDate::shortMonthName(date.month());
    const QString longMonthName = QDate::longMonthName(date.month());
    const QString shortDayName = QDate::shortDayName(date.dayOfWeek());
    const QString longDayName = QDate::longDayName(date.dayOfWeek());

#ifdef Q_WS_MAC
    str = macParseDateLocale(QVariant::Date);
#else
    str = date.toString(Qt::LocalDate);
#endif

    index = str.indexOf(QLatin1String("22"));
    if (index != -1)
        str.replace(index, 2, QLatin1String("dd"));

    index = str.indexOf(shortDayName);
    if (index != -1)
        str.remove(index, shortDayName.size());

    index = str.indexOf(longDayName);
    if (index != -1)
        str.remove(index, longDayName.size());

    index = str.indexOf(QLatin1String("11"));
    if (index != -1) {
        str.replace(index, 2, QLatin1String("MM"));
    } else if ((index = str.indexOf(longMonthName)) != -1) {
        str.replace(index, longMonthName.size(), QLatin1String("MMM"));
    } else if ((index = str.indexOf(shortMonthName)) != -1) {
        str.replace(index, shortMonthName.size(), QLatin1String("MMM"));
    }

    index = str.indexOf(QLatin1String("1999"));
    if (index != -1) {
        str.replace(index, 4, QLatin1String("yyyy"));
    } else {
        index = str.indexOf(QLatin1String("99"));
        if (index != -1)
            str.replace(index, 2, QLatin1String("yy"));
    }

    defaultDateFormat = str;

    // DateTime
#ifdef Q_WS_MAC
    str = macParseDateLocale(QVariant::DateTime);
#else
    str = QDateTime(QDate(1999, 11, 22), QTime(10, 34, 56)).toString(Qt::LocalDate);
#endif
    index = str.indexOf(QLatin1String("10"));
    if (index != -1)
        str.replace(index, 2, QLatin1String("hh"));

    index = str.indexOf(QLatin1String("34"));
    if (index != -1)
        str.replace(index, 2, QLatin1String("mm"));

    index = str.indexOf(QLatin1String("56"));
    if (index != -1)
        str.replace(index, 2, QLatin1String("ss"));

    index = str.indexOf(QLatin1String("pm"), 0, Qt::CaseInsensitive);
    if (index != -1)
        str.replace(index, 2, str.at(index) == QLatin1Char('p') ? QLatin1String("ap") : QLatin1String("AP"));

    index = str.indexOf(QLatin1String("22"));
    if (index != -1)
        str.replace(index, 2, QLatin1String("dd"));

    index = str.indexOf(shortDayName);
    if (index != -1)
        str.remove(index, shortDayName.size());

    index = str.indexOf(longDayName);
    if (index != -1)
        str.remove(index, longDayName.size());

    index = str.indexOf(QLatin1String("11"));
    if (index != -1) {
        str.replace(index, 2, QLatin1String("MM"));
    } else if ((index = str.indexOf(longMonthName)) != -1) {
        str.replace(index, longMonthName.size(), QLatin1String("MMM"));
    } else if ((index = str.indexOf(shortMonthName)) != -1) {
        str.replace(index, shortMonthName.size(), QLatin1String("MMM"));
    }

    index = str.indexOf(QLatin1String("99"));
    if (index > 1 && str.at(index - 1) == QLatin1Char('9') && str.at(index - 2) == QLatin1Char('9')) {
        str.replace(index - 2, 4, QLatin1String("yyyy"));
    } else if (index != -1 ){
        str.replace(index, 2, QLatin1String("yy"));
    }

    defaultDateTimeFormat = str;

    QDTEDEBUG << "default Time:" << defaultTimeFormat << "default date:" << defaultDateFormat << "default date/time" << defaultDateTimeFormat;
}


/*!
    \internal
    Gets the digit from a corevariant. E.g.

    QVariant var(QDate(2004, 02, 02));
    int digit = getDigit(var, Year);
    // digit = 2004
*/

int QDateTimeEditPrivate::getDigit(const QVariant &t, Section s) const
{
    switch(s) {
    case HourSection: {
        int h = t.toTime().hour();
        if (display & AmPmSection) {
            h = h % 12;
            return h == 0 ? 12 : h;
        } else {
            return t.toTime().hour();
        }
    }
    case MinuteSection: return t.toTime().minute();
    case SecondSection: return t.toTime().second();
    case MSecSection: return t.toTime().msec();
    case YearTwoDigitsSection:
    case YearSection: return t.toDate().year();
    case MonthShortNameSection:
    case MonthSection: return t.toDate().month();
    case DaySection: return t.toDate().day();
    case AmPmSection:
    case AmPmLowerCaseSection:
        return t.toTime().hour() > 11 ? 1 : 0;

    default: break;
    }
    qFatal("%s passed to getDigit. This should never happen", sectionName(s).toLatin1().constData());
    return -1;
}

/*!
    \internal
    Sets a digit in a variant. E.g.

    QVariant var(QDate(2004, 02, 02));
    int digit = getDigit(var, Year);
    // digit = 2004
    setDigit(&var, Year, 2005);
    digit = getDigit(var, Year);
    // digit = 2005

    returns true if the newVal was changed to make it work. E.g. If you set 31st when you're in february
*/

void QDateTimeEditPrivate::setDigit(QVariant &v, Section section, int newVal) const
{
    int year, month, day, hour, minute, second, msec;
    const QDateTime &dt = v.toDateTime();
    year = dt.date().year();
    month = dt.date().month();
    day = dt.date().day();
    hour = dt.time().hour();
    minute = dt.time().minute();
    second = dt.time().second();
    msec = dt.time().msec();

    switch(section) {
    case HourSection: hour = newVal; break;
    case MinuteSection: minute = newVal; break;
    case SecondSection: second = newVal; break;
    case MSecSection: msec = newVal; break;
    case YearTwoDigitsSection:
    case YearSection: year = newVal; break;
    case MonthSection:
    case MonthShortNameSection: month = newVal; break;
    case DaySection: day = newVal; break;
    case AmPmSection:
    case AmPmLowerCaseSection: hour = (newVal == 0 ? hour % 12 : (hour % 12) + 12); break;
    default:
        qFatal("%s passed to setDigit. This should never happen", sectionName(section).toLatin1().constData());
        break;
    }

    if (section != DaySection) {
        day = qMax<int>(cachedday, day);
    }

    if (!QDate::isValid(year, month, day)) {
	if (year <= DATE_MIN.year() && (month < DATE_MIN.month() || (month == DATE_MIN.month() && day < DATE_MIN.day()))) {
            month = DATE_MIN.month();
	    day = DATE_MIN.day();
	} else {
	    day = qMin<int>(day, QDate(year, month, 1).daysInMonth());
        }
    }
    v = QVariant(QDateTime(QDate(year, month, day), QTime(hour, minute, second, msec)));
}


/*!
    \internal

    Internal function called by QDateTimeEdit::stepBy(). Also takes a
    Section for which section to step on and a bool \a test for
    whether or not to modify the internal cachedday variable. This is
    necessary because the function is called from the const function
    QDateTimeEdit::stepEnabled() as well as QDateTimeEdit::stepBy().
*/

QVariant QDateTimeEditPrivate::stepBy(Section s, int steps, bool test) const
{
    QVariant v = value;
    QString str = edit->displayText();
    int pos = edit->cursorPosition();

    int val;
    // to make sure it behaves reasonably when typing something and then stepping in non-tracking mode
    if (!test && pendingemit) {
        if (q->validate(str, pos) != QValidator::Acceptable) {
            v = value;
        } else {
	    v = valueFromText(str);
        }
        val = getDigit(v, s);
    } else {
        QValidator::State state;
        val = sectionValue(s, str, state);
        if (state == QValidator::Invalid) {
            return value;
        }
    }

    if (s == HourSection && display & AmPmSection) {
	if (val == 12 && steps > 0) {
	    val = wrapping ? val + steps : 12;
	} else if (val == 23 && steps > 0) {
	    val = wrapping ? val + steps : 23;
	} else {
	    val += steps;
	}
    } else {
	val += steps;
    }
    const int min = absoluteMin(s);
    const int max = absoluteMax(s);

    if (val < min) {
	val = (wrapping ? max - (min - val) + 1 : min);
    } else if (val > max) {
	val = (wrapping ? min + val - max - 1 : max);
    }

    const int tmp = v.toDate().day();
    setDigit(v, s, val); // if this sets year or month it will make
    // sure that days are lowered if needed.

    // changing one section should only modify that section, if possible
    if (!(s & AmPmSection) && (v < minimum || v > maximum)) {
        const int localmin = getDigit(minimum, s);
        const int localmax = getDigit(maximum, s);

        if (wrapping) {
            // just because we hit the roof in one direction, it
            // doesn't mean that we hit the floor in the other
            QVariant oldv = v;
            if (steps > 0) {
                setDigit(v, s, min);
                if (v < minimum) {
                    v = oldv;
                    setDigit(v, s, localmin);
                }
            } else {
                setDigit(v, s, max);
                if (v > maximum) {
                    v = oldv;
                    setDigit(v, s, localmax);
                }
            }
        } else {
            setDigit(v, s, (steps>0) ? localmax : localmin);
        }
    }
    if (!test && tmp != v.toDate().day() && s != DaySection) {
        // this should not happen when called from stepEnabled
        cachedday = qMax<int>(tmp, cachedday);
    }

    if (v < minimum) {
        QVariant t = v;
        setDigit(t, s, steps < 0 ? max : min);
        if (!(t < minimum || t > maximum)) {
            v = t;
        } else {
            setDigit(t, s, getDigit(steps < 0 ? maximum : minimum, s));
            if (!(t < minimum || t > maximum)) {
                v = t;
            }
        }
    } else if (v > maximum) {
        QVariant t = v;
        setDigit(t, s, steps > 0 ? min : max);
        if (!(t < minimum || t > maximum)) {
            v = t;
        } else {
            setDigit(t, s, getDigit(steps > 0 ? minimum : maximum, s));
            if (!(t < minimum || t > maximum)) {
                v = t;
            }
        }
    }

    return bound(v);
}

QVariant QDateTimeEditPrivate::valueForPosition(int pos) const
{
    QStyleOptionSpinBox sb = styleOption();
    QRect r = q->style()->subControlRect(QStyle::CC_SpinBox, &sb, QStyle::SC_SpinBoxSlider, q);

    double percentage = (double)pos / r.width();


    double totalDays = (double)minimum.toDateTime().daysTo(maximum.toDateTime());
    totalDays += (double)minimum.toDateTime().time().msecsTo(maximum.toDateTime().time()) / (24 * 3600 * 1000);

    if (percentage == 0) {
        return minimum;
    } else if (percentage == 1) {
        return maximum;
    }

    double diff = (totalDays * percentage);
    QDate date = minimum.toDate().addDays((qint64)diff); // ### hack. There must be a nicer way
    QTime time = QTime().addMSecs((int)(24 * 3600 * 1000 * (diff - ((double)(qint64)diff))));
    return QVariant(QDateTime(date, time));
}


/*!
    \internal

    Returns the absolute maximum for a section
*/

int QDateTimeEditPrivate::absoluteMax(Section s) const
{
    switch(s) {
    case HourSection: return 23;
    case MinuteSection:
    case SecondSection: return 59;
    case MSecSection: return 999;
    case YearTwoDigitsSection: return 2099;
    case YearSection: return 7999;
    case MonthSection:
    case MonthShortNameSection: return 12;
    case DaySection: return 31;
    case AmPmSection:
    case AmPmLowerCaseSection: return 1;
    default: break;
    }
    qFatal("%s passed to max. This should never happen", sectionName(s).toLatin1().constData());
    return -1;

}

/*!
    \internal

    Returns the absolute minimum for a section
*/

int QDateTimeEditPrivate::absoluteMin(Section s) const
{
    switch(s) {
    case HourSection:
    case MinuteSection:
    case SecondSection:
    case MSecSection: return 0;
    case YearTwoDigitsSection: return 2000;
    case YearSection: return 1753;
    case MonthSection:
    case MonthShortNameSection:
    case DaySection: return 1;
    case AmPmSection:
    case AmPmLowerCaseSection: return 0;
    default: break;
    }
    qFatal("%s passed to min. This should never happen", sectionName(s).toLatin1().constData());
    return -1;
}

/*!
    \internal

    Returns a copy of the sectionNode for the Section \a s.
*/

QDateTimeEditPrivate::SectionNode QDateTimeEditPrivate::sectionNode(Section s) const
{
    if (s == FirstSection) {
        return first;
    } else if (s == LastSection) {
        return last;
    }

    for (int i=0; i<sections.size(); ++i)
	if ((sections.at(i).section & ~Internal) == (s & ~Internal))
	    return sections.at(i);
    SectionNode sn;
    sn.section = NoSection;
    sn.pos = -1;
    return sn;
}

/*!
    \internal

    Returns the starting position for section \a s.
*/

int QDateTimeEditPrivate::sectionPos(Section s) const
{
    if (s == FirstSection) {
        return first.pos;
    } else if (s == LastSection) {
        return last.pos;
    }

    for (int i=0; i<sections.size(); ++i)
	if (sections.at(i).section == s)
	    return sections.at(i).pos;
    return -1;
}

/*!
    \internal

    Adds a section to \a list. If this section already exists returns false.
*/

bool QDateTimeEditPrivate::addSection(QList<SectionNode> &list, Section ds, int pos)
{
    for (int i=0; i<list.size(); ++i) {
	if ((list.at(i).section & ~Internal) == (ds & ~Internal)) {
            QDTEDEBUGN("Could not add section %s to pos %d because it is already in the list", sectionName(ds).toLatin1().constData(), pos);
	    return false;
        }
    }
    SectionNode s;
    s.section = ds;
    s.pos = pos;
    list << s;

    return true;
}


/*!
    \internal

    Selects the section \a s. If \a forward is false selects backwards.
*/

void QDateTimeEditPrivate::setSelected(Section s, bool forward)
{
    switch (s) {
    case NoSection:
    case LastSection:
    case FirstSection:
        return;
    default: break;
    }
    if (forward) {
        edit->setSelection(d->sectionPos(s), d->sectionSize(s));
    } else {
        edit->setSelection(d->sectionPos(s) + d->sectionSize(s), -d->sectionSize(s));
    }
}

/*!
    \internal

    Parses the format \a newFormat. If successful, returns true and
    sets up the format. Else keeps the old format and returns false.

*/

static QString unquote(const QString &str)
{
    QString ret;
    const QChar quote = QLatin1Char('\'');
    QChar status = QLatin1Char('0');
    for (int i=0; i<str.size(); ++i) {
        if (str.at(i) == quote) {
            if (status != quote) {
                status = quote;
            } else if (!ret.isEmpty() && str.at(i - 1) == QLatin1Char('\\')) {
                ret[ret.size() - 1] = quote;
            } else {
                status = QLatin1Char('0');
            }
        } else {
            ret += str.at(i);
        }
    }
    return ret;
}

bool QDateTimeEditPrivate::parseFormat(const QString &newFormat)
{
    QList<SectionNode> list;
    QDateTimeEdit::Sections newDisplay = 0;
    QStringList newSeparators;
    int i, index = 0;
    int add = 0;
    QChar status = QLatin1Char('0');
    const QChar quote = QLatin1Char('\'');
    for (i = 0; i<newFormat.size(); ++i) {
        if (newFormat.at(i) == quote) {
            ++add;
            if (status != quote) {
                status = quote;
            } else if (newFormat.at(i - 1) != QLatin1Char('\\')) {
                status = QLatin1Char('0');
            }
        } else if (i + 1 < newFormat.size() && status != quote) {
	    switch (newFormat.at(i).cell()) {
	    case 'h':
		if (newFormat.at(i+1) == QLatin1Char('h')) {
                    if (!addSection(list, HourSection, i - add))
                        return false;
                    newSeparators << unquote(newFormat.mid(index, i - index));
                    index = ++i + 1;
		    newDisplay |= QDateTimeEdit::HourSection;
		}
		break;
	    case 'm':
		if (newFormat.at(i+1) == QLatin1Char('m')) {
                    if (!addSection(list, MinuteSection, i - add))
                        return false;
                    newSeparators << unquote(newFormat.mid(index, i - index));
                    index = ++i + 1;
		    newDisplay |= QDateTimeEdit::MinuteSection;
		}
		break;
	    case 's':
		if (newFormat.at(i+1) == QLatin1Char('s')) {
                    if (!addSection(list, SecondSection, i - add))
                        return false;
                    newSeparators << unquote(newFormat.mid(index, i - index));
                    index = ++i + 1;
		    newDisplay |= QDateTimeEdit::SecondSection;
		}
		break;
	    case 'z':
		if (i + 2 <newFormat.size()
                    && newFormat.at(i+1) == QLatin1Char('z')
                    && newFormat.at(i+2) == QLatin1Char('z')) {
		    if (!addSection(list, MSecSection, i - add))
                        return false;
                    newSeparators << unquote(newFormat.mid(index, i - index));
                    index = (i += 2) + 1;
		    newDisplay |= QDateTimeEdit::MSecSection;
		}
		break;
	    case 'A':
	    case 'a': {
		const bool cap = newFormat.at(i) == QLatin1Char('A');
		if (newFormat.at(i+1) == (cap ? QLatin1Char('P') : QLatin1Char('p'))) {
		    if (!addSection(list, cap ? AmPmSection : AmPmLowerCaseSection, i - add))
                        return false;
                    newSeparators << unquote(newFormat.mid(index, i - index));
                    index = ++i + 1;
		    newDisplay |= QDateTimeEdit::AmPmSection;
		}
		break; }
	    case 'y':
		if (newFormat.at(i+1) == QLatin1Char('y')) {
                    static const QDate YY_MIN(2000, 1, 1);
                    static const QDate YY_MAX(2000, 12, 31);
                    const bool four = (i + 3 <newFormat.size()
                                       && newFormat.at(i+2) == QLatin1Char('y') && newFormat.at(i+3) == QLatin1Char('y'));
		    if (!addSection(list, four ? YearSection : YearTwoDigitsSection, i - add)
                        || (!four && (maximum.toDate() < YY_MIN || minimum.toDate() > YY_MAX)))
                        return false;

                    newSeparators << unquote(newFormat.mid(index, i - index));
                    index = (i += (four ? 3 : 1)) + 1;
		    newDisplay |= QDateTimeEdit::YearSection;
		}
		break;
	    case 'M':
		if (newFormat.at(i+1) == QLatin1Char('M')) {
		    const bool three = (i + 2 <newFormat.size() && newFormat.at(i+2) == QLatin1Char('M'));
		    if (!addSection(list, three ? MonthShortNameSection : MonthSection, i - add))
                        return false;
                    newSeparators << unquote(newFormat.mid(index, i - index));
                    index = (i += (three ? 2 : 1)) + 1;
		    newDisplay |= QDateTimeEdit::MonthSection;
		}
		break;

	    case 'd':
		if (newFormat.at(i+1) == QLatin1Char('d')) {
		    if (!addSection(list, DaySection, i - add))
                        return false;
                    newSeparators << unquote(newFormat.mid(index, i - index));
                    index = ++i + 1;
		    newDisplay |= QDateTimeEdit::DaySection;
		}
		break;

	    default: break;
	    }
	}
    }
    if (list.isEmpty()) {
	QDTEDEBUGN("Could not parse format. No sections in format '%s'.", newFormat.toLatin1().constData());
	return false;
    }

    newSeparators << (index < newFormat.size() ? unquote(newFormat.mid(index)) : QString());

    displayFormat = newFormat;
    escapedFormat = QString();
    status = QLatin1Char('0');
    for (int i = 0; i < newFormat.length(); ++i) {
        if (newFormat.at(i) == quote){
            if (status == quote) {
                if (!escapedFormat.isEmpty() && newFormat.at(i - 1) == QLatin1Char('\\')) {
                    escapedFormat[escapedFormat.length() - 1] = quote;
                } else {
                    status = QLatin1Char('0');
                }
            } else {
                status = quote;
            }
        } else {
            escapedFormat += newFormat.at(i);
        }
    }
    separators = newSeparators;
//     QDTEDEBUGN("escapedFormat = [%s]", escapedFormat.toLatin1().constData());
//     QDTEDEBUGN("separators:\n%s", separators.join("\n").toLatin1().constData());

    QDTEDEBUGN("display is [%0x]", (uint)display);
    display = newDisplay;
    last.pos = newFormat.size();

    sections = list;
    return true;
}

/*!
    \internal

    Returns the section at index \a index or NoSection if there are no sections there.
*/

QDateTimeEditPrivate::Section QDateTimeEditPrivate::sectionAt(int index) const
{
    if (index < separators.first().size()) {
        return (index == 0 ? FirstSection : NoSection);
    } else if (escapedFormat.size() - index < separators.last().size() + 1) {
        if (separators.last().size() == 0)
            return sections.last().section;
        return (index == last.pos ? LastSection : NoSection);
    }
    for (int i=0; i<sections.size(); ++i) {
	const int tmp = sections.at(i).pos;
        if (index < tmp + sectionSize(sections.at(i).section)) {
            return (index < tmp ? NoSection : sections.at(i).section);
        }
    }
    qWarning("%d index return NoSection. This should not happen", index);
    return NoSection;
}

/*!
    \internal

    Returns the closest section of index \a index. Searches forward
    for a section if \a forward is true. Otherwise searches backwards.
*/

QDateTimeEditPrivate::Section QDateTimeEditPrivate::closestSection(int index, bool forward) const
{
    if (index < separators.first().size()) {
        return forward ? sections.first().section : FirstSection;
    } else if (last.pos - index < separators.last().size() + 1) {
        return forward ? LastSection : sections.last().section;
    }
    for (int i=0; i<sections.size(); ++i) {
	int tmp = sections.at(i).pos;
        if (index < tmp + sectionSize(sections.at(i).section)) {
            if (index < tmp && !forward)
                return sections.at(i-1).section;
            return sections.at(i).section;
        } else if (i == sections.size() - 1 && index > tmp) {
            return sections.at(i).section;
        }
    }
    qWarning("2index return NoSection. This should not happen");
    return NoSection;
}

/*!
    \internal

    Returns a copy of the section that is before or after \a current, depending on \a forward.
*/

QDateTimeEditPrivate::SectionNode QDateTimeEditPrivate::nextPrevSection(Section current, bool forward) const
{
    if (current == FirstSection) {
        return (forward ? sections.first() : first);
    } else if (current == LastSection) {
        return (forward ? last : sections.last());
    }
    for (int i=0; i<sections.size(); ++i) {
	if (sections.at(i).section == current) {
	    int index = i + (forward ? 1 : -1);
	    if (index >= 0 && index < sections.size()) {
		return sections.at(index);
	    } else {
		break;
	    }
	}
    }
    return (forward ? last : first);
}

QStyleOptionSpinBox QDateTimeEditPrivate::styleOption() const
{
    QStyleOptionSpinBox opt;
    opt.init(q);
    opt.stepEnabled = q->stepEnabled();
    opt.activeSubControls = 0;
    opt.buttonSymbols = buttonsymbols;
    opt.subControls = QStyle::SC_SpinBoxUp | QStyle::SC_SpinBoxDown | QStyle::SC_SpinBoxEditField;
    if (slider)
        opt.subControls |= QStyle::SC_SpinBoxSlider;
    if (frame)
        opt.subControls |= QStyle::SC_SpinBoxFrame;

    if (d->buttonstate & Up) {
        opt.activeSubControls = QStyle::SC_SpinBoxUp;
    } else if (buttonstate & Down) {
        opt.activeSubControls = QStyle::SC_SpinBoxDown;
    }

    double days = (double)minimum.toDateTime().daysTo(value.toDateTime());
    days += (double)minimum.toDateTime().time().msecsTo(value.toDateTime().time()) / (24 * 3600 * 1000);
    double totalDays = (double)minimum.toDateTime().daysTo(maximum.toDateTime());
    totalDays += (double)minimum.toDateTime().time().msecsTo(maximum.toDateTime().time()) / (24 * 3600 * 1000);

    opt.percentage = days / totalDays;
    opt.showSliderIndicator = slider;
    opt.showFrame = frame;
    return opt;
}



/*!
    \internal

    Clears the text of section \a s.
*/

void QDateTimeEditPrivate::clearSection(Section s)
{
    int cursorPos = d->edit->cursorPosition();
    bool blocked = d->edit->blockSignals(true);
    QString t = d->edit->text();
    t.replace(sectionPos(s), sectionSize(s), QString().fill(space, sectionSize(s)));
    d->edit->setText(t);
    d->edit->setCursorPosition(cursorPos);
    d->edit->blockSignals(blocked);
}

/*!
    \internal

    Returns the size of section \a s.
*/

int QDateTimeEditPrivate::sectionSize(Section s)
{
    switch(s) {
    case FirstSection:
    case NoSection:
    case LastSection: return 0;

    case HourSection:
    case MinuteSection:
    case SecondSection:
    case AmPmSection:
    case AmPmLowerCaseSection:
    case DaySection:
    case MonthSection:
    case YearTwoDigitsSection: return 2;

    case MonthShortNameSection:
    case MSecSection: return 3;

    case YearSection: return 4;

    case Internal:
    case TimeSectionMask:
    case DateSectionMask: qWarning("Invalid section %s", sectionName(s).toLatin1().constData());
    }
    return -1;
}

/*!
    \internal

    Returns the text of section \a s. This function operates on the
    arg text rather than edit->text().
*/


QString QDateTimeEditPrivate::sectionText(const QString &text, Section s) const
{
    const SectionNode sn = sectionNode(s);
    return sn.section == NoSection ? QString() : text.mid(sn.pos, sectionSize(s));
}

/*!
    \internal

    Parses the part of \a text that corresponds to \a s and returns
    the value of that field. Sets *stateptr to the right state if
    stateptr != 0.
*/

int QDateTimeEditPrivate::sectionValue(Section s, QString &text, QValidator::State &state) const
{
    state = QValidator::Invalid;
    int num = 0;
    QString st = sectionText(text, s);
    if (st.trimmed().isEmpty()) {
        state = QValidator::Intermediate;
    } else {
        const int index = sectionNode(s).pos;
        const int size = sectionSize(s);
        const bool done = !st.contains(space);
        switch(s) {
        case AmPmSection:
        case AmPmLowerCaseSection: {
            st = st.toLower();
            text.replace(index, size, (s == AmPmSection
			 ? text.mid(index, size).toUpper() : text.mid(index, size).toLower()));
            if (done) {
                num = (st == "am" ? 0 : (st == "pm" ? 1 : -1));
                state = (num == -1 ? QValidator::Invalid : QValidator::Acceptable);
            } else {
                const QChar ch = (st.at(0) == space ? st.at(1) : st.at(0));
                switch (ch.toLatin1()) {
                case 'p': num = 1;
                case 'a':
                case 'm': state = QValidator::Intermediate; break;
                default: state = QValidator::Invalid;
                }
            }
            break;
        }
        case MonthShortNameSection: {
            st = st.toLower();
            int tmp = findMonth(st);
            if (tmp != -1) {
                num = tmp;
                state = done ? QValidator::Acceptable : QValidator::Intermediate;
                st[0] = st.at(0).toUpper();
                text.replace(index, size, st);
            } else {
                state = QValidator::Invalid;
                QDTEDEBUG << "invalid because" << st << "doesn't match any month name";
            }
            break;
        }
        case YearTwoDigitsSection: num = 2000;
        case YearSection:
        case MonthSection:
        case HourSection:
        case MinuteSection:
        case SecondSection:
        case MSecSection:
        case DaySection: {
            bool ok;
            num += (int)(st.toUInt(&ok));
            if (!ok) {
                state = QValidator::Invalid;
                QDTEDEBUG << "invalid because" << st << "can't become a uint";
            } else {
                if (s == HourSection && display & AmPmSection) {
                    bool pm = (sectionText(text, AmPmSection).toLower() == "pm");
                    if (pm && num < 12) {
                        num += 12;
                    } else if (!pm && num == 12) {
                        num = 0;
                    } else if (num > 12) {
                        state = QValidator::Invalid;
                        QDTEDEBUG << "invalid because" << st << "num is" << num;

                        break;
                    }
                }
                if (num < absoluteMin(s) || num > absoluteMax(s)) {
                    state = done ? QValidator::Invalid : QValidator::Intermediate;
                    if (done)
                        QDTEDEBUG << "invalid because" << st << "num is" << num
				  << "outside absoluteMin and absoluteMax" << absoluteMin(s) << absoluteMax(s);

                } else {
                    state = QValidator::Acceptable;
                }
            }
            break;
        }
        default: qFatal("NoSection or Internal. This should never happen"); break; }
    }

    return (state != QValidator::Invalid ? num : -1);
}

/*!
    \internal
    \reimp
*/

QVariant QDateTimeEditPrivate::validateAndInterpret(QString &input,
						    int &pos, QValidator::State &state) const
{
    if (cachedtext == input) {
	state = cachedstate;
	QDTEDEBUG << "state" << state << "cachedtext" << cachedtext << "cachedvalue" << cachedvalue;
	return cachedvalue;
    }
    QVariant tmp;
    SectionNode sn;
    int index = 0;

    QDTEDEBUGN("%s", input.toLatin1().constData());
    int diff = input.size() - escapedFormat.size();
    if (diff > 0) {
	const Section s = closestSection(pos - 1, false);
        if (s == FirstSection && s == LastSection) {
            QDTEDEBUG << "invalid because s ==" << sectionName(s);
            return QValidator::Invalid;
        }
        sn = sectionNode(s);
        const int sectionstart = sn.pos;
        const int sectionsize = sectionSize(s);

	QString sub = input.mid(sectionstart, sectionsize + diff);
        if (sub.count(space) < diff) {
            QDTEDEBUGN("sub is '%s' diff is %d sub.count is %d", sub.toLatin1().constData(), diff, sub.count(space));
	    state = QValidator::Invalid;
	    goto end;
        }

        sub.remove(space);
	input.replace(sectionstart, sectionsize + diff, sub.leftJustified(sectionsize, space));
    } else if (diff < 0) {
	const Section s = closestSection(pos, false);
        if (s == FirstSection && s == LastSection) {
            QDTEDEBUG << "invalid because s == " << sectionName(s);
	    state = QValidator::Invalid;
	    goto end;
        }
        sn = sectionNode(s);
        const int sectionstart = sn.pos;
        const int sectionsize = sectionSize(s);

	QString sub = input.mid(sectionstart, sectionsize + diff);
        sub.remove(space);
	input.replace(sectionstart, sectionsize + diff, sub.leftJustified(sectionsize, space));

        sn = sectionNode(currentsection);
    }

    for (int i=0; i<sections.size(); ++i) {
        sn = sections.at(i);
	if (input.mid(index, sn.pos - index) != separators.at(i)) {
	    QDTEDEBUG << "invalid because" << input.mid(index, sn.pos - index) << "!=" << separators.at(i);
	    state = QValidator::Invalid;
	    goto end;
        }
        index = sn.pos + sectionSize(sn.section);
    }

    if (sn.pos + sectionSize(sn.section) < input.size()
	    && input.mid(sn.pos + sectionSize(sn.section)) != separators.last()) {
        QDTEDEBUG << "invalid because" << input.mid(sn.pos + sectionSize(sn.section))
		  << "!=" << separators.last();
	state = QValidator::Invalid;
	goto end;
    }

    {
	int year, month, day, hour, minute, second, msec;
	const QDateTime &dt = value.toDateTime();
	year = dt.date().year();
	month = dt.date().month();
	day = dt.date().day();
	hour = dt.time().hour();
	minute = dt.time().minute();
	second = dt.time().second();
	msec = dt.time().msec();

	state = QValidator::Acceptable;
	for (int i=0; state != QValidator::Invalid && i<sections.size(); ++i) {
	    const Section s = sections.at(i).section;
	    QValidator::State tmpstate;
	    int num = sectionValue(s, input, tmpstate);
	    // Apple's GCC 3.3 and GCC 4.0 CVS flags a warning on qMin,
	    // so code by hand to remove the warning.
	    state = state < tmpstate ? state : tmpstate;

	    if (state != QValidator::Invalid) {
		switch (s) {
		    case HourSection: hour = num; break;
		    case MinuteSection: minute = num; break;
		    case SecondSection: second = num; break;
		    case MSecSection: msec = num; break;
		    case YearTwoDigitsSection:
		    case YearSection: year = (num == 0 ? DATE_INITIAL.year() : num); break;
		    case MonthSection:
		    case MonthShortNameSection: month = qMax<int>(1, num); break;
		    case DaySection: day = qMax<int>(1, num); break;
		    case AmPmSection:
		    case AmPmLowerCaseSection: hour = (num == 0 ? hour % 12 : (hour % 12) + 12); break;
		    default:
			qFatal("%s found in sections validateAndInterpret. This should never happen",
				sectionName(s).toLatin1().constData());
			break;
		}
	    }
	}

	if (state == QValidator::Invalid) {
	    tmp = getZeroVariant();
	} else {
	    bool fixday = false;
	    if (currentsection == DaySection) {
		cachedday = day;
	    } else if (cachedday > day) {
		day = cachedday;
		fixday = true;
	    }

	    if (!QDate::isValid(year, month, day)) {
		if (day < 32) {
		    cachedday = day;
		}
		fixday = true;
	    }
	    if (fixday) {
		day = qMin<int>(day, QDate(year, month, 1).daysInMonth());
		const SectionNode &sn = sectionNode(DaySection);
		input.replace(sn.pos, sectionSize(DaySection), QString::number(day));
	    }

	    QDTEDEBUG << year << month << day << hour << minute << second << msec;
	    tmp = QVariant(QDateTime(QDate(year, month, day), QTime(hour, minute, second, msec)));
	}
	QDTEDEBUGN("'%s' => '%s' (%s)", input.toLatin1().constData(),
		tmp.toString().toLatin1().constData(), stateName(state).toLatin1().constData());
    }
end:
    if (state != QValidator::Invalid && (tmp < minimum || tmp > maximum)) {
	if (!tmp.toDateTime().isValid())
	    QDTEDEBUG << "invalid tmp is invalid";
	state = checkIntermediate(tmp.toDateTime(), input);
    } else {
	QDTEDEBUG << "not checking intermediate because tmp is" << tmp << minimum << maximum;
    }
    cachedtext = input;
    cachedstate = state;
    cachedvalue = tmp;
    return tmp;
}

/*!
    \internal
    finds the first possible monthname that \a str1 can match. Starting from \a index;
*/

int QDateTimeEditPrivate::findMonth(const QString &str1, int index)
{
    Q_ASSERT(str1.size() == 3);
    for (int month=index; month<=12; ++month) {
        QString str2 = QDate::shortMonthName(month).toLower();
        bool found = true;
        for (int i=0; i<str1.size(); ++i) {
            if (str1.at(i) != str2.at(i) && !str1.at(i).isSpace()) {
                found = false;
                break;
            }
        }
        if (found)
            return month;
    }
    return -1;
}

/*!
    \internal
    Max number of units that can be changed by this section.
*/

int QDateTimeEditPrivate::maxChange(QDateTimeEditPrivate::Section s)
{
    switch (s) {
        // Time. unit is msec
    case MSecSection: return 999;
    case SecondSection: return 59 * 1000;
    case MinuteSection: return 59 * 60 * 1000;
    case HourSection: return 59 * 60 * 60 * 1000;

        // Date. unit is day
    case DaySection: return 30;
    case MonthShortNameSection:
    case MonthSection: return 365 - 31;
    case YearSection: return (7999 - 1753) * 365;
    case YearTwoDigitsSection: return 100 * 365;

    default: break;
    }
    qFatal("%s passed to maxChange. This should never happen", sectionName(s).toLatin1().constData());
    return -1;
}


int QDateTimeEditPrivate::multiplier(QDateTimeEditPrivate::Section s)
{
    switch (s) {
        // Time. unit is msec
    case MSecSection: return 1;
    case SecondSection: return 1000;
    case MinuteSection: return 60 * 1000;
    case HourSection: return 60 * 60 * 1000;

        // Date. unit is day
    case DaySection: return 1;
    case MonthShortNameSection:
    case MonthSection: return 30;
    case YearSection: return 365;
    case YearTwoDigitsSection: return 365;

    default: break;
    }
    qFatal("%s passed to multiplier. This should never happen", sectionName(s).toLatin1().constData());
    return -1;
}

/*!
    \internal Get a number that str can become which is between min
    and max or -1 if this is not possible.
*/

int QDateTimeEditPrivate::potentialValue(const QString &str, int min, int max, Section s)
{
    int length = sectionSize(s);
    if (s == YearTwoDigitsSection) {
	min -= 2000;
	max -= 2000; // doesn't matter if max is -1 checking for < 0
    }
    QString simplified = str.simplified();
    Q_ASSERT(str != simplified);
    if (simplified.isEmpty()) {
        return min + (s == YearTwoDigitsSection ? 2000 : 0);
    } else if (simplified.toInt() > max && max >= 0) {
        return -1;
    } else {
        QString temp = simplified;
        while (temp.size() < length)
            temp.prepend(QLatin1Char('9'));
        int t = temp.toInt();
        if (t < min) {
            return -1;
        } else if (t <= max || max < 0) {
	    return t + (s == YearTwoDigitsSection ? 2000 : 0);
        }
    }

    int ret = potentialValueHelper(simplified, min, max, length);
    if (ret == -1)
	return -1;
    return ret + (s == YearTwoDigitsSection ? 2000 : 0);
}

/*!
    \internal internal helper function called by potentialValue
*/

int QDateTimeEditPrivate::potentialValueHelper(const QString &str, int min, int max, int length)
{
    if (str.size() == length) {
        const int val = str.toInt();
        if (val < min || val > max)
            return -1;
	QDTEDEBUG << "SUCCESS" << val << "is >=" << min << "and <=" << max;
        return val;
    }

    for (int i=0; i<=str.size(); ++i) {
        for (int j=0; j<10; ++j) {
            QString tmp = str;
            if (i == str.size()) {
                tmp.append(QChar('0' + j));
            } else {
                tmp.insert(i, QChar('0' + j));
            }
            int ret = potentialValueHelper(tmp, min, max, length);
            if (ret != -1)
                return ret;
        }
    }
    return -1;
}

/*!
    \internal
    \reimp
*/

QString QDateTimeEditPrivate::textFromValue(const QVariant &f) const
{
    return q->textFromDateTime(f.toDateTime());
}

/*!
    \internal
    \reimp
*/

QVariant QDateTimeEditPrivate::valueFromText(const QString &f) const
{
    return QVariant(q->dateTimeFromText(f));
}

/*!
    \internal
    \reimp
*/

void QDateTimeEditPrivate::calculateSizeHints() const
{
    if (sizehintdirty && edit) {
        const QFontMetrics fm(q->fontMetrics());
        int h = edit->sizeHint().height();
        int w = 0;
        QString s;
        s = prefix + textFromValue(minimum) + suffix + QLatin1Char(' ');
        w = qMax<int>(w, fm.width(s));
        s = prefix + textFromValue(maximum) + suffix + QLatin1Char(' ');
        w = qMax<int>(w, fm.width(s));
        if (specialvaluetext.size()) {
            s = specialvaluetext;
            w = qMax<int>(w, fm.width(s));
        }
        w += 2; // cursor blinking space

        QStyleOptionSpinBox opt = getStyleOption();
        QSize hint(w, h);
        QSize extra(35,6);
        opt.rect.setSize(hint + extra);
        extra += hint - q->style()->subControlRect(QStyle::CC_SpinBox, &opt,
                                            QStyle::SC_SpinBoxEditField, q).size();
        // get closer to final result by repeating the calculation
        opt.rect.setSize(hint + extra);
        extra += hint - q->style()->subControlRect(QStyle::CC_SpinBox, &opt,
						   QStyle::SC_SpinBoxEditField, q).size();
        hint += extra;

        if (slider)
            hint.rheight() += q->style()->pixelMetric(QStyle::PM_SpinBoxSliderHeight, &opt, q);
        cachedsizehint = hint.expandedTo(QApplication::globalStrut());
        cachedminimumsizehint = hint.expandedTo(QApplication::globalStrut());
        const_cast<QDateTimeEditPrivate *>(this)->sizehintdirty = false;
    }
}
/*!
    \internal Returns whether \a str is a string which value cannot be
    parsed but still might turn into something valid.
*/

QValidator::State QDateTimeEditPrivate::checkIntermediate(const QDateTime &dt,
                                                          const QString &s) const
{
    const bool tooSmall = dt < minimum;
    const bool tooLarge = dt > maximum;
    Q_ASSERT(tooSmall || tooLarge);

    bool found = false;
    for (int i=0; i<sections.size(); ++i) {
        const SectionNode sn = sections.at(i);
        QString t = sectionText(s, sn.section).toLower();
        if (t.contains(space)) {
            if (found) {
		QDTEDEBUG << "Invalid because no spaces";
                return QValidator::Invalid;
	    }
            found = true;
            switch (sn.section) {
            case MonthShortNameSection: {
                if (tooLarge) {
		    QDTEDEBUG << "Invalid because tooLarge and MonthShortNameSection";
                    return QValidator::Invalid;
		}
                int tmp = dt.date().month();
                // I know the first possible month makes the date too early
                while ((tmp = findMonth(t, tmp + 1)) != -1) {
                    const QVariant copy(dt.addMonths(tmp - dt.date().month()));
                    if (copy >= minimum && copy <= maximum)
                        break;
                }
                if (tmp == -1)
                    return QValidator::Invalid;
            }
            case AmPmSection:
            case AmPmLowerCaseSection:
                if (tooSmall && (t.count(space) == 2 || t.contains('m'))) {
                    const QVariant copy(dt.addSecs(12 * 60 * 60));
                    if (copy >= minimum && copy <= maximum)
                        break;
                }
                return QValidator::Invalid;
            default:
                if (tooSmall) {
                    int toMin;
                    int toMax;
		    int multi = multiplier(sn.section);

                    if (sn.section & TimeSectionMask) {
                        if (dt.daysTo(minimum.toDateTime()) != 0) {
			    QDTEDEBUG << "if (dt.daysTo(minimum.toDateTime()) != 0)" << dt.daysTo(minimum.toDateTime());
                            return QValidator::Invalid; // ### assert?
			}
                        toMin = dt.time().msecsTo(minimum.toDateTime().time());
                        if (dt.daysTo(maximum.toDateTime()) > 0) {
                            toMax = -1; // can't get to max
                        } else {
                            toMax = dt.time().msecsTo(maximum.toDateTime().time());
                        }
                    } else {
                        toMin = dt.daysTo(minimum.toDateTime());
                        toMax = dt.daysTo(maximum.toDateTime());
                    }
                    int maxChange = QDateTimeEditPrivate::maxChange(sn.section);
		    int maxChangeUnits = maxChange * multi;
                    if (toMin > maxChangeUnits) {
                        QDTEDEBUG << "invalid because toMin > maxChangeUnits" << toMin << maxChangeUnits << t << dt << minimum.toDateTime()
			    << multi;

                        return QValidator::Invalid;
		    } else if (toMax > maxChangeUnits) {
			toMax = -1; // can't get to max
		    }

                    int min = getDigit(minimum, sn.section);
                    int max = toMax != -1 ? getDigit(maximum, sn.section) : -1;
                    int tmp = potentialValue(t, min, max, sn.section);
		    QDTEDEBUG << tmp << t << min << max << sectionName(sn.section)  << minimum.toDate() << maximum.toDate();
                    if (tmp == -1) {
                        QDTEDEBUG << "invalid because potentialValue(" << t << min << max
				  << sectionName(sn.section) << "returned" << tmp;
                        return QValidator::Invalid;
                    }

                    QVariant var(dt);
                    setDigit(var, sn.section, tmp);
                    if (var > maximum) {
                        QDTEDEBUG << "invalid because" << var.toString() << ">" << maximum.toString();
                        return QValidator::Invalid;
                    }
                } else {
                    // This has no meaning unless setYMD() is automagical about the year parameter.
                    if (sn.section & TimeSectionMask)
                        Q_ASSERT(maximum.toDateTime().daysTo(dt) == 0);
                    return QValidator::Invalid;
                }
            }
        }
    }

    return found ? QValidator::Intermediate : QValidator::Invalid;
}

/*!
    \internal
    For debugging. Returns the name of the section \a s.
*/

QString QDateTimeEditPrivate::sectionName(int s)
{
    switch(s) {
    case QDateTimeEditPrivate::AmPmSection: return "AmPmSection";
    case QDateTimeEditPrivate::AmPmLowerCaseSection: return "AmPmLowerCaseSection";
    case QDateTimeEditPrivate::DaySection: return "DaySection";
    case QDateTimeEditPrivate::HourSection: return "HourSection";
    case QDateTimeEditPrivate::MSecSection: return "MSecSection";
    case QDateTimeEditPrivate::MinuteSection: return "MinuteSection";
    case QDateTimeEditPrivate::MonthSection: return "MonthSection";
    case QDateTimeEditPrivate::MonthShortNameSection: return "MonthShortNameSection";
    case QDateTimeEditPrivate::SecondSection: return "SecondSection";
    case QDateTimeEditPrivate::YearSection: return "YearSection";
    case QDateTimeEditPrivate::YearTwoDigitsSection: return "YearTwoDigitsSection";
    case QDateTimeEditPrivate::NoSection: return "NoSection";
    case QDateTimeEditPrivate::FirstSection: return "FirstSection";
    case QDateTimeEditPrivate::LastSection: return "LastSection";
    default: return "Unknown section " + QString::number(s);
    }
}

/*!
    \internal
    For debugging. Returns the name of the state \a s.
*/

QString QDateTimeEditPrivate::stateName(int s)
{
    switch(s) {
    case QValidator::Invalid: return "Invalid";
    case QValidator::Intermediate: return "Intermediate";
    case QValidator::Acceptable: return "Acceptable";
    default: return "Unknown state " + QString::number(s);
    }
}

#include "moc_qdatetimeedit.cpp"
