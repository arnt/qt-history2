#ifndef FILTERPLUGIN_H
#define FILTERPLUGIN_H

#include <qplugin.h>
#include <qpluginmanager.h>

#include "filteriface.h"

class FilterPlugIn : public FilterInterface, public QPlugIn
{
public:
    FilterPlugIn( const QString& filename, LibraryPolicy = Default, const char* fn = 0 );

    QCString queryPlugInInterface() const { return "FilterInterface"; }

    QStringList import( const QString& filter, const QString& filename );
};

class FilterPlugInManager : public QPlugInManager<FilterPlugIn>
{
public:
    FilterPlugInManager( const QString& path = QString::null, const QString& filter = "*.dll; *.so",
			 QPlugIn::LibraryPolicy = QPlugIn::Default, const char* fn = 0 );

    QStringList import( const QString& filter, const QString& filename );
};

#endif
