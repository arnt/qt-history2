/*
  config.h
*/

#ifndef CONFIG_H
#define CONFIG_H

#include <qmap.h>
#include <qstringlist.h>

#include "location.h"
#include "set.h"

class Config
{
public:
    Config();

    void load( const QString& fileName );
    void setStringList( const QString& var, const QStringList& values );

    const Location& location() const { return loc; }
    int getInt( const QString& var ) const;
    QString getString( const QString& var ) const;
    Set<QString> getStringSet( const QString& var ) const;
    QStringList getStringList( const QString& var ) const;
    Set<QString> subVars( const QString& var ) const;

    static QString dot( const QString& var, const QString& subVar );
    static QString findFile( const QStringList& paths,
			     const QString& fileName );
    static Set<QString> findAll( const Set<QString>& paths,
				 const QString& nameFilter );

private:
    void reset();
    void load( Location location, const QString& fileName );

    Location loc;
    QMap<QString, QStringList> map;
};

#define CONFIG_ALIAS                "alias"
#define CONFIG_MACRO		    "macro"
#define CONFIG_TABSIZE              "tabsize"
#define CONFIG_TRANSLATORS          "translators"

#endif
