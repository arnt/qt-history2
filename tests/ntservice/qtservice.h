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

    QString serviceName() const;
    QString filePath() const;

protected:
    virtual bool initialize();
    virtual int  run( int argc, char **argv ) = 0;
    virtual void quit() = 0;
    virtual void pause();
    virtual void resume();
    virtual void user( int code );

private:
    void setStatus ( DWORD dwState );

    static void WINAPI serviceMainA( DWORD dwArgc, char** lpszArgv );
    static void WINAPI serviceMainW( DWORD dwArgc, TCHAR** lpszArgv );
    static void WINAPI handler( DWORD dwOpcode );

    QString servicename;
    QString filepath;

    SERVICE_STATUS status;
    SERVICE_STATUS_HANDLE serviceStatus;
};

#endif // QTSERVICE_H
