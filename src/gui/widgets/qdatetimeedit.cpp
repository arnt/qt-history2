/****************************************************************************
**
** Copyright(C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <math.h>
#include <private/qabstractspinbox_p.h>
#include <qabstractspinbox.h>
#include <qapplication.h>
#include <qdatetimeedit.h>
#include <qdebug.h>
#include <qevent.h>
#include <qlineedit.h>
#include <qlocale.h>
#include <qset.h>

#ifndef QT_NO_DATETIMEEDIT

enum {
    Neither = -1,
    AM = 0,
    PM = 1,
    PossibleAM = 2,
    PossiblePM = 3,
    PossibleBoth = 4
};

enum {
    NoSectionIndex = -1,
    FirstSectionIndex = -2,
    LastSectionIndex = -3,
};



//#define QDATETIMEEDIT_QDTEDEBUG
#ifdef QDATETIMEEDIT_QDTEDEBUG
#  define QDTEDEBUG qDebug() << QString("%1:%2").arg(__FILE__).arg(__LINE__)
#  define QDTEDEBUGN qDebug
#else
#  define QDTEDEBUG if (false) qDebug()
#  define QDTEDEBUGN if (false) qDebug
#endif

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
        Hour12Section   = 0x0010,
        Hour24Section   = 0x0020,
        TimeSectionMask = (AmPmSection|MSecSection|SecondSection|MinuteSection|Hour12Section|Hour24Section),
        Internal = 0x8000,
        DaySection = 0x0100,
        MonthSection = 0x0200,
        YearSection = 0x0400,
        DateSectionMask = (DaySection|MonthSection|YearSection),
        FirstSection = 0x1000|Internal,
        LastSection = 0x2000|Internal
    }; // duplicated from qdatetimeedit.h

    struct SectionNode {
        Section type;
        mutable int pos;
        int count;
    };


    QDateTimeEditPrivate();

    void readLocaleSettings();

    void emitSignals(EmitPolicy ep, const QVariant &old);
    QString textFromValue(const QVariant &f) const;
    QVariant valueFromText(const QString &f) const;
    QVariant validateAndInterpret(QString &input, int &, QValidator::State &state, bool fixup = false) const;
    void editorCursorPositionChanged(int lastpos, int newpos);

    QVariant valueForPosition(int pos) const;

    void clearSection(int index);

    int sectionMaxSize(int index) const;
    int sectionSize(int index) const;
    int sectionMaxSize(Section s, int count) const;

    int sectionPos(int index) const;
    int sectionPos(const SectionNode &sn) const;

    int absoluteIndex(Section s, int index) const;
    int absoluteIndex(const SectionNode &s) const;

    void updateEdit();
    SectionNode sectionNode(int index) const;
    SectionNode sectionNode(Section s, int index) const;
    Section sectionType(int index) const;
    QVariant stepBy(int index, int steps, bool test = false) const;
    QString sectionText(const QString &text, int sectionIndex, int index) const;
    int getDigit(const QVariant &dt, Section s) const;
    void setDigit(QVariant &t, Section s, int newval) const;
    int parseSection(int sectionIndex, QString &txt, int index, QValidator::State &state, int *used = 0) const;
    int absoluteMax(int index) const;
    int absoluteMin(int index) const;
    int sectionAt(int pos) const;
    int closestSection(int index, bool forward) const;
    int nextPrevSection(int index, bool forward) const;
    bool parseFormat(const QString &format);
    void setSelected(int index, bool forward = false);
    QValidator::State checkIntermediate(const QDateTime &dt, const QString &str) const;

    int findMonth(const QString &str1, int monthstart, int sectionIndex, QString *monthName = 0, int *used = 0) const;
    int findDay(const QString &str1, int intDaystart, int sectionIndex, QString *dayName = 0, int *used = 0) const;

    int findAmPm(QString &str1, int index, int *used = 0) const;
    int maxChange(int s) const;
    int potentialValue(const QString &str, int min, int max, int index) const;
    int potentialValueHelper(const QString &str, int min, int max, int size) const;
    int multiplier(int s) const;
    QString sectionName(int s) const;
    QString stateName(int s) const;
    QString displayName(QDateTimeEdit::Sections sec) const;

    QString sectionFormat(int index) const;
    QString sectionFormat(Section s, int count) const;

    QDateTimeEdit::Section convertToPublic(QDateTimeEdit::Section sn) const;

    QDateTimeEdit::Section convertToPublic(QDateTimeEditPrivate::Section sn) const;

    bool isFixedNumericSection(int index) const;

    void updateCache(const QVariant &val, const QString &str) const;

    QString displayFormat, reversedFormat;
    QString defaultDateFormat, defaultTimeFormat;
//    mutable QString oldText;
    QList<SectionNode> sectionNodes;
    SectionNode first, last, none;
    QStringList separators;
    QDateTimeEdit::Sections display;
    mutable int cachedDay;
    mutable int currentSectionIndex;
    mutable int sectionCursorOffset;
    Qt::LayoutDirection layoutDirection;
};

bool operator== (const QDateTimeEditPrivate::SectionNode &s1, const QDateTimeEditPrivate::SectionNode &s2)
{
    return s1.type == s2.type && s1.pos == s2.pos;
}


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
  minus 365 days. We've set the order to month, day, year.

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
  \value Hour12Section
  \value Hour24Section
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
    Q_D(QDateTimeEdit);
    d->minimum = QVariant(DATETIME_MIN);
    d->maximum = QVariant(DATETIME_MAX);
    d->value = QVariant(QDateTime(DATE_INITIAL, TIME_MIN));
    setDisplayFormat(d->defaultDateFormat + QLatin1String(" ") + d->defaultTimeFormat);
}

/*!
  Constructs an empty date time editor with a \a parent. The value
  is set to \a datetime.
*/

QDateTimeEdit::QDateTimeEdit(const QDateTime &datetime, QWidget *parent)
    : QAbstractSpinBox(*new QDateTimeEditPrivate, parent)
{
    Q_D(QDateTimeEdit);
    d->minimum = QVariant(DATETIME_MIN);
    d->maximum = QVariant(DATETIME_MAX);
    d->value = datetime.isValid() ? QVariant(datetime) : QVariant(QDateTime(DATE_INITIAL, TIME_MIN));
    setDisplayFormat(d->defaultDateFormat + QLatin1String(" ") + d->defaultTimeFormat);
}

/*!
  \fn QDateTimeEdit::QDateTimeEdit(const QDate &date, QWidget *parent)

  Constructs an empty date time editor with a \a parent.
  The value is set to \a date.
*/

QDateTimeEdit::QDateTimeEdit(const QDate &date, QWidget *parent)
    : QAbstractSpinBox(*new QDateTimeEditPrivate, parent)
{
    Q_D(QDateTimeEdit);
    d->minimum = QVariant(DATETIME_MIN);
    d->maximum = QVariant(DATETIME_MAX);
    d->value = QVariant(QDateTime(date.isValid() ? date : DATE_INITIAL, TIME_MIN));
    setDisplayFormat(d->defaultDateFormat);
}

/*!
  \fn QDateTimeEdit::QDateTimeEdit(const QTime &time, QWidget *parent)

  Constructs an empty date time editor with a \a parent.
  The value is set to \a time.
*/

QDateTimeEdit::QDateTimeEdit(const QTime &time, QWidget *parent)
    : QAbstractSpinBox(*new QDateTimeEditPrivate, parent)
{
    Q_D(QDateTimeEdit);
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
    Q_D(const QDateTimeEdit);
    return d->value.toDateTime();
}

void QDateTimeEdit::setDateTime(const QDateTime &datetime)
{
    Q_D(QDateTimeEdit);
    if (datetime.isValid()) {
        d->cachedDay = -1;
        d->clearCache();
        d->setValue(QVariant(datetime), EmitIfChanged);
    }
}

/*!
  \property QDateTimeEdit::date
  \brief the QDate that is set in the QDateTimeEdit

  \sa time
*/

QDate QDateTimeEdit::date() const
{
    Q_D(const QDateTimeEdit);
    return d->value.toDate();
}

void QDateTimeEdit::setDate(const QDate &date)
{
    Q_D(QDateTimeEdit);
    if (date.isValid()) {
        d->cachedDay = -1;
        d->clearCache();
        d->setValue(QVariant(QDateTime(date, d->value.toTime())), EmitIfChanged);
    }
}

/*!
  \property QDateTimeEdit::time
  \brief the QTime that is set in the QDateTimeEdit

  \sa date
*/

QTime QDateTimeEdit::time() const
{
    Q_D(const QDateTimeEdit);
    return d->value.toTime();
}

void QDateTimeEdit::setTime(const QTime &time)
{
    Q_D(QDateTimeEdit);
    if (time.isValid()) {
        d->clearCache();
        d->cachedDay = -1;
        d->setValue(QVariant(QDateTime(d->value.toDate(), time)), EmitIfChanged);
    }
}

/*!
  \property QDateTimeEdit::dateTime
  \brief the QDateTime that is set in the QDateTimeEdit

  \sa minimumDate, minimumTime, maximumDate, maximumTime
*/

/*!
  \property QDateTimeEdit::minimumDate

  \brief the minimum date of the date time edit

  When setting this property the \l maximumDate is adjusted if
  necessary, to ensure that the range remains valid. If the date is
  not a valid QDate object, this function does nothing.

  \sa minimumTime, maximumTime, setDateRange()
*/

QDate QDateTimeEdit::minimumDate() const
{
    Q_D(const QDateTimeEdit);
    return d->minimum.toDate();
}

void QDateTimeEdit::setMinimumDate(const QDate &min)
{
    Q_D(QDateTimeEdit);
    if (min.isValid()) {
        const QVariant m(QDateTime(min, d->minimum.toTime()));
        d->setRange(m, qMax(d->maximum, m));
    }
}

void QDateTimeEdit::clearMinimumDate()
{
    setMinimumDate(DATE_MIN);
}

/*!
  \property QDateTimeEdit::maximumDate

  \brief the maximum date of the date time edit

  When setting this property the \l minimumDate is adjusted if
  necessary to ensure that the range remains valid. If the date is
  not a valid QDate object, this function does nothing.

  \sa minimumDate, minimumTime, maximumTime, setDateRange()
*/

QDate QDateTimeEdit::maximumDate() const
{
    Q_D(const QDateTimeEdit);
    return d->maximum.toDate();
}

void QDateTimeEdit::setMaximumDate(const QDate &max)
{
    Q_D(QDateTimeEdit);
    if (max.isValid()) {
        const QVariant m(QDateTime(max, d->maximum.toTime()));
        d->setRange(qMin(d->minimum, m), m);
    }
}

void QDateTimeEdit::clearMaximumDate()
{
    setMaximumDate(DATE_MAX);
}

/*!
  \property QDateTimeEdit::minimumTime

  \brief the minimum time of the date time edit

  When setting this property the \l maximumTime is adjusted if
  necessary, to ensure that the range remains valid. If the time is
  not a valid QTime object, this function does nothing.

  \sa maximumTime, minimumDate, maximumDate, setTimeRange()
*/

QTime QDateTimeEdit::minimumTime() const
{
    Q_D(const QDateTimeEdit);
    return d->minimum.toTime();
}

void QDateTimeEdit::setMinimumTime(const QTime &min)
{
    Q_D(QDateTimeEdit);
    if (min.isValid()) {
        const QVariant m(QDateTime(d->minimum.toDate(), min));
        d->setRange(m, qMax(d->maximum, m));
    }
}

void QDateTimeEdit::clearMinimumTime()
{
    setMinimumTime(TIME_MIN);
}

/*!
  \property QDateTimeEdit::maximumTime

  \brief the maximum time of the date time edit

  When setting this property, the \l minimumTime is adjusted if
  necessary to ensure that the range remains valid. If the time is
  not a valid QTime object, this function does nothing.

  \sa minimumTime, minimumDate, maximumDate, setTimeRange()
*/
QTime QDateTimeEdit::maximumTime() const
{
    Q_D(const QDateTimeEdit);
    return d->maximum.toTime();
}

void QDateTimeEdit::setMaximumTime(const QTime &max)
{
    Q_D(QDateTimeEdit);
    if (max.isValid()) {
        const QVariant m(QDateTime(d->maximum.toDate(), max));
        d->setRange(qMin(d->minimum, m), m);
    }
}

void QDateTimeEdit::clearMaximumTime()
{
    setMaximumTime(TIME_MAX);
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
    Q_D(QDateTimeEdit);
    if (min.isValid() && max.isValid()) {
        d->setRange(QVariant(QDateTime(min, d->minimum.toTime())),
                    QVariant(QDateTime(max, d->maximum.toTime())));
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
    Q_D(QDateTimeEdit);
    if (min.isValid() && max.isValid()) {
        d->setRange(QVariant(QDateTime(d->minimum.toDate(), min)),
                    QVariant(QDateTime(d->maximum.toDate(), max)));
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
    Q_D(const QDateTimeEdit);
    return d->display;
}

/*!
  \property QDateTimeEdit::currentSection

  \brief the current section of the spinbox
  \a setCurrentSection()
*/

QDateTimeEdit::Section QDateTimeEdit::currentSection() const
{
    Q_D(const QDateTimeEdit);
    return d->convertToPublic(d->sectionType(d->currentSectionIndex));
}

void QDateTimeEdit::setCurrentSection(Section section)
{
    Q_D(QDateTimeEdit);
    const QDateTimeEditPrivate::Section s = (QDateTimeEditPrivate::Section)section;
    if (s == QDateTimeEditPrivate::NoSection || !(s & d->display))
        return;

    d->updateCache(d->value, d->edit->displayText());
    const int size = d->sectionNodes.size();
    int index = d->currentSectionIndex + 1;
    for (int i=0; i<2; ++i) {
        while (index < size) {
            if (d->convertToPublic(d->sectionType(index)) == section) {
                d->edit->setCursorPosition(d->sectionPos(index));
                return;
            }
            ++index;
        }
        index = 0;
    }
}

/*!
  \fn QString QDateTimeEdit::sectionText(Section section) const

  Returns the text from the given \a section.

  ### note about not working when not Acceptable

  \sa currentSection()
*/

QString QDateTimeEdit::sectionText(Section section) const
{
    Q_D(const QDateTimeEdit);
    const QDateTimeEditPrivate::Section s = (QDateTimeEditPrivate::Section)section;
    if (s == QDateTimeEditPrivate::NoSection || !(s & d->display))
        return QString();

    d->updateCache(d->value, d->edit->displayText());
    const int sectionIndex = d->absoluteIndex(s, 0);
    if (sectionIndex < 0)
        return QString();

    return d->sectionText(d->edit->displayText(), sectionIndex, d->sectionPos(sectionIndex));
}

/*!
  \property QDateTimeEdit::displayFormat

  \brief the format used to display the time/date of the date time edit

  This format is a subset of the format described in QDateTime::toString()

  These expressions may be used:

  \table
  \header \i Expression \i Output
  \row \i hh
  \i the hour with a leading zero(00 to 23 or 01 to 12 if AM/PM display)
  \row \i mm \i the minute with a leading zero(00 to 59)
  \row \i ss \i the second whith a leading zero(00 to 59)
  \row \i zzz \i the milliseconds with leading zeroes(000 to 999)
  \row \i AP
  \i use AM/PM display. \e AP will be replaced by either "AM" or "PM".
  \row \i ap
  \i use am/pm display. \e ap will be replaced by either "am" or "pm".
  \row \i dd \i the day as number with a leading zero(01 to 31)
  \row \i MM \i the month as number with a leading zero(01 to 12)
  \row \i MMM
  \i the abbreviated localized month name(e.g. 'Jan' to 'Dec').
  Uses QDate::shortMonthName().
  \row \i yy \i the year as two digit number(00 to 99)
  \row \i yyyy \i the year as four digit number(1752 to 8000)
  \endtable

  All other input characters or sequence of characters that are
  enclosed in singlequotes will be treated as text and can be used
  as delimiters.

  Example format strings(assuming that the date is 20 July 1969):

  \table
  \header \i Format \i Result
  \row \i dd.MM.yyyy    \i 20.07.1969
  \row \i MMM d yy \i Jul 20 69
  \endtable

  If you specify an invalid format the format will not be set.

  Multiple instances of the same field is not allowed.A format with
  no valid fields is not allowed either.

  \warning Since QDateTimeEdit internally always operates on a
  QDateTime, changing the format can change the minimum time or
  date and the current time or date. For example:

  \code
  QDateTimeEdit edit;     // default format is "yyyy.MM.dd hh:mm:ss"
  edit.setMinimumDate(QDate(2000, 1, 1));
  edit.setMaximumDate(QDate(2003, 1, 1));
  edit.setDateTime(QDateTime(QDate(2002, 5, 5), QTime(10, 10, 10)));
  edit.setDisplayFormat("hh:mm:ss");

  // edit can no longer display dates. This means that the
  // minimum and maximum date will be set to the current date,
  // e.g. 2002, 5, 5.
  \endcode

  \sa QDateTime::toString(), displayedSections()
*/

QString QDateTimeEdit::displayFormat() const
{
    Q_D(const QDateTimeEdit);
    return d->displayFormat;
}

void QDateTimeEdit::setDisplayFormat(const QString &format)
{
    Q_D(QDateTimeEdit);
    if (d->parseFormat(format)) {
        d->clearCache();
        d->currentSectionIndex = qMin(d->currentSectionIndex, d->sectionNodes.size() - 1);
        const bool timeShown = (d->display & QDateTimeEditPrivate::TimeSectionMask);
        const bool dateShown = (d->display & QDateTimeEditPrivate::DateSectionMask);
        Q_ASSERT(dateShown || timeShown);
        if (timeShown && !dateShown) {
            setDateRange(d->value.toDate(), d->value.toDate());
        } else if (dateShown && !timeShown) {
            setTimeRange(TIME_MIN, TIME_MAX);
            d->value = QVariant(QDateTime(d->value.toDate(), QTime()));
        }
        d->updateEdit();
        d->edit->setCursorPosition(0);
        d->editorCursorPositionChanged(-1, 0);
    }
}

/*!
  \reimp
*/

QSize QDateTimeEdit::sizeHint() const
{
    Q_D(const QAbstractSpinBox);
    ensurePolished();

    const QFontMetrics fm(fontMetrics());
    int h = d->edit->sizeHint().height();
    int w = 0;
    QString s;
    s = d->textFromValue(d->minimum) + QLatin1String("   ");
    w = qMax<int>(w, fm.width(s));
    s = d->textFromValue(d->maximum) + QLatin1String("   ");
    w = qMax<int>(w, fm.width(s));
    if (d->specialValueText.size()) {
        s = d->specialValueText;
        w = qMax<int>(w, fm.width(s));
    }
    w += 2; // cursor blinking space

    QStyleOptionSpinBox opt = d->getStyleOption();
    QSize hint(w, h);
    QSize extra(35, 6);
    opt.rect.setSize(hint + extra);
    extra += hint - style()->subControlRect(QStyle::CC_SpinBox, &opt,
                                            QStyle::SC_SpinBoxEditField, this).size();
    // get closer to final result by repeating the calculation
    opt.rect.setSize(hint + extra);
    extra += hint - style()->subControlRect(QStyle::CC_SpinBox, &opt,
                                            QStyle::SC_SpinBoxEditField, this).size();
    hint += extra;

    opt.rect = rect();
    return style()->sizeFromContents(QStyle::CT_SpinBox, &opt, hint, this)
        .expandedTo(QApplication::globalStrut());
}

/*!
  \reimp
*/

bool QDateTimeEdit::event(QEvent *e)
{
    Q_D(QDateTimeEdit);
    switch (e->type()) {
    case QEvent::ApplicationLayoutDirectionChange:
        setDisplayFormat(d->displayFormat);
        break;
    default:
        break;
    }
    return QAbstractSpinBox::event(e);
}

/*!
  \reimp
*/

void QDateTimeEdit::clear()
{
    Q_D(QDateTimeEdit);
    d->clearSection(d->currentSectionIndex);
}
/*!
  \reimp
*/

void QDateTimeEdit::keyPressEvent(QKeyEvent *e)
{
    Q_D(QDateTimeEdit);
    int oldCurrent = d->currentSectionIndex;
    bool select = true;
    bool inserted = false;

    bool forward = true;
    switch (e->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        d->interpret(AlwaysEmit);
        d->setSelected(d->currentSectionIndex, true);
        e->ignore();
        emit editingFinished();
        return;

    case Qt::Key_Left:
        forward = false;
    case Qt::Key_Right:
        if (!(e->modifiers() & Qt::ControlModifier)) {
            select = false;
            break;
        }
#ifdef Q_WS_MAC
        else {
            select = (e->modifiers() & Qt::ShiftModifier);
            break;
        }
#endif

        // fallthroughs intended
    case Qt::Key_Backtab:
    case Qt::Key_Tab: {
        e->accept();
        if (d->specialValue()) {
            d->edit->setSelection(d->edit->cursorPosition(), 0);
            return;
        }
        if (e->key() == Qt::Key_Backtab || (e->key() == Qt::Key_Tab && e->modifiers() & Qt::ShiftModifier)) {
            forward = false;
        }

        const int newSection = d->nextPrevSection(d->currentSectionIndex, forward);
        d->edit->deselect();
        d->edit->setCursorPosition(d->sectionPos(newSection));
        if (select)
            d->setSelected(newSection, true);
        return; }
    default:
        inserted = select = !e->text().isEmpty() && e->text().at(0).isPrint() && !(e->modifiers() & ~Qt::ShiftModifier);
        break;
    }
    QAbstractSpinBox::keyPressEvent(e);
    if (select && !(e->modifiers() & Qt::ShiftModifier) && !d->edit->hasSelectedText()) {
        if (inserted && d->sectionAt(d->edit->cursorPosition()) == NoSectionIndex) {
            QString str = d->edit->displayText();
            int pos = d->edit->cursorPosition();
            QValidator::State state;
            d->validateAndInterpret(str, pos, state);
            if (state == QValidator::Acceptable
                && (d->sectionNode(oldCurrent).count != 1 || d->sectionSize(oldCurrent) == d->sectionMaxSize(oldCurrent))) {
                d->currentSectionIndex = d->closestSection(d->edit->cursorPosition(), true);
            }
        }
        if (d->currentSectionIndex != oldCurrent) {
            d->setSelected(d->currentSectionIndex);
        }
    }
    if (d->specialValue()) {
        d->edit->setSelection(d->edit->cursorPosition(), 0);
    }
}

/*!
  \reimp
*/

#ifndef QT_NO_WHEELEVENT
void QDateTimeEdit::wheelEvent(QWheelEvent *e)
{
    Q_D(QDateTimeEdit);
    int fw = d->frame ? style()->pixelMetric(QStyle::PM_SpinBoxFrameWidth) : 0;
    QPoint pnt(e->pos() - QPoint(fw, fw));
    pnt.rx() -= d->edit->x();
    int index = d->edit->cursorPositionAt(pnt);
    int s = d->closestSection(index, d->edit->cursorPosition() > index); // should it be > pos?
    if (s != d->currentSectionIndex)
        d->edit->setCursorPosition(d->sectionPos(s));
    switch (d->sectionType(s)) {
    case QDateTimeEditPrivate::NoSection:
    case QDateTimeEditPrivate::FirstSection:
    case QDateTimeEditPrivate::LastSection:
        break;
    default:
        QAbstractSpinBox::wheelEvent(e);
        break;
    }
}
#endif

/*!
  \reimp
*/

void QDateTimeEdit::focusInEvent(QFocusEvent *e)
{
    Q_D(QDateTimeEdit);
    QAbstractSpinBox::focusInEvent(e);
    QString *frm = 0;
    if (d->displayFormat == d->defaultTimeFormat) {
        frm = &d->defaultTimeFormat;
    } else if (d->displayFormat == d->defaultDateFormat) {
        frm = &d->defaultDateFormat;
    }

    if (frm) {
        d->readLocaleSettings();
        setDisplayFormat(*frm);
    }
    bool first;
    switch (e->reason()) {
    case Qt::ShortcutFocusReason:
    case Qt::TabFocusReason: first = true; break;
    case Qt::BacktabFocusReason: first = false; break;
    default: return;
    }
    if (QApplication::isRightToLeft())
        first = !first;
    d->setSelected(first ? 0 : d->sectionNodes.size() - 1);
}

/*!
  \reimp
*/

bool QDateTimeEdit::focusNextPrevChild(bool next)
{
    Q_D(QDateTimeEdit);
    if (!focusWidget())
        return false;

    const int newSection = d->nextPrevSection(d->currentSectionIndex, next);
    switch (d->sectionType(newSection)) {
    case QDateTimeEditPrivate::NoSection:
    case QDateTimeEditPrivate::FirstSection:
    case QDateTimeEditPrivate::LastSection:
        break;
    default:
        return false;
    }
    return QAbstractSpinBox::focusNextPrevChild(next);
}

/*!
  \reimp
*/

void QDateTimeEdit::stepBy(int steps)
{
    Q_D(QDateTimeEdit);
    d->setValue(d->stepBy(d->currentSectionIndex, steps, false), EmitIfChanged);
    d->updateCache(d->value, d->edit->displayText());

    d->setSelected(d->currentSectionIndex);
}

/*!
  This virtual function is used by the date time edit whenever it
  needs to display \a dateTime.

  If you reimplement this, you may also need to reimplement
  valueFromText() and validate().

  \sa dateTimeFromText(), validate()
*/
QString QDateTimeEdit::textFromDateTime(const QDateTime &dateTime) const
{
    Q_D(const QDateTimeEdit);

    return dateTime.toString(d->reversedFormat.isEmpty() ? d->displayFormat : d->reversedFormat);
}


/*!
  Returns an appropriate datetime for the given \a text.

  This virtual function is used by the datetime edit whenever it
  needs to interpret text entered by the user as a value.

  \sa textFromDateTime(), validate()
*/
QDateTime QDateTimeEdit::dateTimeFromText(const QString &text) const
{
    Q_D(const QDateTimeEdit);
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
    Q_D(const QDateTimeEdit);
    QValidator::State state;
    d->validateAndInterpret(text, pos, state);
    return state;
}

/*!
  \reimp
*/


void QDateTimeEdit::fixup(QString &input) const
{
    Q_D(const QDateTimeEdit);
    QValidator::State state;
    int copy = d->edit->cursorPosition();

    d->validateAndInterpret(input, copy, state, true);
}


/*!
  \reimp
*/

QDateTimeEdit::StepEnabled QDateTimeEdit::stepEnabled() const
{
    Q_D(const QDateTimeEdit);
    if (d->readOnly)
        return StepEnabled(0);
    if (d->specialValue()) {
        if (d->minimum == d->maximum)
            return StepEnabled(0);
        return d->wrapping
            ? StepEnabled(StepDownEnabled|StepUpEnabled)
            : StepEnabled(StepUpEnabled);
    }
    switch (d->sectionType(d->currentSectionIndex)) {
    case QDateTimeEditPrivate::NoSection:
    case QDateTimeEditPrivate::FirstSection:
    case QDateTimeEditPrivate::LastSection: return 0;
    default: break;
    }
    if (!style()->styleHint(QStyle::SH_SpinControls_DisableOnBounds)
        || d->wrapping)
        return StepEnabled(StepUpEnabled | StepDownEnabled);

    QAbstractSpinBox::StepEnabled ret = 0;

    QVariant v = d->stepBy(d->currentSectionIndex, 1, true);
    if (v != d->value) {
        ret |= QAbstractSpinBox::StepUpEnabled;
    }
    v = d->stepBy(d->currentSectionIndex, -1, true);
    if (v != d->value) {
        ret |= QAbstractSpinBox::StepDownEnabled;
    }

    return ret;
}


/*!
  \class QTimeEdit
  \brief The QTimeEdit class provides a widget for editing times based on
  the QDateTimeEdit widget.

  \ingroup basic
  \mainclass

  \sa QDateEdit QDateTimeEdit
*/

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

QTimeEdit::QTimeEdit(const QTime &time, QWidget *parent)
    : QDateTimeEdit(time, parent)
{
}

/*!
  \class QDateEdit
  \brief The QDateEdit class provides a widget for editing dates based on
  the QDateTimeEdit widget.

  \ingroup basic
  \mainclass

  \sa QTimeEdit QDateTimeEdit
*/

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
    cachedDay = -1;
    sectionCursorOffset = -1;
    currentSectionIndex = FirstSectionIndex;

    if (currentSectionIndex >= sectionNodes.size())
        qFatal("%d currentSectionIndex >= sectionNodes.size()) %d %d", __LINE__,
               currentSectionIndex, sectionNodes.size());

    layoutDirection = QApplication::layoutDirection();
    first.type = FirstSection;
    last.type = LastSection;
    none.type = NoSection;
    first.pos = 0;
    last.pos = -1;
    none.pos = -1;

    readLocaleSettings();
}

/*!
  \internal
  \reimp
*/

void QDateTimeEditPrivate::emitSignals(EmitPolicy ep, const QVariant &old)
{
    Q_Q(QDateTimeEdit);
    if (ep == NeverEmit) {
        return;
    }
    pendingEmit = false;

    const bool dodate = value.toDate().isValid() && (display & DateSectionMask);
    const bool datechanged = (ep == AlwaysEmit || old.toDate() != value.toDate());
    const bool dotime = value.toTime().isValid() && (display & TimeSectionMask);
    const bool timechanged = (ep == AlwaysEmit || old.toTime() != value.toTime());

    updateCache(value, edit->displayText());

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
    Q_Q(QDateTimeEdit);
    if (ignoreCursorPositionChanged || specialValue())
        return;
    updateCache(value, edit->displayText());

    const bool allowChange = !edit->hasSelectedText();
    const bool forward = oldpos <= newpos;
    ignoreCursorPositionChanged = true;
    int s = sectionAt(newpos);
    if (s == NoSectionIndex && forward && newpos > 0) {
        s = sectionAt(newpos - 1);
    }

    int c = newpos;

    const int selstart = edit->selectionStart();
    const int selSection = sectionAt(selstart);
    const int l = selSection != -1 ? sectionSize(selSection) : 0;

    if (s == NoSectionIndex) {
        if (l > 0 && selstart == sectionPos(selSection) && edit->selectedText().size() == l) {
            s = selSection;
            if (allowChange)
                setSelected(selSection, true);
            c = -1;
        } else {
            int closest = closestSection(newpos, forward);
            c = sectionPos(closest) + (forward ? 0 : qMax<int>(0, sectionSize(closest)));

            if (allowChange) {
                edit->setCursorPosition(c);
            }
            s = closest;
        }
    }

    if (allowChange && currentSectionIndex != s) {
        QString tmp = edit->displayText();
        int pos = edit->cursorPosition();
        if (q->validate(tmp, pos) != QValidator::Acceptable) {
            interpret(EmitIfChanged);
            if (c == -1) {
                setSelected(s, true);
            } else {
                edit->setCursorPosition(pos);
            }
        }
        updateSpinBox();
    }
    QDTEDEBUG << "currentSectionIndex is set to" << sectionName(sectionType(s)) << oldpos << newpos
              << "was" << sectionName(sectionType(currentSectionIndex));
    currentSectionIndex = s;
    if (currentSectionIndex >= sectionNodes.size())
        qFatal("%d currentSectionIndex >= sectionNodes.size()) %d %d", __LINE__,
               currentSectionIndex, sectionNodes.size());

    ignoreCursorPositionChanged = false;
}

/*!
  \internal

  Try to get the format from the local settings
*/
void QDateTimeEditPrivate::readLocaleSettings()
{
    const QLocale loc;
    defaultTimeFormat = loc.timeFormat(QLocale::ShortFormat);
    defaultDateFormat = loc.dateFormat(QLocale::ShortFormat);
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
    switch (s) {
    case Hour24Section: case Hour12Section: return t.toTime().hour();
    case MinuteSection: return t.toTime().minute();
    case SecondSection: return t.toTime().second();
    case MSecSection: return t.toTime().msec();
    case YearSection: return t.toDate().year();
    case MonthSection: return t.toDate().month();
    case DaySection: return t.toDate().day();
    case AmPmSection: return t.toTime().hour() > 11 ? 1 : 0;

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

    switch (section) {
    case Hour24Section: case Hour12Section: hour = newVal; break;
    case MinuteSection: minute = newVal; break;
    case SecondSection: second = newVal; break;
    case MSecSection: msec = newVal; break;
    case YearSection: year = newVal; break;
    case MonthSection: month = newVal; break;
    case DaySection: day = newVal; break;
    case AmPmSection: hour = (newVal == 0 ? hour % 12 : (hour % 12) + 12); break;
    default:
        qFatal("%s passed to setDigit. This should never happen", sectionName(section).toLatin1().constData());
        break;
    }

    if (section != DaySection) {
        day = qMax<int>(cachedDay, day);
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
  whether or not to modify the internal cachedDay variable. This is
  necessary because the function is called from the const function
  QDateTimeEdit::stepEnabled() as well as QDateTimeEdit::stepBy().
*/

QVariant QDateTimeEditPrivate::stepBy(int sectionIndex, int steps, bool test) const
{
    Q_Q(const QDateTimeEdit);
    QVariant v = value;
    QString str = edit->displayText();
    int pos = edit->cursorPosition();
    const SectionNode sn = sectionNode(sectionIndex);

    int val;
    // to make sure it behaves reasonably when typing something and then stepping in non-tracking mode
    if (!test && pendingEmit) {
        if (q->validate(str, pos) != QValidator::Acceptable) {
            v = value;
        } else {
            v = valueFromText(str);
        }
        val = getDigit(v, sn.type);
    } else {
        val = getDigit(value, sn.type);
    }

    val += steps;

    const int min = absoluteMin(sectionIndex);
    const int max = absoluteMax(sectionIndex);

    if (val < min) {
        val = (wrapping ? max - (min - val) + 1 : min);
    } else if (val > max) {
        val = (wrapping ? min + val - max - 1 : max);
    }

    const int tmp = v.toDate().day();
    setDigit(v, sn.type, val); // if this sets year or month it will make
    // sure that days are lowered if needed.

    // changing one section should only modify that section, if possible
    if (sn.type != AmPmSection && (v < minimum || v > maximum)) {
        const int localmin = getDigit(minimum, sn.type);
        const int localmax = getDigit(maximum, sn.type);

        if (wrapping) {
            // just because we hit the roof in one direction, it
            // doesn't mean that we hit the floor in the other
            if (steps > 0) {
                setDigit(v, sn.type, min);
                if (sn.type != DaySection && display & DateSectionMask) {
                    int daysInMonth = v.toDate().daysInMonth();
                    if (v.toDate().day() < tmp && v.toDate().day() < daysInMonth)
                        setDigit(v, DaySection, qMin(tmp, daysInMonth));
                }

                if (v < minimum) {
                    setDigit(v, sn.type, localmin);
                    if (v < minimum)
                        setDigit(v, sn.type, localmin + 1);
                }
            } else {
                setDigit(v, sn.type, max);
                if (sn.type != DaySection && display & DateSectionMask) {
                    int daysInMonth = v.toDate().daysInMonth();
                    if (v.toDate().day() < tmp && v.toDate().day() < daysInMonth)
                        setDigit(v, DaySection, qMin(tmp, daysInMonth));
                }

                if (v > maximum) {
                    setDigit(v, sn.type, localmax);
                    if (v > maximum)
                        setDigit(v, sn.type, localmax - 1);
                }
            }
        } else {
            setDigit(v, sn.type, (steps > 0 ? localmax : localmin));
        }
    }
    if (!test && tmp != v.toDate().day() && sn.type != DaySection) {
        // this should not happen when called from stepEnabled
        cachedDay = qMax<int>(tmp, cachedDay);
    }

    if (v < minimum) {
        if (wrapping) {
            QVariant t = v;
            setDigit(t, sn.type, steps < 0 ? max : min);
            if (t >= minimum && t <= maximum) {
                v = t;
            } else {
                setDigit(t, sn.type, getDigit(steps < 0 ? maximum : minimum, sn.type));
                if (t >= minimum && t <= maximum) {
                    v = t;
                }
            }
        } else {
            v = value;
        }
    } else if (v > maximum) {
        if (wrapping) {
            QVariant t = v;
            setDigit(t, sn.type, steps > 0 ? min : max);
            if (t >= minimum && t <= maximum) {
                v = t;
            } else {
                setDigit(t, sn.type, getDigit(steps > 0 ? minimum : maximum, sn.type));
                if (t >= minimum && t <= maximum) {
                    v = t;
                }
            }
        } else {
            v = value;
        }
    }

    const QVariant ret = bound(v, value, steps);
//     if (!test) {
//         clearCache();
//         updateCache(ret, textFromValue(ret));
//     }
    return ret;
}

/*!
  \

  Returns the absolute maximum for a section
*/

int QDateTimeEditPrivate::absoluteMax(int s) const
{
    const SectionNode sn = sectionNode(s);
    switch (sn.type) {
    case Hour24Section: return 23;
    case Hour12Section: return 11;
    case MinuteSection:
    case SecondSection: return 59;
    case MSecSection: return 999;
    case YearSection: return sn.count == 4 ? 7999 : 2099;
    case MonthSection: return 12;
    case DaySection: return 31;
    case AmPmSection: return 1;
    default: break;
    }
    qFatal("%s passed to max. This should never happen", sectionName(s).toLatin1().constData());
    return -1;

}

/*!
  \internal

  Returns the absolute minimum for a section
*/

int QDateTimeEditPrivate::absoluteMin(int s) const
{
    const SectionNode sn = sectionNode(s);
    switch (sn.type)
    case Hour24Section:{
    case Hour12Section:
    case MinuteSection:
    case SecondSection:
    case MSecSection: return 0;
    case YearSection: return sn.count == 4 ? 1752 : 2000;
    case MonthSection:
    case DaySection: return 1;
    case AmPmSection: return 0;
    default: break;
    }
    qFatal("%s passed to min. This should never happen", sectionName(s).toLatin1().constData());
    return -1;
}

/*!
  \internal

  Returns a copy of the sectionNode for the Section \a s.
*/

QDateTimeEditPrivate::SectionNode QDateTimeEditPrivate::sectionNode(int sectionIndex) const
{
    if (sectionIndex == FirstSectionIndex) {
        return first;
    } else if (sectionIndex == LastSectionIndex) {
        return last;
    } else if (sectionIndex == NoSectionIndex) {
        return none;
    }
    Q_ASSERT(sectionIndex >= 0 && sectionIndex < sectionNodes.size());
    return sectionNodes.at(sectionIndex);
}

QDateTimeEditPrivate::SectionNode QDateTimeEditPrivate::sectionNode(Section s, int sectionIndex) const
{
    return sectionNode(absoluteIndex(s, sectionIndex));
}


QDateTimeEditPrivate::Section QDateTimeEditPrivate::sectionType(int sectionIndex) const
{
    return sectionNode(sectionIndex).type;
}

void QDateTimeEditPrivate::updateEdit()
{
//    qDebug() << edit->cursorPosition() << sectionPos(currentSectionIndex) << sectionCursorOffset << edit->displayText();
    int selsize = edit->selectedText().size();
    const QString newText = specialValue() ? specialValueText : textFromValue(value);
    const bool sb = edit->blockSignals(true);
    edit->setText(newText);

    if (!specialValue()) {
        int cursor = sectionPos(currentSectionIndex) + sectionCursorOffset;
//        qDebug() << "cursor is " << cursor << currentSectionIndex << sectionCursorOffset;
        cursor = qBound(0, cursor, edit->displayText().size());
//        qDebug() << cursor;
        if (selsize > 0) {
            edit->setSelection(cursor, selsize);
        } else {
            edit->setCursorPosition(cursor);
        }
    }
    edit->blockSignals(sb);
}

/*!
  \internal

  Returns the starting position for section \a s.
*/

int QDateTimeEditPrivate::sectionPos(int sectionIndex) const
{
    return sectionPos(sectionNode(sectionIndex));
}

int QDateTimeEditPrivate::sectionPos(const SectionNode &sn) const
{
    switch (sn.type) {
    case FirstSection: return 0;
    case LastSection: return edit->displayText().size() - 1;
    default: break;
    }
    if (sn.pos == -1)
        qDebug() << sectionName(sn.type) << sectionNodes.indexOf(sn);
    Q_ASSERT(sn.pos != -1);
    return sn.pos;
}

int QDateTimeEditPrivate::absoluteIndex(Section s, int index) const
{
    Q_ASSERT(s != FirstSection);
    Q_ASSERT(s != LastSection);
    Q_ASSERT(s != NoSection);
    const QDateTimeEdit::Section ss = convertToPublic(s);

    for (int i=0; i<sectionNodes.size(); ++i) {
        if (convertToPublic(sectionNodes.at(i).type) == ss && index-- == 0) {
            return i;
        }
    }
    return NoSectionIndex;
}

int QDateTimeEditPrivate::absoluteIndex(const SectionNode &s) const
{
    return sectionNodes.indexOf(s);
}

/*!
  \internal

  Selects the section \a s. If \a forward is false selects backwards.
*/

void QDateTimeEditPrivate::setSelected(int sectionIndex, bool forward)
{
    if (specialValue()) {
        edit->selectAll();
    } else {
        const SectionNode &node = sectionNode(sectionIndex);
        if (node.type == NoSection || node.type == LastSection || node.type == FirstSection)
            return;

        updateCache(value, edit->displayText());
        const int size = sectionSize(sectionIndex);
        if (forward) {
            edit->setSelection(sectionPos(node), size);
        } else {
            edit->setSelection(sectionPos(node) + size, -size);
        }
    }
}

/*!
  \internal helper function for parseFormat. removes quotes that are
  not escaped and removes the escaping on those that are escaped

*/

static QString unquote(const QString &str)
{
    const char quote = '\'';
    const char slash = '\\';
    const char zero = '0';
    QString ret;
    QChar status = zero;
    for (int i=0; i<str.size(); ++i) {
        if (str.at(i) == quote) {
            if (status != quote) {
                status = quote;
            } else if (!ret.isEmpty() && str.at(i - 1) == slash) {
                ret[ret.size() - 1] = quote;
            } else {
                status = zero;
            }
        } else {
            ret += str.at(i);
        }
    }
    return ret;
}
/*!
  \internal

  Parses the format \a newFormat. If successful, returns true and
  sets up the format. Else keeps the old format and returns false.

*/

static int countRepeat(const QString &str, int index)
{
    Q_ASSERT(index >= 0 && index < str.size());
    int count = 1;
    const QChar ch = str.at(index);
    while (index + count < str.size() && str.at(index + count) == ch)
        ++count;
    return count;
}

// checks if there is an unqoted 'AP' or 'ap' in the string
static bool hasUnquotedAP(const QString &f)
{
    const char quote = '\'';
    bool inquote = false;
    QChar status = QLatin1Char('0');
    for (int i=0; i<f.size(); ++i) {
        if (f.at(i) == quote) {
            inquote = !inquote;
        } else if (!inquote && f.at(i).toUpper() == QLatin1Char('A')) {
            return true;
        }
    }
    return false;
}

bool QDateTimeEditPrivate::parseFormat(const QString &newFormat)
{
    const char quote = '\'';
    const char slash = '\\';
    const char zero = '0';
    if (newFormat == displayFormat && !newFormat.isEmpty()
        && layoutDirection == QApplication::layoutDirection()) {
        return true;
    }
    layoutDirection = QApplication::layoutDirection();

//    qDebug("parseFormat: %s", newFormat.toLatin1().constData());

    const bool ap = hasUnquotedAP(newFormat);
    QList<SectionNode> newSectionNodes;
    QDateTimeEdit::Sections newDisplay = 0;
    QStringList newSeparators;
    int i, index = 0;
    int add = 0;
    QChar status = zero;
    for (i = 0; i<newFormat.size(); ++i) {
        if (newFormat.at(i) == quote) {
            ++add;
            if (status != quote) {
                status = quote;
            } else if (newFormat.at(i - 1) != slash) {
                status = zero;
            }
        } else if (i < newFormat.size() && status != quote) {
            const int repeat = qMin(4, countRepeat(newFormat, i));
            const char sect = newFormat.at(i).toLatin1();
            switch (sect) {
            case 'H':
            case 'h': {
                const Section hour = (ap && sect == 'h') ? Hour12Section : Hour24Section;
                const SectionNode sn = { hour, i - add, qMin(2, repeat) };
                newSectionNodes << sn;
                newSeparators << unquote(newFormat.mid(index, i - index));
                i += sn.count - 1;
                index = i + 1;
                newDisplay |= QDateTimeEdit::HourSection;
                break; }
            case 'm': {
                const SectionNode sn = { MinuteSection, i - add, qMin(2, repeat) };
                newSectionNodes << sn;
                newSeparators << unquote(newFormat.mid(index, i - index));
                i += sn.count - 1;
                index = i + 1;
                newDisplay |= QDateTimeEdit::MinuteSection;
                break; }
            case 's': {
                const SectionNode sn = { SecondSection, i - add, qMin(2, repeat) };
                newSectionNodes << sn;
                newSeparators << unquote(newFormat.mid(index, i - index));
                i += qMin(2, repeat) - 1;
                index = i + 1;
                newDisplay |= QDateTimeEdit::SecondSection;
                break; }

            case 'z': {
                const SectionNode sn = { MSecSection, i - add, (repeat < 3 ? 1 : 3) };
                newSectionNodes << sn;
                newSeparators << unquote(newFormat.mid(index, i - index));
                i += sn.count - 1;
                index = i + 1;
                newDisplay |= QDateTimeEdit::MSecSection;
                break; }
            case 'A':
            case 'a': {
                const bool cap = newFormat.at(i) == QLatin1Char('A');
                const SectionNode sn = { AmPmSection, i - add, (cap ? 1 : 0) };
                newSectionNodes << sn;
                newSeparators << unquote(newFormat.mid(index, i - index));
                newDisplay |= QDateTimeEdit::AmPmSection;
                if (i + 1 < newFormat.size()
                    && newFormat.at(i+1) == (cap ? QLatin1Char('P') : QLatin1Char('p'))) {
                    ++i;
                }
                index = i + 1;
                break; }
            case 'y':
                if (repeat >= 2) {
                    const bool four = repeat >= 4;
                    const SectionNode sn = { YearSection, i - add, four ? 4 : 2 };
                    newSectionNodes << sn;
                    newSeparators << unquote(newFormat.mid(index, i - index));
                    i += sn.count - 1;
                    index = i + 1;
                    newDisplay |= QDateTimeEdit::YearSection;
                }
                break;
            case 'M': {
                const SectionNode sn = { MonthSection, i - add, repeat };
                newSectionNodes << sn;
                newSeparators << unquote(newFormat.mid(index, i - index));
                i += sn.count - 1;
                index = i + 1;
                newDisplay |= QDateTimeEdit::MonthSection;
                break; }

            case 'd': {
                const SectionNode sn = { DaySection, i - add, repeat };
                newSectionNodes << sn;
                newSeparators << unquote(newFormat.mid(index, i - index));
                i += sn.count - 1;
                index = i + 1;
                newDisplay |= QDateTimeEdit::DaySection;
                break; }

            default: break;
            }
        }
    }
    if (newSectionNodes.isEmpty()) {
        QDTEDEBUGN("Could not parse format. No sections in format '%s'.", newFormat.toLatin1().constData());
        return false;
    }

    newSeparators << (index < newFormat.size() ? unquote(newFormat.mid(index)) : QString());


    displayFormat = newFormat;
    separators = newSeparators;
    sectionNodes = newSectionNodes;
    display = newDisplay;
    last.pos = -1;
    reversedFormat.clear();
    if (QApplication::isRightToLeft()) {
        for (int i=newSectionNodes.size() - 1; i>=0; --i) {
            reversedFormat += newSeparators.at(i + 1);
            reversedFormat += sectionFormat(i);
        }
        reversedFormat += newSeparators.at(0);
    }

//     for (int i=0; i<sectionNodes.size(); ++i) {
//         qDebug() << sectionName(sectionNodes.at(i).type) << sectionNodes.at(i).count;
//     }

    QDTEDEBUG << newFormat << displayFormat;
//    qDebug("separators:\n'%s'", separators.join("\n").toLatin1().constData());

//    qDebug("display is [%0x] '%s'", (uint)display, qPrintable(displayName(display)));

    return true;
}

/*!
  \internal

  Returns the section at index \a index or NoSection if there are no sections there.
*/

int QDateTimeEditPrivate::sectionAt(int pos) const
{
    if (pos < separators.first().size()) {
        return (pos == 0 ? FirstSectionIndex : NoSectionIndex);
    } else if (edit->displayText().size() - pos < separators.last().size() + 1) {
        if (separators.last().size() == 0) {
            return sectionNodes.count() - 1;
        }
        return (pos == edit->displayText().size() ? LastSectionIndex : NoSectionIndex);
    }
    updateCache(value, edit->displayText());

    for (int i=0; i<sectionNodes.size(); ++i) {
        const int tmp = sectionPos(i);
        if (pos < tmp + sectionSize(i)) {
            return (pos < tmp ? -1 : i);
        }
    }
    return -1;
}

/*!
  \internal

  Returns the closest section of index \a index. Searches forward
  for a section if \a forward is true. Otherwise searches backwards.
*/

int QDateTimeEditPrivate::closestSection(int pos, bool forward) const
{
    Q_ASSERT(pos >= 0);
    if (pos < separators.first().size()) {
        return forward ? 0 : FirstSectionIndex;
    } else if (edit->displayText().size() - pos < separators.last().size() + 1) {
        return forward ? LastSectionIndex : sectionNodes.size() - 1;
    }
    updateCache(value, edit->displayText());
    for (int i=0; i<sectionNodes.size(); ++i) {
        const int tmp = sectionPos(sectionNodes.at(i));
        if (pos < tmp + sectionSize(i)) {
            if (pos < tmp && !forward) {
                return i-1;
            }
            return i;
        } else if (i == sectionNodes.size() - 1 && pos > tmp) {
            return i;
        }
    }
    qWarning("closestSection return NoSection. This should not happen");
    return NoSectionIndex;
}

/*!
  \internal

  Returns a copy of the section that is before or after \a current, depending on \a forward.
*/

int QDateTimeEditPrivate::nextPrevSection(int current, bool forward) const
{
    if (QApplication::isRightToLeft())
        forward = !forward;


    switch (current) {
    case FirstSectionIndex: return forward ? 0 : FirstSectionIndex;
    case LastSectionIndex: return (forward ? LastSectionIndex : sectionNodes.size() - 1);
    case NoSectionIndex: return FirstSectionIndex;
    default: break;
    }
    Q_ASSERT(current >= 0 && current < sectionNodes.size());

    current += (forward ? 1 : -1);
    if (current >= sectionNodes.size()) {
        return LastSectionIndex;
    } else if (current < 0) {
        return FirstSectionIndex;
    }

    return current;
}

/*!
  \internal

  Clears the text of section \a s.
*/

void QDateTimeEditPrivate::clearSection(int index)
{
    const char space = ' ';
    int cursorPos = edit->cursorPosition();
    bool blocked = edit->blockSignals(true);
    QString t = edit->text();
    const int pos = sectionPos(index);
    if (pos == -1) {
        qWarning("%s:%d this is unexpected", __FILE__, __LINE__);
        return;
    }
    const int size = sectionSize(index);
    t.replace(pos, size, QString().fill(space, size));
    edit->setText(t);
    edit->setCursorPosition(cursorPos);
    edit->blockSignals(blocked);
}

/*!
  \internal

  Returns the size of section \a s.
*/

int QDateTimeEditPrivate::sectionSize(int sectionIndex) const
{
    if (sectionIndex < 0)
        return 0;
    Q_ASSERT(sectionIndex < sectionNodes.size());
    if (sectionIndex == sectionNodes.size() - 1) {
        return edit->displayText().size() - sectionPos(sectionIndex) - separators.last().size();
    } else {
        return sectionPos(sectionIndex + 1) - sectionPos(sectionIndex) - separators.at(sectionIndex + 1).size();
    }
}


int QDateTimeEditPrivate::sectionMaxSize(Section s, int count) const
{
    int mcount = 12;
    QString(*nameFunction)(int) = &QDate::longMonthName;

    switch (s) {
    case FirstSection:
    case NoSection:
    case LastSection: return 0;

    case AmPmSection: {
        int lower = qMin(QDateTimeEdit::tr("pm").size(), QDateTimeEdit::tr("am").size());
        int upper = qMin(QDateTimeEdit::tr("PM").size(), QDateTimeEdit::tr("AM").size());
        return qMin(4, qMin(lower, upper));
    }

    case Hour24Section:
    case Hour12Section:
    case MinuteSection:
    case SecondSection: return 2;
    case DaySection: nameFunction = &QDate::longDayName; mcount = 7;
        // fall through
    case MonthSection:
        if (count <= 3) {
            return qMax(2, count);
        } else {
            int ret = 0;
            for (int i=1; i<=mcount; ++i) { // ### optimize? cache results?
                ret = qMax(nameFunction(i).size(), ret);
            }
            return ret;
        }

    case MSecSection: return 3;
    case YearSection: return count;

    case Internal:
    case TimeSectionMask:
    case DateSectionMask: qWarning("Invalid section %s", sectionName(s).toLatin1().constData());
    }
    return -1;
}


int QDateTimeEditPrivate::sectionMaxSize(int index) const
{
    const SectionNode sn = sectionNode(index);
    return sectionMaxSize(sn.type, sn.count);
}

/*!
  \internal

  Returns the text of section \a s. This function operates on the
  arg text rather than edit->text().
*/


QString QDateTimeEditPrivate::sectionText(const QString &text, int sectionIndex, int index) const
{
    const SectionNode &sn = sectionNode(sectionIndex);
    switch (sn.type) {
    case NoSectionIndex:
    case FirstSectionIndex:
    case LastSectionIndex:
        return QString();
    default: break;
    }

    return text.mid(index, sectionSize(sectionIndex));
}

/*!
  \internal

  Parses the part of \a text that corresponds to \a s and returns
  the value of that field. Sets *stateptr to the right state if
  stateptr != 0.
*/

int QDateTimeEditPrivate::parseSection(int sectionIndex, QString &text, int index,
                                       QValidator::State &state, int *usedptr) const
{
    state = QValidator::Invalid;
    int num = 0;
    const SectionNode sn = sectionNode(sectionIndex);
    Q_ASSERT(sn.type != NoSection && sn.type != FirstSection && sn.type != LastSection);

    QString sectiontext = text.mid(index, sectionMaxSize(sectionIndex));
    if (sectionIndex == currentSectionIndex && !cachedText.isEmpty()) {
        const int diff = text.size() - textFromValue(value).size();
        if (diff < 0) { // text has been removed
            if (sectionIndex + 1 < sectionNodes.size() || !separators.last().isEmpty()) {
                sectiontext.chop(qAbs(diff));
            }
        }
    }

    QDTEDEBUG << "sectionValue for" << sectionName(sn.type)
              << "with text" << text << "and st" << sectiontext
              << index << sectiontext;

    int used = 0;
    if (false && sectiontext.trimmed().isEmpty()) {
        state = QValidator::Intermediate;
    } else {
        switch (sn.type) {
        case AmPmSection: {
            const int ampm = findAmPm(sectiontext, sectionIndex, &used);
            switch (ampm) {
            case AM: // sectiontext == AM
            case PM: // sectiontext == PM
                num = ampm;
                state = QValidator::Acceptable;
                break;
            case PossibleAM: // sectiontext => AM
            case PossiblePM: // sectiontext => PM
                num = ampm - 2;
                state = QValidator::Intermediate;
                break;
            case PossibleBoth: // sectiontext => AM|PM
                num = 0;
                state = QValidator::Intermediate;
                break;
            case Neither:
                state = QValidator::Invalid;
                QDTEDEBUG << "invalid because findAmPm(" << sectiontext << ") returned -1";
                break;
            default:
                QDTEDEBUGN("This should never happen(findAmPm returned %d", ampm);
                break;
            }
            if (state != QValidator::Invalid) {
                QString str = text;
                text.replace(index, used, sectiontext.left(used));
            }
            break;
        }
        case MonthSection:
        case DaySection:
            if (sn.count >= 3) {
                if (sn.type == MonthSection) {
                    num = findMonth(sectiontext.toLower(), 1, sectionIndex, &sectiontext, &used);
                } else {
                    num = findDay(sectiontext.toLower(), 1, sectionIndex, &sectiontext, &used);
                }

                if (num != -1) {
                    state = (used == sectiontext.size() ? QValidator::Acceptable : QValidator::Intermediate);
                    QString str = text;
                    text.replace(index, used, sectiontext.left(used));
                } else {
                    state = QValidator::Intermediate;
                }
                break;
            }
            // fall through
        case YearSection:
        case Hour12Section:
        case Hour24Section:
        case MinuteSection:
        case SecondSection:
        case MSecSection: {
            if (sectiontext.isEmpty()) {
                num = 0;
                used = 0;
                state = QValidator::Intermediate;
            } else {
            const int absMax = absoluteMax(sectionIndex);
            QLocale loc;
            bool ok = true;
            int last = -1;
            used = -1;

            for (int digits=1; digits<=sectiontext.size(); ++digits) {
                int tmp = (int)loc.toUInt(sectiontext.left(digits), &ok, 10);

                if (!ok) {
                    if (sn.count == 1) {
                        used = digits - 1;
                        ok = true;
                    } else if (currentSectionIndex == sectionIndex) {
                        used = digits - 1;
                    } else {
                        used = -1;
                        last = -1;
                    }
                    break;
                } else if (tmp > absMax) {
                    used = -1;
                    break;
                }
                last = tmp;
                used = digits;
            }

            if (last == -1) {
                if (sn.count == 1 && !sectiontext.at(0).isDigit()
                    && separators.at(sectionIndex + 1).startsWith(sectiontext.at(0))) {
                    state = QValidator::Intermediate;
                } else {
                    state = QValidator::Invalid;
                    QDTEDEBUG << "invalid because" << sectiontext << "can't become a uint" << last << ok;
                }
            } else {
                num += last;
                if (num < absoluteMin(sectionIndex) || num > absMax || !ok) {
                    state = (used != -1 && used < sectionMaxSize(sectionIndex) ? QValidator::Intermediate : QValidator::Invalid);
                    if (state == QValidator::Invalid) {
                        QDTEDEBUG << "invalid because" << sectiontext << "num is" << num
                                  << "outside absoluteMin and absoluteMax" << absoluteMin(sectionIndex)
                                  << absoluteMax(sectionIndex);
                    }
                } else {
                    if (sn.count > 1 && used < sn.count) {
                        state = QValidator::Intermediate;
                    } else {
                        state = QValidator::Acceptable;
                    }
                }
            }
            }
            break; }
        default: qFatal("NoSection or Internal. This should never happen"); break;
        }
    }
//     if (state == QValidator::Invalid)
//         qDebug() << "sectionValue" << text << index << sectionIndex << sectionName(sn.type);

    if (usedptr)
        *usedptr = used;

    return (state != QValidator::Invalid ? num : -1);
}

/*!
  \internal
  \reimp
*/

QVariant QDateTimeEditPrivate::validateAndInterpret(QString &input, int &/*position*/,
                                                    QValidator::State &state, bool fixup) const
{
    if (input.isEmpty()) {
        state = QValidator::Invalid;
        return getZeroVariant();
    } else if (cachedText == input && !fixup) {
        state = cachedState;
        return cachedValue;
    }
    QVariant tmp;
    SectionNode sn = {NoSection, 0, false};
    int pos = 0;
    bool conflicts = false;

    QDTEDEBUG << "validateAndInterpret" << input;
    bool specval = false;
    if (!specialValueText.isEmpty() && input == specialValueText) {
        specval = true;
        state = QValidator::Acceptable;
        tmp = minimum;
        goto end;
    }

    {
        QString deb;
        int year, month, day, hour12, hour, minute, second, msec, ampm, dayofweek;
        const QDateTime &dt = value.toDateTime();
        year = dt.date().year();
        month = dt.date().month();
        day = dt.date().day();
        hour = dt.time().hour();
        hour12 = -1;
        minute = dt.time().minute();
        second = dt.time().second();
        msec = dt.time().msec();
        dayofweek = dt.date().dayOfWeek();
        deb += "Start : dayofweek is " + QString::number(dayofweek) + "\n";
        int old = dayofweek;
        ampm = -1;
        QSet<int*> isSet;
        int num;
        QValidator::State tmpstate;
        int *current;

        state = QValidator::Acceptable;

        pos = 0;
        for (int index=0; state != QValidator::Invalid && index<sectionNodes.size(); ++index) {
            if (dayofweek != old) {
                deb += "dayofweek changed to " + QString::number(dayofweek) + QString::number(index - 1) + sectionName(sectionType(index - 1))
                       +  "\n";
                old = dayofweek;
            }

            QString sep = input.mid(pos, separators.at(index).size());

            if (sep != separators.at(index)) {
                QDTEDEBUG << "invalid because" << sep << "!=" << separators.at(index)
                          << index << pos << currentSectionIndex;
                state = QValidator::Invalid;
                goto end;
            }
            pos += separators.at(index).size();
            sectionNodes[index].pos = pos;
            current = 0;
            sn = sectionNodes.at(index);
            int used;

            num = parseSection(index, input, pos, tmpstate, &used);
            QDTEDEBUG << "sectionValue" << sectionName(sectionType(index)) << input << pos << used << stateName(tmpstate);
            if (fixup && tmpstate == QValidator::Intermediate && isFixedNumericSection(index) && used < sn.count) {
                input.insert(pos, QString().fill(QLatin1Char('0'), sn.count - used));
                num = parseSection(index, input, pos, tmpstate, &used);
            }
            pos += qMax(0, used);

            state = qMin<QValidator::State>(state, tmpstate);
            QDTEDEBUG << index << sectionName(sectionType(index)) << "is set to" << pos << "state is" << stateName(state);


            if (state != QValidator::Invalid) {
                switch (sn.type) {
                case Hour24Section: current = &hour; break;
                case Hour12Section: current = &hour12; break;
                case MinuteSection: current = &minute; break;
                case SecondSection: current = &second; break;
                case MSecSection: current = &msec; break;
                case YearSection: current = &year; num = (num == 0 ? DATE_INITIAL.year() : num); break;
                case MonthSection: current = &month; break;
                case DaySection:
                    if (sn.count >= 3) {
                        current = &dayofweek;
                    } else {
                        current = &day; num = qMax<int>(1, num);
                    }
                    break;
                case AmPmSection: current = &ampm; break;
                default:
                    qFatal("%s found in sections validateAndInterpret. This should never happen",
                           sectionName(sn.type).toLatin1().constData());
                    break;
                }
                Q_ASSERT(current);
                if (isSet.contains(current) && *current != num) {
                    conflicts = true;
                    if (index != currentSectionIndex || num == -1) {
                        continue;
                    }
                }
                if (num != -1)
                    *current = num;
                isSet.insert(current);
            }
        }
        if (state != QValidator::Invalid && input.mid(pos) != separators.last()) {
            QDTEDEBUG << "1invalid because" << input.mid(pos)
                      << "!=" << separators.last() << pos;
            state = QValidator::Invalid;
        }

        if (state == QValidator::Invalid) {
            tmp = getZeroVariant();
        } else {
            const QDate date(year, month, day);
            const int diff = dayofweek - date.dayOfWeek() && isSet.contains(&dayofweek);
            if (diff != 0 && state == QValidator::Acceptable) {
                conflicts = true;
                const SectionNode &sn = sectionNode(currentSectionIndex);
                if (sn.type == DaySection && sn.count >= 3) {
                    day -= diff;
                    if (day < 0) {
                        day += 7;
                    } else if (day > date.daysInMonth()) {
                        day -= 7;
                    }
//                    qDebug() << year << month << day << dayofweek << diff << old << QDate(year, month, day).dayOfWeek();
                    Q_ASSERT(QDate(year, month, day).dayOfWeek() == dayofweek); // ### remove those
                    Q_ASSERT(qAbs(QDate(year, month, day).daysTo(date)) <= 7);
                }
            }

            if (isSet.contains(&hour12)) {
                const bool hasHour = isSet.contains(&hour);
                if (ampm == -1) {
                    if (hasHour) {
                        ampm = (hour < 12 ? 0 : 1);
                    } else {
                        ampm = 0; // no way to tell if this is am or pm so I assume am
                    }
                }
                hour12 = (ampm == 0 ? hour12 % 12 : (hour12 % 12) + 12);
                if (!hasHour) {
                    hour = hour12;
                } else if (hour != hour12) {
                    conflicts = true;
                }
            } else if (ampm != -1 && isSet.contains(&hour)) {
                if ((ampm == 0) != (hour < 12)) {
                    conflicts = true;
                }
            }

            bool fixday = false;
            if (sectionType(currentSectionIndex) == DaySection) {
                cachedDay = day;
            } else if (cachedDay > day) {
                day = cachedDay;
                fixday = true;
            }

            if (!QDate::isValid(year, month, day)) {
                if (day < 32) {
                    cachedDay = day;
                }
                if (day > 28 && QDate::isValid(year, month, 1)) {
                    fixday = true;
                }
            }
            if (fixday && state == QValidator::Acceptable) {
                day = qMin<int>(day, QDate(year, month, 1).daysInMonth());

                int i = 0;
                int dayIndex;
                while ((dayIndex = absoluteIndex(DaySection, i++)) != -1) {
                    input.replace(sectionPos(dayIndex), sectionSize(dayIndex), QString::number(day));
                }
            }

            tmp = QVariant(QDateTime(QDate(year, month, day), QTime(hour, minute, second, msec)));
            QDTEDEBUG << year << month << day << hour << minute << second << msec;

        }
        QDTEDEBUGN("'%s' => '%s'(%s)", input.toLatin1().constData(),
                   tmp.toString().toLatin1().constData(), stateName(state).toLatin1().constData());
    }
end:
    if (tmp.toDateTime().isValid()) {
        if (!specval && state != QValidator::Invalid && tmp < minimum) {
            state = checkIntermediate(tmp.toDateTime(), input);
        } else {
            if (tmp > maximum)
                state = QValidator::Invalid;
            QDTEDEBUG << "not checking intermediate because tmp is" << tmp << minimum << maximum;
        }
    }
    cachedState = state;
    if (state == QValidator::Acceptable) {
        if (conflicts) {
            clearCache();
            input = textFromValue(tmp);
            updateCache(tmp, input);
        }
        cachedValue = tmp;
        cachedText = input;
    }

    return tmp;
}

/*!
  \internal finds the first possible monthname that \a str1 can
  match. Starting from \a index; str should already by lowered
*/

int QDateTimeEditPrivate::findMonth(const QString &str1, int startMonth, int sectionIndex, QString *usedMonth, int *used) const
{
    int bestMatch = -1;
    int bestCount = 0;
    if (!str1.isEmpty()) {
    const SectionNode sn = sectionNode(sectionIndex);
    Q_ASSERT(sn.type == MonthSection);
    QString(*nameFunction)(int) = sn.count == 3
                                  ? &QDate::shortMonthName
                                  : &QDate::longMonthName;

//    qDebug() << "findMonth" << str1 << startMonth << sectionIndex;

    for (int month=startMonth; month<=12; ++month) {
        QString str2 = nameFunction(month).toLower();

        if (str1.startsWith(str2)) {
            if (used) {
                QDTEDEBUG << __LINE__ << "used is set to" << str2.size();
                *used = str2.size();
            }
            if (usedMonth)
                *usedMonth = nameFunction(month);
            return month;
        }

        const int limit = qMin(str1.size(), str2.size());

        QDTEDEBUG << "limit is" << limit << str1 << str2;
        bool found = true;
        for (int i=0; i<limit; ++i) {
            if (str1.at(i) != str2.at(i)) {
                if (i > bestCount) {
                    bestCount = i;
                    bestMatch = month;
                }
                found = false;
                break;
            }

        }
        if (found) {
            if (used) {
                *used = limit;
            }
            if (usedMonth)
                *usedMonth = nameFunction(month);
            QDTEDEBUG << __LINE__ << "used is set to" << limit << *usedMonth;

            return month;
        }
    }
        if (usedMonth && bestMatch != -1)
            *usedMonth = nameFunction(bestMatch);

    }
    if (used) {
        QDTEDEBUG << __LINE__ << "used is set to" << bestCount;
        *used = bestCount;
    }
    return bestMatch;
}

int QDateTimeEditPrivate::findDay(const QString &str1, int startDay, int sectionIndex, QString *usedDay, int *used) const
{
    int bestMatch = -1;
    int bestCount = 0;
    if (!str1.isEmpty()) {
    const SectionNode sn = sectionNode(sectionIndex);
    Q_ASSERT(sn.type == DaySection);
    QString(*nameFunction)(int) = sn.count == 3
                                  ? &QDate::shortDayName
                                  : &QDate::longDayName;

    for (int day=startDay; day<=7; ++day) {
        QString str2 = nameFunction(day).toLower();

        if (str1.startsWith(str2)) {
            if (used)
                *used = str2.size();
            if (usedDay)
                *usedDay = nameFunction(day);
            return day;
        }

        const int limit = qMin(str1.size(), str2.size());
        bool found = true;
        for (int i=0; i<limit; ++i) {
            if (str1.at(i) != str2.at(i) && !str1.at(i).isSpace()) {
                if (i > bestCount) {
                    bestCount = i;
                    bestMatch = day;
                }
                found = false;
                break;
            }

        }
        if (found) {
            if (used)
                *used = limit;
            if (usedDay)
                *usedDay = nameFunction(day);
            return day;
        }
    }
    if (usedDay && bestMatch != -1)
        *usedDay = nameFunction(bestMatch);
    }
    if (used)
        *used = bestCount;

    return bestMatch;
}

/*!
  \internal

  returns
  0 if str == QDateTimeEdit::tr("AM")
  1 if str == QDateTimeEdit::tr("PM")
  2 if str can become QDateTimeEdit::tr("AM")
  3 if str can become QDateTimeEdit::tr("PM")
  4 if str can become QDateTimeEdit::tr("PM") and can become QDateTimeEdit::tr("AM")
  -1 can't become anything sensible

*/

int QDateTimeEditPrivate::findAmPm(QString &str, int index, int *used) const
{
    const SectionNode s = sectionNode(index);
    Q_ASSERT(s.type == AmPmSection);
    if (used)
        *used = str.size();
    if (str.trimmed().isEmpty()) {
        return PossibleBoth;
    }
    const char space = ' ';
    int size = sectionMaxSize(index);

    enum {
        amindex = 0,
        pmindex = 1
    };
    QString ampm[2];
    if (s.count == 1) {
        ampm[amindex] = QDateTimeEdit::tr("AM");
        ampm[pmindex] = QDateTimeEdit::tr("PM");
    } else {
        ampm[amindex] = QDateTimeEdit::tr("am");
        ampm[pmindex] = QDateTimeEdit::tr("pm");
    }
    for (int i=0; i<2; ++i)
        ampm[i].truncate(size);

    QDTEDEBUG << "findAmPm" << str << ampm[0] << ampm[1];

    if (str.indexOf(ampm[amindex], 0, Qt::CaseInsensitive) == 0) {
        str = ampm[amindex];
        return AM;
    } else if (str.indexOf(ampm[pmindex], 0, Qt::CaseInsensitive) == 0) {
        str = ampm[pmindex];
        return PM;
    } else if (str.count(space) == 0 && str.size() >= size) {
        return Neither;
    }
    size = qMin(size, str.size());

    bool broken[2] = {false, false};
    for (int i=0; i<size; ++i) {
        if (str.at(i) != space) {
            for (int j=0; j<2; ++j) {
                if (!broken[j]) {
                    int index = ampm[j].indexOf(str.at(i));
                    QDTEDEBUG << "looking for" << str.at(i)
                              << "in" << ampm[j] << "and got" << index;
                    if (index == -1) {
                        if (str.at(i).category() == QChar::Letter_Uppercase) {
                            index = ampm[j].indexOf(str.at(i).toLower());
                            QDTEDEBUG << "trying with" << str.at(i).toLower()
                                      << "in" << ampm[j] << "and got" << index;
                        } else if (str.at(i).category() == QChar::Letter_Lowercase) {
                            index = ampm[j].indexOf(str.at(i).toUpper());
                            QDTEDEBUG << "trying with" << str.at(i).toUpper()
                                      << "in" << ampm[j] << "and got" << index;
                        }
                        if (index == -1) {
                            broken[j] = true;
                            if (broken[amindex] && broken[pmindex]) {
                                QDTEDEBUG << str << "didn't make it";
                                return Neither;
                            }
                            continue;
                        } else {
                            str[i] = ampm[j].at(index); // fix case
                        }
                    }
                    ampm[j].remove(index, 1);
                }
            }
        }
    }
    if (!broken[pmindex] && !broken[amindex])
        return PossibleBoth;
    return (!broken[amindex] ? PossibleAM : PossiblePM);
}

/*!
  \internal
  Max number of units that can be changed by this section.
*/

int QDateTimeEditPrivate::maxChange(int index) const
{
    const SectionNode sn = sectionNode(index);
    switch (sn.type) {
        // Time. unit is msec
    case MSecSection: return 999;
    case SecondSection: return 59 * 1000;
    case MinuteSection: return 59 * 60 * 1000;
    case Hour24Section: case Hour12Section: return 59 * 60 * 60 * 1000;

        // Date. unit is day
    case DaySection: return 30;
    case MonthSection: return 365 - 31;
    case YearSection: return sn.count == 2
            ? 100 * 365
            : (7999 - 1752) * 365;
    default: qFatal("%s passed to maxChange. This should never happen", sectionName(sectionType(index)).toLatin1().constData());
    }
    return -1;
}


int QDateTimeEditPrivate::multiplier(int index) const
{
    switch (sectionType(index)) {
        // Time. unit is msec
    case MSecSection: return 1;
    case SecondSection: return 1000;
    case MinuteSection: return 60 * 1000;
    case Hour24Section: case Hour12Section: return 60 * 60 * 1000;

        // Date. unit is day
    case DaySection: return 1;
    case MonthSection: return 30;
    case YearSection: return 365;

    default: break;
    }
    qFatal("%s passed to multiplier. This should never happen", sectionName(sectionType(index)).toLatin1().constData());
    return -1;
}

bool QDateTimeEditPrivate::isFixedNumericSection(int index) const
{
    const SectionNode sn = sectionNode(index);
    switch (sectionType(index)) {
    case MSecSection:
    case SecondSection:
    case MinuteSection:
    case Hour24Section: case Hour12Section: return sn.count != 1;
    case MonthSection:
    case DaySection: return sn.count == 2;
    case AmPmSection: return false;
    case YearSection: return true;
    default: qFatal("This should not happen %d %s", index, qPrintable(sectionName(sn.type)));
    }
    return false;
}



void QDateTimeEditPrivate::updateCache(const QVariant &val, const QString &str) const
{
    if (val != cachedValue || str != cachedText) {
        QString copy = str;
        int unused = edit->cursorPosition();
        QValidator::State unusedState;
        validateAndInterpret(copy, unused, unusedState);
    }
}

/*!
  \internal Get a number that str can become which is between min
  and max or -1 if this is not possible.
*/


QString QDateTimeEditPrivate::sectionFormat(int index) const
{
    const SectionNode sn = sectionNode(index);
    return sectionFormat(sn.type, sn.count);
}

QString QDateTimeEditPrivate::sectionFormat(Section s, int count) const
{
    QChar fillChar;
    switch (s) {
    case AmPmSection: return count == 1 ? QLatin1String("AP") : QLatin1String("ap");
    case MSecSection: fillChar = QLatin1Char('z'); break;
    case SecondSection: fillChar = QLatin1Char('s'); break;
    case MinuteSection: fillChar = QLatin1Char('m'); break;
    case Hour24Section: fillChar = QLatin1Char('H'); break;
    case Hour12Section: fillChar = QLatin1Char('h'); break;
    case DaySection: fillChar = QLatin1Char('d'); break;
    case MonthSection: fillChar = QLatin1Char('M'); break;
    case YearSection: fillChar = QLatin1Char('y'); break;
    default:
        qFatal("%s passed to sectionFormat. This should never happen", sectionName(s).toLatin1().constData());
        return QString();
    }
    Q_ASSERT(!fillChar.isNull());
    QString str;
    str.fill(fillChar, count);
    return str;
}

QDateTimeEdit::Section QDateTimeEditPrivate::convertToPublic(QDateTimeEditPrivate::Section s) const
{
    switch (s & ~Internal) {
    case AmPmSection: return QDateTimeEdit::AmPmSection;
    case MSecSection: return QDateTimeEdit::MSecSection;
    case SecondSection: return QDateTimeEdit::SecondSection;
    case MinuteSection: return QDateTimeEdit::MinuteSection;
    case DaySection: return QDateTimeEdit::DaySection;
    case MonthSection: return QDateTimeEdit::MonthSection;
    case YearSection: return QDateTimeEdit::YearSection;
    case Hour12Section:
    case Hour24Section: return QDateTimeEdit::HourSection;
    case FirstSection:
    case NoSection:
    case LastSection: break;
    }
    return QDateTimeEdit::NoSection;
}


/*!
  \internal Get a number that str can become which is between min
  and max or -1 if this is not possible.
*/

int QDateTimeEditPrivate::potentialValue(const QString &str, int min, int max, int index) const
{
    const SectionNode sn = sectionNode(index);

    int size = sectionMaxSize(index);
    const int add = (sn.type == YearSection && sn.count == 2) ? 2000 : 0;
    min -= add;
    max -= add; // doesn't matter if max is -1 checking for < 0
    QString simplified = str.simplified();
    if (simplified.isEmpty()) {
        return min + add;
    } else if (simplified.toInt() > max && max >= 0) {
        return -1;
    } else {
        QString temp = simplified;
        while (temp.size() < size)
            temp.prepend(QLatin1Char('9'));
        const int t = temp.toInt();
        if (t < min) {
            return -1;
        } else if (t <= max || max < 0) {
            return t + add;
        }
    }

    const int ret = potentialValueHelper(simplified, min, max, size);
    if (ret == -1)
        return -1;
    return ret + add;
}

/*!
  \internal internal helper function called by potentialValue
*/

int QDateTimeEditPrivate::potentialValueHelper(const QString &str, int min, int max, int size) const
{
    if (str.size() == size) {
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
            int ret = potentialValueHelper(tmp, min, max, size);
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
    Q_Q(const QDateTimeEdit);
    return q->textFromDateTime(f.toDateTime());
}

/*!
  \internal
  \reimp
*/

QVariant QDateTimeEditPrivate::valueFromText(const QString &f) const
{
    Q_Q(const QDateTimeEdit);
    return QVariant(q->dateTimeFromText(f));
}

/*!
  \internal Returns whether \a str is a string which value cannot be
  parsed but still might turn into something valid.
*/

QValidator::State QDateTimeEditPrivate::checkIntermediate(const QDateTime &dt,
                                                          const QString &s) const
{
    const char space = ' ';

    Q_ASSERT(dt < minimum);

    bool found = false;
    for (int i=0; i<sectionNodes.size(); ++i) {
        const SectionNode &sn = sectionNodes.at(i);
        QString t = sectionText(s, i, sn.pos).toLower();
        if (t.contains(space) || t.size() < sectionMaxSize(i)) {
            if (found) {
                QDTEDEBUG << "invalid because no spaces";
                return QValidator::Invalid;
            }
            found = true;
            switch (sn.type) {
            case AmPmSection:
                switch (findAmPm(t, i)) {
                case AM:
                case PM: qFatal("%d This should not happen", __LINE__); return QValidator::Acceptable;
                case Neither: return QValidator::Invalid;
                case PossibleAM:
                case PossiblePM:
                case PossibleBoth: {
                    const QVariant copy(dt.addSecs(12 * 60 * 60));
                    if (copy >= minimum && copy <= maximum)
                        return QValidator::Intermediate;
                    return QValidator::Invalid; }
                }
            case MonthSection:
                if (sn.count >= 3) {
                    int tmp = dt.date().month();
                    // I know the first possible month makes the date too early
                    while ((tmp = findMonth(t, tmp + 1, sn.count)) != -1) {
                        const QVariant copy(dt.addMonths(tmp - dt.date().month()));
                        if (copy >= minimum && copy <= maximum)
                            break;
                    }
                    if (tmp == -1) {
                        return QValidator::Invalid;
                    }
                }
                // fallthrough

            default: {
                int toMin;
                int toMax;
                int multi = multiplier(i);

                if (sn.type & TimeSectionMask) {
                    if (dt.daysTo(minimum.toDateTime()) != 0) {
                        QDTEDEBUG << "if (dt.daysTo(minimum.toDateTime()) != 0)" << dt.daysTo(minimum.toDateTime());
                        return QValidator::Invalid;
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
                int maxChange = QDateTimeEditPrivate::maxChange(i);
                int maxChangeUnits = maxChange * multi;
                if (toMin > maxChangeUnits) {
                    QDTEDEBUG << "invalid because toMin > maxChangeUnits" << toMin
                              << maxChangeUnits << t << dt << minimum.toDateTime()
                              << multi;

                    return QValidator::Invalid;
                } else if (toMax > maxChangeUnits) {
                    toMax = -1; // can't get to max
                }

                int min = getDigit(minimum, sn.type);
                int max = toMax != -1 ? getDigit(maximum, sn.type) : -1;
                int tmp = potentialValue(t, min, max, i);
                QDTEDEBUG << tmp << t << min << max << sectionName(sn.type)  << minimum.toDate() << maximum.toDate();
                if (tmp == -1) {
                    QDTEDEBUG << "invalid because potentialValue(" << t << min << max
                              << sectionName(sn.type) << "returned" << tmp;
                    return QValidator::Invalid;
                }

                QVariant var(dt);
                setDigit(var, sn.type, tmp);
                if (var > maximum) {
                    QDTEDEBUG << "invalid because" << var.toString() << ">" << maximum.toString();
                    return QValidator::Invalid;
                }
                break; }
            }
        }
    }
    return found ? QValidator::Intermediate : QValidator::Invalid;
}

/*!
  \internal
  For debugging. Returns the name of the section \a s.
*/

QString QDateTimeEditPrivate::sectionName(int s) const
{
    switch (s) {
    case QDateTimeEditPrivate::AmPmSection: return QLatin1String("AmPmSection");
    case QDateTimeEditPrivate::DaySection: return QLatin1String("DaySection");
    case QDateTimeEditPrivate::Hour24Section: return QLatin1String("Hour24Section");
    case QDateTimeEditPrivate::Hour12Section: return QLatin1String("Hour12Section");
    case QDateTimeEditPrivate::MSecSection: return QLatin1String("MSecSection");
    case QDateTimeEditPrivate::MinuteSection: return QLatin1String("MinuteSection");
    case QDateTimeEditPrivate::MonthSection: return QLatin1String("MonthSection");
    case QDateTimeEditPrivate::SecondSection: return QLatin1String("SecondSection");
    case QDateTimeEditPrivate::YearSection: return QLatin1String("YearSection");
    case QDateTimeEditPrivate::NoSection: return QLatin1String("NoSection");
    case QDateTimeEditPrivate::FirstSection: return QLatin1String("FirstSection");
    case QDateTimeEditPrivate::LastSection: return QLatin1String("LastSection");
    default: return QLatin1String("Unknown section ") + QString::number(s);
    }
}

/*!
  \internal
  For debugging. Returns the name of the state \a s.
*/

QString QDateTimeEditPrivate::stateName(int s) const
{
    switch (s) {
    case QValidator::Invalid: return "Invalid";
    case QValidator::Intermediate: return "Intermediate";
    case QValidator::Acceptable: return "Acceptable";
    default: return "Unknown state " + QString::number(s);
    }
}

QString QDateTimeEditPrivate::displayName(QDateTimeEdit::Sections sec) const
{
    QString ret;
    if (sec & QDateTimeEdit::AmPmSection)
        ret += QLatin1String("AmPmSection, ");
    if (sec & QDateTimeEdit::HourSection)
        ret += QLatin1String("HourSection, ");
    if (sec & QDateTimeEdit::MinuteSection)
        ret += QLatin1String("MinuteSection, ");
    if (sec & QDateTimeEdit::SecondSection)
        ret += QLatin1String("SecondSection, ");
    if (sec & QDateTimeEdit::MSecSection)
        ret += QLatin1String("MSecSection, ");
    if (sec & QDateTimeEdit::YearSection)
        ret += QLatin1String("YearSection, ");
    if (sec & QDateTimeEdit::MonthSection)
        ret += QLatin1String("MonthSection, ");
    if (sec & QDateTimeEdit::DaySection)
        ret += QLatin1String("DaySection, ");
    ret.chop(2);
    return ret;
}

#include "moc_qdatetimeedit.cpp"

#endif // QT_NO_DATETIMEEDIT
