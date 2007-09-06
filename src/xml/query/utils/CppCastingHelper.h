/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the Patternist project on Trolltech Labs.
**
** $TROLLTECH_GPL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************
*/

#ifndef Patternist_CppCastingHelper_H
#define Patternist_CppCastingHelper_H

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Provides convenience methods for performing static casts between C++ classes.
     *
     * In Patternist, it is very common to do up-casts from Expression or Item, which typically
     * involves writing messy code. Such an old-way cast looks like this:
     *
     * @code
     * static_cast<const MyClass *>(myInstance.get())->myClassMember()
     * @endcode
     *
     * CppCastingHelper provides the convenience method as() for this, which is functionally
     * equivalent to the above code, but simpler:
     *
     * @code
     * myInstance->as<MyClass>()->myClassMember()
     * @endcode
     *
     * The as() function performs a static cast.
     *
     * By using CppCastingHelper, this is achieved:
     *
     * - Const correctness is automatically taken care of
     * - Less code to write
     * - When compiling in debug mode, the as() functions uses a @c dynamic_cast to verify that the
     *   static casts are properly done, such that sensible error messages are given when the casts
     *   are invalid. It also traps invalid casts which nevertheless happen to work on a particular
     *   platform/compiler/hardware architecture.
     *
     * CppCastingHelper is a template class where the TSubClass parameter must be the class
     * inheriting CppCastingHelper. See Item or Expression for demonstration.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    template<typename TSubClass>
    class CppCastingHelper
    {
    public:

        /**
         * Casts this instance to:
         *
         * @code
         * const TCastTarget *
         * @endcode
         *
         * and returns the result.
         *
         * When compiled in debug mode, this function perform a @c dynamic_cast, in order to
         * check the correctness of the cast.
         */
        template<typename TCastTarget>
        inline const TCastTarget *as() const
        {
            /* Qt doesn't have a define for when it's building with no RTTI. It
             * needs to be added, and then this assert being conditionally
             * enabled. */
            /*
            Q_ASSERT_X(dynamic_cast<const TCastTarget *>(static_cast<const TSubClass *>(this)),
                       Q_FUNC_INFO,
                       "The cast is invalid. This class does not inherit the cast target.");
                       */
            return static_cast<const TCastTarget *>(static_cast<const TSubClass *>(this));
        }

        /**
         * Casts this instance to:
         *
         * @code
         * TCastTarget *
         * @endcode
         *
         * and returns the result.
         *
         * When compiled in debug mode, a @c dynamic_cast is attempted, in order to
         * check the correctness of the cast.
         */
        template<typename TCastTarget>
        inline TCastTarget *as()
        {
            /* Qt doesn't have a define for when it's building with no RTTI. It
             * needs to be added, and then this assert being conditionally
             * enabled. */
            Q_ASSERT_X(dynamic_cast<TCastTarget *>(static_cast<TSubClass *>(this)),
                       Q_FUNC_INFO,
                       "The cast is invalid. This class does not inherit the cast target.");
            return static_cast<TCastTarget *>(static_cast<TSubClass *>(this));
        }

    protected:
        /**
         * This constructor is protected because this class must be sub-classed.
         */
        inline CppCastingHelper() {}
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
