#ifndef FILTERPLUGIN_H
#define FILTERPLUGIN_H

#include <qplugin.h>
#include <qpluginmanager.h>

#include "filteriface.h"

class FilterPlugIn : public FilterInterface, public QPlugIn
{
public:
    FilterPlugIn( const QString& filename, QApplicationInterface* = 0, LibraryPolicy = Default );

    QString queryInterface() const { return "FilterInterface"; }

    QStringList import( const QString& filter, const QString& filename );
};

class FilterPlugInManager : public QPlugInManager<FilterPlugIn>
{
public:
    FilterPlugInManager( const QString& path = QString::null, const QString& filter = "*.dll; *.so",
			 QApplicationInterface* = 0, QPlugIn::LibraryPolicy = QPlugIn::Default );

    QStringList import( const QString& filter, const QString& filename );
};

#endif
