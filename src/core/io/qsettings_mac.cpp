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

/* This writes to:
   1) ~/Library/Preferences/...
   2) /Library/Preferences/...
*/

#include "qsettings.h"
#include <private/qsettings_p.h>
#ifndef QT_NO_SETTINGS
#include "qstring.h"
#include "qcleanuphandler.h"
#include <private/qcore_mac_p.h>
#include <stdlib.h>

/*****************************************************************************
  QSettings debug facilities
 *****************************************************************************/
//#define DEBUG_SETTINGS_KEYS

Q_GLOBAL_STATIC(QString, qt_mac_settings_base)
bool qt_verify_key(const QString &); //qsettings.cpp

/*****************************************************************************
  QSettings utility functions
 *****************************************************************************/
#if 1
#include "qurl.h"
#define MACKEY_SEP '.'
static void qt_mac_fix_key(QString &k) {
    if(k.isEmpty())
        return;
#ifdef DEBUG_SETTINGS_KEYS
    QString old_k = k;
#endif
    while(k.length() && k[0] == '/')
        k = k.mid(1);
    k.replace("//", "/");
    k = QString(QUrl::toPercentEncoding(k));
    k.replace(".", "%2E"); //when a . is in a key, we need to url encode it..
    for(int i=0; i<(int)k.length(); i++) {
        if(k[i] == '/')
            k[i] = MACKEY_SEP;
    }
#ifdef DEBUG_SETTINGS_KEYS
    qDebug("QSettings::fixed : %s -> %s", old_k.latin1(), k.latin1());
#endif
}
static void qt_mac_unfix_key(QString &k) {
    if(k.isEmpty())
        return;

#ifdef DEBUG_SETTINGS_KEYS
    QString old_k = k;
#endif
    k.replace(".", "/");
    k.replace("%2E", "."); //just to be sure
    k = QUrl::fromPercentEncoding(k.toAscii());
#ifdef DEBUG_SETTINGS_KEYS
    qDebug("QSettings::unFixed : %s -> %s", old_k.latin1(), k.latin1());
#endif
}
#else
#define qt_mac_fix_key(k)
#define qt_mac_unfix_key(k)
#define MACKEY_SEP '.'
#endif

QString
qt_mac_get_global_setting(QString key, QString ret=QString::null, QString file=QString::null)
{
    if(file.isNull())
        file = ".GlobalPreferences";
    QCFString k(key), id(file);
    if(QCFType<CFPropertyListRef> r = CFPreferencesCopyValue(k, id, kCFPreferencesCurrentUser,
                                                               kCFPreferencesAnyHost)) {
        CFTypeID typeID = CFGetTypeID(r);
        if(typeID == CFStringGetTypeID()) {
            ret = QCFString::toQString(static_cast<CFStringRef>(static_cast<CFPropertyListRef>(r)));
        } else if(typeID == CFBooleanGetTypeID()) {
            ret = CFEqual(static_cast<CFBooleanRef>(static_cast<CFPropertyListRef>(r)),
                          kCFBooleanTrue) ? "TRUE" : "FALSE";
        } else if(typeID == CFNumberGetTypeID()) {
            CFNumberRef number = static_cast<CFNumberRef>(static_cast<CFPropertyListRef>(r));
            CFNumberType numType = CFNumberGetType(number);
            switch (numType) {
            case kCFNumberFloatType: {
                float fnum;
                if (CFNumberGetValue(number, kCFNumberFloatType, &fnum))
                    ret = QString::number(fnum);
                break; }
            default: {
                int inum;
                if (CFNumberGetValue(number, kCFNumberIntType, &inum))
                    ret = QString::number(inum);
                break; }
            }
        } else {
            qWarning("qt-internal::QSettings, %s: unknown CFType %d", key.latin1(),
                     (int)CFGetTypeID(r));
        }
    }
    return ret;
}

/*****************************************************************************
  Developers are allowed to access this to influence the base (defaults to 'com.')
 *****************************************************************************/
void qt_setSettingsBasePath(const QString &s)
{
    (*qt_mac_settings_base()) = s;
}

/*****************************************************************************
  This class given the path and key will generate a proper ApplicationID as well
  as the matching key.
 *****************************************************************************/
class search_keys {
    CFStringRef i, k;
    QString qi, qk;
public:
    search_keys(QString id, QString key, const char * =NULL);
    ~search_keys();

    CFStringRef id() const { return i; }
    const QString &qtId() const { return qi; }
    CFStringRef key() const { return k; }
    const QString &qtKey() const { return qk; }
};
search_keys::search_keys(QString path, QString key, const char *where)
{
#ifndef DEBUG_SETTINGS_KEYS
    Q_UNUSED(where);
#else
    QString oldkey = key, oldpath = path;
#endif
    while(key.endsWith("/"))
	key.chop(1);
    qi = path;
    qk = key;
    while(qk.startsWith("/"))
        qk = qk.mid(1);
    while(qi.startsWith("/"))
        qi = qi.mid(1);
    if(qi.isEmpty()) {
        int slsh = qk.indexOf('/');
        if(slsh != -1) {
            qi += qk.left(slsh);
            qk = qk.mid(slsh+1);
        }
    }
    while(qi.startsWith("/"))
        qi = qi.mid(1);
    qt_mac_unfix_key(qi);
    qi.replace('/', ".");
    qi.replace("..", ".");
    if (!qt_mac_settings_base()->isEmpty())
        qi.prepend(*qt_mac_settings_base());

    qt_mac_fix_key(qk);
#ifdef DEBUG_SETTINGS_KEYS
    qDebug("search_key [%s] %s::%s -> %s::%s", where ? where : "*Unknown*",
           oldpath.latin1(), oldkey.latin1(), qi.latin1(), qk.latin1());
#endif
    i = QCFString::toCFStringRef(qi);
    k = QCFString::toCFStringRef(qk);
}
search_keys::~search_keys()
{
    CFRelease(i);
    CFRelease(k);
}

/*****************************************************************************
  Internal private class
 *****************************************************************************/
class QSettingsSysPrivate
{
public:
    QSettingsSysPrivate();
    bool writeEntry(QString, CFPropertyListRef, bool);
    QCFType<CFPropertyListRef> readEntry(QString, bool);
    QStringList entryList(QString, bool, bool);
    QStringList searchPaths;
    QStringList syncKeys;
};

QSettingsSysPrivate::QSettingsSysPrivate()
{
    if (!qt_mac_settings_base()->isEmpty())
        (*qt_mac_settings_base()) = "com.";
}

struct QMacSettingPerms {
    CFStringRef user;
    CFStringRef host;
} scopes[] = {
    { kCFPreferencesAnyUser, kCFPreferencesCurrentHost }, //global
    { kCFPreferencesCurrentUser, kCFPreferencesAnyHost }, //local
    { NULL, NULL }
};

bool QSettingsSysPrivate::writeEntry(QString key, CFPropertyListRef plr, bool global)
{
    bool ret = false;
    for(int i = searchPaths.size() - 1; i >= 0; --i) {
        search_keys k(searchPaths.at(i), key, "writeEntry");
        for(int scope = (global ? 0 : 1); scopes[scope].user; scope++) {
            CFPreferencesSetValue(k.key(), plr, k.id(), scopes[scope].user, scopes[scope].host);
            if(true) { //no way to tell if there is success!?! --Sam
                if(!syncKeys.indexOf(k.qtId()) != -1)
                    syncKeys.append(k.qtId());
                ret = true;
                break;
            }
        }
        if(ret)
            break;
    }
    return ret;
}

QCFType<CFPropertyListRef> QSettingsSysPrivate::readEntry(QString key, bool global)
{
    for(int i = searchPaths.size() - 1; i >= 0; --i) {
        search_keys k(searchPaths.at(i), key, "readEntry");
	for(int scope = (global ? 0 : 1); scope >= 0; scope--) { //try local, then global (unless global == true, then just global)
            if(QCFType<CFPropertyListRef> ret = CFPreferencesCopyValue(k.key(), k.id(), scopes[scope].user, scopes[scope].host))
                return ret;
        }
    }
    return 0;
}

QStringList QSettingsSysPrivate::entryList(QString key, bool subkey, bool global)
{
    QStringList ret;
    for(int i = searchPaths.size() - 1; i >= 0; --i) {
        search_keys k(searchPaths.at(i), key, subkey ? "subkeyList" : "entryList");
	for(int scope = (global ? 0 : 1); scope >= 0; scope--) { //try local, then global (unless global == true, then just global)
            if(QCFType<CFArrayRef> cfa = CFPreferencesCopyKeyList(k.id(), scopes[scope].user, scopes[scope].host)) {
                QString qk = k.qtKey();
                for(CFIndex i = 0, cnt = CFArrayGetCount(cfa); i < cnt; i++) {
                    QString s
                        = QCFString(static_cast<CFStringRef>(CFArrayGetValueAtIndex(cfa, i)));
                    if(s.left(qk.length()) == qk) {
                        s = s.mid(qk.length());
                        while(s[0] == MACKEY_SEP)
                            s = s.mid(1);
                        int sep = s.indexOf(MACKEY_SEP);
                        if(sep != -1) {
                            if(subkey) {
                                QString fix_s = s.left(sep);
                                qt_mac_unfix_key(fix_s);
                                if(!fix_s.isEmpty() && ret.indexOf(fix_s) == -1)
                                    ret << fix_s;
                            }
                        } else if(!subkey) {
                            QString fix_s = s;
                            qt_mac_unfix_key(fix_s);
                            ret << fix_s;
                        }
                    }
                }
                return ret;
            }
        }
    }
    return ret;
}

void
QSettingsPrivate::sysInit()
{
    sysd = new QSettingsSysPrivate;
    sysInsertSearchPath(QSettings::Mac, "");
}

void
QSettingsPrivate::sysClear()
{
    delete sysd;
    sysd = NULL;
}

bool QSettingsPrivate::sysSync()
{
    bool ret = true;
    for(QStringList::Iterator it = sysd->syncKeys.begin();  it != sysd->syncKeys.end(); ++it) {
#ifdef DEBUG_SETTINGS_KEYS
        qDebug("QSettingsPrivate::sysSync(%s)", (*it).latin1());
#endif
        if(CFPreferencesAppSynchronize(QCFString(*it)))
            ret = false;
    }
    sysd->syncKeys.clear();
    return ret;
}

void QSettingsPrivate::sysInsertSearchPath(QSettings::System s, const QString &path)
{
    if(s != QSettings::Mac)
        return;
    if (!path.isEmpty() && !qt_verify_key(path)) {
        qWarning("QSettings::insertSearchPath: Invalid key: '%s'", path.isNull() ? "(null)" : path.latin1());
        return;
    }

    QString realpath = path;
    while(realpath.endsWith("/"))
        realpath.chop(1);
    if (!realpath.isEmpty())
        sysd->searchPaths.append(realpath);
}

void QSettingsPrivate::sysRemoveSearchPath(QSettings::System s, const QString &path)
{
    if(s != QSettings::Mac)
        return;
    QString realpath = path;
    while(realpath.endsWith("/"))
        realpath.chop(1);
    sysd->searchPaths.removeAll(realpath);
}

bool QSettingsPrivate::sysReadBoolEntry(const QString &key, bool def, bool *ok) const
{
    if(key.isNull() || key.isEmpty()) {
        qWarning("QSettingsPrivate::sysReadBoolEntry: invalid null/empty key.");
        if(ok)
            *ok = false;
        return def;
    }
    if(QCFType<CFPropertyListRef> r = sysd->readEntry(key, globalScope)) {
        if(CFGetTypeID(r) == CFBooleanGetTypeID()) {
            if(ok)
                *ok = true;
            return CFEqual(static_cast<CFBooleanRef>(static_cast<CFPropertyListRef>(r)),
                           kCFBooleanTrue);
        }
    }
    if(ok)
        *ok = false;
    return def;
}

double QSettingsPrivate::sysReadDoubleEntry(const QString &key, double def, bool *ok) const
{
    if(key.isNull() || key.isEmpty()) {
        qWarning("QSettingsPrivate::sysReadDoubleEntry: invalid null/empty key.");
        if(ok)
            *ok = false;
        return def;
    }
    if(QCFType<CFPropertyListRef> r = sysd->readEntry(key, globalScope)) {
        if(CFGetTypeID(r) == CFNumberGetTypeID()) {
            double ret;
            if(!CFNumberGetValue(static_cast<CFNumberRef>(static_cast<CFPropertyListRef>(r)),
                                 kCFNumberDoubleType, &ret)) {
                if(ok)
                    *ok = false;
                ret = def;
            } else if(ok) {
                *ok = true;
            }
            return ret;
        }
    }
    if(ok)
        *ok = false;
    return def;
}

int QSettingsPrivate::sysReadNumEntry(const QString &key, int def, bool *ok) const
{
    if(key.isNull() || key.isEmpty()) {
        qWarning("QSettingsPrivate::sysReadNumEntry: invalid null/empty key.");
        if(ok)
            *ok = false;
        return def;
    }
    if(QCFType<CFPropertyListRef> r = sysd->readEntry(key, globalScope)) {
        if(CFGetTypeID(r) == CFNumberGetTypeID()) {
            int ret;
            if(!CFNumberGetValue(static_cast<CFNumberRef>(static_cast<CFPropertyListRef>(r)),
                                                          kCFNumberIntType, &ret)) {
                if(ok)
                    *ok = false;
                ret = def;
            } else if(ok) {
                *ok = true;
            }
            return ret;
        }
    }
    if(ok)
        *ok = false;
    return def;
}

QString QSettingsPrivate::sysReadEntry(const QString &key, const QString &def, bool *ok) const
{
    if(key.isNull() || key.isEmpty()) {
        qWarning("QSettingsPrivate::sysReadEntry: invalid null/empty key.");
        if(ok)
            *ok = false;
        return def;
    }
    if(QCFType<CFPropertyListRef> r = sysd->readEntry(key, globalScope)) {
        if(CFGetTypeID(r) == CFStringGetTypeID()) {
            if(ok)
                *ok = true;
            return QCFString::toQString(static_cast<CFStringRef>(static_cast<CFPropertyListRef>(r)));
        }
    }
    if(ok)
        *ok = false;
    return def;
}

#if !defined(Q_NO_BOOL_TYPE)
bool QSettingsPrivate::sysWriteEntry(const QString &key, bool value)
{
    if(key.isNull() || key.isEmpty()) {
        qWarning("QSettingsPrivate::sysWriteEntry (bool): invalid null/empty key.");
        return false;
    }
    return sysd->writeEntry(key, value ? kCFBooleanTrue : kCFBooleanFalse, globalScope);
}
#endif

bool QSettingsPrivate::sysWriteEntry(const QString &key, double value)
{
    if(key.isNull() || key.isEmpty()) {
        qWarning("QSettingsPrivate::sysWriteEntry (double): invalid null/empty key.");
        return false;
    }
    return sysd->writeEntry(key, QCFType<CFNumberRef>(CFNumberCreate(NULL, kCFNumberDoubleType, &value)),
                            globalScope);
}

bool QSettingsPrivate::sysWriteEntry(const QString &key, int value)
{
    if(key.isNull() || key.isEmpty()) {
        qWarning("QSettingsPrivate::sysWriteEntry (int): invalid null/empty key.");
        return false;
    }
    return sysd->writeEntry(key, QCFType<CFNumberRef>(CFNumberCreate(NULL, kCFNumberIntType, &value)),
                            globalScope);
}

bool QSettingsPrivate::sysWriteEntry(const QString &key, const QString &value)
{
    if(key.isNull() || key.isEmpty()) {
        qWarning("QSettingsPrivate::sysWriteEntry (QString): invalid null/empty key.");
        return false;
    }
    return sysd->writeEntry(key, QCFString(value), globalScope);
}

bool QSettingsPrivate::sysRemoveEntry(const QString &key)
{
    if(key.isNull() || key.isEmpty()) {
        qWarning("QSettingsPrivate::sysRemoveEntry: invalid null/empty key.");
        return false;
    }
    return sysd->writeEntry(key, NULL, globalScope);
}

QStringList QSettingsPrivate::sysEntryList(const QString &key) const
{
    if(key.isNull() || key.isEmpty()) {
        qWarning("QSettingsPrivate::sysEntryList: invalid null/empty key.");
        return QStringList();
    }
    return sysd->entryList(key, false, globalScope);
}

QStringList QSettingsPrivate::sysSubkeyList(const QString &key) const
{
    if(key.isNull() || key.isEmpty()) {
        qWarning("QSettingsPrivate::sysSubkeyList: invalid null/empty key.");
        return QStringList();
    }
    return sysd->entryList(key, true, globalScope);
}

#endif //QT_NO_SETTINGS
