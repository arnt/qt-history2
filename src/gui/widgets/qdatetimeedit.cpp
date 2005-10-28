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

#include <math.h>
#include <private/qabstractspinbox_p.h>
#include <private/qdatetime_p.h>
#include <qabstractspinbox.h>
#include <qapplication.h>
#include <qdatetimeedit.h>
#include <qdebug.h>
#include <qevent.h>
#include <qlineedit.h>
#include <qlocale.h>
#include <qset.h>

#ifndef QT_NO_DATETIMEEDIT

//#define QDATETIMEEDIT_QDTEDEBUG
#ifdef QDATETIMEEDIT_QDTEDEBUG
#  define QDTEDEBUG qDebug() << QString("%1:%2").arg(__FILE__).arg(__LINE__)
#  define QDTEDEBUGN qDebug
#else
#  define QDTEDEBUG if (false) qDebug()
#  define QDTEDEBUGN if (false) qDebug
#endif


class QDateTimeEditPrivate : public QAbstractSpinBoxPrivate, public QDateTimeParser
{
    Q_DECLARE_PUBLIC(QDateTimeEdit)
public:
    QDateTimeEditPrivate();
    void readLocaleSettings();

    void emitSignals(EmitPolicy ep, const QVariant &old);
    QString textFromValue(const QVariant &f) const;
    QVariant valueFromText(const QString &f) const;
    void editorCursorPositionChanged(int lastpos, int newpos);
    QVariant validateAndInterpret(QString &input, int &, QValidator::State &state, bool fixup = false) const;

    QVariant valueForPosition(int pos) const;

    void clearSection(int index);
    QString displayText() const { return edit->displayText(); }

    int absoluteIndex(QDateTimeEdit::Section s, int index) const;
    int absoluteIndex(const SectionNode &s) const;
    void updateEdit();
    QVariant stepBy(int index, int steps, bool test = false) const;
    int sectionAt(int pos) const;
    int closestSection(int index, bool forward) const;
    int nextPrevSection(int index, bool forward) const;
    void setSelected(int index, bool forward = false);

    void updateCache(const QVariant &val, const QString &str) const;

    QVariant getMinimum() const { return minimum; }
    QVariant getMaximum() const { return maximum; }
    QString valueToText(const QVariant &var) const { return textFromValue(var); }
    QString getAmPmText(AmPm ap, Case cs) const;
    bool isRightToLeft() const { return qApp->layoutDirection() == Qt::RightToLeft; }

    static QDateTimeEdit::Sections convertSections(QDateTimeParser::Sections s);
    static QDateTimeEdit::Section convertToPublic(QDateTimeParser::Section s);

    QDateTimeEdit::Sections sections;
    mutable bool cacheGuard;

    QString defaultDateFormat, defaultTimeFormat;
    Qt::LayoutDirection layoutDirection;
};

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
    Q_D(QDateTimeEdit);
    d->minimum = QVariant(QDATETIME_MIN);
    d->maximum = QVariant(QDATETIME_MAX);
    d->value = QVariant(QDateTime(QDATE_INITIAL, QTIME_MIN));
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
    d->minimum = QVariant(QDATETIME_MIN);
    d->maximum = QVariant(QDATETIME_MAX);
    d->value = datetime.isValid() ? QVariant(datetime) : QVariant(QDateTime(QDATE_INITIAL, QTIME_MIN));
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
    d->minimum = QVariant(QDATETIME_MIN);
    d->maximum = QVariant(QDATETIME_MAX);
    d->value = QVariant(QDateTime(date.isValid() ? date : QDATE_INITIAL, QTIME_MIN));
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
    d->minimum = QVariant(QDATETIME_MIN);
    d->maximum = QVariant(QDATETIME_MAX);
    d->value = QVariant(QDateTime(QDATE_INITIAL, time.isValid() ? time : QTIME_MIN));
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
        d->setRange(m, (d->variantCompare(d->maximum, m) > 0 ? d->maximum : m));
    }
}

void QDateTimeEdit::clearMinimumDate()
{
    setMinimumDate(QDATE_MIN);
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
        d->setRange((d->variantCompare(d->minimum, m) < 0 ? d->minimum : m), m);
    }
}

void QDateTimeEdit::clearMaximumDate()
{
    setMaximumDate(QDATE_MAX);
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
        d->setRange(m, (d->variantCompare(d->maximum, m) > 0 ? d->maximum : m));
    }
}

void QDateTimeEdit::clearMinimumTime()
{
    setMinimumTime(QTIME_MIN);
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
        d->setRange((d->variantCompare(d->minimum, m) < 0 ? d->minimum : m), m);
    }
}

void QDateTimeEdit::clearMaximumTime()
{
    setMaximumTime(QTIME_MAX);
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
    return d->sections;
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
    if (section == NoSection || !(section & d->sections))
        return;

    d->updateCache(d->value, d->displayText());
    const int size = d->sectionNodes.size();
    int index = d->currentSectionIndex + 1;
    for (int i=0; i<2; ++i) {
        while (index < size) {
            if (d->convertToPublic(d->sectionType(index)) == section) {
                d->edit->setCursorPosition(d->sectionPos(index));
                QDTEDEBUG << d->sectionPos(index);
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
    if (section == QDateTimeEdit::NoSection || !(section & d->sections)) {
        return QString();
    }

    d->updateCache(d->value, d->displayText());
    const int sectionIndex = d->absoluteIndex(section, 0);
    if (sectionIndex < 0)
        return QString();

    return d->sectionText(d->displayText(), sectionIndex, d->sectionPos(sectionIndex));
}

/*!
  \property QDateTimeEdit::displayFormat

  \brief the format used to display the time/date of the date time edit

  This format is the same as the one used described in QDateTime::toString()
  and QDateTime::fromString()

  Example format strings(assuming that the date is 2nd of July 1969):

  \table
  \header \i Format \i Result
  \row \i dd.MM.yyyy    \i 02.07.1969
  \row \i MMM d yy \i Jul 2 69
  \row \i MMMM d yy \i July 2 69
  \endtable

  If you specify an invalid format the format will not be set.

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
        d->sections = d->convertSections(d->display);
        d->clearCache();
        d->currentSectionIndex = qMin(d->currentSectionIndex, d->sectionNodes.size() - 1);
        const bool timeShown = (d->sections & TimeSections_Mask);
        const bool dateShown = (d->sections & DateSections_Mask);
        Q_ASSERT(dateShown || timeShown);
        if (timeShown && !dateShown) {
            setDateRange(d->value.toDate(), d->value.toDate());
        } else if (dateShown && !timeShown) {
            setTimeRange(QTIME_MIN, QTIME_MAX);
            d->value = QVariant(QDateTime(d->value.toDate(), QTime()));
        }
        d->updateEdit();
        d->edit->setCursorPosition(0);
        QDTEDEBUG << 0;
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
        QDTEDEBUG << d->sectionPos(newSection);

        if (select)
            d->setSelected(newSection, true);
        return; }
    default:
        inserted = select = !e->text().isEmpty() && e->text().at(0).isPrint() && !(e->modifiers() & ~Qt::ShiftModifier);
        break;
    }
    QAbstractSpinBox::keyPressEvent(e);
    if (select && !(e->modifiers() & Qt::ShiftModifier) && !d->edit->hasSelectedText()) {
        if (inserted && d->sectionAt(d->edit->cursorPosition()) == QDateTimeParser::NoSectionIndex) {
            QString str = d->displayText();
            int pos = d->edit->cursorPosition();
            QValidator::State state;
            d->validateAndInterpret(str, pos, state);
            if (state == QValidator::Acceptable
                && (d->sectionNode(oldCurrent).count != 1 || d->sectionSize(oldCurrent) == d->sectionMaxSize(oldCurrent))) {
                QDTEDEBUG << "Setting currentsection to" << d->closestSection(d->edit->cursorPosition(), true) << e->key()
                    << oldCurrent;
                const int tmp = d->closestSection(d->edit->cursorPosition(), true);
                if (tmp >= 0)
                    d->currentSectionIndex = tmp;
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
    if (s != d->currentSectionIndex) {
        d->edit->setCursorPosition(d->sectionPos(s));
        QDTEDEBUG << d->sectionPos(s);

    }
    switch (d->sectionType(s)) {
    case QDateTimeParser::NoSection:
    case QDateTimeParser::FirstSection:
    case QDateTimeParser::LastSection:
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
    case QDateTimeParser::NoSection:
    case QDateTimeParser::FirstSection:
    case QDateTimeParser::LastSection:
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
    d->updateCache(d->value, d->displayText());

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
    case QDateTimeParser::NoSection:
    case QDateTimeParser::FirstSection:
    case QDateTimeParser::LastSection: return 0;
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
    : QDateTimeEdit(QTIME_MIN, parent)
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
    : QDateTimeEdit(QDATE_INITIAL, parent)
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
    : QDateTimeParser(QVariant::DateTime)
{
    cacheGuard = false;
    fixday = true;
    allowEmpty = false;
    type = QVariant::DateTime;
    sections = 0;
    cachedDay = -1;
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
    sections = 0;

    readLocaleSettings();
}


void QDateTimeEditPrivate::updateEdit()
{
    const QString newText = specialValue() ? specialValueText : textFromValue(value);
    if (newText == displayText())
        return;
    int selsize = edit->selectedText().size();
    const bool sb = edit->blockSignals(true);

    edit->setText(newText);

    if (!specialValue()) {
        int cursor = sectionPos(currentSectionIndex);
        QDTEDEBUG << "cursor is " << cursor << currentSectionIndex;
        cursor = qBound(0, cursor, displayText().size());
        QDTEDEBUG << cursor;
        if (selsize > 0) {
            edit->setSelection(cursor, selsize);
            QDTEDEBUG << cursor << selsize;
        } else {
            edit->setCursorPosition(cursor);
            QDTEDEBUG << cursor;

        }
    }
    edit->blockSignals(sb);
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

        updateCache(value, displayText());
        const int size = sectionSize(sectionIndex);
        if (forward) {
            edit->setSelection(sectionPos(node), size);
        } else {
            edit->setSelection(sectionPos(node) + size, -size);
        }
    }
}


/*!
  \internal

  Returns the section at index \a index or NoSection if there are no sections there.
*/

int QDateTimeEditPrivate::sectionAt(int pos) const
{
    if (pos < separators.first().size()) {
        return (pos == 0 ? FirstSectionIndex : NoSectionIndex);
    } else if (displayText().size() - pos < separators.last().size() + 1) {
        if (separators.last().size() == 0) {
            return sectionNodes.count() - 1;
        }
        return (pos == displayText().size() ? LastSectionIndex : NoSectionIndex);
    }
    updateCache(value, displayText());

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
    } else if (displayText().size() - pos < separators.last().size() + 1) {
        return forward ? LastSectionIndex : sectionNodes.size() - 1;
    }
    updateCache(value, displayText());
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
    QDTEDEBUG << cursorPos;

    edit->blockSignals(blocked);
}


/*!
  \internal

  updates the cached values
*/

void QDateTimeEditPrivate::updateCache(const QVariant &val, const QString &str) const
{
    if (val != cachedValue || str != cachedText || cacheGuard) {
        cacheGuard = true;
        QString copy = str;
        int unused = edit->cursorPosition();
        QValidator::State unusedState;
        validateAndInterpret(copy, unused, unusedState);
        cacheGuard = false;
    }
}

/*!
  \internal

  parses and validates \a input
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
    if (!specialValueText.isEmpty() && input == specialValueText) {
        state = QValidator::Acceptable;
        return minimum;
    }
    StateNode tmp = parse(input, value, fixup);
    input = tmp.input;
    state = *reinterpret_cast<QValidator::State *>(&tmp.state);
    if (state == QValidator::Acceptable) {
        if (tmp.conflicts) {
            clearCache();
            input = textFromValue(tmp.value);
            updateCache(tmp.value, input);
        } else {
            cachedText = input;
            cachedState = state;
            cachedValue = tmp.value;
        }
    } else {
        clearCache();
    }
    return (tmp.value.isNull() ? getZeroVariant() : tmp.value);
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
    QString str = displayText();
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
    if (sn.type != AmPmSection && (variantCompare(v, minimum) < 0) || (variantCompare(v, maximum) > 0)) {
        const int localmin = getDigit(minimum, sn.type);
        const int localmax = getDigit(maximum, sn.type);

        if (wrapping) {
            // just because we hit the roof in one direction, it
            // doesn't mean that we hit the floor in the other
            if (steps > 0) {
                setDigit(v, sn.type, min);
                if (sn.type != DaySection && sections & DateSectionMask) {
                    int daysInMonth = v.toDate().daysInMonth();
                    if (v.toDate().day() < tmp && v.toDate().day() < daysInMonth)
                        setDigit(v, DaySection, qMin(tmp, daysInMonth));
                }

                if (variantCompare(v, minimum) < 0) {
                    setDigit(v, sn.type, localmin);
                    if (variantCompare(v, minimum) < 0)
                        setDigit(v, sn.type, localmin + 1);
                }
            } else {
                setDigit(v, sn.type, max);
                if (sn.type != DaySection && sections & DateSectionMask) {
                    int daysInMonth = v.toDate().daysInMonth();
                    if (v.toDate().day() < tmp && v.toDate().day() < daysInMonth)
                        setDigit(v, DaySection, qMin(tmp, daysInMonth));
                }

                if (variantCompare(v, maximum) > 0) {
                    setDigit(v, sn.type, localmax);
                    if (variantCompare(v, maximum) > 0)
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

    if (variantCompare(v, minimum) < 0) {
        if (wrapping) {
            QVariant t = v;
            setDigit(t, sn.type, steps < 0 ? max : min);
            int mincmp = variantCompare(t, minimum);
            int maxcmp = variantCompare(t, maximum);
            if (mincmp >= 0 && maxcmp <= 0) {
                v = t;
            } else {
                setDigit(t, sn.type, getDigit(steps < 0 ? maximum : minimum, sn.type));
                mincmp = variantCompare(t, minimum);
                maxcmp = variantCompare(t, maximum);
                if (mincmp >= 0 && maxcmp <= 0) {
                    v = t;
                }
            }
        } else {
            v = value;
        }
    } else if (variantCompare(v, maximum) > 0) {
        if (wrapping) {
            QVariant t = v;
            setDigit(t, sn.type, steps > 0 ? min : max);
            int mincmp = variantCompare(t, minimum);
            int maxcmp = variantCompare(t, maximum);
            if (mincmp >= 0 && maxcmp <= 0) {
                v = t;
            } else {
                setDigit(t, sn.type, getDigit(steps > 0 ? minimum : maximum, sn.type));
                mincmp = variantCompare(t, minimum);
                maxcmp = variantCompare(t, maximum);
                if (mincmp >= 0 && maxcmp <= 0) {
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

    const bool dodate = value.toDate().isValid() && (sections & DateSectionMask);
    const bool datechanged = (ep == AlwaysEmit || old.toDate() != value.toDate());
    const bool dotime = value.toTime().isValid() && (sections & TimeSectionMask);
    const bool timechanged = (ep == AlwaysEmit || old.toTime() != value.toTime());

    updateCache(value, displayText());

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
    updateCache(value, displayText());

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
                QDTEDEBUG << c;
            }
            s = closest;
        }
    }

    if (allowChange && currentSectionIndex != s) {
        QString tmp = displayText();
        int pos = edit->cursorPosition();
        if (q->validate(tmp, pos) != QValidator::Acceptable) {
            interpret(EmitIfChanged);
            if (c == -1) {
                setSelected(s, true);
            } else {
                edit->setCursorPosition(pos);
            }
        }
        updateButtons();
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

QDateTimeEdit::Section QDateTimeEditPrivate::convertToPublic(QDateTimeParser::Section s)
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

QDateTimeEdit::Sections QDateTimeEditPrivate::convertSections(QDateTimeParser::Sections s)
{
    QDateTimeEdit::Sections ret = 0;
    if (s & QDateTimeParser::MSecSection)
        ret |= QDateTimeEdit::MSecSection;
    if (s & QDateTimeParser::SecondSection)
        ret |= QDateTimeEdit::SecondSection;
    if (s & QDateTimeParser::MinuteSection)
        ret |= QDateTimeEdit::MinuteSection;
    if (s & (QDateTimeParser::Hour24Section|QDateTimeParser::Hour12Section))
        ret |= QDateTimeEdit::HourSection;
    if (s & QDateTimeParser::AmPmSection)
        ret |= QDateTimeEdit::AmPmSection;
    if (s & QDateTimeParser::DaySection)
        ret |= QDateTimeEdit::DaySection;
    if (s & QDateTimeParser::MonthSection)
        ret |= QDateTimeEdit::MonthSection;
    if (s & QDateTimeParser::YearSection)
        ret |= QDateTimeEdit::YearSection;

    return ret;
}

QString QDateTimeEditPrivate::getAmPmText(AmPm ap, Case cs) const
{
    Q_Q(const QDateTimeEdit);
    if (ap == AmText) {
        return (cs == UpperCase ? q->tr("AM") : q->tr("am"));
    } else {
        return (cs == UpperCase ? q->tr("PM") : q->tr("pm"));
    }
}

int QDateTimeEditPrivate::absoluteIndex(QDateTimeEdit::Section s, int index) const
{
    for (int i=0; i<sectionNodes.size(); ++i) {
        if (convertToPublic(sectionNodes.at(i).type) == s && index-- == 0) {
            return i;
        }
    }
    return NoSectionIndex;
}

int QDateTimeEditPrivate::absoluteIndex(const SectionNode &s) const
{
    return sectionNodes.indexOf(s);
}



#include "moc_qdatetimeedit.cpp"



#endif // QT_NO_DATETIMEEDIT
