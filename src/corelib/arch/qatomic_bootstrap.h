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

#ifndef QATOMIC_BOOTSTRAP_H
#define QATOMIC_BOOTSTRAP_H

QT_BEGIN_HEADER

inline bool QBasicAtomicInt::ref()
{
    return ++_q_value != 0;
}

inline bool QBasicAtomicInt::deref()
{
    return --_q_value != 0;
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetOrdered(T *expectedValue, T *newValue)
{
    if (_q_value == expectedValue) {
        _q_value = newValue;
        return true;
    }
    return false;
}

QT_END_HEADER

#endif // QATOMIC_BOOTSTRAP_H
