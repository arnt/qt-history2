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

    bool run( const QString& command );

protected slots:
    virtual void processExited() = 0;

protected:
    QString data() { return p4Data; }
    QString fileName() { return file; }
    bool success();

signals:
    void finished( const QString&, P4Info* );
    void showStatusBarMessage( const QString &s );

private slots:
    void newData( const QString& );

private:
    QString p4Data;
    QString file;
    QProcess* process;
};


class P4FStat : public P4Action
{
public:
    P4FStat( const QString& filename );

    void fstat();

protected:
    void processExited();
};

class P4Sync : public P4Action
{
public:
    P4Sync( const QString &filename );

    void sync();

protected:
    void processExited();
};

class P4Edit : public P4Action
{
    Q_OBJECT
public:
    P4Edit( const QString &filename, bool s );

    void edit();

protected:
    void processExited();

private slots:
    void fStatResults( const QString&, P4Info* );

private:
    bool silent;    
};


class P4Submit : public P4Action
{
public:
    P4Submit( const QString &filename );

    void submit();

protected:
    void processExited();
};

class P4Revert : public P4Action
{
public:
    P4Revert( const QString &filename );

    void revert();

protected:
    void processExited();
};

class P4Add : public P4Action
{
public:
    P4Add( const QString &filename );

    void add();

protected:
    void processExited();
};

class P4Delete : public P4Action
{
public:
    P4Delete( const QString &filename );

    void del();

protected:
    void processExited();
};

class P4Diff : public P4Action
{
public:
    P4Diff( const QString &filename );

    void diff();

protected:
    void processExited();
};

#endif
