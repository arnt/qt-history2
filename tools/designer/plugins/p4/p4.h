#ifndef P4_H
#define P4_H

#include <qstring.h>
#include <qobject.h>
#include <qdict.h>

class QProcess;
class QComponentInterface;

struct P4Info
{
    P4Info() : controlled(FALSE), opened(FALSE), uptodate(TRUE) {}
    bool operator==(const P4Info &other ) const
    {
	return opened == other.opened &&
	    uptodate == other.uptodate &&
	    depotFile == other.depotFile &&
	    controlled == other.controlled;
    }
    bool operator!=(const P4Info &other ) const
    {
	return opened != other.opened ||
	    uptodate != other.uptodate ||
	    depotFile != other.depotFile ||
    	    controlled != other.controlled;
    }

    QString depotFile;
    bool controlled : 1;
    bool opened : 1;
    bool uptodate : 1;

    static QDict<P4Info> files;
};


class P4Action : public QObject
{
    Q_OBJECT
public:
    P4Action( const QString& filename );
    ~P4Action();

    void run( const QString& command );

protected slots:
    virtual void processExited() = 0;

protected:
    QString data() { return p4Data; }
    QString fileName() { return file; }

signals:
    void finished( const QString&, P4Info* );

private slots:
    void newData( const QString& );

private:
    QString p4Data;
    QString file;
    QProcess* process;
};


class P4FStat : public P4Action
{
    Q_OBJECT
public:
    P4FStat( const QString& filename );

    void fstat();

protected:
    void processExited();
};


class P4Edit : public P4Action
{
    Q_OBJECT
public:
    P4Edit( const QString &filename, QComponentInterface *iface, bool s );

    void edit();

protected:
    void processExited();

signals:
    void showStatusBarMessage( const QString &s, int );

private slots:
    void fStatResults( const QString&, P4Info* );

private:
    bool silent;    
};

#endif
