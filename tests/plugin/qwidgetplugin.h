#ifndef QWIDGETPLUGIN_H
#define QWIDGETPLUGIN_H

#include "qplugin.h"
#include "qpluginmanager.h"
#include "qwidgetinterface.h"
#include "qwidgetfactory.h"

class QDialog;

class QWidgetPlugIn : public QWidgetInterface, public QPlugIn
{
public:
    QString queryInterface() { return "QWidgetInterface"; }

    QWidgetPlugIn( const QString& filename, LibraryPolicy = Default );

    QWidget* create( const QString& classname, QWidget* parent = 0, const char* name = 0 );
    QStringList widgets();
};

class QWidgetPlugInManager : public QPlugInManager<QWidgetPlugIn>, public QWidgetFactory
{
public:
    QWidgetPlugInManager( const QString& path = QString::null, QPlugIn::LibraryPolicy = QPlugIn::Default );

protected:
    bool addPlugIn( QPlugIn* );
    bool removePlugIn( QPlugIn* );

private:
    QWidget* newWidget( const QString& classname, QWidget* parent = 0, const char* name = 0 );
    QStringList widgets();
};

#endif // QWIDGETPLUGIN_H
