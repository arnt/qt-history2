#ifndef QURLINFO_H
#define QURLINFO_H

#include "qurl.h"
#include <qdatetime.h>
#include <qstring.h>

struct QUrlInfoPrivate;

class QUrlInfo
{
public:
    QUrlInfo() {}
    QUrlInfo( const QString &name, int permissions, const QString &owner,
	      const QString &group, uint size, const QDateTime &lastModified,
	      const QDateTime &lastRead, bool isDir, bool isFile, bool isSymLink,
	      bool isWritable, bool isReadable, bool isExecutable );
    QUrlInfo( const QUrl &url, int permissions, const QString &owner,
	      const QString &group, uint size, const QDateTime &lastModified,
	      const QDateTime &lastRead, bool isDir, bool isFile, bool isSymLink,
	      bool isWritable, bool isReadable, bool isExecutable );
    QUrlInfo( const QUrl &path, const QString &file );
    QUrlInfo( const QUrlInfo &ui );
    QUrlInfo &operator=( const QUrlInfo &ui );
    ~QUrlInfo();

    QString name() const;
    int permissions() const;
    QString owner() const;
    QString group() const;
    uint size() const;
    QDateTime lastModified() const;
    QDateTime lastRead() const;
    bool isDir() const;
    bool isFile() const;
    bool isSymLink() const;
    bool isWritable() const;
    bool isReadable() const;
    bool isExecutable() const;

    QString makeUrl( const QUrl &path, bool withProtocolWhenLocal = FALSE ) const;

private:
    QUrlInfoPrivate *d;

};

#endif
