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

#include "EBVType.h"
#include "GenericSequenceType.h"
#include "NoneType.h"

#include "CommonSequenceTypes.h"

/* To avoid the static initialization fiasco, we put the builtin types in this compilation unit, since
 * the sequence types depends on them. */
#include "BuiltinTypes.cpp"

using namespace Patternist;

// STATIC DATA
#define st(var, type, card)                                             \
const SequenceType::Ptr                                                 \
CommonSequenceTypes::var(new GenericSequenceType(BuiltinTypes::type,    \
                                                 Cardinality::card()))

/* Alphabetically. */
st(ExactlyOneAnyURI,                xsAnyURI,               exactlyOne);
st(ExactlyOneAtomicType,            xsAnyAtomicType,        exactlyOne);
st(ExactlyOneAttribute,             attribute,              exactlyOne);
st(ExactlyOneBase64Binary,          xsBase64Binary,         exactlyOne);
st(ExactlyOneBoolean,               xsBoolean,              exactlyOne);
st(ExactlyOneComment,               comment,                exactlyOne);
st(ExactlyOneDate,                  xsDate,                 exactlyOne);
st(ExactlyOneDateTime,              xsDateTime,             exactlyOne);
st(ExactlyOneDayTimeDuration,       xsDayTimeDuration,      exactlyOne);
st(ExactlyOneDecimal,               xsDecimal,              exactlyOne);
st(ExactlyOneDocumentNode,          document,               exactlyOne);
st(ExactlyOneDouble,                xsDouble,               exactlyOne);
st(ExactlyOneDuration,              xsDuration,             exactlyOne);
st(ExactlyOneElement,               element,                exactlyOne);
st(ExactlyOneFloat,                 xsFloat,                exactlyOne);
st(ExactlyOneGDay,                  xsGDay,                 exactlyOne);
st(ExactlyOneGMonth,                xsGMonth,               exactlyOne);
st(ExactlyOneGMonthDay,             xsGMonthDay,            exactlyOne);
st(ExactlyOneGYear,                 xsGYear,                exactlyOne);
st(ExactlyOneGYearMonth,            xsGYearMonth,           exactlyOne);
st(ExactlyOneHexBinary,             xsHexBinary,            exactlyOne);
st(ExactlyOneInteger,               xsInteger,              exactlyOne);
st(ExactlyOneItem,                  item,                   exactlyOne);
st(ExactlyOneNode,                  node,                   exactlyOne);
st(ExactlyOneNumeric,               numeric,                exactlyOne);
st(ExactlyOneProcessingInstruction, pi,                     exactlyOne);
st(ExactlyOneQName,                 xsQName,                exactlyOne);
st(ExactlyOneString,                xsString,               exactlyOne);
st(ExactlyOneTextNode,              text,                   exactlyOne);
st(ExactlyOneTime,                  xsTime,                 exactlyOne);
st(ExactlyOneUntypedAtomic,         xsUntypedAtomic,        exactlyOne);
st(ExactlyOneYearMonthDuration,     xsYearMonthDuration,    exactlyOne);
st(OneOrMoreItems,                  item,                   oneOrMore);
st(ZeroOrMoreAtomicTypes,           xsAnyAtomicType,        zeroOrMore);
st(ZeroOrMoreElements,              element,                zeroOrMore);
st(ZeroOrMoreIntegers,              xsInteger,              zeroOrMore);
st(ZeroOrMoreItems,                 item,                   zeroOrMore);
st(ZeroOrMoreNodes,                 node,                   zeroOrMore);
st(ZeroOrMoreStrings,               xsString,               zeroOrMore);
st(ZeroOrOneAnyURI,                 xsAnyURI,               zeroOrOne);
st(ZeroOrOneAtomicType,             xsAnyAtomicType,        zeroOrOne);
st(ZeroOrOneBoolean,                xsBoolean,              zeroOrOne);
st(ZeroOrOneDate,                   xsDate,                 zeroOrOne);
st(ZeroOrOneDateTime,               xsDateTime,             zeroOrOne);
st(ZeroOrOneDayTimeDuration,        xsDayTimeDuration,      zeroOrOne);
st(ZeroOrOneDecimal,                xsDecimal,              zeroOrOne);
st(ZeroOrOneDocumentNode,           document,               zeroOrOne);
st(ZeroOrOneDuration,               xsDuration,             zeroOrOne);
st(ZeroOrOneInteger,                xsInteger,              zeroOrOne);
st(ZeroOrOneItem,                   item,                   zeroOrOne);
st(ZeroOrOneNCName,                 xsNCName,               zeroOrOne);
st(ZeroOrOneNode,                   node,                   zeroOrOne);
st(ZeroOrOneNumeric,                numeric,                zeroOrOne);
st(ZeroOrOneQName,                  xsQName,                zeroOrOne);
st(ZeroOrOneString,                 xsString,               zeroOrOne);
st(ZeroOrOneTextNode,               text,                   zeroOrOne);
st(ZeroOrOneTime,                   xsTime,                 zeroOrOne);
st(ZeroOrOneYearMonthDuration,      xsYearMonthDuration,    zeroOrOne);

#undef st

/* Special cases. */
const EmptySequenceType::Ptr    CommonSequenceTypes::Empty  (new EmptySequenceType());
const NoneType::Ptr             CommonSequenceTypes::None   (new NoneType());
const SequenceType::Ptr         CommonSequenceTypes::EBV    (new EBVType());

// vim: et:ts=4:sw=4:sts=4

