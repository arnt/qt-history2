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

#include "qsettings.h"

#ifndef QT_NO_SETTINGS

#include "qcache.h"
#include "qsettings_p.h"
#include "qfile.h"
#include "qdir.h"
#include "qfileinfo.h"
#include "qrect.h"
#include "qmutex.h"
#include "qlibraryinfo.h"
#include "qtemporaryfile.h"

#ifndef QT_NO_QOBJECT
#include "qcoreapplication.h"

#ifdef Q_OS_WIN // for homedirpath reading from registry
#include "qt_windows.h"
#include "qlibrary.h"

#endif // Q_OS_WIN
#endif // QT_NO_QOBJECT

#include <stdlib.h>

#ifndef CSIDL_COMMON_APPDATA
#define CSIDL_COMMON_APPDATA	0x0023  // All Users\Application Data
#endif

#ifndef CSIDL_APPDATA
#define CSIDL_APPDATA		0x001a	// <username>\Application Data
#endif

// ************************************************************************
// QConfFile

/*
    QConfFile objects are explicitly shared within the application.
    This ensures that modification to the settings done through one
    QSettings object are immediately reflected in other setting
    objects of the same application.
*/

typedef QHash<QString, QConfFile *> ConfFileHash;
typedef QCache<QString, QConfFile> ConfFileCache;

Q_GLOBAL_STATIC(ConfFileHash, usedHashFunc)
Q_GLOBAL_STATIC(ConfFileCache, unusedCacheFunc)
Q_GLOBAL_STATIC(QMutex, mutex)
Q_GLOBAL_STATIC(QString, defaultSystemIniPath)
Q_GLOBAL_STATIC(QString, defaultUserIniPath)

static ConfFileHash *usedHash;
static ConfFileCache *unusedCache;

QConfFile::QConfFile(const QString &fileName)
    : name(fileName), size(0), ref(1)
{
    usedHash->insert(name, this);
#ifdef Q_OS_WIN
    semHandle = 0;
#endif
}

bool QConfFile::mergeKeyMaps()
{
    if (addedKeys.isEmpty() && removedKeys.isEmpty())
        return false;

    SettingsKeyMap::const_iterator i;

    for (i = removedKeys.begin(); i != removedKeys.end(); ++i)
        originalKeys.remove(i.key());
    removedKeys.clear();

    for (i = addedKeys.begin(); i != addedKeys.end(); ++i)
        originalKeys.insert(i.key(), i.value());
    addedKeys.clear();

    return true;
}

QConfFile *QConfFile::fromName(const QString &fileName)
{
    QConfFile *confFile;
    QString absPath = QFileInfo(fileName).absoluteFilePath();

    usedHash = usedHashFunc();
    unusedCache = unusedCacheFunc();

    QMutexLocker locker(mutex());

    if (!(confFile = usedHash->value(absPath))) {
        if ((confFile = unusedCache->take(absPath)))
            usedHash->insert(absPath, confFile);
    }
    if (confFile) {
        confFile->ref.ref();
        return confFile;
    }
    return new QConfFile(absPath);
}

void QConfFile::clearCache()
{
    unusedCache = unusedCacheFunc();

    QMutexLocker locker(mutex());
    unusedCache->clear();
}

// ************************************************************************
// QSettingsPrivate

QSettingsPrivate::QSettingsPrivate()
    : spec(0), fallbacks(true), pendingChanges(false), status(QSettings::NoError)
{
}

QSettingsPrivate::~QSettingsPrivate()
{
}

QString QSettingsPrivate::fileName() const
{
    return QString();
}

QString QSettingsPrivate::actualKey(const QString &key) const
{
    QString n = normalizedKey(key);
    Q_ASSERT_X(!n.isEmpty(), "QSettings", "empty key");
    n.prepend(groupPrefix);
    return n;
}

/*
    Returns a string that never starts nor ends with a slash (or an
    empty string). Examples:

            "foo"            becomes   "foo"
            "/foo//bar///"   becomes   "foo/bar"
            "///"            becomes   ""

    This function is optimized to avoid a QString deep copy in the
    common case where the key is already normalized.
*/
QString QSettingsPrivate::normalizedKey(const QString &key)
{
    QString result = key;

    int i = 0;
    while (i < result.size()) {
        while (result.at(i) == QLatin1Char('/')) {
            result.remove(i, 1);
            if (i == result.size())
                goto after_loop;
        }
        while (result.at(i) != QLatin1Char('/')) {
            ++i;
            if (i == result.size())
                return result;
        }
        ++i; // leave the slash alone
    }

after_loop:
    if (!result.isEmpty())
        result.truncate(i - 1); // remove the trailing slash
    return result;
}

// see also qsettings_win.cpp and qsettings_mac.cpp

#if !defined(Q_OS_WIN) && !defined(Q_OS_MAC)
QSettingsPrivate *QSettingsPrivate::create(QSettings::Format format,
                                           QSettings::Scope scope,
                                           const QString &organization,
                                           const QString &application)
{
    QConfFileSettingsPrivate *p = new QConfFileSettingsPrivate(format, scope,
                                                               organization, application);
    p->init();
    return p;
}
#endif

#if !defined(Q_OS_WIN)
QSettingsPrivate *QSettingsPrivate::create(const QString &fileName, QSettings::Format format)
{
    QConfFileSettingsPrivate *p = new QConfFileSettingsPrivate(fileName, format);
    p->init();
    return p;
}
#endif

void QSettingsPrivate::processChild(QString key, ChildSpec spec, QMap<QString, QString> &result)
{
    if (spec != AllKeys) {
        int slashPos = key.indexOf(QLatin1Char('/'));
        if (slashPos == -1) {
            if (spec != ChildKeys)
                return;
        } else {
            if (spec != ChildGroups)
                return;
            key.truncate(slashPos);
        }
    }
    result.insert(key, QString());
}

void QSettingsPrivate::beginGroupOrArray(const QSettingsGroup &group)
{
    groupStack.push(group);
    if (!group.name().isEmpty()) {
        groupPrefix += group.name();
        groupPrefix += QLatin1Char('/');
    }
}

/*
    We only set an error if there isn't one set already. This way the user always gets the
    first error that occurred. We always allow clearing errors.
*/

void QSettingsPrivate::setStatus(QSettings::Status status)
{
    if (status == QSettings::NoError || this->status == QSettings::NoError)
        this->status = status;
}

void QSettingsPrivate::update()
{
    flush();
    pendingChanges = false;
}

void QSettingsPrivate::requestUpdate()
{
    if (!pendingChanges) {
        pendingChanges = true;
#ifndef QT_NO_QOBJECT
        Q_Q(QSettings);
        QCoreApplication::postEvent(q, new QEvent(QEvent::UpdateRequest));
#else
        update();
#endif
    }
}

QStringList QSettingsPrivate::variantListToStringList(const QVariantList &l) const
{
    QStringList result;
    QVariantList::const_iterator it = l.constBegin();
    for (; it != l.constEnd(); ++it)
        result.append(variantToString(*it));
    return result;
}

QVariant QSettingsPrivate::stringListToVariantList(const QStringList &l) const
{
    QVariantList variantList;
    bool foundNonStringItem = false;
    bool foundEscapedStringItem = false;

    QStringList::const_iterator it = l.constBegin();
    for (; it != l.constEnd(); ++it) {
        QVariant variant = stringToVariant(*it);
        variantList.append(variant);

        if (variant.type() != QVariant::String)
            foundNonStringItem = true;
        else if (variant.toString() != *it)
            foundEscapedStringItem = true;
    }

    if (foundNonStringItem) {
        return variantList;
    } else if (foundEscapedStringItem) {
        return QVariant(variantList).toStringList();
    } else {
        return l;
    }
}

QString &QSettingsPrivate::escapedLeadingAt(QString &s)
{
    if (s.length() > 0 && s.at(0) == QLatin1Char('@'))
        s.prepend(QLatin1Char('@'));
    return s;
}

QString &QSettingsPrivate::unescapedLeadingAt(QString &s)
{
    if (s.startsWith(QLatin1String("@@")))
        s.remove(0, 1);
    return s;
}

QString QSettingsPrivate::variantToString(const QVariant &v)
{
    QString result;

    switch (v.type()) {
        case QVariant::Invalid:
            result = QLatin1String("@Invalid()");
            break;

        case QVariant::ByteArray: {
            QByteArray a = v.toByteArray();
            result = QLatin1String("@ByteArray(");
            result += QString::fromLatin1(a.constData(), a.size());
            result += QLatin1Char(')');
            break;
        }

        case QVariant::String:
        case QVariant::LongLong:
        case QVariant::ULongLong:
        case QVariant::Int:
        case QVariant::UInt:
        case QVariant::Bool:
        case QVariant::Double: {
            result = v.toString();
            result = escapedLeadingAt(result);
            break;
        }
        case QVariant::Rect: {
            QRect r = qvariant_cast<QRect>(v);
            result += QLatin1String("@Rect(");
            result += QString::number(r.x());
            result += QLatin1Char(' ');
            result += QString::number(r.y());
            result += QLatin1Char(' ');
            result += QString::number(r.width());
            result += QLatin1Char(' ');
            result += QString::number(r.height());
            result += QLatin1Char(')');
            break;
        }
        case QVariant::Size: {
            QSize s = qvariant_cast<QSize>(v);
            result += QLatin1String("@Size(");
            result += QString::number(s.width());
            result += QLatin1Char(' ');
            result += QString::number(s.height());
            result += QLatin1Char(')');
            break;
        }
        case QVariant::Point: {
            QPoint p = qvariant_cast<QPoint>(v);
            result += QLatin1String("@Point(");
            result += QString::number(p.x());
            result += QLatin1Char(' ');
            result += QString::number(p.y());
            result += QLatin1Char(')');
            break;
        }

        default: {
#ifndef QT_NO_DATASTREAM
            QByteArray a;
            {
                QDataStream s(&a, QIODevice::WriteOnly);
                s << v;
            }

            result = QLatin1String("@Variant(");
            result += QString::fromLatin1(a.constData(), a.size());
            result += QLatin1Char(')');
#else
            Q_ASSERT("QSettings: Cannot save custom types without QDataStream support");
#endif
            break;
        }
    }

    return result;
}

QVariant QSettingsPrivate::stringToVariant(const QString &s)
{
    if (s.length() > 3
            && s.at(0) == QLatin1Char('@')
            && s.at(s.length() - 1) == QLatin1Char(')')) {

        if (s.startsWith(QLatin1String("@ByteArray("))) {
            return QVariant(s.toLatin1().mid(11, s.size() - 12));
        } else if (s.startsWith(QLatin1String("@Variant("))) {
#ifndef QT_NO_DATASTREAM
            QByteArray a(s.toLatin1().mid(9));
            QDataStream stream(&a, QIODevice::ReadOnly);
            QVariant result;
            stream >> result;
            return result;
#else
            Q_ASSERT("QSettings: Cannot load custom types without QDataStream support");
#endif
#ifndef QT_NO_GEOM_VARIANT
        } else if (s.startsWith(QLatin1String("@Rect("))) {
            QStringList args = QSettingsPrivate::splitArgs(s, 5);
            if (args.size() == 4) {
                return QVariant(QRect(args[0].toInt(), args[1].toInt(), args[2].toInt(), args[3].toInt()));
            }
        } else if (s.startsWith(QLatin1String("@Size("))) {
            QStringList args = QSettingsPrivate::splitArgs(s, 5);
            if (args.size() == 2) {
                return QVariant(QSize(args[0].toInt(), args[1].toInt()));
            }
        } else if (s.startsWith(QLatin1String("@Point("))) {
            QStringList args = QSettingsPrivate::splitArgs(s, 6);
            if (args.size() == 2) {
                return QVariant(QPoint(args[0].toInt(), args[1].toInt()));
            }
#endif
        } else if (s == QLatin1String("@Invalid()")) {
            return QVariant();
        }
    }

    QString tmp = s;
    return QVariant(unescapedLeadingAt(tmp));
}

static const char hexDigits[] = "0123456789ABCDEF";

void QSettingsPrivate::iniEscapedKey(const QString &key, QByteArray &result)
{
    for (int i = 0; i < key.size(); ++i) {
        uint ch = key.at(i).unicode();

        if (ch == '/') {
            result += '\\';
        } else if (ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z' || ch >= '0' && ch <= '9'
                || ch == '_' || ch == '-' || ch == '.') {
            result += (char)ch;
        } else if (ch <= 0xFF) {
            result += '%';
            result += hexDigits[ch / 16];
            result += hexDigits[ch % 16];
        } else {
            result += "%U";
            QByteArray hexCode;
            for (int i = 0; i < 4; ++i) {
                hexCode.prepend(hexDigits[ch % 16]);
                ch >>= 4;
            }
            result += hexCode;
        }
    }
}

bool QSettingsPrivate::iniUnescapedKey(const QByteArray &key, int from, int to, QString &result)
{
    bool lowerCaseOnly = true;
    int i = from;
    while (i < to) {
        int ch = (uchar)key.at(i);

        if (ch == '\\') {
            result += QLatin1Char('/');
            ++i;
            continue;
        }

        if (ch != '%' || i == to - 1) {
            if (isupper((uchar)ch))
                lowerCaseOnly = false;
            result += QLatin1Char(ch);
            ++i;
            continue;
        }

        int numDigits = 2;
        int firstDigitPos = i + 1;

        ch = key.at(i + 1);
        if (ch == 'U') {
            ++firstDigitPos;
            numDigits = 4;
        }

        if (firstDigitPos + numDigits > to) {
            result += QLatin1Char('%');
            ++i;
            continue;
        }

        bool ok;
        ch = key.mid(firstDigitPos, numDigits).toInt(&ok, 16);
        if (!ok) {
            result += QLatin1Char('%');
            ++i;
            continue;
        }

        QChar qch(ch);
        if (qch.toLower() != qch)
            lowerCaseOnly = false;
        result += qch;
        i = firstDigitPos + numDigits;
    }
    return lowerCaseOnly;
}

void QSettingsPrivate::iniEscapedString(const QString &str, QByteArray &result)
{
    bool needsQuotes = false;
    bool escapeNextIfDigit = false;
    int i;
    int startPos = result.size();

    for (i = 0; i < str.size(); ++i) {
        uint ch = str.at(i).unicode();
        if (ch == ';' || ch == ',' || ch == '=')
            needsQuotes = true;

        if (escapeNextIfDigit
                && ((ch >= '0' && ch <= '9')
                    || (ch >= 'a' && ch <= 'f')
                    || (ch >= 'A' && ch <= 'F'))) {
            result += "\\x";
            result += QByteArray::number(ch, 16);
            continue;
        }

        escapeNextIfDigit = false;

        switch (ch) {
        case '\0':
            result += "\\0";
            break;
        case '\a':
            result += "\\a";
            break;
        case '\b':
            result += "\\b";
            break;
        case '\f':
            result += "\\f";
            break;
        case '\n':
            result += "\\n";
            break;
        case '\r':
            result += "\\r";
            break;
        case '\t':
            result += "\\t";
            break;
        case '\v':
            result += "\\v";
            break;
        case '"':
        case '\\':
            result += '\\';
            result += (char)ch;
            break;
        default:
            if (ch <= 0x1F || ch >= 0x7F) {
                result += "\\x";
                result += QByteArray::number(ch, 16);
                escapeNextIfDigit = true;
            } else {
                result += (char)ch;
            }
        }
    }

    if (needsQuotes
            || (startPos < result.size() && (result.at(startPos) == ' '
                                                || result.at(result.size() - 1) == ' '))) {
        result.insert(startPos, '"');
        result += '"';
    }
}

void QSettingsPrivate::iniChopTrailingSpaces(QString *str)
{
    int n = str->size();
    while (n > 0
            && (str->at(n - 1) == QLatin1Char(' ') || str->at(n - 1) == QLatin1Char('\t')))
        str->truncate(--n);
}

void QSettingsPrivate::iniEscapedStringList(const QStringList &strs, QByteArray &result)
{
    for (int i = 0; i < strs.size(); ++i) {
        if (i != 0)
            result += ", ";
        iniEscapedString(strs.at(i), result);
    }
}

QStringList *QSettingsPrivate::iniUnescapedStringList(const QByteArray &str, int from, int to,
                                                      QString &result)
{
    static const char escapeCodes[][2] =
    {
        { 'a', '\a' },
        { 'b', '\b' },
        { 'f', '\f' },
        { 'n', '\n' },
        { 'r', '\r' },
        { 't', '\t' },
        { 'v', '\v' },
        { '"', '"' },
        { '?', '?' },
        { '\'', '\'' },
        { '\\', '\\' }
    };
    static const int numEscapeCodes = sizeof(escapeCodes) / sizeof(escapeCodes[0]);

    QStringList *strList = 0;
    int i = from;

    enum State { StNormal, StSkipSpaces, StEscape, StHexEscapeFirstChar, StHexEscape,
                    StOctEscape };
    State state = StSkipSpaces;
    int escapeVal = 0;
    bool inQuotedString = false;
    bool currentValueIsQuoted = false;

    while (i < to) {
        char ch = str.at(i);

        switch (state) {
        case StNormal:
            switch (ch) {
            case '\\':
                state = StEscape;
                break;
            case '"':
                currentValueIsQuoted = true;
                inQuotedString = !inQuotedString;
                if (!inQuotedString)
                    state = StSkipSpaces;
                break;
            case ',':
                if (!inQuotedString) {
                    if (!currentValueIsQuoted)
                        iniChopTrailingSpaces(&result);
                    if (!strList)
                        strList = new QStringList;
                    strList->append(result);
                    result.clear();
                    currentValueIsQuoted = false;
                    state = StSkipSpaces;
                    break;
                }
                /* fallthrough */
            default:
                result += QLatin1Char(ch);
            }
            ++i;
            break;
        case StSkipSpaces:
            if (ch == ' ' || ch == '\t')
                ++i;
            else
                state = StNormal;
            break;
        case StEscape:
            for (int j = 0; j < numEscapeCodes; ++j) {
                if (ch == escapeCodes[j][0]) {
                    result += QLatin1Char(escapeCodes[j][1]);
                    ++i;
                    state = StNormal;
                    goto end_of_switch;
                }
            }

            if (ch == 'x') {
                escapeVal = 0;
                state = StHexEscapeFirstChar;
            } else if (ch >= '0' && ch <= '7') {
                escapeVal = ch - '0';
                state = StOctEscape;
            } else {
                state = StNormal;
            }
            ++i;
            break;
        case StHexEscapeFirstChar:
            if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F')
                    || (ch >= 'a' && ch <= 'f'))
                state = StHexEscape;
            else
                state = StNormal;
            break;
        case StHexEscape:
            if (ch >= 'a')
                ch -= 'a' - 'A';
            if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F')) {
                escapeVal <<= 4;
                escapeVal += strchr(hexDigits, ch) - hexDigits;
                ++i;
            } else {
                result += QChar(escapeVal);
                state = StNormal;
            }
            break;
        case StOctEscape:
            if (ch >= '0' && ch <= '7') {
                escapeVal <<= 3;
                escapeVal += ch - '0';
                ++i;
            } else {
                result += QChar(escapeVal);
                state = StNormal;
            }
        }
end_of_switch:
        ;
    }

    if (state == StHexEscape || state == StOctEscape)
        result += QChar(escapeVal);
    if (!currentValueIsQuoted)
        iniChopTrailingSpaces(&result);
    if (strList)
        strList->append(result);
    return strList;
}

QStringList QSettingsPrivate::splitArgs(const QString &s, int idx)
{
    int l = s.length();
    Q_ASSERT(l > 0);
    Q_ASSERT(s.at(idx) == '(');
    Q_ASSERT(s.at(l - 1) == ')');

    QStringList result;
    QString item;

    for (++idx; idx < l; ++idx) {
        QChar c = s.at(idx);
        if (c == QLatin1Char(')')) {
            Q_ASSERT(idx == l - 1);
            result.append(item);
        } else if (c == QLatin1Char(' ')) {
            result.append(item);
            item.clear();
        } else {
            item.append(c);
        }
    }

    return result;
}

// ************************************************************************
// QConfFileSettingsPrivate

static void checkAccess(const QString &name, bool *read, bool *write)
{
    QFileInfo fileInfo(name);

    if (fileInfo.exists()) {
        /*
            The best way to check that an existing file is
            writable is to open it for writing.
        */
        QFile file(name);
        *read = file.open(QIODevice::ReadOnly);
        file.close();

        *write = file.open(QIODevice::Append);
    } else {
        // files that don't exist are considered readable
        *read = true;

        QDir dir;
        if (QDir::isRelativePath(name))
            dir = QDir::current();
        else
            dir = QDir::root();

        /*
            Create the directories to the file.
        */
        QStringList pathElements = name.split(QLatin1Char('/'), QString::SkipEmptyParts);
        for (int i = 0; i < pathElements.size() - 1; ++i) {
            QString elt = pathElements[i];
            if (dir.cd(elt))
                continue;

            if (!dir.mkdir(elt) || !dir.cd(elt))
                break;
        }

        /*
            The best way to check if we can create the file is to
            try to create a temporary file.
        */
        QTemporaryFile file(name + QLatin1String(".XXXXXX"));
        *write = file.open();
    }
}

void QConfFileSettingsPrivate::init()
{
    if (confFiles[spec] == 0) {
        readAccess = false;
        writeAccess = false;
    } else {
        checkAccess(confFiles[spec]->name, &readAccess, &writeAccess);
    }
    if (!readAccess)
        setStatus(QSettings::AccessError);

    cs = Qt::CaseInsensitive;
    sync();       // loads the files the first time
}

#ifdef Q_OS_WIN
static QString windowsConfigPath(int type)
{
    QString result;

#ifndef QT_NO_QOBJECT
    // We can't use QLibrary if there is QT_NO_QOBJECT is defined
    // This only happens when bootstrapping qmake.
    QLibrary library("shell32");
    QT_WA( {
        typedef BOOL (WINAPI*GetSpecialFolderPath)(HWND, LPTSTR, int, BOOL);
        GetSpecialFolderPath SHGetSpecialFolderPath = (GetSpecialFolderPath)library.resolve("SHGetSpecialFolderPathW");
        if (SHGetSpecialFolderPath) {
            TCHAR path[MAX_PATH];
            SHGetSpecialFolderPath(0, path, type, FALSE);
            result = QString::fromUtf16((ushort*)path);
        }
    } , {
        typedef BOOL (WINAPI*GetSpecialFolderPath)(HWND, char*, int, BOOL);
        GetSpecialFolderPath SHGetSpecialFolderPath = (GetSpecialFolderPath)library.resolve("SHGetSpecialFolderPathA");
        if (SHGetSpecialFolderPath) {
            char path[MAX_PATH];
            SHGetSpecialFolderPath(0, path, type, FALSE);
            result = QString::fromLocal8Bit(path);
        }
    } );
#endif // QT_NO_QOBJECT

    if (result.isEmpty()) {
        switch (type) {
            case CSIDL_COMMON_APPDATA:
                result = QLatin1String("C:\\temp\\qt-common");
                break;
            case CSIDL_APPDATA:
                result = QLatin1String("C:\\temp\\qt-user");
                break;
            default:
                break;
        }
    }

    return result;
}
#endif // Q_OS_WIN

static QString systemIniPath()
{
    QString result;

    result = *defaultSystemIniPath();
    if (result.isEmpty()) {
#ifdef Q_OS_WIN
        result = windowsConfigPath(CSIDL_COMMON_APPDATA);
#else
        result = QLibraryInfo::location(QLibraryInfo::SettingsPath);
#endif // Q_OS_WIN
    }

    return result + QDir::separator();
}

static QString userIniPath()
{
    QString result;

    result = *defaultUserIniPath();
    if (result.isEmpty()) {
#ifdef Q_OS_WIN
        result = windowsConfigPath(CSIDL_APPDATA);
#else
        char *env = getenv("XDG_CONFIG_HOME");
        if (env == 0)
            result = QDir::homePath() + QDir::separator() + QLatin1String(".config");
        else if (*env == '/')
            result = QLatin1String(env);
        else
            result = QDir::homePath() + QDir::separator() + QLatin1String(env);
#endif // Q_OS_WIN
    }

    return result + QDir::separator();
}

QConfFileSettingsPrivate::QConfFileSettingsPrivate(QSettings::Format format,
                                                   QSettings::Scope scope,
                                                   const QString &organization,
                                                   const QString &application)
{
    int i;
    this->format = format;
    for (i = 0; i < NumConfFiles; ++i)
        confFiles[i] = 0;
    cs = Qt::CaseSensitive;

    QString org = organization;
    if (org.isEmpty()) {
        setStatus(QSettings::AccessError);
        org = QLatin1String("unknown-organization.trolltech.com");
    }

    const char *extension = format == QSettings::IniFormat ? ".ini" : ".conf";

    QString appFile = org + QDir::separator() + application + QLatin1String(extension);
    QString orgFile = org + QLatin1String(extension);

    if (scope == QSettings::UserScope) {
        if (!application.isEmpty())
            confFiles[F_User | F_Application] = QConfFile::fromName(userIniPath() + appFile);
        confFiles[F_User | F_Organization] = QConfFile::fromName(userIniPath() + orgFile);
    }
    if (!application.isEmpty())
        confFiles[F_Global | F_Application] = QConfFile::fromName(systemIniPath() + appFile);
    confFiles[F_Global | F_Organization] = QConfFile::fromName(systemIniPath() + orgFile);

    for (i = 0; i < NumConfFiles; ++i) {
        if (confFiles[i]) {
            spec = i;
            break;
        }
    }
}

QConfFileSettingsPrivate::QConfFileSettingsPrivate(const QString &fileName,
                                                   QSettings::Format format)
{
    confFiles[0] = QConfFile::fromName(fileName);
    for (int i = 1; i < NumConfFiles; ++i)
        confFiles[i] = 0;
    this->format = format;
    cs = Qt::CaseSensitive;
}

QConfFileSettingsPrivate::~QConfFileSettingsPrivate()
{
    QMutexLocker locker(mutex());

    for (int i = 0; i < NumConfFiles; ++i) {
        if (confFiles[i] && !confFiles[i]->ref.deref()) {
            usedHash->remove(confFiles[i]->name);

            if (confFiles[i]->size == 0) {
                delete confFiles[i];
            } else {
                unusedCache->insert(confFiles[i]->name, confFiles[i],
                                      10 + (confFiles[i]->originalKeys.size() / 4));
            }
        }
    }
}

void QConfFileSettingsPrivate::remove(const QString &key)
{
    if (!writeAccess) {
        setStatus(QSettings::AccessError);
        return;
    }

    QSettingsKey theKey(key, cs);
    QSettingsKey prefix(key + QLatin1Char('/'), cs);

    QConfFile *confFile = confFiles[spec];
    QMutexLocker locker(mutex());

    SettingsKeyMap::iterator i = confFile->addedKeys.lowerBound(prefix);
    while (i != confFile->addedKeys.end() && i.key().startsWith(prefix))
        i = confFile->addedKeys.erase(i);
    confFile->addedKeys.remove(theKey);

    SettingsKeyMap::const_iterator j = confFile->originalKeys.lowerBound(prefix);
    while (j != confFile->originalKeys.constEnd() && j.key().startsWith(prefix)) {
        confFile->removedKeys.insert(j.key(), QVariant());
        ++j;
    }
    if (confFile->originalKeys.contains(theKey))
        confFile->removedKeys.insert(theKey, QVariant());
}

void QConfFileSettingsPrivate::set(const QString &key, const QVariant &value)
{
    if (!writeAccess) {
        setStatus(QSettings::AccessError);
        return;
    }

    QSettingsKey theKey(key, cs);

    QConfFile *confFile = confFiles[spec];
    QMutexLocker locker(mutex());
    confFile->removedKeys.remove(theKey);
    confFile->addedKeys.insert(theKey, value);
}

bool QConfFileSettingsPrivate::get(const QString &key, QVariant *value) const
{
    QSettingsKey theKey(key, cs);
    SettingsKeyMap::const_iterator j;
    bool found = false;

    QMutexLocker locker(mutex());

    for (int i = 0; i < NumConfFiles; ++i) {
        if (QConfFile *confFile = confFiles[i]) {
            if (!confFile->addedKeys.isEmpty()) {
                j = confFile->addedKeys.find(theKey);
                found = (j != confFile->addedKeys.constEnd());
            }
            if (!found) {
                j = confFile->originalKeys.find(theKey);
                found = (j != confFile->originalKeys.constEnd()
                         && !confFile->removedKeys.contains(theKey));
            }

            if (found && value)
                *value = *j;

            if (found)
                return true;
            if (!fallbacks)
                break;
        }
    }
    return false;
}

QStringList QConfFileSettingsPrivate::children(const QString &prefix, ChildSpec spec) const
{
    QMap<QString, QString> result;
    SettingsKeyMap::const_iterator j;

    QSettingsKey thePrefix(prefix, cs);
    int startPos = prefix.size();

    QMutexLocker locker(mutex());

    for (int i = 0; i < NumConfFiles; ++i) {
        if (QConfFile *confFile = confFiles[i]) {
            j = confFile->originalKeys.lowerBound(thePrefix);
            while (j != confFile->originalKeys.constEnd() && j.key().startsWith(thePrefix)) {
                if (!confFile->removedKeys.contains(j.key()))
                    processChild(j.key().realKey().mid(startPos), spec, result);
                ++j;
            }

            j = confFile->addedKeys.lowerBound(thePrefix);
            while (j != confFile->addedKeys.constEnd() && j.key().startsWith(thePrefix)) {
                processChild(j.key().realKey().mid(startPos), spec, result);
                ++j;
            }

            if (!fallbacks)
                break;
        }
    }
    return result.keys();
}

void QConfFileSettingsPrivate::clear()
{
    if (!writeAccess) {
        setStatus(QSettings::AccessError);
        return;
    }

    QConfFile *confFile = confFiles[spec];
    QMutexLocker locker(mutex());
    confFile->addedKeys.clear();
    confFile->removedKeys = confFiles[spec]->originalKeys;
}

void QConfFileSettingsPrivate::sync()
{
    QMutexLocker locker(mutex());

    // people probably won't be checking the status a whole lot, so in case of
    // error we just try to go on and make the best of it

    for (int i = 0; i < NumConfFiles; ++i) {
        QConfFile *confFile = confFiles[i];
        if (confFile) {
            if (!readFile(confFile)) {
                // Only problems with the file we actually write to change the status. The
                // other files are just optional fallbacks.
                if (i == spec)
                    setStatus(QSettings::FormatError);
            }
            if (i == spec && confFile->mergeKeyMaps()) {
                if (!writeFile(confFile))
                    setStatus(QSettings::AccessError);
            }
        }
    }
}

void QConfFileSettingsPrivate::flush()
{
    sync();
}

QString QConfFileSettingsPrivate::fileName() const
{
    QConfFile *confFile = confFiles[spec];
    if (confFile == 0)
        return QString();
    return confFile->name;
}

bool QConfFileSettingsPrivate::isWritable() const
{
    return writeAccess;
}

/*
    The following openFile() and closeFile() functions lock the file
    (using fcntl() on Unix and a global mutex on Windows), ensuring
    that if two instances of the same applications access the file at
    the same time, the file isn't corrupted.
*/

#ifdef Q_OS_UNIX
const int ReadFlags = O_RDONLY | O_CREAT;
const int WriteFlags = O_WRONLY | O_CREAT | O_APPEND;
#else
static const int FileLockSemMax = 50;
static const int ReadFlags = 1;
static const int WriteFlags = 2;
const char SemNamePrefix[] = "QSettings semaphore ";
#endif

static bool openFile(QFile &file, QConfFile &confFile, int flags)
{
#ifdef Q_OS_UNIX
    Q_UNUSED(confFile);
    int fd = QT_OPEN(QFile::encodeName(file.fileName()), flags, S_IRUSR | S_IWUSR);
    if (fd < 0)
        return false;

    struct flock fl;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_type = (flags == WriteFlags) ? F_WRLCK : F_RDLCK;
    fcntl(fd, F_SETLKW, &fl);

    if (flags == WriteFlags)
        QT_FTRUNCATE(fd, 0);

    return file.open(flags == WriteFlags ? QIODevice::WriteOnly | QIODevice::Text
                     : QIODevice::OpenMode(QIODevice::ReadOnly),
                     fd);
#else
    // on Windows we use a named semaphore
    if (confFile.semHandle == 0) {
        QString semName = QString::fromAscii(SemNamePrefix);
        semName.append(file.fileName());
        QT_WA( {
            confFile.semHandle = CreateSemaphoreW(0, FileLockSemMax, FileLockSemMax, reinterpret_cast<const wchar_t *>(semName.utf16()));
        } , {
            confFile.semHandle = CreateSemaphoreA(0, FileLockSemMax, FileLockSemMax, semName.toLocal8Bit());
        } );
    }

    if (confFile.semHandle != 0) {
        int decrement;
        if (flags == ReadFlags)
            decrement = 1;
        else
            decrement = FileLockSemMax;

        for (int i = 0; i < decrement; ++i)
            WaitForSingleObject(confFile.semHandle, INFINITE);
    }

    return file.open(flags == WriteFlags ? QIODevice::WriteOnly | QIODevice::Text
                                         : QIODevice::OpenMode(QIODevice::ReadOnly));
#endif
}

static void closeFile(QFile &file, QConfFile &confFile)
{
#ifdef Q_OS_UNIX
    Q_UNUSED(confFile);
    int fd = file.handle();
    file.close();
    QT_CLOSE(fd);
#else
    int increment;
    if (file.openMode() & QIODevice::ReadOnly)
        increment = 1;
    else
        increment = FileLockSemMax;

    ReleaseSemaphore(confFile.semHandle, increment, 0);
    CloseHandle(confFile.semHandle);
    confFile.semHandle = 0;

    file.close();
#endif
}

/*
    This only returns false on format errors. Files which don't exist, or which
    we don't have read permission for, are treated as empty sets of keys.
*/
bool QConfFileSettingsPrivate::readFile(QConfFile *confFile)
{
    QFileInfo fileInfo(confFile->name);
    int actualSize = fileInfo.size();
    QDateTime actualTimeStamp = fileInfo.lastModified();

    if (confFile->size == actualSize) {
        // no need to reload the file if the timestamps and file sizes match
        if (confFile->timeStamp == actualTimeStamp)
            return true;
    }

    SettingsKeyMap newKeys;

    bool ok = true; // we treat unexisting/unreadable files the same as empty files

    if (actualSize != 0) {
#ifdef Q_OS_MAC
        if (format == QSettings::NativeFormat) {
            ok = readPlistFile(confFile->name, &newKeys);
        } else
#endif
        {
            QFile file(confFile->name);
            if (openFile(file, *confFile, ReadFlags)) {
                ok = readIniFile(file, &newKeys);
                closeFile(file, *confFile);
            }
        }
    }

    confFile->originalKeys = newKeys;
    confFile->size = actualSize;
    confFile->timeStamp = actualTimeStamp;

    return ok;
}

bool QConfFileSettingsPrivate::writeFile(QConfFile *confFile)
{
    bool ok = false;

#ifdef Q_OS_MAC
    if (format == QSettings::NativeFormat) {
        ok = writePlistFile(confFile->name, confFile->originalKeys);
    } else
#endif
    {
        QFile file(confFile->name);
        if (openFile(file, *confFile, WriteFlags)) {
            ok = writeIniFile(file, confFile->originalKeys);
            closeFile(file, *confFile);
        } else {
            return false;
        }
    }

    QFileInfo fileInfo(confFile->name);
    confFile->size = fileInfo.size();
    confFile->timeStamp = fileInfo.lastModified();

    return ok;
}

bool QConfFileSettingsPrivate::readIniLine(QIODevice &device, QByteArray &line, int &len,
                                           int &equalsCharPos)
{
#define MAYBE_GROW() \
    if (pos + 4 > line.size()) { \
        line.resize(pos << 1); \
        data = line.data(); \
    }

    char *data = line.data();
    char ch, ch2;
    int pos = 0;

    equalsCharPos = -1;

    while (device.getChar(&ch)) {
    process_ch:
        MAYBE_GROW();

        switch (ch) {
        case '"':
            data[pos++] = '"';
            while (!device.getChar(&ch) || ch != '"') {
                MAYBE_GROW();

                if (static_cast<signed char>(ch) == -1)
                    goto end;

                if (ch == '\\') {
                    data[pos++] = '\\';
                    if (!device.getChar(&ch))
                        goto end;
                }
                data[pos++] = ch;
            }
            data[pos++] = '"';
            break;
        case ' ':
        case '\t':
            if (pos > 0)
                data[pos++] = ch;
            break;
        case '\n':
        case '\r':
        process_newline:
            /*
                According to the specs, a line ends with CF, LF,
                CR+LF, or LF+CR. In practice, this is irrelevant and
                the ungetch() call is expensive, so let's not do it.
            */
#if 0
            if (!device.getChar(&ch2))
                goto end;
            if ((ch2 != '\n' && ch2 != '\r') || ch == ch2)
                device.ungetChar(ch2);
#endif
            if (pos > 0)
                goto end;
            break;
        case '\\':
            if (!device.getChar(&ch))
                goto end;

            if (ch == '\n' || ch == '\r') {
                if (!device.getChar(&ch2)) {
                    if ((ch2 != '\n' && ch2 != '\r') || ch == ch2) {
                        ch = ch2;
                        goto process_ch;
                    }
                }
            } else {
                data[pos++] = '\\';
                data[pos++] = ch;
            }
            break;
        case ';':
            while (device.getChar(&ch)) {
                if (ch == '\n' || ch == '\r')
                    goto process_newline;
            }
            break;
        case '=':
            if (equalsCharPos == -1) {
                while (pos > 0 && (ch = data[pos - 1]) == ' ' || ch == '\t')
                    --pos;
                equalsCharPos = pos;
            }
            data[pos++] = '=';
            break;
        default:
            data[pos++] = ch;
        }
    }

end:
    data[pos] = '\0';
    len = pos;
    return pos > 0;
}

/*
    Returns false on parse error. However, as many keys are read as
    possible, so if the user doesn't check the status he will get the
    most out of the file anyway.
*/
bool QConfFileSettingsPrivate::readIniFile(QIODevice &device, SettingsKeyMap *map)
{
    QString currentSection;
    bool currentSectionIsLowerCase = true;
    QByteArray line;
    line.resize(512);
    int equalsCharPos;
    int len;

    bool ok = true;

    while (readIniLine(device, line, len, equalsCharPos)) {
        if (line.at(0) == '[') {
            // this is a section
            QByteArray iniSection;
            int idx = line.indexOf(']');
            if (idx == -1) {
                ok = false;
                iniSection = line.mid(1);
            } else {
                iniSection = line.mid(1, idx - 1);
            }

            iniSection = iniSection.trimmed();

            if (qstricmp(iniSection, "general") == 0) {
                currentSection.clear();
            } else if (qstricmp(iniSection, "%general") == 0) {
                currentSection = QLatin1String("general");
                currentSection += QLatin1Char('/');
            } else {
                currentSection.clear();
                currentSectionIsLowerCase = iniUnescapedKey(iniSection, 0, iniSection.size(),
                                                            currentSection);
                currentSection += QLatin1Char('/');
            }
        } else {
            if (equalsCharPos < 1) {
                ok = false;
                continue;
            }

            QString key = currentSection;
            bool keyIsLowerCase = (iniUnescapedKey(line, 0, equalsCharPos, key)
                                   && currentSectionIsLowerCase);

            QString strValue;
            strValue.reserve(len - equalsCharPos);
            QStringList *strListValue = iniUnescapedStringList(line, equalsCharPos + 1, len,
                                                               strValue);
            QVariant variant;
            if (strListValue) {
                variant = stringListToVariantList(*strListValue);
                delete strListValue;
            } else {
                variant = stringToVariant(strValue);
            }

            /*
                We try to avoid the expensive toLower() call in
                QSettingsKey by passing Qt::CaseSensitive when the
                key is already in lower-case.
            */
            map->insert(QSettingsKey(key, keyIsLowerCase ? Qt::CaseSensitive : Qt::CaseInsensitive),
                        variant);
        }
    }

    return ok;
}

bool QConfFileSettingsPrivate::writeIniFile(QIODevice &device, const SettingsKeyMap &map)
{
    typedef QMap<QString, QVariantMap> IniMap;
    IniMap iniMap;
    IniMap::const_iterator i;

    for (SettingsKeyMap::const_iterator j = map.constBegin(); j != map.constEnd(); ++j) {
        QString section;
        QString key = j.key().realKey();
        int slashPos;

        if ((slashPos = key.indexOf(QLatin1Char('/'))) != -1) {
            section = key.left(slashPos);
            key.remove(0, slashPos + 1);
        }
        iniMap[section][key] = j.value();
    }

    bool writeError = false;
    for (i = iniMap.constBegin(); !writeError && i != iniMap.constEnd(); ++i) {
        QByteArray realSection;

        iniEscapedKey(i.key(), realSection);

        if (realSection.isEmpty()) {
            realSection = "[General]";
        } else if (qstricmp(realSection, "general") == 0) {
            realSection = "[%General]";
        } else {
            realSection.prepend('[');
            realSection.append(']');
        }

        if (i != iniMap.constBegin())
            realSection.prepend('\n');
        realSection += '\n';

        device.write(realSection);

        const QVariantMap &ents = i.value();
        for (QVariantMap::const_iterator j = ents.constBegin(); j != ents.constEnd(); ++j) {
            QByteArray block;
            iniEscapedKey(j.key(), block);
            block += '=';

            const QVariant &value = j.value();
            if (value.type() == QVariant::StringList || value.type() == QVariant::List) {
                iniEscapedStringList(variantListToStringList(value.toList()), block);
            } else {
                iniEscapedString(variantToString(value), block);
            }
            block += '\n';
            if (device.write(block) == -1) {
                writeError = true;
                break;
            }
        }
    }
    return !writeError;
}

/*!
    \class QSettings
    \brief The QSettings class provides persistent platform-independent application settings.

    \ingroup io
    \ingroup misc
    \mainclass

    Users normally expect an application to remember its settings
    (window sizes and positions, options, etc.) across sessions. This
    information is often stored in the system registry on Windows,
    and in XML preferences files on Mac OS X. On X11 and embedded Linux,
    in the absense of a standard, many applications (including the KDE
    applications) use INI text files.

    QSettings is an abstraction around these technologies,
    enabling you to save and restore application settings in a
    portable manner.

    QSettings's API is based on QVariant, allowing you to save
    most value-based types, such as QString, QRect, and QImage,
    with the minimum of effort.

    \tableofcontents section1

    \section1 Basic Usage

    When creating a QSettings object, you must pass the domain
    name of your company or organization as well as the name of your
    application. For example, if your product is called StarRunner and
    you own the mysoft.org Internet domain name, you would
    construct the QSettings object as follows:

    \quotefromfile snippets/settings/settings.cpp
    \skipuntil snippet_ctor1
    \skipline {
    \printline QSettings settings

    QSettings objects can be created either on the stack or on
    the heap (i.e. using \c new). Constructing and destroying a
    QSettings object is very fast.

    If you use QSettings from many places in your application, you
    might want to specify the organization domain name and the
    application name using QCoreApplication::setOrganizationDomain()
    and QCoreApplication::setApplicationName(), and then use the
    default QSettings constructor:

    \skipuntil snippet_ctor2
    \skipline {
    \printline setOrganizationDomain
    \printline setApplicationName
    \dots
    \printline QSettings settings;

    QSettings stores settings. Each setting consists of a QString
    that specifies the setting's name (the \e key) and a QVariant
    that stores the data associated with the key. To write a setting,
    use setValue(). For example:

    \printline setValue(

    If there already exists a setting with the same key, the existing
    value is overwritten by the new value. For efficiency, the
    changes may not be saved to permanent storage immediately. (You
    can always call sync() to commit your changes.)

    You can get a setting's value back using value():

    \printline settings.value(

    If there is no setting with the specified name, QSettings
    returns a null QVariant (which can be converted to the integer 0).
    You can specify another default value by passing a second
    argument to value():

    \skipline {
    \printline /settings.value\(.*,.*\)/
    \skipline }

    To test whether a given key exists, call contains(). To remove
    the setting associated with a key, call remove(). To obtain the
    list of all keys, call allKeys(). To remove all keys, call
    clear().

    \section1 QVariant and GUI Types

    Because QVariant is part of the \l QtCore library, it cannot provide
    conversion functions to data types such as QColor, QImage, and
    QPixmap, which are part of \l QtGui. In other words, there is no
    \c QVariant::toColor() function.

    Instead, you can use the qvariant_value<T>() global function. For
    example:

    \code
        QSettings settings("mysoft.org", "StarRunner");
        QColor color = qvariant_value<QColor>(
                settings.value("DataPump/bgcolor"));
    \endcode

    The inverse conversion (e.g., from QColor to QVariant) is
    automatic for all data types supported by QVariant, including
    GUI-related types:

    \code
        QSettings settings("mysoft.org", "StarRunner");
        QColor color = palette().background().color();
        settings.setValue("DataPump/bgcolor", color);
    \endcode

    \section1 Key Syntax

    Setting keys can contain any Unicode characters. The Windows
    registry and INI files use case-insensitive keys, whereas the
    Carbon Preferences API on Mac OS X uses case-sensitive keys. To
    avoid portability problems, follow these two simple rules:

    \list 1
    \o Always refer to the same key using the same case. For example,
       if you refer to a key as "text fonts" in one place in your
       code, don't refer to it as "Text Fonts" somewhere else.

    \o Avoid key names that are identical except for the case. For
       example, if you have a key called "MainWindow", don't try to
       save another key as "mainwindow".
    \endlist

    You can form hierarchical keys using the '/' character as a
    separator, similar to Unix file paths. For example:

    \printline setValue
    \printline setValue
    \printline setValue

    If you want to save or restore many settings with the same
    prefix, you can specify the prefix using beginGroup() and call
    endGroup() at the end. Here's the same example again, but this
    time using the group mechanism:

    \printline beginGroup
    \printuntil endGroup
    \printline beginGroup
    \printuntil endGroup

    If a group is set using beginGroup(), the behavior of most
    functions changes consequently. Groups can be set recursively.

    In addition to groups, QSettings also supports an "array"
    concept. See beginReadArray() and beginWriteArray() for details.

    \section1 Fallback Mechanism

    Let's assume that you have created a QSettings object with
    the organization domain name "mysoft.org" and the application
    name "StarRunner". When you look up a value, up to four locations
    are searched in that order:

    \list 1
    \o a user-specific location for the StarRunner application
    \o a user-spefific location for all applications by mysoft.org
    \o a system-wide location for the StarRunner application
    \o a system-wide location for all applications by mysoft.org
    \endlist

    On Unix with X11 and on embedded Linux, these locations are the
    following files:

    \list 1
    \o \c{$HOME/.config/mysoft.org/StarRunner.conf}
    \o \c{$HOME/.config/mysoft.org.conf}
    \o \c{/etc/xdg/mysoft.org/StarRunner.conf}
    \o \c{/etc/xdg/mysoft.org.conf}
    \endlist

    On Mac OS X versions 10.2 and 10.3, these files are used:

    \list 1
    \o \c{$HOME/Library/Preferences/org.mysoft.StarRunner.plist}
    \o \c{$HOME/Library/Preferences/org.mysoft.plist}
    \o \c{/Library/Preferences/org.mysoft.StarRunner.plist}
    \o \c{/Library/Preferences/org.mysoft.plist}
    \endlist

    On Windows, the settings are stored in the following registry
    paths:

    \list 1
    \o \c{HKEY_CURRENT_USER\Software\mysoft.org\StarRunner}
    \o \c{HKEY_CURRENT_USER\Software\mysoft.org}
    \o \c{HKEY_LOCAL_MACHINE\Software\mysoft.org\StarRunner}
    \o \c{HKEY_LOCAL_MACHINE\Software\mysoft.org}
    \endlist

    If a key cannot be found in the first location, the search goes
    on in the second location, and so on. This enables you to store
    system-wide or organization-wide settings and to override them on
    a per-user or per-application basis. To turn off this mechanism,
    call setFallbacksEnabled(false).

    Although keys from all four locations are available for reading,
    only the first file (the user-specific location for the
    application at hand) is accessible for writing. To write to any
    of the other files, omit the application name and/or specify
    QSettings::SystemScope (as opposed to QSettings::UserScope, the default).

    Let's see with an example:

    \skipuntil snippet_locations
    \skipline {
    \printline obj1
    \printuntil obj4

    The table below summarizes which QSettings objects access
    which location. "\bold{X}" means that the location is the main
    location associated to the QSettings object and is used both
    for reading and for writing; "o" means that the location is used
    as a fallback when reading.

    \table
    \header \o Locations               \o \c{obj1} \o \c{obj2} \o \c{obj3} \o \c{obj4}
    \row    \o 1. User, Application    \o \bold{X} \o          \o          \o
    \row    \o 2. User, Organization   \o o        \o \bold{X} \o          \o
    \row    \o 3. System, Application  \o o        \o          \o \bold{X} \o
    \row    \o 4. System, Organization \o o        \o o        \o o        \o \bold{X}
    \endtable

    The beauty of this mechanism is that it works on all platforms
    supported by Qt and that it still gives you a lot of flexibility,
    without requiring you to specify any file names or registry
    paths.

    If you want to use INI files on all platforms instead of the
    native API, you can pass QSettings::IniFormat as the first argument to
    the QSettings constructor, followed by the scope, the
    organization domain name, and the application name:

    \printline /settings\(.*,$/
    \printline );

    Sometimes you do want to access settings stored in a specific
    file or registry path. In that case, you can use a constructor
    that takes a file name (or registry path) and a file format. For
    example:

    \printline /QSettings settings.*Ini/

    The file format can either be QSettings::IniFormat or QSettings::NativeFormat.
    On Mac OS X, the native format is an XML-based format called \e
    plist. On Windows, the native format is the Windows registry, and
    the first argument is a path in the registry rather than a file
    name, for example:

    \printline HKEY
    \printline Native

    On X11 and embedded Linux, QSettings::IniFormat and QSettings::NativeFormat have
    the same meaning.

    \section1 Restoring the State of a GUI Application

    QSettings is often used to store the state of a GUI
    application. The following example will illustrate how to use we
    will use QSettings to save and restore the geometry of an
    application's main window.

    \skipto ::writeSettings
    \printuntil /^\}$/
    \skipto ::readSettings
    \printuntil /^\}$/

    See \l{Window Geometry} for a discussion on why it is better to
    call QWidget::resize() and QWidget::move() rather than QWidget::setGeometry()
    to restore a window's geometry.

    The \c readSettings() and \c writeSettings() functions must be
    called from the main window's constructor and close event handler
    as follows:

    \skipto ::MainWindow
    \printuntil {
    \dots
    \printline readSettings
    \printline }

    \skipto ::closeEvent
    \printuntil /^\}/

    See the \l{mainwindows/application}{Application} example for a
    self-contained example that uses QSettings.

    \section1 Accessing Settings from Multiple Threads or Processes Simultaneously

    QSettings is \l{reentrant}. This means that you can use
    distinct QSettings object in different threads
    simultaneously. This guarantee stands even when the QSettings
    objects refer to the same files on disk (or to the same entries
    in the system registry). If a setting is modified through one
    QSettings object, the change will immediately be visible in
    any other QSettings objects that operate on the same location
    and that live in the same process.

    QSettings can safely be used from different processes (which
    can be different instances of your application running at the
    same time or different applications altogether) to read and write
    to the same system locations. It uses a smart merging algorithm
    to ensure data integrity. Changes performed by another process
    aren't visible in the current process until sync() is called.

    \section1 Platform-Specific Notes

    While QSettings attempts to smooth over the differences
    between the different supported platforms, there are still a few
    differences that you should be aware of when porting your
    application:

    \list
    \o  The Windows system registry has the following limitations: A
        subkey may not exceed 255 characters, an entry's value may
        not exceed 16,383 characters, and all the values of a key may
        not exceed 65,535 characters. One way to work around these
        limitations is to store the settings using the IniFormat
        instead of the NativeFormat.

    \o  On Mac OS X, allKeys() will return some extra keys for global
        settings that apply to all applications. These keys can be
        read using value() but cannot be changed, only shadowed.
        Calling setFallbacksEnabled(false) will hide these global
        settings.

    \endlist

    \sa QVariant, QSessionManager
*/

/*! \enum QSettings::Status

    The following status values are possible:

    \value NoError  No error occurred.
    \value AccessError  An access error occurred (e.g. trying to write to a read-only file).
    \value FormatError  A format error occurred (e.g. loading a malformed INI file).

    \sa status()
*/

/*! \enum QSettings::Format

    This enum type specifies the storage format used by QSettings.

    \value NativeFormat  Store the settings using the most
                         appropriate storage format for the platform.
                         On Windows, this means the system registry;
                         on Mac OS X, this means the CFPreferences
                         API; on Unix/X11, this means textual
                         configuration files in INI format.
    \value IniFormat  Store the settings in INI files.

    On Unix/X11, NativeFormat and IniFormat mean the same
    thing, except that the file extension is different (\c .conf for
    NativeFormat, \c .ini for IniFormat).

    The INI file format is a Windows file format that Qt supports on
    all platforms.
*/

/*! \enum QSettings::Scope

    This enum specifies whether settings are user-specific or shared
    by all users of the same system.

    \value UserScope  Store settings in a location specific to the
                      current user (e.g., in the user's home
                      directory).
    \value SystemScope  Store settings in a global location, so that
                        all users on the same machine access the same
                        set of settings.
    \omitvalue User
    \omitvalue Global
*/

#ifndef QT_NO_QOBJECT
/*!
    Constructs a QSettings object for accessing settings of the
    application called \a application from the organization with the
    Internet domain name \a organization, and with parent \a parent.

    Example:
    \code
        QSettings settings("technopro.co.jp", "Facturo-Pro");
    \endcode

    The scope is QSettings::UserScope and the format is QSettings::NativeFormat.

    \sa {Fallback Mechanism}
*/
QSettings::QSettings(const QString &organization, const QString &application, QObject *parent)
    : QObject(*QSettingsPrivate::create(NativeFormat, UserScope, organization, application),
              parent)
{
}

/*!
    Constructs a QSettings object for accessing settings of the
    application called \a application from the organization with the
    Internet domain name \a organization, and with parent \a parent.

    If \a scope is QSettings::UserScope, the QSettings object searches
    user-specific settings first, before it searches system-wide
    settings as a fallback. If \a scope is
    QSettings::SystemScope, the QSettings object ignores user-specific
    settings and provides access to system-wide settings.

    The storage format is always QSettings::NativeFormat.

    If no application name is given, the QSettings object will
    only access the organization-wide \l{Fallback Mechanism}{locations}.
*/
QSettings::QSettings(Scope scope, const QString &organization, const QString &application,
                     QObject *parent)
    : QObject(*QSettingsPrivate::create(NativeFormat, scope, organization, application), parent)
{
}

/*!
    Constructs a QSettings object for accessing settings of the
    application called \a application from the organization with the
    Internet domain name \a organization, and with parent \a parent.

    If \a scope is QSettings::UserScope, the QSettings object searches
    user-specific settings first, before it searches system-wide
    settings as a fallback. If \a scope is
    QSettings::SystemScope, the QSettings object ignores user-specific
    settings and provides access to system-wide settings.

    If \a format is QSettings::NativeFormat, the native API is used for
    storing settings. If \a format is QSettings::IniFormat, the INI format
    is used.

    If no application name is given, the QSettings object will
    only access the organization-wide \l{Fallback Mechanism}{locations}.
*/
QSettings::QSettings(Format format, Scope scope, const QString &organization,
                     const QString &application, QObject *parent)
    : QObject(*QSettingsPrivate::create(format, scope, organization, application),
              parent)
{
}

/*!
    Constructs a QSettings object for accessing the settings
    stored in the file called \a fileName, with parent \a parent. If
    the file doesn't already exist, it is created.

    If \a format is QSettings::NativeFormat, the meaning of \a fileName
    depends on the platform. On Unix/X11, \a fileName is the name of
    an INI file. On Mac OS X, \a fileName is the name of a .plist
    file. On Windows, \a fileName is a path in the system registry.

    If \a format is QSettings::IniFormat, \a fileName is the name of an INI
    file.

    \sa fileName()
*/
QSettings::QSettings(const QString &fileName, Format format, QObject *parent)
    : QObject(*QSettingsPrivate::create(fileName, format), parent)
{
}

/*!
    Constructs a QSettings object for accessing settings of the
    application and organization set previously with a call to
    QCoreApplication::setOrganizationDomain() and
    QCoreApplication::setApplicationName().

    The scope is QSettings::UserScope and the format is QSettings::NativeFormat.

    The code

    \code
        QSettings settings("technopro.co.jp", "Facturo-Pro");
    \endcode

    is equivalent to

    \code
        QApplication::setOrganizationDomain("technopro.co.jp");
        QApplication::setApplicationName("Facturo-Pro");
        QSettings settings;
    \endcode

    If QApplication::setOrganizationDomain() and
    QApplication::setApplicationName() has not been previously called,
    the QSettings object will not be able to read or write any
    settings, and status() will return AccessError.
*/
QSettings::QSettings(QObject *parent)
    : QObject(*QSettingsPrivate::create(NativeFormat, UserScope,
                                        QCoreApplication::organizationDomain(),
                                        QCoreApplication::applicationName()),
              parent)
{
}

#else
QSettings::QSettings(const QString &organization, const QString &application)
    : d_ptr(QSettingsPrivate::create(QSettings::NativeFormat, QSettings::UserScope, organization, application))
{
    d_ptr->q_ptr = this;
}

QSettings::QSettings(Scope scope, const QString &organization, const QString &application)
    : d_ptr(QSettingsPrivate::create(QSettings::NativeFormat, scope, organization, application))
{
    d_ptr->q_ptr = this;
}

QSettings::QSettings(Format format, Scope scope, const QString &organization,
                     const QString &application)
    : d_ptr(QSettingsPrivate::create(format, scope, organization, application))
{
    d_ptr->q_ptr = this;
}

QSettings::QSettings(const QString &fileName, Format format)
    : d_ptr(QSettingsPrivate::create(fileName, format))
{
    d_ptr->q_ptr = this;
}
#endif

/*!
    Destroys the QSettings object.

    Any unsaved changes will eventually be written to permanent
    storage.

    \sa sync()
*/
QSettings::~QSettings()
{
    Q_D(QSettings);
    if (d->pendingChanges)
        d->flush();
}

/*!
    Removes all entries in the primary location associated to this
    QSettings object.

    Entries in fallback locations are not removed.

    If you only want to remove the entries in the current group(),
    use remove("") instead.

    \sa remove(), setFallbacksEnabled()
*/
void QSettings::clear()
{
    Q_D(QSettings);
    d->clear();
    d->requestUpdate();
}

/*!
    Writes any unsaved changes to permanent storage, and reloads any
    settings that have been changed in the meantime by another
    application.

    Unless you use QSettings as a communication mechanism between
    different processes, you normally don't need to call this
    function.
*/
void QSettings::sync()
{
    Q_D(QSettings);
    d->sync();
}

/*!
    Returns the path where settings written using this QSettings
    object are stored.

    On Windows, if the format is QSettings::NativeFormat, the return value
    is a system registry path, not a file path.

    \sa isWritable()
*/
QString QSettings::fileName() const
{
    Q_D(const QSettings);
    return d->fileName();
}

/*!
    Returns a status code indicating the first error that was met by
    QSettings, or QSettings::NoError if no error occurred.
*/
QSettings::Status QSettings::status() const
{
    Q_D(const QSettings);
    return d->status;
}

/*!
    Appends \a prefix to the current group.

    The current group is automatically prepended to all keys
    specified to QSettings. In addition, query functions such as
    childGroups(), childKeys(), and allKeys() are based on the group.
    By default, no group is set.

    Groups are useful to avoid typing in the same setting paths over
    and over. For example:

    \code
        settings.beginGroup("mainwindow");
        settings.setValue("size", win->size());
        settings.setValue("fullScreen", win->isFullScreen());
        settings.endGroup();

        settings.beginGroup("outputpanel");
        settings.setValue("visible", panel->isVisible());
        settings.endGroup();
    \endcode

    This will set the value of three settings:

    \list
    \o \c mainwindow/size
    \o \c mainwindow/fullScreen
    \o \c outputpanel/visible
    \endlist

    Call endGroup() to reset the current group to what it was before
    the corresponding beginGroup() call. Groups can be nested.

    \sa endGroup(), group()
*/
void QSettings::beginGroup(const QString &prefix)
{
    Q_D(QSettings);
    d->beginGroupOrArray(QSettingsGroup(d->normalizedKey(prefix)));
}

/*!
    Resets the group to what it was before the corresponding
    beginGroup() call.

    Example:

    \code
        settings.beginGroup("alpha");
        // settings.group() == "alpha"

        settings.beginGroup("beta");
        // settings.group() == "alpha/beta"

        settings.endGroup();
        // settings.group() == "alpha"

        settings.endGroup();
        // settings.group() == ""
    \endcode

    \sa beginGroup(), group()
*/
void QSettings::endGroup()
{
    Q_D(QSettings);
    if (d->groupStack.isEmpty()) {
        qWarning("QSettings::endGroup: No matching beginGroup()");
        return;
    }

    QSettingsGroup group = d->groupStack.pop();
    int len = group.toString().size();
    if (len > 0)
        d->groupPrefix.truncate(d->groupPrefix.size() - (len + 1));

    if (group.isArray())
        qWarning("QSettings::endGroup: Expected endArray() instead");
}

/*!
    Returns the current group.

    \sa beginGroup(), endGroup()
*/
QString QSettings::group() const
{
    Q_D(const QSettings);
    return d->groupPrefix.left(d->groupPrefix.size() - 1);
}

/*!
    Adds \a prefix to the current group and starts reading from an
    array. Returns the size of the array.

    Example:

    \code
        struct Login {
            QString userName;
            QString password;
        };
        QList<Login> logins;
        ...

        QSettings settings;
        int size = settings.beginReadArray("logins");
        for (int i = 0; i < size; ++i) {
            settings.setArrayIndex(i);
            Login login;
            login.userName = settings.value("userName");
            login.password = settings.value("password");
            logins.append(login);
        }
        settings.endArray();
    \endcode

    Use beginWriteArray() to write the array in the first place.

    \sa beginWriteArray(), endArray(), setArrayIndex()
*/
int QSettings::beginReadArray(const QString &prefix)
{
    Q_D(QSettings);
    d->beginGroupOrArray(QSettingsGroup(d->normalizedKey(prefix), false));
    return value("size").toInt();
}

/*!
    Adds \a prefix to the current group and starts writing an array
    of size \a size. If \a size is -1 (the default), it is automatically
    determined based on the indexes of the entries written.

    If you have many occurrences of a certain set of keys, you can
    use arrays to make your life easier. For example, let's suppose
    that you want to save a variable-length list of user names and
    passwords. You could then write:

    \code
        struct Login {
            QString userName;
            QString password;
        };
        QList<Login> logins;
        ...

        QSettings settings;
        settings.beginWriteArray("logins");
        for (int i = 0; i < logins.size(); ++i) {
            settings.setArrayIndex(i);
            settings.setValue("userName", list.at(i).userName);
            settings.setValue("password", list.at(i).password);
        }
        settings.endArray();
    \endcode

    The generated keys will have the form

    \list
    \o \c logins/1/userName
    \o \c logins/1/password
    \o \c logins/2/userName
    \o \c logins/2/password
    \o \c logins/3/userName
    \o \c logins/3/password
    \o ...
    \endlist

    To read back an array, use beginReadArray().

    \sa beginReadArray(), endArray(), setArrayIndex()
*/
void QSettings::beginWriteArray(const QString &prefix, int size)
{
    Q_D(QSettings);
    d->beginGroupOrArray(QSettingsGroup(d->normalizedKey(prefix), size < 0));

    if (size < 0)
        remove(QLatin1String("size"));
    else
        setValue(QLatin1String("size"), size);
}

/*!
    Closes the array that was started using beginReadArray() or
    beginWriteArray().

    \sa beginReadArray(), beginWriteArray()
*/
void QSettings::endArray()
{
    Q_D(QSettings);
    if (d->groupStack.isEmpty()) {
        qWarning("QSettings::endArray: No matching beginArray()");
        return;
    }

    QSettingsGroup group = d->groupStack.top();
    int len = group.toString().size();
    d->groupStack.pop();
    if (len > 0)
        d->groupPrefix.truncate(d->groupPrefix.size() - (len + 1));

    if (group.arraySizeGuess() != -1)
        setValue(group.name() + QLatin1String("/size"), group.arraySizeGuess());

    if (!group.isArray())
        qWarning("QSettings::endArray: Expected endGroup() instead");
}

/*!
    Sets the current array index to \a i. Calls to functions such as
    setValue(), value(), remove(), and contains() will operate on the
    array entry at that index.

    You must call beginReadArray() or beginWriteArray() before you
    can call this function.
*/
void QSettings::setArrayIndex(int i)
{
    Q_D(QSettings);
    if (d->groupStack.isEmpty() || !d->groupStack.top().isArray()) {
        qWarning("QSettings::setArrayIndex: Missing beginArray()");
        return;
    }

    QSettingsGroup &top = d->groupStack.top();
    int len = top.toString().size();
    top.setArrayIndex(qMax(i, 0));
    d->groupPrefix.replace(d->groupPrefix.size() - len - 1, len, top.toString());
}

/*!
    Returns a list of all keys, including subkeys, that can be read
    using the QSettings object.

    Example:

    \code
        QSettings settings;
        settings.setValue("fridge/color", Qt::white);
        settings.setValue("fridge/size", QSize(32, 96));
        settings.setValue("sofa", true);
        settings.setValue("tv", false);

        QStringList keys = settings.allKeys();
        // keys: ["fridge/color", "fridge/size", "sofa", "tv"]
    \endcode

    If a group is set using beginGroup(), only the keys in the group
    are returned, without the group prefix:

    \code
        settings.beginGroup("fridge");
        keys = settings.allKeys();
        // keys: ["color", "size"]
    \endcode

    \sa childGroups(), childKeys()
*/
QStringList QSettings::allKeys() const
{
    Q_D(const QSettings);
    return d->children(d->groupPrefix, QSettingsPrivate::AllKeys);
}

/*!
    Returns a list of all top-level keys that can be read using the
    QSettings object.

    Example:

    \code
        QSettings settings;
        settings.setValue("fridge/color", Qt::white);
        settings.setValue("fridge/size", QSize(32, 96));
        settings.setValue("sofa", true);
        settings.setValue("tv", false);

        QStringList keys = settings.childKeys();
        // keys: ["sofa", "tv"]
    \endcode

    If a group is set using beginGroup(), the top-level keys in that
    group are returned, without the group prefix:

    \code
        settings.beginGroup("fridge");
        keys = settings.childKeys();
        // keys: ["color", "size"]
    \endcode

    You can navigate through the entire setting hierarchy using
    childKeys() and childGroups() recursively.

    \sa childGroups(), allKeys()
*/
QStringList QSettings::childKeys() const
{
    Q_D(const QSettings);
    return d->children(d->groupPrefix, QSettingsPrivate::ChildKeys);
}

/*!
    Returns a list of all key top-level groups that contain keys that
    can be read using the QSettings object.

    Example:

    \code
        QSettings settings;
        settings.setValue("fridge/color", Qt::white);
        settings.setValue("fridge/size", QSize(32, 96));
        settings.setValue("sofa", true);
        settings.setValue("tv", false);

        QStringList groups = settings.childGroups();
        // group: ["fridge"]
    \endcode

    If a group is set using beginGroup(), the first-level keys in
    that group are returned, without the group prefix.

    \code
        settings.beginGroup("fridge");
        groups = settings.childGroups();
        // groups: []
    \endcode

    You can navigate through the entire setting hierarchy using
    childKeys() and childGroups() recursively.

    \sa childKeys(), allKeys()
*/
QStringList QSettings::childGroups() const
{
    Q_D(const QSettings);
    return d->children(d->groupPrefix, QSettingsPrivate::ChildGroups);
}

/*!
    Returns true if settings can be written using this QSettings
    object; returns false otherwise.

    One reason why isWritable() might return false is if
    QSettings operates on a read-only file.

    \sa fileName(), status()
*/
bool QSettings::isWritable() const
{
    Q_D(const QSettings);
    return d->isWritable();
}

/*!
    Sets the value of setting \a key to \a value.

    If the key already exists, the previous value is overwritten.

    Example:

    \code
        QSettings settings;
        settings.setValue("interval", 30);
        settings.value("interval").toInt();     // returns 30

        settings.setValue("interval", 6.55);
        settings.value("interval").toDouble();  // returns 6.55
    \endcode

    \sa value(), remove(), contains()
*/
void QSettings::setValue(const QString &key, const QVariant &value)
{
    Q_D(QSettings);
    QString k = d->actualKey(key);
    d->set(k, value);
    d->requestUpdate();
}

/*!
    Removes the setting \a key and any sub-settings of \a key.

    Example:

    \code
        QSettings settings;
        settings.setValue("ape");
        settings.setValue("monkey", 1);
        settings.setValue("monkey/sea", 2);
        settings.setValue("monkey/doe", 4);

        settings.remove("monkey");
        QStringList keys = settings.allKeys();
        // keys: ["ape"]
    \endcode

    Be aware that if one of the fallback locations contains a setting
    with the same key, that setting will be visible after calling
    remove().

    If \a key is an empty string, all keys in the current group() are
    removed. For example:

    \code
        QSettings settings;
        settings.setValue("ape");
        settings.setValue("monkey", 1);
        settings.setValue("monkey/sea", 2);
        settings.setValue("monkey/doe", 4);

        settings.beginGroup("monkey");
        settings.remove("");
        settings.endGroup();

        QStringList keys = settings.allKeys();
        // keys: ["ape"]
    \endcode

    \sa setValue(), value(), contains()
*/
void QSettings::remove(const QString &key)
{
    Q_D(QSettings);
    /*
        We cannot use actualKey(), because remove() supports empty
        keys. The code is also tricky because of slash handling.
    */
    QString theKey = d->normalizedKey(key);
    if (theKey.isEmpty())
        theKey = group();
    else
        theKey.prepend(d->groupPrefix);

    if (theKey.isEmpty()) {
        d->clear();
    } else {
        d->remove(theKey);
    }
    d->requestUpdate();
}

/*!
    Returns true if there exists a setting called \a key; returns
    false otherwise.

    If a group is set using beginGroup(), \a key is taken to be
    relative to that group.

    \sa value(), setValue()
*/
bool QSettings::contains(const QString &key) const
{
    Q_D(const QSettings);
    QString k = d->actualKey(key);
    return d->get(k, 0);
}

/*!
    Sets whether fallbacks are enabled to \a b.

    By default, fallbacks are enabled.

    \sa fallbacksEnabled()
*/
void QSettings::setFallbacksEnabled(bool b)
{
    Q_D(QSettings);
    d->fallbacks = !!b;
}

/*!
    Returns true if fallbacks are enabled; returns false otherwise.

    By default, fallbacks are enabled.

    \sa setFallbacksEnabled()
*/
bool QSettings::fallbacksEnabled() const
{
    Q_D(const QSettings);
    return d->fallbacks;
}

#ifndef QT_NO_QOBJECT
/*!
    \reimp
*/
bool QSettings::event(QEvent *event)
{
    Q_D(QSettings);
    if (event->type() == QEvent::UpdateRequest) {
        d->update();
        return true;
    }
    return QObject::event(event);
}
#endif

/*!
    Returns the value for setting \a key. If the setting doesn't
    exist, returns \a defaultValue.

    If no default value is specified, a default QVariant is
    returned.

    Example:

    \code
        QSettings settings;
        settings.setValue("animal/snake", 58);
        settings.value("animal/snake", 1024).toInt();   // returns 58
        settings.value("animal/zebra", 1024).toInt();   // returns 1024
        settings.value("animal/zebra").toInt();         // returns 0
    \endcode

    \sa setValue(), contains(), remove()
*/
QVariant QSettings::value(const QString &key, const QVariant &defaultValue) const
{
    Q_D(const QSettings);
    QVariant result = defaultValue;
    QString k = d->actualKey(key);
    d->get(k, &result);
    return result;
}

/*!
    Sets the directory where QSettings stores its SystemScope \c .ini files to \a dir.

    On Unix systems, the default directory is \c /etc/xdg in accordance with FreeDesktop's
    XDG Base Directory Specification. This default can be changed when compiling Qt by passing
    the \c --sysconfdir flag to \c configure.

    On Windows, the default directory is \c{C:\Documents and Settings\All Users\Application Data}.

    A call to this function should precede any instantiations of QSettings objects.

    \sa setUserIniPath()
*/
void QSettings::setSystemIniPath(const QString &dir)
{
    *defaultSystemIniPath() = dir;
}

/*!
    Sets the directory where QSettings stores its UserScope \c .ini files to \a dir.

    On Unix systems, the default directory is read from the \c $XDG_CONFIG_HOME environment
    variable. If this variable is empty or unset, \c $HOME/.config is used, in accordance with
    the FreeDesktop's XDG Base Directory Specification. Calling this function overrides the
    path specified in \c $XDG_CONFIG_HOME.

    On Windows, the default directory is \c{C:\Documents and Settings\<username>\Application Data}.

    A call to this function should precede any instantiations of QSettings objects.

    \sa setSystemIniPath()
*/

void QSettings::setUserIniPath(const QString &dir)
{
    *defaultUserIniPath() = dir;
}

#ifdef QT3_SUPPORT
/*! \fn bool QSettings::writeEntry(const QString &key, bool value)

    Sets the value of setting \a key to \a value.

    Use setValue() instead.
*/

/*! \fn bool QSettings::writeEntry(const QString &key, double value)

    \overload
*/

/*! \fn bool QSettings::writeEntry(const QString &key, int value)

    \overload
*/

/*! \fn bool QSettings::writeEntry(const QString &key, const char *value)

    \overload
*/

/*! \fn bool QSettings::writeEntry(const QString &key, const QString &value)

    \overload
*/

/*! \fn bool QSettings::writeEntry(const QString &key, const QStringList &value)

    \overload
*/

/*! \fn bool QSettings::writeEntry(const QString &key, const QStringList &value, QChar separator)

    \overload

    Use setValue(\a key, \a value) instead. You don't need \a separator.
*/

/*! \fn QStringList QSettings::readListEntry(const QString &key, bool *ok = 0)

    Returns the value of setting \a key converted to a QStringList.

    If \a ok is not 0, *\a{ok} is set to true if the key exists,
    otherwise *\a{ok} is set to false.

    Use value() instead.

    \oldcode
        bool ok;
        QStringList list = settings.readListEntry("recentFiles", &ok);
    \newcode
        bool ok = settings.contains("recentFiles");
        QStringList list = settings.value("recentFiles").toStringList();
    \endcode
*/

/*! \fn QStringList QSettings::readListEntry(const QString &key, QChar separator, bool *ok)

    Returns the value of setting \a key converted to a QStringList.
    \a separator is ignored.

    If \a ok is not 0, *\a{ok} is set to true if the key exists,
    otherwise *\a{ok} is set to false.

    Use value() instead.

    \oldcode
        bool ok;
        QStringList list = settings.readListEntry("recentFiles", ":", &ok);
    \newcode
        bool ok = settings.contains("recentFiles");
        QStringList list = settings.value("recentFiles").toStringList();
    \endcode
*/

/*! \fn QString QSettings::readEntry(const QString &key, const QString &defaultValue, bool *ok)

    Returns the value for setting \a key converted to a QString. If
    the setting doesn't exist, returns \a defaultValue.

    If \a ok is not 0, *\a{ok} is set to true if the key exists,
    otherwise *\a{ok} is set to false.

    Use value() instead.

    \oldcode
        bool ok;
        QString str = settings.readEntry("userName", "administrator", &ok);
    \newcode
        bool ok = settings.contains("userName");
        QString str = settings.value("userName", "administrator").toString();
    \endcode
*/

/*! \fn int QSettings::readNumEntry(const QString &key, int defaultValue, bool *ok)

    Returns the value for setting \a key converted to an \c int. If
    the setting doesn't exist, returns \a defaultValue.

    If \a ok is not 0, *\a{ok} is set to true if the key exists,
    otherwise *\a{ok} is set to false.

    Use value() instead.

    \oldcode
        bool ok;
        int max = settings.readNumEntry("maxConnections", 30, &ok);
    \newcode
        bool ok = settings.contains("maxConnections");
        int max = settings.value("maxConnections", 30).toInt();
    \endcode
*/

/*! \fn double QSettings::readDoubleEntry(const QString &key, double defaultValue, bool *ok)

    Returns the value for setting \a key converted to a \c double. If
    the setting doesn't exist, returns \a defaultValue.

    If \a ok is not 0, *\a{ok} is set to true if the key exists,
    otherwise *\a{ok} is set to false.

    Use value() instead.

    \oldcode
        bool ok;
        double pi = settings.readDoubleEntry("pi", 3.141592, &ok);
    \newcode
        bool ok = settings.contains("pi");
        double pi = settings.value("pi", 3.141592).toDouble();
    \endcode
*/

/*! \fn bool QSettings::readBoolEntry(const QString &key, bool defaultValue, bool *ok)

    Returns the value for setting \a key converted to a \c bool. If
    the setting doesn't exist, returns \a defaultValue.

    If \a ok is not 0, *\a{ok} is set to true if the key exists,
    otherwise *\a{ok} is set to false.

    Use value() instead.

    \oldcode
        bool ok;
        bool grid = settings.readBoolEntry("showGrid", true, &ok);
    \newcode
        bool ok = settings.contains("showGrid");
        bool grid = settings.value("showGrid", true).toBool();
    \endcode
*/

/*! \fn bool QSettings::removeEntry(const QString &key)

    Use remove() instead.
*/

/*! \enum QSettings::System
    \compat

    \value Unix Unix/X11 systems
    \value Windows Microsoft Windows systems
    \value Mac Mac OS X systems

    \sa insertSearchPath(), removeSearchPath()
*/

/*! \fn void QSettings::insertSearchPath(System system, const QString &path)

    This function is implemented as a no-op. It is provided for
    source compatibility with Qt 3. The new QSettings class has no
    concept of "search path".
*/

/*! \fn void QSettings::removeSearchPath(System system, const QString &path)

    This function is implemented as a no-op. It is provided for
    source compatibility with Qt 3. The new QSettings class has no
    concept of "search path".
*/

/*! \fn void QSettings::setPath(const QString &organization, const QString &application, \
                                Scope scope)

    Specifies the \a organization, \a application, and \a scope to
    use by the QSettings object.

    Use the appropriate constructor instead, with QSettings::UserScope
    instead of QSettings::User and QSettings::SystemScope instead of
    QSettings::Global.

    \oldcode
        QSettings settings;
        settings.setPath("twikimaster.com", "Kanooth", QSettings::Global);
    \newcode
        QSettings settings(QSettings::SystemScope, "twikimaster.com", "Kanooth");
    \endcode
*/

/*! \fn void QSettings::resetGroup()

    Sets the current group to be the empty string.

    Use endGroup() instead (possibly multiple times).

    \oldcode
        QSettings settings;
        settings.beginGroup("mainWindow");
        settings.beginGroup("leftPanel");
        ...
        settings.resetGroup();
    \newcode
        QSettings settings;
        settings.beginGroup("mainWindow");
        settings.beginGroup("leftPanel");
        ...
        settings.endGroup();
        settings.endGroup();
    \endcode
*/

/*! \fn QStringList QSettings::entryList(const QString &key) const

    Returns a list of all sub-keys of \a key.

    Use childKeys() instead.

    \oldcode
        QSettings settings;
        QStringList keys = settings.entryList("cities");
        ...
    \newcode
        QSettings settings;
        settings.beginGroup("cities");
        QStringList keys = settings.childKeys();
        ...
        settings.endGroup();
    \endcode
*/

/*! \fn QStringList QSettings::subkeyList(const QString &key) const

    Returns a list of all sub-keys of \a key.

    Use childGroups() instead.

    \oldcode
        QSettings settings;
        QStringList groups = settings.entryList("cities");
        ...
    \newcode
        QSettings settings;
        settings.beginGroup("cities");
        QStringList groups = settings.childKeys();
        ...
        settings.endGroup();
    \endcode
*/
#endif

#endif // QT_NO_SETTINGS
