/*
  config.h
*/

#ifndef CONFIG_H
#define CONFIG_H

#include <qmap.h>
#include <qpair.h>
#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>

#include "stringset.h"

class Location;

class Config
{
public:
    Config( int argc, char **argv );

    void setVersion( const QString& version );

    QString verbatimHref( const QString& sourceFileName ) const;
    QString classRefHref( const QString& className ) const;
    QString classMembersHref( const QString& className ) const;
    QString classImageHref( const QString& className ) const;
    QString defgroupHref( const QString& groupName ) const;
    QString findDepth( const QString& name, const QStringList& dirList ) const;
    QStringList findAll( const QString& nameFilter,
			 const QStringList& dirList ) const;

    const QStringList& sourceDirList() const { return sourcedirs; }
    const QStringList& docDirList() const { return docdirs; }
    const QStringList& includeDirList() const { return includedirs; }
    const QStringList& exampleDirList() const { return exampledirs; }
    const QStringList& bookDirList() const { return bookdirs; }
    const QStringList& imageDirList() const { return imagedirs; }
    const QString& outputDir() const { return outputdir; }

    const QString& base() const { return bas; }
    const QString& product() const { return prod; }
    const QString& company() const { return co; }
    const QString& version() const { return vers; }
    const QString& versionSymbol() const { return verssym; }
    const QString& postHeader() const { return posth; }
    const QString& footer() const { return foot; }
    const QString& address() const { return addr; }
    const QString& style() const { return styl; }
    bool isTrue( const QString& condition ) const;
    bool isDef( const QString& symbol ) const;
    bool generateFile( const QString& fileName ) const;
    bool serialComma() const { return FALSE; }
    bool isInternal() const { return internal; }
    bool autoHrefs() const { return autoh; }
    bool supervisor() const { return super; }
    bool lint() const { return lin; }
    bool isFriendly() const { return frend; }

    bool needImage( const Location& loc, const QString& fileName,
		    int *width = 0, int *height = 0 );
    QString unalias( const Location& loc, const QString& alias,
		     const QString& format, const QStringList& args ) const;

private:
    bool matchLine( QString *key, QStringList *val );
    void showHelp();
    void showHelpShort();
    void showVersion();

    QString yyIn;
    int yyPos;

    QStringList sourcedirs;
    QStringList docdirs;
    QStringList includedirs;
    QStringList exampledirs;
    QStringList bookdirs;
    QStringList imagedirs;
    QString outputdir;

    int maxSim;
    int maxAll;
    int wlevel;
    QString bas;
    QString prod;
    QString co;
    QString vers;
    QString verssym;
    QString posth;
    QString foot;
    QString addr;
    QString styl;
    QRegExp falsesym;
    QRegExp defsym;
    QRegExp onlyfn;
    bool internal;
    bool autoh;
    bool super;
    bool lin;
    bool frend;

    QMap<QString, QPair<int, int> > imagesCopied;
    QMap<QString, QString> aliasMap;

    QString dotHtml;
    QString membersDotHtml;
    QString dotPng;
};

extern Config *config;

#endif
