#ifndef QDEFAULTPLUGIN_H
#define QDEFAULTPLUGIN_H

#include "qplugin.h"

class QDefaultPlugIn : public QPlugIn
{
public:
    QDefaultPlugIn( const QString& filename, LibraryPolicy = DefaultPolicy );

    bool load();

    QWidget* createWidget( const QString& classname, bool init, QWidget* parent = 0, const char* name = 0 );
    const char* enumerateWidgets();

    QWidget* processFile( QIODevice* f, const QString& filetype );
    const char* enumerateFileTypes();

    QAction* createAction( const QString &actionname, bool& self, QObject *parent );
    const char* enumerateActions();

    bool addToManager( QDict<QPlugIn>& dict );

private:
    typedef QWidget* (*CREATEWIDGETPROC)(const QString&, bool, QWidget* = 0, const char* = 0 );
    typedef QWidget* (*PROCESSFILEPROC)( QIODevice*, const QString& );
    typedef QAction* (*CREATEACTIONPROC)(const QString&, bool&, QObject* = 0 );

    CREATEWIDGETPROC createWidgetPtr;
    STRINGPROC enumerateWidgetsPtr;

    PROCESSFILEPROC processFilePtr;
    STRINGPROC enumerateFileTypesPtr;

    CREATEACTIONPROC createActionPtr;
    STRINGPROC enumerateActionsPtr;
};

class QDefaultPlugInManager : public QWidgetFactory, public QActionFactory, public QPlugInManager<QDefaultPlugIn>
{
public:
    QDefaultPlugInManager( const QString& path = QString::null, QPlugIn::LibraryPolicy = QPlugIn::DefaultPolicy );

private:
    QString factoryName() const { return "QPlugInManager"; }

    QWidget* newWidget( const QString& classname, bool init, QWidget* parent = 0, const char* name = 0 );
    QStringList enumerateWidgets();

    QWidget* processFile( QIODevice* file, const QString& filetype );
    QStringList enumerateFileTypes();    

    QAction* newAction( const QString& actionname, bool& self, QObject* parent = 0 );
    QStringList enumerateActions();
};

#endif // QDEFAULTPLUGIN_H