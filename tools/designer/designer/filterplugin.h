#ifndef FILTERPLUGIN_H
#define FILTERPLUGIN_H

#include <qinterfacemanager.h>
#include "filteriface.h"

class FilterPlugInManager : public QInterfaceManager<FilterInterface>
{
public:
    FilterPlugInManager( const QString& path = QString::null, const QString& filter = "*.dll; *.so",
			 QApplicationInterface* = 0, QPlugIn::LibraryPolicy = QPlugIn::Default );

    QStringList import( const QString& filter, const QString& filename );
};

#endif
