#ifndef QDEFAULTPLUGIN_H
#define QDEFAULTPLUGIN_H

#include "qplugin.h"

class QDefaultPlugIn : public QPlugIn
{
public:
    QDefaultPlugIn( const QString& filename, LibraryPolicy = DefaultPolicy );

    bool load();

    QWidget* create( const QString& classname, QWidget* parent = 0, const char* name = 0 );
    const char* widgets();
    QAction* create( const QString &actionname, bool& self, QObject *parent );
    const char* actions();

    bool addToManager( QPlugInDict& dict );
    bool removeFromManager( QPlugInDict& dict );

private:
    typedef QWidget* (*CreateWidgetProc)(const QString&, QWidget* = 0, const char* = 0 );
    typedef QAction* (*CreateActionProc)(const QString&, bool&, QObject* = 0 );

    CreateWidgetProc createWidgetPtr;
    StringProc widgetsPtr;

    CreateActionProc createActionPtr;
    StringProc actionsPtr;
};

class QDefaultPlugInManager : public QWidgetFactory, public QActionFactory, public QPlugInManager<QDefaultPlugIn>
{
public:
    QDefaultPlugInManager( const QString& path = QString::null, QPlugIn::LibraryPolicy = QPlugIn::DefaultPolicy );

private:
    QString factoryName() const { return "QPlugInManager"; }

    QWidget* newWidget( const QString& classname, QWidget* parent = 0, const char* name = 0 );
    QStringList widgets();

    QAction* newAction( const QString& actionname, bool& self, QObject* parent = 0 );
    QStringList actions();
};

#endif // QDEFAULTPLUGIN_H