#ifndef QTSERVICE_H
#define QTSERVICE_H

#include <qstringlist.h>
#include <qt_windows.h>

class QtService
{
public:
    QtService( const QString &name, bool canPause = FALSE );
    virtual ~QtService();

    bool isInstalled() const;
    bool isRunning() const;

    virtual bool install();
    virtual bool uninstall();
    virtual bool start();

    void exec( int argc, char **argv );
    void stop();

    QString serviceName() const;
    QString filePath() const;

protected:
    enum EventType
    {
	Success = 0, Error, Warning, Information
    };

    virtual bool initialize();
    virtual int  run( int argc, char **argv ) = 0;
    virtual void quit() = 0;
    virtual void pause();
    virtual void resume();
    virtual void user( int code );

    virtual void reportEvent( const QString &, EventType type = Success, uint category = 0 );

private:
    void setStatus ( DWORD dwState );

    static void WINAPI serviceMain( DWORD dwArgc, TCHAR** lpszArgv );
    static void WINAPI handler( DWORD dwOpcode );

    QString servicename;
    QString filepath;

    SERVICE_STATUS status;
    SERVICE_STATUS_HANDLE serviceStatus;
};

#endif // QTSERVICE_H
