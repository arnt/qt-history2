#ifndef QUUIDDEFS_H
#define QUUIDDEFS_H

#ifndef QT_H
#include <qglobal.h>
#endif // QT_H

#include <memory.h>
#if defined(Q_OS_WIN32)
#include <guiddef.h>
#endif

struct Q_EXPORT QUuid
{
    QUuid()
	: data1( 0 ), data2( 0 ), data3( 0 )
    {
	memset( &data4, 0, sizeof(data4) );
    }
    QUuid( uint l, ushort w1, ushort w2, uchar b1, uchar b2, uchar b3, uchar b4, uchar b5, uchar b6, uchar b7, uchar b8 )
	: data1( l ), data2( w1 ), data3( w2 )
    {
	data4[0] = b1;
	data4[1] = b2;
	data4[2] = b3;
	data4[3] = b4;
	data4[4] = b5;
	data4[5] = b6;
	data4[6] = b7;
	data4[7] = b8;
    }
    QUuid( const QUuid &uuid )
    {
	memcpy( this, &uuid, sizeof(QUuid) );
    }
    
    // Implementented in qcomponentinterface.cpp
    QUuid( const QString & );
    QString toString() const;

    QUuid operator=(const QUuid &orig )
    {
	QUuid uuid;
	memcpy( &uuid, &orig, sizeof(QUuid) );
	return uuid;
    }

    bool operator==( const QUuid &uuid ) const
    {
	return !memcmp( this, &uuid, sizeof( QUuid ) );
    }

    bool operator!=( const QUuid &uuid ) const
    {
	return !( *this == uuid );
    }

#if defined(Q_OS_WIN32)
    // On Windows we have a type GUID that is used by the platform API, so we 
    // provide convenience operators to cast from and to this type.
    QUuid( const GUID &guid )
    {
	memcpy( this, &guid, sizeof(GUID) );
    }

    QUuid operator=(const GUID &orig )
    {
	QUuid uuid;
	memcpy( &uuid, &orig, sizeof(QUuid) );
	return uuid;
    }

    operator GUID() const
    { 
	GUID guid = { data1, data2, data3, { data4[0], data4[1], data4[2], data4[3], data4[4], data4[5], data4[6], data4[7] } };
	return guid;
    }

    operator==( const GUID &guid ) const
    {
	return !memcmp( this, &guid, sizeof(QUuid) );
    }

    operator!=( const GUID &guid ) const
    {
	return !( *this == guid );
    }

#endif

    uint   data1;
    ushort data2;
    ushort data3;
    uchar  data4[ 8 ];
};

#endif //QUUIDDEF_H
