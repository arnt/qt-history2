/****************************************************************************
** $Id$
**
** Implementation of QSettings class
**
** Created : 2000.06.26
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
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

#include "qt_windows.h"
#include "qregexp.h"

#ifndef QT_NO_SETTINGS

static bool settingsTryUser = TRUE;
static bool settingsTryLocal = TRUE;
static QString *settingsBasePath = 0;

void Q_EXPORT qt_setSettingsTryUser( bool tryUser )
{
    settingsTryUser = tryUser;
}

void Q_EXPORT qt_setSettingsTryLocal( bool tryLocal )
{
    settingsTryLocal = tryLocal;
}

void Q_EXPORT qt_setSettingsBasePath( const QString &base )
{
    if ( settingsBasePath ) {
	qWarning( "qt_setSettingsBasePath has to be called without any settings object being instantiated!" );
	return;
    }
    settingsBasePath = new QString( base );
}

class QSettingsWinPrivate
{
public:
    QSettingsWinPrivate();
    ~QSettingsWinPrivate();

    HKEY user;
    HKEY local;

    QString folder( const QString& );
    QString entry( const QString& );

    QString validateKey( const QString &key );

    bool writeKey( const QString &key, const QByteArray &value, ulong type );
    QByteArray readKey( const QString &key, bool *ok );

    HKEY openKey( const QString &key, bool write );

    QStringList paths;

private:
    static uint refCount;
};

uint QSettingsWinPrivate::refCount = 0;

QSettingsWinPrivate::QSettingsWinPrivate()
{
    paths.append( "" );
    if ( !settingsBasePath ) {
	settingsBasePath = new QString("Software");
    }
    refCount++;
    local = 0;
    user  = 0 ;

    LONG res;
    if ( settingsTryLocal ) {
#ifdef Q_OS_TEMP
	res = RegOpenKeyExW( HKEY_LOCAL_MACHINE, NULL, 0, KEY_ALL_ACCESS, &local );
#else
#if defined(UNICODE)
	if ( qWinVersion() & Qt::WV_NT_based )
	    res = RegOpenKeyExW( HKEY_LOCAL_MACHINE, NULL, 0, KEY_ALL_ACCESS, &local );
	else
#endif
	    res = RegOpenKeyExA( HKEY_LOCAL_MACHINE, NULL, 0, KEY_ALL_ACCESS, &local );
#endif

	if ( res != ERROR_SUCCESS ) {
#ifdef Q_OS_TEMP
	    res = RegOpenKeyExW( HKEY_LOCAL_MACHINE, NULL, 0, KEY_READ, &local );
#else
#if defined(UNICODE)
	    if ( qWinVersion() & Qt::WV_NT_based )
		res = RegOpenKeyExW( HKEY_LOCAL_MACHINE, NULL, 0, KEY_READ, &local );
	    else
#endif
		res = RegOpenKeyExA( HKEY_LOCAL_MACHINE, NULL, 0, KEY_READ, &local );
#endif
	    if ( res != ERROR_SUCCESS ) {
		local = NULL;
	    }
	}
    }

    if ( settingsTryUser ) {
#ifdef Q_OS_TEMP
	res = RegOpenKeyExW( HKEY_CURRENT_USER, NULL, 0, KEY_ALL_ACCESS, &user );
#else
#if defined(UNICODE)
	if ( qWinVersion() & Qt::WV_NT_based ) 
	    res = RegOpenKeyExW( HKEY_CURRENT_USER, NULL, 0, KEY_ALL_ACCESS, &user );
	else
#endif
	    res = RegOpenKeyExA( HKEY_CURRENT_USER, NULL, 0, KEY_ALL_ACCESS, &user );
#endif

	if ( res != ERROR_SUCCESS ) {
#ifdef Q_OS_TEMP
	    res = RegOpenKeyExW( HKEY_CURRENT_USER, NULL, 0, KEY_READ, &user );
#else
#if defined(UNICODE)
	    if ( qWinVersion() & Qt::WV_NT_based ) 
		res = RegOpenKeyExW( HKEY_CURRENT_USER, NULL, 0, KEY_READ, &user );
	    else
#endif
		res = RegOpenKeyExA( HKEY_CURRENT_USER, NULL, 0, KEY_READ, &user );
#endif
	    if ( res != ERROR_SUCCESS ) {
		user = NULL;
	    }
	}
    }

#if defined(QT_CHECK_STATE)
    if ( !local && !user )
	qSystemWarning( "Error opening registry!", res );
#endif
}

QSettingsWinPrivate::~QSettingsWinPrivate()
{
    LONG res;
    if ( local ) {
	res = RegCloseKey( local );
#if defined(QT_CHECK_STATE)
	if ( res != ERROR_SUCCESS )
	    qSystemWarning( "Error closing local machine!", res );
#endif
    }
    if ( user ) {
	res = RegCloseKey( user );
#if defined(QT_CHECK_STATE)
	if ( res != ERROR_SUCCESS )
	    qSystemWarning( "Error closing current user!", res );
#endif
    }

    // Make sure that we only delete the base path if no one else is using it anymore
    if (refCount > 0) {
	refCount--;

	if (refCount == 0) {
	    delete settingsBasePath;
	    settingsBasePath = 0;
	}
    }
}

QString QSettingsWinPrivate::validateKey( const QString &key )
{
    if ( key.isEmpty() )
	return key;

    QString newKey = key;
    newKey = newKey.replace( QRegExp( "[/]+" ), "\\" );

    if ( newKey[0] != '\\' )
	newKey = "\\" + newKey;
    if ( newKey[(int)newKey.length() - 1] == '\\' )
	newKey = newKey.left( newKey.length() - 2 );

    return newKey;
}

QString QSettingsWinPrivate::folder( const QString &key )
{
    QString k = validateKey( key );
    Q_ASSERT(settingsBasePath);
    return *settingsBasePath + k.left( k.findRev( "\\" ) );
}

QString QSettingsWinPrivate::entry( const QString &key )
{
    QString k = validateKey( key );
    return k.right( k.length() - k.findRev( "\\" ) - 1 );
}

HKEY QSettingsWinPrivate::openKey( const QString &key, bool write )
{
    QString f = folder( key );

    HKEY handle = 0;
    LONG res = ERROR_FILE_NOT_FOUND;

    // if we write and there is a user specific setting, overwrite that
    if ( write && user ) {
#if defined(UNICODE)
#ifndef Q_OS_TEMP
	if ( qWinVersion() & Qt::WV_NT_based )
#endif
	    res = RegOpenKeyExW( user, (TCHAR*)qt_winTchar( f, TRUE ), 0, KEY_ALL_ACCESS, &handle );
#ifndef Q_OS_TEMP
	else
#endif
#endif
#ifndef Q_OS_TEMP
	    res = RegOpenKeyExA( user, f.local8Bit(), 0, KEY_ALL_ACCESS, &handle );
#endif
    }

    if ( res != ERROR_SUCCESS && local ) {
#if defined(UNICODE)
#ifndef Q_OS_TEMP
	if ( qWinVersion() & Qt::WV_NT_based ) {
#endif
	    if ( write )
		res = RegCreateKeyExW( local, (TCHAR*)qt_winTchar( f, TRUE ), 0, (TCHAR*)qt_winTchar( "", TRUE ), REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &handle, NULL );
	    else
		res = RegOpenKeyExW( local, (TCHAR*)qt_winTchar( f, TRUE ), 0, KEY_ALL_ACCESS, &handle );
#ifndef Q_OS_TEMP
	} else
#endif
#endif
#ifndef Q_OS_TEMP
	{
	    if ( write )
		res = RegCreateKeyExA( local, f.local8Bit(), 0, "", REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &handle, NULL );
	    else
		res = RegOpenKeyExA( local, f.local8Bit(), 0, KEY_ALL_ACCESS, &handle );
	}
#endif
    }
    if ( !handle && user ) {
#if defined(UNICODE)
#ifndef Q_OS_TEMP
	if ( qWinVersion() & Qt::WV_NT_based ) {
#endif
	    if ( write )
		res = RegCreateKeyExW( user, (TCHAR*)qt_winTchar( f, TRUE ), 0, (TCHAR*)qt_winTchar( "", TRUE ), REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &handle, NULL );
	    else
		res = RegOpenKeyExW( user, (TCHAR*)qt_winTchar( f, TRUE ), 0, KEY_ALL_ACCESS, &handle );
#ifndef Q_OS_TEMP
	} else
#endif
#endif
#ifndef Q_OS_TEMP
	{
	    if ( write )
		res = RegCreateKeyExA( user, f.local8Bit(), 0, "", REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &handle, NULL );
	    else
		res = RegOpenKeyExA( user, f.local8Bit(), 0, KEY_ALL_ACCESS, &handle );
	}
#endif
    }
    return handle;
}

bool QSettingsWinPrivate::writeKey( const QString &key, const QByteArray &value, ulong type )
{
    QString e;
    LONG res;

    HKEY handle = 0;
    for ( QStringList::Iterator it = paths.fromLast(); it != paths.end(); --it ) {
	QString k = *it + "/" + key;
	e = entry( k );
	handle = openKey( k, TRUE );
	if ( handle )
	    break;
    }
    if ( !handle )
	return FALSE;
    
    if (e == "Default" || e == "." )
	e = "";

    if ( value.size() ) {
#ifdef Q_OS_TEMP
	res = RegSetValueExW( handle, e.isEmpty() ? 0 : (TCHAR*)qt_winTchar( e, TRUE ), 0, type, (const uchar*)value.data(), value.size() );
#else
#if defined(UNICODE)
	if ( qWinVersion() & Qt::WV_NT_based )
	    res = RegSetValueExW( handle, e.isEmpty() ? 0 : (TCHAR*)qt_winTchar( e, TRUE ), 0, type, (const uchar*)value.data(), value.size() );
	else
#endif
	    res = RegSetValueExA( handle, e.isEmpty() ? (const char*)0 : (const char*)e.local8Bit(), 0, type, (const uchar*)value.data(), value.size() );
#endif
	
	if ( res != ERROR_SUCCESS ) {
#if defined(QT_CHECK_STATE)
	    qSystemWarning( "Couldn't write value " + key, res );
#endif
	    return FALSE;
	}
    }
    
    RegCloseKey( handle );
    return TRUE;
}

QByteArray QSettingsWinPrivate::readKey( const QString &key, bool *ok )
{
    HKEY handle = 0;
    LONG res;
    ulong size = 0;
    QString e;
    for ( QStringList::Iterator it = paths.fromLast(); it != paths.end(); --it ) {
	if ( handle ) {
	    RegCloseKey( handle );
	    handle = 0;
	}

	QString k = *it + "/" + key;
	QString f = folder( k );
	e = entry( k );
	if ( e == "Default" || e == "." )
	    e = "";
	if ( user ) {
#ifdef Q_OS_TEMP
	    res = RegOpenKeyExW( user, (TCHAR*)qt_winTchar( f, TRUE ), 0, KEY_READ, &handle );
#else
#if defined(UNICODE)
	    if ( qWinVersion() & Qt::WV_NT_based )
		res = RegOpenKeyExW( user, (TCHAR*)qt_winTchar( f, TRUE ), 0, KEY_READ, &handle );
	    else
#endif
		res = RegOpenKeyExA( user, f.local8Bit(), 0, KEY_READ, &handle );
#endif

	    if ( res == ERROR_SUCCESS ) {
#ifdef Q_OS_TEMP
		res = RegQueryValueExW( handle, e.isEmpty() ? 0 : (TCHAR*)qt_winTchar( e, TRUE ), NULL, NULL, NULL, &size );
#else
#if defined(UNICODE)
		if ( qWinVersion() & Qt::WV_NT_based )
		    res = RegQueryValueExW( handle, e.isEmpty() ? 0 : (TCHAR*)qt_winTchar( e, TRUE ), NULL, NULL, NULL, &size );
		else
#endif
		    res = RegQueryValueExA( handle, e.isEmpty() ? (const char*)0 : (const char*)e.local8Bit(), NULL, NULL, NULL, &size );
#endif
	    }
	}
	if ( res != ERROR_SUCCESS )
	    size = 0;
	else if ( size )
	    break;
    }

    if ( !size && local ) {
	for ( QStringList::Iterator it = paths.fromLast(); it != paths.end(); --it ) {
	    if ( handle ) {
		RegCloseKey( handle );
		handle = 0;
	    }

	    QString k = *it + "/" + key;

	    QString f = folder( k );
	    e = entry( k );
	    if ( e == "Default" || e == "." )
		e = "";

#ifdef Q_OS_TEMP
	    res = RegOpenKeyExW( local, (TCHAR*)qt_winTchar( f, TRUE ), 0, KEY_READ, &handle );
#else
#if defined(UNICODE)
	    if ( qWinVersion() & Qt::WV_NT_based )
		res = RegOpenKeyExW( local, (TCHAR*)qt_winTchar( f, TRUE ), 0, KEY_READ, &handle );
	    else
#endif
		res = RegOpenKeyExA( local, f, 0, KEY_READ, &handle );
#endif

	    if ( res == ERROR_SUCCESS ) {
#ifdef Q_OS_TEMP
		res = RegQueryValueExW( handle, e.isEmpty() ? 0 : (TCHAR*)qt_winTchar( e, TRUE ), NULL, NULL, NULL, &size );
#else
#if defined(UNICODE)
		if ( qWinVersion() & Qt::WV_NT_based )
		    res = RegQueryValueExW( handle, e.isEmpty() ? 0 : (TCHAR*)qt_winTchar( e, TRUE ), NULL, NULL, NULL, &size );
		else
#endif
		    res = RegQueryValueExA( handle, e.isEmpty() ? (const char*)0 : (const char*)e.local8Bit(), NULL, NULL, NULL, &size );
#endif
	    }
	    if ( res != ERROR_SUCCESS )
		size = 0;
	    else if ( size )
		break;
	}
    }

    if ( !size ) {
	if ( ok )
	    *ok = FALSE;
	if ( handle )
	    RegCloseKey( handle );
	return QByteArray();
    }

    uchar* data = new uchar[ size ];
#ifdef Q_OS_TEMP
    RegQueryValueExW( handle, e.isEmpty() ? 0 : (TCHAR*)qt_winTchar( e, TRUE ), NULL, NULL, data, &size );
#else
#if defined(UNICODE)
    if ( qWinVersion() & Qt::WV_NT_based )
	RegQueryValueExW( handle, e.isEmpty() ? 0 : (TCHAR*)qt_winTchar( e, TRUE ), NULL, NULL, data, &size );
    else
#endif
	RegQueryValueExA( handle, e.isEmpty() ? (const char*)0 : (const char*)e.local8Bit(), NULL, NULL, data, &size );
#endif

    QByteArray result;
    result.setRawData( (const char*)data, size );
    RegCloseKey( handle );

    if ( ok )
	*ok = TRUE;
    return result;
}

#endif //QT_NO_SETTINGS
