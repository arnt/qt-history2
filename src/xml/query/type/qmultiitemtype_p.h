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

#ifndef Patternist_MultiItemType_H
#define Patternist_MultiItemType_H

#include <QList>

#include "qitemtype_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Represents multiple types such as <tt>document()</tt> @em or <tt>xs:integer</tt>.
     *
     * In some situations two or more different types are allowed. For example, XQuery's
     * @c validate expression accepts document or element nodes(but not attribute
     * nodes, for example). MultiItemType is useful in such situations, its constructor
     * takes a list of ItemType instantances which its member functions treats as a wholeness.
     *
     * For example, xdtTypeMatches() returns @c true if any of the represented types matches.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class MultiItemType : public ItemType
    {
    public:
        /**
         * Creates a MultiItemType representing the types in @p typeList. @p typeList must
         * contain two or more types.
         */
        MultiItemType(const ItemType::List &typeList);

        /**
         * The display name are the names concatenated with "|" as separator. For example,
         * if this MultiItemType represents the types <tt>document()</tt>, <tt>xs:integer</tt>,
         * and <tt>xs:anyAtomicType</tt>, the display name is
         * "document() | xs:integer | xs:anyAtomicType".
         */
        virtual QString displayName(const NamePool::Ptr &np) const;

        /**
         * If any of the types this MultiItemType represents matches @p item, it is
         * considered a match.
         *
         * @returns @c true if any of the housed ItemType instances matches @p item, otherwise @c false
         */
        virtual bool itemMatches(const Item &item) const;

        /**
         * If any of the types this MultiItemType represents matches @p other, it is
         * considered a match.
         *
         * @returns @c true if any of the housed ItemType instances matches @p other, otherwise @c false
         */
        virtual bool xdtTypeMatches(const ItemType::Ptr &other) const;

        /**
         * @returns @c true if any of the represented types is a node type.
         */
        virtual bool isNodeType() const;

        /**
         * @returns @c true if any of the represented types is an atomic type.
         */
        virtual bool isAtomicType() const;

        /**
         * Determines the union type of all the represented types super types. For example,
         * if the represented types are <tt>xs:integer</tt>, <tt>document()</tt>
         * and <tt>xs:string</tt>, <tt>item()</tt> is returned.
         */
        virtual ItemType::Ptr xdtSuperType() const;

        /**
         * Determines the union type of all the represented types atomized types. For example,
         * if the represented types are <tt>xs:integer</tt> and <tt>document()</tt>,
         * <tt>xs:anyAtomicType</tt> is returned, because that's the super type of <tt>xs:integer</tt>
         * and <tt>xs:untypedAtomic</tt>.
         */
        virtual ItemType::Ptr atomizedType() const;

    private:
        const ItemType::List m_types;
        const ItemType::List::const_iterator m_end;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
