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
    QStringList getAllFiles( const QString& filesVar, const QString& dirsVar,
			     const QString& nameFilter );
    QString findFile( const QString& filesVar, const QString& dirsVar,
		      const QString& fileName );

    static QString dot( const QString& var, const QString& subVar );

private:
    void reset();
    void load( Location location, const QString& fileName );
    QStringList findHere( const QString& dir, const QString& nameFilter );

    Location loc;
    QMap<QString, QStringList> map;
};

#define CONFIG_ALIAS                "alias"
#define CONFIG_BASE                 "base"
#define CONFIG_DEFINES              "defines"
#define CONFIG_DOCDIRS              "docdirs"
#define CONFIG_FORMATS              "formats"
#define CONFIG_DOCS                 "docs"
#define CONFIG_EXAMPLEDIRS          "exampledirs"
#define CONFIG_EXAMPLES             "examples"
#define CONFIG_FALSEHOODS           "falsehoods"
#define CONFIG_IMAGEDIRS            "imagedirs"
#define CONFIG_IMAGES               "images"
#define CONFIG_INCLUDEDIRS          "includedirs"
#define CONFIG_INCLUDES             "includes"
#define CONFIG_MACRO                "macro"
#define CONFIG_OUTPUTDIR            "outputdir"
#define CONFIG_SOURCEDIRS           "sourcedirs"
#define CONFIG_SOURCELANGUAGE       "sourcelanguage"
#define CONFIG_SOURCES              "sources"
#define CONFIG_TABSIZE              "tabsize"
#define CONFIG_TARGETLANGUAGE       "targetlanguage"
#define CONFIG_TRANSLATORS          "translators"
#define CONFIG_VERSION              "version"
#define CONFIG_VERSIONSYM           "versionsym"

#endif
