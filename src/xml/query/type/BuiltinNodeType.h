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

#ifndef Patternist_BuiltinNodeType_H
#define Patternist_BuiltinNodeType_H

#include "Item.h"
#include "AnyNodeType.h"
#include "BuiltinTypes.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Instances of this class represents types that are sub-classes
     * of <tt>node()</tt>, such as <tt>processing-instruction()</tt>.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    template <const Node::NodeKind kind>
    class BuiltinNodeType : public AnyNodeType
    {
    public:
        virtual bool xdtTypeMatches(const ItemType::Ptr &other) const;
        virtual bool itemMatches(const Item &item) const;

        /**
         * @returns for example "text()", depending on what the constructor was passed
         */
        virtual QString displayName(const NamePool::Ptr &np) const;

        virtual ItemType::Ptr xdtSuperType() const;
        virtual ItemType::Ptr atomizedType() const;

        Node::NodeKind nodeKind() const;

    protected:
        friend class BuiltinTypes;

        /**
         * This constructor does nothing, but exists in order to make it impossible to
         * instantiate this class from anywhere but from BuiltinTypes.
         */
        BuiltinNodeType();
    };

#include "BuiltinNodeType.cpp"
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
