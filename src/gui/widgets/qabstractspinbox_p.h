/****************************************************************************
**
** Definition of some Qt private functions.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QABSTRACTSPINBOX_P_H
#define QABSTRACTSPINBOX_P_H

#include "qabstractspinbox.h"
#include <qlineedit.h>
#include <qstyleoption.h>
#include <qdatetime.h>
#include <qvariant.h>
#include <qvalidator.h>
#include <private/qwidget_p.h>

#define TIME_MIN QTime(0, 0, 0, 0)
#define TIME_MAX QTime(23, 59, 59, 999)
#define DATE_MIN QDate(1752, 9, 14)
#define DATE_MAX QDate(7999, 12, 31)
#define DATETIME_MIN QDateTime(DATE_MIN, TIME_MIN)
#define DATETIME_MAX QDateTime(DATE_MAX, TIME_MAX)
#define DATE_INITIAL QDate(1753, 1, 1)

enum EmitPolicy {
    EmitIfChanged,
    AlwaysEmit,
    NeverEmit
};

enum Boundary {
    Minimum,
    Maximum
};
enum Button {
    None = 0x000,
    Keyboard = 0x001,
    Mouse = 0x002,
    Wheel = 0x004,
    ButtonMask = 0x008,
    Up = 0x010,
    Down = 0x020,
    DirectionMask = 0x040,
};

class QAbstractSpinBoxPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QAbstractSpinBox);
public:
    QAbstractSpinBoxPrivate();

    void init();
    void resetState();
    void strip(QString *text) const;
    bool specialValue() const;
    QCoreVariant getZeroVariant() const;
    void setBoundary(Boundary b, const QCoreVariant &val);
    void setValue(const QCoreVariant &val, EmitPolicy ep);
    QCoreVariant bound(const QCoreVariant &val, const QCoreVariant &old = QCoreVariant(), int steps = 0) const;
    QLineEdit *lineEdit();
    void updateSpinBox();
    void updateSlider();
    void update();
    void updateEdit() const;
    void calculateSizeHints() const;

    QStyleOptionSpinBox styleOption() const;

    QCoreVariant valueForPosition(int pos) const;

    virtual void emitSignals();
    virtual void refresh(EmitPolicy ep);
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;
    virtual QValidator::State validate(QString *input, int *pos, QCoreVariant *val) const;
    virtual QCoreVariant mapTextToValue(QString *text, QValidator::State *state) const;
    virtual QString mapValueToText(const QCoreVariant &n) const;

    void editorTextChanged(const QString &);
    virtual void editorCursorPositionChanged(int oldpos, int newpos);

    bool eq(const QCoreVariant &arg1, const QCoreVariant &arg2) const;

    QLineEdit *edit;
    QString prefix, suffix, specialvaluetext;
    QCoreVariant value, minimum, maximum, singlestep;
    QCoreVariant::Type type;
    int spinclicktimerid, spinkeytimerid, spinclicktimerinterval, spinkeytimerinterval;
    uint buttonstate;
    mutable QSize cachedsizehint, cachedminimumsizehint;
    mutable uint sizehintdirty : 1;
    mutable uint dirty : 1;
    uint useprivate : 1;
    uint pendingemit : 1;
    uint spindownenabled : 1;
    uint spinupenabled : 1;
    uint tracking : 1;
    uint wrapping : 1;
    uint dragging : 1;
    uint ignorecursorpositionchanged : 1;
    uint slider : 1;
    uint sliderpressed : 1;
    QAbstractSpinBox::ButtonSymbols buttonsymbols;
};

class QSpinBoxValidator : public QValidator
{
public:
    QSpinBoxValidator(QAbstractSpinBoxPrivate *p, QObject *parent);
    QValidator::State validate(QString & input, int &) const;
private:
    QAbstractSpinBoxPrivate *dptr;
};

#endif
