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

#ifndef QNUMERIC_H
#define QNUMERIC_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Core)

Q_CORE_EXPORT bool qIsInf(double d);
Q_CORE_EXPORT bool qIsNaN(double d);
Q_CORE_EXPORT bool qIsFinite(double d);
Q_CORE_EXPORT bool qIsInf(float f);
Q_CORE_EXPORT bool qIsNaN(float f);
Q_CORE_EXPORT bool qIsFinite(float f);
Q_CORE_EXPORT double qSNaN();
Q_CORE_EXPORT double qQNaN();
Q_CORE_EXPORT double qInf();

#define Q_INFINITY (::qInf())
#define Q_SNAN (::qSNaN())
#define Q_QNAN (::qQNaN())

QT_END_NAMESPACE

QT_END_HEADER

#endif // QNUMERIC_H
