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

#ifndef QTEST_H
#define QTEST_H

#include "QtTest/qtest_global.h"
#include "QtTest/qtestcase.h"
#include "QtTest/qtestdata.h"

#include <QtCore/qbytearray.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qobject.h>

Q_DECLARE_METATYPE(QStringList)

namespace QTest
{
inline bool compare_strs(QString const &t1, QString const &t2, const char *file, int line)
{
     char msg[1024];
     msg[0] = '\0';
     bool isOk = true;
     if (t1 != t2) {
         qt_snprintf(msg, 1024, "Compared values of type QString are not the same.\n"
                                "   Actual  : '%s'\n"
                                "   Expected: '%s'", t1.toLatin1().constData(), t2.toLatin1().constData());
         isOk = false;
     } else {
         qt_snprintf(msg, 1024, "COMPARE('%s', type QString)", t1.toLatin1().constData());
     }
     return compare_helper(isOk, msg, file, line);;
}
template<>
inline bool compare(QString const &t1, QString const &t2, const char *file, int line)
{
    return compare_strs(t1, t2, file, line);
}
#ifndef QTEST_NO_PARTIAL_SPECIALIZATIONS
template<>
#endif
inline bool compare(QLatin1String const &t1, QLatin1String const &t2, const char *file, int line)
{
    return compare_strs(t1, t2, file, line);
}
#ifndef QTEST_NO_PARTIAL_SPECIALIZATIONS
template<>
#endif
inline bool compare(QString const &t1, QLatin1String const &t2, const char *file, int line)
{
    return compare_strs(t1, t2, file, line);
}
#ifndef QTEST_NO_PARTIAL_SPECIALIZATIONS
template<>
#endif
inline bool compare(QLatin1String const &t1, QString const &t2, const char *file, int line)
{
    return compare_strs(t1, t2, file, line);
}
template<>
inline bool compare(QStringList const &t1, QStringList const &t2, const char *file, int line)
{
    char msg[1024];
    msg[0] = '\0';
    bool isOk = true;
    if (t1.count() != t2.count()) {
        qt_snprintf(msg, 1024, "Compared QStringLists have different sizes.\n"
                    "   Actual size  : '%d'\n"
                    "   Expected size: '%d'", t1.count(), t2.count());
        isOk = false;
    }
    const int min = qMin(t1.count(), t2.count());
    for (int i = 0; i < min; ++i) {
        if (t1.at(i) != t2.at(i)) {
            qt_snprintf(msg, 1024, "Compared QStringLists differ at index %d.\n"
                        "   Actual  : '%s'\n"
                        "   Expected: '%s'", i, t1.at(i).toLatin1().constData(),
                        t2.at(i).toLatin1().constData());
            isOk = false;
        }
    }
    return compare_helper(isOk, msg, file, line);
}

template<>
inline char *toString(const QTime &time)
{
    return time.isValid()
        ? qstrdup(time.toString(QLatin1String("hh:mm:ss.zzz")).toLatin1())
        : qstrdup("Invalid QTime");
}

template<>
inline char *toString(const QDate &date)
{
    return date.isValid()
        ? qstrdup(date.toString(QLatin1String("yyyy/MM/dd")).toLatin1())
        : qstrdup("Invalid QDate");
}

template<>
inline char *toString(const QDateTime &dateTime)
{
    return dateTime.isValid()
        ? qstrdup(dateTime.toString(QLatin1String("yyyy/MM/dd hh:mm:ss.zzz")).toLatin1())
        : qstrdup("Invalid QDateTime");
}

template<>
inline char *toString(const QChar &c)
{
    return qstrdup(QString::fromLatin1("QChar: '%1' (0x%2)").arg(c).arg(QString::number(c.unicode(), 16)).toLatin1().constData());
}

template <typename T>
inline bool compare(QFlags<T> const &t1, T const &t2, const char *file, int line)
{
    return compare(int(t1), int(t2), file, line);
}

template <typename T>
inline bool compare(QFlags<T> const &t1, int const &t2, const char *file, int line)
{
    return compare(int(t1), t2, file, line);
}

}

#define QTEST_APPLESS_MAIN(TestObject) \
int main(int argc, char *argv[]) \
{ \
    TestObject tc; \
    return QTest::exec(&tc, argc, argv); \
}

#define QTEST_NOOP_MAIN \
int main(int argc, char *argv[]) \
{ \
    QObject tc; \
    return QTest::exec(&tc, argc, argv); \
}

#ifdef QT_GUI_LIB

#include "QtTest/qtest_gui.h"

#define QTEST_MAIN(TestObject) \
int main(int argc, char *argv[]) \
{ \
    QApplication app(argc, argv); \
    TestObject tc; \
    return QTest::exec(&tc, argc, argv); \
}

#else

#define QTEST_MAIN(TestObject) \
int main(int argc, char *argv[]) \
{ \
    QCoreApplication app(argc, argv); \
    TestObject tc; \
    return QTest::exec(&tc, argc, argv); \
}

#endif //QT_GUI_LIB

#endif

