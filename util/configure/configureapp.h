#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>

class Configure
{
public:
    Configure( int& argc, char** argv );

    void parseCmdLine();
    void buildModulesList();
    void validateArgs();
    bool displayHelp();
    void generateOutputVars();
    void generateCachefile();
    void displayConfig();
    void buildQmake();
    void generateMakefiles();
    void generateConfigfiles();
    void showSummary();
    void findProjects( const QString& dirName );

    void readLicense();

    enum ProjectType {
	App,
	Lib,
	Subdirs
    };

    ProjectType projectType( const QString& proFileName );
    bool isDone();
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

    QStringList qmakeSql;
    QStringList qmakeSqlPlugins;

    QStringList qmakeStyles;
    QStringList qmakeStylePlugins;

    QStringList qmakeFormatPlugins;

    QStringList qmakeVars;
    QStringList qmakeDefines;
    QStringList makeList;
    QStringList qmakeIncludes;
    QStringList qmakeLibs;
    QStringList::Iterator makeListIterator;
    
    QString qtDir;
    QMap<QString,QString> licenseInfo;
    QString outputLine;

    void reloadCmdLine();
    void saveCmdLine();

};
