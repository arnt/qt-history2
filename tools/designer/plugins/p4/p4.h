#ifndef P4_H
#define P4_H

#include <qstring.h>
#include <qobject.h>

class QProcess;
class QComponentInterface;

class P4Edit : public QObject
{
    Q_OBJECT

public:
    P4Edit( const QString &filename, QComponentInterface *iface, bool s );
    ~P4Edit();
    void setFileName( const QString &filename ) { fileName = filename; }
    void edit();

signals:
    void showStatusBarMessage( const QString &s, int );

private slots:
    void newData( const QString &s );
    void processExited();

private:
    enum { FStat, Edit, Done } state;
    QString fileName;
    QString fstatData;
    QProcess *process;
    QComponentInterface *mwIface;
    bool silent;
    
};

#endif
