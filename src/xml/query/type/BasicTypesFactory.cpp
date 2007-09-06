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
***************************************************************************
*/

#include <QtDebug>

#include "BuiltinTypes.h"
#include "CommonNamespaces.h"

#include "BasicTypesFactory.h"

using namespace Patternist;

// STATIC DATA
static SchemaTypeFactory::Ptr s_self;

SchemaTypeFactory::Ptr BasicTypesFactory::self(const NamePool::Ptr &np)
{
    if(s_self)
        return s_self;

    s_self = SchemaTypeFactory::Ptr(new BasicTypesFactory(np));
    return s_self;
}

BasicTypesFactory::BasicTypesFactory(const NamePool::Ptr &np)
{
    m_types.reserve(48);

#define add(aName)   m_types.insert(BuiltinTypes::aName->name(np), AtomicType::Ptr(BuiltinTypes::aName))
#define addNA(aName) m_types.insert(BuiltinTypes::aName->name(np), BuiltinTypes::aName)
    add(xsString);
    add(xsBoolean);
    add(xsDecimal);
    add(xsDouble);
    add(xsFloat);
    add(xsDate);
    add(xsTime);
    add(xsDateTime);
    add(xsDuration);
    add(xsAnyURI);
    add(xsGDay);
    add(xsGMonthDay);
    add(xsGMonth);
    add(xsGYearMonth);
    add(xsGYear);
    add(xsBase64Binary);
    add(xsHexBinary);
    add(xsQName);
    add(xsInteger);
    addNA(xsAnyType);
    addNA(xsAnySimpleType);
    add(xsYearMonthDuration);
    add(xsDayTimeDuration);
    add(xsAnyAtomicType);
    addNA(xsUntyped);
    add(xsUntypedAtomic);
    add(xsNOTATION);
    /* Add derived primitives. */
    add(xsNonPositiveInteger);
    add(xsNegativeInteger);
    add(xsLong);
    add(xsInt);
    add(xsShort);
    add(xsByte);
    add(xsNonNegativeInteger);
    add(xsUnsignedLong);
    add(xsUnsignedInt);
    add(xsUnsignedShort);
    add(xsUnsignedByte);
    add(xsPositiveInteger);
    add(xsNormalizedString);
    add(xsToken);
    add(xsLanguage);
    add(xsNMTOKEN);
    add(xsName);
    add(xsNCName);
    add(xsID);
    add(xsIDREF);
    add(xsENTITY);

#undef add
#undef addNA
}

SchemaType::Ptr BasicTypesFactory::createSchemaType(const QName name) const
{
    return m_types.value(name);
}

SchemaType::Hash BasicTypesFactory::types() const
{
    return m_types;
}

// vim: et:ts=4:sw=4:sts=4
