#ifndef QACTIONPLUGIN_H
#define QACTIONPLUGIN_H

#include <qplugin.h>
#include <qpluginmanager.h>

#include "qactioninterface.h"
#include "qactionfactory.h"

class ActionPlugIn : public ActionInterface, public QPlugIn
{
public:
    QActionPlugIn( const QString& filename, LibraryPolicy = Default, const char* fn = 0 );

    QString queryInterface() const { return "ActionInterface"; }

    QAction* create( const QString& classname, QObject* parent = 0 );
};

class ActionPlugInManager : public QPlugInManager<ActionPlugIn>
{
public:
    ActionPlugInManager( const QString& path = QString::null, const QString& filter = "*.dll; *.so", 
	QPlugIn::LibraryPolicy = QPlugIn::Default, const char* fn = 0 );

    public 
private:
    QAction* newAction( const QString& classname, QObject* parent = 0 );
    QStringList actions();
};

#endif // QACTIONPLUGIN_H
