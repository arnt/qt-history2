#include "qsettings.h"
#include "qregexp.h"
#include "qt_windows.h"

class QSettingsPrivate
{
public:
    QSettingsPrivate();
    ~QSettingsPrivate();

    HKEY user;
    HKEY local;

    QString folder( const QString& );
    QString entry( const QString& );

    QString validateKey( const QString &key );

    bool writeKey( const QString &key, const QByteArray &value, ulong type );
    QByteArray readKey( const QString &key, bool *ok );

    HKEY openKey( const QString &key, bool create );

    QStringList paths;
};

QSettingsPrivate::QSettingsPrivate()
{
    paths.append( "" );

    long res;
#ifdef _WIN32_WCE
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
#ifdef _WIN32_WCE
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
#ifdef _WIN32_WCE
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
#ifdef _WIN32_WCE
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
#if defined(QT_CHECK_STATE)
    if ( !local && !user )
	qSystemWarning( "Error opening registry!", res );
#endif
}

QSettingsPrivate::~QSettingsPrivate()
{
    long res;
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
}

inline QString QSettingsPrivate::validateKey( const QString &key )
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

inline QString QSettingsPrivate::folder( const QString &key )
{
    QString k = validateKey( key );
    return "Software" + k.left( k.findRev( "\\" ) );
}

inline QString QSettingsPrivate::entry( const QString &key )
{
    QString k = validateKey( key );
    return k.right( k.length() - k.findRev( "\\" ) - 1 );
}

inline HKEY QSettingsPrivate::openKey( const QString &key, bool create )
{
    QString f = folder( key );

    HKEY handle = 0;
    long res;

    if ( local ) {
#if defined(UNICODE)
#ifndef _WIN32_WCE
	if ( qWinVersion() & Qt::WV_NT_based ) {
#endif
	    if ( create )
		res = RegCreateKeyExW( local, (TCHAR*)qt_winTchar( f, TRUE ), 0, (TCHAR*)qt_winTchar( "", TRUE ), REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &handle, NULL );
	    else
		res = RegOpenKeyExW( local, (TCHAR*)qt_winTchar( f, TRUE ), 0, KEY_ALL_ACCESS, &handle );
#ifndef _WIN32_WCE
	} else
#endif
#endif
#ifndef _WIN32_WCE
	{
	    if ( create )
		res = RegCreateKeyExA( local, f.local8Bit(), 0, "", REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &handle, NULL );
	    else
		res = RegOpenKeyExA( local, f.local8Bit(), 0, KEY_ALL_ACCESS, &handle );
	}
#endif
#if defined(QT_CHECK_STATE)
	if ( res != ERROR_SUCCESS )
	    qSystemWarning("Couldn't open folder " + f + " for writing", res );
#endif
    }
    if ( !handle && user ) {
#if defined(UNICODE)
#ifndef _WIN32_WCE
	if ( qWinVersion() & Qt::WV_NT_based ) {
#endif
	    if ( create )
		res = RegCreateKeyExW( user, (TCHAR*)qt_winTchar( f, TRUE ), 0, (TCHAR*)qt_winTchar( "", TRUE ), REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &handle, NULL );
	    else
		res = RegOpenKeyExW( user, (TCHAR*)qt_winTchar( f, TRUE ), 0, KEY_ALL_ACCESS, &handle );
#ifndef _WIN32_WCE
	} else
#endif
#endif
#ifndef _WIN32_WCE
	{
	    if ( create )
		res = RegCreateKeyExA( user, f.local8Bit(), 0, "", REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &handle, NULL );
	    else
		res = RegOpenKeyExA( user, f.local8Bit(), 0, KEY_ALL_ACCESS, &handle );
	}
#endif
#if defined(QT_CHECK_STATE)
	if ( res != ERROR_SUCCESS )
	    qSystemWarning( "Couldn't open folder " + f + " for writing", res );
#endif
    }
    return handle;
}

inline bool QSettingsPrivate::writeKey( const QString &key, const QByteArray &value, ulong type )
{
    QString e;
    long res;

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
    
    if (e == "Default" )
	e = "";

#ifdef _WIN32_WCE
	res = RegSetValueExW( handle, (TCHAR*)qt_winTchar( e, TRUE ), 0, type, (const uchar*)value.data(), value.size() );
#else
#if defined(UNICODE)
    if ( qWinVersion() & Qt::WV_NT_based )
	res = RegSetValueExW( handle, (TCHAR*)qt_winTchar( e, TRUE ), 0, type, (const uchar*)value.data(), value.size() );
    else
#endif
	res = RegSetValueExA( handle, e.local8Bit(), 0, type, (const uchar*)value.data(), value.size() );
#endif

    if ( res != ERROR_SUCCESS ) {
#if defined(QT_CHECK_STATE)
	qSystemWarning( "Couldn't write value " + key, res );
#endif
	return FALSE;
    }

    RegCloseKey( handle );
    return TRUE;
}

inline QByteArray QSettingsPrivate::readKey( const QString &key, bool *ok )
{
    HKEY handle = 0;
    long res;
    ulong size = 0;
    QString e;
    for ( QStringList::Iterator it = paths.fromLast(); it != paths.end(); --it ) {
	QString k = *it + "/" + key;
	QString f = folder( k );
	e = entry( k );
	if ( e == "Default" )
	    e = "";
	if ( user ) {
#ifdef _WIN32_WCE
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
#ifdef _WIN32_WCE
		    res = RegQueryValueExW( handle, (TCHAR*)qt_winTchar( e, TRUE ), NULL, NULL, NULL, &size );
#else
#if defined(UNICODE)
		if ( qWinVersion() & Qt::WV_NT_based )
		    res = RegQueryValueExW( handle, (TCHAR*)qt_winTchar( e, TRUE ), NULL, NULL, NULL, &size );
		else
#endif
		    res = RegQueryValueExA( handle, e.local8Bit(), NULL, NULL, NULL, &size );
#endif
	    }
	}
	if ( size )
	    break;
    }

    if ( !size && local ) {
	for ( QStringList::Iterator it = paths.fromLast(); it != paths.end(); --it ) {
	    QString k = *it + "/" + key;

	    QString f = folder( k );
	    e = entry( k );
	    if ( e == "Default" )
		e = "";

#ifdef _WIN32_WCE
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
#ifdef _WIN32_WCE
		    res = RegQueryValueExW( handle, (TCHAR*)qt_winTchar( e, TRUE ), NULL, NULL, NULL, &size );
#else
#if defined(UNICODE)
		if ( qWinVersion() & Qt::WV_NT_based )
		    res = RegQueryValueExW( handle, (TCHAR*)qt_winTchar( e, TRUE ), NULL, NULL, NULL, &size );
		else
#endif
		    res = RegQueryValueExA( handle, e.local8Bit(), NULL, NULL, NULL, &size );
#endif
	    }
	    if ( size )
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
#ifdef _WIN32_WCE
	RegQueryValueExW( handle, (TCHAR*)qt_winTchar( e, TRUE ), NULL, NULL, data, &size );
#else
#if defined(UNICODE)
    if ( qWinVersion() & Qt::WV_NT_based )
	RegQueryValueExW( handle, (TCHAR*)qt_winTchar( e, TRUE ), NULL, NULL, data, &size );
    else
#endif
	RegQueryValueExA( handle, e.local8Bit(), NULL, NULL, data, &size );
#endif

    QByteArray result;
    result.setRawData( (const char*)data, size );
    RegCloseKey( handle );

    if ( ok )
	*ok = TRUE;
    return result;
}

QSettings::QSettings()
{
    d = new QSettingsPrivate;
    Q_CHECK_PTR( d );
}

QSettings::~QSettings()
{
    delete d;
}

bool QSettings::sync()
{
    if ( d->local )
	RegFlushKey( d->local );
    if ( d->user )
	RegFlushKey( d->user );

    return TRUE;
}

bool QSettings::writeEntry( const QString &key, bool value )
{
    return writeEntry( key, int(value) );
}

bool QSettings::writeEntry( const QString &key, double value )
{
    QByteArray array( sizeof(double) );
    const char *data = (char*)&value;

    for ( int i = 0; i < sizeof(double); ++i )
	array[i] = data[i];

    return d->writeKey( key, array, REG_BINARY );
}

bool QSettings::writeEntry( const QString &key, int value )
{
    QByteArray array( sizeof(int) );
    const char* data = (char*)&value;
    for ( int i = 0; i < sizeof(int ); ++i )
	array[i] = data[i];

    return d->writeKey( key, array, REG_DWORD );
}

bool QSettings::writeEntry( const QString &key, const char *value )
{
    return writeEntry( key, QString( value ) );
}

bool QSettings::writeEntry( const QString &key, const QString &value )
{
    QByteArray array( 0 );
#if defined(UNICODE)
#ifndef _WIN32_WCE
    if ( qWinVersion() & Qt::WV_NT_based ) {
#endif
	array.resize( value.length() * 2 + 2 );
	const QChar *data = value.unicode();
	int i;
	for ( i = 0; i < (int)value.length(); ++i ) {
	    array[ 2*i ] = data[ i ].cell();
	    array[ (2*i)+1 ] = data[ i ].row();
	}

	array[ (2*i) ] = 0;
	array[ (2*i)+1 ] = 0;
#ifndef _WIN32_WCE
    } else
#endif
#endif
#ifndef _WIN32_WCE
    {
	array.resize( value.length() );
	array = value.local8Bit();
    }
#endif

    return d->writeKey( key, array, REG_SZ );
}

bool QSettings::writeEntry( const QString &key, const QStringList &value, const QChar &sep )
{
    QString joined = value.join( sep );
    return writeEntry( key, joined );
}

QStringList QSettings::readListEntry( const QString &key, const QChar &sep, bool *ok )
{
    QString joined = readEntry( key, QString::null, ok );
    if ( (!ok) || ( ok && *ok ) )
	return QStringList::split( sep, joined );
    else
	return QStringList();
}

QString QSettings::readEntry( const QString &key, const QString &def, bool *ok )
{
    if ( ok )
	*ok = FALSE;
    bool temp;
    QByteArray array = d->readKey( key, &temp );
    if ( !temp ) {
	char *data = array.data();
	array.resetRawData( data, array.size() );
	delete[] data;
	return def;
    }

    if ( ok )
	*ok = TRUE;
    QString result = QString::null;

#if defined(UNICODE)
#ifndef _WIN32_WCE
    if ( qWinVersion() & Qt::WV_NT_based ) {
#endif
	int s = array.size();
	for ( int i = 0; i < s; i+=2 ) {
	    QChar c( array[ i ], array[ i+1 ] );
	    if( !c.isNull() )
		result+=c;
	}
#ifndef _WIN32_WCE
    } else
#endif
#endif
#ifndef _WIN32_WCE
	result = QString::fromLocal8Bit( array );
#endif

    char *data = array.data();
    array.resetRawData( data, array.size() );
    delete[] data;

    return result;
}

int QSettings::readNumEntry( const QString &key, int def, bool *ok )
{
    if ( ok )
	*ok = FALSE;
    bool temp;
    QByteArray array = d->readKey( key, &temp );
    if ( !temp ) {
	char *data = array.data();
	array.resetRawData( data, array.size() );
	delete[] data;

	return def;
    }

    if ( array.size() != sizeof(int) )
	return def;

    if ( ok )
	*ok = TRUE;

    int res = 0;
    char* data = (char*)&res;
    for ( int i = 0; i < sizeof(int); ++i )
	data[i] = array[ i ];

    char *adata = array.data();
    array.resetRawData( adata, array.size() );
    delete[] adata;
    return res;
}

double QSettings::readDoubleEntry( const QString &key, double def, bool *ok )
{
    if ( ok )
	*ok = FALSE;
    bool temp;
    QByteArray array = d->readKey( key, &temp );
    if ( !temp ) {
	char *data = array.data();
	array.resetRawData( data, array.size() );
	delete[] data;
	return def;
    }
    if ( array.size() != sizeof(double) )
	return def;

    if ( ok )
	*ok = TRUE;

    double res = 0;
    char* data = (char*)&res;
    for ( int i = 0; i < sizeof(double); ++i )
	data[i] = array[ i ];

    char *adata = array.data();
    array.resetRawData( adata, array.size() );
    delete[] adata;
    return res;
}

bool QSettings::readBoolEntry( const QString &key, bool def, bool *ok )
{
    return readNumEntry( key, def, ok );
}

bool QSettings::removeEntry( const QString &key )
{
    QString e;
    long res;

    HKEY handle = 0;
    for ( QStringList::Iterator it = d->paths.fromLast(); it != d->paths.end(); --it ) {
	QString k = (*it).isEmpty() ? key : *it + "/" + key;
	handle = d->openKey( k, FALSE );
	e = d->entry( k );
	if ( handle )
	    break;
    }
    if ( !handle )
	return TRUE;
    if ( e == "Default" )
	e = "";
#ifdef _WIN32_WCE
	res = RegDeleteValueW( handle, (TCHAR*)qt_winTchar( e, TRUE ) );
#else
#if defined(UNICODE)
    if ( qWinVersion() & Qt::WV_NT_based )
	res = RegDeleteValueW( handle, (TCHAR*)qt_winTchar( e, TRUE ) );
    else
#endif
	res = RegDeleteValueA( handle, e.local8Bit() );
#endif

    if ( res != ERROR_SUCCESS && res != ERROR_FILE_NOT_FOUND ) {
#if defined(QT_CHECK_STATE)
	qSystemWarning( "Error deleting value " + key, res );
#endif
	return FALSE;
    }
    char vname[1];
    DWORD vnamesz = 1;
    FILETIME lastWrite;
#ifdef _WIN32_WCE
    LONG res2 = RegEnumValue( handle, 0, (LPTSTR)qt_winTchar(vname,TRUE), &vnamesz, NULL, NULL, NULL, NULL );
    LONG res3 = RegEnumKeyEx( handle, 0, (LPTSTR)qt_winTchar(vname,TRUE), &vnamesz, NULL, NULL, NULL, &lastWrite );
#else
    LONG res2 = RegEnumValueA( handle, 0, vname, &vnamesz, NULL, NULL, NULL, NULL );
    LONG res3 = RegEnumKeyExA( handle, 0, vname, &vnamesz, NULL, NULL, NULL, &lastWrite );
#endif
    if ( res2 == ERROR_NO_MORE_ITEMS && res3 == ERROR_NO_MORE_ITEMS )
#ifdef _WIN32_WCE
	RegDeleteKeyW( handle, L"" );
#else
	RegDeleteKeyA( handle, "" );
#endif
    else
	RegCloseKey( handle );
    return TRUE;
}

QStringList QSettings::entryList( const QString &key ) const
{
    QStringList result;
    
    HKEY handle = 0;
    for ( QStringList::Iterator it = d->paths.fromLast(); it != d->paths.end(); --it ) {
	QString k = (*it).isEmpty() ? key : *it + "/" + key + "/fake";
	handle = d->openKey( k, FALSE );
	if ( handle )
	    break;
    }
    if ( !handle )
	return result;

    DWORD count;
    DWORD maxlen;
#ifdef _WIN32_WCE
	RegQueryInfoKeyW( handle, NULL, NULL, NULL, NULL, NULL, NULL, &count, &maxlen, NULL, NULL, NULL );
#else
#if defined(UNICODE)
    if ( qWinVersion() & Qt::WV_NT_based )
	RegQueryInfoKeyW( handle, NULL, NULL, NULL, NULL, NULL, NULL, &count, &maxlen, NULL, NULL, NULL );
    else
#endif
	RegQueryInfoKeyA( handle, NULL, NULL, NULL, NULL, NULL, NULL, &count, &maxlen, NULL, NULL, NULL );
#endif

    if ( qWinVersion() & Qt::WV_NT_based )
	maxlen++;

    DWORD index = 0;

    TCHAR *vnameT = new TCHAR[ maxlen ];
    char *vnameA = new char[ maxlen ];
    QString qname;

    DWORD vnamesz = 0;
    LONG res = ERROR_SUCCESS;

    while ( res != ERROR_NO_MORE_ITEMS ) {
	vnamesz = maxlen;
#if defined(UNICODE)
#ifndef _WIN32_WCE
	if ( qWinVersion() & Qt::WV_NT_based ) {
#endif
	    res = RegEnumValueW( handle, index, vnameT, &vnamesz, NULL, NULL, NULL, NULL );
	    qname = qt_winQString( vnameT );
#ifndef _WIN32_WCE
	} else
#endif
#endif
#ifndef _WIN32_WCE
	{
	    res = RegEnumValueA( handle, index, vnameA, &vnamesz, NULL, NULL, NULL, NULL );
	    qname = vnameA;
	}
#endif
	if ( res == ERROR_NO_MORE_ITEMS )
	    break;
	if ( qname.isEmpty() )
	    result.append( "Default" );
	else
	    result.append( qname );
	++index;
    }

    delete [] vnameA;
    delete [] vnameT;

    RegCloseKey( handle );
    return result;
}

QStringList QSettings::subkeyList( const QString &key ) const
{
    QStringList result;
    
    HKEY handle = 0;
    for ( QStringList::Iterator it = d->paths.fromLast(); it != d->paths.end(); --it ) {
	QString k = (*it).isEmpty() ? key : *it + "/" + key + "/fake";
	handle = d->openKey( k, FALSE );
	if ( handle )
	    break;
    }
    if ( !handle )
	return result;

    DWORD count;
    DWORD maxlen;
#ifdef _WIN32_WCE
	RegQueryInfoKeyW( handle, NULL, NULL, NULL, &count, &maxlen, NULL, NULL, NULL, NULL, NULL, NULL );
#else
#if defined(UNICODE)
    if ( qWinVersion() & Qt::WV_NT_based )
	RegQueryInfoKeyW( handle, NULL, NULL, NULL, &count, &maxlen, NULL, NULL, NULL, NULL, NULL, NULL );
    else
#endif
	RegQueryInfoKeyA( handle, NULL, NULL, NULL, &count, &maxlen, NULL, NULL, NULL, NULL, NULL, NULL );
#endif

    if ( qWinVersion() & Qt::WV_NT_based )
	maxlen++;

    DWORD index = 0;
    FILETIME lastWrite;

    TCHAR *vnameT = new TCHAR[ maxlen ];
    char *vnameA = new char[ maxlen ];
    QString qname;

    DWORD vnamesz = 0;
    LONG res = ERROR_SUCCESS;

    while ( res != ERROR_NO_MORE_ITEMS ) {
	vnamesz = maxlen;
#if defined(UNICODE)
#ifndef _WIN32_WCE
	if ( qWinVersion() & Qt::WV_NT_based ) {
#endif
	    res = RegEnumKeyExW( handle, index, vnameT, &vnamesz, NULL, NULL, NULL, &lastWrite );
	    qname = qt_winQString( vnameT );
#ifndef _WIN32_WCE
	} else
#endif
#endif
#ifndef _WIN32_WCE
	{
	    res = RegEnumKeyExA( handle, index, vnameA, &vnamesz, NULL, NULL, NULL, &lastWrite );
	    qname = vnameA;
	}
#endif

	if ( res == ERROR_NO_MORE_ITEMS )
	    break;
	result.append( qname );
	++index;
    }

    delete [] vnameA;
    delete [] vnameT;

    RegCloseKey( handle );
    return result;
}

QDateTime QSettings::lastModficationTime(const QString &)
{
    return QDateTime();
}

void QSettings::insertSearchPath( System s, const QString &p )
{
    if ( s != Windows || p.isEmpty() )
	return;
    QString path = p;
    if ( path[0] != '/' )
	path = "/" + path;
    d->paths.append( path );
}

void QSettings::removeSearchPath( System s, const QString &p )
{
    if ( s != Windows || p.isEmpty() )
	return;
    QString path = p;
    if ( path[0] != '/' )
	path = "/" + path;
    d->paths.remove( path );
}
