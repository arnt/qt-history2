#ifndef ACTIONPLUGIN_H
#define ACTIONPLUGIN_H

#include <qinterfacemanager.h>

#include "actioniface.h"

class ActionPlugInManager : public QInterfaceManager<ActionInterface>
{
public:
    ActionPlugInManager( const QString& path = QString::null, const QString& filter = "*.dll; *.so",
			 QApplicationInterface* = 0, QPlugIn::LibraryPolicy = QPlugIn::Default );

    QAction* create( const QString& actionname, QObject* parent = 0 );
    QString group( const QString &actionname );
};

#endif
