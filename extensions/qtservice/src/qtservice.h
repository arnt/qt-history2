#ifndef QTSERVICE_H
#define QTSERVICE_H

#include <qstringlist.h>

class QtServicePrivate;

#if defined(QT_DLL)
#  if defined(QNTS_MAKEDLL)
#    define QNTS_EXPORT __declspec(dllexport)
#  else
#    define QNTS_EXPORT __declspec(dllimport)
#  endif
#else
#  define QNTS_EXPORT
#endif

class QNTS_EXPORT QtService
{
public:
    enum EventType
    {
	Success = 0, Error, Warning, Information
    };

    QtService( const QString &name, bool canPause = FALSE, bool interactive = TRUE );
    virtual ~QtService();

    bool isInstalled() const;
    bool isRunning() const;
    bool isInteractive() const;
    bool canPause() const;

    virtual bool install();
    virtual bool uninstall();
    virtual bool start();
    virtual void reportEvent( const QString &, EventType type = Success, int ID = 0, uint category = 0, const QByteArray &data = QByteArray() );

    int tryStart( int argc, char **argv );
    void tryStop();

    QString serviceName() const;
    QString filePath() const;


protected:
    virtual bool initialize();
    virtual int  run( int argc, char **argv ) = 0;
    virtual void stop() = 0;
    virtual void pause();
    virtual void resume();
    virtual void user( int code );

private:
    friend class QtServicePrivate;

    QtServicePrivate *d;
};

extern QNTS_EXPORT QtService *qService;

#endif // QTSERVICE_H
