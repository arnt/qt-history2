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
#if defined(QT_THREAD_SUPPORT)
#define QT_THREADED_BUILD 1
#define Q_PLUGIN_FLAGS_STRING "11"
#else
#define QT_THREADED_BUILD 0
#define Q_PLUGIN_FLAGS_STRING "01"
#endif

// this is duplicated at Q_UCM_VERIFICATION_DATA in qcom_p.h
#define Q_PLUGIN_VERIFICATION_DATA \
	const char *ucm_instance_verification_data =			\
            "pattern=UCM_INSTANCE_VERIFICATION_DATA\n"			\
            "version="QT_VERSION_STR"\n"				\
            "flags="Q_PLUGIN_FLAGS_STRING"\n"				\
	    "buildkey="QT_BUILD_KEY"\0";

#define Q_PLUGIN_INSTANTIATE( IMPLEMENTATION )	\
	{ \
	    IMPLEMENTATION *i = new IMPLEMENTATION;	\
	    return i->iface(); \
	} \
        Q_PLUGIN_VERIFICATION_DATA

#define Q_PLUGIN_QUERY \
	{ \
	    switch ( flag ) { \
	    case 1:  *(uint*)out = QT_VERSION; break;\
	    case 2:  *(uint*)out = 1; \
		if ( QT_THREADED_BUILD ) \
		    *(uint*)out |= 2;\
	    break; \
	    case 3:  *(const char**)out  = QT_BUILD_KEY; break;\
	    default: return 1; }\
	    return 0; \
	}
#    ifdef Q_WS_WIN
#	ifdef Q_CC_BOR
#	    define Q_EXPORT_PLUGIN(PLUGIN) \
		Q_EXTERN_C __declspec(dllexport) int __stdcall qt_ucm_query( int flag, void* out ) \
		    Q_PLUGIN_QUERY \
		Q_EXTERN_C __declspec(dllexport) QUnknownInterface* __stdcall ucm_instantiate() \
		    Q_PLUGIN_INSTANTIATE( PLUGIN )
#	else
#	    define Q_EXPORT_PLUGIN(PLUGIN) \
		Q_EXTERN_C __declspec(dllexport) int qt_ucm_query( int flag, void* out ) \
		    Q_PLUGIN_QUERY \
		Q_EXTERN_C __declspec(dllexport) QUnknownInterface* ucm_instantiate() \
		    Q_PLUGIN_INSTANTIATE( PLUGIN )
#	endif
#    else
#	define Q_EXPORT_PLUGIN(PLUGIN) \
	    Q_EXTERN_C int qt_ucm_query( int flag, void* out ) \
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
