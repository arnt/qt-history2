/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qlonglongvalidator.h"

using namespace qdesigner_internal;

// ----------------------------------------------------------------------------
QLongLongValidator::QLongLongValidator(QObject * parent)
    : QValidator(parent),
      b(Q_UINT64_C(0x8000000000000000)), t(Q_UINT64_C(0x7FFFFFFFFFFFFFFF))
{
}

QLongLongValidator::QLongLongValidator(qlonglong minimum, qlonglong maximum,
                              QObject * parent)
    : QValidator(parent), b(minimum), t(maximum)
{
}

QLongLongValidator::~QLongLongValidator()
{
    // nothing
}

QValidator::State QLongLongValidator::validate(QString & input, int &) const
{
    if (input.contains(QLatin1Char(' ')))
        return Invalid;
    if (input.isEmpty() || (b < 0 && input == QString(QLatin1Char('-'))))
        return Intermediate;
    bool ok;
    qlonglong entered = input.toLongLong(&ok);
    if (!ok || (entered < 0 && b >= 0)) {
        return Invalid;
    } else if (entered >= b && entered <= t) {
        return Acceptable;
    } else {
        if (entered >= 0)
            return (entered > t) ? Invalid : Intermediate;
        else
            return (entered < b) ? Invalid : Intermediate;
    }
}

void QLongLongValidator::setRange(qlonglong bottom, qlonglong top)
{
    b = bottom;
    t = top;
}

void QLongLongValidator::setBottom(qlonglong bottom)
{
    setRange(bottom, top());
}

void QLongLongValidator::setTop(qlonglong top)
{
    setRange(bottom(), top);
}


// ----------------------------------------------------------------------------
QULongLongValidator::QULongLongValidator(QObject * parent)
    : QValidator(parent),
      b(0), t(Q_UINT64_C(0xFFFFFFFFFFFFFFFF))
{
}

QULongLongValidator::QULongLongValidator(qulonglong minimum, qulonglong maximum,
                              QObject * parent)
    : QValidator(parent), b(minimum), t(maximum)
{
}

QULongLongValidator::~QULongLongValidator()
{
    // nothing
}

QValidator::State QULongLongValidator::validate(QString & input, int &) const
{
    if (input.isEmpty())
        return Intermediate;

    bool ok;
    qulonglong entered = input.toULongLong(&ok);
    if (input.contains(QLatin1Char(' ')) || input.contains(QLatin1Char('-')) || !ok)
        return Invalid;

    if (entered >= b && entered <= t)
        return Acceptable;

    return Invalid;
}

void QULongLongValidator::setRange(qulonglong bottom, qulonglong top)
{
    b = bottom;
    t = top;
}

void QULongLongValidator::setBottom(qulonglong bottom)
{
    setRange(bottom, top());
}

void QULongLongValidator::setTop(qulonglong top)
{
    setRange(bottom(), top);
}

