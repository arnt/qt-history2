#ifndef QURL_H
#define QURL_H

#include <qobjectdefs.h>
#include <qmap.h>
#include <qstring.h>

#if !defined QT_NO_COMPAT
#include <qfileinfo.h>
#endif

class QByteArray;
class QUrlPrivate;

class Q_CORE_EXPORT QUrl
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

    QUrl resolved(const QUrl &relative) const;

    bool isRelative() const;
    bool isParentOf(const QUrl &url) const;

    static QUrl fromLocalFile(const QString &localfile);
    QString toLocalFile() const;

    QString toString(int formattingOptions = None) const;

    QByteArray toEncoded() const;
    static QUrl fromEncoded(const QByteArray &url);

    void detach();

    bool operator <(const QUrl &url) const;
    bool operator ==(const QUrl &url) const;
    bool operator !=(const QUrl &url) const;
    QUrl &operator =(const QUrl &copy);

    static QString fromPercentEncoding(const QByteArray &);
    static QByteArray toPercentEncoding(const QString &, const char *alsoEncode = 0);
    static QString fromPunycode(const QByteArray &);
    static QByteArray toPunycode(const QString &);

#if !defined QT_NO_COMPAT
    inline QT_COMPAT QString protocol() const { return scheme(); }
    inline QT_COMPAT void setProtocol(const QString &s) { setScheme(s); }
    inline QT_COMPAT void setUser(const QString &s) { setUserName(s); }
    inline QT_COMPAT QString user() const { return userName(); }
    inline QT_COMPAT bool hasUser() const { return !userName().isEmpty(); }
    inline QT_COMPAT bool hasPassword() const { return !password().isEmpty(); }
    inline QT_COMPAT bool hasHost() const { return !host().isEmpty(); }
    inline QT_COMPAT bool hasPort() const { return port() != -1; }
    inline QT_COMPAT bool hasPath() const { return !path().isEmpty(); }
    inline QT_COMPAT void setEncodedPathAndQuery(const QString &enc)
    {
        int offset = enc.indexOf('/');
        if (offset != -1) {
            setPath(fromPercentEncoding(enc.left(offset).toLatin1()));
            setEncodedQuery(enc.mid(offset + 1).toLatin1());
        } else {
            setPath(fromPercentEncoding(enc.toLatin1()));
        }
    }
    inline QT_COMPAT QString encodedPathAndQuery() const
    {
        return toPercentEncoding(path()) + "/" + encodedQuery();
    }
    inline QT_COMPAT void setQuery(const QString &txt)
    {
        setEncodedQuery(toPercentEncoding(txt));
    }
    inline QT_COMPAT QString query() const
    {
        return fromPercentEncoding(encodedQuery());
    }
    inline QT_COMPAT QString ref() const { return fragment(); }
    inline QT_COMPAT void setRef(const QString &txt) { setFragment(txt); }
    inline QT_COMPAT bool hasRef() const { return !fragment().isEmpty(); }
    inline QT_COMPAT void addPath(const QString &p) { setPath(path() + "/" + p); }
    inline QT_COMPAT void setFileName(const QString &txt)
    {
        QFileInfo fileInfo(path());
        fileInfo.setFile(txt);
        setPath(fileInfo.filePath());
    }
    inline QT_COMPAT QString fileName() const
    {
        QFileInfo fileInfo(path());
        return fileInfo.fileName();
    }
    inline QT_COMPAT QString dirPath() const
    {
        QFileInfo fileInfo(path());
        if (fileInfo.isAbsolute())
            return fileInfo.absolutePath();
        return fileInfo.path();
    }
    static inline QT_COMPAT void decode(QString &url)
    {
        url = QUrl::fromPercentEncoding(url.toLatin1());
    }
    static inline QT_COMPAT void encode(QString &url)
    {
        url = QUrl::toPercentEncoding(url);
    }
    inline QT_COMPAT operator QString() const { return toString(); }
    inline QT_COMPAT bool cdUp() { *this = resolved(QUrl("..")); return true; }
    static inline QT_COMPAT bool isRelativeUrl(const QString &url)
    {
        return QUrl(url).isRelative();
    }
#endif

protected:
#if !defined (QT_NO_COMPAT)
    inline QT_COMPAT void reset() { clear(); }
#endif

    QUrl(QUrlPrivate &d);

private:
    QUrlPrivate *d;
};

#endif
