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

#ifndef Patternist_SequenceType_H
#define Patternist_SequenceType_H

template<typename T> class QList;

#include <QSharedData>

#include "qcardinality_p.h"
#include "qitemtype_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    class ItemType;

    /**
     * @short A SequenceType instance represents the type of a sequence of Item instances.
     *
     * It carries a Cardinality and ItemType, and is hence conceptually identical to the SequenceType
     * EBNF construct.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     * @see <a href="http://www.w3.org/TR/xpath20/#id-sequencetype-syntax">XML
     * Path Language (XPath) 2.0, 2.5.3 SequenceType Syntax</a>
     */
    class SequenceType : public virtual QSharedData
    {
    public:
        /**
         * A smart pointer wrapping SequenceType instances.
         */
        typedef PlainSharedPtr<SequenceType> Ptr;
        /**
         * A list of SequenceType instances, each wrapped in a smart pointer.
         */
        typedef QList<SequenceType::Ptr> List;

        SequenceType();

        virtual ~SequenceType();

        /**
         * Generates a name for the sequence type for display purposes. The
         * prefix used for the QName identifying the schema type is conventional.
         * An example of a display name for a SequenceType is "xs:integer?".
         */
        virtual QString displayName(const NamePool::Ptr &np) const = 0;

        virtual Cardinality cardinality() const = 0;

        virtual ItemType::Ptr itemType() const = 0;

        /**
         * Determines whether @p other is identical to, or a sub-type
         * of this SequenceType. For example, if this SequenceType is
         * <tt>xs:anyAtomicType</tt>, @c false is returned if @p other is <tt>element()</tt>,
         * but @c true if @p other is <tt>xs:string</tt>.
         *
         * The return values of cardinality() and itemType() used with ItemType::xdtTypeMatches
         * and Cardinality::isWithinScope() is used for achieving this.
         *
         * @see <a href="http://www.w3.org/TR/xquery/#id-sequencetype-matching">XQuery 1.0:
         * An XML Query Language, 2.5.4 SequenceType Matching</a>
         */
        bool matches(const SequenceType::Ptr other) const;

    private:
        Q_DISABLE_COPY(SequenceType)
    };
}

Q_DECLARE_TYPEINFO(Patternist::SequenceType::Ptr, Q_MOVABLE_TYPE);

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
