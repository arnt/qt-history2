#ifndef QGPLUGIN_H
#define QGPLUGIN_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H

#ifndef QT_NO_COMPONENT

#ifndef Q_EXTERN_C
#ifdef __cplusplus
#define Q_EXTERN_C    extern "C"
#else
#define Q_EXTERN_C    extern
#endif
#endif

#ifndef Q_EXPORT_PLUGIN
#define Q_PLUGIN_INSTANTIATE( IMPLEMENTATION )	\
	{ \
	    IMPLEMENTATION *i = new IMPLEMENTATION;	\
	    return i->iface(); \
	}

#if defined(QT_THREAD_SUPPORT)
#define QT_THREADED_BUILD 1
#else
#define QT_THREADED_BUILD 0
#endif

#define Q_PLUGIN_QUERY \
	{ \
	    if ( version ) \
		*version = QT_VERSION; \
	    if ( flags ) { \
		*flags = 1; \
		if ( QT_THREADED_BUILD ) \
		    *flags |= 2; \
	    } \
	    if ( key ) \
		*key = QT_BUILD_KEY; \
	    return 0; \
	}
#    ifdef Q_WS_WIN
#	ifdef Q_CC_BOR
#	    define Q_EXPORT_PLUGIN(PLUGIN) \
		Q_EXTERN_C __declspec(dllexport) int __stdcall qt_ucm_query( uint * version, uint* flags, const char** key ) \
		    Q_PLUGIN_QUERY \
		Q_EXTERN_C __declspec(dllexport) QUnknownInterface* __stdcall ucm_instantiate() \
		    Q_PLUGIN_INSTANTIATE( PLUGIN )
#	else
#	    define Q_EXPORT_PLUGIN(PLUGIN) \
		Q_EXTERN_C __declspec(dllexport) int qt_ucm_query( uint * version, uint* flags, const char** key ) \
		    Q_PLUGIN_QUERY \
		Q_EXTERN_C __declspec(dllexport) QUnknownInterface* ucm_instantiate() \
		    Q_PLUGIN_INSTANTIATE( PLUGIN )
#	endif
#    else
#	define Q_EXPORT_PLUGIN(PLUGIN) \
	    Q_EXTERN_C int qt_ucm_query( uint * version, uint* flags, const char** key ) \
	        Q_PLUGIN_QUERY \
	    Q_EXTERN_C QUnknownInterface* ucm_instantiate() \
	        Q_PLUGIN_INSTANTIATE( PLUGIN )
#    endif

#endif

struct QUnknownInterface;

class Q_EXPORT QGPlugin : public QObject
{
    Q_OBJECT
public:
    QGPlugin( QUnknownInterface *i );
    ~QGPlugin();

    QUnknownInterface* iface();
    void setIface( QUnknownInterface *iface );

private:
    QGPlugin();
    QUnknownInterface* _iface;
};

#endif // QT_NO_COMPONENT

#endif // QGPLUGIN_H
