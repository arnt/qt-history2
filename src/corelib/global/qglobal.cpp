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

#include "qplatformdefs.h"

#include <qstring.h>

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>
#include <errno.h>

#if defined(Q_CC_MSVC) && !defined(Q_OS_TEMP)
#include <crtdbg.h>
#endif

/*!
    \relates QApplication

    Returns the Qt version number as a string, for example, "2.3.0" or
    "3.0.5".

    The \c QT_VERSION define has the numeric value in the form:
    0xMMIIPP (M = major, I = minor, B = patch). For example, Qt
    3.0.5's \c QT_VERSION is 0x030005.
*/

const char *qVersion()
{
    return QT_VERSION_STR;
}

bool qSharedBuild()
{
#ifdef QT_SHARED
    return true;
#else
    return false;
#endif
}

/*****************************************************************************
  System detection routines
 *****************************************************************************/

#if !defined(Q_BYTE_ORDER) && defined(QT_BUILD_QMAKE)
// needed to bootstrap qmake
static const unsigned int qt_one = 1;
const int QSysInfo::ByteOrder = ((*((unsigned char *) &qt_one) == 0) ? BigEndian : LittleEndian);
#endif

#if !defined(QWS) && defined(Q_OS_MAC)

#include <private/qcore_mac_p.h>
#include "qnamespace.h"

// This function has descended from Apple Source Code (FSpLocationFromFullPath),
// but changes have been made. [Creates a minimal alias from the full pathname]
Q_CORE_EXPORT OSErr qt_mac_create_fsspec(const QString &file, FSSpec *spec)
{
    FSRef fref;
    QByteArray utfs = file.toUtf8();
    OSErr ret = FSPathMakeRef((const UInt8 *)utfs.data(), &fref, NULL);
    if(ret == noErr)
        ret = FSGetCatalogInfo(&fref, kFSCatInfoNone, NULL, NULL, spec, NULL);
    return ret;
}

Q_CORE_EXPORT void qt_mac_to_pascal_string(QString s, Str255 str, TextEncoding encoding=0, int len=-1)
{
    if(len == -1)
        len = s.length();
#if 0
    UnicodeMapping mapping;
    mapping.unicodeEncoding = CreateTextEncoding(kTextEncodingUnicodeDefault,
                                                 kTextEncodingDefaultVariant,
                                                 kUnicode16BitFormat);
    mapping.otherEncoding = (encoding ? encoding : );
    mapping.mappingVersion = kUnicodeUseLatestMapping;

    UnicodeToTextInfo info;
    OSStatus err = CreateUnicodeToTextInfo(&mapping, &info);
    if(err != noErr) {
        qDebug("Qt: internal: Unable to create pascal string '%s'::%d [%ld]",
               s.left(len).latin1(), (int)encoding, err);
        return;
    }
    const int unilen = len * 2;
    const UniChar *unibuf = (UniChar *)s.unicode();
    ConvertFromUnicodeToPString(info, unilen, unibuf, str);
    DisposeUnicodeToTextInfo(&info);
#else
    Q_UNUSED(encoding);
    CFStringGetPascalString(QCFString(s), str, 256, CFStringGetSystemEncoding());
#endif
}

Q_CORE_EXPORT QString qt_mac_from_pascal_string(const Str255 pstr) {
    return QCFString::toQString(CFStringCreateWithPascalString(0, pstr, CFStringGetSystemEncoding()));
}

static QSysInfo::MacVersion macVersion()
{
    long gestalt_version;
    if (Gestalt(gestaltSystemVersion, &gestalt_version) == noErr) {
        if (gestalt_version >= 0x1040 && gestalt_version < 0x1050)
            return QSysInfo::MV_10_4;
        else if (gestalt_version >= 0x1030 && gestalt_version < 0x1040)
            return QSysInfo::MV_10_3;
        else if (gestalt_version >= 0x1020 && gestalt_version < 0x1030)
            return QSysInfo::MV_10_2;
        else if (gestalt_version >= 0x1010 && gestalt_version < 0x1020)
            return QSysInfo::MV_10_1;
        else if (gestalt_version >= 0x1000 && gestalt_version < 0x1010)
            return QSysInfo::MV_10_0;
    }
    return QSysInfo::MV_Unknown;
}
const QSysInfo::MacVersion QSysInfo::MacintoshVersion = macVersion();
#elif defined(Q_OS_WIN32) || defined(Q_OS_CYGWIN) || defined(Q_OS_TEMP)

#include "qt_windows.h"

static QSysInfo::WinVersion winVersion()
{
#ifndef VER_PLATFORM_WIN32s
#define VER_PLATFORM_WIN32s            0
#endif
#ifndef VER_PLATFORM_WIN32_WINDOWS
#define VER_PLATFORM_WIN32_WINDOWS  1
#endif
#ifndef VER_PLATFORM_WIN32_NT
#define VER_PLATFORM_WIN32_NT            2
#endif
#ifndef VER_PLATFORM_WIN32_CE
#define VER_PLATFORM_WIN32_CE            3
#endif

    static QSysInfo::WinVersion winver = QSysInfo::WV_NT;
#ifndef Q_OS_TEMP
    OSVERSIONINFOA osver;
    osver.dwOSVersionInfoSize = sizeof(osver);
    GetVersionExA(&osver);
#else
    DWORD qt_cever = 0;
    OSVERSIONINFOW osver;
    osver.dwOSVersionInfoSize = sizeof(osver);
    GetVersionEx(&osver);
    qt_cever = osver.dwMajorVersion * 100;
    qt_cever += osver.dwMinorVersion * 10;
#endif
    switch (osver.dwPlatformId) {
    case VER_PLATFORM_WIN32s:
        winver = QSysInfo::WV_32s;
        break;
    case VER_PLATFORM_WIN32_WINDOWS:
        // We treat Windows Me (minor 90) the same as Windows 98
        if (osver.dwMinorVersion == 90)
            winver = QSysInfo::WV_Me;
        else if (osver.dwMinorVersion == 10)
            winver = QSysInfo::WV_98;
        else
            winver = QSysInfo::WV_95;
        break;
#ifdef Q_OS_TEMP
    case VER_PLATFORM_WIN32_CE:
#ifdef Q_OS_TEMP
        if (qt_cever >= 400)
            winver = QSysInfo::WV_CENET;
        else
#endif
            winver = QSysInfo::WV_CE;
        break;
#endif
    default: // VER_PLATFORM_WIN32_NT
        if (osver.dwMajorVersion < 5) {
            winver = QSysInfo::WV_NT;
        } else if (osver.dwMinorVersion == 0) {
            winver = QSysInfo::WV_2000;
        } else if (osver.dwMinorVersion == 1) {
            winver = QSysInfo::WV_XP;
        } else if (osver.dwMinorVersion == 2) {
            winver = QSysInfo::WV_2003;
        } else {
            qWarning("Untested Windows version detected!");
            winver = QSysInfo::WV_NT_based;
        }
    }

    return winver;
}

const QSysInfo::WinVersion QSysInfo::WindowsVersion = winVersion();

#endif


/*!
    \macro void Q_ASSERT(bool test)

    \relates QApplication

    Prints a warning message containing the source code file name and
    line number if \a test is false.

    This is really a macro defined in \c qglobal.h.

    Q_ASSERT is useful for testing pre- and post-conditions during
    development. It does nothing if \c QT_NO_DEBUG was defined during
    compilation.

    Example:
    \code
        //
        // File: div.cpp
        //

        #include <qglobal.h>

        int divide(int a, int b)
        {
            Q_ASSERT(b != 0);                        // this is line 9
            return a/b;
        }
    \endcode

    If \c b is zero, the Q_ASSERT statement will output the following
    message using the qFatal() function:
    \code
    ASSERT: "b == 0" in file div.cpp, line 9
    \endcode

    \sa Q_ASSERT_X(), qFatal(), \link debug.html Debugging\endlink
*/

/*!
    \macro void Q_ASSERT_X(bool test, const char *where, const char *what)

    \relates QApplication

    Prints the message \a what together with the location \a where,
    the source file name and line number if \a test is false.

    This is really a macro defined in \c qglobal.h.

    Q_ASSERT_X is useful for testing pre- and post-conditions during
    development. It does nothing if \c QT_NO_DEBUG was defined during
    compilation.

    Example:
    \code
        //
        // File: div.cpp
        //

        #include <qglobal.h>

        int divide(int a, int b)
        {
            Q_ASSERT_X(b != 0, "divide", "division by zero");   // this is line 9
            return a/b;
        }
    \endcode

    If \c b is zero, the Q_ASSERT_X statement will output the following
    message using the qFatal() function:
    \code
    ASSERT failure in divide: "division by zero", file div.cpp, line 9
    \endcode

    \sa Q_ASSERT(), qFatal(), \link debug.html Debugging\endlink
*/

/*!
    \macro void Q_CHECK_PTR(void *p)

    \relates QApplication

    If \a p is 0, prints a warning message containing the source code file
    name and line number, saying that the program ran out of memory.

    This is really a macro defined in \c qglobal.h.

    Q_CHECK_PTR does nothing if \c QT_NO_DEBUG was defined during
    compilation.

    Example:
    \code
        int *a;

        Q_CHECK_PTR(a = new int[80]);  // WRONG!

        a = new (nothrow) int[80];       // Right
        Q_CHECK_PTR(a);
    \endcode

    \sa qWarning(), \link debug.html Debugging\endlink
*/


/*
  The Q_CHECK_PTR macro calls this function if an allocation check
  fails.
*/
void qt_check_pointer(const char *n, int l)
{
    qWarning("In file %s, line %d: Out of memory", n, l);
}

/*
  The Q_ASSERT macro calls this this function when the test fails.
*/
void qt_assert(const char *assertion, const char *file, int line)
{
    qFatal("ASSERT: \"%s\" in file %s, line %d", assertion, file, line);
}

/*
  The Q_ASSERT_X macro calls this this function when the test fails.
*/
void qt_assert_x(const char *where, const char *what, const char *file, int line)
{
    qFatal("ASSERT failure in %s: \"%s\", file %s, line %d", where, what, file, line);
}


/*
    Dijkstra's bisection algorithm to find the square root of an integer.
    Deliberately not exported as part of the Qt API, but used in both
    qsimplerichtext.cpp and qgfxraster_qws.cpp
*/
Q_CORE_EXPORT unsigned int qt_int_sqrt(unsigned int n)
{
    // n must be in the range 0...UINT_MAX/2-1
    if (n >= (UINT_MAX>>2)) {
        unsigned int r = 2 * qt_int_sqrt(n / 4);
        unsigned int r2 = r + 1;
        return (n >= r2 * r2) ? r2 : r;
    }
    uint h, p= 0, q= 1, r= n;
    while (q <= n)
        q <<= 2;
    while (q != 1) {
        q >>= 2;
        h= p + q;
        p >>= 1;
        if (r >= h) {
            p += q;
            r -= h;
        }
    }
    return p;
}

#if defined(qMemCopy)
#  undef qMemCopy
#endif
#if defined(qMemSet)
#  undef qMemSet
#endif

void *qMalloc(size_t size) { return ::malloc(size); }
void qFree(void *ptr) { ::free(ptr); }
void *qRealloc(void *ptr, size_t size) { return ::realloc(ptr, size); }
void *qMemCopy(void *dest, const void *src, size_t n) { return memcpy(dest, src, n); }
void *qMemSet(void *dest, int c, size_t n) { return memset(dest, c, n); }


static QtMsgHandler handler = 0;                // pointer to debug handler
static const int QT_BUFFER_LENGTH = 8192;       // internal buffer length

#ifdef Q_CC_MWERKS
#include <CoreServices/CoreServices.h>
extern bool qt_is_gui_used;
static void mac_default_handler(const char *msg)
{
    if (qt_is_gui_used) {
        Str255 pmsg;
        qt_mac_to_pascal_string(msg, pmsg);
        DebugStr(pmsg);
    } else {
        fprintf(stderr, msg);
    }
}
#endif // Q_CC_MWERKS


QString qt_error_string(int errorCode)
{
    const char *s = 0;
    QString ret;
    if (errorCode == -1) {
#if defined(Q_OS_WIN32)
        errorCode = GetLastError();
#else
        errorCode = errno;
#endif
    }
    switch (errorCode) {
    case 0:
        break;
    case EACCES:
        s = QT_TRANSLATE_NOOP("QIODevice", "Permission denied");
        break;
    case EMFILE:
        s = QT_TRANSLATE_NOOP("QIODevice", "Too many open files");
        break;
    case ENOENT:
        s = QT_TRANSLATE_NOOP("QIODevice", "No such file or directory");
        break;
    case ENOSPC:
        s = QT_TRANSLATE_NOOP("QIODevice", "No space left on device");
        break;
    default: {
#ifdef Q_OS_WIN
        QT_WA({
            unsigned short *string = 0;
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
                          NULL,
                          errorCode,
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                          (LPTSTR)&string,
                          0,
                          NULL);
            ret = QString::fromUtf16(string);
            LocalFree((HLOCAL)string);
        }, {
            char *string = 0;
            FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
                           NULL,
                           errorCode,
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                           (LPSTR)&string,
                           0,
                           NULL);
            ret = QString::fromLocal8Bit(string);
            LocalFree((HLOCAL)string);
        });
#else
        ret = QString::fromLocal8Bit(strerror(errorCode));
#endif
	break; }
    }
    if (s)
        // ######## this breaks moc build currently
//         ret = QCoreApplication::translate("QIODevice", s);
        ret = QString::fromLatin1(s);
    return ret.trimmed();
}

/*!
    \relates QApplication

    Installs a Qt message handler \a h. Returns a pointer to the
    message handler previously defined.

    The message handler is a function that prints out debug messages,
    warnings and fatal error messages. The Qt library (debug version)
    contains hundreds of warning messages that are printed when
    internal errors (usually invalid function arguments) occur. If you
    implement your own message handler, you get total control of these
    messages.

    The default message handler prints the message to the standard
    output under X11 or to the debugger under Windows. If it is a
    fatal message, the application aborts immediately.

    Only one message handler can be defined, since this is usually
    done on an application-wide basis to control debug output.

    To restore the message handler, call \c qInstallMsgHandler(0).

    Example:
    \code
        #include <qapplication.h>
        #include <stdio.h>
        #include <stdlib.h>

        void myMessageOutput(QtMsgType type, const char *msg)
        {
            switch (type) {
                case QtDebugMsg:
                    fprintf(stderr, "Debug: %s\n", msg);
                    break;
                case QtWarningMsg:
                    fprintf(stderr, "Warning: %s\n", msg);
                    break;
                case QtCriticalMsg:
                    fprintf(stderr, "Critical: %s\n", msg);
                    break;
                case QtFatalMsg:
                    fprintf(stderr, "Fatal: %s\n", msg);
                    abort(); // deliberately core dump
            }
        }

        int main(int argc, char **argv)
        {
            qInstallMsgHandler(myMessageOutput);
            QApplication a(argc, argv);
            ...
            return a.exec();
        }
    \endcode

    \sa qDebug(), qWarning(), qFatal(), \link debug.html Debugging\endlink
*/
QtMsgHandler qInstallMsgHandler(QtMsgHandler h)
{
    QtMsgHandler old = handler;
    handler = h;
    return old;
}


void qt_message_output(QtMsgType msgType, const char *buf)
{
    if (handler) {
        (*handler)(msgType, buf);
    } else {
#if defined(Q_CC_MWERKS)
        mac_default_handler(buf);
#elif defined(Q_OS_TEMP)
        QString fstr(buf);
        OutputDebugString((fstr + "\n").utf16());
#else
        fprintf(stderr, "%s\n", buf);
#endif
    }

    if (msgType == QtFatalMsg
        || (msgType == QtWarningMsg && (qgetenv("QT_FATAL_WARNINGS") != 0)) ) {

#if defined(Q_CC_MSVC) && defined(QT_DEBUG) && defined(_DEBUG) && defined(_CRT_ERROR)
        if (_CrtDbgReport(_CRT_ERROR, __FILE__, __LINE__, QT_VERSION_STR, buf) == 1)
            _CrtDbgBreak();
#endif

#if defined(Q_OS_UNIX) && defined(QT_DEBUG)
        abort(); // trap; generates core dump
#else
        exit(1); // goodbye cruel world
#endif
    }
}

#undef qDebug
/*!
    \relates QApplication

    Calls the message handler with the debug message \a msg. If no
    message handler has been installed, the message is printed to
    stderr. Under Windows, the message is sent to the debugger. This
    function does nothing if \c QT_NO_DEBUG_OUTPUT was defined during
    compilation.

    If you pass the function a format string and a list of arguments,
    it works in similar way to the C printf() function.

    Example:
    \code
        qDebug("my window handle = %x", myWidget->id());
    \endcode

    A more convenient syntax is also available:
    \code
        qDebug() << "Brush:" << myQBrush << "Other value:" << i;
    \endcode
    This syntax automatically puts a single space between each item,
    and outputs a newline at the end. It supports many C++ and Qt
    types.

    \warning The internal buffer is limited to 8192 bytes, including
    the '\0'-terminator.

    \warning Passing (const char *)0 as argument to qDebug might lead
    to crashes on certain platforms due to the platform's printf() implementation.

    \sa qWarning(), qCritical(), qFatal(), qInstallMsgHandler(),
        \link debug.html Debugging\endlink
*/
void qDebug(const char *msg, ...)
{
    char buf[QT_BUFFER_LENGTH];
    buf[QT_BUFFER_LENGTH - 1] = '\0';
    va_list ap;
    va_start(ap, msg);                        // use variable arg list
    qvsnprintf(buf, QT_BUFFER_LENGTH - 1, msg, ap);
    va_end(ap);

    qt_message_output(QtDebugMsg, buf);
}

#undef qWarning
/*!
    \relates QApplication

    Calls the message handler with the warning message \a msg. If no
    message handler has been installed, the message is printed to
    stderr. Under Windows, the message is sent to the debugger. This
    function does nothing if \c QT_NO_WARNING_OUTPUT was defined
    during compilation; it exits if the environment variable \c
    QT_FATAL_WARNINGS is defined.

    This function takes a format string and a list of arguments,
    similar to the C printf() function.

    Example:
    \code
        void f(int c)
        {
            if (c > 200)
                qWarning("f: bad argument, c == %d", c);
        }
    \endcode

    \warning The internal buffer is limited to 8192 bytes, including
    the '\0'-terminator.

    \warning Passing (const char *)0 as argument to qWarning might lead
    to crashes on certain platforms due to the platforms printf implementation.

    \sa qDebug(), qCritical(), qFatal(), qInstallMsgHandler(),
        \link debug.html Debugging\endlink
*/
void qWarning(const char *msg, ...)
{
    char buf[QT_BUFFER_LENGTH];
    buf[QT_BUFFER_LENGTH - 1] = '\0';
    va_list ap;
    va_start(ap, msg); // use variable arg list
    qvsnprintf(buf, QT_BUFFER_LENGTH - 1, msg, ap);
    va_end(ap);

    qt_message_output(QtWarningMsg, buf);
}

/*!
    \relates QApplication

    Calls the message handler with the critical message \a msg. If no
    message handler has been installed, the message is printed to
    stderr. Under Windows, the message is sent to the debugger.

    This function takes a format string and a list of arguments, similar
    to the C printf() function.

    Example:
    \code
        void load(const QString &filename)
        {
            QFile file(filename);
            if (!file.exists())
                qCritical("file '%s' does not exist!", filename.local8bit());
        }
    \endcode

    \warning The internal buffer is limited to 8192 bytes, including
    the '\0'-terminator.

    \warning Passing (const char *)0 as argument to qCritical might
    lead to crashes on certain platforms due to the platforms printf
    implementation.

    \sa qDebug(), qWarning(), qFatal(), qInstallMsgHandler(),
        \link debug.html Debugging\endlink
*/
void qCritical(const char *msg, ...)
{
    char buf[QT_BUFFER_LENGTH];
    buf[QT_BUFFER_LENGTH - 1] = '\0';
    va_list ap;
    va_start(ap, msg); // use variable arg list
    qvsnprintf(buf, QT_BUFFER_LENGTH - 1, msg, ap);
    va_end(ap);

    qt_message_output(QtCriticalMsg, buf);
}
#ifdef QT3_SUPPORT
void qSystemWarning(const char *msg, int code)
   { qCritical("%s (%s)", msg, qt_error_string(code).toLocal8Bit().constData()); }
#endif // QT3_SUPPORT

void qErrnoWarning(const char *msg, ...)
{
    char buf[QT_BUFFER_LENGTH];
    buf[QT_BUFFER_LENGTH - 1] = '\0';
    va_list ap;
    va_start(ap, msg);
    qvsnprintf(buf, QT_BUFFER_LENGTH - 1, msg, ap);
    va_end(ap);

    qCritical("%s (%s)", buf, qt_error_string(-1).toLocal8Bit().constData());
}

void qErrnoWarning(int code, const char *msg, ...)
{
    char buf[QT_BUFFER_LENGTH];
    buf[QT_BUFFER_LENGTH - 1] = '\0';
    va_list ap;
    va_start(ap, msg);
    qvsnprintf(buf, QT_BUFFER_LENGTH - 1, msg, ap);
    va_end(ap);

    qCritical("%s (%s)", buf, qt_error_string(code).toLocal8Bit().constData());
}

/*!
    \relates QApplication

    Calls the message handler with the fatal message \a msg. If no
    message handler has been installed, the message is printed to
    stderr. Under Windows, the message is sent to the debugger.

    For a release library this function will exit the application
    with return value 1. For the debug version this function will
    abort on Unix systems to create a core dump, and report a
    _CRT_ERROR on Windows allowing to connect a debugger to the
    application.

    This function takes a format string and a list of arguments,
    similar to the C printf() function.

    Example:
    \code
        int divide(int a, int b)
        {
            if (b == 0)                                // program error
                qFatal("divide: cannot divide by zero");
            return a / b;
        }
    \endcode

    \warning The internal buffer is limited to 8192 bytes, including
    the '\0'-terminator.

    \warning Passing (const char *)0 as argument to qFatal might lead
    to crashes on certain platforms due to the platforms printf implementation.

    \sa qDebug(), qCritical(), qWarning(), qInstallMsgHandler(),
        \link debug.html Debugging\endlink
*/
void qFatal(const char *msg, ...)
{
    char buf[QT_BUFFER_LENGTH];
    buf[QT_BUFFER_LENGTH - 1] = '\0';
    va_list ap;
    va_start(ap, msg); // use variable arg list
    qvsnprintf(buf, QT_BUFFER_LENGTH - 1, msg, ap);
    va_end(ap);

    qt_message_output(QtFatalMsg, buf);
}

// getenv is declared as deprecated in VS2005. This function
// makes use of the new secure getenv function.
char *qgetenv(const char *varName)
{
#if defined(_MSC_VER) && _MSC_VER >= 1400
	size_t requiredSize;
	char *buffer = 0;
	getenv_s(&requiredSize, buffer, 0, varName);
    if (requiredSize == 0)
        return 0;
	buffer = (char*)qMalloc(requiredSize*sizeof(char));
	if (!buffer)
		return 0;
	getenv_s(&requiredSize, buffer, requiredSize, varName);
	return buffer;
#else
	return getenv(varName);
#endif
}

#ifdef QT3_SUPPORT
#include <qlibraryinfo.h>
static const char *qInstallLocation(QLibraryInfo::LibraryLocation loc)
{
    static QByteArray ret; //yuck
    ret = QLibraryInfo::location(loc).toLatin1();
    return ret.constData();
}
const char *qInstallPath()
{
    return qInstallLocation(QLibraryInfo::PrefixPath);
}
const char *qInstallPathDocs()
{
    return qInstallLocation(QLibraryInfo::DocumentationPath);
}
const char *qInstallPathHeaders()
{
    return qInstallLocation(QLibraryInfo::HeadersPath);
}
const char *qInstallPathLibs()
{
    return qInstallLocation(QLibraryInfo::LibrariesPath);
}
const char *qInstallPathBins()
{
    return qInstallLocation(QLibraryInfo::BinariesPath);
}
const char *qInstallPathPlugins()
{
    return qInstallLocation(QLibraryInfo::PluginsPath);
}
const char *qInstallPathData()
{
    return qInstallLocation(QLibraryInfo::DataPath);
}
const char *qInstallPathTranslations()
{
    return qInstallLocation(QLibraryInfo::TranslationsPath);
}
const char *qInstallPathSysconf()
{
    return qInstallLocation(QLibraryInfo::SettingsPath);
}
#endif
