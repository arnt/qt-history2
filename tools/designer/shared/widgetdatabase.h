/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef WIDGETDATABASE_H
#define WIDGETDATABASE_H

#include <qiconset.h>
#include <qstring.h>
#include <qnamespace.h>
#include "../interfaces/widgetinterface.h" // up here for GCC 2.7.* compatibility
#include <private/qpluginmanager_p.h>


extern QPluginManager<WidgetInterface> *widgetManager();

struct WidgetDatabaseRecord
{
    WidgetDatabaseRecord();
    ~WidgetDatabaseRecord();
    QString iconSet, name, group, toolTip, whatsThis, includeFile;
    uint isContainer : 1;
    uint isForm : 1;
    uint isCommon : 1;
    uint isPlugin : 1;
    QIconSet *icon;
    int nameCounter;
};

class WidgetDatabase : public Qt
{
public:
    WidgetDatabase();
    static void setupDataBase( int id );
    static void setupPlugins();

    static int count();
    static int startCustom();

    static QIconSet iconSet( int id );
    static QString className( int id );
    static QString group( int id );
    static QString toolTip( int id );
    static QString whatsThis( int id );
    static QString includeFile( int id );
    static bool isForm( int id );
    static bool isContainer( int id );
    static bool isCommon( int id );

    static int idFromClassName( const QString &name );
    static QString createWidgetName( int id );

    static WidgetDatabaseRecord *at( int index );
    static void insert( int index, WidgetDatabaseRecord *r );
    static void append( WidgetDatabaseRecord *r );

    static QString widgetGroup( const QString &g );
    static QString widgetGroup( int i );
    static int numWidgetGroups();
    static bool isGroupVisible( const QString &g );
    static bool isGroupEmpty( const QString &grp );

    static int addCustomWidget( WidgetDatabaseRecord *r );
    static bool isCustomWidget( int id );
    static bool isCustomPluginWidget( int id );

    static bool isWhatsThisLoaded();
    static void loadWhatsThis( const QString &docPath );

    static bool hasWidget( const QString &name );
    static void customWidgetClassNameChanged( const QString &oldName, const QString &newName );

};

#endif
