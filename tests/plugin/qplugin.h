#ifndef QPLUGIN_H
#define QPLUGIN_H

#include "qstringlist.h"
#include "qdict.h"
#include "qwindowdefs.h"

class QAction;

#ifndef _OS_WIN32_
#define LoadLibrary(name) dlopen(name)
#define FreeLibrary(handle) dlclose(handle)
#define GetProcAddress(handle, name) dlsym(handle, name)
#include <dlfnc.h>
#endif

class QPlugIn
{
    friend class QPlugInManager;

public:
    QPlugIn( HINSTANCE pHnd );
    ~QPlugIn();

private:
    HINSTANCE pHnd;

    typedef QWidget* (*CREATEWIDGETPROC)(const QString&, QWidget* = 0, const char* = 0, Qt::WFlags = 0 );
    typedef const char* (*ENUMERATEWIDGETSPROC)();

    typedef QAction* (*CREATEACTIONPROC)(const QString&, QObject* = 0 );
    typedef const char* (*ENUMERATEACTIONSPROC)();

    CREATEWIDGETPROC createWidget;
    ENUMERATEWIDGETSPROC enumerateWidgets;

    CREATEACTIONPROC createAction;
    ENUMERATEACTIONSPROC enumerateActions;
};

class QPlugInManager : public Qt
{
public:
    QPlugInManager();
    QPlugInManager( const QString& path );

    void addPlugInPath( const QString& path );
    bool addPlugIn( const QString& fullname );

    QWidget* createWidget( const QString& classname, QWidget* parent = 0, const char* name = 0, WFlags f = 0 );
    QStringList enumerateWidgets();
    QAction* createAction( const QString& actionname, QObject* parent = 0 );
    QStringList enumerateActions();

private:
    void init();

    QDict<QPlugIn> pHnds;
    QDict<QPlugIn> pLibs;
};

#endif // QPLUGIN_H

