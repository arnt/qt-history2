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

#ifndef Patternist_GenericSequenceType_H
#define Patternist_GenericSequenceType_H

#include "qsequencetype_p.h"

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{
    /**
     * @todo Documentation is missing.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class GenericSequenceType : public SequenceType
    {
    public:
        GenericSequenceType(const ItemType::Ptr &itemType, const Cardinality &card);

        /**
         * Generates a name for the sequence type for display purposes. The
         * prefix used for the QName identifying the schema type is conventional.
         * An example of a display name for a GenericSequenceType is "xs:integer?".
         */
        virtual QString displayName(const NamePool::Ptr &np) const;

        virtual Cardinality cardinality() const;

        virtual ItemType::Ptr itemType() const;

    private:
        const ItemType::Ptr m_itemType;
        const Cardinality m_cardinality;
    };

    /**
     * @short An object generator for GenericSequenceType.
     *
     * makeGenericSequenceType() is a convenience function for avoiding invoking
     * the @c new operator, and wrapping the result in GenericSequenceType::Ptr.
     *
     * @returns a smart pointer to to a GenericSequenceType instaniated from @p itemType and @p cardinality.
     * @relates GenericSequenceType
     */
    static inline SequenceType::Ptr
    makeGenericSequenceType(const ItemType::Ptr &itemType, const Cardinality &cardinality)
    {
        return SequenceType::Ptr(new GenericSequenceType(itemType, cardinality));
    }
}

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
