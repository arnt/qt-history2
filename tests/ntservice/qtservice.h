#ifndef QTSERVICE_H
#define QTSERVICE_H

#include <qstringlist.h>

class QtServicePrivate;

class QtService
{
public:
    QtService( const QString &name, bool canPause = FALSE, bool useGui = TRUE );
    virtual ~QtService();

    bool isInstalled() const;
    bool isRunning() const;
    bool isInteractive() const;
    bool canPause() const;

    virtual bool install();
    virtual bool uninstall();
    virtual bool start();

    int exec( int argc, char **argv );
    void stop();

    QString serviceName() const;
    QString filePath() const;

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

    virtual void reportEvent( const QString &, EventType type = Success, int ID = 0, uint category = 0, const QByteArray &data = QByteArray() );

private:
    QtServicePrivate *d;
};

#endif // QTSERVICE_H
