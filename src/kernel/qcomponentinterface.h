#ifndef QCOMPONENTINTERFACE_H
#define QCOMPONENTINTERFACE_H

#ifndef QT_H
#include <qstring.h>
#include <memory.h>
#include <qguiddefs.h>
#endif // QT_H

#ifndef QT_NO_COMPONENT

#ifndef interface
#define interface struct
#endif

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

Q_EXTERN_C Q_EXPORT const QGuid IID_QUnknownInterface;
Q_EXTERN_C Q_EXPORT const QGuid IID_QComponentInterface;

#endif //QT_NO_COMPONENT

#endif //QCOMPONENTINTERFACE_H
