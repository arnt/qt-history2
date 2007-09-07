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

#include "qschematype_p.h"

using namespace Patternist;

SchemaType::SchemaType()
{
}

SchemaType::~SchemaType()
{
}

bool SchemaType::isSimpleType() const
{
    switch(category())
    {
        /* Fallthrough */
        case SimpleTypeAtomic:
        case SimpleTypeList:
        case SimpleTypeUnion:
            return true;
        default:
            return false;
    }
}

bool SchemaType::isComplexType() const
{
    return category() == ComplexType;
}

// vim: et:ts=4:sw=4:sts=4
