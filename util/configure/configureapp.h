#include <qapplication.h>
#include <qprocess.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>

class ConfigureApp : public QApplication
{
	Q_OBJECT
public:
    ConfigureApp( int& argc, char** argv );

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

    bool isProjectLibrary( const QString& proFileName );
    bool isDone();
private:
    QProcess qmakeBuilder;
    QProcess qmake;

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

private slots:
    virtual void readQmakeBuilderOutput();
    virtual void readQmakeBuilderError();
    virtual void qmakeBuilt();
    virtual void readQmakeOutput();
    virtual void readQmakeError();
    virtual void qmakeDone();
};
