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

#include "QtTest/qtestcase.h"
#include "QtTest/qtestassert.h"

#include <QtCore/qobject.h>
#include <QtCore/qmetaobject.h>

#include "QtTest/private/qtestlog_p.h"
#include "QtTest/private/qtesttable_p.h"
#include "QtTest/qtestdata.h"
#include "QtTest/private/qtestresult_p.h"
#include "QtTest/private/qsignaldumper_p.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef Q_OS_WIN32
#include <windows.h> // for Sleep
#endif
#ifdef Q_OS_UNIX
#include <time.h>
#endif


/*! \namespace QTest

   \brief The QTest namespace contains all the functions and
   declarations that are related to the QTestLib tool.

   Please refer to the \l{QTestLib Manual} documentation for information on
   how to write unit tests.
*/

/*! \macro QVERIFY(condition)

   \relates QTest

   The QVERIFY() macro checks whether the \a condition is true or not. If it is
   true, execution continues. If not, a failure is recorded in the test log
   and the test won't be executed further.

   \bold {Note:} This macro can only be used in a test function that is invoked
   by the test framework.

   Example:
   \code
   QVERIFY(1 + 1 == 2);
   \endcode

   \sa QCOMPARE()
*/

/*! \macro QCOMPARE(actual, expected)

   \relates QTest

   The QCOMPARE macro compares an \a actual value to an \a expected value using
   the equals operator. If \a actual and \a expected are identical, execution
   continues. If not, a failure is recorded in the test log and the test
   won't be executed further.

   QCOMPARE tries to output the contents of the values if the comparison fails,
   so it is visible from the test log why the comparison failed.

   \bold {Note:} QCOMPARE is very strict on the data types. Both \a
   actual and \a expected have to be of the same type, otherwise the
   test won't compile. This prohibits unspecified behavior from being
   introduced; that is behavior that usually occurs when the compiler
   implicitely casts the argument.

   Note that, for your own classes, you can use \l QTest::toString()
   to format values for outputting into the test log.

   \bold {Note:} This macro can only be used in a test function that is invoked
   by the test framework.

   Example:
   \code
   QCOMPARE(QString("hello").toUpper(), QString("HELLO"));
   \endcode

   \sa QVERIFY(), QTest::toString()
*/

/*! \macro QFETCH(type, name)

   \relates QTest

   The fetch macro creates a local variable named \a name with the type \a type
   on the stack. \a name has to match the element name from the test's data.
   If no such element exists, the test will assert.

   Assuming a test has the following data:

   \code
   void TestQString::toInt_data()
   {
       QTest::addColumn<QString>("aString");
       QTest::addColumn<int>("expected");

       QTest::newRow("positive value") << "42" << 42;
       QTest::newRow("negative value") << "-42" << -42;
       QTest::newRow("zero") << "0" << 0;
   }
   \endcode

   The test data has two elements, a QString called \c aString and an integer
   called \c expected. To fetch these values in the actual test:

   \code
   void TestQString::toInt()
   {
        QFETCH(QString, aString);
        QFETCH(int, expected);

        QCOMPARE(aString.toInt(), expected);
   }
   \endcode

   \c aString and \c expected are variables on the stack that are initialized with
   the current test data.

   \bold {Note:} This macro can only be used in a test function that is invoked
   by the test framework. The test function must have a _data function.
*/

/*! \macro QWARN(message)

   \relates QTest
   \threadsafe

   Appends \a message as a warning to the test log. This macro can be used anywhere
   in your tests.
*/

/*! \macro QFAIL(message)

   \relates QTest

   This macro can be used to force a test failure. The test stops
   executing and the failure \a message is appended to the test log.

   \bold {Note:} This macro can only be used in a test function that is invoked
   by the test framework.

   Example:

   \code
   if (sizeof(int) != 4)
       QFAIL("This test has not been ported to this platform yet.");
   \endcode
*/

/*! \macro QTEST(actual, testElement)

   \relates QTest

   QTEST() is a convenience macro for \l QCOMPARE() that compares
   the value \a actual with the element \a testElement from the test's data.
   If there is no such element, the test asserts.

   Apart from that, QTEST() behaves exactly as \l QCOMPARE().

   Instead of writing:

   \code
   QFETCH(QString, myString);
   QCOMPARE(QString("hello").toUpper(), myString);
   \endcode

   you can write:

   \code
   QTEST(QString("hello").toUpper(), "myString");
   \endcode

   \sa QCOMPARE()
*/

/*! \macro QSKIP(description, mode)

   \relates QTest

   The QSKIP() macro stops execution of the test without adding a failure to the
   test log. You can use it to skip tests that wouldn't make sense in the current
   configuration. The text \a description is appended to the test log and should
   contain an explanation why the test couldn't be executed. \a mode is a QTest::SkipMode
   and describes whether to proceed with the rest of the test data or not.

   \bold {Note:} This macro can only be used in a test function that is invoked
   by the test framework.

   Example:
   \code
   if (!QSqlDatabase::drivers().contains("SQLITE"))
       QSKIP("This test requires the SQLITE database driver", SkipAll);
   \endcode

   \sa QTest::SkipMode
*/

/*! \macro QEXPECT_FAIL(dataIndex, comment, mode)

   \relates QTest

   The QEXPECT_FAIL() macro marks the next \l QCOMPARE() or \l QVERIFY() as an
   expected failure. Instead of adding a failure to the test log, an expected
   failure will be reported.

   If a \l QVERIFY() or \l QCOMPARE() is marked as an expected failure,
   but passes instead, an unexpected pass (XPASS) is written to the test log.
   Unexpected passes behave exactly as failures, the test will stop executing.

   The parameter \a dataIndex describes for which entry in the test data the
   failure is expected. Pass an empty string (\c{""}) if the failure
   is expected for all entries or if no test data exists.

   \a comment will be appended to the test log for the expected failure.

   \a mode is a \l QTest::TestFailMode and sets whether the test should
   continue to execute or not.

   \bold {Note:} This macro can only be used in a test function that is invoked
   by the test framework.

   Example 1:
   \code
   QEXPECT_FAIL("", "Will fix in the next release", Continue);
   QCOMPARE(i, 42);
   QCOMPARE(j, 43);
   \endcode

   In the example above, an expected fail will be written into the test output
   if the variable \c i is not 42. If the variable \c i is 42, an unexpected pass
   is written instead. The QEXPECT_FAIL() has no influence on the second QCOMPARE()
   statement in the example.

   Example 2:
   \code
   QEXPECT_FAIL("data27", "Oh my, this is soooo broken", Abort);
   QCOMPARE(i, 42);
   \endcode

   The above testfunction will not continue executing for the test data
   entry \c{data27}.

   \sa QTest::TestFailMode, QVERIFY(), QCOMPARE()
*/

/*! \macro QTEST_MAIN(TestClass)

    \relates QTest

    Implements a main() function that instanciates a QApplication object and
    the \a TestClass, and executes all tests in the order they were defined.
    Use this macro to build stand-alone executables.

    Example:
    \code
    class TestQString: public QObject { ... };
    QTEST_MAIN(TestQString)
    \endcode

    \sa QTEST_APPLESS_MAIN(), QTest::exec()
*/

/*! \macro QTEST_APPLESS_MAIN(TestClass)

    \relates QTest

    Implements a main() function that executes all tests in \a TestClass.

    Behaves like \l QTEST_MAIN(), but doesn't instanciate a QApplication
    object. Use this macro for really simple stand-alone non-GUI tests.

    \sa QTEST_MAIN()
*/

/*! \macro QTEST_NOOP_MAIN()

    \relates QTest

    Implements a main() function with a test class that does absolutely nothing.
    Use this macro to create a test that produces valid test output but just
    doesn't execute any test, for example in conditional compilations:

    \code
    #ifdef Q_WS_X11
        QTEST_MAIN(MyX11Test)
    #else
        // do nothing on non-X11 platforms
        QTEST_NOOP_MAIN
    #endif
    \endcode

    \sa QTEST_MAIN()
*/

/*! \enum QTest::SkipMode

    This enum describes the modes for skipping tests during execution
    of the test data.

    \value SkipSingle Skips the current entry in the test table; continues
           execution of all the other entries in the table.

    \value SkipAll Skips all the entries in the test table; the test won't
           be executed further.

    \sa QSKIP()
*/

/*! \enum QTest::TestFailMode

    This enum describes the modes for handling an expected failure of the
    \l QVERIFY() or \l QCOMPARE() macros.

    \value Abort Aborts the execution of the test. Use this mode when it
           doesn't make sense to execute the test any further after the
           expected failure.

    \value Continue Continues execution of the test after the expected failure.

    \sa QEXPECT_FAIL()
*/

/*! \enum QTest::KeyAction

    This enum describes possible actions for key handling.

    \value Press    The key is pressed.
    \value Release  The key is released.
    \value Click    The key is clicked (pressed and released).
*/

/*! \enum QTest::MouseAction

    This enum describes possible actions for mouse handling.

    \value MousePress    A mouse button is pressed.
    \value MouseRelease  A mouse button is released.
    \value MouseClick    A mouse button is clicked (pressed and released).
    \value MouseDClick   A mouse button is double clicked (pressed and released twice).
    \value MouseMove     The mouse pointer has moved.
*/

/*! \fn void QTest::keyClick(QWidget *widget, char key, Qt::KeyboardModifiers modifier = Qt::NoModifier, int delay=-1)

    \overload

    Simulates clicking of \a key with an optional \a modifier on a \a widget. If \a delay is larger than 0, the test will wait for \a delay milliseconds.

    Example:
    \code
    QTest::keyClick(myWidget, 'a');
    \endcode

    The example above simulates clicking \c a on \c myWidget without
    any keyboard modifiers and without delay of the test.

    \sa QTest::keyClicks()
*/

/*! \fn void QTest::keyClick(QWidget *widget, Qt::Key key, Qt::KeyboardModifiers modifier = Qt::NoModifier, int delay=-1)

    Simulates clicking of \a key with an optional \a modifier on a \a widget. If \a delay is larger than 0, the test will wait for \a delay milliseconds.

    Examples:
    \code
    QTest::keyClick(myWidget, Qt::Key_Escape);

    QTest::keyClick(myWidget, Qt::Key_Escape, Qt::ShiftModifier, 200);
    \endcode

    The first example above simulates clicking the \c escape key on \c
    myWidget without any keyboard modifiers and without delay. The
    second example simulates clicking \c shift-escape on \c myWidget
    with a following 200 ms delay of the test.

    \sa QTest::keyClicks()
*/

/*! \fn void QTest::keyEvent(KeyAction action, QWidget *widget, Qt::Key key, Qt::KeyboardModifiers modifier = Qt::NoModifier, int delay=-1)

    Sends a Qt key event to \a widget with the given \a key and an associated \a action. Optionally, a keyboard \a modifier can be specified, as well as a \a delay (in milliseconds) of the test after sending the event.
*/

/*! \fn void QTest::keyEvent(KeyAction action, QWidget *widget, char ascii, Qt::KeyboardModifiers modifier = Qt::NoModifier, int delay=-1)

    \overload

    Sends a Qt key event to \a widget with the given key \a ascii and an associated \a action. Optionally, a keyboard \a modifier can be specified, as well as a \a delay (in milliseconds) of the test after sending the event.

*/

/*! \fn void QTest::keyPress(QWidget *widget, Qt::Key key, Qt::KeyboardModifiers modifier = Qt::NoModifier, int delay=-1)

    Simulates pressing a \a key with an optional \a modifier on a \a widget. If \a delay is larger than 0, the test will wait for \a delay milliseconds.

    \bold {Note:} At some point you should release the key using \l keyRelease().

    \sa QTest::keyRelease(), QTest::keyClick()
*/

/*! \fn void QTest::keyPress(QWidget *widget, char key, Qt::KeyboardModifiers modifier = Qt::NoModifier, int delay=-1)

    \overload

    Simulates pressing a \a key with an optional \a modifier on a \a widget. If \a delay is larger than 0, the test will wait for \a delay milliseconds.

    \bold {Note:} At some point you should release the key using \l keyRelease().

    \sa QTest::keyRelease(), QTest::keyClick()
*/

/*! \fn void QTest::keyRelease(QWidget *widget, Qt::Key key, Qt::KeyboardModifiers modifier = Qt::NoModifier, int delay=-1)

    Simulates releasing a \a key with an optional \a modifier on a \a widget. If \a delay is larger than 0, the test will wait for \a delay milliseconds.

    \sa QTest::keyPress(), QTest::keyClick()
*/

/*! \fn void QTest::keyRelease(QWidget *widget, char key, Qt::KeyboardModifiers modifier = Qt::NoModifier, int delay=-1)

    \overload

    Simulates releasing a \a key with an optional \a modifier on a \a widget. If \a delay is larger than 0, the test will wait for \a delay milliseconds.

    \sa QTest::keyClick()
*/


/*! \fn void QTest::keyClicks(QWidget *widget, const QString &sequence, Qt::KeyboardModifiers modifier = Qt::NoModifier, int delay=-1)

    Simulates clicking a \a sequence of keys on a \a
    widget. Optionally, a keyboard \a modifier can be specified as
    well as a \a delay (in milliseconds) of the test after each key
    click.

    Example:
    \code
    QTest::keyClicks(myWidget, "hello world");
    \endcode

    The example above simulates clicking the sequence of keys
    representing "hello world" on \c myWidget without any keyboard
    modifiers and without delay of the test.

    \sa QTest::keyClick()
*/

/*! \fn void QTest::mousePress(QWidget *widget, Qt::MouseButton button, Qt::KeyboardModifiers modifier = 0, QPoint pos = QPoint(), int delay=-1)

    Simulates pressing a mouse \a button with an optional \a modifier
    on a \a widget.  The position is defined by \a pos; the default
    position is the center of the widget. If \a delay is specified,
    the test will wait for the specified amount of milliseconds after
    the press.

    \sa QTest::mouseRelease(), QTest::mouseClick()
*/

/*! \fn void QTest::mouseRelease(QWidget *widget, Qt::MouseButton button, Qt::KeyboardModifiers modifier = 0, QPoint pos = QPoint(), int delay=-1)

    Simulates releasing a mouse \a button with an optional \a modifier
    on a \a widget.  The position of the release is defined by \a pos;
    the default position is the center of the widget. If \a delay is
    specified, the test will wait for the specified amount of
    milliseconds after releasing the button.

    \sa QTest::mousePress(), QTest::mouseClick()
*/

/*! \fn void QTest::mouseClick(QWidget *widget, Qt::MouseButton button, Qt::KeyboardModifiers modifier = 0, QPoint pos = QPoint(), int delay=-1)

    Simulates clicking a mouse \a button with an optional \a modifier
    on a \a widget.  The position of the click is defined by \a pos;
    the default position is the center of the widget. If \a delay is
    specified, the test will wait for the specified amount of
    milliseconds after pressing and after releasing the button.

    \sa QTest::mousePress(), QTest::mouseRelease()
*/

/*! \fn void QTest::mouseDClick(QWidget *widget, Qt::MouseButton button, Qt::KeyboardModifiers modifier = 0, QPoint pos = QPoint(), int delay=-1)

    Simulates double clicking a mouse \a button with an optional \a
    modifier on a \a widget.  The position of the click is defined by
    \a pos; the default position is the center of the widget. If \a
    delay is specified, the test will wait for the specified amount of
    milliseconds after each press and release.

    \sa QTest::mouseClick()
*/

/*! \fn void QTest::mouseMove(QWidget *widget, QPoint pos = QPoint(), int delay=-1)

    Moves the mouse pointer to a \a widget. If \a pos is not
    specified, the mouse pointer moves to the center of the widget. If
    a \a delay (in milliseconds) is given, the test will wait after
    moving the mouse pointer.
*/

/*!
    \fn char *QTest::toString(const T &value)

    Returns a textual representation of \a value. This function is used by
    \l QCOMPARE() to output verbose information in case of a test failure.

    You can add specializations of this function to your test to enable
    verbose output.

    \bold {Note:} The caller of toString() must delete the returned data
    using \c{delete[]}.  Your implementation should return a string
    created with \c{new[]} or qstrdup().

    Example:

    \code
    namespace QTest {
        template<>
        char *toString(const MyPoint &point)
        {
            QByteArray ba = "MyPoint(";
            ba += QByteArray::number(point.x()) + ", " + QByteArray::number(point.y());
            ba += ")";
            return qstrdup(ba.data());
        }
    }
    \endcode

    The example above defines a toString() specialization for a class
    called \c MyPoint. Whenever a comparison of two instances of \c
    MyPoint fails, \l QCOMPARE() will call this function to output the
    contents of \c MyPoint to the test log.

    \sa QCOMPARE()
*/

/*! \fn void QTest::qWait(int ms)

    Waits for \a ms milliseconds. While waiting, events will be processed and
    your test will stay responsive to user interface events or network communication.

    Example:
    \code
    int i = 0;
    while (myNetworkServerNotResponding() && i++ < 50)
        QTest::qWait(250);
    \endcode

    The code above will wait until the network server is responding for a
    maximum of about 12.5 seconds.

    \sa qSleep()
*/

/*! \fn int QTest::defaultMouseDelay()
    \internal
*/

/*! \fn int QTest::defaultKeyDelay()
    \internal
*/

/*! \fn int QTest::defaultEventDelay()
    \internal
*/

/*! \fn void QTest::mouseEvent(MouseAction action, QWidget *widget, Qt::MouseButton button, Qt::KeyboardModifiers stateKey, QPoint pos, int delay=-1)

    \internal
*/

/*! \fn void QTest::simulateEvent(QWidget *widget, bool press, int code, Qt::KeyboardModifiers modifier, QString text, bool repeat, int delay=-1)
    \internal
*/

/*! \fn void QTest::sendKeyEvent(KeyAction action, QWidget *widget, Qt::Key code, QString text, Qt::KeyboardModifiers modifier, int delay=-1)
    \internal
*/

/*! \fn void QTest::sendKeyEvent(KeyAction action, QWidget *widget, Qt::Key code, char ascii, Qt::KeyboardModifiers modifier, int delay=-1)
    \internal
*/

namespace QTest
{
    static bool skipCurrentTest = false;
    static QObject *currentTestObject = 0;

    struct TestFunction {
        TestFunction():function(0), data(0) {}
        ~TestFunction() { delete [] data; }
        int function;
        char *data;
    } testFuncs[512];
    static int lastTestFuncIdx = -1;

    static int keyDelay = -1;
    static int mouseDelay = -1;
    static int eventDelay = -1;
    static int keyVerbose = -1;

/*! \internal
 */
int qt_snprintf(char *str, int size, const char *format, ...)
{
    va_list ap;
    int res = 0;

    va_start(ap, format);
    ::qvsnprintf(str, size, format, ap);
    va_end(ap);
    str[size - 1] = '\0';

    char *idx = str;
    while (*idx) {
        if (((*idx < 0x20 && *idx != '\n' && *idx != '\t') || *idx > 0x7e))
            *idx = '?';
        ++idx;
    }
    return res;
}

bool Q_TESTLIB_EXPORT defaultKeyVerbose()
{
    if (keyVerbose == -1) {
        keyVerbose = ::qgetenv("QTEST_KEYEVENT_VERBOSE").constData() ? 1 : 0;
    }
    return keyVerbose == 1;
}

int Q_TESTLIB_EXPORT defaultEventDelay()
{
    if (eventDelay == -1) {
        if (qgetenv("QTEST_EVENT_DELAY").constData())
            eventDelay = atoi(::qgetenv("QTEST_EVENT_DELAY"));
        else
            eventDelay = 0;
    }
    return eventDelay;
}

int Q_TESTLIB_EXPORT defaultMouseDelay()
{
    if (mouseDelay == -1) {
        if (::qgetenv("QTEST_MOUSEEVENT_DELAY").constData())
            mouseDelay = atoi((::qgetenv("QTEST_MOUSEEVENT_DELAY")));
        else
            mouseDelay = defaultEventDelay();
    }
    return mouseDelay;
}

int Q_TESTLIB_EXPORT defaultKeyDelay()
{
    if (keyDelay == -1) {
        if (::qgetenv("QTEST_KEYEVENT_DELAY").constData())
            keyDelay = atoi(qgetenv("QTEST_KEYEVENT_DELAY").constData());
        else
            keyDelay = defaultEventDelay();
    }
    return keyDelay;
}

static bool isValidSlot(const QMetaMethod &sl)
{
    if (sl.access() != QMetaMethod::Private || !sl.parameterTypes().isEmpty()
        || qstrlen(sl.typeName()) || sl.methodType() != QMetaMethod::Slot)
        return false;
    const char *sig = sl.signature();
    int len = qstrlen(sig);
    if (len < 2)
        return false;
    if (sig[len - 2] != '(' || sig[len - 1] != ')')
        return false;
    if (len > 7 && strcmp(sig + (len - 7), "_data()") == 0)
        return false;
    if (strcmp(sig, "initTestCase()") == 0 || strcmp(sig, "cleanupTestCase()") == 0
        || strcmp(sig, "cleanup()") == 0 || strcmp(sig, "init()") == 0)
        return false;
    return true;
}

static void qPrintTestSlots()
{
    for (int i = 0; i < QTest::currentTestObject->metaObject()->methodCount(); ++i) {
        QMetaMethod sl = QTest::currentTestObject->metaObject()->method(i);
        if (isValidSlot(sl))
            printf("%s\n", sl.signature());
    }
}

static int qToInt(char *str)
{
    char *pEnd;
    int l = (int)strtol(str, &pEnd, 10);
    if (*pEnd != 0) {
        printf("Invalid numeric parameter: '%s'\n", str);
        exit(1);
    }
    return l;
}

static void qParseArgs(int argc, char *argv[])
{
    const char *testOptions =
         " options:\n"
         " -functions : Returns a list of current testfunctions\n"
         " -xml       : Outputs results as XML document\n"
         " -lightxml  : Outputs results as stream of XML tags\n"
         " -o filename: Writes all output into a file\n"
         " -v1        : Print enter messages for each testfunction\n"
         " -v2        : Also print out each QVERIFY/QCOMPARE/QTEST\n"
         " -vs        : Print every signal emitted\n"
         " -eventdelay ms    : Set default delay for mouse and keyboard simulation to ms milliseconds\n"
         " -keydelay ms      : Set default delay for keyboard simulation to ms milliseconds\n"
         " -mousedelay ms    : Set default delay for mouse simulation to ms milliseconds\n"
         " -keyevent-verbose : Turn on verbose messages for keyboard simulation\n"
         " -maxwarnings n    : Sets the maximum amount of messages to output.\n"
         "                     0 means unlimited, default: 2000\n"
         " -help      : This help\n";

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "--help") == 0
            || strcmp(argv[i], "/?") == 0) {
            printf(" Usage: %s [options] [testfunctions[:testdata]]...\n"
                   "    By default, all testfunction will be run.\n\n"
                   "%s", argv[0], testOptions);
            exit(0);
        } else if (strcmp(argv[i], "-functions") == 0) {
            qPrintTestSlots();
            exit(0);
        } else if (strcmp(argv[i], "-xml") == 0) {
            QTestLog::setLogMode(QTestLog::XML);
        } else if (strcmp(argv[i], "-lightxml") == 0) {
            QTestLog::setLogMode(QTestLog::LightXML);
        } else if (strcmp(argv[i], "-v1") == 0) {
            QTestLog::setVerboseLevel(1);
        } else if (strcmp(argv[i], "-v2") == 0) {
            QTestLog::setVerboseLevel(2);
        } else if (strcmp(argv[i], "-vs") == 0) {
            QSignalDumper::startDump();
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc) {
                printf("-o needs an extra parameter specifying the filename\n");
                exit(1);
            } else {
                QTestLog::redirectOutput(argv[++i]);
            }
        } else if (strcmp(argv[i], "-eventdelay") == 0) {
            if (i + 1 >= argc) {
                printf("-eventdelay needs an extra parameter to indicate the delay(ms)\n");
                exit(1);
            } else {
                QTest::eventDelay = qToInt(argv[++i]);
            }
        } else if (strcmp(argv[i], "-keydelay") == 0) {
            if (i + 1 >= argc) {
                printf("-keydelay needs an extra parameter to indicate the delay(ms)\n");
                exit(1);
            } else {
                QTest::keyDelay = qToInt(argv[++i]);
            }
        } else if (strcmp(argv[i], "-mousedelay") == 0) {
            if (i + 1 >= argc) {
                printf("-mousedelay needs an extra parameter to indicate the delay(ms)\n");
                exit(1);
            } else {
                QTest::mouseDelay = qToInt(argv[++i]);
            }
        } else if (strcmp(argv[i], "-maxwarnings") == 0) {
            if (i + 1 >= argc) {
                printf("-maxwarnings needs an extra parameter with the amount of warnings\n");
                exit(1);
            } else {
                QTestLog::setMaxWarnings(qToInt(argv[++i]));
            }
        } else if (strcmp(argv[i], "-keyevent-verbose") == 0) {
            QTest::keyVerbose = 1;
        } else if (strcmp(argv[i], "-qws") == 0) {
            // do nothing
        } else if (argv[i][0] == '-') {
            printf("Unknown option: '%s'\n\n%s", argv[i], testOptions);
            exit(1);
        } else {
            int colon = -1;
            char buf[512], *data=0;
            for(int off = 0; *(argv[i]+off); ++off) {
                if (*(argv[i]+off) == ':') {
                    colon = off;
                    break;
                }
            }
            if(colon != -1) {
                *(argv[i]+colon) = '\0';
                data = qstrdup(argv[i]+colon+1);
            }
            QTest::qt_snprintf(buf, 512, "%s()", argv[i]);
            int idx = QTest::currentTestObject->metaObject()->indexOfMethod(buf);
            if (idx < 0 || !isValidSlot(QTest::currentTestObject->metaObject()->method(idx))) {
                printf("Unknown testfunction: '%s'\n", buf);
                printf("Available testfunctions:\n");
                qPrintTestSlots();
                exit(1);
            }
            ++QTest::lastTestFuncIdx;
            QTest::testFuncs[QTest::lastTestFuncIdx].function = idx;
            QTest::testFuncs[QTest::lastTestFuncIdx].data = data;
            QTEST_ASSERT(QTest::lastTestFuncIdx < 512);
        }
    }
}


static bool qInvokeTestMethod(const char *slotName, const char *data=0)
{
    QTEST_ASSERT(slotName);

    char cur[512];
    QTestTable table;

    char *sl = qstrdup(slotName);
    sl[strlen(sl) - 2] = '\0';
    QTestResult::setCurrentTestFunction(sl);

    const QTestTable *gTable = QTestTable::globalTestTable();
    const int globalDataCount = gTable->dataCount();
    int curGlobalDataIndex = 0;
    do {
        if (!gTable->isEmpty())
            QTestResult::setCurrentGlobalTestData(gTable->testData(curGlobalDataIndex));

        if (curGlobalDataIndex == 0) {
            QTestResult::setCurrentTestLocation(QTestResult::DataFunc);
            QTest::qt_snprintf(cur, 512, "%s_data", sl);
            QMetaObject::invokeMethod(QTest::currentTestObject, cur, Qt::DirectConnection);
        }

        bool foundFunction = false;
        if (!QTest::skipCurrentTest) {
            int curDataIndex = 0;
            const int dataCount = table.dataCount();
            do {
                if (!data || !qstrcmp(data, table.testData(curDataIndex)->dataTag())) {
                    foundFunction = true;
                    if (!table.isEmpty())
                        QTestResult::setCurrentTestData(table.testData(curDataIndex));
                    QTestResult::setCurrentTestLocation(QTestResult::InitFunc);
                    QMetaObject::invokeMethod(QTest::currentTestObject, "init");
                    if (QTest::skipCurrentTest)
                        break;

                    QTestResult::setCurrentTestLocation(QTestResult::Func);
                    if (!QMetaObject::invokeMethod(QTest::currentTestObject, sl,
                                                  Qt::DirectConnection)) {
                        QTestResult::addFailure("Unable to execute slot", __FILE__, __LINE__);
                        break;
                    }

                    QTestResult::setCurrentTestLocation(QTestResult::CleanupFunc);
                    QMetaObject::invokeMethod(QTest::currentTestObject, "cleanup");
                    QTestResult::setCurrentTestLocation(QTestResult::NoWhere);
                    QTestResult::setCurrentTestData(0);

                    if (QTest::skipCurrentTest)
                        // check whether SkipAll was requested
                        break;
                    if (data)
                        break;
                }
                ++curDataIndex;
            } while (curDataIndex < dataCount);
        }
        QTest::skipCurrentTest = false;

        if (data && !foundFunction) {
            printf("Unknown testdata for function %s: '%s'\n", slotName, data);
            printf("Available testdata:\n");
            for(int i = 0; i < table.dataCount(); ++i)
                printf("%s\n", table.testData(i)->dataTag());
            return false;
        }

        QTestResult::setCurrentGlobalTestData(0);
        ++curGlobalDataIndex;
    } while (curGlobalDataIndex < globalDataCount);

    QTestResult::finishedCurrentTestFunction();
    delete[] sl;

    return true;
}

void *fetchData(QTestData *data, const char *tagName, int typeId)
{
    QTEST_ASSERT(typeId);
    QTEST_ASSERT_X(data, "QTest::fetchData()", "Test data requested, but no testdata available .");
    QTEST_ASSERT(data->parent());

    int idx = data->parent()->indexOf(tagName);

    if (typeId != data->parent()->elementTypeId(idx)) {
        qFatal("Requested type '%s' does not match available type '%s'.",
               QMetaType::typeName(typeId),
               QMetaType::typeName(data->parent()->elementTypeId(idx)));
    }

    return data->data(idx);
}

} // namespace

/*! \fn int QTest::qExec(QObject *testObject, int argc = 0, char **argv = 0)

    Executes tests declared in \a testObject. Optionally, the command line arguments
    \a argc and \a argv can be provided. For a list of recognized arguments, read
    \l {QTestLib Command Line Arguments}.

    For stand-alone tests, the convenience macro \l QTEST_MAIN() can
    be used to declare a main method that parses the command line arguments
    and executes the tests.

    \sa QTEST_MAIN()
*/
int QTest::qExec(QObject *testObject, int argc, char **argv)
{
#ifndef QT_NO_EXCEPTION
    try {
#endif

#if defined(Q_CC_MSVC)
    SetErrorMode(SetErrorMode(0) | SEM_NOGPFAULTERRORBOX);
#endif

    QTEST_ASSERT(testObject);
    QTEST_ASSERT(!currentTestObject);
    currentTestObject = testObject;

    const QMetaObject *mo = testObject->metaObject();
    QTEST_ASSERT(mo);

    QTestResult::setCurrentTestObject(mo->className());
    qParseArgs(argc, argv);

    QTestLog::startLogging();

    QTestResult::setCurrentTestFunction("initTestCase");
    QTestResult::setCurrentTestLocation(QTestResult::DataFunc);
    QTestTable *gTable = QTestTable::globalTestTable();
    QMetaObject::invokeMethod(testObject, "initTestCase_data", Qt::DirectConnection);

    if (!QTest::skipCurrentTest) {
        QTestResult::setCurrentTestLocation(QTestResult::Func);
        QMetaObject::invokeMethod(testObject, "initTestCase");
        QTestResult::finishedCurrentTestFunction();

        if (lastTestFuncIdx >= 0) {
            for (int i = 0; i <= lastTestFuncIdx; ++i) {
                qInvokeTestMethod(mo->method(testFuncs[i].function).signature(), testFuncs[i].data);
            }
        } else {
            int sc = mo->methodCount();
            for (int i = 0; i < sc; ++i) {
                QMetaMethod sl = mo->method(i);
                if (!isValidSlot(sl))
                    continue;
                qInvokeTestMethod(sl.signature());
            }
        }

        QTestResult::setCurrentTestFunction("cleanupTestCase");
        QMetaObject::invokeMethod(testObject, "cleanupTestCase");
    }
    QTestResult::finishedCurrentTestFunction();
    QTestResult::setCurrentTestFunction(0);
    delete gTable; gTable = 0;

#ifndef QT_NO_EXCEPTION
    } catch (...) {
        QTestResult::addFailure("Caught unhandled exception", __FILE__, __LINE__);
        if (QTestResult::currentTestFunction()) {
            QTestResult::finishedCurrentTestFunction();
            QTestResult::setCurrentTestFunction(0);
        }

        QTestLog::stopLogging();
        return -1;
    }
#endif

    QTestLog::stopLogging();
    currentTestObject = 0;
#if defined(QTEST_NOEXITCODE) || defined(QT_BUILD_INTERNAL)
    return 0;
#else
    return QTestResult::failCount();
#endif
}

/*! \internal
 */
void QTest::qFail(const char *statementStr, const char *file, int line)
{
    QTestResult::addFailure(statementStr, file, line);
}

/*! \internal
 */
bool QTest::qVerify(bool statement, const char *statementStr, const char *description,
                   const char *file, int line)
{
    return QTestResult::verify(statement, statementStr, description, file, line);
}

/*! \internal
 */
void QTest::qSkip(const char *message, QTest::SkipMode mode,
                 const char *file, int line)
{
    QTestResult::addSkip(message, mode, file, line);
    if (mode == QTest::SkipAll)
        skipCurrentTest = true;
}

/*! \internal
 */
bool QTest::qExpectFail(const char *dataIndex, const char *comment,
                       QTest::TestFailMode mode, const char *file, int line)
{
    return QTestResult::expectFail(dataIndex, comment, mode, file, line);
}

/*! \internal
 */
void QTest::qWarn(const char *message)
{
    QTestLog::warn(message);
}

/*! \fn void QTest::ignoreMessage(QtMsgType type, const char *message)

    Ignores messages created by qDebug() or qWarning(). If the \a message
    with the corresponding \a type is outputted, it will be removed from the
    test log. If the test finished and the \a message was not outputted,
    a test failure is appended to the test log.

    \bold {Note:} Invoking this function will only ignore one message.
    If the message you want to ignore is outputted twice, you have to
    call ignoreMessage() twice, too.

    Example:
    \code
    QDir dir;

    QTest::ignoreMessage(QtWarningMessage, "QDir::mkdir: Empty or null file name(s)");
    dir.mkdir("");
    \endcode

    The example above tests that QDir::mkdir() outputs the right warning when invoked
    with an invalid file name.
*/
void QTest::ignoreMessage(QtMsgType type, const char *message)
{
    QTestResult::ignoreMessage(type, message);
}

/*! \internal
 */
void *QTest::qData(const char *tagName, int typeId)
{
    return fetchData(QTestResult::currentTestData(), tagName, typeId);
}

/*! \internal
 */
void *QTest::qGlobalData(const char *tagName, int typeId)
{
    return fetchData(QTestResult::currentGlobalTestData(), tagName, typeId);
}

/*! \internal
 */
void *QTest::qElementData(const char *tagName, int metaTypeId)
{
    QTEST_ASSERT(tagName);
    QTestData *data = QTestResult::currentTestData();
    QTEST_ASSERT(data);
    QTEST_ASSERT(data->parent());

    int idx = data->parent()->indexOf(tagName);
    QTEST_ASSERT(idx != -1);
    QTEST_ASSERT(data->parent()->elementTypeId(idx) == metaTypeId);

    return data->data(data->parent()->indexOf(tagName));
}

/*! \internal
 */
void QTest::addColumnInternal(int id, const char *name)
{
    QTestTable *tbl = QTestTable::currentTestTable();
    QTEST_ASSERT_X(tbl, "QTest::addColumn()", "Cannot add testdata outside of a _data slot.");

    tbl->addColumn(id, name);
}

/*!
    Appends a new row to the current test data. \a dataTag is the name of
    the testdata that will appear in the test output. Returns a QTestData reference
    that can be used to stream in data.

    Example:
    \code
    void myTestFunction_data() {
        QTest::addColumn<QString>("aString");
        QTest::newRow("just hello") << QString("hello");
        QTest::newRow("a null string") << QString();
    }
    \endcode

    \bold {Note:} This macro can only be used in a test's data function
    that is invoked by the test framework.

    ### link to test-driven

    \sa addColumn(), QTestData, QFETCH()
*/
QTestData &QTest::newRow(const char *dataTag)
{
    QTestTable *tbl = QTestTable::currentTestTable();
    QTEST_ASSERT_X(tbl, "QTest::addColumn()", "Cannot add testdata outside of a _data slot.");

    return *tbl->newData(dataTag);
}

/*! \fn void addColumn(const char *name, T *dummy = 0)

    Adds a column with type \c{T} to the current test data.
    \a name is the name of the column. \a dummy is a workaround
    for buggy compilers and can be ignored.

    To populate the column with values, newRow() can be used. Use
    \l QFETCH() to fetch the data in the actual test.

    Example:
    \code
    void myTestFunction_data() {
        QTest::addColumn<int>("intval");
        QTest::addColumn<QString>("str");
        QTest::addColumn<double>("dbl");

        QTest::newRow("row1") << 1 << "hello" << 1.5;
    }
    \endcode

    To add custom types to the testdata, the type must be registered with
    QMetaType via \l Q_DECLARE_METATYPE().

    \bold {Note:} This macro can only be used in a test's data function
    that is invoked by the test framework.

    ### link to test-driven

    \sa newRow(), QFETCH(), QMetaType
*/

/*! \fn const char *QTest::currentTestFunction()

    Returns the name of the test function that is currently executed.

    Example:

    \code
    void MyTestClass::cleanup()
    {
        if (qstrcmp(currentTestFunction(), "myDatabaseTest") == 0) {
            // clean up all database connections
            closeAllDatabases();
        }
    }
    \endcode
*/
const char *QTest::currentTestFunction()
{
    return QTestResult::currentTestFunction();
}

/*! \fn const char *QTest::currentDataTag()

    Returns the name of the current test data. If the test doesn't
    have any assigned testdata, the function returns 0.

    \sa QTestTable
*/
const char *QTest::currentDataTag()
{
    return QTestResult::currentDataTag();
}

/*! \fn void QTest::currentTestFailed()

    Returns true if the current test function failed, otherwise false.
*/
bool QTest::currentTestFailed()
{
    return QTestResult::currentTestFailed();
}

/*! \fn void QTest::qSleep(int ms)

    Sleeps for \a ms milliseconds, blocking execution of the
    test. qSleep() will not do any event processing and leave your test
    unresponsive. Network communication might time out while
    sleeping. Use \l qWait() to do non-blocking sleeping.

    \bold {Note:} The qSleep() function calls either \c nanosleep() on
    unix or \c Sleep() on windows, so the accuracy of time spent in
    qSleep() depends on the operating system.

    Example:
    \code
    QTest::qSleep(250);
    \endcode

    \sa qWait()
*/
void QTest::qSleep(int ms)
{
#ifdef Q_OS_WIN32
    Sleep(uint(ms));
#else
    struct timespec ts = { 0, 0 };
    // a nanosecond is 1/1000 of a microsecond, a microsecond is 1/1000 of a millisecond
    ts.tv_nsec = ms * 1000 * 1000;
    nanosleep(&ts, NULL);
#endif
}

/*! \internal
 */
QObject *QTest::testObject()
{
    return currentTestObject;
}

/*! \internal
 */
bool QTest::compare_helper(bool success, const char *msg, const char *file, int line)
{
    return QTestResult::compare(success, msg, file, line);
}

/*! \internal
 */
bool QTest::compare_helper(bool success, const char *msg, char *val1, char *val2,
                    const char *actual, const char *expected, const char *file, int line)
{
    return QTestResult::compare(success, msg, val1, val2, actual, expected, file, line);
}

template <>
bool QTest::qCompare<float>(float const &t1, float const &t2, const char *actual, const char *expected,
                    const char *file, int line)
{
    return (qAbs(t1 - t2) < 0.00001f)
            ? compare_helper(true, "COMPARE()", file, line)
            : compare_helper(false, "Compared floats are not the same (fuzzy compare)",
                             toString(t1), toString(t2), actual, expected, file, line);
}

template <>
bool QTest::qCompare<double>(double const &t1, double const &t2, const char *actual, const char *expected,
                    const char *file, int line)
{
    return (qAbs(t1 - t2) < 0.000000000001)
            ? compare_helper(true, "COMPARE()", file, line)
            : compare_helper(false, "Compared doubles are not the same (fuzzy compare)",
                             toString(t1), toString(t2), actual, expected, file, line);
}

#define COMPARE_IMPL2(TYPE, FORMAT) \
template <> char *QTest::toString<TYPE >(const TYPE &t) \
{ \
    char *msg = new char[128]; \
    qt_snprintf(msg, 128, #FORMAT, t); \
    return msg; \
}

COMPARE_IMPL2(short, %hd)
COMPARE_IMPL2(ushort, %hu)
COMPARE_IMPL2(int, %d)
COMPARE_IMPL2(uint, %u)
COMPARE_IMPL2(long, %ld)
COMPARE_IMPL2(ulong, %lu)
COMPARE_IMPL2(qint64, %lld)
COMPARE_IMPL2(quint64, %llu)
COMPARE_IMPL2(bool, %d)
COMPARE_IMPL2(char, %c)
COMPARE_IMPL2(float, %f);
COMPARE_IMPL2(double, %lf);

/*! \internal
 */
char *QTest::toString(const char *str)
{
    if (!str)
        return 0;
    char *msg = new char[strlen(str) + 1];
    return qstrcpy(msg, str);
}

/*! \internal
 */
char *QTest::toString(const void *p)
{
    char *msg = new char[128];
    qt_snprintf(msg, 128, "%p", p);
    return msg;
}

/*! \internal
 */
bool QTest::compare_string_helper(const char *t1, const char *t2, const char *actual,
                                  const char *expected, const char *file, int line)
{
    return (qstrcmp(t1, t2) == 0)
            ? compare_helper(true, "COMPARE()", file, line)
            : compare_helper(false, "Compared strings are not the same",
                             toString(t1), toString(t2), actual, expected, file, line);
}
