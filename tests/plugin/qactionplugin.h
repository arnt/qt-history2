#ifndef QACTIONPLUGIN_H
#define QACTIONPLUGIN_H

#include "qplugin.h"
#include "qactioninterface.h"
#include "qactionfactory.h"

class QActionPlugIn : public QActionInterface, public QPlugIn
{
public:
    QString queryInterface() { return "QActionInterface"; }

    QActionPlugIn( const QString& filename, LibraryPolicy = Default );

    QAction* create( const QString& classname, bool& self, QObject* parent = 0 );
    QStringList actions();
    
    bool addToManager( QPlugInDict& dict );
    bool removeFromManager( QPlugInDict& dict );
};

class QActionPlugInManager : public QPlugInManager<QActionPlugIn>, public QActionFactory
{
public:
    QActionPlugInManager( const QString& path = QString::null, QPlugIn::LibraryPolicy = QPlugIn::Default );

    QString factoryName() const { return "QActionPlugInManager"; }

private:
    QAction* newAction( const QString& classname, bool& self, QObject* parent = 0 );
    QStringList actions();
};
#endif // QACTIONPLUGIN_H
