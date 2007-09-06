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

#ifndef Patternist_ToCodepointsIterator_H
#define Patternist_ToCodepointsIterator_H

#include "Item.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Represents a stream of Unicode codepoints, which are computed from a string.
     *
     * ToCodepointsIterator takes in its constructor a string, whose Unicode characters'
     * codepoints it forms an Iterator over.
     *
     * @see <a href="http://www.w3.org/TR/xpath-functions/#func-string-to-codepoints">XQuery 1.0
     * and XPath 2.0 Functions and Operators, 7.2.2 fn:string-to-codepoints</a>
     * @see StringToCodepointsFN
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_iterators
     */
    class ToCodepointsIterator : public Item::Iterator
    {
    public:
        /**
         * Constructs a ToCodepointsIterator.
         *
         * @param string the string to retrieve Unicode codepoints from. Can not be
         * empty.
         */
        ToCodepointsIterator(const QString &string);
        virtual Item next();
        virtual Item current() const;
        virtual xsInteger position() const;
        virtual xsInteger count();
        virtual Cardinality cardinality();
        virtual Item::Iterator::Ptr copy() const;

    private:
        const QString m_string;
        const int m_len;
        Item m_current;
        xsInteger m_position;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
