#include <qsocket.h>
#include <qsocketdevice.h>
#include <qapplication.h>
#include <qstring.h>
#include <sys/socket.h>
#include <qstringlist.h>
#include "qurlinfo.h"

class FTP : public QObject
{
    Q_OBJECT

public:
    FTP();
    ~FTP();
    FTP &operator=( const FTP &ftp );
    void read();
    void open( const QString &host_, int port, const QString &path_ = "/"  );
    void close();

protected:
    void parseDir( const QString &buffer, QUrlInfo &info );

    QSocket *commandSocket, *dataSocket;
    QString host;
    QCString buffer;
    QString path;

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

};
