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

#ifndef QDATETIMEEDIT_H
#define QDATETIMEEDIT_H

#include <QtCore/qdatetime.h>
#include <QtGui/qabstractspinbox.h>

class QDateTimeEditPrivate;
class Q_GUI_EXPORT QDateTimeEdit : public QAbstractSpinBox
{
    Q_OBJECT

    Q_ENUMS(Section)
    Q_FLAGS(Sections)
    Q_PROPERTY(QDateTime dateTime READ dateTime WRITE setDateTime)
    Q_PROPERTY(QDate date READ date WRITE setDate)
    Q_PROPERTY(QTime time READ time WRITE setTime)
    Q_PROPERTY(QDate maximumDate READ maximumDate WRITE setMaximumDate RESET clearMaximumDate)
    Q_PROPERTY(QDate minimumDate READ minimumDate WRITE setMinimumDate RESET clearMinimumDate)
    Q_PROPERTY(QTime maximumTime READ maximumTime WRITE setMaximumTime RESET clearMaximumTime)
    Q_PROPERTY(QTime minimumTime READ minimumTime WRITE setMinimumTime RESET clearMinimumTime)
    Q_PROPERTY(Section currentSection READ currentSection WRITE setCurrentSection)
    Q_PROPERTY(Sections displayedSections READ displayedSections)
    Q_PROPERTY(QString displayFormat READ displayFormat WRITE setDisplayFormat)

public:
    enum Section {
	NoSection = 0x0000,
	AmPmSection = 0x0001,
	MSecSection = 0x0002,
	SecondSection = 0x0004,
	MinuteSection = 0x0008,
	HourSection   = 0x0010,
	DaySection    = 0x0100,
	MonthSection  = 0x0200,
	YearSection   = 0x0400,
        TimeSections_Mask = AmPmSection|MSecSection|SecondSection|MinuteSection|HourSection,
        DateSections_Mask = DaySection|MonthSection|YearSection
    };

    Q_DECLARE_FLAGS(Sections, Section)

    explicit QDateTimeEdit(QWidget *parent = 0);
    explicit QDateTimeEdit(const QDateTime &dt, QWidget *parent = 0);
    explicit QDateTimeEdit(const QDate &d, QWidget *parent = 0);
    explicit QDateTimeEdit(const QTime &t, QWidget *parent = 0);

    QDateTime dateTime() const;
    QDate date() const;
    QTime time() const;

    QDate minimumDate() const;
    void setMinimumDate(const QDate &min);
    void clearMinimumDate();

    QDate maximumDate() const;
    void setMaximumDate(const QDate &max);
    void clearMaximumDate();

    void setDateRange(const QDate &min, const QDate &max);

    QTime minimumTime() const;
    void setMinimumTime(const QTime &min);
    void clearMinimumTime();

    QTime maximumTime() const;
    void setMaximumTime(const QTime &max);
    void clearMaximumTime();

    void setTimeRange(const QTime &min, const QTime &max);

    Sections displayedSections() const;
    Section currentSection() const;
    void setCurrentSection(Section section);

    QString sectionText(Section s) const;

    QString displayFormat() const;
    void setDisplayFormat(const QString &format);

    void clear();
signals:
    void dateTimeChanged(const QDateTime &date);
    void timeChanged(const QTime &date);
    void dateChanged(const QDate &date);

public slots:
    void setDateTime(const QDateTime &dateTime);
    void setDate(const QDate &date);
    void setTime(const QTime &time);

protected:
    virtual void changeEvent(QEvent *e);
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void wheelEvent(QWheelEvent *e);
    virtual void focusInEvent(QFocusEvent *e);
    virtual bool focusNextPrevChild(bool next);
    virtual QString mapDateTimeToText(const QDateTime &date) const;
    virtual QDateTime mapTextToDateTime(QString *text, QValidator::State *state) const;
    virtual void stepBy(int steps);
    virtual StepEnabled stepEnabled() const;

private:
    Q_DECLARE_PRIVATE(QDateTimeEdit)
    Q_DISABLE_COPY(QDateTimeEdit)
};

class QTimeEdit : public QDateTimeEdit
{
    Q_OBJECT
public:
    QTimeEdit(QWidget *parent = 0);
    QTimeEdit(const QTime &t, QWidget *parent = 0);
};

class QDateEdit : public QDateTimeEdit
{
    Q_OBJECT
public:
    QDateEdit(QWidget *parent = 0);
    QDateEdit(const QDate &t, QWidget *parent = 0);
};

#endif // QDATETIMEEDIT_H
