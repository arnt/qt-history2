#ifndef QEXTURL_H
#define QEXTURL_H

#include "qurl.h"
#include "qftp.h"

class QExtUrl : public QUrl
{
    Q_OBJECT
public:
    QExtUrl();
    QExtUrl( const QString& url );
    QExtUrl( const QUrl& url );
    QExtUrl( const QUrl& url, const QString& relUrl_ );

    virtual void listEntries( const QString &nameFilter, int filterSpec = QDir::DefaultFilter,
			      int sortSpec   = QDir::DefaultSort );
    virtual void mkdir( const QString &dirname );
    virtual QString toString() const;

private:
    QFtp ftp;
    
};

#endif
