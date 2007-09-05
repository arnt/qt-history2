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

#ifndef Patternist_TestHelper_H
#define Patternist_TestHelper_H

/**
 * @file
 * @short Contains utility functions for QTestLib tests.
 */

QT_BEGIN_HEADER 

namespace Patternist
{
    class Cardinality;
}

namespace QTest
{
    /**
     * This function is used by Qt's QTestLib.
     *
     * @returns a display name of @p card for debugging purposes
     */
    template<> char *toString(const Patternist::Cardinality &card)
    {
        QByteArray ba("Cardinality(");
        ba += card.minimum();
        ba += ", ";
        ba += card.maximum();
        ba += ")";

        return qstrdup(ba.data());
    }

    /**
     * Compares a @c short to an @c int by casting the @c short to an @c int.
     */
    template <> static inline bool qCompare<short, int>(short const &a,
                                                        int const &b,
                                                        char const *p1,
                                                        char const *p2,
                                                        char const *p3,
                                                        int p4)
    {
        return qCompare(int(a), b, p1, p2, p3, p4);
    }

    /**
     * Compares an @c int to a <tt>qint64</tt> by casting the @c int to a <tt>qint64</tt>.
     */
    template <> static inline bool qCompare<qint64, int>(qint64 const &a,
                                                         int const &b,
                                                         char const *p1,
                                                         char const *p2,
                                                         char const *p3,
                                                         int p4)
    {
        return qCompare(a, qint64(b), p1, p2, p3, p4);
    }
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
