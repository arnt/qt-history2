#ifndef QDEFAULTPLUGIN_H
#define QDEFAULTPLUGIN_H

#include "qplugin.h"
#include <qiconset.h>
#include "qdefaultinterface.h"
#include "qwidgetfactory.h"

class QDialog;

class QDefaultPlugIn : public QPlugIn, public QDefaultInterface
{
public:
    QDefaultPlugIn( const QString& filename, LibraryPolicy = DefaultPolicy );

    QWidget* create( const QString& classname, QWidget* parent = 0, const char* name = 0 );
    QStringList widgets();
    
    bool addToManager( QPlugInDict& dict );
    bool removeFromManager( QPlugInDict& dict );
};

class QDefaultPlugInManager : public QPlugInManager<QDefaultPlugIn>, public QWidgetFactory
{
public:
    QDefaultPlugInManager( const QString& path = QString::null, QPlugIn::LibraryPolicy = QPlugIn::DefaultPolicy );

private:
    QString factoryName() const { return "QDefaultPlugInManager"; }

    QWidget* newWidget( const QString& classname, QWidget* parent = 0, const char* name = 0 );
    QStringList widgets();
};

#endif // QDEFAULTPLUGIN_H
