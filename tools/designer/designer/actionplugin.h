#ifndef ACTIONPLUGIN_H
#define ACTIONPLUGIN_H

#include <qplugin.h>
#include <qpluginmanager.h>

#include "actioniface.h"

class ActionPlugIn : public ActionInterface, public QPlugIn
{
public:
    ActionPlugIn( const QString& filename, QApplicationInterface* = 0, LibraryPolicy = Default );

    QString queryInterface() const { return "ActionInterface"; }

    QAction* create( const QString& actionname, QObject* parent = 0 );
    QString group( const QString &actionname );
};

class ActionPlugInManager : public QPlugInManager<ActionPlugIn>
{
public:
    ActionPlugInManager( const QString& path = QString::null, const QString& filter = "*.dll; *.so",
			 QApplicationInterface* = 0, QPlugIn::LibraryPolicy = QPlugIn::Default );
    QAction* create( const QString& actionname, QObject* parent = 0 );
    QString group( const QString &actionname );

};

#endif
