#ifndef QWIDGETPLUGIN_H
#define QWIDGETPLUGIN_H

#include <qplugin.h>
#include <qpluginmanager.h>

#include "widgetinterface.h"

class WidgetPlugIn : public WidgetInterface, public QPlugIn
{
public:
    WidgetPlugIn( const QString& filename, LibraryPolicy = Default, const char* fn = 0 );

    QCString queryPlugInInterface() const { return "WidgetInterface"; }

    QWidget* create( const QString& classname, QWidget* parent = 0, const char* name = 0 );
    QString group( const QString& );
    QString iconSet( const QString& );
    QString includeFile( const QString& );
    QString toolTip( const QString& );
    QString whatsThis( const QString& );
    bool isContainer( const QString& );
};

class WidgetPlugInManager : public QPlugInManager<WidgetPlugIn>
{
public:
    WidgetPlugInManager( const QString& path = QString::null, const QString& filter = "*.dll; *.so", 
	QPlugIn::LibraryPolicy = QPlugIn::Default, const char* fn = 0 );

    QWidget* create( const QString& classname, QWidget* parent = 0, const char* name = 0 );
    QString group( const QString& );
    QString iconSet( const QString& );
    QString includeFile( const QString& );
    QString toolTip( const QString& );
    QString whatsThis( const QString& );
    bool isContainer( const QString& );
};

#endif // QWIDGETPLUGIN_H
