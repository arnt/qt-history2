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

#ifndef Patternist_AbstractNodeTest_H
#define Patternist_AbstractNodeTest_H

#include "AnyNodeType.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short A name test that is of the type <tt>prefix:ncName</tt>.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AbstractNodeTest : public AnyNodeType
    {
    public:
        AbstractNodeTest(const ItemType::Ptr &primaryType);

        virtual bool xdtTypeMatches(const ItemType::Ptr &other) const;
        virtual ItemType::Ptr xdtSuperType() const;
        virtual ItemType::Ptr atomizedType() const;

    protected:
        const ItemType::Ptr m_primaryType;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
