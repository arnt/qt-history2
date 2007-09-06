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

#ifndef Patternist_PatternPlatform_H
#define Patternist_PatternPlatform_H

#include <QFlags>
#include <QRegExp>

#include "FunctionCall.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Contains functionality for functions and expressions that
     * uses regular expressions.
     *
     * @ingroup Patternist_utils
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class PatternPlatform : public FunctionCall
    {
    public:
        /**
         * @see <a href="http://www.w3.org/TR/xpath-functions/#flags">XQuery 1.0 and
         * XPath 2.0 Functions and Operators, 7.6.1.1 Flags</a>
         */
        enum Flag
        {
            /**
             * No flags are set. Default behavior is used.
             */
            NoFlags             = 0,

            /**
             * Flag @c s
             */
            DotAllMode          = 1,

            /**
             * Flag @c m
             */
            MultiLineMode       = 2,

            /**
             * Flag @c i
             */
            CaseInsensitive     = 4,

            /**
             * Flag @c x
             */
            SimplifyWhitespace  = 8
        };
        typedef QFlags<Flag> Flags;

        /**
         * Enum telling whether the flags, pattern, or both
         * have been compiled at compile time.
         */
        enum PreCompiledPart
        {
            NoPart          = 0,
            OnlyPattern     = 1,
            OnlyFlags       = 2,
            FlagsAndPattern = OnlyPattern | OnlyFlags

        };
        typedef QFlags<PreCompiledPart> PreCompiledParts;

        /**
         * @short This constructor is protected, because this class is supposed to be sub-classed.
         *
         * @param flagsPosition an index position specifying the operand containing the pattern
         * flags.
         */
        PatternPlatform(const qint8 flagsPosition);

        virtual Expression::Ptr compress(const StaticContext::Ptr &context);

        /**
         * Retrieves the pattern supplied in the arguments, taking care of compiling it,
         * settings its flags, and everything else required for getting it ready to use. If an error
         * occurs, an appropriate error is raised via @p context.
         */
        const QRegExp pattern(const DynamicContext::Ptr &context) const;

        /**
         * @returns the number of captures, also called parenthesized sub-expressions, the pattern has.
         *
         * If the pattern isn't precompiled, -1 is returned.
         */
        inline int captureCount() const;

    private:
        Q_DISABLE_COPY(PatternPlatform)

        Flags parseFlags(const QString &flags,
                         const DynamicContext::Ptr &context) const;

        QRegExp parsePattern(const QString &pattern,
                             const DynamicContext::Ptr &context) const;

        static void applyFlags(const Flags flags, QRegExp &pattern);

        /**
         * The parts that have been pre-compiled at compile time.
         */
        PreCompiledParts    m_compiledParts;
        Flags               m_flags;
        QRegExp             m_pattern;
        const qint8         m_flagsPosition;
    };

    inline int PatternPlatform::captureCount() const
    {
        if((m_compiledParts & OnlyPattern) == OnlyPattern)
            return m_pattern.numCaptures();
        else
            return -1;
    }

    Q_DECLARE_OPERATORS_FOR_FLAGS(PatternPlatform::Flags)
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
