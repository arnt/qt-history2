#ifndef TESTCOMPILER_H
#define TESTCOMPILER_H

#include <qobject.h>
#include <qstringlist.h>

QT_DECLARE_CLASS(Q3Process)

#define COMPILE_ERROR "Compile error"
#define COMPILE_SUCCESS "Compile successfull"
#define COMPILE_NOT_AVAIL "Binary not available for testing"
#define SELF_TEST "self-test"

enum BuildType { Exe, Dll, Lib, Plain };

class TestCompiler : public QObject
{
Q_OBJECT

public:
    TestCompiler();
    virtual ~TestCompiler();

    void setBaseCommands( QString makeCmd, QString qmakeCmd, bool qwsMode );

    // builds a complete project, e.g. qmake, make clean, make and exists.
    bool buildProject( const QString &project, BuildType buildType, const QString &targetName, const QString &destPath, const QString &version );

    // executes a make clean in the specified workPath
    bool makeClean( const QString &workPath );
    // executes a make dist clean in the specified workPath
    bool makeDistClean( const QString &workPath );
    // executes a qmake on proName in the specified workDir, output goes to buildDir or workDir if it's null
    bool qmake( const QString &workDir, const QString &proName, const QString &buildDir = QString() );
    // executes a make in the specified workPath, with an optional target (eg. install)
    bool make( const QString &workPath, const QString &target = QString() );
    // executes a make clean and then deletes the makefile in workpath + deletes the executable
    // in destPath.
    bool cleanAll( const QString &workPath, const QString &destPath, const QString &exeName, const QString &exeExt );
    // checks if the executable exists in destDir
    bool exists( const QString &destDir, const QString &exeName, BuildType buildType, const QString &version );
    // removes the makefile
    bool removeMakefile( const QString &workPath );

private:
    QString     make_cmd;
    QString     qmake_cmd;

    Q3Process	*childProc;
    QStringList env_list;

    bool	child_show;
    bool        qws_mode;
    bool	exit_ok;

private:
    bool runChild( bool showOutput, QStringList argList, QStringList *envList );
    void addMakeResult( const QString &result );
    QStringList make_result;

private slots:
    void childReady();
    void childHasData();
};

#endif // TESTCOMPILER_H
