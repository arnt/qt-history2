#ifndef QCOMPONENTINTERFACE_H
#define QCOMPONENTINTERFACE_H

#ifndef QT_H
#include <qstringlist.h>
#include <quuiddefs.h>
#endif // QT_H

#ifndef QT_NO_COMPONENT

struct Q_EXPORT QUnknownInterface
{
    virtual QUnknownInterface* queryInterface( const QUuid& ) = 0;
    virtual ulong addRef() = 0;
    virtual ulong release() = 0;
};

struct Q_EXPORT QComponentInterface : public QUnknownInterface
{
    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual QString author() const = 0;
    virtual QString version() const = 0;
};

struct Q_EXPORT QLibraryInterface : public QUnknownInterface
{
    virtual bool init() = 0;
    virtual void cleanup() = 0;
    virtual bool canUnload() const = 0;
};

struct Q_EXPORT QFeatureListInterface : public QUnknownInterface
{
    virtual QStringList featureList() const = 0;
};

#ifndef Q_CREATE_INSTANCE
    #define Q_CREATE_INSTANCE( IMPLEMENTATION )		\
	IMPLEMENTATION *i = new IMPLEMENTATION;	\
	i->addRef();					\
	return i;
#endif

#ifndef Q_EXTERN_C
#ifdef __cplusplus
#define Q_EXTERN_C    extern "C"
#else
#define Q_EXTERN_C    extern
#endif
#endif

#ifndef Q_EXPORT_INTERFACE
    #ifdef Q_WS_WIN
	#define Q_EXPORT_INTERFACE() \
	    Q_EXTERN_C __declspec(dllexport) QUnknownInterface *qt_load_interface()
    #else
	#define Q_EXPORT_INTERFACE() \
	    Q_EXTERN_C QUnknownInterface *qt_load_interface()
    #endif
#endif

// {1D8518CD-E8F5-4366-99E8-879FD7E482DE}
#ifndef IID_QUnknownInterface 
#define IID_QUnknownInterface QUuid(0x1d8518cd, 0xe8f5, 0x4366, 0x99, 0xe8, 0x87, 0x9f, 0xd7, 0xe4, 0x82, 0xde)
#endif

// {5F3968A5-F451-45b1-96FB-061AD98F926E}
#ifndef IID_QComponentInterface 
#define IID_QComponentInterface QUuid(0x5f3968a5, 0xf451, 0x45b1, 0x96, 0xfb, 0x6, 0x1a, 0xd9, 0x8f, 0x92, 0x6e)
#endif

// {D16111D4-E1E7-4C47-8599-24483DAE2E07} 
#ifndef IID_QLibraryInterface
#define IID_QLibraryInterface QUuid( 0xd16111d4, 0xe1e7, 0x4c47, 0x85, 0x99, 0x24, 0x48, 0x3d, 0xae, 0x2e, 0x07)
#endif

// {3F8FDC44-3015-4f3e-B6D6-E4AAAABDEAAD}
#ifndef IID_QFeatureListInterface 
#define IID_QFeatureListInterface  QUuid(0x3f8fdc44, 0x3015, 0x4f3e, 0xb6, 0xd6, 0xe4, 0xaa, 0xaa, 0xbd, 0xea, 0xad)
#endif

#endif //QT_NO_COMPONENT

#endif //QCOMPONENTINTERFACE_H
