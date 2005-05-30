/*
  config.h
*/

#ifndef CONFIG_H
#define CONFIG_H

#include <QMap>
#include <QSet>
#include <QStringList>

#include "location.h"

class Config
{
public:
    Config( const QString& programName );
    ~Config();

    void load( const QString& fileName );
    void setStringList( const QString& var, const QStringList& values );

    const QString& programName() const { return prog; }
    const Location& location() const { return loc; }
    const Location& lastLocation() const { return lastLoc; }
    int getInt( const QString& var ) const;
    QString getString( const QString& var ) const;
    QSet<QString> getStringSet( const QString& var ) const;
    QStringList getStringList( const QString& var ) const;
    QRegExp getRegExp( const QString& var ) const;
    QList<QRegExp> getRegExpList( const QString& var ) const;
    QSet<QString> subVars( const QString& var ) const;
    QStringList getAllFiles( const QString& filesVar, const QString& dirsVar,
			     const QString& defaultNameFilter );

    static QStringList getFilesHere( const QString& dir,
                                     const QString& nameFilter );
    static QString findFile( const Location& location, const QStringList &files,
			     const QStringList& dirs, const QString& fileName,
			     QString& userFriendlyFilePath );
    static QString findFile( const Location &location, const QStringList &files,
			     const QStringList &dirs, const QString &fileBase,
			     const QStringList &fileExtensions,
			     QString &userFriendlyFilePath );
    static QString copyFile( const Location& location,
			     const QString& sourceFilePath,
			     const QString& userFriendlySourceFilePath,
			     const QString& targetDirPath );
    static int numParams( const QString& value );
    static bool removeDirContents( const QString& dir );

    QT_STATIC_CONST QString dot;

private:
    static bool isMetaKeyChar( QChar ch );
    void load( Location location, const QString& fileName );

    QString prog;
    Location loc;
    Location lastLoc;
    QMap<QString, Location> locMap;
    QMap<QString, QStringList> stringListValueMap;
    QMap<QString, QString> stringValueMap;

    static QMap<QString, QString> uncompressedFiles;
    static QMap<QString, QString> extractedDirs;
    static int numInstances;
};

#define CONFIG_ALIAS                    "alias"
#define CONFIG_BASE                     "base"      // ### don't document for now
#define CONFIG_DEFINES                  "defines"
#define CONFIG_EXAMPLEDIRS              "exampledirs"
#define CONFIG_EXAMPLES                 "examples"
#define CONFIG_EXTRAIMAGES              "extraimages"
#define CONFIG_FALSEHOODS               "falsehoods"
#define CONFIG_FORMATTING               "formatting"
#define CONFIG_HEADERDIRS               "headerdirs"
#define CONFIG_HEADERS                  "headers"
#define CONFIG_IGNOREDIRECTIVES         "ignoredirectives"
#define CONFIG_IGNORETOKENS             "ignoretokens"
#define CONFIG_IMAGEDIRS                "imagedirs"
#define CONFIG_IMAGES                   "images"
#define CONFIG_LANGUAGE                 "language"
#define CONFIG_MACRO                    "macro"
#define CONFIG_OUTPUTDIR                "outputdir"
#define CONFIG_OUTPUTFORMATS            "outputformats"
#define CONFIG_SOURCEDIRS               "sourcedirs"
#define CONFIG_SOURCES                  "sources"
#define CONFIG_SPURIOUS                 "spurious"
#define CONFIG_TABSIZE                  "tabsize"
#define CONFIG_TRANSLATORS              "translators" // ### don't document for now
#define CONFIG_VERSION                  "version"
#define CONFIG_VERSIONSYM               "versionsym"

#define CONFIG_FILEEXTENSIONS           "fileextensions"

#endif
