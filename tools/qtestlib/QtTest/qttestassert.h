#ifndef QTESTASSERT_H
#define QTESTASSERT_H

#include <QtCore/qglobal.h>

#define QTEST_ASSERT(cond) do {if(!(cond))qt_assert(#cond,__FILE__,__LINE__);} while (0)

#define QTEST_ASSERT_X(cond, where, what) do {if(!(cond))qt_assert_x(where, what,__FILE__,__LINE__);} while (0)

#endif

