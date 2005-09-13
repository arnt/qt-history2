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

#define QTEST_VERSION     0x020000
#define QTEST_VERSION_STR "2.0.0"

namespace QTest
{
    enum SkipMode { SkipSingle = 1, SkipAll = 2 };
    enum TestFailMode { Abort = 1, Continue = 2 };

    int Q_TESTLIB_EXPORT qt_snprintf(char *str, int size, const char *format, ...);
    int Q_TESTLIB_EXPORT defaultEventDelay();
}

#endif

