#ifndef QUUID_DEFINED
#define QUUID_DEFINED

#ifndef QT_H
#include <qglobal.h>
#include <memory.h>
#endif // QT_H

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

    uint   data1;
    ushort data2;
    ushort data3;
    uchar  data4[ 8 ];
};

#endif //QUUIDDEF_H
