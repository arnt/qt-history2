#ifndef Q4URL_H
#define Q4URL_H

#include <qobjectdefs.h>
#include <qmap.h>
#include <qstring.h>

class QByteArray;
class QUrlPrivate;

class QUrl
{
public:
    QUrl(const QString &url = QString::null);
    QUrl(const QUrl &copy);
    ~QUrl();

    // encoding / toString values
    enum FormattingOptions {
        None = 0x0,
        RemoveScheme = 0x1,
        RemovePassword = 0x2,
        RemoveUserInfo = RemovePassword | 0x4,
        RemovePort = 0x8,
        RemoveAuthority = RemoveUserInfo | RemovePort | 0x10,
        RemovePath = 0x20,
        RemoveQuery = 0x40,
        RemoveFragment = 0x80,

        StripTrailingSlash = 0x10000
    };

    void setUrl(const QString &url);
    void setEncodedUrl(const QByteArray &url);

    bool isValid() const;

    void clear();

    void setScheme(const QString &scheme);
    QString scheme() const;

    void setAuthority(const QString &authority);
    QString authority() const;

    void setUserInfo(const QString &userInfo);
    QString userInfo() const;

    void setUserName(const QString &userName);
    QString userName() const;

    void setPassword(const QString &password);
    QString password() const;

    void setHost(const QString &host);
    QString host() const;

    void setPort(int port);
    int port() const;

    void setPath(const QString &path);
    QString path() const;

    void setEncodedQuery(const QByteArray &query);
    QByteArray encodedQuery() const;

    void setQueryDelimiters(char valueDelimiter, char pairDelimiter);
    char queryValueDelimiter() const;
    char queryPairDelimiter() const;

    void setQueryItems(const QMap<QString, QString> &query);
    void addQueryItem(const QString &key, const QString &value);
    void removeQueryItem(const QString &key);
    QMap<QString, QString> queryItems() const;

    void setFragment(const QString &fragment);
    QString fragment() const;

    QUrl resolved(const QUrl &relative) const; // ### name!

    bool isRelative() const;

    bool isLocalFile() const;
    static QUrl fromLocalFile(const QString &localfile);
    QString toLocalFile() const;

    QString toString(int formattingOptions = None) const;

    QByteArray toEncoded() const;
    static QUrl fromEncoded(const QByteArray &url);

    static QString fromPercentageEncodingThenUtf8(const QByteArray &);
    static QByteArray toUtf8ThenPercentageEncoding(const QString &, const char alsoEncode[] = "");

    void detach();

    bool operator <(const QUrl &url) const;
    bool operator ==(const QUrl &url) const;
    bool operator !=(const QUrl &url) const;
    QUrl &operator =(const QUrl &copy);

    bool isParentOf(const QUrl &url) const;

protected:
    QUrl(QUrlPrivate &d);

private:
    QUrlPrivate *d;
};

#endif
