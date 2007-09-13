/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <math.h>

#include "qboolean_p.h"
#include "qbuiltintypes_p.h"
#include "qcommonvalues_p.h"
#include "qdecimal_p.h"
#include "qinteger_p.h"

#include "qnumeric_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

AtomicValue::Ptr Numeric::fromLexical(const QString &number)
{
    Q_ASSERT(!number.isEmpty());
    Q_ASSERT_X(!number.contains(QLatin1Char('e')) &&
               !number.contains(QLatin1Char('E')),
               Q_FUNC_INFO, "Should not contain any e/E");

    if(number.contains(QLatin1Char('.'))) /* an xs:decimal. */
        return Decimal::fromLexical(number);
    else /* It's an integer, of some sort. E.g, -3, -2, -1, 0, 1, 2, 3 */
        return Integer::fromLexical(number);
}

xsDouble Numeric::nearByInt(const xsDouble val)
{
#ifdef Q_OS_WIN
    // TODO qRound is unsuitable, 9 failures in XQTS.
    return qRound(val);
#else
    return nearbyint(val);
#endif
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE
