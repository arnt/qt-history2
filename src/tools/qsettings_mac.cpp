/****************************************************************************
** $Id$
**
** Implementation of QSettings class
**
** Created: 2001.11.30
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Macintosh may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

/* This writes to:
   1) ~/Library/Preferences/...
   2) /Library/Preferences/...
*/

#include "qsettings.h"
#include <private/qsettings_p.h>
#ifndef QT_NO_SETTINGS
#include "qstring.h"
#include "qptrlist.h"
// ### remove this hack, tools shouldn't include kernel files
#include "qwindowdefs.h"
#include <CoreFoundation/CoreFoundation.h>
#include <stdlib.h>

/*****************************************************************************
  QSettings debug facilities
 *****************************************************************************/
//#define DEBUG_SETTINGS_KEYS

static QString *qt_mac_settings_base = NULL;

/*****************************************************************************
  QSettings utility functions
 *****************************************************************************/
#if 1
#define MACKEY_SEP '.'
static void qt_mac_fix_key(QString &k) {
    k.replace("//", "/");
    if(k.length() && k[0] == '/')
	k = k.mid(1);
    for ( int i=0; i<(int)k.length(); i++ ) {
	if ( k[i] == '/' )
	    k[i] = MACKEY_SEP;
	else if(k[i] == MACKEY_SEP || k[i].isSpace())
	    k[i] = '_';
    }
}
#else
#define qt_mac_fix_key(k) 
#define MACKEY_SEP '/'
#endif

static QString cfstring2qstring(CFStringRef str)
{
    CFIndex length = CFStringGetLength(str); 
    if(const UniChar *chars = CFStringGetCharactersPtr(str)) 
	return QString((QChar *)chars, length);
    UniChar *buffer = (UniChar*)malloc(length * sizeof(UniChar)); 
    CFStringGetCharacters(str, CFRangeMake(0, length), buffer); 
    QString ret((QChar *)buffer, length);
    free(buffer); 
    return ret;
}

static void cleanup_qsettings()
{
    delete qt_mac_settings_base;
    qt_mac_settings_base = NULL;
}

/*****************************************************************************
  Developers are allowed to access this to influence the base (defaults to 'com.')
 *****************************************************************************/
void qt_setSettingsBasePath( const QString &s )
{
    if(qt_mac_settings_base) {
	qWarning( "qt_setSettingsBasePath has to be called without any settings object being instantiated!" );
	return;
    }
    qAddPostRoutine( cleanup_qsettings );	
    qt_mac_settings_base = new QString(s);
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
#endif
    qi = path;
    qk = key;
    int start = 0;
    if(qk[0] == '/')
	start++;
    int slsh = qk.find('/', start);
    if(slsh != -1) {
	qi += qk.left(slsh);
	qk = qk.mid(slsh+1);
    }
    qt_mac_fix_key(qi);
    if(qt_mac_settings_base)
	qi.prepend(*qt_mac_settings_base);
    qt_mac_fix_key(qk);
#ifdef DEBUG_SETTINGS_KEYS
    qDebug("[QSettings::%s] %s::%s -> %s::%s", where ? where : "*Unknown*", 
	   path.latin1(), key.latin1(), qi.latin1(), qk.latin1());
#endif
    i = CFStringCreateWithCharacters(NULL, (UniChar *)qi.unicode(), qi.length());
    k = CFStringCreateWithCharacters(NULL, (UniChar *)qk.unicode(), qk.length());
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
    bool writeEntry(QString, CFPropertyListRef);
    QStringList searchPaths;
    QStringList syncKeys;
};

QSettingsSysPrivate::QSettingsSysPrivate() 
{
    if(!qt_mac_settings_base) {
	qt_mac_settings_base = new QString("com.");
	qAddPostRoutine( cleanup_qsettings );	
    }
}

bool QSettingsSysPrivate::writeEntry(QString key, CFPropertyListRef plr)
{
    bool ret = FALSE;
    for ( QStringList::Iterator it = searchPaths.fromLast(); 
	  it != searchPaths.end(); --it ) {
	search_keys k((*it), key, "writeEntry");
	CFPreferencesSetAppValue(k.key(), plr, k.id());    
	if(TRUE) { //hmmm..
	    if(!syncKeys.findIndex(k.qtId()) != -1)
		syncKeys.append(k.qtId());
	    ret = TRUE;
	    break;
	}
    }
    return ret;
}

void
QSettingsPrivate::sysInit()
{
    sysd = new QSettingsSysPrivate;
    Q_CHECK_PTR(sysd);
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
    bool ret = TRUE;
    for ( QStringList::Iterator it = sysd->syncKeys.begin();  it != sysd->syncKeys.end(); --it ) {
	CFStringRef csr = CFStringCreateWithCharacters(NULL, (UniChar *)(*it).unicode(), 
						       (*it).length());
#ifdef DEBUG_SETTINGS_KEYS
	qDebug("QSettingsPrivate::sysSync(%s)", (*it).latin1());
#endif
	if(CFPreferencesAppSynchronize(csr))
	    ret = FALSE;
	CFRelease(csr);
    }
    sysd->syncKeys.clear();
    return ret;
}

void QSettingsPrivate::sysInsertSearchPath( QSettings::System s, const QString &path)
{
    if ( s != QSettings::Mac )
	return;
    QString realpath = path;
    while(realpath.right(1) == "/")
	realpath.truncate(realpath.length() -1);
    sysd->searchPaths.append(realpath);
}

void QSettingsPrivate::sysRemoveSearchPath( QSettings::System s, const QString &path)
{
    if ( s != QSettings::Mac ) 
	return;
    QString realpath = path;
    while(realpath.right(1) == "/")
	realpath.truncate(realpath.length() -1);
    sysd->searchPaths.remove(realpath);
}

bool QSettingsPrivate::sysReadBoolEntry(const QString &key, bool def, bool *ok ) const
{
#ifdef QT_CHECK_STATE
    if (key.isNull() || key.isEmpty()) {
	qWarning("QSettingsPrivate::sysReadBoolEntry: invalid null/empty key.");
	if ( ok )
	    *ok = FALSE;
	return def;
    }
#endif // QT_CHECK_STATE
    for ( QStringList::Iterator it = sysd->searchPaths.fromLast(); 
	  it != sysd->searchPaths.end(); --it ) {
	search_keys k((*it), key, "readBoolEntry");
	if(CFPropertyListRef r = CFPreferencesCopyAppValue(k.key(), k.id())) {
	    if(CFGetTypeID(r) != CFBooleanGetTypeID()) {
		CFRelease(r);
		break;
	    }
	    bool ret = FALSE;
	    if(CFEqual((CFBooleanRef)r, kCFBooleanTrue))
		ret = TRUE;
	    CFRelease(r);
	    return ret;
	}
    }
    if(ok)
	*ok = FALSE;
    return def;
}

double QSettingsPrivate::sysReadDoubleEntry(const QString &key, double def, bool *ok ) const
{
#ifdef QT_CHECK_STATE
    if (key.isNull() || key.isEmpty()) {
	qWarning("QSettingsPrivate::sysReadDoubleEntry: invalid null/empty key.");
	if ( ok )
	    *ok = FALSE;
	return def;
    }
#endif // QT_CHECK_STATE
    for ( QStringList::Iterator it = sysd->searchPaths.fromLast(); 
	  it != sysd->searchPaths.end(); --it ) {
	search_keys k((*it), key, "readDoubleEntry");
	if(CFPropertyListRef r = CFPreferencesCopyAppValue(k.key(), k.id())) {
	    if(CFGetTypeID(r) != CFNumberGetTypeID()) {
		CFRelease(r);
		break;
	    }
	    double ret;
	    if(!CFNumberGetValue( (CFNumberRef)r, kCFNumberDoubleType, &ret)) {
		if(ok)
		    *ok = FALSE;
		ret = def;
	    } else if(ok) {
		*ok = TRUE;
	    }
	    CFRelease(r);
	    return ret;
	}
    }
    if(ok)
	*ok = FALSE;
    return def;
} 

int QSettingsPrivate::sysReadNumEntry(const QString &key, int def, bool *ok ) const
{
#ifdef QT_CHECK_STATE
    if (key.isNull() || key.isEmpty()) {
	qWarning("QSettingsPrivate::sysReadNumEntry: invalid null/empty key.");
	if ( ok )
	    *ok = FALSE;
	return def;
    }
#endif // QT_CHECK_STATE
    for ( QStringList::Iterator it = sysd->searchPaths.fromLast(); 
	  it != sysd->searchPaths.end(); --it ) {
	search_keys k((*it), key, "readNumEntry");
	if(CFPropertyListRef r = CFPreferencesCopyAppValue(k.key(), k.id())) {
	    if(CFGetTypeID(r) != CFNumberGetTypeID()) {
		CFRelease(r);
		break;
	    }
	    int ret;
	    if(!CFNumberGetValue( (CFNumberRef)r, kCFNumberIntType, &ret)) {
		if(ok)
		    *ok = FALSE;
		ret = def;
	    } else if(ok) {
		*ok = TRUE;
	    }
	    CFRelease(r);
	    return ret;
	}
    }
    if(ok)
	*ok = FALSE;
    return def;
}

QString QSettingsPrivate::sysReadEntry(const QString &key, const QString &def, bool *ok ) const
{
#ifdef QT_CHECK_STATE
    if (key.isNull() || key.isEmpty()) {
	qWarning("QSettingsPrivate::sysReadEntry: invalid null/empty key.");
	if ( ok )
	    *ok = FALSE;
	return def;
    }
#endif // QT_CHECK_STATE
    for ( QStringList::Iterator it = sysd->searchPaths.fromLast(); 
	  it != sysd->searchPaths.end(); --it ) {
	search_keys k((*it), key, "readEntry");
	if(CFPropertyListRef r = CFPreferencesCopyAppValue(k.key(), k.id())) {
	    if(CFGetTypeID(r) != CFStringGetTypeID()) {
		CFRelease(r);
		break;
	    }
	    if(ok)
		*ok = TRUE;
	    QString ret = cfstring2qstring((CFStringRef)r);
	    CFRelease(r);
	    return ret;
	}
    }
    if(ok)
	*ok = FALSE;
    return def;
}

#if !defined(Q_NO_BOOL_TYPE)
bool QSettingsPrivate::sysWriteEntry(const QString &key, bool value)
{
#ifdef QT_CHECK_STATE
    if (key.isNull() || key.isEmpty()) {
	qWarning("QSettingsPrivate::sysWriteEntry (bool): invalid null/empty key.");
	return FALSE;
    }
#endif // QT_CHECK_STATE
    CFBooleanRef val = value ? kCFBooleanTrue : kCFBooleanFalse;
    bool ret = sysd->writeEntry(key, val);
    return ret;
}
#endif

bool QSettingsPrivate::sysWriteEntry(const QString &key, double value)
{
#ifdef QT_CHECK_STATE
    if (key.isNull() || key.isEmpty()) {
	qWarning("QSettingsPrivate::sysWriteEntry (double): invalid null/empty key.");
	return FALSE;
    }
#endif // QT_CHECK_STATE
    CFNumberRef val = CFNumberCreate(NULL, kCFNumberDoubleType, &value);
    bool ret = sysd->writeEntry(key, val);
    CFRelease(val);
    return ret;
}

bool QSettingsPrivate::sysWriteEntry(const QString &key, int value)
{
#ifdef QT_CHECK_STATE
    if (key.isNull() || key.isEmpty()) {
	qWarning("QSettingsPrivate::sysWriteEntry (int): invalid null/empty key.");
	return FALSE;
    }
#endif // QT_CHECK_STATE
    CFNumberRef val = CFNumberCreate(NULL, kCFNumberIntType, &value);
    bool ret = sysd->writeEntry(key, val);
    CFRelease(val);
    return ret;
}

bool QSettingsPrivate::sysWriteEntry(const QString &key, const QString &value)
{
#ifdef QT_CHECK_STATE
    if (key.isNull() || key.isEmpty()) {
	qWarning("QSettingsPrivate::sysWriteEntry (QString): invalid null/empty key.");
	return FALSE;
    }
#endif // QT_CHECK_STATE
    CFStringRef val = CFStringCreateWithCharacters(NULL, (UniChar *)value.unicode(), 
						 value.length());
    bool ret = sysd->writeEntry(key, val);
    CFRelease(val);
    return ret;
}

bool QSettingsPrivate::sysRemoveEntry(const QString &key)
{
#ifdef QT_CHECK_STATE
    if (key.isNull() || key.isEmpty()) {
	qWarning("QSettingsPrivate::sysRemoveEntry: invalid null/empty key.");
	return FALSE;
    }
#endif // QT_CHECK_STATE
    return sysd->writeEntry(key, NULL);
}

QStringList QSettingsPrivate::sysEntryList(const QString &key) const
{
#ifdef QT_CHECK_STATE
    if ( key.isNull() || key.isEmpty() ) {
	qWarning("QSettingsPrivate::sysEntryList: invalid null/empty key.");
	return QStringList();
    }
#endif // QT_CHECK_STATE
    for ( QStringList::Iterator it = sysd->searchPaths.fromLast(); 
	  it != sysd->searchPaths.end(); --it ) {
	search_keys k((*it), key, "entryList");
	// perhaps we should iterate over all the possible values (ie kCFPreferenceAllUsers)
	if(CFArrayRef cfa = CFPreferencesCopyKeyList(k.id(), kCFPreferencesCurrentUser,
						     kCFPreferencesAnyHost)) {
	    QStringList ret;
	    QString qk = cfstring2qstring(k.key());
	    for(CFIndex i = 0, cnt = CFArrayGetCount(cfa); i < cnt; i++) {
		QString s = cfstring2qstring((CFStringRef)CFArrayGetValueAtIndex(cfa, i));
		if(s.left(qk.length()) == qk) {
		    s = s.mid(qk.length());
		    if(!s.contains(MACKEY_SEP))
			ret << s;
		}
	    }
	    CFRelease(cfa);
	    return ret;
	}
    }
    return QStringList();
}

QStringList QSettingsPrivate::sysSubkeyList(const QString &key) const
{
#ifdef QT_CHECK_STATE
    if ( key.isNull() || key.isEmpty() ) {
	qWarning("QSettingsPrivate::sysSubkeyList: invalid null/empty key.");
	return QStringList();
    }
#endif // QT_CHECK_STATE
    for ( QStringList::Iterator it = sysd->searchPaths.fromLast(); 
	  it != sysd->searchPaths.end(); --it ) {
	search_keys k((*it), key, "subkeyList");
	// perhaps we should iterate over all the possible values (ie kCFPreferenceAllUsers)
	if(CFArrayRef cfa = CFPreferencesCopyKeyList(k.id(), kCFPreferencesCurrentUser,
						     kCFPreferencesAnyHost)) {
	    QStringList ret;
	    QString qk = cfstring2qstring(k.key());
	    for(CFIndex i = 0, cnt = CFArrayGetCount(cfa); i < cnt; i++) {
		QString s = cfstring2qstring((CFStringRef)CFArrayGetValueAtIndex(cfa, i));
		if(s.left(qk.length()) == qk) {
		    s = s.mid(qk.length());
		    int sep = s.find(MACKEY_SEP);
		    if(sep != -1) {
			s = s.left(sep);
			if(!s.isEmpty() && ret.findIndex(s) == -1)
			    ret << s;
		    }
		}
	    }
	    CFRelease(cfa);
	    return ret;
	}
    }
    return QStringList();
}

#endif //QT_NO_SETTINGS
