/****************************************************************************
**
** Definition of date and time edit classes.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt Compat Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3DATETIMEEDIT_H
#define Q3DATETIMEEDIT_H

#ifndef QT_H
#include "qwidget.h"
#include "qstring.h"
#include "qdatetime.h"
#endif // QT_H

#ifndef QT_NO_DATETIMEEDIT

class Q_COMPAT_EXPORT Q3DateTimeEditBase : public QWidget
{
    Q_OBJECT
public:
    Q3DateTimeEditBase(QWidget* parent=0, const char* name=0)
        : QWidget(parent, name) {}

    virtual bool setFocusSection(int sec) = 0;
    virtual QString sectionFormattedText(int sec) = 0;
    virtual void addNumber(int sec, int num) = 0;
    virtual void removeLastNumber(int sec) = 0;

public slots:
    virtual void stepUp() = 0;
    virtual void stepDown() = 0;

private:
#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=
    Q3DateTimeEditBase(const Q3DateTimeEditBase &);
    Q3DateTimeEditBase &operator=(const Q3DateTimeEditBase &);
#endif
};

class Q3DateEditPrivate;

class Q_COMPAT_EXPORT Q3DateEdit : public Q3DateTimeEditBase
{
    Q_OBJECT
    Q_ENUMS(Order)
    Q_PROPERTY(Order order READ order WRITE setOrder)
    Q_PROPERTY(QDate date READ date WRITE setDate)
    Q_PROPERTY(bool autoAdvance READ autoAdvance WRITE setAutoAdvance)
    Q_PROPERTY(QDate maxValue READ maxValue WRITE setMaxValue)
    Q_PROPERTY(QDate minValue READ minValue WRITE setMinValue)

public:
    Q3DateEdit(QWidget* parent=0,  const char* name=0);
    Q3DateEdit(const QDate& date, QWidget* parent=0,  const char* name=0);
    ~Q3DateEdit();

    enum Order { DMY, MDY, YMD, YDM };

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

public slots:
    virtual void setDate(const QDate& date);

public:
    QDate date() const;
    virtual void setOrder(Order order);
    Order order() const;
    virtual void setAutoAdvance(bool advance);
    bool autoAdvance() const;

    virtual void setMinValue(const QDate& d) { setRange(d, maxValue()); }
    QDate minValue() const;
    virtual void setMaxValue(const QDate& d) { setRange(minValue(), d); }
    QDate maxValue() const;
    virtual void setRange(const QDate& min, const QDate& max);
    QString separator() const;
    virtual void setSeparator(const QString& s);

    // Make removeFirstNumber() virtual in Q3DateTimeEditBase in 4.0
    void removeFirstNumber(int sec);

signals:
    void valueChanged(const QDate& date);

protected:
    bool event(QEvent *e);
    void timerEvent(QTimerEvent *);
    void resizeEvent(QResizeEvent *);
    void stepUp();
    void stepDown();
    QString sectionFormattedText(int sec);
    void addNumber(int sec, int num);

    void removeLastNumber(int sec);
    bool setFocusSection(int s);

    virtual void setYear(int year);
    virtual void setMonth(int month);
    virtual void setDay(int day);
    virtual void fix();
    virtual bool outOfRange(int y, int m, int d) const;

protected slots:
    void updateButtons();

private:
    void init();
    int sectionOffsetEnd(int sec) const;
    int sectionLength(int sec) const;
    QString sectionText(int sec) const;
    Q3DateEditPrivate* d;

#if defined(Q_DISABLE_COPY)
    Q3DateEdit(const Q3DateEdit &);
    Q3DateEdit &operator=(const Q3DateEdit &);
#endif
};

class Q3TimeEditPrivate;

class Q_COMPAT_EXPORT Q3TimeEdit : public Q3DateTimeEditBase
{
    Q_OBJECT
    Q_FLAGS(Display)
    Q_PROPERTY(QTime time READ time WRITE setTime)
    Q_PROPERTY(bool autoAdvance READ autoAdvance WRITE setAutoAdvance)
    Q_PROPERTY(QTime maxValue READ maxValue WRITE setMaxValue)
    Q_PROPERTY(QTime minValue READ minValue WRITE setMinValue)
    Q_PROPERTY(Display display READ display WRITE setDisplay)

public:
    enum Display {
        Hours        = 0x01,
        Minutes        = 0x02,
        Seconds        = 0x04,
        /*Reserved = 0x08,*/
        AMPM        = 0x10
    };

    Q3TimeEdit(QWidget* parent=0,  const char* name=0);
    Q3TimeEdit(const QTime& time, QWidget* parent=0,  const char* name=0);
    ~Q3TimeEdit();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

public slots:
    virtual void setTime(const QTime& time);

public:
    QTime time() const;
    virtual void setAutoAdvance(bool advance);
    bool autoAdvance() const;

    virtual void setMinValue(const QTime& d) { setRange(d, maxValue()); }
    QTime minValue() const;
    virtual void setMaxValue(const QTime& d) { setRange(minValue(), d); }
    QTime maxValue() const;
    virtual void setRange(const QTime& min, const QTime& max);
    QString separator() const;
    virtual void setSeparator(const QString& s);

    uint display() const;
    void setDisplay(uint disp);

    // Make removeFirstNumber() virtual in Q3DateTimeEditBase in 4.0
    void removeFirstNumber(int sec);

signals:
    void valueChanged(const QTime& time);

protected:
    bool event(QEvent *e);
    void timerEvent(QTimerEvent *e);
    void resizeEvent(QResizeEvent *);
    void stepUp();
    void stepDown();
    QString sectionFormattedText(int sec);
    void addNumber(int sec, int num);
    void removeLastNumber(int sec);
    bool setFocusSection(int s);

    virtual bool outOfRange(int h, int m, int s) const;
    virtual void setHour(int h);
    virtual void setMinute(int m);
    virtual void setSecond(int s);

protected slots:
    void updateButtons();

private:
    void init();
    QString sectionText(int sec);
    Q3TimeEditPrivate* d;

#if defined(Q_DISABLE_COPY)
    Q3TimeEdit(const Q3TimeEdit &);
    Q3TimeEdit &operator=(const Q3TimeEdit &);
#endif
};


class Q3DateTimeEditPrivate;

class Q_COMPAT_EXPORT Q3DateTimeEdit : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QDateTime dateTime READ dateTime WRITE setDateTime)

public:
    Q3DateTimeEdit(QWidget* parent=0, const char* name=0);
    Q3DateTimeEdit(const QDateTime& datetime, QWidget* parent=0,
                   const char* name=0);
    ~Q3DateTimeEdit();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

public slots:
    virtual void setDateTime(const QDateTime & dt);

public:
    QDateTime dateTime() const;

    Q3DateEdit* dateEdit() { return de; }
    Q3TimeEdit* timeEdit() { return te; }

    virtual void setAutoAdvance(bool advance);
    bool autoAdvance() const;

signals:
    void valueChanged(const QDateTime& datetime);

protected:
    // ### make init() private in Qt 4.0
    void init();
    void resizeEvent(QResizeEvent *);

protected slots:
    // ### make these two functions private in Qt 4.0,
    //     and merge them into one with no parameter
    void newValue(const QDate& d);
    void newValue(const QTime& t);

private:
    Q3DateEdit* de;
    Q3TimeEdit* te;
    Q3DateTimeEditPrivate* d;

#if defined(Q_DISABLE_COPY)
    Q3DateTimeEdit(const Q3DateTimeEdit &);
    Q3DateTimeEdit &operator=(const Q3DateTimeEdit &);
#endif
};

#endif
#endif
