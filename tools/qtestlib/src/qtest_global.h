/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTEST_GLOBAL_H
#define QTEST_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef QTEST_EMBED
# define Q_TESTLIB_EXPORT
#else
# ifdef QTESTLIB_MAKEDLL
#  define Q_TESTLIB_EXPORT Q_DECL_EXPORT
# else
#  define Q_TESTLIB_EXPORT Q_DECL_IMPORT
# endif
#endif

#if defined (Q_CC_MSVC) || defined (Q_CC_SUN) || defined (Q_CC_XLC) || (defined (Q_CC_GNU) && (__GNUC__ - 0 < 3))
# define QTEST_NO_PARTIAL_SPECIALIZATIONS
#endif

#define QTEST_VERSION     0x040100
#define QTEST_VERSION_STR "4.1.0"

namespace QTest
{
    enum SkipMode { SkipSingle = 1, SkipAll = 2 };
    enum TestFailMode { Abort = 1, Continue = 2 };

    int Q_TESTLIB_EXPORT qt_snprintf(char *str, int size, const char *format, ...);
    int Q_TESTLIB_EXPORT defaultEventDelay();
}

#endif
