#ifndef QWIDGETPLUGIN_H
#define QWIDGETPLUGIN_H

#include "qplugin.h"
#include <qiconset.h>
#include "qwidgetinterface.h"
#include "qwidgetfactory.h"

class QDialog;

class QWidgetPlugIn : public QPlugIn, public QWidgetInterface
{
public:
    QWidgetPlugIn( const QString& filename, LibraryPolicy = Default );

    QWidget* create( const QString& classname, QWidget* parent = 0, const char* name = 0 );
    QStringList widgets();
    
    bool addToManager( QPlugInDict& dict );
    bool removeFromManager( QPlugInDict& dict );
};

class QWidgetPlugInManager : public QPlugInManager<QWidgetPlugIn>, public QWidgetFactory
{
public:
    QWidgetPlugInManager( const QString& path = QString::null, QPlugIn::LibraryPolicy = QPlugIn::Default );

private:
    QString factoryName() const { return "QWidgetPlugInManager"; }

    QWidget* newWidget( const QString& classname, QWidget* parent = 0, const char* name = 0 );
    QStringList widgets();
};

#endif // QWIDGETPLUGIN_H
