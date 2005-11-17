#ifndef TESTLUPDATE_H
#define TESTLUPDATE_H

#include <qobject.h>
#include <qstringlist.h>
#include <QtCore/QProcess>
class TestLUpdate : public QObject
{
Q_OBJECT

public:
    TestLUpdate();
    virtual ~TestLUpdate();

    void setWorkingDirectory(const QString &workDir);
    bool updateProFile( const QString &pathProFile);

private:
    QString     m_cmdLupdate;
    QString     m_workDir;
    QProcess	*childProc;
    QStringList env_list;

    bool	child_show;
    bool    qws_mode;
    bool	exit_ok;

private:
    bool runChild( bool showOutput, const QString &program, const QStringList &argList);
    void addMakeResult( const QString &result );
    QStringList make_result;
    void childHasData();

private slots:
    void childReady(int exitCode);
};

#endif // TESTLUPDATE_H
