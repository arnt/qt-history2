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

#include <qsqldatabase.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>
#include <qmap.h>

class FormWindow;

class Project
{
public:
    struct DatabaseConnection
    {
	DatabaseConnection( Project *p ) : connection( 0 ), project( p ), loaded( FALSE ) {}
	QString name;
	QString driver, dbName, username, password, hostname;
	QSqlDatabase *connection;
	Project *project;
	bool loaded;

	QStringList tables;
	QMap<QString, QStringList> fields;

	bool connect( bool keepOpen = FALSE );
	bool sync( bool keepOpen = FALSE );
	void close();
    };

    Project( const QString &fn, const QString &pName = QString::null );

    void setFileName( const QString &fn, bool doClear = TRUE );
    QString fileName() const;
    void setProjectName( const QString &n );
    QString projectName() const;

    void setDatabaseDescription( const QString &db );
    QString databaseDescription() const;

    void setDescription( const QString &s );
    QString description() const;

    QStringList uiFiles() const;
    void addUiFile( const QString &f, FormWindow *fw );
    void removeUiFile( const QString &f, FormWindow *fw );
    void setUiFiles( const QStringList &lst );
    void formClosed( FormWindow *fw );

    bool isValid() const;

    void setFormWindow( const QString &f, FormWindow *fw );
    void setFormWindowFileName( FormWindow *fw, const QString &f );

    QString makeAbsolute( const QString &f );
    QString makeRelative( const QString &f );

    void save();

    QList<Project::DatabaseConnection> databaseConnections() const;
    void setDatabaseConnections( const QList<Project::DatabaseConnection> &lst );
    void addDatabaseConnection( Project::DatabaseConnection *conn );
    Project::DatabaseConnection *databaseConnection( const QString &name );

    QStringList databaseConnectionList();
    QStringList databaseTableList( const QString &connection );
    QStringList databaseFieldList( const QString &connection, const QString &table );
    void saveConnections();
    void loadConnections();

    void openDatabase( const QString &connection );
    void closeDatabase( const QString &connection );

    void connectTables( QObject *toplevel, const QMap<QString, QStringList> &tables );
    
private:
    void parse();
    void clear();

private:
    QString filename;
    QStringList uifiles;
    QString dbFile;
    QString proName;
    QStringList loadedForms;
    QString desc;
    QMap<FormWindow*, QString> formWindows;
    QList<Project::DatabaseConnection> dbConnections;

};

#endif
