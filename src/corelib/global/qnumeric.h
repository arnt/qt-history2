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

#include <qglobal.h>

QT_BEGIN_HEADER

QT_MODULE(Core)

Q_CORE_EXPORT bool qIsInf(double d);
Q_CORE_EXPORT bool qIsNan(double d);
Q_CORE_EXPORT bool qIsFinite(double d);
Q_CORE_EXPORT bool qIsInf(float f);
Q_CORE_EXPORT bool qIsNan(float f);
Q_CORE_EXPORT bool qIsFinite(float f);

QT_END_HEADER

#endif // QNUMERIC_H
