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

#ifndef QTEST_GUI_H
#define QTEST_GUI_H

#include <QtTest/qtestassert.h>
#include <QtTest/qtest.h>
#include <QtTest/qtestevent.h>
#include <QtTest/qtestmouse.h>
#include <QtTest/qtestkeyboard.h>

#include <QtGui/qicon.h>
#include <QtGui/qpixmap.h>

#if 0
// inform syncqt
#pragma qt_no_master_include
#endif

namespace QTest
{

template<>
inline bool qCompare(QIcon const &t1, QIcon const &t2, const char *actual, const char *expected,
                    const char *file, int line)
{
    QTEST_ASSERT(sizeof(QIcon) == sizeof(void *));
    return qCompare<void *>(*reinterpret_cast<void * const *>(&t1),
                   *reinterpret_cast<void * const *>(&t2), actual, expected, file, line);
}

template<>
inline bool qCompare(QPixmap const &t1, QPixmap const &t2, const char *actual, const char *expected,
                    const char *file, int line)
{
    return qCompare(t1.toImage(), t2.toImage(), actual, expected, file, line);
}

}

/* compatibility */

inline static bool pixmapsAreEqual(const QPixmap *actual, const QPixmap *expected)
{
    if (!actual && !expected)
        return true;
    if (!actual || !expected)
        return false;
    if (actual->isNull() && expected->isNull())
        return true;
    if (actual->isNull() || expected->isNull() || actual->size() != expected->size())
        return false;
    return actual->toImage() == expected->toImage();
}

#endif
