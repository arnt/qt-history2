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

    static QString folder( const QString& );
    static QString entry( const QString& );

    bool writeKey( const QString &key, const QByteArray &value, ulong type );
    QByteArray readKey( const QString &key, ulong type, bool *ok );

    HKEY createFolder( const QString &folder );

    QStringList paths;
};

QSettingsPrivate::QSettingsPrivate()
{
    paths.append( "" );

    long res;
#if defined(UNICODE)
    if ( qWinVersion() & Qt::WV_NT_based )
	res = RegOpenKeyExW( HKEY_LOCAL_MACHINE, NULL, 0, KEY_ALL_ACCESS, &local );
    else
#endif
	res = RegOpenKeyExA( HKEY_LOCAL_MACHINE, NULL, 0, KEY_ALL_ACCESS, &local );

    if ( res != ERROR_SUCCESS ) {
#if defined(UNICODE)
	if ( qWinVersion() & Qt::WV_NT_based )
	    res = RegOpenKeyExW( HKEY_LOCAL_MACHINE, NULL, 0, KEY_READ, &local );
	else
#endif
	    res = RegOpenKeyExA( HKEY_LOCAL_MACHINE, NULL, 0, KEY_READ, &local );

	if ( res != ERROR_SUCCESS ) {
	    local = NULL;
	}
    }
#if defined(UNICODE)
    if ( qWinVersion() & Qt::WV_NT_based ) 
	res = RegOpenKeyExW( HKEY_CURRENT_USER, NULL, 0, KEY_ALL_ACCESS, &user );
    else
#endif
	res = RegOpenKeyExA( HKEY_CURRENT_USER, NULL, 0, KEY_ALL_ACCESS, &user );
	
    if ( res != ERROR_SUCCESS ) {
#if defined(UNICODE)
	if ( qWinVersion() & Qt::WV_NT_based ) 
	    res = RegOpenKeyExW( HKEY_CURRENT_USER, NULL, 0, KEY_READ, &user );
	else
#endif
	    res = RegOpenKeyExA( HKEY_CURRENT_USER, NULL, 0, KEY_READ, &user );
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

inline QString QSettingsPrivate::folder( const QString &key )
{
    return "Software\\" + key.mid( 1, key.findRev( "/" )-1 ).replace( QRegExp("/"), "\\" );
}

inline QString QSettingsPrivate::entry( const QString &key )
{
    return key.right( key.length() - key.findRev( "/" ) - 1 );
}

inline HKEY QSettingsPrivate::createFolder( const QString &f )
{
    HKEY handle = 0;
    long res;

    if ( local ) {
#if defined(UNICODE)
	if ( qWinVersion() & Qt::WV_NT_based )
	    res = RegCreateKeyExW( local, (TCHAR*)qt_winTchar( f, TRUE ), 0, (TCHAR*)qt_winTchar( "", TRUE ), REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &handle, NULL );
	else
#endif
	    res = RegCreateKeyExA( local, f.local8Bit(), 0, "", REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &handle, NULL );
#if defined(QT_CHECK_STATE)
	if ( res != ERROR_SUCCESS )
	    qSystemWarning("Couldn't open folder " + f + " for writing", res );
#endif
    }
    if ( !handle && user ) {
#if defined(UNICODE)
	if ( qWinVersion() & Qt::WV_NT_based )
	    res = RegCreateKeyExW( user, (TCHAR*)qt_winTchar( f, TRUE ), 0, (TCHAR*)qt_winTchar( "", TRUE ), REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &handle, NULL );
	else
#endif
	    res = RegCreateKeyExA( user, f.local8Bit(), 0, "", REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &handle, NULL );
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
	QString k = (*it).isEmpty() ? key : *it + "/" + key;
	e = entry( k );
	handle = createFolder( folder( k ) );
	if ( handle )
	    break;
    }
    if ( !handle )
	return FALSE;

#if defined(UNICODE)
    if ( qWinVersion() & Qt::WV_NT_based )
	res = RegSetValueExW( handle, (TCHAR*)qt_winTchar( e, TRUE ), 0, type, (const uchar*)value.data(), value.size() );
    else
#endif
	res = RegSetValueExA( handle, e.local8Bit(), 0, type, (const uchar*)value.data(), value.size() );

    if ( res != ERROR_SUCCESS ) {
#if defined(QT_CHECK_STATE)
	qSystemWarning( "Couldn't write value " + key, res );
#endif
	return FALSE;
    }

    RegCloseKey( handle );
    return TRUE;
}

inline QByteArray QSettingsPrivate::readKey( const QString &key, ulong t, bool *ok )
{
    HKEY handle = 0;
    long res;
    ulong size = 0;
    ulong type = 0;
    QString e;
    for ( QStringList::Iterator it = paths.fromLast(); it != paths.end(); --it ) {
	QString k;
	if ( (*it).isEmpty() )
	    k = key;
	else
	    k = *it + "/" + key;

	QString f = folder( k );
	e = entry( k );
	if ( user ) {
#if defined(UNICODE)
	    if ( qWinVersion() & Qt::WV_NT_based )
		res = RegOpenKeyExW( user, (TCHAR*)qt_winTchar( f, TRUE ), 0, KEY_READ, &handle );
	    else
#endif
		res = RegOpenKeyExA( user, f.local8Bit(), 0, KEY_READ, &handle );

	    if ( res == ERROR_SUCCESS ) {
#if defined(UNICODE)
		if ( qWinVersion() & Qt::WV_NT_based )
		    res = RegQueryValueExW( handle, (TCHAR*)qt_winTchar( e, TRUE ), NULL, &type, NULL, &size );
		else
#endif
		    res = RegQueryValueExA( handle, e.local8Bit(), NULL, &type, NULL, &size );

		if ( res != ERROR_SUCCESS && handle )
		    RegCloseKey( handle );
	    }
	}
	if ( size )
	    break;
    }

    if ( !size && local ) {
	for ( QStringList::Iterator it = paths.fromLast(); it != paths.end(); --it ) {
	    QString k;
	    if ( (*it).isEmpty() )
		k = key;
	    else
		k = *it + "/" + key;

	    QString f = folder( k );
	    e = entry( k );

#if defined(UNICODE)
	    if ( qWinVersion() & Qt::WV_NT_based )
		res = RegOpenKeyExW( local, (TCHAR*)qt_winTchar( f, TRUE ), 0, KEY_READ, &handle );
	    else
#endif
		res = RegOpenKeyExA( local, f, 0, KEY_READ, &handle );

	    if ( res == ERROR_SUCCESS ) {
#if defined(UNICODE)
		if ( qWinVersion() & Qt::WV_NT_based )
		    res = RegQueryValueExW( handle, (TCHAR*)qt_winTchar( e, TRUE ), NULL, &type, NULL, &size );
		else
#endif
		    res = RegQueryValueExA( handle, e.local8Bit(), NULL, &type, NULL, &size );
#if defined(QT_CHECK_STATE)
		if ( res != ERROR_SUCCESS )
		    qSystemWarning( "Couldn't read value " + key, res );
#endif
	    }
	    if ( size )
		break;
	}
    }

    if ( !size || type != t ) {
	if ( ok )
	    *ok = FALSE;
	return QByteArray();
    }

    uchar* data = new uchar[ size ];
#if defined(UNICODE)
    if ( qWinVersion() & Qt::WV_NT_based )
	RegQueryValueExW( handle, (TCHAR*)qt_winTchar( e, TRUE ), NULL, NULL, data, &size );
    else
#endif
	RegQueryValueExA( handle, e.local8Bit(), NULL, NULL, data, &size );

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
    if ( qWinVersion() & Qt::WV_NT_based ) {
	array.resize( value.length() * 2 + 2 );
	const QChar *data = value.unicode();
	int i;
	for ( i = 0; i < (int)value.length(); ++i ) {
	    array[ 2*i ] = data[ i ].cell();
	    array[ (2*i)+1 ] = data[ i ].row();
	}

	array[ (2*i) ] = 0;
	array[ (2*i)+1 ] = 0;
    } else
#endif
    {
	array.resize( value.length() );
	array = value.local8Bit();
    }

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
    if ( ok && *ok )
	return QStringList::split( sep, joined );
    else
	return QStringList();
}

QString QSettings::readEntry( const QString &key, const QString &def, bool *ok )
{
    if ( ok )
	*ok = FALSE;
    bool temp;
    QByteArray array = d->readKey( key, REG_SZ, &temp );
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
    if ( qWinVersion() & Qt::WV_NT_based ) {
	int s = array.size();
	for ( int i = 0; i < s; i+=2 ) {
	    QChar c( array[ i ], array[ i+1 ] );
	    if( !c.isNull() )
		result+=c;
	}
    } else
#endif
	result = QString::fromLocal8Bit( array );

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
    QByteArray array = d->readKey( key, REG_DWORD, &temp );
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
    QByteArray array = d->readKey( key, REG_BINARY, &temp );
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
    QString e = d->entry( key );
    long res;
    HKEY handle = d->createFolder( d->folder( key ) );
    if ( !handle )
	return FALSE;

#if defined(UNICODE)
    if ( qWinVersion() & Qt::WV_NT_based )
	res = RegDeleteValueW( handle, (TCHAR*)qt_winTchar( e, TRUE ) );
    else
#endif
	res = RegDeleteValueA( handle, e.local8Bit() );

    if ( res != ERROR_SUCCESS ) {
#if defined(QT_CHECK_STATE)
	qSystemWarning( "Error deleting value " + key, res );
#endif
	return FALSE;
    }
    return TRUE;
}

QDateTime QSettings::lastModficationTime(const QString &)
{
    return QDateTime();
}

void QSettings::insertSearchPath( System s, const QString &p )
{
    if ( s != Windows )
	return;
    d->paths.append( p );
}

void QSettings::removeSearchPath( System s, const QString &p )
{
    if ( s != Windows )
	return;
    if ( d->paths.count() > 1 )
	d->paths.remove( p );
}
