#ifndef QASSISTANTCLIENT_H
#define QASSISTANTCLIENT_H

#include <qobject.h>

class QSocket;
class QProcess;

class QAssistantClient : public QObject
{
    Q_OBJECT
public:
    QAssistantClient( const QString &path, QObject *parent = 0, const char *name = 0 );
    ~QAssistantClient();
    virtual void openAssistant();
    virtual void closeAssistant();
    virtual void showPage( const QString &page );
    bool isOpen() const;

signals:
    void assistantOpened();
    void assistantClosed();
    void error( const QString& );

private slots:
    void socketConnected();
    void socketConnectionClosed();
    void readPort();

private:
    QSocket *socket;
    QProcess *proc;
    Q_UINT16 port;
    QString host, assistantCommand, pageBuffer;
    bool opened;
};

#endif
