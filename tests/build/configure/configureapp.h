#include <qapplication.h>
#include <qprocess.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>

class ConfigureApp : public QApplication
{
	Q_OBJECT;
public:
    ConfigureApp( int& argc, char** argv );

    void parseCmdLine();
    void buildModulesList();
    void buildSqlList();
    void validateArgs();
    bool displayHelp();
    void generateOutputVars();
    void generateCachefile();
    void displayConfig();
    void buildQmake();
    void generateMakefiles();
    void showSummary();
    void findProjects( const QString& dirName );
    void copyDefsFile();
private:
    QProcess qmakeBuilder;
    QProcess qmake;

    // Our variable dictionaries
    QMap<QString,QString> dictionary;
    QStringList allModules;
    QStringList allSqlDrivers;
    QStringList allConfigs;
    QStringList modules;
    QStringList sqlDrivers;
    QStringList configCmdLine;
    QStringList qmakeConfig;
    QStringList qmakeSql;
    QStringList qmakeVars;
    QStringList qmakeDefines;
    QStringList makeList;
    QStringList::Iterator makeListIterator;
    
    QString qtDir;

    QString outputLine;

private slots:
    virtual void readQmakeBuilderOutput();
    virtual void readQmakeBuilderError();
    virtual void qmakeBuilt();
    virtual void readQmakeOutput();
    virtual void readQmakeError();
    virtual void qmakeDone();
};
