#ifndef QURL_H
#define QURL_H

#include <qstring.h>
#include <qdir.h>
#include <qobject.h>

struct QUrlPrivate;
class QUrlInfo;

class QUrl : public QObject
{
    Q_OBJECT

public:
    QUrl();
    QUrl( const QString& url );
    QUrl( const QUrl& url );
    QUrl( const QUrl& url, const QString& relUrl_ );
    ~QUrl();

    QString protocol() const;
    void setProtocol( const QString& protocol );

    QString user() const;
    void setUser( const QString& user );
    bool hasUser() const;

    QString pass() const;
    void setPass( const QString& pass );
    bool hasPass() const;

    QString host() const;
    void setHost( const QString& user );
    bool hasHost() const;

    int port() const;
    void setPort( int port );

    QString path() const;
    QString path( int trailing ) const;
    void setPath( const QString& path );
    bool hasPath() const;

    void setEncodedPathAndQuery( const QString& enc );
    QString encodedPathAndQuery( int trailing = 0, bool noEmptyPath = FALSE );

    void setQuery( const QString& txt );
    QString query() const;

    QString ref() const;
    void setRef( const QString& txt );
    bool hasRef() const;

    bool isMalformed() const;

    bool isLocalFile() const;

    void addPath( const QString& path );
    void setFileName( const QString& txt );

    QString filename( bool ignoreTrailingSlashInPath = TRUE );
    QString directory( bool stripTrailingSlashFromResult = TRUE,
		       bool ignoreTrailingSlashInPath = TRUE );

    QString url();
    QString url( int trailing, bool stripRef = FALSE );

    QUrl& operator=( const QUrl& url );
    QUrl& operator=( const QString& url );

    bool operator==( const QUrl& url ) const;
    bool operator==( const QString& url ) const;
    bool cmp( QUrl &url, bool ignoreTrailing = FALSE );

    static void decode( QString& url );
    static void encode( QString& url );

    void listEntries( int filterSpec = QDir::DefaultFilter,
		      int sortSpec   = QDir::DefaultSort );
    void listEntries( const QString &nameFilter, int filterSpec = QDir::DefaultFilter,
		      int sortSpec   = QDir::DefaultSort );
    void mkdir( const QString &dirname );
    void remove( const QString &filename );
    void rename( const QString &oldname, const QString &newname );
    void copy( const QString &from, const QString &to );
    void copy( const QStringList &files, const QString &dest, bool move );
    
    void setNameFilter( const QString &nameFilter );
    QString nameFilter() const;

    QUrlInfo makeInfo() const;
    operator QString() const;

    bool cdUp();

signals:
    void entry( const QUrlInfo & );
    void error( int, const QString & );
    void finished();
    void start();
    void createdDirectory( const QUrlInfo & );
    void removed( const QString & );
    void couldNotDelete( const QString & );
    void itemChanged( const QString &oldname, const QString &newname );
    
protected:
    void reset();
    void parse( const QString& url );

    static char hex2int( char c );

private:
    QUrlPrivate *d;

};


#endif
