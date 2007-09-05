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

#ifndef Patternist_DoxygenExamplesUnitTest
#define Patternist_DoxygenExamplesUnitTest

#include <QObject>

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @defgroup Patternist_testing Test Utilities(internal)
     * @author Frans Englich <fenglich@trolltech.com>
     */

    /**
     * @short Performs unit testing of the Doxygen examples with QtTestLib.
     *
     * The purpose of the testing is to ensure that the examples do
     * what they claim to.
     *
     * @ingroup Patternist_testing
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DoxygenExamplesUnitTests : public QObject
    {
    public:
        Q_OBJECT

    private Q_SLOTS:
        void runTests();
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
