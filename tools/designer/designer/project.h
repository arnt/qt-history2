/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef PROJECT_H
#define PROJECT_H

#include <qfeatures.h>

#ifndef QT_NO_SQL
#include <qsqldatabase.h>
#endif

#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>
#include <qmap.h>

class FormWindow;
class QObjectList;
struct DesignerProject;
struct DesignerDatabase;

class Project
{
public:
    struct DatabaseConnection
    {
	DatabaseConnection( Project *p ) :
#ifndef QT_NO_SQL
	    connection( 0 ),
#endif
	    project( p ), loaded( FALSE ), iface( 0 ) {}
	~DatabaseConnection();
	
	QString name;
	QString driver, dbName, username, password, hostname;
	QStringList tables;
	QMap<QString, QStringList> fields;

	bool refreshCatalog();
	bool open();
	void close();
	DesignerDatabase *iFace();
    private:
#ifndef QT_NO_SQL
	QSqlDatabase *connection;
#endif
	Project *project;
	bool loaded;
	DesignerDatabase *iface;
    };


    Project( const QString &fn, const QString &pName = QString::null );
    ~Project();

    void setFileName( const QString &fn, bool doClear = TRUE );
    QString fileName() const;
    void setProjectName( const QString &n );
    QString projectName() const;

    void setDatabaseDescription( const QString &db );
    QString databaseDescription() const;

    void setDescription( const QString &s );
    QString description() const;

    void setLanguage( const QString &l );
    QString language() const;

    QStringList uiFiles() const;
    void addUiFile( const QString &f, FormWindow *fw );
    void removeUiFile( const QString &f, FormWindow *fw );
    void setUiFiles( const QStringList &lst );
    void formClosed( FormWindow *fw );

    bool isValid() const;

    bool hasFormWindow( FormWindow* fw ) const;
    void setFormWindow( const QString &f, FormWindow *fw );
    void setFormWindowFileName( FormWindow *fw, const QString &f );

    QString makeAbsolute( const QString &f );
    QString makeRelative( const QString &f );

    void save();

    QList<DatabaseConnection> databaseConnections() const;
    void setDatabaseConnections( const QList<DatabaseConnection> &lst );
    void addDatabaseConnection( DatabaseConnection *conn );
    Project::DatabaseConnection *databaseConnection( const QString &name );

    QStringList databaseConnectionList();
    QStringList databaseTableList( const QString &connection );
    QStringList databaseFieldList( const QString &connection, const QString &table );
    void saveConnections();
    void loadConnections();

    void openDatabase( const QString &connection );
    void closeDatabase( const QString &connection );

    QObjectList *formList() const;

    DesignerProject *iFace();

private:
    void parse();
    void clear();

private:
    QString filename;
    QStringList uifiles;
    QString proName;
    QStringList loadedForms;
    QString desc;
    QMap<FormWindow*, QString> formWindows;
    QString dbFile;
    QList<DatabaseConnection> dbConnections;
    QString lang;
    DesignerProject *iface;

};

#endif
