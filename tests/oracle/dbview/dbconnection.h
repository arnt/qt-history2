#ifndef DBCONNECTION_H
#define DBCONNECTION_H

#include "qpixmap.h"
#include "qstringlist.h"
#include "qmap.h"

class QSqlDatabase;
class QListViewItem;

class DBConnection
{
public:
    DBConnection();
    ~DBConnection();

private:
    QSqlDatabase* db;
    QStringList userList;
    QStringList tableSpaceList;
    QMap< QString, QStringList > tableMap;
    QMap< QString, QStringList > viewMap;
    QMap< QString, QStringList > indexMap;

public:
    QSqlDatabase* database();

    QStringList& users();
    QStringList& tableSpaces();
    QStringList& tables( QString userName );
    QStringList& views( QString userName );
    QStringList& indexes( QString userName );
};

class ObjectInfo
{
public:
    typedef enum {
	OBJTYPE_UNKNOWN,
	OBJTYPE_TABLE,
	OBJTYPE_INDEX,
	OBJTYPE_TABLESPACE,
	OBJTYPE_VIEW,
	OBJTYPE_USER
    } ObjectType;

    ObjectInfo( QString name = QString::null, ObjectType objectType = OBJTYPE_UNKNOWN, QString owner = QString::null )
    {
	this->name = name;
	this->owner = owner;
	this->objectType = objectType;
    }
    QString name;
    QString owner;
    ObjectType objectType;
};

#endif
