#ifndef QCOMPONENTINTERFACE_H
#define QCOMPONENTINTERFACE_H

#ifndef QT_H
#include <qstring.h>
#include <memory.h>
#endif // QT_H

#ifndef QT_NO_COMPONENT

#ifndef interface
#define interface struct
#endif

struct Q_EXPORT QGuid
{
    unsigned int   data1;
    unsigned short data2;
    unsigned short data3;
    unsigned char  data4[ 8 ];
};

interface Q_EXPORT QUnknownInterface
{
    virtual QUnknownInterface* queryInterface( const QGuid& ) = 0;
    virtual unsigned long addRef() = 0;
    virtual unsigned long release() = 0;
};

interface Q_EXPORT QComponentInterface : public QUnknownInterface
{
    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual QString author() const = 0;
    virtual QString version() const = 0;
};

#ifndef Q_EXTERN_C
#ifdef __cplusplus
#define Q_EXTERN_C    extern "C"
#else
#define Q_EXTERN_C    extern
#endif
#endif

#ifndef Q_CREATE_INSTANCE
    #define Q_CREATE_INSTANCE( IMPLEMENTATION )		\
	QUnknownInterface *i = new IMPLEMENTATION;	\
	i->addRef();					\
	return i;
#endif

#ifndef Q_EXPORT_INTERFACE
    #ifdef Q_WS_WIN
	#define Q_EXPORT_INTERFACE( IMPLEMENTATION ) \
	    Q_EXTERN_C __declspec(dllexport) QUnknownInterface *qt_load_interface() { Q_CREATE_INSTANCE( IMPLEMENTATION ) }
    #else
	#define Q_EXPORT_INTERFACE( IMPLEMENTATION ) \
	    Q_EXTERN_C QUnknownInterface *qt_load_interface() { Q_CREATE_INSTANCE( IMPLEMENTATION ) }
    #endif
#endif

#if defined(Q_INITGUID)
#define Q_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8 ) \
        Q_EXTERN_C const QGuid name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#else
#define Q_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8 ) \
        Q_EXTERN_C const QGuid name;
#endif

inline bool operator==( const QGuid &guid1, const QGuid &guid2 )
{
    return !memcmp( &guid1, &guid2, sizeof( QGuid ) );
}

#ifdef Q_INIT_INTERFACES
// {1D8518CD-E8F5-4366-99E8-879FD7E482DE}
Q_GUID(IID_QUnknownInterface, 
0x1d8518cd, 0xe8f5, 0x4366, 0x99, 0xe8, 0x87, 0x9f, 0xd7, 0xe4, 0x82, 0xde);

// {5F3968A5-F451-45b1-96FB-061AD98F926E}
Q_GUID(IID_QComponentInterface, 
0x5f3968a5, 0xf451, 0x45b1, 0x96, 0xfb, 0x6, 0x1a, 0xd9, 0x8f, 0x92, 0x6e);
#endif

#endif //QT_NO_COMPONENT

#endif //QCOMPONENTINTERFACE_H
