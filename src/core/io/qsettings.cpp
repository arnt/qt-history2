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

// POSIX Large File Support redefines open -> open64
static inline int qt_open(const char *pathname, int flags, mode_t mode)
{ return ::open(pathname, flags, mode); }
#if defined(open)
# undef open
#endif

// POSIX Large File Support redefines truncate -> truncate64
#if defined(truncate)
# undef truncate
#endif

#include "qsettings.h"

#ifndef QT_NO_SETTINGS

#include "qdir.h"
#include "qfile.h"
#include "qfileinfo.h"
#include "qmap.h"
#include "qtextstream.h"
#include "qregexp.h"
#include <private/qsettings_p.h>
#ifndef NO_ERRNO_H
#include <errno.h>
#endif

/*!
    \class QSettings
    \brief The QSettings class provides persistent platform-independent application settings.

    \ingroup io
    \ingroup misc
    \mainclass

    On Unix systems, QSettings uses text files to store settings. On Windows
    systems, QSettings uses the system registry.  On Mac OS X, QSettings uses
    the Carbon preferences API.

    Each setting comprises an identifying key and the data associated with
    the key. A key is a unicode string which consists of \e two or more
    subkeys. A subkey is a slash, '/', followed by one or more unicode
    characters (excluding slashes, newlines, carriage returns and equals,
    '=', signs). The associated data, called the entry or value, may be a
    boolean, an integer, a double, a string or a list of strings. Entry
    strings may contain any unicode characters.

    If you want to save and restore the entire desktop's settings, i.e.
    which applications are running, use QSettings to save the settings
    for each individual application and QSessionManager to save the
    desktop's session.

    Example settings:
    \code
    /MyCompany/MyApplication/background color
    /MyCompany/MyApplication/foreground color
    /MyCompany/MyApplication/geometry/x
    /MyCompany/MyApplication/geometry/y
    /MyCompany/MyApplication/geometry/width
    /MyCompany/MyApplication/geometry/height
    /MyCompany/MyApplication/recent files/1
    /MyCompany/MyApplication/recent files/2
    /MyCompany/MyApplication/recent files/3
    \endcode
    Each line above is a complete key, made up of subkeys.

    A typical usage pattern for reading settings at application
    startup:
    \code
    QSettings settings;
    settings.setPath("MyCompany.com", "MyApplication");

    QString bgColor = settings.readEntry("/colors/background", "white");
    int width = settings.readNumEntry("/geometry/width", 640);
    // ...
    \endcode

    A typical usage pattern for saving settings at application exit or
    'save preferences':
    \code
    QSettings settings;
    settings.setPath("MyCompany.com", "MyApplication");

    settings.writeEntry("/colors/background", bgColor);
    settings.writeEntry("/geometry/width", width);
    // ...
    \endcode

    A key prefix can be prepended to all keys using beginGroup(). The
    application of the prefix is stopped using endGroup(). For
    example:
    \code
    QSettings settings;

    settings.beginGroup("/MainWindow");
        settings.beginGroup("/Geometry");
            int x = settings.readEntry("/x");
            // ...
        settings.endGroup();
        settings.beginGroup("/Toolbars");
            // ...
        settings.endGroup();
    settings.endGroup();
    \endcode

    You can get a list of entry-holding keys by calling entryList(), and
    a list of key-holding keys using subkeyList().

    \code
    QStringList keys = entryList("/MyApplication");
    // keys contains 'background color' and 'foreground color'.

    QStringList keys = entryList("/MyApplication/recent files");
    // keys contains '1', '2' and '3'.

    QStringList subkeys = subkeyList("/MyApplication");
    // subkeys contains 'geometry' and 'recent files'

    QStringList subkeys = subkeyList("/MyApplication/recent files");
    // subkeys is empty.
    \endcode

    Since settings for Windows are stored in the registry there are
    some size limitations as follows:
    \list
    \i A subkey may not exceed 255 characters.
    \i An entry's value may not exceed 16,300 characters.
    \i All the values of a key (for example, all the 'recent files'
    subkeys values), may not exceed 65,535 characters.
    \endlist

    These limitations are not enforced on Unix or Mac OS X.

    \warning Creating multiple, simultaneous instances of QSettings writing
    to a text file may lead to data loss! This is a known issue which will
    be fixed in a future release of Qt.

    \section1 Notes for Mac OS X Applications

    The location where settings are stored is not formally defined by
    the CFPreferences API.

    At the time of writing settings are stored (either on a global or
    user basis, preferring locally) into a plist file in \c
    $ROOT/System/Library/Preferences (in XML format). QSettings will
    create an appropriate plist file (\c{com.<first group name>.plist})
    out of the full path to a key.

    For further information on CFPreferences see
    \link http://developer.apple.com/documentation/CoreFoundation/Conceptual/CFPreferences/index.html
    Apple's Specifications\endlink

    \section1 Notes for Unix Applications

    There is no universally accepted place for storing application
    settings under Unix. In the examples the settings file will be
    searched for in the following directories:
    \list 1
    \i \c SYSCONF - the default value is \c INSTALL/etc/settings
    \i \c /opt/MyCompany/share/etc
    \i \c /opt/MyCompany/share/MyApplication/etc
    \i \c $HOME/.qt
    \endlist
    When reading settings the files are searched in the order shown
    above, with later settings overriding earlier settings. Files for
    which the user doesn't have read permission are ignored. When saving
    settings QSettings works in the order shown above, writing
    to the first settings file for which the user has write permission.
    (\c INSTALL is the directory where Qt was installed.  This can be
    modified by using the configure script's -prefix argument)

    If you want to put the settings in a particular place in the
    filesystem you could do this:
    \code
    settings.insertSearchPath(QSettings::Unix, "/opt/MyCompany/share");
    \endcode

    But in practice you may prefer not to use a search path for Unix.
    For example the following code:
    \code
    settings.writeEntry("/MyApplication/geometry/width", width);
    \endcode
    will end up writing the "geometry/width" setting to the file
    \c{$HOME/.qt/myapplicationrc} (assuming that the application is
    being run by an ordinary user, i.e. not by root).

    For cross-platform applications you should ensure that the
    \link #sizelimit Windows size limitations \endlink are not exceeded.
*/

/*!
    \enum QSettings::System

    \value Mac Macintosh execution environments
    \value Unix Mac OS X, Unix, Linux and Unix-like execution environments
    \value Windows Windows execution environments
*/

/*!
    \enum QSettings::Format

    \value Native Store the settings in a platform dependent location
    \value Ini Store the settings in a text file
*/

/*!
    \enum QSettings::Scope

    \value Global Save settings as global as possible
    \value User Save settings in user space
*/

#if defined(Q_OS_UNIX)
typedef int HANDLE;
#define Q_LOCKREAD F_RDLCK
#define Q_LOCKWRITE F_WRLCK
/*
  Locks the file specified by name.  The lockfile is created as a
  hidden file in the same directory as the target file, with .lock
  appended to the name. For example, "/etc/settings/onerc" uses a
  lockfile named "/etc/settings/.onerc.lock".  The type argument
  controls the type of the lock, it can be either F_RDLCK for a read
  lock, or F_WRLCK for a write lock.

  A file descriptor for the lock file is returned, and should be
  closed with closelock() when the lock is no longer needed.
 */
static HANDLE openlock(const QString &name, int type)
{
    QFileInfo info(name);
    // lockfile should be hidden, and never removed
    QString lockfile = info.path() + QLatin1String("/.") + info.fileName() + QLatin1String(".lock");

    // open the lockfile
    HANDLE fd = qt_open(QFile::encodeName(lockfile),
                         O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    if (fd < 0) {
        // failed to open the lock file, most likely because of permissions
        return fd;
    }

    struct flock fl;
    fl.l_type = type;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    if (fcntl(fd, F_SETLKW, &fl) == -1) {
        // the lock failed, so we should fail silently, so that people
        // using filesystems that do not support locking don't see
        // numerous warnings about a failed lock
        close(fd);
        fd = -1;
    }

    return fd;
}

/*
  Closes the lock file specified by fd.  fd is the file descriptor
  returned by the openlock() function.
*/
static void closelock(HANDLE fd)
{
    if (fd < 0) {
        // the lock file is not open
        return;
    }

    struct flock fl;
    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    // ignore the return value, so that the unlock fails silently
    (void) fcntl(fd, F_SETLKW, &fl);

    close(fd);
}
#elif defined(Q_WS_WIN)
#define Q_LOCKREAD 1
#define Q_LOCKWRITE 2

static HANDLE openlock(const QString &name, int type)
{
    return 0; // ###
    if (!QFile::exists(name))
        return 0;

    HANDLE fd = 0;
    DWORD shareFlag = (type & Q_LOCKREAD) ? 0 : FILE_SHARE_READ;
    shareFlag |= (type & Q_LOCKWRITE) ? 0 : FILE_SHARE_WRITE;

    QT_WA({
        fd = CreateFileW((TCHAR*)name.utf16(), GENERIC_READ, shareFlag, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    } , {
        fd = CreateFileA(name.local8Bit(), GENERIC_READ, shareFlag, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    });

    if (!fd)
        qWarning("QSettings: openlock failed!");
    return fd;
}

static void closelock(HANDLE fd)
{
    if (!fd)
        return;

    CloseHandle(fd);
}
#endif


QSettingsGroup::QSettingsGroup()
    : modified(false)
{
}




void QSettingsHeading::read(const QString &filename)
{
    if (! QFileInfo(filename).exists())
        return;

    HANDLE lockfd = openlock(filename, Q_LOCKREAD);

    QFile file(filename);
    if (! file.open(IO_ReadOnly)) {
        qWarning("QSettings: failed to open file '%s'", filename.latin1());
        return;
    }

    git = end();

    QTextStream stream(&file);
    stream.setEncoding(QTextStream::UnicodeUTF8);
    while (! stream.atEnd())
        parseLine(stream);

    git = end();

    file.close();

    closelock(lockfd);
}


void QSettingsHeading::parseLine(QTextStream &stream)
{
    QString line = stream.readLine();
    if (line.isEmpty())
        // empty line... we'll allow it
        return;

    if (line[0] == QLatin1Char('#'))
        // commented line
        return;

    if (line[0] == QLatin1Char('[')) {
        QString gname = line;

        gname = gname.remove((uint)0, 1);
        if (gname[(int)gname.length() - 1] == QLatin1Char(']'))
            gname = gname.remove(gname.length() - 1, 1);

        git = find(gname);
        if (git == end())
            git = insert(gname, QSettingsGroup());
    } else {
        if (git == end()) {
            qWarning("QSettings: line '%s' out of group", line.latin1());
            return;
        }

        int i = line.indexOf(QLatin1Char('='));
        if (i == -1) {
            qWarning("QSettings: malformed line '%s' in group '%s'",
                     line.latin1(), git.key().latin1());
            return;
        } else {
            QString key, value;
            key = line.left(i);
            value = QLatin1String("");
            bool esc=true;
            i++;
            while (esc) {
                esc = false;
                for (; i < (int)line.length(); i++) {
                    if (esc) {
                        if (line[i] == QLatin1Char('n'))
                            value.append(QLatin1Char('\n')); // escaped newline
                        else if (line[i] == QLatin1Char('0'))
                            value = QString::null; // escaped empty string
                        else
                            value.append(line[i]);
                        esc = false;
                    } else if (line[i] == QLatin1Char('\\'))
                        esc = true;
                    else
                        value.append(line[i]);
                }
                if (esc) {
                    // Backwards-compatibility...
                    // still escaped at EOL - manually escaped "newline"
                    if (stream.atEnd()) {
                        qWarning("QSettings: reached end of file, expected continued line");
                        break;
                    }
                    value.append(QLatin1Char('\n'));
                    line = stream.readLine();
                    i = 0;
                }
            }

            (*git).insert(key, value);
        }
    }
}

#ifdef Q_WS_WIN // for homedirpath reading from registry
#include "qt_windows.h"
#include "qlibrary.h"

#ifndef CSIDL_APPDATA
#define CSIDL_APPDATA                   0x001a        // <user name>\Application Data
#endif
#ifndef CSIDL_COMMON_APPDATA
#define CSIDL_COMMON_APPDATA            0x0023        // All Users\Application Data
#endif

#endif

QSettingsPrivate::QSettingsPrivate(QSettings::Format format)
    : groupDirty(true), modified(false), globalScope(true)
{
#if !defined(QWS) && (defined(Q_WS_WIN) || defined(Q_OS_MAC))
    if (format != QSettings::Ini)
        return;
#else
    Q_UNUSED(format);
#endif

    QString appSettings(QDir::homePath() + QLatin1String("/.qt/"));
    QString defPath;
#ifdef Q_WS_WIN
#ifdef Q_OS_TEMP
        TCHAR path[MAX_PATH];
        SHGetSpecialFolderPath(0, path, CSIDL_APPDATA, false);
        appSettings  = QString::fromUtf16(path);
        SHGetSpecialFolderPath(0, path, CSIDL_COMMON_APPDATA, false);
        defPath = QString::fromUtf16(path);
#else
    QT_WA({
        HINSTANCE pHnd = LoadLibraryW(L"shell32.dll");
        typedef BOOL (WINAPI*GetSpecialFolderPath)(HWND, LPTSTR, int, BOOL);
        GetSpecialFolderPath SHGetSpecialFolderPath = (GetSpecialFolderPath)GetProcAddress(pHnd, "SHGetSpecialFolderPathW");;
        if (SHGetSpecialFolderPath) {
            TCHAR path[MAX_PATH];
            SHGetSpecialFolderPath(0, path, CSIDL_APPDATA, false);
            appSettings  = QString::fromUtf16((ushort*)path);
            SHGetSpecialFolderPath(0, path, CSIDL_COMMON_APPDATA, false);
            defPath = QString::fromUtf16((ushort*)path);
        }
    } , {
        HINSTANCE pHnd = LoadLibraryA("shell32.dll");
        typedef BOOL (WINAPI*GetSpecialFolderPath)(HWND, char*, int, BOOL);
        GetSpecialFolderPath SHGetSpecialFolderPath = (GetSpecialFolderPath)GetProcAddress(pHnd, "SHGetSpecialFolderPathA");;
        if (SHGetSpecialFolderPath) {
            char path[MAX_PATH];
            SHGetSpecialFolderPath(0, path, CSIDL_APPDATA, false);
            appSettings = QString::fromLocal8Bit(path);
            SHGetSpecialFolderPath(0, path, CSIDL_COMMON_APPDATA, false);
            defPath = QString::fromLocal8Bit(path);
        }
    });
#endif // Q_OS_TEMP
#else
    defPath = QString::fromLocal8Bit(qInstallPathSysconf());
#endif
    QDir dir(appSettings);
    if (! dir.exists()) {
        if (! dir.mkdir(dir.path()))
            qWarning("QSettings: error creating %s", dir.path().latin1());
    }

    if (defPath.size())
        searchPaths.append(defPath);
    searchPaths.append(dir.path());
}

QSettingsPrivate::~QSettingsPrivate()
{
}

QSettingsGroup QSettingsPrivate::readGroup()
{
    QSettingsHeading hd;
    QSettingsGroup grp;

    QMap<QString,QSettingsHeading>::Iterator headingsit = headings.find(heading);
    if (headingsit != headings.end())
        hd = *headingsit;

    QSettingsHeading::Iterator grpit = hd.find(group);
    if (grpit == hd.end()) {
        QStringList::Iterator it = searchPaths.begin();
        if (!globalScope)
            ++it;
        while (it != searchPaths.end()) {
            QString filebase = heading.toLower().replace(QRegExp(QString::fromLatin1("\\s+")), QLatin1String("_"));
            QString fn((*it++) + QLatin1Char('/') + filebase + QLatin1String("rc"));
            if (! hd.contains(fn + QLatin1String("cached"))) {
                hd.read(fn);
                hd.insert(fn + QLatin1String("cached"), QSettingsGroup());
            }
        }

        headings.insert(heading, hd);

        grpit = hd.find(group);
        if (grpit != hd.end())
            grp = *grpit;
    } else if (hd.count() != 0)
        grp = *grpit;

    return grp;
}


void QSettingsPrivate::removeGroup(const QString &key)
{
    QSettingsHeading hd;
    QSettingsGroup grp;
    bool found = false;

    QMap<QString,QSettingsHeading>::Iterator headingsit = headings.find(heading);
    if (headingsit != headings.end())
        hd = *headingsit;

    QSettingsHeading::Iterator grpit = hd.find(group);
    if (grpit == hd.end()) {
        QStringList::Iterator it = searchPaths.begin();
        if (!globalScope)
            ++it;
        while (it != searchPaths.end()) {
            QString filebase = heading.toLower().replace(QRegExp(QLatin1String("\\s+")), QLatin1String("_"));
            QString fn((*it++) + QLatin1Char('/') + filebase + QLatin1String("rc"));
            if (! hd.contains(fn + QLatin1String("cached"))) {
                hd.read(fn);
                hd.insert(fn + QLatin1String("cached"), QSettingsGroup());
            }
        }

        headings.insert(heading, hd);

        grpit = hd.find(group);
        if (grpit != hd.end()) {
            found = true;
            grp = *grpit;
        }
    } else if (hd.count() != 0) {
        found = true;
        grp = *grpit;
    }

    if (found) {
        grp.remove(key);

        if (grp.count() > 0)
            hd.insert(group, grp);
        else
            hd.remove(group);

        if (hd.count() > 0)
            headings.insert(heading, hd);
        else
            headings.remove(heading);

        modified = true;
    }
}


void QSettingsPrivate::writeGroup(const QString &key, const QString &value)
{
    QSettingsHeading hd;
    QSettingsGroup grp;

    QMap<QString,QSettingsHeading>::Iterator headingsit = headings.find(heading);
    if (headingsit != headings.end())
        hd = *headingsit;

    QSettingsHeading::Iterator grpit = hd.find(group);
    if (grpit == hd.end()) {
        QStringList::Iterator it = searchPaths.begin();
        if (!globalScope)
            ++it;
        while (it != searchPaths.end()) {
            QString filebase = heading.toLower().replace(QRegExp(QLatin1String("\\s+")), QLatin1String("_"));
            QString fn((*it++) + QLatin1Char('/') + filebase + QLatin1String("rc"));
            if (! hd.contains(fn + QLatin1String("cached"))) {
                hd.read(fn);
                hd.insert(fn + QLatin1String("cached"), QSettingsGroup());
            }
        }

        headings.insert(heading, hd);

        grpit = hd.find(group);
        if (grpit != hd.end())
            grp = *grpit;
    } else if (hd.count() != 0)
        grp = *grpit;

    grp.modified = true;
    grp.insert(key, value);
    hd.insert(group, grp);
    headings.insert(heading, hd);

    modified = true;
}


QDateTime QSettingsPrivate::modificationTime()
{
    QSettingsHeading hd = headings[heading];
    QSettingsGroup grp = hd[group];

    QDateTime datetime;

    QStringList::Iterator it = searchPaths.begin();
    if (!globalScope)
        ++it;
    while (it != searchPaths.end()) {
        QFileInfo fi((*it++) + QLatin1Char('/') + heading + QLatin1String("rc"));
        if (fi.exists() && fi.lastModified() > datetime)
            datetime = fi.lastModified();
    }

    return datetime;
}

bool qt_verify_key(const QString &key)
{
    if (key.isEmpty() || key[0] != QLatin1Char('/') || (bool)key.contains(QRegExp(QLatin1String("[=\\r\\n]"))))
        return false;
    return true;
}

static QString groupKey(const QString &group, const QString &key)
{
    QString grp_key;
    if (group.isEmpty() || (group.length() == 1 && group[0] == QLatin1Char('/'))) {
        // group is empty, or it contains a single '/', so we just return the key
        if (key.startsWith(QLatin1String("/")))
            grp_key = key;
        else
            grp_key = QLatin1Char('/') + key;
    } else if (group.endsWith(QLatin1String("/")) || key.startsWith(QLatin1String("/"))) {
        grp_key = group + key;
    } else {
        grp_key = group + QLatin1Char('/') + key;
    }
    return grp_key;
}

/*!
  Inserts \a path into the settings search path. The semantics of \a
  path depends on the system \a s. It is usually easier and better to
  use setPath() instead of this function.

  When \a s is \e Windows and the execution environment is \e not
  Windows the function does nothing. Similarly when \a s is \e Unix and
  the execution environment is \e not Unix the function does nothing.

  When \a s is \e Windows, and the execution environment is Windows, the
  search path list will be used as the first subfolder of the "Software"
  folder in the registry.

  When reading settings the folders are searched forwards from the
  first folder (listed below) to the last, returning the first
  settings found, and ignoring any folders for which the user doesn't
  have read permission.
  \list 1
  \i HKEY_CURRENT_USER/Software/MyCompany/MyApplication
  \i HKEY_LOCAL_MACHINE/Software/MyCompany/MyApplication
  \i HKEY_CURRENT_USER/Software/MyApplication
  \i HKEY_LOCAL_MACHINE/Software/MyApplication
  \endlist

  \code
  QSettings settings;
  settings.insertSearchPath(QSettings::Windows, "/MyCompany");
  settings.writeEntry("/MyApplication/Tip of the day", true);
  \endcode
  The code above will write the subkey "Tip of the day" into the \e
  first of the registry folders listed below that is found and for
  which the user has write permission.
  \list 1
  \i HKEY_LOCAL_MACHINE/Software/MyCompany/MyApplication
  \i HKEY_CURRENT_USER/Software/MyCompany/MyApplication
  \i HKEY_LOCAL_MACHINE/Software/MyApplication
  \i HKEY_CURRENT_USER/Software/MyApplication
  \endlist
  If a setting is found in the HKEY_CURRENT_USER space, this setting
  is overwritten independently of write permissions in the
  HKEY_LOCAL_MACHINE space.

  When \a s is \e Unix, and the execution environment is Unix, the
  search path list will be used when trying to determine a suitable
  filename for reading and writing settings files. By default, there are
  two entries in the search path:

  \list 1
  \i \c SYSCONF - where \c SYSCONF is a directory specified when
  configuring Qt; by default it is INSTALL/etc/settings.
  \i \c $HOME/.qt/ - where \c $HOME is the user's home directory.
  \endlist

  All insertions into the search path will go before $HOME/.qt/.
  For example:
  \code
  QSettings settings;
  settings.insertSearchPath(QSettings::Unix, "/opt/MyCompany/share/etc");
  settings.insertSearchPath(QSettings::Unix, "/opt/MyCompany/share/MyApplication/etc");
  // ...
  \endcode
  Will result in a search path of:
  \list 1
  \i SYSCONF
  \i /opt/MyCompany/share/etc
  \i /opt/MyCompany/share/MyApplication/etc
  \i $HOME/.qt
  \endlist
    When reading settings the files are searched in the order shown
    above, with later settings overriding earlier settings. Files for
    which the user doesn't have read permission are ignored. When saving
    settings QSettings works in the order shown above, writing
    to the first settings file for which the user has write permission.

    Note that paths in the file system are not created by this
    function, so they must already exist to be useful.

  Settings under Unix are stored in files whose names are based on the
  first subkey of the key (not including the search path). The algorithm
  for creating names is essentially: lowercase the first subkey, replace
  spaces with underscores and add 'rc', e.g.
  <tt>/MyCompany/MyApplication/background color</tt> will be stored in
  <tt>myapplicationrc</tt> (assuming that <tt>/MyCompany</tt> is part of
  the search path).

  \sa removeSearchPath()

*/
void QSettings::insertSearchPath(System s, const QString &path)
{
#if !defined(QWS) && (defined(Q_WS_WIN) || defined(Q_OS_MAC))
    if (d->sysd) {
        d->sysInsertSearchPath(s, path);
        return;
    }
#endif

#if !defined(Q_WS_WIN)
    if (s == Windows)
        return;
#endif
#if !defined(Q_OS_MAC)
    if (s == Mac)
        return;
#endif

    if (!qt_verify_key(path)) {
        qWarning("QSettings::insertSearchPath: Invalid key: '%s'", path.latin1());
        return;
    }

#if !defined(QWS) && (defined(Q_WS_WIN) || defined(Q_OS_MAC))
    if (d->sysd && s != Unix) {
#else
    if (s != Unix) {
#endif
#if !defined(QWS) && defined(Q_OS_MAC)
        if(s != Mac) //mac is respected on the mac as well
#endif
            return;
    }

    QString realPath = path;
#if defined(Q_WS_WIN)
    QString defPath = d->globalScope ? d->searchPaths.first() : d->searchPaths.last();
    realPath = defPath + path;
#endif

    int idx = d->searchPaths.indexOf(d->searchPaths.last());
    if (idx != -1) {
        d->searchPaths.insert(idx, realPath);
    }
}


/*!
  Removes all occurrences of \a path (using exact matching) from the
  settings search path for system \a s. Note that the default search
  paths cannot be removed.

  \sa insertSearchPath()
*/
void QSettings::removeSearchPath(System s, const QString &path)
{
    if (!qt_verify_key(path)) {
        qWarning("QSettings::insertSearchPath: Invalid key: '%s'", path.latin1());
        return;
    }

#ifdef Q_WS_WIN
    if (d->sysd) {
        d->sysRemoveSearchPath(s, path);
        return;
    }
#endif
#if !defined(QWS) && (defined(Q_WS_WIN) || defined(Q_OS_MAC))
    if (d->sysd && s != Unix) {
#else
    if (s != Unix) {
#endif
#if !defined(QWS) && defined(Q_OS_MAC)
        if(s != Mac) //mac is respected on the mac as well
#endif
            return;
    }

    if (path == d->searchPaths.first() || path == d->searchPaths.last())
        return;

    d->searchPaths.removeAll(path);
}


/*!
  Creates a settings object.
*/
QSettings::QSettings()
{
    d = new QSettingsPrivate(Native);

#if !defined(QWS) && (defined(Q_WS_WIN) || defined(Q_OS_MAC))
    d->sysd = 0;
    d->sysInit();
#endif
}

/*!
  Creates a settings object. If \a format is 'Ini' the settings will
  be stored in a text file, using the Unix strategy (see above). If \a format
  is 'Native', the settings will be stored in a platform specific way
  (ie. the Windows registry).
*/
QSettings::QSettings(Format format)
{
    d = new QSettingsPrivate(format);

#if !defined(QWS) && (defined(Q_WS_WIN) || defined(Q_OS_MAC))
    d->sysd = 0;
    if (format == Native)
        d->sysInit();
#else
    Q_UNUSED(format);
#endif
}

/*!
  Destroys the settings object.  All modifications made to the settings
  will automatically be saved.

*/
QSettings::~QSettings()
{
    sync();

#if !defined(QWS) && (defined(Q_WS_WIN) || defined(Q_OS_MAC))
    if (d->sysd)
        d->sysClear();
#endif

    delete d;
}


/*! \internal
  Writes all modifications to the settings to disk.  If any errors are
  encountered, this function returns false, otherwise it will return true.
*/
bool QSettings::sync()
{
#if !defined(QWS) && (defined(Q_WS_WIN) || defined(Q_OS_MAC))
    if (d->sysd)
        return d->sysSync();
#endif
    if (! d->modified)
        // fake success
        return true;

    bool success = true;
    QMap<QString,QSettingsHeading>::Iterator it = d->headings.begin();

    while (it != d->headings.end()) {
        // determine filename
        QSettingsHeading hd(*it);
        QSettingsHeading::Iterator hdit = hd.begin();
        QString filename;

        QStringList::Iterator pit = d->searchPaths.begin();
        if (!d->globalScope)
            ++pit;
        while (pit != d->searchPaths.end()) {
            QString filebase = it.key().toLower().replace(QRegExp(QLatin1String("\\s+")), QLatin1String("_"));
            QFileInfo di(*pit);
            if (!di.exists()) {
                QDir dir;
                dir.mkdir(*pit);
            }

            QFileInfo fi((*pit++) + QLatin1Char('/') + filebase + QLatin1String("rc"));

            if ((fi.exists() && fi.isFile() && fi.isWritable()) ||
                (! fi.exists() && di.isDir()
#ifndef Q_WS_WIN
                && di.isWritable()
#else
                && ((QSysInfo::WindowsVersion&QSysInfo::WV_NT_based) > QSysInfo::WV_2000 || di.isWritable())
#endif
               )) {
                filename = fi.filePath();
                break;
            }
        }

        ++it;

        if (filename.isEmpty()) {
            qWarning("QSettings::sync: filename is null/empty");
            success = false;
            continue;
        }

        HANDLE lockfd = openlock(filename, Q_LOCKWRITE);

        QFile file(filename + QLatin1String(".tmp"));
        if (! file.open(IO_WriteOnly)) {
            qWarning("QSettings::sync: failed to open '%s' for writing",
                     file.fileName().latin1());
            success = false;
            continue;
        }

        // spew to file
        QTextStream stream(&file);
        stream.setEncoding(QTextStream::UnicodeUTF8);

        while (hdit != hd.end()) {
            if ((*hdit).count() > 0) {
                stream << "[" << hdit.key() << "]" << endl;

                QSettingsGroup grp(*hdit);
                QSettingsGroup::Iterator grpit = grp.begin();

                while (grpit != grp.end()) {
                    QString v = grpit.value();
                    if (v.isNull()) {
                        v = QLatin1String("\\0"); // escape null string
                    } else {
                        v.replace(QLatin1Char('\\'), QLatin1String("\\\\")); // escape backslash
                        v.replace(QLatin1Char('\n'), QLatin1String("\\n")); // escape newlines
                    }

                    stream << grpit.key() << QLatin1Char('=') << v << endl;
                    ++grpit;
                }

                stream << endl;
            }

            ++hdit;
        }

        if (file.status() != IO_Ok) {
            qWarning("QSettings::sync: error at end of write");
            success = false;
        }

        file.close();

        if (success) {
            QDir dir(QFileInfo(file).absoluteDir());
            if (dir.exists(filename) && !dir.remove(filename) ||
                 !dir.rename(file.fileName(), filename, true)) {
                qWarning("QSettings::sync: error writing file '%s'",
                          QFile::encodeName(filename).constData());
                success = false;
            }
        }

        // remove temporary file
        file.remove();

        closelock(lockfd);
    }

    d->modified = false;

    return success;
}


/*!
  \fn bool QSettings::readBoolEntry(const QString &key, bool def, bool *ok) const

  Reads the entry specified by \a key, and returns a bool, or the
  default value, \a def, if the entry couldn't be read.
  If \a ok is non-null, *ok is set to true if the key was read, false
  otherwise.

  \sa readEntry(), readNumEntry(), readDoubleEntry(), writeEntry(), removeEntry()
*/

/*!
    \internal
*/
bool QSettings::readBoolEntry(const QString &key, bool def, bool *ok)
{
    QString grp_key(groupKey(group(), key));
    if (!qt_verify_key(grp_key)) {
        qWarning("QSettings::readBoolEntry: Invalid key: '%s'", grp_key.latin1());
        if (ok)
            *ok = false;

        return def;
    }

#if !defined(QWS) && (defined(Q_WS_WIN) || defined(Q_OS_MAC))
    if (d->sysd)
        return d->sysReadBoolEntry(grp_key, def, ok);
#endif

    QString value = readEntry(key, QLatin1String(def ? "true" : "false"), ok);

    if (value.toLower() == QLatin1String("true"))
        return true;
    else if (value.toLower() == QLatin1String("false"))
        return false;
    else if (value == QLatin1String("1"))
        return true;
    else if (value == QLatin1String("0"))
        return false;

    if (! value.isEmpty())
        qWarning("QSettings::readBoolEntry: '%s' is not 'true' or 'false'",
                 value.latin1());
    if (ok)
        *ok = false;
    return def;
}


/*!
    \fn double QSettings::readDoubleEntry(const QString &key, double def, bool *ok) const

  Reads the entry specified by \a key, and returns a double, or the
  default value, \a def, if the entry couldn't be read.
  If \a ok is non-null, *ok is set to true if the key was read, false
  otherwise.

  \sa readEntry(), readNumEntry(), readBoolEntry(), writeEntry(), removeEntry()
*/

/*!
    \internal
*/
double QSettings::readDoubleEntry(const QString &key, double def, bool *ok)
{
    QString grp_key(groupKey(group(), key));
    if (!qt_verify_key(grp_key)) {
        qWarning("QSettings::readDoubleEntry: Invalid key: '%s'", grp_key.latin1());
        if (ok)
            *ok = false;

        return def;
    }

#if !defined(QWS) && (defined(Q_WS_WIN) || defined(Q_OS_MAC))
    if (d->sysd)
        return d->sysReadDoubleEntry(grp_key, def, ok);
#endif

    QString value = readEntry(key, QString::number(def), ok);
    bool conv_ok;
    double retval = value.toDouble(&conv_ok);
    if (conv_ok)
        return retval;
    if (! value.isEmpty())
        qWarning("QSettings::readDoubleEntry: '%s' is not a number",
                  value.latin1());
    if (ok)
        *ok = false;
    return def;
}


/*!
    \fn int QSettings::readNumEntry(const QString &key, int def, bool *ok) const

  Reads the entry specified by \a key, and returns an integer, or the
  default value, \a def, if the entry couldn't be read.
  If \a ok is non-null, *ok is set to true if the key was read, false
  otherwise.

  \sa readEntry(), readDoubleEntry(), readBoolEntry(), writeEntry(), removeEntry()
*/

/*!
    \internal
*/
int QSettings::readNumEntry(const QString &key, int def, bool *ok)
{
    QString grp_key(groupKey(group(), key));
    if (!qt_verify_key(grp_key)) {
        qWarning("QSettings::readNumEntry: Invalid key: '%s'", grp_key.latin1());
        if (ok)
            *ok = false;
        return def;
    }

#if !defined(QWS) && (defined(Q_WS_WIN) || defined(Q_OS_MAC))
    if (d->sysd)
        return d->sysReadNumEntry(grp_key, def, ok);
#endif

    QString value = readEntry(key, QString::number(def), ok);
    bool conv_ok;
    int retval = value.toInt(&conv_ok);
    if (conv_ok)
        return retval;
    if (! value.isEmpty())
        qWarning("QSettings::readNumEntry: '%s' is not a number",
                  value.latin1());
    if (ok)
        *ok = false;
    return def;
}


/*!
    \fn QString QSettings::readEntry(const QString &key, const QString &def, bool *ok) const

  Reads the entry specified by \a key, and returns a QString, or the
  default value, \a def, if the entry couldn't be read.
  If \a ok is non-null, *ok is set to true if the key was read, false
  otherwise.

  \sa readListEntry(), readNumEntry(), readDoubleEntry(), readBoolEntry(), writeEntry(), removeEntry()
*/

/*!
    \internal
*/
QString QSettings::readEntry(const QString &key, const QString &def, bool *ok)
{
    QString grp_key(groupKey(group(), key));
    if (!qt_verify_key(grp_key)) {
        qWarning("QSettings::readEntry: Invalid key: '%s'", grp_key.latin1());
        if (ok)
            *ok = false;

        return def;
    }

#if !defined(QWS) && (defined(Q_WS_WIN) || defined(Q_OS_MAC))
    if (d->sysd)
        return d->sysReadEntry(grp_key, def, ok);
#endif

    if (ok) // no, everything is not ok
        *ok = false;

    QString realkey;

    if (grp_key[0] == QLatin1Char('/')) {
        // parse our key
        QStringList list(grp_key.split(QLatin1Char('/'), QString::SkipEmptyParts));

        if (list.count() < 2) {
            qWarning("QSettings::readEntry: invalid key '%s'", grp_key.latin1());
            if (ok)
                *ok = false;
            return def;
        }

        if (list.count() == 2) {
            d->heading = list[0];
            d->group = QLatin1String("General");
            realkey = list[1];
        } else {
            d->heading = list[0];
            d->group = list[1];

            // remove the heading from the list
            list.removeFirst();
            // remove the group from the list
            list.removeFirst();

            realkey = list.join(QLatin1String("/"));
        }
    } else {
        realkey = grp_key;
    }

    QSettingsGroup grp = d->readGroup();
    QSettingsGroup::const_iterator it = grp.find(realkey), end = grp.end();
    QString retval = def;
    if (it != end) {
        // found the value we needed
        retval = *it;
        if (ok) *ok = true;
    }
    return retval;
}


#if !defined(Q_NO_BOOL_TYPE)
/*!
    Writes the boolean entry \a value into key \a key. The \a key is
    created if it doesn't exist. Any previous value is overwritten by \a
    value.

    If an error occurs the settings are left unchanged and false is
    returned; otherwise true is returned.

  \sa readListEntry(), readNumEntry(), readDoubleEntry(), readBoolEntry(), removeEntry()
*/
bool QSettings::writeEntry(const QString &key, bool value)
{
    QString grp_key(groupKey(group(), key));
    if (!qt_verify_key(grp_key)) {
        qWarning("QSettings::writeEntry: Invalid key: '%s'", grp_key.latin1());
        return false;
    }

#if !defined(QWS) && (defined(Q_WS_WIN) || defined(Q_OS_MAC))
    if (d->sysd)
        return d->sysWriteEntry(grp_key, value);
#endif
    QString s(QLatin1String(value ? "true" : "false"));
    return writeEntry(key, s);
}
#endif


/*!
    \overload
    Writes the double entry \a value into key \a key. The \a key is
    created if it doesn't exist. Any previous value is overwritten by \a
    value.

    If an error occurs the settings are left unchanged and false is
    returned; otherwise true is returned.

  \sa readListEntry(), readNumEntry(), readDoubleEntry(), readBoolEntry(), removeEntry()
*/
bool QSettings::writeEntry(const QString &key, double value)
{
    QString grp_key(groupKey(group(), key));
    if (!qt_verify_key(grp_key)) {
        qWarning("QSettings::writeEntry: Invalid key: '%s'", grp_key.latin1());
        return false;
    }

#if !defined(QWS) && (defined(Q_WS_WIN) || defined(Q_OS_MAC))
    if (d->sysd)
        return d->sysWriteEntry(grp_key, value);
#endif
    QString s(QString::number(value));
    return writeEntry(key, s);
}


/*!
    \overload
    Writes the integer entry \a value into key \a key. The \a key is
    created if it doesn't exist. Any previous value is overwritten by \a
    value.

    If an error occurs the settings are left unchanged and false is
    returned; otherwise true is returned.

  \sa readListEntry(), readNumEntry(), readDoubleEntry(), readBoolEntry(), removeEntry()
*/
bool QSettings::writeEntry(const QString &key, int value)
{
    QString grp_key(groupKey(group(), key));
    if (!qt_verify_key(grp_key)) {
        qWarning("QSettings::writeEntry: Invalid key: '%s'", grp_key.latin1());
        return false;
    }

#if !defined(QWS) && (defined(Q_WS_WIN) || defined(Q_OS_MAC))
    if (d->sysd)
        return d->sysWriteEntry(grp_key, value);
#endif
    QString s(QString::number(value));
    return writeEntry(key, s);
}


/*!
    \internal

  Writes the entry specified by \a key with the string-literal \a value,
  replacing any previous setting.  If \a value is zero-length or null, the
  entry is replaced by an empty setting.

  \e NOTE: This function is provided because some compilers use the
  writeEntry (const QString &, bool) overload for this code:
  writeEntry ("/foo/bar", "baz")

  If an error occurs, this functions returns false and the object is left
  unchanged.

  \sa readEntry(), removeEntry()
*/
bool QSettings::writeEntry(const QString &key, const char *value)
{
    return writeEntry(key, QString::fromLatin1(value));
}


/*!
    \overload
    Writes the string entry \a value into key \a key. The \a key is
    created if it doesn't exist. Any previous value is overwritten by \a
    value. If \a value is an empty string or a null string the key's
    value will be an empty string.

    If an error occurs the settings are left unchanged and false is
    returned; otherwise true is returned.

  \sa readListEntry(), readNumEntry(), readDoubleEntry(), readBoolEntry(), removeEntry()
*/
bool QSettings::writeEntry(const QString &key, const QString &value)
{
    QString grp_key(groupKey(group(), key));
    if (!qt_verify_key(grp_key)) {
        qWarning("QSettings::writeEntry: Invalid key: '%s'", grp_key.latin1());
        return false;
    }

#if !defined(QWS) && (defined(Q_WS_WIN) || defined(Q_OS_MAC))
    if (d->sysd)
        return d->sysWriteEntry(grp_key, value);
#endif
    // NOTE: we *do* allow value to be a null/empty string

    QString realkey;

    if (grp_key[0] == QLatin1Char('/')) {
        // parse our key
        QStringList list(grp_key.split(QLatin1Char('/'), QString::SkipEmptyParts));

        if (list.count() < 2) {
            qWarning("QSettings::writeEntry: invalid key '%s'", grp_key.latin1());

            return false;
        }

        if (list.count() == 2) {
            d->heading = list[0];
            d->group = QLatin1String("General");
            realkey = list[1];
        } else {
            d->heading = list[0];
            d->group = list[1];

            // remove the heading from the list
            list.removeFirst();
            // remove the group from the list
            list.removeFirst();

            realkey = list.join(QLatin1String("/"));
        }
    } else {
        realkey = grp_key;
    }

    d->writeGroup(realkey, value);
    return true;
}


/*!
  Removes the entry specified by \a key.

    Returns true if the entry existed and was removed; otherwise returns false.

  \sa readEntry(), writeEntry()
*/
bool QSettings::removeEntry(const QString &key)
{
    QString grp_key(groupKey(group(), key));
    if (!qt_verify_key(grp_key)) {
        qWarning("QSettings::removeEntry: Invalid key: '%s'", grp_key.latin1());
        return false;
    }

#if !defined(QWS) && (defined(Q_WS_WIN) || defined(Q_OS_MAC))
    if (d->sysd)
        return d->sysRemoveEntry(grp_key);
#endif

    QString realkey;
    if (grp_key[0] == QLatin1Char('/')) {
        // parse our key
        QStringList list(grp_key.split(QLatin1Char('/'), QString::SkipEmptyParts));

        if (list.count() < 2) {
            qWarning("QSettings::removeEntry: invalid key '%s'", grp_key.latin1());

            return false;
        }

        if (list.count() == 2) {
            d->heading = list[0];
            d->group = QLatin1String("General");
            realkey = list[1];
        } else {
            d->heading = list[0];
            d->group = list[1];

            // remove the heading from the list
            list.removeFirst();
            // remove the group from the list
            list.removeFirst();

            realkey = list.join(QLatin1String("/"));
        }
    } else {
        realkey = grp_key;
    }

    d->removeGroup(realkey);
    return true;
}


/*!
  Returns a list of the keys which contain entries under \a key. Does \e
  not return any keys that contain keys.

    Example settings:
    \code
    /MyCompany/MyApplication/background color
    /MyCompany/MyApplication/foreground color
    /MyCompany/MyApplication/geometry/x
    /MyCompany/MyApplication/geometry/y
    /MyCompany/MyApplication/geometry/width
    /MyCompany/MyApplication/geometry/height
    \endcode
    \code
    QStringList keys = entryList("/MyCompany/MyApplication");
    \endcode
    \c keys contains 'background color' and 'foreground color'. It does
    not contain 'geometry' because this key contains keys not entries.

    To access the geometry values could either use subkeyList() to read
    the keys and then read each entry, or simply read each entry
    directly by specifying its full key, e.g.
    "/MyCompany/MyApplication/geometry/y".

  \sa subkeyList()
*/
QStringList QSettings::entryList(const QString &key) const
{
    QString grp_key(groupKey(group(), key));
    if (!qt_verify_key(grp_key)) {
        qWarning("QSettings::entryList: Invalid key: %s", grp_key.latin1());
        return QStringList();
    }

#if !defined(QWS) && (defined(Q_WS_WIN) || defined(Q_OS_MAC))
    if (d->sysd)
        return d->sysEntryList(grp_key);
#endif

    QString realkey;
    if (grp_key[0] == QLatin1Char('/')) {
        // parse our key
        QStringList list(grp_key.split(QLatin1Char('/'), QString::SkipEmptyParts));

        if (list.count() < 1) {
            qWarning("QSettings::listEntries: invalid key '%s'", grp_key.latin1());

            return QStringList();
        }

        if (list.count() == 1) {
            d->heading = list[0];
            d->group = QLatin1String("General");
        } else {
            d->heading = list[0];
            d->group = list[1];

            // remove the heading from the list
            list.removeFirst();
            // remove the group from the list
            list.removeFirst();

            realkey = list.join(QLatin1String("/"));
        }
    } else
        realkey = grp_key;

    QSettingsGroup grp = d->readGroup();
    QSettingsGroup::Iterator it = grp.begin();
    QStringList ret;
    QString itkey;
    while (it != grp.end()) {
        itkey = it.key();
        ++it;

        if (realkey.length() > 0) {
            if (itkey.left(realkey.length()) != realkey)
                continue;
            else
                itkey.remove(0, realkey.length() + 1);
        }

        if (itkey.indexOf(QLatin1Char('/')) != -1)
            continue;

        ret << itkey;
    }

    return ret;
}


/*!
  Returns a list of the keys which contain keys under \a key. Does \e
  not return any keys that contain entries.

    Example settings:
    \code
    /MyCompany/MyApplication/background color
    /MyCompany/MyApplication/foreground color
    /MyCompany/MyApplication/geometry/x
    /MyCompany/MyApplication/geometry/y
    /MyCompany/MyApplication/geometry/width
    /MyCompany/MyApplication/geometry/height
    /MyCompany/MyApplication/recent files/1
    /MyCompany/MyApplication/recent files/2
    /MyCompany/MyApplication/recent files/3
    \endcode
    \code
    QStringList keys = subkeyList("/MyCompany/MyApplication");
    \endcode
    \c keys contains 'geometry' and 'recent files'. It does not contain
    'background color' or 'foreground color' because they are keys which
    contain entries not keys. To get a list of keys that have values
    rather than subkeys use entryList().

  \sa entryList()
*/
QStringList QSettings::subkeyList(const QString &key) const
{
    QString grp_key(groupKey(group(), key));
    if (!qt_verify_key(grp_key)) {
        qWarning("QSettings::subkeyList: Invalid key: %s", grp_key.latin1());
        return QStringList();
    }

#if !defined(QWS) && (defined(Q_WS_WIN) || defined(Q_OS_MAC))
    if (d->sysd)
        return d->sysSubkeyList(grp_key);
#endif

    QString realkey;
    int subkeycount = 2;
    if (grp_key[0] == QLatin1Char('/')) {
        // parse our key
        QStringList list(grp_key.split(QLatin1Char('/'), QString::SkipEmptyParts));

        if (list.count() < 1) {
            qWarning("QSettings::subkeyList: invalid key '%s'", grp_key.latin1());
            return QStringList();
        }

        subkeycount = list.count();

        if (list.count() == 1) {
            d->heading = list[0];
            d->group = QLatin1String("General");
        } else {
            d->heading = list[0];
            d->group = list[1];

            // remove the heading from the list
            list.removeFirst();
            // remove the group from the list
            list.removeFirst();

            realkey = list.join(QLatin1String("/"));
        }

    } else
        realkey = grp_key;

    QStringList ret;
    if (subkeycount == 1) {
        QMap<QString,QSettingsHeading>::Iterator it = d->headings.begin();
        while (it != d->headings.end()) {
            if (it.key() != QLatin1String("General") && ! ret.contains(it.key()))
                ret << it.key();
            ++it;
        }

        return ret;
    }

    QSettingsGroup grp = d->readGroup();
    QSettingsGroup::Iterator it = grp.begin();
    QString itkey;
    while (it != grp.end()) {
        itkey = it.key();
        ++it;

        if (realkey.length() > 0) {
            if (itkey.left(realkey.length()) != realkey)
                continue;
            else
                itkey.remove(0, realkey.length() + 1);
        }

        int slash = itkey.indexOf(QLatin1Char('/'));
        if (slash == -1)
            continue;
        itkey.truncate(slash);

        if (! ret.contains(itkey))
            ret << itkey;
    }

    return ret;
}


/*!
    \internal

  This function returns the time of last modification for \a key.
*/
QDateTime QSettings::lastModificationTime(const QString &key)
{
    QString grp_key(groupKey(group(), key));
    if (!qt_verify_key(grp_key)) {
        qWarning("QSettings::lastModificationTime: Invalid key '%s'", grp_key.ascii());
        return QDateTime();
    }

#if !defined(QWS) && (defined(Q_WS_WIN) || defined(Q_OS_MAC))
    if (d->sysd)
        return QDateTime();
#endif

    if (grp_key[0] == QLatin1Char('/')) {
        // parse our key
        QStringList list(grp_key.split(QLatin1Char('/'), QString::SkipEmptyParts));

        if (list.count() < 2) {
            qWarning("QSettings::lastModificationTime: Invalid key '%s'", grp_key.latin1());

            return QDateTime();
        }

        if (list.count() == 2) {
            d->heading = list[0];
            d->group = QLatin1String("General");
        } else {
            d->heading = list[0];
            d->group = list[1];
        }
    }

    return d->modificationTime();
}


/*!
    \overload

    Writes the string list entry \a value into key \a key. The \a key
    is created if it doesn't exist. Any previous value is overwritten
    by \a value. The list is stored as a sequence of strings separated
    by \a separator (using QStringList::join()), so none of the
    strings in the list should contain the separator. If the list is
    empty or null the key's value will be an empty string.

    \warning If \a value contains null strings, this function converts
    them to empty strings.  The list returned by readListEntry() will
    not be identical to \a value.  We recommend using the writeEntry()
    and readListEntry() overloads that do not take a \a separator
    argument.

    If an error occurs the settings are left unchanged and false is
    returned; otherwise returns true.

    \sa readListEntry(), readNumEntry(), readDoubleEntry(), readBoolEntry(), removeEntry()
*/
bool QSettings::writeEntry(const QString &key, const QStringList &value,
                           const QChar &separator)
{
    QString s(value.join(QString(separator)));
    return writeEntry(key, s);
}

/*!
    \overload

    Writes the string list entry \a value into key \a key. The \a key
    is created if it doesn't exist. Any previous value is overwritten
    by \a value.

    If an error occurs the settings are left unchanged and false is
    returned; otherwise returns true.

    \sa readListEntry(), readNumEntry(), readDoubleEntry(), readBoolEntry(), removeEntry()
*/
bool QSettings::writeEntry(const QString &key, const QStringList &value)
{
    QString s;
    for (QStringList::ConstIterator it=value.begin(); it!=value.end(); ++it) {
        QString el = *it;
        if (el.isNull()) {
            el = QLatin1String("^0");
        } else {
            el.replace(QLatin1String("^"), QLatin1String("^^"));
        }
        s+=el;
        s+=QLatin1String("^e"); // end of element
    }
    return writeEntry(key, s);
}


/*!
    \fn QStringList QSettings::readListEntry(const QString &key, const QChar &separator, bool *ok) const

    \overload

    Reads the entry specified by \a key as a string. The \a separator
    is used to create a QStringList by calling entry.split(\a
    separator). If \a ok is not 0: \c{*}\a{ok} is set to true
    if the key was read, otherwise \c{*}\a{ok} is set to false.

    \warning If the string list argument to the writeEntry() function
    contains null strings, the return value of this function will not
    be identical to the original data.  All null strings are converted
    to emptry strings by the writeEntry() function.  We recommend
    using the writeEntry() and readListEntry() overloads that do not
    take a \a separator argument.

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QStringList list = mySettings.readListEntry("size", " ");
    QStringList::Iterator it = list.begin();
    while(it != list.end()) {
        myProcessing(*it);
        ++it;
    }
    \endcode

    \sa readEntry(), readDoubleEntry(), readBoolEntry(), writeEntry(), removeEntry()
*/

/*!
    \internal
*/
QStringList QSettings::readListEntry(const QString &key, const QChar &separator, bool *ok)
{
    QString value = readEntry(key, QString::null, ok);
    if ((ok && !*ok) || value.isEmpty())
        return QStringList();

    return value.split(separator);
}

/*!
    \fn QStringList QSettings::readListEntry(const QString &key, bool *ok) const

    Reads the entry specified by \a key as a string. If \a ok is not
    0, \c{*}\a{ok} is set to true if the key was read, otherwise
    \c{*}\a{ok} is set to false.

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QStringList list = mySettings.readListEntry("recentfiles");
    QStringList::Iterator it = list.begin();
    while(it != list.end()) {
        myProcessing(*it);
        ++it;
    }
    \endcode

    \sa readEntry(), readDoubleEntry(), readBoolEntry(), writeEntry(), removeEntry(), QString::split()
*/

/*!
    \internal
*/
QStringList QSettings::readListEntry(const QString &key, bool *ok)
{
    QString value = readEntry(key, QString::null, ok);
    if (ok && !*ok)
        return QStringList();
    QStringList l;
    QString s;
    bool esc=false;
    for (int i=0; i<(int)value.length(); i++) {
        if (esc) {
            if (value[i] == QLatin1Char('e')) { // end-of-string
                l.append(s);
                s= QLatin1String("");
            } else if (value[i] == QLatin1Char('0')) { // null string
                s=QString::null;
            } else {
                s.append(value[i]);
            }
            esc=false;
        } else if (value[i] == QLatin1Char('^')) {
            esc = true;
        } else {
            s.append(value[i]);
            if (i == (int)value.length()-1)
                l.append(s);
        }
    }
    return l;
}

#ifdef Q_OS_MAC
void qt_setSettingsBasePath(const QString &); //qsettings_mac.cpp
#endif

/*!
    Insert platform-dependent paths from platform-independent information.

    The \a domain should be an Internet domain name
    controlled by the producer of the software, eg. Trolltech products
    use "trolltech.com".

    The \a product should be the official name of the product.

    The \a scope should be
    QSettings::User for user-specific settings, or
    QSettings::Global for system-wide settings (generally
    these will be read-only to many users).

    Not all information is relevant on all systems.
*/

void QSettings::setPath(const QString &domain, const QString &product, Scope scope)
{
//    On Windows, any trailing ".com(\..*)" is stripped from the domain. The
//    Global scope corresponds to HKEY_LOCAL_MACHINE, and User corresponds to
//    HKEY_CURRENT_USER. Note that on some installations, not all users can
//    write to the Global scope. On UNIX, any trailing ".com(\..*)" is stripped
//    from the domain. The Global scope corresponds to "/opt" (this would be
//    configurable at library build time - eg. to "/usr/local" or "/usr"),
//    while the User scope corresponds to $HOME/.*rc.
//    Note that on most installations, not all users can write to the System
//    scope.
//
//    On MacOS X, if there is no "." in domain, append ".com", then reverse the
//    order of the elements (Mac OS uses "com.apple.finder" as domain+product).
//    The Global scope corresponds to /Library/Preferences/*.plist, while the
//    User scope corresponds to ~/Library/Preferences/*.plist.
//    Note that on most installations, not all users can write to the System
//    scope.
    d->globalScope = scope == Global;

    QString actualSearchPath;
    int lastDot = domain.lastIndexOf(QLatin1Char('.'));

#if defined(Q_WS_WIN)
    actualSearchPath = "/" + domain.mid(0, lastDot) + "/" + product;
    insertSearchPath(Windows, actualSearchPath);
#elif !defined(QWS) && defined(Q_OS_MAC)
    if(lastDot != -1) {
        QString topLevelDomain = domain.right(domain.length() - lastDot - 1) + ".";
        if (!topLevelDomain.isEmpty())
            qt_setSettingsBasePath(topLevelDomain);
    }
    actualSearchPath = "/" + domain.left(lastDot) + "." + product;
    insertSearchPath(Mac, actualSearchPath);
#else
    if (scope == User)
        actualSearchPath = QDir::homePath() + QLatin1String("/.");
    else
        actualSearchPath = QString::fromLocal8Bit(qInstallPathSysconf()) + QLatin1Char('/');
    actualSearchPath += domain.mid(0, lastDot) + QLatin1Char('/') + product;
    insertSearchPath(Unix, actualSearchPath);
#endif
}

/*!
    Appends \a group to the current key prefix.

    \code
    QSettings settings;
    settings.beginGroup("/MainWindow");
    // read values
    settings.endGroup();
    \endcode
*/
void QSettings::beginGroup(const QString &group)
{
    d->groupStack.push(group);
    d->groupDirty = true;
}

/*!
    Undo previous calls to beginGroup(). Note that a single beginGroup("a/b/c") is undone
    by a single call to endGroup().

    \code
    QSettings settings;
    settings.beginGroup("/MainWindow/Geometry");
    // read values
    settings.endGroup();
    \endcode
*/
void QSettings::endGroup()
{
    if (d->groupStack.isEmpty())
        qWarning("Cannot end more groups than previously started.");
    else
        d->groupStack.pop();
    d->groupDirty = true;
}

/*!
    Set the current key prefix to the empty string.
*/
void QSettings::resetGroup()
{
    d->groupStack.clear();
    d->groupDirty = false;
    d->groupPrefix = QString::null;
}

/*!
    Returns the current key prefix, or a null string if there is no key prefix set.

    \sa beginGroup();
*/
QString QSettings::group() const
{
    if (d->groupDirty) {
        d->groupDirty = false;
        d->groupPrefix = QString::null;

        QStack<QString>::Iterator it = d->groupStack.begin();
        while (it != d->groupStack.end()) {
            QString group = *it;
            ++it;
            if (group[0] != QLatin1Char('/'))
                group.prepend(QLatin1Char('/'));
            d->groupPrefix += group;
        }
    }
    return d->groupPrefix;
}

#endif
