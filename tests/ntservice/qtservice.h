#ifndef QTSERVICE_H
#define QTSERVICE_H

#include <qstringlist.h>

class QtServicePrivate;

class QtService
{
    friend class QtServicePrivate;
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
    QtServicePrivate *d;
};

#endif // QTSERVICE_H
