/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QURL_H
#define QURL_H

#include <QtCore/qobjectdefs.h>
#include <QtCore/qmap.h>
#include <QtCore/qstring.h>

class QByteArray;
class QUrlPrivate;
class QDataStream;

class Q_CORE_EXPORT QUrl
{
public:
    QUrl();
    QUrl(const QString &url);
    QUrl(const QUrl &copy);
    ~QUrl();

    // encoding / toString values
    enum FormattingOption {
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
    Q_DECLARE_FLAGS(FormattingOptions, FormattingOption)

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

    QString toString(FormattingOptions options = None) const;

    QByteArray toEncoded(FormattingOptions options = None) const;
    static QUrl fromEncoded(const QByteArray &url);

    void detach();
    bool isDetached() const;

    bool operator <(const QUrl &url) const;
    bool operator ==(const QUrl &url) const;
    bool operator !=(const QUrl &url) const;
    QUrl &operator =(const QUrl &copy);

    static QString fromPercentEncoding(const QByteArray &);
    static QByteArray toPercentEncoding(const QString &, const char *alsoEncode = 0);
    static QString fromPunycode(const QByteArray &);
    static QByteArray toPunycode(const QString &);

#if defined QT_COMPAT
    inline QT_COMPAT QString protocol() const { return scheme(); }
    inline QT_COMPAT void setProtocol(const QString &s) { setScheme(s); }
    inline QT_COMPAT void setUser(const QString &s) { setUserName(s); }
    inline QT_COMPAT QString user() const { return userName(); }
    inline QT_COMPAT bool hasUser() const { return !userName().isEmpty(); }
    inline QT_COMPAT bool hasPassword() const { return !password().isEmpty(); }
    inline QT_COMPAT bool hasHost() const { return !host().isEmpty(); }
    inline QT_COMPAT bool hasPort() const { return port() != -1; }
    inline QT_COMPAT bool hasPath() const { return !path().isEmpty(); }
    inline QT_COMPAT void setQuery(const QString &txt)
    {
        setEncodedQuery(txt.toLatin1());
    }
    inline QT_COMPAT QString query() const
    {
        return QString::fromLatin1(encodedQuery());
    }
    inline QT_COMPAT QString ref() const { return fragment(); }
    inline QT_COMPAT void setRef(const QString &txt) { setFragment(txt); }
    inline QT_COMPAT bool hasRef() const { return !fragment().isEmpty(); }
    inline QT_COMPAT void addPath(const QString &p) { setPath(path() + QLatin1String("/") + p); }
    QT_COMPAT void setFileName(const QString &txt);
    QT_COMPAT QString fileName() const;
    QT_COMPAT QString dirPath() const;
    static inline QT_COMPAT void decode(QString &url)
    {
        url = QUrl::fromPercentEncoding(url.toLatin1());
    }
    static inline QT_COMPAT void encode(QString &url)
    {
        url = QString::fromLatin1(QUrl::toPercentEncoding(url));
    }
    inline QT_COMPAT operator QString() const { return toString(); }
    inline QT_COMPAT bool cdUp()
    {
        *this = resolved(QUrl(QLatin1String("..")));
        return true;
    }
    static inline QT_COMPAT bool isRelativeUrl(const QString &url)
    {
        return QUrl(url).isRelative();
    }
#endif

protected:
#if defined (QT_COMPAT)
    inline QT_COMPAT void reset() { clear(); }
#endif

    QUrl(QUrlPrivate &d);

private:
    QUrlPrivate *d;
};

Q_DECLARE_TYPEINFO(QUrl, Q_MOVABLE_TYPE);
Q_DECLARE_SHARED(QUrl);
Q_DECLARE_OPERATORS_FOR_FLAGS(QUrl::FormattingOptions)

#ifndef QT_NO_DATASTREAM
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QUrl &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QUrl &);
#endif


#endif
