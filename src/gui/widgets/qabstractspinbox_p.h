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

#ifndef QABSTRACTSPINBOX_P_H
#define QABSTRACTSPINBOX_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qabstractspinbox.h"
#include <qlineedit.h>
#include <qstyleoption.h>
#include <qdatetime.h>
#include <qvariant.h>
#include <qvalidator.h>
#include <private/qwidget_p.h>

bool operator<(const QVariant &arg1, const QVariant &arg2);
bool operator>(const QVariant &arg1, const QVariant &arg2);
bool operator<=(const QVariant &arg1, const QVariant &arg2);
bool operator>=(const QVariant &arg1, const QVariant &arg2);
QVariant operator+(const QVariant &arg1, const QVariant &arg2);
QVariant operator-(const QVariant &arg1, const QVariant &arg2);
QVariant operator*(const QVariant &arg1, double multiplier);
double operator/(const QVariant &arg1, const QVariant &arg2);

#define TIME_MIN QTime(0, 0, 0, 0)
#define TIME_MAX QTime(23, 59, 59, 999)
#define DATE_MIN QDate(1752, 9, 14)
#define DATE_MAX QDate(7999, 12, 31)
#define DATETIME_MIN QDateTime(DATE_MIN, TIME_MIN)
#define DATETIME_MAX QDateTime(DATE_MAX, TIME_MAX)
#define DATE_INITIAL QDate(2000, 1, 1)

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
    DirectionMask = 0x040
};

class QAbstractSpinBoxPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QAbstractSpinBox)
public:
    QAbstractSpinBoxPrivate();

    void init();
    void resetState();
    void updateState(bool up);
    void strip(QString *text) const;
    bool specialValue() const;
    QVariant getZeroVariant() const;
    void setBoundary(Boundary b, const QVariant &val);
    void setValue(const QVariant &val, EmitPolicy ep, bool updateEdit = true);
    virtual QVariant bound(const QVariant &val, const QVariant &old = QVariant(), int steps = 0) const;
    QLineEdit *lineEdit();
    void updateSpinBox();
    void update();
    void updateEdit() const;
    virtual void calculateSizeHints() const;

    virtual QStyleOptionSpinBox getStyleOption() const;

    virtual void emitSignals(EmitPolicy ep, const QVariant &old);
    virtual void refresh(EmitPolicy ep);
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;
    virtual QString textFromValue(const QVariant &n) const;
    virtual QVariant valueFromText(const QString &input) const;

    void editorTextChanged(const QString &);
    virtual void editorCursorPositionChanged(int oldpos, int newpos);

    bool eq(const QVariant &arg1, const QVariant &arg2) const;
    QStyle::SubControl newHoverControl(const QPoint &pos);
    bool updateHoverControl(const QPoint &pos);

    QLineEdit *edit;
    QString prefix, suffix, specialvaluetext;
    QVariant value, minimum, maximum, singlestep;
    QVariant::Type type;
    int spinclicktimerid, spinclicktimerinterval;
    uint buttonstate;
    mutable QSize cachedsizehint, cachedminimumsizehint;
    mutable uint sizehintdirty : 1;
    mutable uint dirty : 1;
    mutable QString cachedtext;
    mutable QVariant cachedvalue;
    mutable QValidator::State cachedstate;
    uint pendingemit : 1;
    uint spindownenabled : 1;
    uint spinupenabled : 1;
    uint readonly : 1;
    uint tracking : 1;
    uint wrapping : 1;
    uint dragging : 1;
    uint ignorecursorpositionchanged : 1;
    uint frame : 1;
    QStyle::SubControl hoverControl;
    QRect hoverRect;
    QAbstractSpinBox::ButtonSymbols buttonsymbols;
};

class QSpinBoxValidator : public QValidator
{
public:
    QSpinBoxValidator(QAbstractSpinBox *qptr, QAbstractSpinBoxPrivate *dptr);
    QValidator::State validate(QString &input, int &) const;
    void fixup(QString &) const;
private:
    QAbstractSpinBox *qptr;
    QAbstractSpinBoxPrivate *dptr;
};

#endif // QABSTRACTSPINBOX_P_H
