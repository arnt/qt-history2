#ifndef QGUID_DEFINED
#define QGUID_DEFINED

#include <qglobal.h>
#include <memory.h>

struct Q_EXPORT QGuid
{
    unsigned int   data1;
    unsigned short data2;
    unsigned short data3;
    unsigned char  data4[ 8 ];
};

#endif //QGUID_DEFINED

#ifndef QDECLSPEC_SELECTANY
#if (_MSC_VER >= 1100)
#define QDECLSPEC_SELECTANY  __declspec(selectany)
#else
#define QDECLSPEC_SELECTANY
#endif
#endif

#ifdef Q_GUID
#undef Q_GUID
#endif

#if defined(Q_INITGUID)
#define Q_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8 ) \
        Q_EXTERN_C const QGuid QDECLSPEC_SELECTANY name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#else
#define Q_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8 ) \
        Q_EXTERN_C const QGuid name;
#endif

#ifndef QGUIDDEF_H
#define QGUIDDEF_H

inline bool operator==( const QGuid &guid1, const QGuid &guid2 )
{
    return !memcmp( &guid1, &guid2, sizeof( QGuid ) );
}

inline bool operator!=( const QGuid &guid1, const QGuid &guid2 )
{
    return !( guid1 == guid2 );
}

#endif //QGUIDDEF_H
