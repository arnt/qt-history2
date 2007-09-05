/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the Patternist project on Trolltech Labs.
**
** $TROLLTECH_GPL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************
*/

#ifndef Patternist_BuiltinTypes_H
#define Patternist_BuiltinTypes_H

#include "AnyNodeType.h"
#include "AnySimpleType.h"
#include "AnyType.h"
#include "BuiltinAtomicType.h"
#include "ItemType.h"
#include "NumericType.h"
#include "Untyped.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Provides access to singleton instances of ItemType and SchemaType sub-classes.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class BuiltinTypes
    {
    public:
        static const SchemaType::Ptr        xsAnyType;
        static const SchemaType::Ptr        xsAnySimpleType;
        static const SchemaType::Ptr        xsUntyped;

        static const AtomicType::Ptr        xsAnyAtomicType;
        static const AtomicType::Ptr        xsUntypedAtomic;
        static const AtomicType::Ptr        xsDateTime;
        static const AtomicType::Ptr        xsDate;
        static const AtomicType::Ptr        xsTime;
        static const AtomicType::Ptr        xsDuration;
        static const AtomicType::Ptr        xsYearMonthDuration;
        static const AtomicType::Ptr        xsDayTimeDuration;

        /**
         * An artificial type for implementation purposes
         * that represents the XPath type @c numeric.
         */
        static const AtomicType::Ptr        numeric;
        static const AtomicType::Ptr          xsFloat;
        static const AtomicType::Ptr          xsDouble;
        static const AtomicType::Ptr        xsInteger;
        static const AtomicType::Ptr        xsDecimal;
        static const AtomicType::Ptr        xsNonPositiveInteger;
        static const AtomicType::Ptr        xsNegativeInteger;
        static const AtomicType::Ptr        xsLong;
        static const AtomicType::Ptr        xsInt;
        static const AtomicType::Ptr        xsShort;
        static const AtomicType::Ptr        xsByte;
        static const AtomicType::Ptr        xsNonNegativeInteger;
        static const AtomicType::Ptr        xsUnsignedLong;
        static const AtomicType::Ptr        xsUnsignedInt;
        static const AtomicType::Ptr        xsUnsignedShort;
        static const AtomicType::Ptr        xsUnsignedByte;
        static const AtomicType::Ptr        xsPositiveInteger;


        static const AtomicType::Ptr        xsGYearMonth;
        static const AtomicType::Ptr        xsGYear;
        static const AtomicType::Ptr        xsGMonthDay;
        static const AtomicType::Ptr        xsGDay;
        static const AtomicType::Ptr        xsGMonth;

        static const AtomicType::Ptr        xsBoolean;

        static const AtomicType::Ptr        xsBase64Binary;
        static const AtomicType::Ptr        xsHexBinary;
        static const AtomicType::Ptr        xsAnyURI;
        static const AtomicType::Ptr        xsQName;
        static const AtomicType::Ptr        xsString;
        static const AtomicType::Ptr        xsNormalizedString;
        static const AtomicType::Ptr        xsToken;
        static const AtomicType::Ptr        xsLanguage;
        static const AtomicType::Ptr        xsNMTOKEN;
        static const AtomicType::Ptr        xsName;
        static const AtomicType::Ptr        xsNCName;
        static const AtomicType::Ptr        xsID;
        static const AtomicType::Ptr        xsIDREF;
        static const AtomicType::Ptr        xsENTITY;

        static const AtomicType::Ptr        xsNOTATION;
        static const ItemType::Ptr          item;

        static const AnyNodeType::Ptr       node;
        static const ItemType::Ptr          attribute;
        static const ItemType::Ptr          comment;
        static const ItemType::Ptr          document;
        static const ItemType::Ptr          element;
        static const ItemType::Ptr          pi;
        static const ItemType::Ptr          text;

    private:
        /**
         * The constructor is protected because this class is not meant to be instantiated,
         * but should only be used via its static const members.
         */
        BuiltinTypes();
        Q_DISABLE_COPY(BuiltinTypes)
    };
}

QT_END_HEADER 

#endif

// vim: et:ts=4:sw=4:sts=4
