/****************************************************************************
** $Id$
**
** Implementation of QSettings class
**
** Created: 2001.11.30
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
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

#include "qsettings.h"
#ifndef QT_NO_SETTINGS
#include <qstring.h>
#include <qptrlist.h>
#include <qwindowdefs.h>
#include <CoreFoundation/CoreFoundation.h>

static QString *qt_mac_settings_base = NULL;

/*****************************************************************************
  QSettings utility functions
 *****************************************************************************/
#if 1
#define MACKEY_SEP '.'
static void qt_mac_fix_key(QString &k) {
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
public:
    search_keys(QString id, QString key);
    ~search_keys();

    CFStringRef id() const { return i; }
    CFStringRef key() const { return k; }
};
search_keys::search_keys(QString path, QString key) 
{
//    qDebug("-> %s::%s", path.latin1(), key.latin1());
    int start = 0;
    if(key[0] == '/')
	start++;
    int slsh = key.find('/', start);
    if(slsh != -1) {
	path += key.left(slsh);
	key = key.mid(slsh+1);
    }
    qt_mac_fix_key(path);
    if(qt_mac_settings_base)
	path.prepend(*qt_mac_settings_base);
    qt_mac_fix_key(key);
//    qDebug("<- %s::%s", path.latin1(), key.latin1());
    i = CFStringCreateWithCharacters(NULL, (UniChar *)path.unicode(), path.length());
    k = CFStringCreateWithCharacters(NULL, (UniChar *)key.unicode(), key.length());
}
search_keys::~search_keys()
{
    CFRelease(i);
    CFRelease(k);
}

/*****************************************************************************
  Internal private class
 *****************************************************************************/
class QSettingsPrivate
{
public:
    QSettingsPrivate();
    bool writeEntry(QString, CFPropertyListRef);
    QStringList searchPaths;
};

QSettingsPrivate::QSettingsPrivate() 
{
    if(!qt_mac_settings_base) {
	qt_mac_settings_base = new QString("com.");
	qAddPostRoutine( cleanup_qsettings );	
    }
}

bool QSettingsPrivate::writeEntry(QString key, CFPropertyListRef plr)
{	
    bool ret = FALSE;
    for ( QStringList::Iterator it = searchPaths.fromLast(); 
	  it != searchPaths.end(); --it ) {
	search_keys k((*it), key);
	CFPreferencesSetAppValue(k.key(), plr, k.id());    
	if(TRUE) { //hmmm..
	    ret = TRUE;
	    break;
	}
    }
    return ret;
}

QSettings::QSettings()
{
    d = new QSettingsPrivate;
    Q_CHECK_PTR(d);
    insertSearchPath(Mac, "");
}

QSettings::~QSettings()
{
    sync();
    delete d;
    d = NULL;
}

bool QSettings::sync()
{
    if(CFPreferencesAppSynchronize(kCFPreferencesAnyApplication))
	return FALSE;
    return TRUE;
}

void QSettings::insertSearchPath( System s, const QString &path)
{
    if ( s != Mac )
	return;
    QString realpath = path;
    while(realpath.right(1) == "/")
	realpath.truncate(realpath.length() -1);
    d->searchPaths.append(realpath);
}

void QSettings::removeSearchPath( System s, const QString &path)
{
    if ( s != Mac ) 
	return;
    QString realpath = path;
    while(realpath.right(1) == "/")
	realpath.truncate(realpath.length() -1);
    d->searchPaths.remove(realpath);
}

bool QSettings::readBoolEntry(const QString &key, bool def, bool *ok )
{
#ifdef QT_CHECK_STATE
    if (key.isNull() || key.isEmpty()) {
	qWarning("QSettings::readBoolEntry: invalid null/empty key.");
	if ( ok )
	    *ok = FALSE;
	return def;
    }
#endif // QT_CHECK_STATE
    for ( QStringList::Iterator it = d->searchPaths.fromLast(); 
	  it != d->searchPaths.end(); --it ) {
	search_keys k((*it), key);
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

double QSettings::readDoubleEntry(const QString &key, double def, bool *ok )
{
#ifdef QT_CHECK_STATE
    if (key.isNull() || key.isEmpty()) {
	qWarning("QSettings::readDoubleEntry: invalid null/empty key.");
	if ( ok )
	    *ok = FALSE;
	return def;
    }
#endif // QT_CHECK_STATE
    for ( QStringList::Iterator it = d->searchPaths.fromLast(); 
	  it != d->searchPaths.end(); --it ) {
	search_keys k((*it), key);
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

int QSettings::readNumEntry(const QString &key, int def, bool *ok )
{
#ifdef QT_CHECK_STATE
    if (key.isNull() || key.isEmpty()) {
	qWarning("QSettings::readNumEntry: invalid null/empty key.");
	if ( ok )
	    *ok = FALSE;
	return def;
    }
#endif // QT_CHECK_STATE
    for ( QStringList::Iterator it = d->searchPaths.fromLast(); 
	  it != d->searchPaths.end(); --it ) {
	search_keys k((*it), key);
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

QString QSettings::readEntry(const QString &key, const QString &def, bool *ok )
{
#ifdef QT_CHECK_STATE
    if (key.isNull() || key.isEmpty()) {
	qWarning("QSettings::readEntry: invalid null/empty key.");
	if ( ok )
	    *ok = FALSE;
	return def;
    }
#endif // QT_CHECK_STATE
    for ( QStringList::Iterator it = d->searchPaths.fromLast(); 
	  it != d->searchPaths.end(); --it ) {
	search_keys k((*it), key);
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
bool QSettings::writeEntry(const QString &key, bool value)
{
#ifdef QT_CHECK_STATE
    if (key.isNull() || key.isEmpty()) {
	qWarning("QSettings::writeEntry (bool): invalid null/empty key.");
	return FALSE;
    }
#endif // QT_CHECK_STATE
    CFBooleanRef val = value ? kCFBooleanTrue : kCFBooleanFalse;
    bool ret = d->writeEntry(key, val);
    CFRelease(val);
    return ret;
}
#endif

bool QSettings::writeEntry(const QString &key, double value)
{
#ifdef QT_CHECK_STATE
    if (key.isNull() || key.isEmpty()) {
	qWarning("QSettings::writeEntry (double): invalid null/empty key.");
	return FALSE;
    }
#endif // QT_CHECK_STATE
    CFNumberRef val = CFNumberCreate(NULL, kCFNumberDoubleType, &value);
    bool ret = d->writeEntry(key, val);
    CFRelease(val);
    return ret;
}

bool QSettings::writeEntry(const QString &key, const char *value)
{
#ifdef QT_CHECK_STATE
    if (key.isNull() || key.isEmpty()) {
	qWarning("QSettings::writeEntry (const char *): invalid null/empty key.");
	return FALSE;
    }
#endif // QT_CHECK_STATE
    CFStringRef val = CFStringCreateWithCString(NULL, value, kCFStringEncodingMacRoman);
    bool ret = d->writeEntry(key, val);
    CFRelease(val);
    return ret;
}

bool QSettings::writeEntry(const QString &key, int value)
{
#ifdef QT_CHECK_STATE
    if (key.isNull() || key.isEmpty()) {
	qWarning("QSettings::writeEntry (int): invalid null/empty key.");
	return FALSE;
    }
#endif // QT_CHECK_STATE
    CFNumberRef val = CFNumberCreate(NULL, kCFNumberIntType, &value);
    bool ret = d->writeEntry(key, val);
    CFRelease(val);
    return ret;
}

bool QSettings::writeEntry(const QString &key, const QString &value)
{
#ifdef QT_CHECK_STATE
    if (key.isNull() || key.isEmpty()) {
	qWarning("QSettings::writeEntry (QString): invalid null/empty key.");
	return FALSE;
    }
#endif // QT_CHECK_STATE
    CFStringRef val = CFStringCreateWithCharacters(NULL, (UniChar *)value.unicode(), 
						 value.length());
    bool ret = d->writeEntry(key, val);
    CFRelease(val);
    return ret;
}

bool QSettings::removeEntry(const QString &key)
{
#ifdef QT_CHECK_STATE
    if (key.isNull() || key.isEmpty()) {
	qWarning("QSettings::removeEntry: invalid null/empty key.");
	return FALSE;
    }
#endif // QT_CHECK_STATE
    return d->writeEntry(key, NULL);
}

QStringList QSettings::entryList(const QString &key) const
{
#ifdef QT_CHECK_STATE
    if ( key.isNull() || key.isEmpty() ) {
	qWarning("QSettings::entryList: invalid null/empty key.");
	return QStringList();
    }
#endif // QT_CHECK_STATE
    for ( QStringList::Iterator it = d->searchPaths.fromLast(); 
	  it != d->searchPaths.end(); --it ) {
	search_keys k((*it), key);
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

QStringList QSettings::subkeyList(const QString &key) const
{
#ifdef QT_CHECK_STATE
    if ( key.isNull() || key.isEmpty() ) {
	qWarning("QSettings::subkeyList: invalid null/empty key.");
	return QStringList();
    }
#endif // QT_CHECK_STATE
    for ( QStringList::Iterator it = d->searchPaths.fromLast(); 
	  it != d->searchPaths.end(); --it ) {
	search_keys k((*it), key);
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

QDateTime QSettings::lastModficationTime(const QString &)
{
    return QDateTime(); //N/A
}

#endif //QT_NO_SETTINGS
