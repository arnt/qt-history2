#include "environment.h"
#include <stdlib.h>
#include <windows.h>
#include <qnamespace.h>

QString QEnvironment::getEnv( QString varName, int envBlock )
{

    if( envBlock & DefaultEnv ) {
	HKEY env;
	QByteArray buffer;
	DWORD size( 0 );
	QString value;

	if( RegOpenKeyExW( HKEY_CURRENT_USER, (WCHAR*)qt_winTchar( QString( "Environment" ), true ), 0, KEY_READ, &env ) == ERROR_SUCCESS ) {
	    RegQueryValueExW( env, (WCHAR*)qt_winTchar( varName, true ), 0, NULL, NULL, &size );
	    buffer.resize( size );
	    RegQueryValueExW( env, (WCHAR*)qt_winTchar( varName, true ), 0, NULL, (unsigned char*)buffer.data(), &size );
	    for( int i = 0; i < buffer.size(); i += 2 ) {
		QChar c( buffer[ i ], buffer[ i + 1 ] );
		if( !c.isNull() )
		    value += c;
	    }
	    RegCloseKey( env );
	    return value;
	}
	else {
	    return QString::null;
	}
    }
    if( envBlock & LocalEnv ) {
	return QString( getenv( varName ) );
    }
    return QString::null;
}

void QEnvironment::putEnv( QString varName, QString varValue, int envBlock )
{
    if( envBlock & DefaultEnv ) {
	HKEY env;
	QByteArray buffer;

	buffer.resize( varValue.length() * 2 + 2 );
	const QChar *data = varValue.unicode();
	for ( int i = 0; i < (int)varValue.length(); ++i ) {
	    buffer[ 2*i ] = data[ i ].cell();
	    buffer[ (2*i)+1 ] = data[ i ].row();
	}
	buffer[ (2*i) ] = 0;
	buffer[ (2*i)+1 ] = 0;

	if( RegOpenKeyExW( HKEY_CURRENT_USER, (WCHAR*)qt_winTchar( QString( "Environment" ), true ), 0, KEY_WRITE, &env ) == ERROR_SUCCESS ) {
	    RegSetValueExW( env, (WCHAR*)qt_winTchar( varName, true ), 0, REG_EXPAND_SZ, (const unsigned char*)buffer.data(), buffer.size() );
	    RegCloseKey( env );
	}
	else {
	}
    }
    if( envBlock & LocalEnv ) {
	putenv( varName + QString( "=" ) + varValue );
    }
}

bool QEnvironment::recordUninstall( QString displayName, QString cmdString )
{
    HKEY key;
    QByteArray buffer;

    if( int( qWinVersion() ) & int( Qt::WV_NT_based ) ) {
	if( RegCreateKeyExW( HKEY_LOCAL_MACHINE, (WCHAR*)qt_winTchar( QString( "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" ) + displayName, true ), 0, NULL, 0, KEY_WRITE, NULL, &key, NULL ) == ERROR_SUCCESS ) {
	    const QChar* data;
	    int i;

	    buffer.resize( displayName.length() * 2 + 2 );
	    data = displayName.unicode();
	    for ( i = 0; i < (int)displayName.length(); ++i ) {
		buffer[ 2*i ] = displayName[ i ].cell();
		buffer[ (2*i)+1 ] = displayName[ i ].row();
	    }
	    buffer[ (2*i) ] = 0;
	    buffer[ (2*i)+1 ] = 0;
	    RegSetValueExW( key, (WCHAR*)qt_winTchar( "DisplayName", true ), 0, REG_SZ, (const unsigned char*)buffer.data(), buffer.size() );
	    
	    buffer.resize( cmdString.length() * 2 + 2 );
	    data = cmdString.unicode();
	    for ( i = 0; i < (int)cmdString.length(); ++i ) {
		buffer[ 2*i ] = cmdString[ i ].cell();
		buffer[ (2*i)+1 ] = cmdString[ i ].row();
	    }
	    buffer[ (2*i) ] = 0;
	    buffer[ (2*i)+1 ] = 0;
	    RegSetValueExW( key, (WCHAR*)qt_winTchar( "UninstallString", true ), 0, REG_SZ, (const unsigned char*)buffer.data(), buffer.size() );
	    
	    RegCloseKey( key );
	    return true;
	}
    }
    else {
	if( RegCreateKeyExA( HKEY_LOCAL_MACHINE, QString( QString( "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" ) + displayName ).local8Bit(), 0, NULL, 0, KEY_WRITE, NULL, &key, NULL ) == ERROR_SUCCESS ) {
	    RegSetValueExA( key, "DisplayName", 0, REG_SZ, (const unsigned char*)displayName.latin1(), displayName.length() + 1 );
	    RegSetValueExA( key, "UninstallString", 0, REG_SZ, (const unsigned char*)cmdString.latin1(), cmdString.length() + 1 );

	    RegCloseKey( key );
	    return true;
	}
    }
    return false;
}

bool QEnvironment::removeUninstall( QString displayName )
{
    HKEY key;
    
    if( int( qWinVersion() ) & int( Qt::WV_NT_based ) ) {
	if( RegOpenKeyExW( HKEY_LOCAL_MACHINE, (WCHAR*)qt_winTchar( QString( "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall" ), true ), 0, KEY_WRITE, &key ) == ERROR_SUCCESS )
	    RegDeleteKeyW( key, (WCHAR*)qt_winTchar( QString( displayName ), true ) );
	return true;
    }
    else {
	if( RegOpenKeyExA( HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", 0, KEY_WRITE, &key ) == ERROR_SUCCESS )
	    RegDeleteKeyA( key, displayName.local8Bit() );
	return true;
    }
    return false;
}

QString QEnvironment::getRegistryString( QString keyName, QString valueName, int scope )
{
    QString value;
    HKEY scopeKeys[] = { HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE };
    HKEY key;
    DWORD valueSize( 0 );
    QByteArray buffer, expBuffer;

    if( int( qWinVersion() ) & int(Qt::WV_NT_based) ) {
	if( RegOpenKeyExW( scopeKeys[ scope ], (WCHAR*)qt_winTchar( keyName, true ), 0, KEY_READ, &key ) == ERROR_SUCCESS ) {
	    if( RegQueryValueExW( key, (WCHAR*)qt_winTchar( valueName, true ), NULL, NULL, NULL, &valueSize ) == ERROR_SUCCESS ) {
		buffer.resize( valueSize );
		if( RegQueryValueExW( key, (WCHAR*)qt_winTchar( valueName, true ), NULL, NULL, (unsigned char*)buffer.data(), &valueSize ) == ERROR_SUCCESS ) {
		    valueSize = ExpandEnvironmentStringsW( (WCHAR*)buffer.data(), NULL, 0 );
		    expBuffer.resize( valueSize * 2 );
		    ExpandEnvironmentStringsW( (WCHAR*)buffer.data(), (WCHAR*)expBuffer.data(), valueSize );
		    for( int i = 0; i < expBuffer.size(); i += 2 ) {
			QChar c( expBuffer[ i ], expBuffer[ i + 1 ] );
			if ( !c.isNull() )
			    value += c;
		    }
		}
	    }
	    RegCloseKey( key );
	}
    }
    else {
	if( RegOpenKeyExA( HKEY_CURRENT_USER, keyName.local8Bit(), 0, KEY_READ, &key ) == ERROR_SUCCESS ) {
	    if( RegQueryValueExA( key, valueName.local8Bit(), NULL, NULL, NULL, &valueSize ) == ERROR_SUCCESS ) {
		buffer.resize( valueSize );
		if( RegQueryValueExA( key, valueName.local8Bit(), NULL, NULL, (unsigned char*)buffer.data(), &valueSize ) == ERROR_SUCCESS ) {
		    valueSize = ExpandEnvironmentStringsA( buffer.data(), NULL, 0 );
		    expBuffer.resize( valueSize );
		    ExpandEnvironmentStringsA( buffer.data(), expBuffer.data(), valueSize );
		    value = expBuffer.data();
		}
	    }
	    RegCloseKey( key );
	}
    }
    
    return value;
}

QString QEnvironment::getTempPath()
{
    DWORD tmpSize;
    QByteArray tmp;
    QString tmpPath;

    if( int( qWinVersion() ) & int( Qt::WV_NT_based ) ) {
	tmpSize = GetTempPathW( 0, NULL );
	tmp.resize( tmpSize * 2 );
	GetTempPathW( tmpSize, (WCHAR*)tmp.data() );
	for( int i = 0; i < tmp.size(); i += 2 ) {
	    QChar c( tmp[ i ], tmp[ i + 1 ] );
	    if( !c.isNull() )
		tmpPath += c;
	}
    }
    else {
	tmpSize = GetTempPathA( 0, NULL );
	tmp.resize( tmpSize * 2 );
	GetTempPathA( tmpSize, tmp.data() );
	tmpPath = tmp.data();
    }

    return tmpPath;
}
