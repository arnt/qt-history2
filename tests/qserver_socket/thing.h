#include <qvbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qtextview.h>
#include <qmultilineedit.h>
#include <qcheckbox.h>

#include <qsocket.h>

class Thing : public QHBox
{
    Q_OBJECT
public:
    Thing( int fd );
    Thing( QString host, int port );
    ~Thing();

public slots:
    void printInfo();
    void read();
    void readLine();
    void readMuch();
    void write();
    void flush();
    void writeMuch();
    void waitForMore();
    void closeSocket();

private slots:
    void hostFound();
    void connected();
    void closed();
    void delayedCloseFinished();
    void readyRead();
    void bytesWritten( int );
    void error( int );

private:
    void init();
    QString getInfo();

    QSocket *socket;
    QTextView *infoText;
    QTextView *readText;
    QTextView *signalText;
    QMultiLineEdit *writeEdit;
    QCheckBox *asciiCheck;
};
