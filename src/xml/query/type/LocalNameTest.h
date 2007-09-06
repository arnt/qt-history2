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

#ifndef Patternist_LocalNameTest_H
#define Patternist_LocalNameTest_H

#include "AbstractNodeTest.h"

template<typename Key, typename Value> class QHash;

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short A name test that is of the type <tt>*:local-name</tt>.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class LocalNameTest : public AbstractNodeTest
    {
    public:
        typedef QHash<QString, ItemType::Ptr> Hash;

        static ItemType::Ptr create(const ItemType::Ptr &primaryType, const QName::LocalNameCode localName);

        /**
         * @note This function assumes that @p item is a Node.
         */
        virtual bool itemMatches(const Item &item) const;

        virtual QString displayName(const NamePool::Ptr &np) const;

        virtual bool operator==(const ItemType &other) const;

    protected:
        virtual InstanceOf instanceOf() const;

    private:
        LocalNameTest(const ItemType::Ptr &primaryType, const QName::LocalNameCode &ncName);

        const QName::LocalNameCode m_ncName;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
