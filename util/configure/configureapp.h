#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>
#include <qtextstream.h>

class MakeItem;

class Configure
{
public:
    Configure( int& argc, char** argv );
    ~Configure();

    void parseCmdLine();
#if !defined(EVAL)
    void validateArgs();
#endif
    bool displayHelp();
    void autoDetection();
    void generateOutputVars();
#if !defined(EVAL)
    void generateCachefile();
    void displayConfig();
    void buildQmake();
#endif
    void generateMakefiles();
    void appendMakeItem(int inList, const QString &item);
#if !defined(EVAL)
    void generateConfigfiles();
#endif
    void showSummary();
    void findProjects( const QString& dirName );
    QString firstLicensePath();

#if !defined(EVAL)
    void readLicense();
#endif

    QString addDefine(QString def);

    enum ProjectType {
	App,
	Lib,
	Subdirs
    };

    ProjectType projectType( const QString& proFileName );
    bool isDone();
    bool isOk();
private:
    // Our variable dictionaries
    QMap<QString,QString> dictionary;
    QStringList licensedModules;
    QStringList allSqlDrivers;
    QStringList allConfigs;
    QStringList disabledModules;
    QStringList enabledModules;
    QStringList modules;
//    QStringList sqlDrivers;
    QStringList configCmdLine;
    QStringList qmakeConfig;
    QStringList qtConfig;

    QStringList qmakeSql;
    QStringList qmakeSqlPlugins;

    QStringList qmakeStyles;
    QStringList qmakeStylePlugins;

    QStringList qmakeFormatPlugins;

    QStringList qmakeVars;
    QStringList qmakeDefines;
    //  makeList[0] for qt and qtmain
    //  makeList[1] for subdirs and libs
    //  makeList[2] for the rest
    QList<MakeItem*> makeList[3];
    QStringList qmakeIncludes;
    QStringList qmakeLibs;

    QMap<QString,QString> licenseInfo;
    QString outputLine;

    QTextStream outStream;

    static bool findFile(const QString &fileName);
    static bool findFileInPaths(const QString &fileName, const QStringList &paths);
#if !defined(EVAL)
    void reloadCmdLine();
    void saveCmdLine();
#endif

};

class MakeItem
{
public:
    MakeItem( const QString &d, const QString &p, const QString &t, Configure::ProjectType qt )
	: directory( d ),
	  proFile( p ),
	  target( t ),
	  qmakeTemplate( qt )
    { }

    QString directory;
    QString proFile;
    QString target;
    Configure::ProjectType qmakeTemplate;
};

