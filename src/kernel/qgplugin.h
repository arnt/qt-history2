#ifndef QGPLUGIN_H
#define QGPLUGIN_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H

#ifndef Q_EXTERN_C
#ifdef __cplusplus
#define Q_EXTERN_C    extern "C"
#else
#define Q_EXTERN_C    extern
#endif
#endif

#if defined(QT_THREAD_SUPPORT)
#define QT_THREADED_BUILD 1
#else
#define QT_THREADED_BUILD 0
#endif

#if defined(QT_DEBUG)
#define QT_DEBUG_BUILD 1
#else
#define QT_DEBUG_BUILD 0
#endif

#ifndef Q_EXPORT_PLUGIN
#define Q_PLUGIN_INITIALIZE \
    { \
	if ( !qApp && theApp ) \
	    qt_ucm_initialize( theApp ); \
	if ( mt ) \
	    *mt = QT_THREADED_BUILD; \
	if ( debug ) \
	    *debug = QT_DEBUG_BUILD; \
	return QT_VERSION; \
    }
#define Q_PLUGIN_INSTANTIATE( IMPLEMENTATION )	\
    { \
	IMPLEMENTATION *i = new IMPLEMENTATION;	\
	return i->iface(); \
    }

#    ifdef Q_WS_WIN
#	ifdef Q_CC_BOR
#	    define Q_EXPORT_PLUGIN( PLUGIN ) \
		class QApplication;\
		extern Q_EXPORT QApplication *qApp; \
		extern Q_EXPORT void qt_ucm_initialize( QApplication *theApp ); \
		Q_EXTERN_C __declspec(dllexport) int __stdcall ucm_initialize( QApplication *theApp, bool *mt, bool *debug ) \
		    Q_PLUGIN_INITIALIZE \
		Q_EXTERN_C __declspec(dllexport) QUnknownInterface* __stdcall ucm_instantiate() \
		    Q_PLUGIN_INSTANTIATE( PLUGIN )
#	else
#	    define Q_EXPORT_PLUGIN( PLUGIN ) \
		class QApplication;\
		extern Q_EXPORT QApplication *qApp; \
		extern Q_EXPORT void qt_ucm_initialize( QApplication *theApp ); \
		Q_EXTERN_C __declspec(dllexport) int ucm_initialize( QApplication *theApp, bool *mt, bool *debug ) \
		    Q_PLUGIN_INITIALIZE \
		Q_EXTERN_C __declspec(dllexport) QUnknownInterface* ucm_instantiate() \
		    Q_PLUGIN_INSTANTIATE( PLUGIN )
#	endif
#    else
#	define Q_EXPORT_PLUGIN( PLUGIN ) \
	    class QApplication;\
	    extern Q_EXPORT QApplication *qApp; \
	    extern Q_EXPORT void qt_ucm_initialize( QApplication *theApp ); \
	    Q_EXTERN_C int ucm_initialize( QApplication *theApp, bool *mt, bool *debug ) \
		Q_PLUGIN_INITIALIZE \
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

private:
    QGPlugin();
    QUnknownInterface* _iface;
};



#endif // QGPLUGIN_H
