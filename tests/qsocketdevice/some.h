#include <qvbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qmultilineedit.h>
#include <qcheckbox.h>

#include <qsocketdevice.h>

class Some : public QHBox
{
    Q_OBJECT
public:
    Some( const QString& host_, uint port_ );
    ~Some();

public slots:
    void quit();
    void printInfo();
    void setBlock( bool );

    void connect();
    void bind();
    void listen();
    void accept();
    void waitForMore();

    void read();
    void write();

signals:
    void dataRead( const QString& );
    void quitted();

private:
    QString getInfo();

    QSocketDevice *sd;
    QHostAddress address;
    uint port;

    QLabel *infoLabel;
    QMultiLineEdit *writeEdit;
    QCheckBox *blockingBox;
};
