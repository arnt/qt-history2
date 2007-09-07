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

#ifndef Patternist_QNameTest_H
#define Patternist_QNameTest_H

#include "AbstractNodeTest.h"

template<typename Key, typename Value> class QHash;

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short A name test that is of the type <tt>prefix:ncName</tt>.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class QNameTest : public AbstractNodeTest
    {
    public:
        typedef QHash<QString, QNameTest::Ptr> Hash;

        static ItemType::Ptr create(const ItemType::Ptr &primaryType, const QName qName);

        /**
         * @note This function assumes that @p item is a Node.
         */
        virtual bool itemMatches(const Item &item) const;

        virtual QString displayName(const NamePool::Ptr &np) const;

        virtual bool operator==(const ItemType &other) const;

    protected:
        virtual InstanceOf instanceOf() const;

    private:
        QNameTest(const ItemType::Ptr &primaryType, const QName qName);

        const QName m_qName;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
