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

#include "qvector.h"
#include "qmap.h"
#include "qt_windows.h"

#include "qsettings.h"
#include "qsettings_p.h"

/*  Keys are stored in QStrings. If the variable name starts with 'u', this is a "user"
    key, ie. "foo/bar/alpha/beta". If the variable name starts with 'r', this is a "registry"
    key, ie. "\foo\bar\alpha\beta". */

/*******************************************************************************
** Private classes
*/

struct RegistryLocation {
    RegistryLocation()
        : handle(0), readOnly(false) {}

    QString key;
    HKEY handle;
    bool readOnly;
};

typedef QVector<RegistryLocation> RegistryLocationList;

class QWinSettingsPrivate : public QSettingsPrivate
{
public:
    QWinSettingsPrivate(QSettings::Scope scope, const QString &organization,
                        const QString &application);
    QWinSettingsPrivate(QString rKey);
    ~QWinSettingsPrivate();

    void remove(const QString &uKey);
    void set(const QString &uKey, const QVariant &value);
    bool get(const QString &uKey, QVariant *value) const;
    QStringList children(const QString &uKey, ChildSpec spec) const;
    void clear();
    void sync();
    void flush();
    bool isWritable() const;
    HKEY writeHandle() const;
    bool readKey(HKEY parentHandle, const QString &rSubKey, QVariant *value) const;
    QString fileName() const;

private:
    RegistryLocationList regList; // list of registry locations to search for keys
    bool deleteWriteHandleOnExit;
};

typedef QMap<QString, QString> NameSet;

/*******************************************************************************
** Some convenience functions
*/

static QString keyPath(const QString &rKey)
{
    int idx = rKey.lastIndexOf(QLatin1Char('\\'));
    if (idx == -1)
        return QString();
    return rKey.left(idx + 1);
}

static QString keyName(const QString &rKey)
{
    int idx = rKey.lastIndexOf(QLatin1Char('\\'));
    if (idx == -1)
        return rKey;

    QString res(rKey.mid(idx + 1));
    if (res == "Default" || res == ".")
        res = "";
    return res;
}

static QString escapedKey(QString uKey)
{
    QChar *data = uKey.data();
    int l = uKey.length();
    for (int i = 0; i < l; ++i) {
        ushort &ucs = data[i].unicode();
        if (ucs == '\\')
            ucs = '/';
        else if (ucs == '/')
            ucs = '\\';
    }
    return uKey;
}

static QString unescapedKey(QString rKey)
{
    return escapedKey(rKey);
}

static void mergeKeySets(NameSet *dest, const NameSet &src)
{
    NameSet::const_iterator it = src.constBegin();
    for (; it != src.constEnd(); ++it)
        dest->insert(unescapedKey(it.key()), QString());
}

static void mergeKeySets(NameSet *dest, const QStringList &src)
{
    QStringList::const_iterator it = src.constBegin();
    for (; it != src.constEnd(); ++it)
        dest->insert(unescapedKey(*it), QString());
}

/*******************************************************************************
** Wrappers for the insane windows registry API
*/

static QString errorCodeToString(DWORD errorCode)
{
    QString result;
    char *data = 0;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    0, errorCode, 0,
                    (char*)&data, 0, 0);
    result = QString::fromLocal8Bit(data);
    if (data != 0)
        LocalFree(data);

    if (result.endsWith("\n"))
        result.truncate(result.length() - 1);

    return result;
}

// Open a key with the specified perms
static HKEY openKey(HKEY parentHandle, REGSAM perms, const QString &rSubKey)
{
    HKEY resultHandle = 0;

    LONG res;
    QT_WA( {
        res = RegOpenKeyExW(parentHandle, reinterpret_cast<const wchar_t *>(rSubKey.utf16()),
                            0, perms, &resultHandle);
    } , {
        res = RegOpenKeyExA(parentHandle, rSubKey.toLocal8Bit(),
                            0, perms, &resultHandle);
    } );

    if (res == ERROR_SUCCESS)
        return resultHandle;

    return 0;
}

// Open a key with the specified perms, create it if it does not exist
static HKEY createOrOpenKey(HKEY parentHandle, REGSAM perms, const QString &rSubKey)
{
    // try to open it
    HKEY resultHandle = openKey(parentHandle, perms, rSubKey);
    if (resultHandle != 0)
        return resultHandle;

    // try to create it
    LONG res;
    QT_WA( {
        res = RegCreateKeyExW(parentHandle, reinterpret_cast<const wchar_t *>(rSubKey.utf16()), 0, 0,
                              REG_OPTION_NON_VOLATILE, perms, 0, &resultHandle, 0);
    } , {
        res = RegCreateKeyExA(parentHandle, rSubKey.toLocal8Bit(), 0, 0,
                              REG_OPTION_NON_VOLATILE, perms, 0, &resultHandle, 0);
    } );

    if (res == ERROR_SUCCESS)
        return resultHandle;

    qWarning("QSettings: failed to create subkey \"%s\": %s",
            rSubKey.toLatin1().data(), errorCodeToString(res).toLatin1().data());

    return 0;
}

// Open or create a key in read-write mode if possible, otherwise read-only
static HKEY createOrOpenKey(HKEY parentHandle, const QString &rSubKey, bool *readOnly)
{
    // try to open it read/write
    HKEY resultHandle = openKey(parentHandle, KEY_ALL_ACCESS, rSubKey);
    if (resultHandle != 0) {
        if (readOnly != 0)
            *readOnly = false;
        return resultHandle;
    }

    // try to open it read/only
    resultHandle = openKey(parentHandle, KEY_READ, rSubKey);
    if (resultHandle != 0) {
        if (readOnly != 0)
            *readOnly = true;
        return resultHandle;
    }

    // try to create it
    resultHandle = createOrOpenKey(parentHandle, KEY_ALL_ACCESS, rSubKey);
    if (resultHandle != 0) {
        if (readOnly != 0)
            *readOnly = false;
        return resultHandle;
    }

     return 0;
}

/* Open or create a key in read-write mode if possible, otherwise read-only,
   and initialize loc with it */
static bool createOrOpenKey(HKEY parentHandle, const QString &rSubKey, RegistryLocation *loc)
{
    Q_ASSERT(loc != 0);

    loc->key = rSubKey;
    loc->handle = createOrOpenKey(parentHandle, loc->key, &loc->readOnly);
    return loc->handle != 0;
}

static QStringList childKeysOrGroups(HKEY parentHandle, QSettingsPrivate::ChildSpec spec)
{
    QStringList result;
    LONG res;
    DWORD numKeys;
    DWORD maxKeySize;
    DWORD numSubgroups;
    DWORD maxSubgroupSize;

    // Find the number of keys and subgroups, as well as the max of their lengths.
    QT_WA( {
        res = RegQueryInfoKeyW(parentHandle, 0, 0, 0, &numSubgroups, &maxSubgroupSize, 0,
                               &numKeys, &maxKeySize, 0, 0, 0);
    }, {
        res = RegQueryInfoKeyA(parentHandle, 0, 0, 0, &numSubgroups, &maxSubgroupSize, 0,
                               &numKeys, &maxKeySize, 0, 0, 0);
    } );

    if (res != ERROR_SUCCESS) {
        qWarning("QSettings: RegQueryInfoKey() failed: %s", errorCodeToString(res).toLatin1().data());
        return result;
    }

    ++maxSubgroupSize;
    ++maxKeySize;

    int n;
    int m;
    if (spec == QSettingsPrivate::ChildKeys) {
        n = numKeys;
        m = maxKeySize;
    } else {
        n = numSubgroups;
        m = maxSubgroupSize;
    }

    /* Windows NT/2000/XP: The size does not include the terminating null character.
       Windows Me/98/95: The size includes the terminating null character. */
    ++m;

    // Get the list
    QByteArray buff(m*sizeof(ushort), 0);
    for (int i = 0; i < n; ++i) {
        QString item;
        QT_WA( {
            DWORD l = buff.size() / sizeof(ushort);
            if (spec == QSettingsPrivate::ChildKeys) {
                res = RegEnumValueW(parentHandle, i,
                                    reinterpret_cast<wchar_t *>(buff.data()),
                                    &l, 0, 0, 0, 0);
            } else {
                res = RegEnumKeyExW(parentHandle, i,
                                    reinterpret_cast<wchar_t *>(buff.data()),
                                    &l, 0, 0, 0, 0);
            }
            if (res == ERROR_SUCCESS)
                item = QString::fromUtf16(reinterpret_cast<ushort*>(buff.data()), l);
        }, {
            DWORD l = buff.size();
            if (spec == QSettingsPrivate::ChildKeys)
                res = RegEnumValueA(parentHandle, i, buff.data(), &l, 0, 0, 0, 0);
            else
                res = RegEnumKeyExA(parentHandle, i, buff.data(), &l, 0, 0, 0, 0);
            if (res == ERROR_SUCCESS)
                item = QString::fromLocal8Bit(buff.data(), l);
        } );

        if (res != ERROR_SUCCESS) {
            qWarning("QSettings: RegEnumValue failed: %s", errorCodeToString(res).toLatin1().data());
            continue;
        }
        if (item.isEmpty())
            item = QLatin1String(".");
        result.append(item);
    }
    return result;
}

static void allKeys(HKEY parentHandle, const QString &rSubKey, NameSet *result)
{
    HKEY handle = openKey(parentHandle, KEY_READ, rSubKey);
    if (handle == 0)
        return;

    QStringList childKeys = childKeysOrGroups(handle, QSettingsPrivate::ChildKeys);
    QStringList childGroups = childKeysOrGroups(handle, QSettingsPrivate::ChildGroups);
    RegCloseKey(handle);

    for (int i = 0; i < childKeys.size(); ++i) {
        QString s = rSubKey;
        if (!s.isEmpty())
            s += QLatin1Char('\\');
        s += childKeys.at(i);
        result->insert(s, QString());
    }

    for (int i = 0; i < childGroups.size(); ++i) {
        QString s = rSubKey;
        if (!s.isEmpty())
            s += QLatin1Char('\\');
        s += childGroups.at(i);
        allKeys(parentHandle, s, result);
    }
}

static void deleteChildGroups(HKEY parentHandle)
{
    QStringList childGroups = childKeysOrGroups(parentHandle, QSettingsPrivate::ChildGroups);

    for (int i = 0; i < childGroups.size(); ++i) {
        QString group = childGroups.at(i);

        // delete subgroups in group
        HKEY childGroupHandle = openKey(parentHandle, KEY_ALL_ACCESS, group);
        if (childGroupHandle == 0)
            continue;
        deleteChildGroups(childGroupHandle);
        RegCloseKey(childGroupHandle);

        // delete group itself
        LONG res;
        QT_WA( {
            res = RegDeleteKeyW(parentHandle, reinterpret_cast<const wchar_t *>(group.utf16()));
        }, {
            res = RegDeleteKeyA(parentHandle, group.toLocal8Bit());
        } );
        if (res != ERROR_SUCCESS) {
            qWarning("QSettings: RegDeleteKey failed on subkey \"%s\": %s",
                      group.toLatin1().data(), errorCodeToString(res).toLatin1().data());
            return;
        }
    }
}

/*******************************************************************************
** class QWinSettingsPrivate
*/

QWinSettingsPrivate::QWinSettingsPrivate(QSettings::Scope scope, const QString &organization,
                                         const QString &application)
{
    deleteWriteHandleOnExit = false;

    if (!organization.isEmpty()) {
        QString prefix = QLatin1String("Software\\") + organization;
        QString orgPrefix = prefix + QLatin1String("\\OrganizationDefaults");
        QString appPrefix = prefix + QLatin1Char('\\') + application;

        RegistryLocation loc;
        if (scope == QSettings::UserScope) {
            if (!application.isEmpty()) {
                if (createOrOpenKey(HKEY_CURRENT_USER, appPrefix, &loc))
                    regList.append(loc);
            }

            if (createOrOpenKey(HKEY_CURRENT_USER, orgPrefix, &loc))
                regList.append(loc);
        }

        if (!application.isEmpty()) {
            if (createOrOpenKey(HKEY_LOCAL_MACHINE, appPrefix, &loc))
                regList.append(loc);
        }

        if (createOrOpenKey(HKEY_LOCAL_MACHINE, orgPrefix, &loc))
            regList.append(loc);
    }

    if (regList.isEmpty())
        setStatus(QSettings::AccessError);
}

QWinSettingsPrivate::QWinSettingsPrivate(QString rPath)
{
    deleteWriteHandleOnExit = false;

    if (rPath.startsWith("\\"))
        rPath = rPath.mid(1);

    RegistryLocation loc;
    if (rPath.startsWith("HKEY_CURRENT_USER\\")) {
        if (createOrOpenKey(HKEY_CURRENT_USER, rPath.mid(18), &loc))
            regList.append(loc);
    } else if (rPath == QLatin1String("HKEY_CURRENT_USER")) {
        if (createOrOpenKey(HKEY_CURRENT_USER, QString(), &loc))
            regList.append(loc);
    } else if (rPath.startsWith("HKEY_LOCAL_MACHINE\\")) {
        if (createOrOpenKey(HKEY_LOCAL_MACHINE, rPath.mid(19), &loc))
            regList.append(loc);
    } else if (rPath == QLatin1String("HKEY_LOCAL_MACHINE")) {
        if (createOrOpenKey(HKEY_LOCAL_MACHINE, QString(), &loc))
            regList.append(loc);
    } else {
        if (createOrOpenKey(HKEY_LOCAL_MACHINE, QString(), &loc))
            regList.append(loc);
    }

    if (regList.isEmpty())
        setStatus(QSettings::AccessError);
}

bool QWinSettingsPrivate::readKey(HKEY parentHandle, const QString &rSubKey, QVariant *value) const
{
    QString rSubkeyName = keyName(rSubKey);
    QString rSubkeyPath = keyPath(rSubKey);

    // open a handle on the subkey
    HKEY handle = openKey(parentHandle, KEY_READ, rSubkeyPath);
    if (handle == 0)
        return false;

    // get the size and type of the value
    DWORD dataType;
    DWORD dataSize;
    LONG res;
    QT_WA( {
        res = RegQueryValueExW(handle, reinterpret_cast<const wchar_t *>(rSubkeyName.utf16()), 0, &dataType, 0, &dataSize);
    }, {
        res = RegQueryValueExA(handle, rSubkeyName.toLocal8Bit(), 0, &dataType, 0, &dataSize);
    } );
    if (res != ERROR_SUCCESS) {
        RegCloseKey(handle);
        return false;
    }

    // get the value
    QByteArray data(dataSize, 0);
    QT_WA( {
        res = RegQueryValueExW(handle, reinterpret_cast<const wchar_t *>(rSubkeyName.utf16()), 0, 0,
                               reinterpret_cast<unsigned char*>(data.data()), &dataSize);
    }, {
        res = RegQueryValueExA(handle, rSubkeyName.toLocal8Bit(), 0, 0,
                               reinterpret_cast<unsigned char*>(data.data()), &dataSize);
    } );
    if (res != ERROR_SUCCESS) {
        RegCloseKey(handle);
        return false;
    }

    switch (dataType) {
        case REG_EXPAND_SZ:
        case REG_SZ: {
            QString s;
            QT_WA( {
                s = QString::fromUtf16(((const ushort*)data.constData()));
            }, {
                s = QString::fromLatin1(data.constData());
            } );

            if (value != 0)
                *value = stringToVariant(s);
            break;
        }

        case REG_MULTI_SZ: {
            QStringList l;
            int i = 0;
            for (;;) {
                QString s;
                QT_WA( {
                    s = QString::fromUtf16((const ushort*)data.constData() + i);
                }, {
                    s = QString::fromLatin1(data.constData() + i);
                } );
                i += s.length() + 1;

                if (s.isEmpty())
                    break;
                l.append(s);
            }
            if (value != 0)
                *value = stringListToVariantList(l);
            break;
        }

        case REG_NONE:
        case REG_BINARY: {
            QString s;
            QT_WA( {
                s = QString::fromUtf16((const ushort*)data.constData(), data.size()/2);
            }, {
                s = QString::fromLatin1(data.constData(), data.size());
            } );
            if (value != 0)
                *value = stringToVariant(s);
            break;
        }

        case REG_DWORD_BIG_ENDIAN:
        case REG_DWORD: {
            Q_ASSERT(data.size() == sizeof(int));
            int i;
            memcpy((char*)&i, data.constData(), sizeof(int));
            if (value != 0)
                *value = i;
            break;
        }

        default:
            qWarning("QSettings: unknown data %d type in windows registry", static_cast<int>(dataType));
            if (value != 0)
                *value = QVariant();
            break;
    }

    RegCloseKey(handle);
    return true;
}

HKEY QWinSettingsPrivate::writeHandle() const
{
    if (regList.isEmpty())
        return 0;
    const RegistryLocation &loc = regList.at(0);
    if (loc.readOnly == true)
        return 0;
    return loc.handle;
}

QWinSettingsPrivate::~QWinSettingsPrivate()
{
    if (deleteWriteHandleOnExit
            && !regList.isEmpty()
            && !regList.at(0).readOnly) {

        QString emptyKey;
        DWORD res;
        QT_WA( {
            res = RegDeleteKeyW(writeHandle(), reinterpret_cast<const wchar_t *>(emptyKey.utf16()));
        }, {
            res = RegDeleteKeyA(writeHandle(), emptyKey.toLocal8Bit());
        } );
        if (res != ERROR_SUCCESS) {
            qWarning("QSettings: failed to delete key \"%s\": %s",
                    regList.at(0).key.toLatin1().data(), errorCodeToString(res).toLatin1().data());
        }
    }

    for (int i = 0; i < regList.size(); ++i)
        RegCloseKey(regList.at(i).handle);
}

void QWinSettingsPrivate::remove(const QString &uKey)
{
    QString rKey = escapedKey(uKey);

    // try to delete value bar in key foo
    LONG res;
    HKEY handle = openKey(writeHandle(), KEY_ALL_ACCESS, keyPath(rKey));
    if (handle != 0) {
        QT_WA( {
            res = RegDeleteValueW(handle, reinterpret_cast<const wchar_t *>(keyName(rKey).utf16()));
        }, {
            res = RegDeleteValueA(handle, keyName(rKey).toLocal8Bit());
        } );
        RegCloseKey(handle);
    }

    // try to delete key foo/bar and all subkeys
    handle = openKey(writeHandle(), KEY_ALL_ACCESS, rKey);
    if (handle != 0) {
        deleteChildGroups(handle);

        if (rKey.isEmpty()) {
            QStringList childKeys = childKeysOrGroups(handle, QSettingsPrivate::ChildKeys);

            for (int i = 0; i < childKeys.size(); ++i) {
                QString group = childKeys.at(i);

                LONG res;
                QT_WA( {
                    res = RegDeleteValueW(handle, reinterpret_cast<const wchar_t *>(group.utf16()));
                }, {
                    res = RegDeleteValueA(handle, group.toLocal8Bit());
                } );
                if (res != ERROR_SUCCESS) {
                    qWarning("QSettings: RegDeleteValue failed on subkey \"%s\": %s",
                              group.toLatin1().data(), errorCodeToString(res).toLatin1().data());
                }
            }
        } else {
            QT_WA( {
                res = RegDeleteKeyW(writeHandle(), reinterpret_cast<const wchar_t *>(rKey.utf16()));
            }, {
                res = RegDeleteKeyA(writeHandle(), rKey.toLocal8Bit());
            } );

            if (res != ERROR_SUCCESS) {
                qWarning("QSettings: RegDeleteKey failed on key \"%s\": %s",
                            rKey.toLatin1().data(), errorCodeToString(res).toLatin1().data());
            }
        }
        RegCloseKey(handle);
    }
}

static bool stringContainsNullChar(const QString &s)
{
    for (int i = 0; i < s.length(); ++i) {
        if (s.at(i).unicode() == 0)
            return true;
    }
    return false;
}

void QWinSettingsPrivate::set(const QString &uKey, const QVariant &value)
{
    QString rKey = escapedKey(uKey);

    HKEY handle = createOrOpenKey(writeHandle(), KEY_ALL_ACCESS, keyPath(rKey));
    if (handle == 0)
        return;

    DWORD type;
    QByteArray regValueBuff;

    // Determine the type
    switch (value.type()) {
        case QVariant::List:
        case QVariant::StringList: {
            // If none of the elements contains '\0', we can use REG_MULTI_SZ, the
            // native registry string list type. Otherwise we use REG_BINARY.
            type = REG_MULTI_SZ;
            QStringList l = variantListToStringList(value.toList());
            QStringList::const_iterator it = l.constBegin();
            for (; it != l.constEnd(); ++it) {
                if ((*it).length() == 0 || stringContainsNullChar(*it)) {
                    type = REG_BINARY;
                    break;
                }
            }

            if (type == REG_BINARY) {
                QString s = variantToString(value);
                QT_WA( {
                    regValueBuff = QByteArray((const char*)s.utf16(), s.length()*2);
                }, {
                    regValueBuff = QByteArray((const char*)s.toLocal8Bit(), s.length());
                } );
            } else {
                QStringList::const_iterator it = l.constBegin();
                for (; it != l.constEnd(); ++it) {
                    const QString &s = *it;
                    QT_WA( {
                        regValueBuff += QByteArray((const char*)s.utf16(), (s.length() + 1)*2);
                    }, {
                        regValueBuff += QByteArray((const char*)s.toLocal8Bit(), s.length() + 1);
                    } );
                }
                QT_WA( {
                    regValueBuff.append((char)0);
                    regValueBuff.append((char)0);
                }, {
                    regValueBuff.append((char)0);
                } );
            }
            break;
        }

        case QVariant::Int: {
            type = REG_DWORD;
            int i = value.toInt();
            regValueBuff = QByteArray((const char*)&i, sizeof(int));
            break;
        }

        default: {
            // If the string does not contain '\0', we can use REG_SZ, the native registry
            // string type. Otherwise we use REG_BINARY.
            QString s = variantToString(value);
            type = stringContainsNullChar(s) ? REG_BINARY : REG_SZ;
            if (type == REG_BINARY) {
                QT_WA( {
                    regValueBuff = QByteArray((const char*)s.utf16(), s.length()*2);
                }, {
                    regValueBuff = QByteArray((const char*)s.toLocal8Bit(), s.length());
                } );
            } else {
                QT_WA( {
                    regValueBuff = QByteArray((const char*)s.utf16(), (s.length() + 1)*2);
                }, {
                    regValueBuff = QByteArray((const char*)s.toLocal8Bit(), s.length() + 1);
                } );
            }
            break;
        }
    }

    // set the value
    LONG res;
    QT_WA( {
        res = RegSetValueExW(handle, reinterpret_cast<const wchar_t *>(keyName(rKey).utf16()), 0, type,
                             reinterpret_cast<const unsigned char*>(regValueBuff.constData()),
                             regValueBuff.size());
    }, {
        res = RegSetValueExA(handle, keyName(rKey).toLocal8Bit(), 0, type,
                             reinterpret_cast<const unsigned char*>(regValueBuff.constData()),
                             regValueBuff.size());
    } );

    if (res == ERROR_SUCCESS) {
        deleteWriteHandleOnExit = false;
    } else {
        qWarning("QSettings: failed to set subkey \"%s\": %s",
                rKey.toLatin1().data(), errorCodeToString(res).toLatin1().data());
    }

    RegCloseKey(handle);
}

bool QWinSettingsPrivate::get(const QString &uKey, QVariant *value) const
{
    QString rKey = escapedKey(uKey);

    for (int i = 0; i < regList.size(); ++i) {
        if (readKey(regList.at(i).handle, rKey, value))
            return true;

        if (!fallbacks)
            return false;
    }

    return false;
}

QStringList QWinSettingsPrivate::children(const QString &uKey, ChildSpec spec) const
{
    NameSet result;
    QString rKey = escapedKey(uKey);

    for (int i = 0; i < regList.size(); ++i) {
        HKEY handle = openKey(regList.at(i).handle, KEY_READ, rKey);
        if (handle == 0)
            continue;

        if (spec == AllKeys) {
            NameSet keys;
            allKeys(handle, QLatin1String(""), &keys);
            mergeKeySets(&result, keys);
        } else { // ChildGroups or ChildKeys
            QStringList names = childKeysOrGroups(handle, spec);
            mergeKeySets(&result, names);
        }

        RegCloseKey(handle);

        if (!fallbacks)
            return result.keys();
    }

    return result.keys();
}

void QWinSettingsPrivate::clear()
{
    remove(QString());
    deleteWriteHandleOnExit = true;
}

void QWinSettingsPrivate::sync()
{
    RegFlushKey(writeHandle());
}

void QWinSettingsPrivate::flush()
{
    // Windows does this for us.
}

QString QWinSettingsPrivate::fileName() const
{
    if (regList.isEmpty())
        return QString();
    return regList.at(0).key;
}

bool QWinSettingsPrivate::isWritable() const
{
    return writeHandle() != 0;
}

QSettingsPrivate *QSettingsPrivate::create(QSettings::Format format,
                                           QSettings::Scope scope,
                                           const QString &organization,
                                           const QString &application)
{
    if (format == QSettings::NativeFormat) {
        QWinSettingsPrivate *p = new QWinSettingsPrivate(scope, organization, application);
        return p;
    } else {
        QConfFileSettingsPrivate *p = new QConfFileSettingsPrivate(format, scope,
                                                                   organization, application);
        p->init();
        return p;
    }
}

QSettingsPrivate *QSettingsPrivate::create(const QString &fileName,
                                           QSettings::Format format)
{
    if (format == QSettings::NativeFormat) {
        QWinSettingsPrivate *p = new QWinSettingsPrivate(fileName);
        return p;
    } else {
        QConfFileSettingsPrivate *p = new QConfFileSettingsPrivate(fileName, format);
        p->init();
        return p;
    }
}
