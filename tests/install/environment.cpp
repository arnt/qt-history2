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

QString QEnvironment::getRegistryString( QString keyName, QString valueName, int scope )
{
    QString value;
    HKEY scopeKeys[] = { HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE };
    HKEY key;
    DWORD valueSize( 0 );
    QByteArray buffer, expBuffer;

    if( int(qWinVersion) & int(Qt::WV_NT_based) ) {
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

    if( int( qWinVersion ) & int( Qt::WV_NT_based ) ) {
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
