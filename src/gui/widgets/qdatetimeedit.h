#ifndef QDATETIMEEDIT_H
#define QDATETIMEEDIT_H

#include <qdatetime.h>
#include <qabstractspinbox.h>

class QDateTimeEditPrivate;
class QDateTimeEdit : public QAbstractSpinBox
{
    Q_OBJECT

    Q_ENUMS(Section)
    Q_PROPERTY(QDateTime dateTime READ dateTime WRITE setDateTime)
    Q_PROPERTY(QDate date READ date WRITE setDate)
    Q_PROPERTY(QTime time READ time WRITE setTime)
    Q_PROPERTY(QDate maximumDate READ maximumDate WRITE setMaximumDate RESET clearMaximumDate)
    Q_PROPERTY(QDate minimumDate READ minimumDate WRITE setMinimumDate RESET clearMinimumDate)
    Q_PROPERTY(QTime maximumTime READ maximumTime WRITE setMaximumTime RESET clearMaximumTime)
    Q_PROPERTY(QTime minimumTime READ minimumTime WRITE setMinimumTime RESET clearMinimumTime)
    Q_PROPERTY(SectionFlags currentSection READ currentSection WRITE setCurrentSection)
    Q_PROPERTY(Section display READ display)
    Q_PROPERTY(QString format READ format WRITE setFormat)

public:
    enum SectionFlags {
	None = 0x0000,
	AMPM = 0x0001,
	MSecs = 0x0002,
	Seconds = 0x0004,
	Minutes = 0x0008,
	Hours = 0x0010,
	Days = 0x0100,
	Months = 0x0200,
	Years = 0x0400
    };

    Q_DECLARE_FLAGS(Section, SectionFlags);

    QDateTimeEdit(QWidget *parent = 0, WFlags f = 0);
    QDateTimeEdit(const QDateTime &t, QWidget *parent = 0, WFlags f = 0);
    QDateTimeEdit(const QTime &t, QWidget *parent = 0, WFlags f = 0);

    QDateTime dateTime() const;
    QDate date() const;
    QTime time() const;

    QDate minimumDate() const;
    void setMinimumDate(const QDate &min);
    void clearMinimumDate();

    QDate maximumDate() const;
    void setMaximumDate(const QDate &max);
    void clearMaximumDate();

    QTime minimumTime() const;
    void setMinimumTime(const QTime &min);
    void clearMinimumTime();

    QTime maximumTime() const;
    void setMaximumTime(const QTime &max);
    void clearMaximumTime();

    Section display() const;

    SectionFlags currentSection() const;
    void setCurrentSection(SectionFlags section);

    QString format() const;
    bool setFormat(const QString &format);

protected:
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void wheelEvent(QWheelEvent *e);
    virtual void focusInEvent(QFocusEvent *e);
    virtual bool focusNextPrevChild(bool next);
    virtual QString mapDateTimeToText(const QDateTime &date) const;
    virtual QDateTime mapTextToDateTime(QString *text, QValidator::State *state) const;
    virtual void stepBy(int steps);
    virtual StepEnabled stepEnabled() const;

public slots:
    void setDateTime(const QDateTime &dateTime);
    void setDate(const QDate &date);
    void setTime(const QTime &time);

signals:
    void dateTimeChanged(const QDateTime &date);
    void timeChanged(const QTime &date);
    void dateChanged(const QDate &date);

private:
    Q_DECLARE_PRIVATE(QDateTimeEdit);
};

#endif
