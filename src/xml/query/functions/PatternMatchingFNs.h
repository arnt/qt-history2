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

#ifndef Patternist_PatternMatchingFNs_H
#define Patternist_PatternMatchingFNs_H

#include "PatternPlatform.h"

/**
 * @file
 * @short Contains classes implementing the functions found in
 * <a href="http://www.w3.org/TR/xpath-functions/#string.match">XQuery 1.0 and
 * XPath 2.0 Functions and Operators, 7.6 AtomicString Functions that Use Pattern Matching</a>.
 *
 * @ingroup Patternist_functions
 */

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Implements the function <tt>fn:matches()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class MatchesFN : public PatternPlatform
    {
    public:
        MatchesFN();
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
    };

    /**
     * @short Implements the function <tt>fn:replace()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ReplaceFN : public PatternPlatform
    {
    public:
        ReplaceFN();
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
        /**
         * Overriden to attempt to pre-compile the replacement string.
         */
        virtual Expression::Ptr compress(const StaticContext::Ptr &context);

    private:
        /**
         * @short Centralizes the translation string.
         */
        static inline QString errorAtEnd(const char ch);

        /**
         * Reads the string in the third argument and converts it to a a QRegExp compatible
         * replacement string, containing sub-group references and so forth.
         */
        QString parseReplacement(const int captureCount,
                                 const DynamicContext::Ptr &context) const;

        QString m_replacementString;
    };

    /**
     * @short Implements the function <tt>fn:tokenize()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class TokenizeFN : public PatternPlatform
    {
    public:
        TokenizeFN();
        inline Item mapToItem(const QString &subject, const DynamicContext::Ptr &) const;
        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
    private:
        typedef PlainSharedPtr<TokenizeFN> Ptr;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
