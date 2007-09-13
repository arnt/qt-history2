/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTEST_GLOBAL_H
#define QTEST_GLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

#ifdef QTEST_EMBED
# define Q_TESTLIB_EXPORT
#elif !defined(QT_SHARED)
# define Q_TESTLIB_EXPORT
#else
# ifdef QTESTLIB_MAKEDLL
#  define Q_TESTLIB_EXPORT Q_DECL_EXPORT
# else
#  define Q_TESTLIB_EXPORT Q_DECL_IMPORT
# endif
#endif

#if (defined (Q_CC_MSVC) && _MSC_VER < 1310) || defined (Q_CC_SUN) || defined (Q_CC_XLC) || (defined (Q_CC_GNU) && (__GNUC__ - 0 < 3))
# define QTEST_NO_SPECIALIZATIONS
#endif

#if (defined Q_CC_HPACC) && (defined __ia64)
# ifdef Q_TESTLIB_EXPORT
#  undef Q_TESTLIB_EXPORT
# endif
# define Q_TESTLIB_EXPORT
#endif

#define QTEST_VERSION     0x040400
#define QTEST_VERSION_STR "4.4.0"

namespace QTest
{
    enum SkipMode { SkipSingle = 1, SkipAll = 2 };
    enum TestFailMode { Abort = 1, Continue = 2 };

    int Q_TESTLIB_EXPORT qt_snprintf(char *str, int size, const char *format, ...);
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
