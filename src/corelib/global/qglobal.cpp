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
    \class QFlag
    \brief The QFlag class is a helper data type for QFlags.

    It is equivalent to a plain \c int, except with respect to
    function overloading and type conversions. You should never need
    to use it in your applications.

    \sa QFlags
*/

/*!
    \fn QFlag::QFlag(int value)

    Constructs a QFlag object that stores the given \a value.
*/

/*!
    \fn QFlag::operator int() const

    Returns the value stored by the QFlag object.
*/

/*!
    \class QFlags
    \brief The QFlags class provides a type-safe way of storing
    OR-combinations of enum values.

    \mainclass
    \ingroup tools

    The QFlags<Enum> class is a template class, where Enum is an enum
    type. QFlags is used throughout Qt for storing combinations of
    enum values.

    The traditional C++ approach for storing OR-combinations of enum
    values is to use a \c int or \c uint variable. The inconvenient
    with that approach is that there's no type checking at all; any
    enum value can be OR'd with any other enum value and passed on to
    a function that takes a \c int or \c uint.

    Qt uses QFlags to provide type safety. For example, the
    Qt::Alignment type is simply a typedef for
    QFlags<Qt::AlignmentFlag>. QLabel::setAlignment() takes a
    Qt::Alignment parameter, which means that any combination of
    Qt::AlignmentFlag values is legal:

    \code
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    \endcode

    If you try to pass a value from another enum, the compiler will
    report an error.

    If you want to use QFlags for your own enum types, you must use
    the Q_DECLARE_FLAGS() and Q_DECLARE_OPERATORS_FOR_FLAGS().
    For example:

    \code
        class MyClass
        {
        public:
            enum Option {
                NoOptions = 0x0,
                ShowTabs = 0x1,
                ShowAll = 0x2,
                SqueezeBlank = 0x4
            };
            Q_DECLARE_FLAGS(Options, Option)
            ...
        };

        Q_DECLARE_OPERATORS_FOR_FLAGS(MyClass::Options)
    \endcode

    You can then use the \c MyClass::Options type to store
    combinations of \c MyClass::Option values.

    A sensible naming convension for enum types and associated QFlags
    types is to give a singular name to the enum type (e.g., \c
    Option) and a plural name to the QFlags type (e.g., \c Options).
    When a singular name is desired for the QFlags type (e.g., \c
    Alignment), you can use \c Flag as the suffix for the enum type
    (e.g., \c AlignmentFlag).

    \sa QFlag
*/

/*!
    \typedef QFlags::enum_type

    Typedef for the Enum template type.
*/

/*!
    \fn QFlags::QFlags(const QFlags &other)

    Constructs a copy of \a other.
*/

/*!
    \fn QFlags::QFlags(Enum flag)

    Constructs a QFlags object storing the given \a flag.
*/

/*!
    \fn QFlags::QFlags(Zero zero)

    Constructs a QFlags object with no flags set. \a zero must be a
    literal 0 value.
*/

/*!
    \fn QFlags::QFlags(QFlag value)

    Constructs a QFlags object initialized with the given integer \a
    value.

    The QFlag type is a helper type. By using it here instead of \c
    int, we effectively ensure that arbitrary enum values cannot be
    cast to a QFlags, whereas untyped enum values (i.e., \c int
    values) can.
*/

/*!
    \fn QFlags &QFlags::operator=(const QFlags &other)

    Assigns \a other to this object and returns a reference to this
    object.
*/

/*!
    \fn QFlags &QFlags::operator&=(int mask)

    Performs a bitwise AND operation with \a mask and stores the
    result in this QFlags object. Returns a reference to this object.

    \sa operator&(), operator|=(), operator^=()
*/

/*!
    \fn QFlags &QFlags::operator&=(uint mask)

    \overload
*/

/*!
    \fn QFlags &QFlags::operator|=(QFlags other)

    Performs a bitwise OR operation with \a other and stores the
    result in this QFlags object. Returns a reference to this object.

    \sa operator|(), operator&=(), operator^=()
*/

/*!
    \fn QFlags &QFlags::operator|=(Enum other)

    \overload
*/

/*!
    \fn QFlags &QFlags::operator^=(QFlags other)

    Performs a bitwise XOR operation with \a other and stores the
    result in this QFlags object. Returns a reference to this object.

    \sa operator^(), operator&=(), operator|=()
*/

/*!
    \fn QFlags &QFlags::operator^=(Enum other)

    \overload
*/

/*!
    \fn QFlags::operator int() const

    Returns the value stored in the QFlags object as an integer.
*/

/*!
    \fn QFlags QFlags::operator|(QFlags other) const

    Returns a QFlags object containing the result of the bitwise OR
    operation on this object and \a other.

    \sa operator|=(), operator^(), operator&(), operator~()
*/

/*!
    \fn QFlags QFlags::operator|(Enum other) const

    \overload
*/

/*!
    \fn QFlags QFlags::operator^(QFlags other) const

    Returns a QFlags object containing the result of the bitwise XOR
    operation on this object and \a other.

    \sa operator^=(), operator&(), operator|(), operator~()
*/

/*!
    \fn QFlags QFlags::operator^(Enum other) const

    \overload
*/

/*!
    \fn QFlags QFlags::operator&(int mask) const

    Returns a QFlags object containing the result of the bitwise AND
    operation on this object and \a mask.

    \sa operator&=(), operator|(), operator^(), operator~()
*/

/*!
    \fn QFlags QFlags::operator&(uint mask) const

    \overload
*/

/*!
    \fn QFlags QFlags::operator&(Enum mask) const

    \overload
*/

/*!
    \fn QFlags QFlags::operator~() const

    Returns a QFlags object that contains the bitwise negation of
    this objec;t.

    \sa operator&(), operator|(), operator^()
*/

/*!
    \fn bool QFlags::operator!() const

    Returns true if no flag is set (i.e., if the value stored by the
    QFlags object is 0); otherwise returns false.
*/

/*!
    \macro Q_DECLARE_FLAGS(Flags, Enum)
    \relates QFlags

    The Q_DECLARE_FLAGS() macro expands to

    \code
        typedef QFlags<Enum> Flags;
    \endcode

    \a Enum is the name of an existing enum type, whereas \a Flags is
    the name of the QFlags<\e{Enum}> typedef.

    See the QFlags documentation for details.

    \sa Q_DECLARE_OPERATORS_FOR_FLAGS()
*/

/*!
    \macro Q_DECLARE_OPERATORS_FOR_FLAGS(Flags)
    \relates QFlags

    The Q_DECLARE_OPERATORS_FOR_FLAGS() macro declares global \c
    operator|() functions for \a Flags, which is of type QFlags<T>.

    See the QFlags documentation for details.

    \sa Q_DECLARE_FLAGS()
*/

/*!
    \headerfile <QtGlobal>
    \title Global Qt Declarations

    \brief The <QtGlobal> header file provides basic declarations and is included by all other Qt headers.

    \sa <QtAlgorithms>
*/

/*!
    \typedef qreal
    \relates <QtGlobal>

    Typedef for \c double.
*/

/*! \typedef uchar
    \relates <QtGlobal>

    Convenience typedef for \c{unsigned char}.
*/

/*! \typedef ushort
    \relates <QtGlobal>

    Convenience typedef for \c{unsigned short}.
*/

/*! \typedef uint
    \relates <QtGlobal>

    Convenience typedef for \c{unsigned int}.
*/

/*! \typedef ulong
    \relates <QtGlobal>

    Convenience typedef for \c{unsigned long}.
*/

/*! \typedef qint8
    \relates <QtGlobal>

    Typedef for \c{signed char}. This type is guaranteed to be 8-bit
    on all platforms supported by Qt.
*/

/*! \typedef quint8
    \relates <QtGlobal>

    Typedef for \c{unsigned signed char}. This type is guaranteed to
    be 8-bit on all platforms supported by Qt.
*/

/*! \typedef qint16
    \relates <QtGlobal>

    Typedef for \c{signed short}. This type is guaranteed to be
    16-bit on all platforms supported by Qt.
*/

/*! \typedef quint16
    \relates <QtGlobal>

    Typedef for \c{unsigned signed short}. This type is guaranteed to
    be 16-bit on all platforms supported by Qt.
*/

/*! \typedef qint32
    \relates <QtGlobal>

    Typedef for \c{signed int}. This type is guaranteed to be 32-bit
    on all platforms supported by Qt.
*/

/*! \typedef quint32
    \relates <QtGlobal>

    Typedef for \c{unsigned signed int}. This type is guaranteed to
    be 32-bit on all platforms supported by Qt.
*/

/*! \typedef qint64
    \relates <QtGlobal>

    Typedef for \c{long long int} (\c __int64 on Windows). This type
    is guaranteed to be 64-bit on all platforms supported by Qt.

    Literals of that type can be created using the Q_INT64_C() macro:

    \code
        qint64 value = Q_INT64_C(932838457459459);
    \endcode

    \sa Q_INT64_C(), quint64
*/

/*! \typedef quint64
    \relates <QtGlobal>

    Typedef for \c{unsigned long long int} (\c{unsigned __int64} on
    Windows). This type is guaranteed to be 64-bit on all platforms
    supported by Qt.

    Literals of that type can be created using the Q_UINT64_C()
    macro:

    \code
        quint64 value = Q_UINT64_C(932838457459459);
    \endcode

    \sa Q_UINT64_C(), qint64
*/

/*! \macro qint64 Q_INT64_C(literal)
    \relates <QtGlobal>

    Wraps the signed 64-bit integer \a literal in a
    platform-independent way. For example:

    \code
        qint64 value = Q_INT64_C(932838457459459);
    \endcode

    \sa qint64, Q_UINT64_C()
*/

/*! \macro quint64 Q_UINT64_C(literal)
    \relates <QtGlobal>

    Wraps the unsigned 64-bit integer \a literal in a
    platform-independent way. For example:

    \code
        quint64 value = Q_UINT64_C(932838457459459);
    \endcode

    \sa quint64, Q_INT64_C()
*/

/*! \typedef qlonglong
    \relates <QtGlobal>

    Typedef for \c{long long int} (\c __int64 on Windows). This is
    the same as \l qint64.

    \sa Q_INT64_C(), qulonglong
*/

/*! \typedef qulonglong
    \relates <QtGlobal>

    Typedef for \c{unsigned long long int} (\c{unsigned __int64} on
    Windows). This is the same as \l quint64.

    \sa Q_UINT64_C(), qlonglong
*/

/*! \fn const T &qAbs(const T &value)
    \relates <QtGlobal>

    Returns the absolute value of \a value.
*/

/*! \fn int qRound(double value)
    \relates <QtGlobal>

    Rounds \a value up to the nearest integer.
*/

/*! \fn qint64 qRound64(double value)
    \relates <QtGlobal>

    Rounds \a value up to the nearest 64-bit integer.
*/

/*! \fn const T &qMin(const T &value1, const T &value2)
    \relates <QtGlobal>

    Returns the minimum of \a value1 and \a value2.

    \sa qMax(), qBound()
*/

/*! \fn const T &qMax(const T &value1, const T &value2)
    \relates <QtGlobal>

    Returns the maximum of \a value1 and \a value2.

    \sa qMin(), qBound()
*/

/*! \fn const T &qBound(const T &min, const T &value, const T &max)
    \relates <QtGlobal>

    Returns \a value bounded by \a min and \a max. This is equivalent
    to qMax(\a min, qMin(\a value, \a max)).

    \sa qMin(), qMax()
*/

/*!
    \typedef Q_INT8
    \relates <QtGlobal>
    \compat

    Use \l qint8 instead.
*/

/*!
    \typedef Q_UINT8
    \relates <QtGlobal>
    \compat

    Use \l quint8 instead.
*/

/*!
    \typedef Q_INT16
    \relates <QtGlobal>
    \compat

    Use \l qint16 instead.
*/

/*!
    \typedef Q_UINT16
    \relates <QtGlobal>
    \compat

    Use \l quint16 instead.
*/

/*!
    \typedef Q_INT32
    \relates <QtGlobal>
    \compat

    Use \l qint32 instead.
*/

/*!
    \typedef Q_UINT32
    \relates <QtGlobal>
    \compat

    Use \l quint32 instead.
*/

/*!
    \typedef Q_INT64
    \relates <QtGlobal>
    \compat

    Use \l qint64 instead.
*/

/*!
    \typedef Q_UINT64
    \relates <QtGlobal>
    \compat

    Use \l quint64 instead.
*/

/*!
    \typedef Q_LLONG
    \relates <QtGlobal>
    \compat

    Use \l qint64 instead.
*/

/*!
    \typedef Q_ULLONG
    \relates <QtGlobal>
    \compat

    Use \l quint64 instead.
*/

/*!
    \typedef Q_LONG
    \relates <QtGlobal>
    \compat

    Use \c{void *} instead.
*/

/*!
    \typedef Q_ULONG
    \relates <QtGlobal>
    \compat

    Use \c{void *} instead.
*/

/*! \fn bool qSysInfo(int *wordSize, bool *bigEndian)
    \relates <QtGlobal>

    Use QSysInfo::WordSize and QSysInfo::ByteOrder instead.
*/

/*!
    \fn bool qt_winUnicode()
    \relates <QtGlobal>

    Use !(QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based) instead.

    \sa QSysInfo
*/

/*!
    \fn int qWinVersion()
    \relates <QtGlobal>

    Use QSysInfo::WindowsVersion instead.

    \sa QSysInfo
*/

/*!
    \fn int qMacVersion()
    \relates <QtGlobal>

    Use QSysInfo::MacintoshVersion instead.

    \sa QSysInfo
*/

/*!
    \relates <QtGlobal>

    Returns the Qt version number as a string, for example, "4.0.1" or
    "4.1.3".

    The \c QT_VERSION define has the numeric value in the form:
    0xMMNNPP (MM = major, NN = minor, PP = patch). For example, Qt
    4.0.1's \c QT_VERSION is 0x040001.
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

/*!
    \class QSysInfo
    \brief The QSysInfo class provides information about the system.

    \list
    \o \l WordSize specifies the size of a pointer for the platform
       on which the application is compiled.
    \o \l ByteOrder specifies whether the platform is big-endian or
       little-endian.
    \o \l WindowsVersion specifies the version of the Windows operating
       system on which the application is run (Windows only)
    \o \l MacintoshVersion specifies the version of the Macintosh
       operating system on which the application is run (Mac only).
    \endlist

    Some constants are defined only on certain platforms. You can use
    the preprocessor symbols \c Q_WS_WIN and \c Q_WS_MAC to test that
    the application is compiled under Windows or Mac.

    \sa QLibraryInfo
*/

/*!
    \variable QSysInfo::WordSize
    \brief the size in bits of a pointer for the platform on which
           the application is compiled (32 or 64)
*/

/*!
    \variable QSysInfo::WindowsVersion
    \brief the version of the Windows operating system on which the
           application is run (Windows only)
*/

/*!
    \variable QSysInfo::MacintoshVersion
    \brief the version of the Macintosh operating system on which
           the application is run (Mac only).
*/

/*!
    \enum QSysInfo::Endian

    \value BigEndian  Big-endian byte order (also called Network byte order)
    \value LittleEndian  Little-endian byte order
    \value ByteOrder  Equals BigEndian or LittleEndian, depending on
                      the platform's byte order.
*/

/*!
    \enum QSysInfo::WinVersion

    This enum provides symbolic names for the various versions of the
    Windows operating system. On Windows, the
    QSysInfo::WindowsVersion variable gives the version of the system
    on which the application is run.

    MS-DOS-based versions:

    \value WV_32s   Windows 3.1 wth Win 32s
    \value WV_95    Windows 95
    \value WV_98    Windows 98
    \value WV_Me    Windows Me

    NT-based versions:

    \value WV_NT    Windows NT
    \value WV_2000  Windows 2000
    \value WV_XP    Windows XP
    \value WV_2003  Windows XP 2003

    CE-based versions:

    \value WV_CE    Windows CE
    \value WV_CENET Windows CE .NET

    The following masks can be used for testing whether a Windows
    version is MS-DOS-based, NT-based, or CE-based:

    \value WV_DOS_based MS-DOS-based version of Windows
    \value WV_NT_based  NT-based version of Windows
    \value WV_CE_based  CE-based version of Windows

    \sa MacVersion
*/

/*!
    \enum QSysInfo::MacVersion

    This enum provides symbolic names for the various versions of the
    Macintosh operating system. On Mac, the
    QSysInfo::MacintoshVersion variable gives the version of the
    system on which the application is run.

    \value MV_9        MacOS 9 (unsupported)
    \value MV_10_0     Mac OS X 10.0
    \value MV_10_1     Mac OS X 10.1
    \value MV_10_2     Mac OS X 10.2
    \value MV_10_3     Mac OS X 10.3
    \value MV_10_4     Mac OS X 10.4
    \value MV_Unknown  Other

    \sa WinVersion
*/

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
    return QCFString(CFStringCreateWithPascalString(0, pstr, CFStringGetSystemEncoding()));
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
    \relates <QtGlobal>

    Prints a warning message containing the source code file name and
    line number if \a test is false.

    Q_ASSERT() is useful for testing pre- and post-conditions
    during development. It does nothing if \c QT_NO_DEBUG was defined
    during compilation.

    Example:
    \code
        //
        // File: div.cpp
        //

        #include <QtGlobal>

        int divide(int a, int b)
        {
            Q_ASSERT(b != 0);
            return a / b;
        }
    \endcode

    If \c b is zero, the Q_ASSERT statement will output the following
    message using the qFatal() function:

    \code
        ASSERT: "b == 0" in file div.cpp, line 9
    \endcode

    \sa Q_ASSERT_X(), qFatal(), {Debugging Techniques}
*/

/*!
    \macro void Q_ASSERT_X(bool test, const char *where, const char *what)
    \relates <QtGlobal>

    Prints the message \a what together with the location \a where,
    the source file name and line number if \a test is false.

    Q_ASSERT_X is useful for testing pre- and post-conditions during
    development. It does nothing if \c QT_NO_DEBUG was defined during
    compilation.

    Example:
    \code
        //
        // File: div.cpp
        //

        #include <QtGlobal>

        int divide(int a, int b)
        {
            Q_ASSERT_X(b != 0, "divide", "division by zero");
            return a/b;
        }
    \endcode

    If \c b is zero, the Q_ASSERT_X statement will output the following
    message using the qFatal() function:

    \code
        ASSERT failure in divide: "division by zero", file div.cpp, line 9
    \endcode

    \sa Q_ASSERT(), qFatal(), {Debugging Techniques}
*/

/*!
    \macro void Q_CHECK_PTR(void *p)
    \relates <QtGlobal>

    If \a p is 0, prints a warning message containing the source code file
    name and line number, saying that the program ran out of memory.

    Q_CHECK_PTR does nothing if \c QT_NO_DEBUG was defined during
    compilation.

    Example:
    \code
        int *a;

        Q_CHECK_PTR(a = new int[80]);  // WRONG!

        a = new (nothrow) int[80];       // Right
        Q_CHECK_PTR(a);
    \endcode

    \sa qWarning(), {Debugging Techniques}
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
    \relates <QtGlobal>

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

    \sa qDebug(), qWarning(), qFatal(), {Debugging Techniques}
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
        || (msgType == QtWarningMsg
            && (!qgetenv("QT_FATAL_WARNINGS").isNull())) ) {

#if defined(Q_CC_MSVC) && defined(QT_DEBUG) && defined(_DEBUG) && defined(_CRT_ERROR)
        // get the current report mode
        int reportMode = _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_WNDW);
        _CrtSetReportMode(_CRT_ERROR, reportMode);
        int ret = _CrtDbgReport(_CRT_ERROR, __FILE__, __LINE__, QT_VERSION_STR, buf);
        if (ret == 0  && reportMode & _CRTDBG_MODE_WNDW)
            return; // ignore
        else if (ret == 1)
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
    \relates <QtGlobal>

    Calls the message handler with the debug message \a msg. If no
    message handler has been installed, the message is printed to
    stderr. Under Windows, the message is sent to the debugger. This
    function does nothing if \c QT_NO_DEBUG_OUTPUT was defined during
    compilation.

    If you pass the function a format string and a list of arguments,
    it works in similar way to the C printf() function.

    Example:
    \code
        qDebug("items in list: %d", myList.size());
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
        {Debugging Techniques}
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
    \relates <QtGlobal>

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
        {Debugging Techniques}
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
    \relates <QtGlobal>

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
                qCritical("file '%s' does not exist!", filename.toLocal8Bit());
        }
    \endcode

    \warning The internal buffer is limited to 8192 bytes, including
    the '\0'-terminator.

    \warning Passing (const char *)0 as argument to qCritical might
    lead to crashes on certain platforms due to the platforms printf
    implementation.

    \sa qDebug(), qWarning(), qFatal(), qInstallMsgHandler(),
        {Debugging Techniques}
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
    \relates <QtGlobal>

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
        {Debugging Techniques}
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
QByteArray qgetenv(const char *varName)
{
#if defined(_MSC_VER) && _MSC_VER >= 1400
    size_t requiredSize;
    QByteArray buffer;
    getenv_s(&requiredSize, 0, 0, varName);
    if (requiredSize == 0)
        return buffer;
    buffer.resize(requiredSize);
    getenv_s(&requiredSize, buffer.data(), requiredSize, varName);
    return buffer;
#else
    return QByteArray(::getenv(varName));
#endif
}

/*!
    \macro foreach(variable, container)
    \relates <QtGlobal>

    This macro is used to implement Qt's \c foreach loop. \a variable
    is a variable name or variable definition; \a container is a Qt
    container whose value type corresponds to the type of the
    variable. See \l{The foreach Keyword} for details.

    If you're worried about namespace pollution, you can disable this
    macro by adding the following line to your \c .pro file:

    \code
        CONFIG += no_keywords
    \endcode

    \sa Q_FOREACH()
*/

/*!
    \macro Q_FOREACH(variable, container)
    \relates <QtGlobal>

    Same as foreach(\a variable, \a container).
*/

/*!
    \macro const char *QT_TR_NOOP(const char *sourceText)
    \relates <QtGlobal>

    Marks the string literal \a sourceText for translation in the
    current context. Expands to \a sourceText.

    Example:

    \code
        QString FriendlyConversation::greeting(int type)
        {
	    static const char *greeting_strings[] = {
	        QT_TR_NOOP("Hello"),
	        QT_TR_NOOP("Goodbye")
	    };
	    return tr(greeting_strings[type]);
        }
    \endcode

    \sa QT_TRANSLATE_NOOP(), {Internationalization with Qt}
*/

/*!
    \macro const char *QT_TRANSLATE_NOOP(const char *context, const char *sourceText)
    \relates <QtGlobal>

    Marks the string literal \a sourceText for translation in the
    given \a context. Expands to \a sourceText.

    Example:

    \code
        static const char *greeting_strings[] = {
	    QT_TRANSLATE_NOOP("FriendlyConversation", "Hello"),
	    QT_TRANSLATE_NOOP("FriendlyConversation", "Goodbye")
        };

        QString FriendlyConversation::greeting(int type)
        {
	    return tr(greeting_strings[type]);
        }

        QString global_greeting(int type)
        {
	    return qApp->translate("FriendlyConversation",
				   greeting_strings[type]);
        }
    \endcode

    \sa QT_TR_NOOP(), {Internationalization with Qt}
*/

#ifdef QT3_SUPPORT
#include <qlibraryinfo.h>
static const char *qInstallLocation(QLibraryInfo::LibraryLocation loc)
{
    static QByteArray ret;
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
