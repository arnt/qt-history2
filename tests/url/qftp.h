#ifndef QFTP_H
#define QFTP_H

#include <qsocket.h>
#include <qsocketdevice.h>
#include <qapplication.h>
#include <qstring.h>
#include <sys/socket.h>
#include <qstringlist.h>
#include "qurlinfo.h"

class QFtp : public QObject
{
    Q_OBJECT

public:
    QFtp();
    ~QFtp();
    QFtp &operator=( const QFtp &ftp );
    enum Command {
	List = 0,
	Mkdir
    };
    void read();
    void open( const QString &host_, int port, const QString &path_ = "/",
	       const QString &username_ = "anonymous", const QString &passwd_ = "Qt is cool!",
	       Command cmd = List, const QString &extraData_ = QString::null );
    void close();

protected:
    void parseDir( const QString &buffer, QUrlInfo &info );

    QSocket *commandSocket, *dataSocket;
    QString host;
    QCString buffer;
    QString path, username, passwd;
    Command command;
    QString extraData;

protected slots:
    void hostFound();
    void connected();
    void closed();
    void readyRead();
    void dataHostFound();
    void dataConnected();
    void dataClosed();
    void dataReadyRead();

signals:
    void newEntry( const QUrlInfo & );
    void listFinished();

};

#endif
