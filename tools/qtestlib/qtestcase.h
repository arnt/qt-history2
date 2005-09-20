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

#ifndef QTESTCASE_P_H
#define QTESTCASE_P_H

#include <QtCore/qnamespace.h>
#include <QtCore/qmetatype.h>

#include "QtTest/qtest_global.h"

#define VERIFY(statement) \
do {\
    if (!QTest::verify((statement), #statement, "", __FILE__, __LINE__))\
        return;\
} while (0)

#define FAIL(message) \
do {\
    QTest::fail(message, __FILE__, __LINE__);\
    return;\
} while (0)

#define VERIFY2(statement, description) \
do {\
    if (statement) {\
        if (!QTest::verify(true, #statement, (description), __FILE__, __LINE__))\
            return;\
    } else {\
        if (!QTest::verify(false, #statement, (description), __FILE__, __LINE__))\
            return;\
    }\
} while (0)

#define COMPARE(actual, expected) \
do {\
    if (!QTest::compare(actual, expected, #actual, #expected, __FILE__, __LINE__))\
        return;\
} while (0)

#define SKIP(statement, mode) \
do {\
    QTest::skip(statement, QTest::mode, __FILE__, __LINE__);\
    return;\
} while (0)

#define EXPECT_FAIL(dataIndex, comment, mode)\
do {\
    if (!QTest::expectFail(dataIndex, comment, QTest::mode, __FILE__, __LINE__))\
        return;\
} while (0)

#define FETCH(type, name)\
    type name = *static_cast<type *>(QTest::data(#name, qMetaTypeId<type >()))

#define FETCH_GLOBAL(type, name)\
    type name = *static_cast<type *>(QTest::globalData(#name, qMetaTypeId<type >()))

#define DEPENDS_ON(funcName)

#define TEST(actual, testElement)\
do {\
    if (!QTest::test(actual, testElement, #actual, #testElement, __FILE__, __LINE__))\
        return;\
} while (0)

#define WARN(msg)\
    QTest::warn(msg)

class QObject;
class QTestData;

#define QTEST_COMPARE_DECL(KLASS)\
    template<> Q_TESTLIB_EXPORT char *toString<KLASS >(const KLASS &);

namespace QTest
{
    template <typename T>
    inline char *toString(const T &)
    {
        return 0;
    }

    Q_TESTLIB_EXPORT char *toString(const char *);
    Q_TESTLIB_EXPORT char *toString(const void *);

    Q_TESTLIB_EXPORT int exec(QObject *testObject, int argc = 0, char **argv = 0);

    Q_TESTLIB_EXPORT bool verify(bool statement, const char *statementStr, const char *description,
                                 const char *file, int line);
    Q_TESTLIB_EXPORT void fail(const char *statementStr, const char *file, int line);
    Q_TESTLIB_EXPORT void skip(const char *message, SkipMode mode, const char *file, int line);
    Q_TESTLIB_EXPORT bool expectFail(const char *dataIndex, const char *comment, TestFailMode mode,
                           const char *file, int line);
    Q_TESTLIB_EXPORT void warn(const char *message);
    Q_TESTLIB_EXPORT void ignoreMessage(QtMsgType type, const char *message);

    Q_TESTLIB_EXPORT void *data(const char *tagName, int typeId);
    Q_TESTLIB_EXPORT void *globalData(const char *tagName, int typeId);
    Q_TESTLIB_EXPORT void *elementData(const char *elementName, int metaTypeId);
    Q_TESTLIB_EXPORT QObject *testObject();

    Q_TESTLIB_EXPORT const char *currentTestFunction();
    Q_TESTLIB_EXPORT const char *currentDataTag();
    Q_TESTLIB_EXPORT bool currentTestFailed();

    Q_TESTLIB_EXPORT Qt::Key asciiToKey(char ascii);
    Q_TESTLIB_EXPORT char keyToAscii(Qt::Key key);

    Q_TESTLIB_EXPORT bool compare_helper(bool success, const char *msg, const char *file, int line);
    Q_TESTLIB_EXPORT bool compare_helper(bool success, const char *msg, char *val1, char *val2,
                                         const char *expected, const char *actual,
                                         const char *file, int line);
    Q_TESTLIB_EXPORT void sleep(int ms);
    Q_TESTLIB_EXPORT void addColumnInternal(int id, const char *name);

    template <typename T>
    inline void addColumn(const char *name, T * = 0)
    {
        addColumnInternal(qMetaTypeId<T>(), name);
    }
    Q_TESTLIB_EXPORT QTestData &newRow(const char *dataTag);

    template <typename T>
    inline bool compare(T const &t1, T const &t2, const char *actual, const char *expected,
                        const char *file, int line)
    {
        return (t1 == t2)
            ? compare_helper(true, "COMPARE()", file, line)
            : compare_helper(false, "Compared values are not the same",
                             toString<T>(t1), toString<T>(t2), actual, expected, file, line);
    }

    template <>
    Q_TESTLIB_EXPORT bool compare<float>(float const &t1, float const &t2,
                    const char *actual, const char *expected, const char *file, int line);

    template <>
    Q_TESTLIB_EXPORT bool compare<double>(double const &t1, double const &t2,
                    const char *actual, const char *expected, const char *file, int line);

    inline bool compare_ptr_helper(const void *t1, const void *t2, const char *actual,
                                   const char *expected, const char *file, int line)
    {
        return (t1 == t2)
            ? compare_helper(true, "COMPARE()", file, line)
            : compare_helper(false, "Compared pointers are not the same",
                             toString(t1), toString(t2), actual, expected, file, line);
    }

    inline bool compare_string_helper(const char *t1, const char *t2, const char *actual,
                                      const char *expected, const char *file, int line)
    {
        return (t1 == t2)
            ? compare_helper(true, "COMPARE()", file, line)
            : compare_helper(false, "Compared strings are not the same",
                             toString(t1), toString(t2), actual, expected, file, line);
    }

#ifndef qdoc
    QTEST_COMPARE_DECL(short)
    QTEST_COMPARE_DECL(ushort)
    QTEST_COMPARE_DECL(int)
    QTEST_COMPARE_DECL(uint)
    QTEST_COMPARE_DECL(long)
    QTEST_COMPARE_DECL(ulong)
    QTEST_COMPARE_DECL(qint64)
    QTEST_COMPARE_DECL(quint64)

    QTEST_COMPARE_DECL(float)
    QTEST_COMPARE_DECL(double)
    QTEST_COMPARE_DECL(char)
    QTEST_COMPARE_DECL(bool)
#endif

#ifndef QTEST_NO_PARTIAL_SPECIALIZATIONS
    template <typename T1, typename T2>
    bool compare(T1 const &, T2 const &, const char *, const char *, const char *, int);

    template <typename T>
    inline bool compare(const T *t1, const T *t2, const char *actual, const char *expected,
                        const char *file, int line)
    {
        return compare_ptr_helper(t1, t2, actual, expected, file, line);
    }
    template <typename T>
    inline bool compare(T *t1, T *t2, const char *actual, const char *expected,
                        const char *file, int line)
    {
        return compare_ptr_helper(t1, t2, actual, expected, file, line);
    }

    template <typename T1, typename T2>
    inline bool compare(const T1 *t1, const T2 *t2, const char *actual, const char *expected,
                        const char *file, int line)
    {
        return compare_ptr_helper(t1, static_cast<const T1 *>(t2), actual, expected, file, line);
    }
    template <typename T1, typename T2>
    inline bool compare(T1 *t1, T2 *t2, const char *actual, const char *expected,
                        const char *file, int line)
    {
        return compare_ptr_helper(const_cast<const T1 *>(t1),
                static_cast<const T1 *>(const_cast<const T2 *>(t2)), actual, expected, file, line);
    }
#endif
#ifndef QTEST_NO_PARTIAL_SPECIALIZATIONS
    template<>
#endif
    inline bool compare(const char *t1, const char *t2, const char *actual, const char *expected,
                        const char *file, int line)
    {
        return compare_string_helper(t1, t2, actual, expected, file, line);
    }
#ifndef QTEST_NO_PARTIAL_SPECIALIZATIONS
    template<>
#endif
    inline bool compare(char *t1, char *t2, const char *actual, const char *expected,
                        const char *file, int line)
    {
        return compare_string_helper(t1, t2, actual, expected, file, line);
    }

    template <class T>
    inline bool test(const T& actual, const char *elementName, const char *actualStr,
                     const char *expected, const char *file, int line)
    {
        return compare(actual, *static_cast<const T *>(QTest::elementData(elementName,
                       QMetaTypeId<T>::qt_metatype_id())), actualStr, expected, file, line);
    }
}

#undef QTEST_COMPARE_DECL

#endif

