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

#include "qbuiltintypes_p.h"
#include "qdebug_p.h"
#include "qlistiterator_p.h"
#include "qnamespaceresolver_p.h"

#include "qitem_p.h"

/**
 * @file
 * @short Contains the implementation for Node. The definition is in qitem_p.h.
 */

using namespace Patternist;

QString Node::axisName(const Axis axis)
{
    const char *result = 0;

    switch(axis)
    {
        /* These must not be translated. */
        case Child:             result = "child";               break;
        case Descendant:        result = "descendant";          break;
        case AttributeAxis:     result = "attribute";           break;
        case Self:              result = "self";                break;
        case DescendantOrSelf:  result = "descendant-or-self";  break;
        case FollowingSibling:  result = "following-sibling";   break;
        case NamespaceAxis:     result = "namespace";           break;
        case Following:         result = "following";           break;
        case Parent:            result = "parent";              break;
        case Ancestor:          result = "ancestor";            break;
        case PrecedingSibling:  result = "preceding-sibling";   break;
        case Preceding:         result = "preceding";           break;
        case AncestorOrSelf:    result = "ancestor-or-self";    break;

        case ForwardAxis:
        /* Fallthrough */
        case ReverseAxis:
        {
            Q_ASSERT_X(false, Q_FUNC_INFO,
                       "It makes no sense to use ForwardAxis or ReverseAxis independently.");
        }
    }

    Q_ASSERT_X(result, Q_FUNC_INFO, "An unknown axis type was apparently encountered.");
    return QString::fromLatin1(result);
}

// vim: et:ts=4:sw=4:sts=4
