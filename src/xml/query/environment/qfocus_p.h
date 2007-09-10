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

#ifndef Patternist_Focus_H
#define Patternist_Focus_H

#include "qdelegatingdynamiccontext_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short A DynamicContext that maintains the focus, a sequence
     * of items.
     *
     * Focus implements both the outer and inner focus. The focus is one of
     * the things that characterizes the XPath language. The focus is what's
     * iterated over in a predicate, whose current item can be received
     * via the context item expression, <tt>.</tt>(the dot),
     * and whose size is retrievable via the function <tt>fn:last()</tt>.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class Q_AUTOTEST_EXPORT Focus : public DelegatingDynamicContext
    {
    public:
        Focus(const DynamicContext::Ptr &prevContext);

        virtual xsInteger contextPosition() const;
        virtual Item contextItem() const;
        virtual xsInteger contextSize();

        virtual void setFocusIterator(const Item::Iterator::Ptr &it);
        virtual Item::Iterator::Ptr focusIterator() const;

    private:
        Item::Iterator::Ptr m_focusIterator;
        xsInteger           m_contextSizeCached;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
