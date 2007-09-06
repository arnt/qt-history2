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

#ifndef Patternist_PatternistTest_H
#define Patternist_PatternistTest_H

class QFile;
#include <QObject>

namespace Patternist
{
    /**
     * @short Contains QTestLib tests verifying the @c patternist command line utility.
     *
     * @ingroup Patternist_testing
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class PatternistTest : public QObject
    {
        Q_OBJECT
    public:
        /**
         * Initializes private data.
         */
        PatternistTest();

    private Q_SLOTS:
        void shouldFail_data();
        void shouldFail();

        void initTestCase();
        void cleanupTestCase();

    private:
        QFile *const m_validQueryFile;
    };
}

#endif
// vim: et:ts=4:sw=4:sts=4
