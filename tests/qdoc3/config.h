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
    Config( const QString& programName );

    void load( const QString& fileName );
    void setStringList( const QString& var, const QStringList& values );

    const QString& programName() const { return prog; }
    const Location& location() const { return loc; }
    Location location( const QString& var ) const;
    int getInt( const QString& var ) const;
    QString getString( const QString& var ) const;
    Set<QString> getStringSet( const QString& var ) const;
    QStringList getStringList( const QString& var ) const;
    QRegExp getRegExp( const QString& var ) const;
    QValueList<QRegExp> getRegExpList( const QString& var ) const;
    Set<QString> subVars( const QString& var ) const;
    QStringList getAllFiles( const QString& filesVar, const QString& dirsVar,
			     const QString& nameFilter );

    static QString findFile( const QStringList& files,
			     const QStringList& dirsVar,
			     const QString& fileName );

    QT_STATIC_CONST QString dot;

private:
    void reset();
    void load( Location location, const QString& fileName );
    QStringList getFilesHere( const QString& dir, const QString& nameFilter );

    QString prog;
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
#define CONFIG_HEADERDIRS           "headerdirs"
#define CONFIG_HEADERS              "headers"
#define CONFIG_IMAGEDIRS            "imagedirs"
#define CONFIG_IMAGES               "images"
#define CONFIG_LANGUAGE             "language"
#define CONFIG_MACRO                "macro"
#define CONFIG_OUTPUTDIR            "outputdir"
#define CONFIG_SOURCEDIRS           "sourcedirs"
#define CONFIG_SOURCES              "sources"
#define CONFIG_SPURIOUS             "spurious"
#define CONFIG_TABSIZE              "tabsize"
#define CONFIG_TRANSLATORS          "translators"
#define CONFIG_VERSION              "version"
#define CONFIG_VERSIONSYM           "versionsym"

#endif
