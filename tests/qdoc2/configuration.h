/*
  configuration.h
*/

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>

class Configuration
{
public:
    Configuration( int argc, char **argv );

    void setVersion( const QString& version );

    QString verbatimHref( const QString& sourceFileName ) const;
    QString classRefHref( const QString& className ) const;
    QString classMembersHref( const QString& className ) const;
    QString defgroupHref( const QString& groupName ) const;
    QString findDepth( const QString& name, const QStringList& dirList ) const;

    const QStringList& sourceDirList() const { return sourcedirs; }
    const QStringList& docDirList() const { return docdirs; }
    const QStringList& includeDirList() const { return includedirs; }
    const QStringList& exampleDirList() const { return exampledirs; }
    const QString& outputDir() const { return outputdir; }

    const QString& base() const { return bas; }
    const QString& moduleShort() const { return modshort; }
    const QString& moduleLong() const { return modlong; }
    const QString& company() const { return co; }
    const QString& version() const { return vers; }
    const QString& versionSymbol() const { return verssym; }
    const QString& postHeader() const { return posth; }
    const QString& footer() const { return foot; }
    const QString& address() const { return addr; }
    const QString& style() const { return styl; }
    bool isTrue( const QString& condition ) const;
    bool isDef( const QString& symbol ) const;
    bool processClass( const QString& className ) const;
    bool serialComma() const { return serial; }
    bool isInternal() const { return internal; }
    bool readExampleHeaders() const { return readh; }
    bool autoHrefs() const { return autoh; }
    bool supervisor() const { return super; }

private:
#if defined(Q_DISABLE_COPY)
    Configuration( const Configuration& );
    Configuration& operator=( const Configuration& );
#endif

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
    QString outputdir;

    int maxSame;
    int maxAll;
    int wlevel;
    QString bas;
    QString modshort;
    QString modlong;
    QString co;
    QString vers;
    QString verssym;
    QString posth;
    QString foot;
    QString addr;
    QString styl;
    QRegExp falsesym;
    QRegExp defsym;
    QRegExp onlysym;
    bool serial;
    bool internal;
    bool readh;
    bool autoh;
    bool super;

    QString dotHtml;
    QString membersDotHtml;
};

extern Configuration *config;

#endif
