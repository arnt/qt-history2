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

#include "qsettings_p.h"
#include "qcache.h"
#include "qfile.h"
#include "qdir.h"
#include "qfileinfo.h"
#include "qmutex.h"
#include "qlibraryinfo.h"
#include "qtemporaryfile.h"

#ifndef QT_NO_GEOM_VARIANT
#include "qsize.h"
#include "qpoint.h"
#include "qrect.h"
#endif // !QT_NO_GEOM_VARIANT

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

struct QConfFileCustomFormat
{
    QString extension;
    QSettings::ReadFunc readFunc;
    QSettings::WriteFunc writeFunc;
    Qt::CaseSensitivity caseSensitivity;
};

typedef QHash<QString, QConfFile *> ConfFileHash;
typedef QCache<QString, QConfFile> ConfFileCache;
typedef QHash<int, QString> PathHash;
typedef QVector<QConfFileCustomFormat> CustomFormatVector;

Q_GLOBAL_STATIC(ConfFileHash, usedHashFunc)
Q_GLOBAL_STATIC(ConfFileCache, unusedCacheFunc)
Q_GLOBAL_STATIC(PathHash, pathHashFunc)
Q_GLOBAL_STATIC(CustomFormatVector, customFormatVectorFunc)
Q_GLOBAL_STATIC(QMutex, globalMutex)

#ifndef Q_OS_WIN
static bool unixLock(int handle, int lockType)
{
    struct flock fl;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_type = lockType;
    return !fcntl(handle, F_SETLKW, &fl);
}
#endif

QConfFile::QConfFile(const QString &fileName)
    : name(fileName), size(0), ref(1)
{
    usedHashFunc()->insert(name, this);
}

InternalSettingsMap QConfFile::mergedKeyMap() const
{
    InternalSettingsMap result = originalKeys;
    InternalSettingsMap::const_iterator i;

    for (i = removedKeys.begin(); i != removedKeys.end(); ++i)
        result.remove(i.key());
    for (i = addedKeys.begin(); i != addedKeys.end(); ++i)
        result.insert(i.key(), i.value());
    return result;
}

QConfFile *QConfFile::fromName(const QString &fileName)
{
    QString absPath = QFileInfo(fileName).absoluteFilePath();

    ConfFileHash *usedHash = usedHashFunc();
    ConfFileCache *unusedCache = unusedCacheFunc();

    QConfFile *confFile;
    QMutexLocker locker(globalMutex());

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
    QMutexLocker locker(globalMutex());
    unusedCacheFunc()->clear();
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
QSettingsPrivate *QSettingsPrivate::create(QSettings::Format format, QSettings::Scope scope,
                                           const QString &organization, const QString &application)
{
    return new QConfFileSettingsPrivate(format, scope, organization, application);
}
#endif

#if !defined(Q_OS_WIN)
QSettingsPrivate *QSettingsPrivate::create(const QString &fileName, QSettings::Format format)
{
    return new QConfFileSettingsPrivate(fileName, format);
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
    }
    return l;
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
        case QVariant::Double:
        case QVariant::KeySequence: {
            result = v.toString();
            result = escapedLeadingAt(result);
            break;
        }
#ifndef QT_NO_GEOM_VARIANT
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
#endif // !QT_NO_GEOM_VARIANT

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
    result.reserve(result.length()+key.length()*3/2);
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
    bool lowercaseOnly = true;
    int i = from;
    result.reserve(result.length()+(to-from+1)*3/2);
    while (i < to) {
        int ch = (uchar)key.at(i);

        if (ch == '\\') {
            result += QLatin1Char('/');
            ++i;
            continue;
        }

        if (ch != '%' || i == to - 1) {
            if (isupper((uchar)ch))
                lowercaseOnly = false;
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
            lowercaseOnly = false;
        result += qch;
        i = firstDigitPos + numDigits;
    }
    return lowercaseOnly;
}

void QSettingsPrivate::iniEscapedString(const QString &str, QByteArray &result)
{
    bool needsQuotes = false;
    bool escapeNextIfDigit = false;
    int i;
    int startPos = result.size();

    result.reserve(startPos+str.size()*3/2);
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
            escapeNextIfDigit = true;
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
                // fallthrough
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
    Q_ASSERT(s.at(idx) == QLatin1Char('('));
    Q_ASSERT(s.at(l - 1) == QLatin1Char(')'));

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

/*
    If we don't have the permission to read the file, returns false.
    If the file doesn't exist, returns true.
*/
static bool checkAccess(const QString &name)
{
    QFileInfo fileInfo(name);

    if (fileInfo.exists()) {
        QFile file(name);
        // if the file exists but we can't open it, report an error
        return file.open(QFile::ReadOnly);
    } else {
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
            const QString &elt = pathElements.at(i);
            if (dir.cd(elt))
                continue;

            if (dir.mkdir(elt) && dir.cd(elt))
                continue;

            if (dir.cd(elt))
                continue;

            // if the path can't be created/reached, report an error
            return false;
        }
        // we treat non-existent files as if they existed but were empty
        return true;
    }
}

void QConfFileSettingsPrivate::initFormat()
{
    extension = (format == QSettings::NativeFormat) ? QLatin1String(".conf") : QLatin1String(".ini");
    readFunc = 0;
    writeFunc = 0;
#ifdef Q_OS_MAC
    caseSensitivity = (format == QSettings::NativeFormat) ? Qt::CaseSensitive : Qt::CaseInsensitive;
#else
    caseSensitivity = Qt::CaseInsensitive;
#endif

    if (format > QSettings::IniFormat) {
        QMutexLocker locker(globalMutex());
        const CustomFormatVector *customFormatVector = customFormatVectorFunc();

        int i = (int)format - (int)QSettings::CustomFormat1;
        if (i >= 0 && i < customFormatVector->size()) {
            QConfFileCustomFormat info = customFormatVector->at(i);
            extension = info.extension;
            readFunc = info.readFunc;
            writeFunc = info.writeFunc;
            caseSensitivity = info.caseSensitivity;
        }
    }
}

void QConfFileSettingsPrivate::initAccess()
{
    bool readAccess = false;
    if (confFiles[spec]) {
        readAccess = checkAccess(confFiles[spec]->name);
        if (format > QSettings::IniFormat) {
            if (!readFunc)
                readAccess = false;
        }
    }

    if (!readAccess)
        setStatus(QSettings::AccessError);

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
            ;
        }
    }

    return result;
}
#endif // Q_OS_WIN

static inline int pathHashKey(QSettings::Format format, QSettings::Scope scope)
{
    return int((uint(format) << 1) | uint(scope == QSettings::SystemScope));
}

static QString getPath(QSettings::Format format, QSettings::Scope scope)
{
    Q_ASSERT((int)QSettings::NativeFormat == 0);
    Q_ASSERT((int)QSettings::IniFormat == 1);

    QString homePath = QDir::homePath();
    QString systemPath;

    globalMutex()->lock();
    PathHash *pathHash = pathHashFunc();
    bool loadSystemPath = pathHash->isEmpty();
    globalMutex()->unlock();

    if (loadSystemPath) {
        /*
           QLibraryInfo::location() uses QSettings, so in order to
           avoid a dead-lock, we can't hold the global mutex while
           calling it.
       */
        systemPath = QLibraryInfo::location(QLibraryInfo::SettingsPath);
        systemPath += QLatin1Char('/');
    }

    QMutexLocker locker(globalMutex());
    if (pathHash->isEmpty()) {
        /*
           Lazy initialization of pathHash. We initialize the
           IniFormat paths and (on Unix) the NativeFormat paths.
           (The NativeFormat paths are not configurable for the
           Windows registry and the Mac CFPreferences.)
       */
#ifdef Q_OS_WIN
        pathHash->insert(pathHashKey(QSettings::IniFormat, QSettings::UserScope),
                         windowsConfigPath(CSIDL_APPDATA) + QDir::separator());
        pathHash->insert(pathHashKey(QSettings::IniFormat, QSettings::SystemScope),
                         windowsConfigPath(CSIDL_COMMON_APPDATA) + QDir::separator());
#else
        QString userPath;
        char *env = getenv("XDG_CONFIG_HOME");
        if (env == 0) {
            userPath = homePath;
            userPath += QLatin1Char('/');
#ifdef Q_WS_QWS
            userPath += QLatin1String("Settings");
#else
            userPath += QLatin1String(".config");
#endif
        } else if (*env == '/') {
            userPath = QLatin1String(env);
        } else {
            userPath = homePath;
            userPath += QLatin1Char('/');
            userPath += QLatin1String(env);
        }
        userPath += QLatin1Char('/');

        pathHash->insert(pathHashKey(QSettings::IniFormat, QSettings::UserScope), userPath);
        pathHash->insert(pathHashKey(QSettings::IniFormat, QSettings::SystemScope), systemPath);
#ifndef Q_OS_MAC
        pathHash->insert(pathHashKey(QSettings::NativeFormat, QSettings::UserScope), userPath);
        pathHash->insert(pathHashKey(QSettings::NativeFormat, QSettings::SystemScope), systemPath);
#endif
#endif
    }

    QString result = pathHash->value(pathHashKey(format, scope));
    if (!result.isEmpty())
        return result;

    // fall back on INI path
    return pathHash->value(pathHashKey(QSettings::IniFormat, scope));
}

QConfFileSettingsPrivate::QConfFileSettingsPrivate(QSettings::Format format,
                                                   QSettings::Scope scope,
                                                   const QString &organization,
                                                   const QString &application)
{
    int i;
    this->format = format;
    initFormat();

    for (i = 0; i < NumConfFiles; ++i)
        confFiles[i] = 0;

    QString org = organization;
    if (org.isEmpty()) {
        setStatus(QSettings::AccessError);
        org = QLatin1String("Unknown Organization");
    }

    QString appFile = org + QDir::separator() + application + extension;
    QString orgFile = org + extension;

    if (scope == QSettings::UserScope) {
        QString userPath = getPath(format, QSettings::UserScope);
        if (!application.isEmpty())
            confFiles[F_User | F_Application] = QConfFile::fromName(userPath + appFile);
        confFiles[F_User | F_Organization] = QConfFile::fromName(userPath + orgFile);
    }

    QString systemPath = getPath(format, QSettings::SystemScope);
    if (!application.isEmpty())
        confFiles[F_System | F_Application] = QConfFile::fromName(systemPath + appFile);
    confFiles[F_System | F_Organization] = QConfFile::fromName(systemPath + orgFile);

    for (i = 0; i < NumConfFiles; ++i) {
        if (confFiles[i]) {
            spec = i;
            break;
        }
    }

    initAccess();
}

QConfFileSettingsPrivate::QConfFileSettingsPrivate(const QString &fileName,
                                                   QSettings::Format format)
{
    this->format = format;
    initFormat();

    confFiles[0] = QConfFile::fromName(fileName);
    for (int i = 1; i < NumConfFiles; ++i)
        confFiles[i] = 0;

    initAccess();
}

QConfFileSettingsPrivate::~QConfFileSettingsPrivate()
{
    QMutexLocker locker(globalMutex());
    ConfFileHash *usedHash = usedHashFunc();
    ConfFileCache *unusedCache = unusedCacheFunc();

    for (int i = 0; i < NumConfFiles; ++i) {
        if (confFiles[i] && !confFiles[i]->ref.deref()) {
            if (usedHash)
                usedHash->remove(confFiles[i]->name);

            if (confFiles[i]->size == 0) {
                delete confFiles[i];
            } else if (unusedCache) {
                unusedCache->insert(confFiles[i]->name, confFiles[i],
                                    10 + (confFiles[i]->originalKeys.size() / 4));
            }
        }
    }
}

void QConfFileSettingsPrivate::remove(const QString &key)
{
    QConfFile *confFile = confFiles[spec];
    if (!confFile)
        return;

    QSettingsKey theKey(key, caseSensitivity);
    QSettingsKey prefix(key + QLatin1Char('/'), caseSensitivity);
    QMutexLocker locker(&confFile->mutex);

    InternalSettingsMap::iterator i = confFile->addedKeys.lowerBound(prefix);
    while (i != confFile->addedKeys.end() && i.key().startsWith(prefix))
        i = confFile->addedKeys.erase(i);
    confFile->addedKeys.remove(theKey);

    InternalSettingsMap::const_iterator j = confFile->originalKeys.lowerBound(prefix);
    while (j != confFile->originalKeys.constEnd() && j.key().startsWith(prefix)) {
        confFile->removedKeys.insert(j.key(), QVariant());
        ++j;
    }
    if (confFile->originalKeys.contains(theKey))
        confFile->removedKeys.insert(theKey, QVariant());
}

void QConfFileSettingsPrivate::set(const QString &key, const QVariant &value)
{
    QConfFile *confFile = confFiles[spec];
    if (!confFile)
        return;

    QSettingsKey theKey(key, caseSensitivity);
    QMutexLocker locker(&confFile->mutex);
    confFile->removedKeys.remove(theKey);
    confFile->addedKeys.insert(theKey, value);
}

bool QConfFileSettingsPrivate::get(const QString &key, QVariant *value) const
{
    QSettingsKey theKey(key, caseSensitivity);
    InternalSettingsMap::const_iterator j;
    bool found = false;

    for (int i = 0; i < NumConfFiles; ++i) {
        if (QConfFile *confFile = confFiles[i]) {
            QMutexLocker locker(&confFile->mutex);

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
    InternalSettingsMap::const_iterator j;

    QSettingsKey thePrefix(prefix, caseSensitivity);
    int startPos = prefix.size();

    for (int i = 0; i < NumConfFiles; ++i) {
        if (QConfFile *confFile = confFiles[i]) {
            QMutexLocker locker(&confFile->mutex);

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
    QConfFile *confFile = confFiles[spec];
    if (!confFile)
        return;

    QMutexLocker locker(&confFile->mutex);
    confFile->addedKeys.clear();
    confFile->removedKeys = confFile->originalKeys;
}

void QConfFileSettingsPrivate::sync()
{
    // people probably won't be checking the status a whole lot, so in case of
    // error we just try to go on and make the best of it

    for (int i = 0; i < NumConfFiles; ++i) {
        QConfFile *confFile = confFiles[i];
        if (confFile) {
            QMutexLocker locker(&confFile->mutex);
            syncConfFile(i);
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
    if (!confFile)
        return QString();
    return confFile->name;
}

bool QConfFileSettingsPrivate::isWritable() const
{
    if (format > QSettings::IniFormat && !writeFunc)
        return false;

    QConfFile *confFile = confFiles[spec];
    if (!confFile)
        return false;

    if (QFile::exists(confFile->name)) {
        QFile file(confFile->name);;
        return file.open(QFile::ReadWrite);
    } else {
        // we use a temporary file to avoid race conditions
        QTemporaryFile file(confFile->name);
        return file.open();
    }
}

void QConfFileSettingsPrivate::syncConfFile(int confFileNo)
{
    QConfFile *confFile = confFiles[confFileNo];
    bool readOnly = confFile->addedKeys.isEmpty() && confFile->removedKeys.isEmpty();
    bool ok;

    /*
        We can often optimize the read-only case, if the file on disk
        hasn't changed.
    */
    if (readOnly) {
        QFileInfo fileInfo(confFile->name);
        if (confFile->size == fileInfo.size() && confFile->timeStamp == fileInfo.lastModified())
            return;
    }

    /*
        Open the configuration file and try to use it using a named
        semaphore on Windows and an advisory lock on Unix-based
        systems. This protect us against other QSettings instances
        trying to access the same file from other threads or
        processes.

        As it stands now, the locking mechanism doesn't work for
        .plist files.
    */
    QFile file(confFile->name);

    if (!readOnly)
        file.open(QFile::ReadWrite);
    if (!file.isOpen())
        file.open(QFile::ReadOnly);

#ifdef Q_OS_WIN
    HANDLE semaphore = 0;
    static const int FileLockSemMax = 50;
    int numLocks = readOnly ? 1 : FileLockSemMax;

    if (file.isOpen()) {
        QString semName = QLatin1String("QSettings semaphore ");
        semName.append(file.fileName());

        QT_WA( {
            semaphore = CreateSemaphoreW(0, FileLockSemMax, FileLockSemMax, reinterpret_cast<const wchar_t *>(semName.utf16()));
        } , {
            semaphore = CreateSemaphoreA(0, FileLockSemMax, FileLockSemMax, semName.toLocal8Bit());
        } );

        if (semaphore) {
            for (int i = 0; i < numLocks; ++i)
                WaitForSingleObject(semaphore, INFINITE);
        } else {
            setStatus(QSettings::AccessError);
            return;
        }
    }
#else
    if (file.isOpen())
        unixLock(file.handle(), readOnly ? F_RDLCK : F_WRLCK);
#endif

    /*
        We hold the lock. Let's reread the file if it has changed
        since last time we read it.
    */
    QFileInfo fileInfo(confFile->name);
    bool mustReadFile = true;

    if (!readOnly) {
        mustReadFile = (confFile->size != fileInfo.size()
                        || (confFile->size != 0 && confFile->timeStamp != fileInfo.lastModified()));
    }

    if (mustReadFile) {
        InternalSettingsMap newKeys;

        /*
            Files that we can't read (because of permissions or
            because they don't exist) are treated as empty files.
        */
        if (file.isReadable() && fileInfo.size() != 0) {
#ifdef Q_OS_MAC
            if (format == QSettings::NativeFormat) {
                ok = readPlistFile(confFile->name, &newKeys);
            } else
#endif
            {
                if (format <= QSettings::IniFormat) {
                    ok = readIniFile(file, &newKeys);
                } else {
                    if (readFunc) {
                        QSettings::SettingsMap tempNewKeys;
                        ok = readFunc(file, tempNewKeys);

                        if (ok) {
                            QSettings::SettingsMap::const_iterator i = tempNewKeys.constBegin();
                            while (i != tempNewKeys.constEnd()) {
                                newKeys.insert(QSettingsKey(i.key(), caseSensitivity), i.value());
                                ++i;
                            }
                        }
                    } else {
                        ok = false;
                    }
                }
            }

            if (!ok)
                setStatus(QSettings::FormatError);
        }

        confFile->originalKeys = newKeys;
        confFile->size = fileInfo.size();
        confFile->timeStamp = fileInfo.lastModified();
    }

    /*
        We also need to save the file. We still hold the file lock,
        so everything is under control.
    */
    if (!readOnly) {
        InternalSettingsMap mergedKeys = confFile->mergedKeyMap();

        if (file.isWritable()) {
#ifdef Q_OS_MAC
            if (format == QSettings::NativeFormat) {
                ok = writePlistFile(confFile->name, mergedKeys);
            } else
#endif
            {
                file.seek(0); // shouldn't be necessary
                file.resize(0);

                if (format <= QSettings::IniFormat) {
                    ok = writeIniFile(file, mergedKeys);
                } else {
                    if (writeFunc) {
                        QSettings::SettingsMap tempOriginalKeys;

                        InternalSettingsMap::const_iterator i = mergedKeys.constBegin();
                        while (i != mergedKeys.constEnd()) {
                            tempOriginalKeys.insert(i.key(), i.value());
                            ++i;
                        }
                        ok = writeFunc(file, tempOriginalKeys);
                    } else {
                        ok = false;
                    }
                }
            }
        } else {
            ok = false;
        }

        if (ok) {
            confFile->originalKeys = mergedKeys;
            confFile->addedKeys.clear();
            confFile->removedKeys.clear();

            QFileInfo fileInfo(confFile->name);
            confFile->size = fileInfo.size();
            confFile->timeStamp = fileInfo.lastModified();
        } else {
            setStatus(QSettings::AccessError);
        }
    }

    /*
        Release the file lock.
    */
#ifdef Q_OS_WIN
    if (semaphore != 0) {
        ReleaseSemaphore(semaphore, numLocks, 0);
        CloseHandle(semaphore);
    }
#endif
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

    QByteArray linein;
    do {
        linein = device.readLine();
    } while (!linein.isEmpty() && (linein.at(0)=='\r' || linein.at(0)=='\n')); // skip blanks here
    int posin = 0;

    while (posin < linein.length()) {
        ch = linein.at(posin++);
    process_ch:
        MAYBE_GROW();

        switch (ch) {
        case '"':
            data[pos++] = '"';
            while (posin < linein.length() && (ch=linein.at(posin++)) != '"') {
                MAYBE_GROW();

                if (static_cast<signed char>(ch) == -1)
                    goto end;

                if (ch == '\\') {
                    data[pos++] = '\\';
                    if (posin >= linein.length())
                        goto end;
                    ch = linein.at(posin++);
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
            /*
            if (!device.getChar(&ch2))
                goto end;
            if ((ch2 != '\n' && ch2 != '\r') || ch == ch2)
                device.ungetChar(ch2);
            */
            // never tested, this was already disabled when getChar was replaced
            if (posin >= linein.length())
                goto end;
            ch2 = linein.at(posin++);
            if ((ch2 != '\n' && ch2 != '\r') || ch == ch2)
                --posin;
#endif
            goto end;
            break;
        case '\\':
            if (posin >= linein.length())
                goto end;
            ch = linein.at(posin++);

            if (ch == '\n' || ch == '\r') {
                if (posin < linein.length()) {
                    ch2 = linein.at(posin++);
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
            while (posin < linein.length()) {
                ch = linein.at(posin++);
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
bool QConfFileSettingsPrivate::readIniFile(QIODevice &device, InternalSettingsMap *map)
{
    QString currentSection;
    bool currentSectionIsLowercase = true;
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
                currentSectionIsLowercase = iniUnescapedKey(iniSection, 0, iniSection.size(),
                                                            currentSection);
                currentSection += QLatin1Char('/');
            }
        } else {
            if (equalsCharPos < 1) {
                ok = false;
                continue;
            }

            QString key = currentSection;
            bool keyIsLowercase = (iniUnescapedKey(line, 0, equalsCharPos, key)
                                   && currentSectionIsLowercase);

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
                key is already in lowercase.
            */
            map->insert(QSettingsKey(key, keyIsLowercase ? Qt::CaseSensitive : Qt::CaseInsensitive),
                        variant);
        }
    }

    return ok;
}

bool QConfFileSettingsPrivate::writeIniFile(QIODevice &device, const InternalSettingsMap &map)
{
    typedef QMap<QString, QVariantMap> IniMap;
    IniMap iniMap;
    IniMap::const_iterator i;

#ifdef Q_OS_WIN
    const char * const eol = "\r\n";
#else
    const char eol = '\n';
#endif

    for (InternalSettingsMap::const_iterator j = map.constBegin(); j != map.constEnd(); ++j) {
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
            realSection.prepend(eol);
        realSection += eol;

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
            block += eol;
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
    \reentrant

    Users normally expect an application to remember its settings
    (window sizes and positions, options, etc.) across sessions. This
    information is often stored in the system registry on Windows,
    and in XML preferences files on Mac OS X. On Unix systems, in the
    absence of a standard, many applications (including the KDE
    applications) use INI text files.

    QSettings is an abstraction around these technologies, enabling
    you to save and restore application settings in a portable
    manner. It also supports \l{registerFormat()}{custom storage
    formats}.

    QSettings's API is based on QVariant, allowing you to save
    most value-based types, such as QString, QRect, and QImage,
    with the minimum of effort.

    \tableofcontents section1

    \section1 Basic Usage

    When creating a QSettings object, you must pass the name of your
    company or organization as well as the name of your application.
    For example, if your product is called Star Runner and your
    company is called MySoft, you would construct the QSettings
    object as follows:

    \quotefromfile snippets/settings/settings.cpp
    \skipuntil snippet_ctor1
    \skipline {
    \printline QSettings settings

    QSettings objects can be created either on the stack or on
    the heap (i.e. using \c new). Constructing and destroying a
    QSettings object is very fast.

    If you use QSettings from many places in your application, you
    might want to specify the organization name and the application
    name using QCoreApplication::setOrganizationName() and
    QCoreApplication::setApplicationName(), and then use the default
    QSettings constructor:

    \skipuntil snippet_ctor2
    \skipline {
    \printline setOrganizationName
    \printline setOrganizationDomain
    \printline setApplicationName
    \dots
    \printline QSettings settings;

    (Here, we also specify the organization's Internet domain. When
    the Internet domain is set, it is used on Mac OS X instead of the
    organization name, since Mac OS X applications conventionally use
    Internet domains to identify themselves. If no domain is set, a
    fake domain is derived from the organization name. See the
    \l{Platform-Specific Notes} below for details.)

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
    \c toColor(), \c toImage(), or \c toPixmap() functions in QVariant.

    Instead, you can use the QVariant::value() or the qVariantValue()
    template function. For example:

    \code
        QSettings settings("MySoft", "Star Runner");
        QColor color = settings.value("DataPump/bgcolor").value<QColor>();
    \endcode

    The inverse conversion (e.g., from QColor to QVariant) is
    automatic for all data types supported by QVariant, including
    GUI-related types:

    \code
        QSettings settings("MySoft", "Star Runner");
        QColor color = palette().background().color();
        settings.setValue("DataPump/bgcolor", color);
    \endcode

    Custom types registered using qRegisterMetaType() and
    qRegisterMetaTypeStreamOperators() can be stored using QSettings.

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

    Let's assume that you have created a QSettings object with the
    organization name MySoft and the application name Star Runner.
    When you look up a value, up to four locations are searched in
    that order:

    \list 1
    \o a user-specific location for the Star Runner application
    \o a user-spefific location for all applications by MySoft
    \o a system-wide location for the Star Runner application
    \o a system-wide location for all applications by MySoft
    \endlist

    (See \l{Platform-Specific Notes} below for information on what
    these locations are on the different platforms supported by Qt.)

    If a key cannot be found in the first location, the search goes
    on in the second location, and so on. This enables you to store
    system-wide or organization-wide settings and to override them on
    a per-user or per-application basis. To turn off this mechanism,
    call setFallbacksEnabled(false).

    Although keys from all four locations are available for reading,
    only the first file (the user-specific location for the
    application at hand) is accessible for writing. To write to any
    of the other files, omit the application name and/or specify
    QSettings::SystemScope (as opposed to QSettings::UserScope, the
    default).

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
    native API, you can pass QSettings::IniFormat as the first
    argument to the QSettings constructor, followed by the scope, the
    organization name, and the application name:

    \skipline {
    \printline /settings\(.*,$/
    \printline );

    Sometimes you do want to access settings stored in a specific
    file or registry path. In that case, you can use a constructor
    that takes a file name (or registry path) and a file format. For
    example:

    \skipline }
    \skipline {
    \printline /QSettings settings.*Ini/

    The file format can either be QSettings::IniFormat or QSettings::NativeFormat.
    On Mac OS X, the native format is an XML-based format called \e
    plist. On Windows, the native format is the Windows registry, and
    the first argument is a path in the registry rather than a file
    name, for example:

    \skipline }
    \skipline {
    \printline HKEY
    \printline Native

    On Unix systems, QSettings::IniFormat and QSettings::NativeFormat
    have the same meaning.

    The \l{tools/settingseditor}{Settings Editor} example lets you
    experiment with different settings location and with fallbacks
    turned on or off.

    \section1 Restoring the State of a GUI Application

    QSettings is often used to store the state of a GUI
    application. The following example illustrates how to use we
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

    QSettings can safely be used from different processes (which can
    be different instances of your application running at the same
    time or different applications altogether) to read and write to
    the same system locations. It uses advisory file locking and a
    smart merging algorithm to ensure data integrity. Changes
    performed by another process aren't visible in the current
    process until sync() is called.

    \section1 Platform-Specific Notes

    As mentioned in the \l{Fallback Mechanism} section, QSettings
    stores settings for an application in up to four locations,
    depending on whether the settings are user-specific or
    system-wide and whether the the settings are application-specific
    or organization-wide. For simplicity, we're assuming the
    organization is called MySoft and the application is called Star
    Runner.

    On Unix systems, if the file format is NativeFormat, the
    following files are used by default:

    \list 1
    \o \c{$HOME/.config/MySoft/Star Runner.conf}
    \o \c{$HOME/.config/MySoft.conf}
    \o \c{/etc/xdg/MySoft/Star Runner.conf}
    \o \c{/etc/xdg/MySoft.conf}
    \endlist

    On Mac OS X versions 10.2 and 10.3, these files are used by
    default:

    \list 1
    \o \c{$HOME/Library/Preferences/com.MySoft.Star Runner.plist}
    \o \c{$HOME/Library/Preferences/com.MySoft.plist}
    \o \c{/Library/Preferences/com.MySoft.Star Runner.plist}
    \o \c{/Library/Preferences/com.MySoft.plist}
    \endlist

    On Windows, NativeFormat settings are stored in the following
    registry paths:

    \list 1
    \o \c{HKEY_CURRENT_USER\Software\MySoft\Star Runner}
    \o \c{HKEY_CURRENT_USER\Software\MySoft}
    \o \c{HKEY_LOCAL_MACHINE\Software\MySoft\Star Runner}
    \o \c{HKEY_LOCAL_MACHINE\Software\MySoft}
    \endlist

    If the file format is IniFormat, the following files are
    used on Unix and Mac OS X:

    \list 1
    \o \c{$HOME/.config/MySoft/Star Runner.ini}
    \o \c{$HOME/.config/MySoft.ini}
    \o \c{/etc/xdg/MySoft/Star Runner.ini}
    \o \c{/etc/xdg/MySoft.ini}
    \endlist

    On Windows, the following files are used:

    \list 1
    \o \c{%APPDATA%\MySoft\Star Runner.ini}
    \o \c{%APPDATA%\MySoft.ini}
    \o \c{%COMMON_APPDATA%\MySoft\Star Runner.ini}
    \o \c{%COMMON_APPDATA%\MySoft.ini}
    \endlist

    The \c %APPDATA% path is usually \tt{C:\\Documents and
    Settings\\\e{User Name}\\Application Data}; the \c
    %COMMON_APPDATA% path is usually \tt{C:\\Documents and
    Settings\\All Users\\Application Data}.

    The paths for the \c .ini and \c .conf files can be changed using
    setPath(). On Unix and Mac OS X, the user can override them by by
    setting the \c XDG_CONFIG_HOME environment variable; see
    setPath() for details.

    While QSettings attempts to smooth over the differences between
    the different supported platforms, there are still a few
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

    \o  On Mac OS X, the CFPreferences API used by QSettings expects
        Internet domain names rather than organization names. To
        provide a uniform API, QSettings derives a fake domain name
        from the organization name (unless the organization name
        already is a domain name, e.g. OpenOffice.org). The algorithm
        appends ".com" to the company name and replaces spaces and
        other illegal characters with hyphens. If you want to specify
        a different domain name, call
        QCoreApplication::setOrganizationDomain(),
        QCoreApplication::setOrganizationName(), and
        QCoreApplication::setApplicationName() in your \c main()
        function and then use the default QSettings constructor.
        Another solution is to use preprocessor directives, for
        example:

        \code
        #ifdef Q_WS_MAC
            QSettings settings("grenoullelogique.fr", "Squash");
        #else
            QSettings settings("Grenoulle Logique", "Squash");
        #endif
        \endcode
    \endlist

    \sa QVariant, QSessionManager,
        {tools/settingseditor}{Settings Editor Example}
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
                         API; on Unix, this means textual
                         configuration files in INI format.
    \value IniFormat  Store the settings in INI files.
    \value InvalidFormat Special value returned by registerFormat().
    \omitvalue CustomFormat1
    \omitvalue CustomFormat2
    \omitvalue CustomFormat3
    \omitvalue CustomFormat4
    \omitvalue CustomFormat5
    \omitvalue CustomFormat6
    \omitvalue CustomFormat7
    \omitvalue CustomFormat8
    \omitvalue CustomFormat9
    \omitvalue CustomFormat10
    \omitvalue CustomFormat11
    \omitvalue CustomFormat12
    \omitvalue CustomFormat13
    \omitvalue CustomFormat14
    \omitvalue CustomFormat15
    \omitvalue CustomFormat16

    On Unix, NativeFormat and IniFormat mean the same thing, except
    that the file extension is different (\c .conf for NativeFormat,
    \c .ini for IniFormat).

    The INI file format is a Windows file format that Qt supports on
    all platforms. In the absence of an INI standard, we try to
    follow what Microsoft does, with the following exceptions:

    \list
    \o  If you store types that QVariant can't convert to QString
        (e.g., QPoint, QRect, and QSize), Qt uses an \c{@}-based
        syntax to encode the type. For example:

        \code
        pos = @Point(100 100)
        \endcode

        To minimize compatibility issues, any \c @ that doesn't
        appear at the first position in the value or that isn't
        followed by a Qt type (\c Point, \c Rect, \c Size, etc.) is
        treated as a normal character.

    \o  Although backslash is a special character in INI files, most
        Windows applications don't escape backslashes (\c{\}) in file
        paths:

        \code
        windir = C:\Windows
        \endcode

        QSettings always treats backslash as a special character and
        provides no API for reading or writing such entries.

    \o  The INI file format has severe restrictions on the syntax of
        a key. Qt works around this by using \c % as an escape
        character in keys. In addition, if you save a top-level
        setting (a key with no slashes in it, e.g., "someKey"), it
        will appear in the INI file's "General" section. To avoid
        overwriting other keys, if you save something using the a key
        such as "General/someKey", the key will be located in the
        "%General" section, \e not in the "General" section.
    \endlist

    \sa registerFormat(), setPath()
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

    \sa setPath()
*/

#ifndef QT_NO_QOBJECT
/*!
    Constructs a QSettings object for accessing settings of the
    application called \a application from the organization called \a
    organization, and with parent \a parent.

    Example:
    \code
        QSettings settings("Moose Tech", "Facturo-Pro");
    \endcode

    The scope is QSettings::UserScope and the format is
    QSettings::NativeFormat.

    \sa {Fallback Mechanism}
*/
QSettings::QSettings(const QString &organization, const QString &application, QObject *parent)
    : QObject(*QSettingsPrivate::create(NativeFormat, UserScope, organization, application),
              parent)
{
}

/*!
    Constructs a QSettings object for accessing settings of the
    application called \a application from the organization called \a
    organization, and with parent \a parent.

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
    application called \a application from the organization called
    \a organization, and with parent \a parent.

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

    If \a format is QSettings::NativeFormat, the meaning of \a
    fileName depends on the platform. On Unix, \a fileName is the
    name of an INI file. On Mac OS X, \a fileName is the name of a
    .plist file. On Windows, \a fileName is a path in the system
    registry.

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
    QCoreApplication::setOrganizationName(),
    QCoreApplication::setOrganizationDomain(), and
    QCoreApplication::setApplicationName().

    The scope is QSettings::UserScope and the format is QSettings::NativeFormat.

    The code

    \code
        QSettings settings("Moose Soft", "Facturo-Pro");
    \endcode

    is equivalent to

    \code
        QCoreApplication::setOrganizationName("Moose Soft");
        QCoreApplication::setApplicationName("Facturo-Pro");
        QSettings settings;
    \endcode

    If QCoreApplication::setOrganizationName() and
    QCoreApplication::setApplicationName() has not been previously
    called, the QSettings object will not be able to read or write
    any settings, and status() will return AccessError.

    On Mac OS X, if both a name and an Internet domain are specified
    for the organization, the domain is preferred over the name. On
    other platforms, the name is preferred over the domain.

    \sa QCoreApplication::setOrganizationName(),
        QCoreApplication::setOrganizationDomain(),
        QCoreApplication::setApplicationName()
*/
QSettings::QSettings(QObject *parent)
    : QObject(*QSettingsPrivate::create(NativeFormat, UserScope,
#ifdef Q_OS_MAC
                                        QCoreApplication::organizationDomain().isEmpty()
                                            ? QCoreApplication::organizationName()
                                            : QCoreApplication::organizationDomain()
#else
                                        QCoreApplication::organizationName().isEmpty()
                                            ? QCoreApplication::organizationDomain()
                                            : QCoreApplication::organizationName()
#endif
                                        , QCoreApplication::applicationName()),
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
#ifdef QT_NO_QOBJECT
    delete d;
#endif
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
    return value(QLatin1String("size")).toInt();
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

    \warning This function is not perfectly reliable, because the
    file permissions can change at any time.

    \sa fileName(), status(), sync()
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
    \obsolete

    Use setPath() instead.

    \oldcode
        setSystemIniPath(path);
    \newcode
        setPath(QSettings::NativeFormat, QSettings::SystemScope, path);
        setPath(QSettings::IniFormat, QSettings::SystemScope, path);
    \endcode
*/
void QSettings::setSystemIniPath(const QString &dir)
{
    setPath(IniFormat, SystemScope, dir);
#if !defined(Q_OS_WIN) && !defined(Q_OS_MAC)
    setPath(NativeFormat, SystemScope, dir);
#endif
}

/*!
    \obsolete

    Use setPath() instead.
*/

void QSettings::setUserIniPath(const QString &dir)
{
    setPath(IniFormat, UserScope, dir);
#if !defined(Q_OS_WIN) && !defined(Q_OS_MAC)
    setPath(NativeFormat, UserScope, dir);
#endif
}

/*!
    \since 4.1

    Sets the path used for storing settings for the given \a format
    and \a scope, to \a path. The \a format can be a custom format.

    The table below summarizes the default values:

    \table
    \header \o Platform      \o Format                       \o Scope       \o Path
    \row    \o{1,2} Windows  \o{1,2} IniFormat               \o UserScope   \o \c %APPDATA%
    \row                                                     \o SystemScope \o \c %COMMON_APPDATA%
    \row    \o{1,2} Unix     \o{1,2} NativeFormat, IniFormat \o UserScope   \o \c $HOME/.config
    \row                                                     \o SystemScope \o \c /etc/xdg
    \row    \o{1,2} Mac OS X \o{1,2} IniFormat               \o UserScope   \o \c $HOME/.config
    \row                                                     \o SystemScope \o \c /etc/xdg
    \endtable

    The default UserScope paths on Unix and Mac OS X (\c
    $HOME/.config) can be overridden by the user by setting the \c
    XDG_CONFIG_HOME environment variable. The default SystemScope
    paths on Unix and Mac OS X (\c /etc/xdg) can be overridden when
    building the Qt library using the \c configure script's \c
    --sysconfdir flag (see QLibraryInfo for details).

    Setting the NativeFormat paths on Windows and Mac OS X has no
    effect.

    \warning This function doesn't affect existing QSettings objects.

    \sa registerFormat()
*/
void QSettings::setPath(Format format, Scope scope, const QString &path)
{
    QMutexLocker locker(globalMutex());
    PathHash *pathHash = pathHashFunc();
    pathHash->insert(pathHashKey(format, scope), path + QDir::separator());
}

/*!
    \typedef QSettings::SettingsMap

    Typedef for QMap<QString, QVariant>.

    \sa registerFormat()
*/

/*!
    \typedef QSettings::ReadFunc

    Typedef for a pointer to a function with the following signature:

    \code
        bool myReadFunc(QIODevice &device, QSettings::SettingsMap &map);
    \endcode

    \sa WriteFunc, registerFormat()
*/

/*!
    \typedef QSettings::WriteFunc

    Typedef for a pointer to a function with the following signature:

    \code
        bool myWriteFunc(QIODevice &device, const QSettings::SettingsMap &map);
    \endcode

    \sa ReadFunc, registerFormat()
*/

/*!
    \since 4.1

    Registers a custom storage format. On success, returns a special
    Format value that can then be passed to the QSettings constuctor.
    On failure, returns InvalidFormat.

    The \a extension is the file
    extension associated to the format (without the '.').

    The \a readFunc and \a writeFunc parameters are pointers to
    functions that read and write a set of (key, value) pairs. The
    QIODevice parameter to the read and write functions is always
    opened in binary mode (i.e., without the QIODevice::Text flag).

    The \a caseSensitivity parameter specifies whether keys are case
    sensitive or not. This makes a difference when looking up values
    using QSettings. The default is case sensitive.

    By default, if you use one of the constructors that work in terms
    of an organization name and an application name, the file system
    locations used are the same as for IniFormat. Use setPath() to
    specify other locations.

    Example:

    \code
        bool readXmlFile(QIODevice &device, QSettings::SettingsMap &map);
        bool writeXmlFile(QIODevice &device, const QSettings::SettingsMap &map);

        int main(int argc, char *argv[])
        {
            const QSettings::Format XmlFormat =
                    QSettings::registerFormat("xml", readXmlFunc, writeXmlFunc);

            QSettings settings(XmlFormat, QSettings::UserSettings, "MySoft",
                               "Star Runner");

            ...
        }
    \endcode

    \sa setPath()
*/
QSettings::Format QSettings::registerFormat(const QString &extension, ReadFunc readFunc,
                                            WriteFunc writeFunc,
                                            Qt::CaseSensitivity caseSensitivity)
{
    QMutexLocker locker(globalMutex());
    CustomFormatVector *customFormatVector = customFormatVectorFunc();
    int index = customFormatVector->size();
    if (index == 16) // the QSettings::Format enum has room for 16 custom formats
        return QSettings::InvalidFormat;

    QConfFileCustomFormat info;
    info.extension = QLatin1Char('.');
    info.extension += extension;
    info.readFunc = readFunc;
    info.writeFunc = writeFunc;
    info.caseSensitivity = caseSensitivity;
    customFormatVector->append(info);

    return QSettings::Format((int)QSettings::CustomFormat1 + index);
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

    \value Unix Unix systems (X11 and Qtopia Core)
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
