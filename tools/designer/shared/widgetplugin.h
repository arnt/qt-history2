#ifndef QWIDGETPLUGIN_H
#define QWIDGETPLUGIN_H

#include <qinterfacemanager.h>
#include "widgetinterface.h"

class WidgetPlugInManager : public QInterfaceManager<WidgetInterface>
{
public:
    WidgetPlugInManager( const QString& path = QString::null, const QString& filter = "*.dll; *.so", 
	QApplicationInterface* = 0, QPlugIn::LibraryPolicy = QPlugIn::Default );

    QWidget* create( const QString& classname, QWidget* parent = 0, const char* name = 0 );
    QString group( const QString& );
    QString iconSet( const QString& );
    QIconSet iconset( const QString& );
    QString includeFile( const QString& );
    QString toolTip( const QString& );
    QString whatsThis( const QString& );
    bool isContainer( const QString& );
};

#endif // QWIDGETPLUGIN_H
