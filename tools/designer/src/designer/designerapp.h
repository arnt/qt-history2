#ifndef DESIGNERAPP_H
#define DESIGNERAPP_H

#include <qapplication.h>

class QTcpServer;
class QTcpSocket;
class DesignerServer : public QObject
{
    Q_OBJECT
public:
    DesignerServer(QObject* parent);
    static void sendOpenRequest(int port, const QStringList &files);
    Q_UINT16 serverPort() const;
private slots:
    void handleNewConnection();
    void readFromClient();
    void socketClosed();
private:
    QTcpServer *m_server;
    QTcpSocket *m_socket;
};

class MainWindow;
class DesignerApplication : public QApplication
{
public:
    DesignerApplication(int &argc, char *argv[]);
    virtual ~DesignerApplication();
    void setMainWindow(MainWindow *mw);

protected:
    bool event(QEvent *ev);

private:
    MainWindow *mMainWindow;
};
#endif
