#ifndef QPLUGIN_H
#define QPLUGIN_H

#include "qwidgetfactory.h"
#include "qactionfactory.h"
#include "qwindowdefs.h"

class QAction;

#ifndef _OS_WIN32_
#define LoadLibrary(name) dlopen(name)
#define FreeLibrary(handle) dlclose(handle)
#define GetProcAddress(handle, name) dlsym(handle, name)
#include <dlfnc.h>
#endif

class QPlugInManager : public QWidgetFactory, public QActionFactory
{
public:
    QPlugInManager();
    QPlugInManager( const QString& path );

    void addPlugInPath( const QString& path );
    bool addPlugIn( const QString& fullname );

private:
    QString factoryName() const { return "QPlugInManager"; }

    QWidget* newWidget( const QString& classname, QWidget* parent = 0, const char* name = 0, Qt::WFlags f = 0 );
    QStringList enumerateWidgets();

    QWidget* processFile( QFile* dev, bool &ok );
    QStringList enumerateFileTypes();    

    QAction* newAction( const QString& actionname, QObject* parent = 0 );
    QStringList enumerateActions();

    void init();
};

#endif // QPLUGIN_H

