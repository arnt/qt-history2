#ifndef QUUID_DEFINED
#define QUUID_DEFINED

#ifndef QT_H
#include <qglobal.h>
#include <memory.h>
#endif // QT_H

#ifndef Q_EXTERN_C
#ifdef __cplusplus
#define Q_EXTERN_C    extern "C"
#else
#define Q_EXTERN_C    extern
#endif
#endif

struct Q_EXPORT QUuid
{
    unsigned int   data1;
    unsigned short data2;
    unsigned short data3;
    unsigned char  data4[ 8 ];
};

#endif //QUUID_DEFINED

#ifndef QDECLSPEC_SELECTANY
#if (_MSC_VER >= 1100)
#define QDECLSPEC_SELECTANY  __declspec(selectany)
#else
#define QDECLSPEC_SELECTANY
#endif
#endif

#ifdef Q_UUID
#undef Q_UUID
#endif

#if defined(Q_UUIDIMPL)
#define Q_UUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8 ) \
        Q_EXTERN_C const QUuid QDECLSPEC_SELECTANY name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#else
#define Q_UUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8 ) \
        Q_EXTERN_C const QUuid name;
#endif

#ifndef QUUIDDEF_H
#define QUUIDDEF_H

inline bool operator==( const QUuid &uuid1, const QUuid &uuid2 )
{
    return !memcmp( &uuid1, &uuid2, sizeof( QUuid ) );
}

inline bool operator!=( const QUuid &uuid1, const QUuid &uuid2 )
{
    return !( uuid1 == uuid2 );
}

#endif //QUUIDDEF_H
