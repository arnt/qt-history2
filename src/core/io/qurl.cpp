
/*! \class U4rl

    \brief The QUrl class provides a convenient interface for working
    with URLs.
    \reentrant

*/

#include <private/qunicodetables_p.h>
#include <qatomic.h>
#include <qbytearray.h>
#include <qlist.h>
#include <qmap.h>
#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qstack.h>
#include <qstringlist.h>
#include <qtextcodec.h>
#include <qvarlengtharray.h>
#include "qurl.h"

//#define Q4URL_DEBUG

Q_CORE_EXPORT bool qt_resolve_symlinks = true; //### this can sit here for now but needs to go some where else

class QUrlPrivate
{
public:
    QUrlPrivate();

    bool setUrl(const QString &url);

    QString authority(int formattingOptions = QUrl::None) const;
    void setAuthority(const QString &auth);
    void setUserInfo(const QString &userInfo);
    QString userInfo(int formattingOptions = QUrl::None) const;

    QString mergePaths(const QString &relativePath) const;

    void removeDotsFromPath();

    enum ParseOptions {
        ParseAndSet,
        ParseOnly
    };

    void validate() const;
    void parse(ParseOptions parseOptions = ParseAndSet) const;
    void clear();

    QByteArray toEncoded() const;

    QAtomic ref;

    QString scheme;
    QString userName;
    QString password;
    QString host;
    int port;
    QString path;
    QByteArray query;
    QByteArray fragment;

    QByteArray encodedOriginal;

    bool isValid;

    char valueDelimiter;
    char pairDelimiter;

    bool isParsed;
    bool isValidated;
};

#if defined Q_OS_WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

// ### put this somewhere. and ues wa{}
QString qGetHostName()
{
#if defined Q_OS_WIN32
    char hostname[512];
    DWORD size = sizeof(hostname);
    if (GetComputerNameA(hostname, &size))
	return QString::fromAscii(hostname, size);
#else
    char hostname[512];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        hostname[sizeof(hostname) - 1] = '\0';
        return QString::fromAscii(hostname);
    }
#endif
    return QString();
}


class UrlParser {
public:
    // inline for speed
    inline bool _char(char **ptr, char expected)
    {
        if (*((*ptr)) == expected) {
            ++(*ptr);
            return true;
        }

        return false;
    }

    inline bool _HEXDIG(char **ptr, char *dig)
    {
        char *ptrBackup = *ptr;
        char ch = *((*ptr)++);
        if ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')) {
            *dig = ch;
            return true;
        }

        *ptr = ptrBackup;
        return false;
    }

    // pct-encoded = "%" HEXDIG HEXDIG
    inline bool _pctEncoded(char **ptr, char pct[])
    {
        char *ptrBackup = *ptr;
        if (!_char(ptr, '%')) return false;

        char hex1, hex2;
        if (!_HEXDIG(ptr, &hex1)) { *ptr = ptrBackup; return false; }
        if (!_HEXDIG(ptr, &hex2)) { *ptr = ptrBackup; return false; }

        pct[0] = '%';
        pct[1] = hex1;
        pct[2] = hex2;
        pct[3] = '\0';

        return true;
    }

    // gen-delims  = ":" / "/" / "?" / "#" / "[" / "]" / "@"
    inline bool _genDelims(char **ptr, char *c)
    {
        char *ptrBackup = *ptr;
        char ch = *((*ptr)++);
        switch (ch) {
        case ':': case '/': case '?': case '#':
        case '[': case ']': case '@':
            *c = ch;
            return true;
        default:
            *ptr = ptrBackup;
            return false;
        }
    }

    // sub-delims  = "!" / "$" / "&" / "'" / "(" / ")"
    //             / "*" / "+" / "," / ";" / "="
    inline bool _subDelims(char **ptr, char *c)
    {
        char *ptrBackup = *ptr;
        char ch = *((*ptr)++);
        switch (ch) {
        case '!': case '$': case '&': case '\'':
        case '(': case ')': case '*': case '+':
        case ',': case ';': case '=':
            *c = ch;
            return true;
        default:
            *ptr = ptrBackup;
            return false;
        }
    }

    inline bool _ALPHA_(char **ptr, char *c)
    {
        char *ptrBackup = *ptr;
        char ch = *((*ptr)++);
        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
            *c = ch;
            return true;
        }

        *ptr = ptrBackup;
        return false;
    }

    inline bool _DIGIT_(char **ptr, char *c)
    {
        char *ptrBackup = *ptr;
        char ch = *((*ptr)++);
        if (ch >= '0' && ch <= '9') {
            *c = ch;
            return true;
        }

        *ptr = ptrBackup;
        return false;
    }

    // unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~"
    inline bool _unreserved(char **ptr, char *c)
    {
        if (_ALPHA_(ptr, c) || _DIGIT_(ptr, c))
            return true;

        char *ptrBackup = *ptr;
        char ch = *((*ptr)++);
        switch (ch) {
        case '-': case '.': case '_': case '~':
            *c = ch;
            return true;
        default:
            *ptr = ptrBackup;
            return false;
        }
    }

    // scheme      = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
    bool _scheme(char **ptr, QByteArray *scheme)
    {
        bool first = true;

        for (;;) {
            char ch = *(*ptr);
            if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
                *scheme += ch;
            } else if (!first && ((ch >= '0' && ch <= '9') || ch == '+' || ch == '-' || ch == '.')) {
                *scheme += ch;
            } else {
                break;
            }

            (*ptr)++;
            first = false;
        }

        return true;
    }

    // IPvFuture  = "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )
    bool _IPvFuture(char **ptr, QByteArray *host)
    {
        char *ptrBackup = *ptr;
        char ch = *((*ptr)++);
        if (ch != 'v') {
            *ptr = ptrBackup;
            return false;
        }

        *host += ch;

        if (!_HEXDIG(ptr, &ch)) {
            *ptr = ptrBackup;
            return false;
        }

        *host += ch;

        while (_HEXDIG(ptr, &ch))
            *host += ch;

        if (*((*ptr)++) != '.') {
            *ptr = ptrBackup;
            return false;
        }

        if (!_unreserved(ptr, &ch) && !_subDelims(ptr, &ch) && (ch = *((*ptr)++)) != ':') {
            *ptr = ptrBackup;
            return false;
        }

        *host += ch;

        while (_unreserved(ptr, &ch) || _subDelims(ptr, &ch) || (ch = *((*ptr)++)) == ':')
            *host += ch;

        return true;
    }

    // h16         = 1*4HEXDIG
    //             ; 16 bits of address represented in hexadecimal
    bool _h16(char **ptr, QByteArray *c)
    {
        char ch;
        if (!_HEXDIG(ptr, &ch))
            return false;
        *c += ch;

        for (int i = 0; i < 3; ++i) {
            if (!_HEXDIG(ptr, &ch))
                break;
            *c += ch;
        }

        return true;
    }

    // dec-octet   = DIGIT                 ; 0-9
    //             / %x31-39 DIGIT         ; 10-99
    //             / "1" 2DIGIT            ; 100-199
    //             / "2" %x30-34 DIGIT     ; 200-249
    //             / "25" %x30-35          ; 250-255
    bool _decOctet(char **ptr, QByteArray *octet)
    {
        char *ptrBackup = *ptr;
        char c1 = *((*ptr)++);
        if (c1 < '0' || c1 > '9') {
            *ptr = ptrBackup;
            return false;
        }

        *octet += c1;

        if (c1 == '0')
            return true;

        char c2 = *((*ptr)++);
        if (c2 < '0' || c2 > '9') {
            *ptr = ptrBackup;
            return true;
        }

        *octet += c2;

        char c3 = *((*ptr)++);
        if (c3 < '0' || c3 > '9') {
            *ptr = ptrBackup;
            return true;
        }

        *octet += c3;

        // If there is a three digit number larger than 255, reject the
        // whole token.
        if (c1 > '2' || c2 > '5' || c3 > '5') {
            *ptr = ptrBackup;
            return false;
        }

        return true;
    }

    // IPv4address = dec-octet "." dec-octet "." dec-octet "." dec-octet
    bool _IPv4Address(char **ptr, QByteArray *c)
    {
        char *ptrBackup = *ptr;
        QByteArray tmp1; tmp1.reserve(32);

        if (!_decOctet(ptr, &tmp1))
            return false;

        *c += tmp1;

        for (int i = 0; i < 3; ++i) {
            if (*((*ptr)++) != '.') {
                *ptr = ptrBackup;
                return false;
            }

            *c += '.';

            tmp1.truncate(0);
            if (!_decOctet(ptr, &tmp1))
                return false;

            *c += tmp1;
        }

        return true;
    }

    // ls32        = ( h16 ":" h16 ) / IPv4address
    //             ; least-significant 32 bits of address
    bool _ls32(char **ptr, QByteArray *c)
    {
        char *ptrBackup = *ptr;
        QByteArray tmp1;
        QByteArray tmp2;
        if (_h16(ptr, &tmp1) && _char(ptr, ':') && _h16(ptr, &tmp2)) {
            *c = tmp1;
            *c += ':';
            *c += tmp2;
            return true;
        }

        *ptr = ptrBackup;
        return _IPv4Address(ptr, c);
    }

    // IPv6address =                            6( h16 ":" ) ls32
    //             /                       "::" 5( h16 ":" ) ls32
    //             / [               h16 ] "::" 4( h16 ":" ) ls32
    //             / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32
    //             / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32
    //             / [ *3( h16 ":" ) h16 ] "::"    h16 ":"   ls32
    //             / [ *4( h16 ":" ) h16 ] "::"              ls32
    //             / [ *5( h16 ":" ) h16 ] "::"              h16
    //             / [ *6( h16 ":" ) h16 ] "::"
    bool _IPv6Address(char **ptr, QByteArray *host)
    {
        (void)host;

        char *ptrBackup = *ptr;

        QByteArray tmp1;
        if (_h16(ptr, &tmp1)) {
            // ### FIXME!!!
            // 6( h16 ":" ) ls32
            // [ h16 ] "::" 4( h16 ":" ) ls32
            // [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32
            // [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32
            // [ *3( h16 ":" ) h16 ] "::"    h16 ":"   ls32
            // [ *4( h16 ":" ) h16 ] "::"              ls32
            // [ *5( h16 ":" ) h16 ] "::"              h16
            // [ *6( h16 ":" ) h16 ] "::"
        } else {
            // "::" 5( h16 ":" ) ls32
            if (!(_char(ptr, ':') && _char(ptr, ':'))) {
                *ptr = ptrBackup;
                return false;
            }

        }

        return false;
    }

    // IP-literal = "[" ( IPv6address / IPvFuture  ) "]"
    bool _IPLiteral(char **ptr, QByteArray *host)
    {
        char *ptrBackup = *ptr;
        if (!_char(ptr, '['))
            return false;

        *host += '[';

        if (!_IPv6Address(ptr, host) && !_IPvFuture(ptr, host)) {
            *ptr = ptrBackup;
            return false;
        }

        if (!_char(ptr, ']')) {
            *ptr = ptrBackup;
            return false;
        }

        *host += ']';

        return true;
    }

    // reg-name    = 0*255( unreserved / pct-encoded / sub-delims )
    bool _regName(char **ptr, QByteArray *host)
    {
        char pctTmp[4];
        for (int i = 0; i < 255; ++i) {
            char ch;
            if (!_unreserved(ptr, &ch) && !_subDelims(ptr, &ch)) {
                if (!_pctEncoded(ptr, pctTmp))
                    break;
                *host += pctTmp;
            } else {
                *host += ch;
            }
        }

        return true;
    }

    // host        = IP-literal / IPv4address / reg-name
    bool _host(char **ptr, QByteArray *host)
    {
        return (_IPLiteral(ptr, host) || _IPv4Address(ptr, host) || _regName(ptr, host));
    }

    // userinfo    = *( unreserved / pct-encoded / sub-delims / ":" )
    bool _userInfo(char **ptr, QByteArray *userInfo)
    {
        for (;;) {
            char ch;
            if (_unreserved(ptr, &ch) || _subDelims(ptr, &ch)) {
                *userInfo += ch;
            } else {
                char pctTmp[4];
                if (_pctEncoded(ptr, pctTmp)) {
                    *userInfo += pctTmp;
                } else if (_char(ptr, ':')) {
                    *userInfo += ':';
                } else {
                    break;
                }
            }
        }

        return true;
    }

    // port        = *DIGIT
    bool _port(char **ptr, int *port)
    {
        bool first = true;

        for (;;) {
            char *ptrBackup = *ptr;
            char ch = *((*ptr)++);
            if (ch < '0' || ch > '9') {
                *ptr = ptrBackup;
                return true;
            }

            if (first) {
                first = false;
                *port = 0;
            }

            *port *= 10;
            *port += ch - '0';
        }

        return true;
    }

    // authority   = [ userinfo "@" ] host [ ":" port ]
    bool _authority(char **ptr, QByteArray *userInfo, QByteArray *host, int *port)
    {
        char *ptrBackup = *ptr;
        if (_userInfo(ptr, userInfo)) {
            if (*((*ptr)++) != '@') {
                *ptr = ptrBackup;
                userInfo->clear();
                // fall through
            }
        }

        if (!_host(ptr, host)) {
            *ptr = ptrBackup;
            return false;
        }

        char *ptrBackup2 = *ptr;
        if (*((*ptr)++) != ':') {
            *ptr = ptrBackup2;
            return true;
        }

        if (!_port(ptr, port)) {
            *ptr = ptrBackup2;
            return false;
        }

        return true;
    }

    // pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"
    inline bool _pchar(char **ptr, char pc[])
    {
        char c = *(*ptr);

        switch (c) {
        case '!': case '$': case '&': case '\'': case '(': case ')': case '*':
        case '+': case ',': case ';': case '=': case ':': case '@':
        case '-': case '.': case '_': case '~':
            pc[0] = c;
            pc[1] = '\0';
            ++(*ptr);
            return true;
        default:
            break;
        };

        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
            pc[0] = c;
            pc[1] = '\0';
            ++(*ptr);
            return true;
        }

        if (_pctEncoded(ptr, pc))
            return true;

        return false;
    }

    // segment       = *pchar
    bool _segment(char **ptr, QByteArray *segment)
    {
        for (;;) {
            char pctTmp[4];
            if (!_pchar(ptr, pctTmp))
                break;

            *segment += pctTmp;
        }

        return true;
    }

    // segment       = *pchar
    bool _segmentNZ(char **ptr, QByteArray *segment)
    {
        char pctTmp[4];
        if (!_pchar(ptr, pctTmp))
            return false;

        *segment += pctTmp;

        for (;;) {
            if (!_pchar(ptr, pctTmp))
                break;

            *segment += pctTmp;
        }

        return true;
    }

    // path-abempty  = *( "/" segment )
    bool _pathAbEmpty(char **ptr, QByteArray *path)
    {
        for (;;) {
            char *ptrBackup = *ptr;
            if (*((*ptr)++) != '/') {
                *ptr = ptrBackup;
                break;
            }

            *path += '/';

            char pctTmp[4];
            if (_pchar(ptr, pctTmp)) {
                *path += pctTmp;
                while (_pchar(ptr, pctTmp))
                    *path += pctTmp;
            }
        }

        return true;
    }

    // path-abs      = "/" [ segment-nz *( "/" segment ) ]
    bool _pathAbs(char **ptr, QByteArray *path)
    {
        char *ptrBackup = *ptr;
        char ch = *((*ptr)++);
        if (ch != '/') {
            *ptr = ptrBackup;
            return false;
        }

        *path += '/';

        // ### be smart
        QByteArray tmp;
        if (!_segmentNZ(ptr, &tmp))
            return true;

        *path += tmp;

        for (;;) {
            char *ptrBackup2 = *ptr;
            if (*((*ptr)++) != '/') {
                *ptr = ptrBackup2;
                break;
            }

            // ### be smart
            QByteArray segment;
            if (!_segment(ptr, &segment)) {
                *ptr = ptrBackup2;
                break;
            }

            *path += '/';
            *path += segment;
        }

        return true;
    }

    // path-rootless = segment-nz *( "/" segment )
    bool _pathRootless(char **ptr, QByteArray *path)
    {
        // ### be smart
        QByteArray segment;
        if (!_segmentNZ(ptr, &segment))
            return false;

        *path += segment;

        for (;;) {
            char *ptrBackup2 = *ptr;
            if (*((*ptr)++) != '/') {
                *ptr = ptrBackup2;
                break;
            }

            // ### be smart
            QByteArray segment;
            if (!_segment(ptr, &segment)) {
                *ptr = ptrBackup2;
                break;
            }

            *path += '/';
            *path += segment;
        }

        return true;
    }

    // path-empty    = 0<pchar>
    bool _pathEmpty(char **, QByteArray *path)
    {
        path->truncate(0);
        return true;
    }

    // hier-part   = "//" authority path-abempty
    //             / path-abs
    //             / path-rootless
    //             / path-empty
    bool _hierPart(char **ptr, QByteArray *userInfo, QByteArray *host, int *port, QByteArray *path)
    {
        char *ptrBackup = *ptr;
        if (*((*ptr)++) == '/' && *((*ptr)++) == '/') {
            if (!_authority(ptr, userInfo, host, port)) { *ptr = ptrBackup; return false; }
            if (!_pathAbEmpty(ptr, path)) { *ptr = ptrBackup; return false; }
            return true;
        } else {
            *ptr = ptrBackup;
            return (_pathAbs(ptr, path) || _pathRootless(ptr, path) || _pathEmpty(ptr, path));
        }
    }

    // query       = *( pchar / "/" / "?" )
    bool _query(char **ptr, QByteArray *query)
    {
        for (;;) {
            char tmp[4];
            if (_pchar(ptr, tmp)) {
                *query += tmp;
            } else {
                char *ptrBackup = *ptr;
                char ch = *((*ptr)++);
                if (ch == '/' || ch == '?')
                    *query += ch;
                else {
                    *ptr = ptrBackup;
                    break;
                }
            }
        }

        return true;
    }

    // fragment    = *( pchar / "/" / "?" )
    bool _fragment(char **ptr, QByteArray *fragment)
    {
        for (;;) {
            char tmp[4];
            if (_pchar(ptr, tmp)) {
                *fragment += tmp;
            } else {
                char *ptrBackup = *ptr;
                char ch = *((*ptr)++);

                // exception: allow several '#' characters within a
                // fragment.
                if (ch == '/' || ch == '?' || ch == '#')
                    *fragment += ch;
                else {
                    *ptr = ptrBackup;
                    break;
                }
            }
        }

        return true;
    }
};

QUrlPrivate::QUrlPrivate()
{
    ref = 1;
    port = -1;
    isValid = false;
    valueDelimiter = '=';
    pairDelimiter = '&';
    isParsed = false;
    isValidated = false;
}

QString QUrlPrivate::authority(int formattingOptions) const
{
    if ((formattingOptions & QUrl::RemoveAuthority) == QUrl::RemoveAuthority)
        return QString();

    QString tmp = userInfo(formattingOptions);
    if (!tmp.isEmpty()) tmp += "@";
    tmp += host;
    if (!(formattingOptions & QUrl::RemovePort) && port != -1)
        tmp += ":" + QString::number(port);

    return tmp;
}

void QUrlPrivate::setAuthority(const QString &auth)
{
    if (auth.isEmpty())
        return;

    // find the port section of the authority by searching from the
    // end towards the beginning for numbers until a ':' is reached.
    int portIndex = auth.length() - 1;
    if (portIndex == 0) {
        portIndex = -1;
    } else {
        short c = auth.at(portIndex - 1).unicode();
        if (c < '0' || c > '9') {
            portIndex = -1;
        } else while (portIndex > 0) {
            c = auth.at(portIndex - 1).unicode();
            if (c == ':') {
                portIndex--;
                break;
            }
            portIndex--;
        }
    }

    port = -1;
    if (portIndex != -1) {
        for (int i = portIndex + 1; i < auth.length(); ++i) {
            short c = auth.at(i).unicode();
            if (c < '0' || c > '9') break;
            if (port == -1) port = 0;
            port = (port * 10) + (c - '0');
        }
    }

    int userInfoIndex = auth.indexOf('@');
    if (userInfoIndex != -1 && (portIndex == -1 || userInfoIndex < portIndex))
        setUserInfo(auth.left(userInfoIndex));

    int hostIndex = 0;
    if (userInfoIndex != -1)
        hostIndex = userInfoIndex + 1;
    int hostLength = auth.length() - hostIndex;
    if (portIndex != -1)
        hostLength -= (auth.length() - portIndex);

    host = auth.mid(hostIndex, hostLength).toLower().trimmed();
}

void QUrlPrivate::setUserInfo(const QString &userInfo)
{
    int delimIndex = userInfo.indexOf(':');
    if (delimIndex == -1) {
        userName = userInfo;
        password.clear();
        return;
    }
    userName = userInfo.left(delimIndex);
    password = userInfo.right(userInfo.length() - delimIndex - 1);
}

QString QUrlPrivate::userInfo(int formattingOptions) const
{
    if ((formattingOptions & QUrl::RemoveUserInfo) == QUrl::RemoveUserInfo)
        return QString::null;

    QString tmp;
    tmp += userName;

    if (!(formattingOptions & QUrl::RemovePassword) && !password.isEmpty())
        tmp += ":" + password;

    return tmp;
}

/*
    From draft-fielding-uri-rfc2396bis-05.txt, 5.2.3: Merge paths

    Returns a merge of the current path with the relative path passed
    as argument.
*/
QString QUrlPrivate::mergePaths(const QString &relativePath) const
{
    // If the base URI has a defined authority component and an empty
    // path, then return a string consisting of "/" concatenated with
    // the reference's path; otherwise,
    if (!authority().isEmpty() && path.isEmpty())
        return "/" + relativePath;

    // Return a string consisting of the reference's path component
    // appended to all but the last segment of the base URI's path
    // (i.e., excluding any characters after the right-most "/" in the
    // base URI path, or excluding the entire base URI path if it does
    // not contain any "/" characters).
    QString newPath;
    if (!path.contains(QLatin1String("/")))
        newPath = relativePath;
    else
        newPath = path.left(path.lastIndexOf(QLatin1String("/")) + 1) + relativePath;

    return newPath;
}


/*
    From draft-fielding-uri-rfc2396bis-05.txt, 5.2.4: Remove dot segments

    Removes unnecessary ../ and ./ from the path. Used for normalizing
    the URL.
*/
void QUrlPrivate::removeDotsFromPath()
{
    // The input buffer is initialized with the now-appended path
    // components and the output buffer is initialized to the empty
    // string.
    QString origPath = path;
    path.clear();
    path.reserve(origPath.length());

    //###
    const QString Dot = QLatin1String(".");
    const QString Slash = QLatin1String("/");
    const QString DotDot = QLatin1String("..");
    const QString DotSlash = QLatin1String("./");
    const QString SlashDot = QLatin1String("/.");
    const QString DotDotSlash = QLatin1String("../");
    const QString SlashDotSlash = QLatin1String("/./");
    const QString SlashDotDotSlash = QLatin1String("/../");
    const QString SlashDotDot = QLatin1String("/..");

    // While the input buffer is not empty, loop:
    while (!origPath.isEmpty()) {

        // If the input buffer begins with a prefix of "../" or "./",
        // then remove that prefix from the input buffer;
        if (origPath.startsWith(DotSlash)) {
            origPath.remove(0, 2);
        } else if (origPath.startsWith(DotDotSlash)) {
            origPath.remove(0, 3);
        } else {
            // otherwise, if the input buffer begins with a prefix of
            // "/./" or "/.", where "." is a complete path segment,
            // then replace that prefix with "/" in the input buffer;
            if (origPath.startsWith(SlashDotSlash)) {
                origPath.remove(0, 2);
            } else if (origPath == SlashDot) {
                origPath = Slash;
            } else {
                // otherwise, if the input buffer begins with a prefix
                // of "/../" or "/..", where ".." is a complete path
                // segment, then replace that prefix with "/" in the
                // input buffer and remove the last //segment and its
                // preceding "/" (if any) from the output buffer;
                if (origPath.startsWith(SlashDotDotSlash)) {
                    origPath.remove(0, 3);
                    if (path.contains(Slash))
                        path.truncate(path.lastIndexOf(Slash));
                } else if (origPath == SlashDotDot) {
                    origPath = Slash;
                    if (path.contains(Slash))
                        path.truncate(path.lastIndexOf(Slash));
                } else {
                    // otherwise, if the input buffer consists only of
                    // "." or "..", then remove that from the input
                    // buffer;
                    if (origPath == Dot || origPath == DotDot) {
                        origPath.clear();
                    } else {
                        // otherwise move the first path segment in
                        // the input buffer to the end of the output
                        // buffer, including the initial "/" character
                        // (if any) and any subsequent characters up
                        // to, but not including, the next "/"
                        // character or the end of the input buffer.
                        int index = origPath.indexOf(Slash);
                        if (index == 0) {
                            path += Slash;
                            origPath.remove(0, 1);
                            index = origPath.indexOf(Slash);
                        }
                        if (index != -1) {
                            path += origPath.left(index);
                            origPath.remove(0, index);
                        } else {
                            path += origPath;
                            origPath.clear();
                        }
                    }
                }
            }
        }
    }
}

void QUrlPrivate::validate() const
{
    QUrlPrivate *that = (QUrlPrivate *)this;
    that->encodedOriginal = that->toEncoded(); // may detach
    parse(ParseOnly);
}

void QUrlPrivate::parse(ParseOptions parseOptions) const
{
    QUrlPrivate *that = (QUrlPrivate *)this;
    if (encodedOriginal.isEmpty()) {
        that->isValid = false;
        that->isValidated = true;
        that->isParsed = true;
        return;
    }

    UrlParser parser;

    QByteArray __scheme; __scheme.reserve(8);
    QByteArray __userInfo; __userInfo.reserve(32);
    QByteArray __host; __host.reserve(32);
    int __port = -1;

    QByteArray __path; __path.reserve(32);
    QByteArray __query; __query.reserve(64);
    QByteArray __fragment; __fragment.reserve(32);

    char *pptr = (char *) encodedOriginal.data();
    char **ptr = &pptr;

#if defined (Q4URL_DEBUG)
    qDebug("QUrlPrivate::parse(), parsing \"%s\"", pptr);
#endif

    // optional scheme
    char *ptrBackup = *ptr;
    if (parser._scheme(ptr, &__scheme)) {
        char ch = *((*ptr)++);
        if (ch != ':') {
            *ptr = ptrBackup;
            __scheme.clear();
        }
    }

    // hierpart, fails on syntax error
    if (!parser._hierPart(ptr, &__userInfo, &__host, &__port, &__path)) {
        that->isValid = false;
        that->isValidated = true;
        that->isParsed = true;
        return;
    }

    // optional query
    char ch = *((*ptr)++);
    if (ch == '?' && parser._query(ptr, &__query))
        ch = *((*ptr)++);

    // optional fragment
    if (ch == '#') {
        (void) parser._fragment(ptr, &__fragment);
    } else if (ch != '\0') {
        that->isValid = false;
        that->isValidated = true;
        that->isParsed = true;
#if defined (Q4URL_DEBUG)
        qDebug("QUrlPrivate::parse(), unrecognized: %c%s", ch, *ptr);
#endif
        return;
    }

    // when doing lazy validation, this function is called after
    // encodedOriginal has been constructed from the individual parts,
    // only to see if the constructed URL can be parsed. in that case,
    // parse() is called in ParseOnly mode; we don't want to set all
    // the members over again.
    if (parseOptions == ParseAndSet) {
        that->scheme = QUrl::fromPercentageEncodingThenUtf8(__scheme).toLower();
        that->setUserInfo(QUrl::fromPercentageEncodingThenUtf8(__userInfo));
        that->host = QUrl::fromPercentageEncodingThenUtf8(__host).toLower();
        that->port = __port;
        that->path = QUrl::fromPercentageEncodingThenUtf8(__path);
        that->query = QUrl::fromPercentageEncodingThenUtf8(__query).ascii();
        that->fragment = QUrl::fromPercentageEncodingThenUtf8(__fragment).ascii();
    }

    that->isValid = true;
    that->isValidated = true;
    that->isParsed = true;

#if defined (Q4URL_DEBUG)
    qDebug("QUrl::setUrl(), scheme = %s", QUrl::fromPercentageEncodingThenUtf8(__scheme).toLower().latin1());
    qDebug("QUrl::setUrl(), userInfo = %s", QUrl::fromPercentageEncodingThenUtf8(__userInfo).latin1());
    qDebug("QUrl::setUrl(), host = %s", QUrl::fromPercentageEncodingThenUtf8(__host).toLower().latin1());
    qDebug("QUrl::setUrl(), port = %i", __port);
    qDebug("QUrl::setUrl(), path = %s", QUrl::fromPercentageEncodingThenUtf8(__path).latin1());
    qDebug("QUrl::setUrl(), query = %s", QUrl::fromPercentageEncodingThenUtf8(__query).ascii());
    qDebug("QUrl::setUrl(), fragment = %s", QUrl::fromPercentageEncodingThenUtf8(__fragment).latin1());
#endif
}

void QUrlPrivate::clear()
{
    *this = QUrlPrivate();
}

QByteArray QUrlPrivate::toEncoded() const
{
    if (!isParsed) parse();

    QByteArray url;

    if (!scheme.isEmpty()) {
        url += scheme.ascii();
        url += ":";
    }
    if (!authority().isEmpty()) {
        url += "//";

        if (!userName.isEmpty()) {
            url += QUrl::toUtf8ThenPercentageEncoding(userName, ":");
            if (!password.isEmpty())
                url += ":" + QUrl::toUtf8ThenPercentageEncoding(password, ":");
            url += "@";
        }

        // IDNA / rfc3490 describes these four delimiters used for
        // separating labels in unicode international domain
        // names.
        const unsigned short delimiters[] = {0x2e, 0x3002, 0xff0e, 0xff61, 0};
        QTextCodec *codec = QTextCodec::codecForName("Punycode");

        if (!codec)
            return QByteArray();

        QStringList labels = host.split(QRegExp("[" + QString::fromUtf16(delimiters) + "]"));
        for (int i = 0; i < labels.count(); ++i) {
            if (i != 0) url += '.';

            QString label = QUnicodeTables::normalize(labels.at(i), QUnicodeTables::NormalizationMode_KC, QChar::Unicode_3_1);
            url += codec->fromUnicode(label);
        }

        if (port != -1) {
            url += ":";
            url += QString::number(port).ascii();
        }
    }

    url += QUrl::toUtf8ThenPercentageEncoding(path, " \t");

    if (!query.isEmpty())
        url += "?" + query;
    if (!fragment.isEmpty())
        url += "#" + QUrl::toUtf8ThenPercentageEncoding(fragment, " \t");

    return url;
}

QUrl::QUrl(const QString &url) : d(new QUrlPrivate)
{
    if (!url.isEmpty())
        setUrl(url);
}

QUrl::QUrl(const QUrl &copy) : d(copy.d)
{
    ++d->ref;
}

QUrl::~QUrl()
{
    if (!--d->ref)
        delete d;
}

bool QUrl::isValid() const
{
    if (!d->isParsed) d->parse();
    if (!d->isValidated) d->validate();

    return d->isValid;
}

/*!
    Resets the content of the QUrl. After calling this function, the
    QUrl is equal to one that has been constructed with the default
    empty constructor.
*/
void QUrl::clear()
{
    detach();
    d->clear();
}

/*!
    Constructs a URL by parsing the contents of \a url.

    \a url is assumed to be unencoded in unicode format. Before it is
    parsed, the string is converted to UTF-8, and all non ASCII
    characters are then percentage encoded.

    Calling isValid() will tell whether or not a valid URL was
    constructed.

    \sa setEncodedUrl()
*/
void QUrl::setUrl(const QString &url)
{
    setEncodedUrl(QUrl::toUtf8ThenPercentageEncoding(url, " \t"));
}

/*!
    Constructs a URL by parsing the contents of \a url.

    \a encodedUrl is assumed to be a URL string in percentage encoded
    form, containing only ASCII characters.

    Calling isValid() will tell whether or not a valid URL was
    constructed.

    \sa setUrl()
*/
void QUrl::setEncodedUrl(const QByteArray &encodedUrl)
{
    clear();
    d->encodedOriginal = encodedUrl;
}

/*!
    Sets the scheme of the URL to \a scheme. \a scheme is trimmed and
    converted to lower case before it is stored. As a scheme can only
    contain ASCII characters, no conversion or encoding is done on the
    input.

    The scheme describes the type (or protocol) of the URL. It's
    represented by optional first ASCII characters of the URL, and is
    followed by a ':'. The following example shows a URL where the
    scheme is "http":

    \code
        http://www.trolltech.com/
        \__/
        scheme
    \endcode

    The scheme can also be empty, in which case the URL is interpreted
    as \e relative.

    \sa scheme(), isRelative()
*/
void QUrl::setScheme(const QString &scheme)
{
    if (!d->isParsed) d->parse();
    detach();
    d->isValidated = false;

    d->scheme = scheme.toLower().trimmed();
}

/*!
    Returns the scheme of the URL. If QString::null is returned, this
    means the scheme is undefined and the URL is then relative.

    \sa setScheme(), isRelative()
*/
QString QUrl::scheme() const
{
    if (!d->isParsed) d->parse();

    return d->scheme;
}

/*!
    Sets the authority of the URL to \a authority.

    The authority of a URL is the combination of user info, a host
    name and a port. All of these elements are optional; an empty
    authority is perfectly valid.

    The user info and host are separated by a '@', and the host and
    port are separated by a ':'. If the user info is empty, the '@'
    must be omitted; although a stray ':' is permitted if the port is
    empty.

    The following example shows a valid authority string:

    \code
        ftp://tray:5uQQo_f@ftp.example.com:2021
              \__________/ \_____________/ \__/
                user info       host       port
              \_______________________________/
                          authority
    \endcode
*/
void QUrl::setAuthority(const QString &authority)
{
    if (!d->isParsed) d->parse();
    detach();
    d->isValidated = false;

    d->setAuthority(authority);
}

/*!
    Returns the authority of the URL if it is defined; otherwise
    QString::null is returned.

    \sa setAuthority()
*/
QString QUrl::authority() const
{
    if (!d->isParsed) d->parse();

    return d->authority();
}

/*!
    Sets the user info of the URL to \a userInfo. The user info is an
    optional part of the authority of the URL, as described in
    setAuthority().

    The user info consists of a user name and optionally a password,
    separated by a ':'. If the password is empty, the colon must be
    omitted. The following example shows a valid user info string:

    \code
        ftp://tray:5uQQo_f@ftp.example.com/
              \__/ \_____/
              user password
              \___________/
                user info
    \encode

    \sa userInfo(), setUserName(), setPassword(), setAuthority()
*/
void QUrl::setUserInfo(const QString &userInfo)
{
    if (!d->isParsed) d->parse();
    detach();
    d->isValidated = false;

    d->setUserInfo(userInfo.trimmed());
}

/*!
    Returns the user info of the URL, or QString::null if the user
    info is undefined.
*/
QString QUrl::userInfo() const
{
    if (!d->isParsed) d->parse();

    return d->userInfo();
}

/*!
    Sets the user name of the URL. The user name is part of the user
    info element in the authority of the URL, as described in
    setUserInfo().

    \sa userName(), setUserInfo()
*/
void QUrl::setUserName(const QString &userName)
{
    if (!d->isParsed) d->parse();
    detach();
    d->isValidated = false;

    d->userName = userName;
}

/*!
    Returns the user name of the URL if it is defined; otherwise
    QString::null is returned.

    \sa setUserName()
*/
QString QUrl::userName() const
{
    if (!d->isParsed) d->parse();

    return d->userName;
}

/*!
    Sets the password of the URL. The password is part of the user
    info element in the authority of the URL, as described in
    setUserInfo().

    \a password is converted to UTF-8, and then all non-ASCII
    characters are percentage encoded before it is stored.

    \sa password(), setUserInfo()
*/
void QUrl::setPassword(const QString &password)
{
    if (!d->isParsed) d->parse();
    detach();
    d->isValidated = false;

    d->password = password;
}

/*!
    Returns the password of the URL if it is defined; otherwise
    QString::null is returned.

    \sa setUserName()
*/
QString QUrl::password() const
{
    if (!d->isParsed) d->parse();

    return d->password;
}

/*!
    Sets the host of the URL to \a host. The host is part of the
    authority.

    The host name is trimmed and converted to lower case.

    \sa host(), setAuthority()
*/
void QUrl::setHost(const QString &host)
{
    if (!d->isParsed) d->parse();
    detach();
    d->isValidated = false;

    d->host = host.toLower().trimmed();
}

/*!
    Returns the host of the URL if it is defined; otherwise
    QString::null is returned.
*/
QString QUrl::host() const
{
    if (!d->isParsed) d->parse();

    return d->host;
}

/*!
    Sets the port of the URL to \a port. The port is part of the
    authority of the URL, as described in setAuthority().

    If \a port is negative or has a value larger than 65535, it will
    be set to -1 and isValid() will return false.
*/
void QUrl::setPort(int port)
{
    if (!d->isParsed) d->parse();
    detach();
    d->isValidated = false;

    if (port < 0 || port > 65535) {
        qWarning("QUrl::setPort() called with out of range port");
        port = -1;
        return;
    }

    d->port = port;
}

/*!
    Returns the port of the URL, or -1 if the port is undefined.
*/
int QUrl::port() const
{
    if (!d->isParsed) d->parse();

    return d->port;
}

/*!
    Sets the path of the URL to \a path. The path is the part of the
    URL that comes after the authority but before the query string.

    \code
        ftp://ftp.example.com/pub/something/
                             \_____________/
                                   path
    \endcode

    For non-hierarchical schemes, the path will be everything
    following the scheme declaration, as in the following example:

    \code
        mailto:postmaster@example.com
               \____________________/
                       path
    \endcode

    \sa path()
*/
void QUrl::setPath(const QString &path)
{
    if (!d->isParsed) d->parse();
    detach();
    d->isValidated = false;

    d->path = path;
}

/*!
    Returns the path of the URL.

    \sa setPath()
*/
QString QUrl::path() const
{
    if (!d->isParsed) d->parse();

    return d->path;
}

/*!
    Sets the characters used for delimiting between keys and values,
    and between key-value pairs in the URL's query string. The default
    value delimiter is '=' and the default pair delimiter is '&'.

    \code
        http://www.example.com/cgi-bin/drawgraph.cgi?type=pie&color=green
                                                     \__________________/
                                                        query string
    \endcode

    \a valueDelimiter will be used for separating keys from values,
    and \a pairDelimiter will be used to separate key-value pairs.
    Any occurrances of these delimiting characters in the encoded
    representation of the keys and values of the query string are
    percentage encoded when calling toEncoded().

    If \a valueDelimiter is set to '-' and \a pairDelimiter is '/',
    the above query string would instead be represented like this:

    \code
        http://www.example.com/cgi-bin/drawgraph.cgi?type-pie/color-green
    \endcode

    Calling this function does not change the delimiters of the
    current query string. It only affects queryItems(),
    setQueryItems() and addQueryItems().
*/
void QUrl::setQueryDelimiters(char valueDelimiter, char pairDelimiter)
{
    detach();

    d->valueDelimiter = valueDelimiter;
    d->pairDelimiter = pairDelimiter;
}

/*!
    Returns the character used to delimit between key-value pairs in
    the query string of the URL.
*/
char QUrl::queryPairDelimiter() const
{
    return d->pairDelimiter;
}

/*!
    Returns the character used to delimit between keys and values in
    the query string of the URL.
*/
char QUrl::queryValueDelimiter() const
{
    return d->valueDelimiter;
}

/*!
    Sets the query string of the URL to \a query. The string is
    inserted as-is, and no further encoding is performed when calling
    toEncoded().

    This function is useful if you need to pass a query string that
    does not fit into the key-value pattern, or that uses a different
    scheme for encoding special characters than what is suggested by
    QUrl.
*/
void QUrl::setEncodedQuery(const QByteArray &query)
{
    if (!d->isParsed) d->parse();
    detach();
    d->isValidated = false;

    d->query = query;
}

/*!
    Sets the query string of the URL to an encoded version of \a
    query. The contents of \a query are converted to a string
    internally, each pair delimited by the character returned by
    pairDelimiter(), and the key and value are delimited by
    valueDelimiter().

    \sa setQueryDelimiters(), queryItems()
*/
void QUrl::setQueryItems(const QMap<QString, QString> &query)
{
    if (!d->isParsed) d->parse();
    detach();

    QString alsoEncode = QLatin1String(" \t#");
    alsoEncode += d->valueDelimiter;
    alsoEncode += d->pairDelimiter;

    QByteArray queryTmp;
    QMap<QString, QString>::ConstIterator i = query.constBegin();
    bool first = true;
    while (i != query.constEnd()) {
        if (first) first = false;
        else queryTmp += d->pairDelimiter;

        queryTmp += QUrl::toUtf8ThenPercentageEncoding(i.key(), alsoEncode.ascii());
        queryTmp += d->valueDelimiter;
        queryTmp += QUrl::toUtf8ThenPercentageEncoding(i.value(), alsoEncode.ascii());
        ++i;
    }

    d->query = queryTmp;
}

/*!
    Returns the query string of the URL, represented as a QMap.

    \sa setQueryItems(), setEncodedQuery()
*/
QMap<QString, QString> QUrl::queryItems() const
{
    if (!d->isParsed) d->parse();

    QMap<QString, QString> itemMap;

    QList<QByteArray> items = d->query.split(d->pairDelimiter);
    for (int i = 0; i < items.count(); ++i) {
        QList<QByteArray> keyValuePair = items.at(i).split(d->valueDelimiter);
        itemMap.insert(QUrl::fromPercentageEncodingThenUtf8(keyValuePair.at(0)),
                       QUrl::fromPercentageEncodingThenUtf8(keyValuePair.at(1)));
    }

    return itemMap;
}

/*!
    Inserts the pair \a key = \a value into the query string of the
    URL.
*/
void QUrl::addQueryItem(const QString &key, const QString &value)
{
    if (!d->isParsed) d->parse();
    detach();

    QString alsoEncode = QLatin1String(" \t#");
    alsoEncode += d->valueDelimiter;
    alsoEncode += d->pairDelimiter;

    if (!d->query.isEmpty())
        d->query += d->pairDelimiter;

    d->query += QUrl::toUtf8ThenPercentageEncoding(key, alsoEncode.ascii());
    d->query += d->valueDelimiter;
    d->query += QUrl::toUtf8ThenPercentageEncoding(value, alsoEncode.ascii());
}

/*!
    Removes the query string pair whose key is equal to \a key from
    the URL.
*/
void QUrl::removeQueryItem(const QString &key)
{
    if (!d->isParsed) d->parse();
    detach();

    QMap<QString, QString> items = queryItems();
    if (!items.contains(key))
        return;

    items.remove(key);
    setQueryItems(items);
}

/*!
    Returns the query string of the URL in percentage encoded form.
*/
QByteArray QUrl::encodedQuery() const
{
    if (!d->isParsed) d->parse();

    return d->query;
}

/*!
    Sets the fragment of the URL to \a fragment. The fragment is the
    last part of the URL, represented by a '#' followed by a string of
    characters. It is typically used in HTTP for referring to a
    certain link or point on a page:

    \code
        http://www.example.net/faq.html?#question13
                                         \________/
                                          fragment
    \endcode

    The fragment is sometimes also referred to as the URL "reference".

    \sa fragment()
*/
void QUrl::setFragment(const QString &fragment)
{
    if (!d->isParsed) d->parse();
    detach();
    d->isValidated = false;

    d->fragment = QUrl::toUtf8ThenPercentageEncoding(fragment);
}

/*!
    Returns the fragment of the URL.

    \sa setFragment()
*/
QString QUrl::fragment() const
{
    if (!d->isParsed) d->parse();

    return d->fragment;
}

/*!
    Returns the result of the merge of this URL with \a relative. This
    URL is used as a base to convert \a relative to an absolute URL.

    If \a relative is not a relative URL, this function will return \a
    relative directly. Otherwise, the paths of the two URLs are
    merged, and the new URL returned has the scheme and authority of
    the base URL, but with the merged path, as in the following
    example:

    \code
         QUrl baseUrl("http://www.trolltech.com/support");
         QUrl relativeUrl("../products/solutions");
         qDebug(baseUrl.resolved(relativeUrl).toString());

         // qDebug prints "http://www.trolltech.com/products/solutions"
    \encode

    Calling resolved() with ".." returns a QUrl whose directory is
    one level higher than the original. Similarly, calling resolved()
    with "../.." removes two levels from the path. If \a relative is
    "/", the path becomes "/".

    The algorithm for this merge is described

    \sa isRelative()
*/
QUrl QUrl::resolved(const QUrl &relative) const
{
    if (!d->isParsed) d->parse();

    QUrl r(relative), t;

    // be non strict and allow scheme in relative url
    if (r.scheme() == d->scheme)
        r.setScheme(QString::null);

    if (!r.scheme().isEmpty()) {
        t.setScheme(r.scheme());
        t.setAuthority(r.authority());
        t.setPath(r.path());
        t.d->removeDotsFromPath();
        t.setEncodedQuery(r.encodedQuery());
    } else {
        if (!r.authority().isEmpty()) {
            t.setAuthority(r.authority());
            t.setPath(r.path());
            t.d->removeDotsFromPath();
            t.setEncodedQuery(r.encodedQuery());
        } else {
            if (r.path().isEmpty()) {
                t.setPath(d->path);
                if (!r.encodedQuery().isEmpty())
                    t.setEncodedQuery(r.encodedQuery());
                else
                    t.setEncodedQuery(d->query);
            } else {
                if (r.path().startsWith("/")) {
                    t.setPath(r.path());
                    t.d->removeDotsFromPath();
                } else {
                    t.setPath(d->mergePaths(r.path()));
                    t.d->removeDotsFromPath();
                }
                t.setEncodedQuery(r.encodedQuery());
            }
            t.setAuthority(d->authority());
        }
        t.setScheme(d->scheme);
    }
    t.setFragment(r.fragment());

    return t;
}

/*!
    Returns true if the URL is relative; otherwise returns false. A
    URL is relative if its scheme is undefined; this function is
    therefore equivalent to calling scheme().isEmpty().
*/
bool QUrl::isRelative() const
{
    if (!d->isParsed) d->parse();

    return d->scheme.isEmpty();
}

/*!
    Returns the human-displayable string representation of the
    URL. The output can be customized by passing flags with \a
    formattingOptions.

    \sa FormattingOptions, toEncoded()
*/
QString QUrl::toString(int formattingOptions) const
{
    if (!d->isParsed) d->parse();

    QString url;

    if (!(formattingOptions & QUrl::RemoveScheme) && !d->scheme.isEmpty())
        url += d->scheme + ":";
    if ((formattingOptions & QUrl::RemoveAuthority) != QUrl::RemoveAuthority) {
        QString tmp = d->authority(formattingOptions);
        if (!tmp.isEmpty()) {
            if (!url.isEmpty())
                url += "//";
            url += tmp;
        }
    }
    if (!(formattingOptions & QUrl::RemovePath))
	url += d->path;
    if (!(formattingOptions & QUrl::RemoveQuery) && !d->query.isEmpty())
        url += "?" + d->query;
    if (!(formattingOptions & QUrl::RemoveFragment) && !d->fragment.isEmpty())
        url += "#" + d->fragment;

    return url;
}

/*!
    Returns the encoded representation of the URL if it's valid;
    otherwise QByteArray::null is returned.

    The user info, path and fragment are all converted to UTF-8, and
    all non-ASCII characters are then percentageencoded. The host name
    is encoded using Punycode.
*/
QByteArray QUrl::toEncoded() const
{
    return d->toEncoded();
}

/* ###

*/
QUrl QUrl::fromEncoded(const QByteArray &input)
{
    QUrl tmp;
    tmp.setEncodedUrl(input);
    return tmp;
}

/*!
    Returns a decoded copy of \a input. \a input is first decoded from
    percentageencoding, then converted from UTF-8 to unicode.
*/
QString QUrl::fromPercentageEncodingThenUtf8(const QByteArray &input)
{
    QVarLengthArray<char> tmp(input.size());

    char *data = tmp.data();
    const char *inputPtr = input.data();

    int i = 0;
    int len = input.count();
    int a, b;
    char c;
    while (i < len) {
        c = inputPtr[i];
        if (c == '%' && i + 2 < len) {
            a = inputPtr[++i];
            b = inputPtr[++i];

            if (a >= '0' && a <= '9') a -= '0';
            else if (a >= 'a' && a <= 'f') a = a - 'a' + 10;
            else if (a >= 'A' && a <= 'F') a = a - 'A' + 10;

            if (b >= '0' && b <= '9') b -= '0';
            else if (b >= 'a' && b <= 'f') b  = b - 'a' + 10;
            else if (b >= 'A' && b <= 'F') b  = b - 'A' + 10;

            *data++ = (char)((a << 4) | b);
        } else {
            *data++ = c;
        }

        ++i;
    }

    *data = '\0';

#if defined Q4URL_DEBUG
    qDebug("QUrl::fromPercentageEncodingThenUtf8(\"%s\") == \"%s\"", input.data(), QString::fromUtf8(tmp.data()).latin1());
#endif

    return QString::fromUtf8(tmp.data());
}

inline bool q_strchr(const char str[], char chr)
{
    const char *ptr = str;
    char c;
    while ((c = *ptr++))
        if (c == chr)
            return true;
    return false;
}

static const char hexnumbers[] = "0123456789ABCDEF";
inline char toHex(char c)
{
    return hexnumbers[c & 0xf];
}

/*!
    Returns an encoded copy of \a input. \a input is first converted
    to UTF-8, and then all non-ASCII characters, including any
    characters in \a alsoEncode, are percentage encoded.
*/
QByteArray QUrl::toUtf8ThenPercentageEncoding(const QString &input, const char alsoEncode[])
{
    QByteArray tmp = input.toUtf8();
    QVarLengthArray<char> output(tmp.size() * 3);

    int len = tmp.count();
    char *data = output.data();
    const char *inputData = tmp.constData();
    int length = 0;

    if (alsoEncode[0] == '\0') {
        for (int i = 0; i < len; ++i) {
            unsigned char c = *inputData++;
            if (c < 0x80 && c != '%') {
                data[length++] = c;
            } else {
                data[length++] = '%';
                data[length++] = toHex((c & 0xf0) >> 4);
                data[length++] = toHex(c & 0xf);
            }
        }
    } else {
        for (int i = 0; i < len; ++i) {
            unsigned char c = *inputData++;
            if (c < 0x80 && c != '%' && !q_strchr(alsoEncode, c)) {
                data[length++] = c;
            } else {
                data[length++] = '%';
                data[length++] = toHex((c & 0xf0) >> 4);
                data[length++] = toHex(c & 0xf);
            }
        }
    }

#if defined Q4URL_DEBUG
    qDebug("QUrl::toUtf8ThenPercentageEncoding(\"%s\") == \"%s\"", input.latin1(), QByteArray(output.data(), length).data());
#endif

    return QByteArray(output.data(), length);
}

bool QUrl::operator ==(const QUrl &url) const
{
    return toString(None) == url.toString(None);
}

bool QUrl::operator !=(const QUrl &url) const
{
    return !(*this == url);
}

QUrl &QUrl::operator =(const QUrl &copy)
{
    QUrlPrivate *x = new QUrlPrivate;
    *x = *copy.d;
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref)
        delete x;

    return *this;
}

void QUrl::detach()
{
    if (d->ref != 1) {
        QUrlPrivate *x = new QUrlPrivate;
        *x = *d;
        x = qAtomicSetPtr(&d, x);
        if (!--x->ref)
            delete x;
    }
}

/*!
    Returns true if this URL represents a local file. The file is
    local if the scheme is "file", and the host is either empty or
    equal to the current host name.
*/
bool QUrl::isLocalFile() const
{
    if (!d->isParsed) d->parse();

    return (d->scheme.isEmpty() || d->scheme == QLatin1String("file"))
	   && (d->host.isEmpty() || d->host == QLatin1String("localhost")
	   || d->host == qGetHostName().toLower());
}

/*!
    Returns a QUrl representation of \a localFile, interpreted as a
    local file.


*/
QUrl QUrl::fromLocalFile(const QString &localFile) //### set host name option ??
{
    QUrl url;
    url.setScheme(QLatin1String("file"));
    QString deslashified = localFile;
    deslashified.replace('\\', '/');

    // magic for drives on windows
    if (deslashified.length() > 1 && deslashified.at(1) == ':' && deslashified.at(0) != '/')
        url.setPath(QLatin1String("/") + deslashified);
    else
        url.setPath(deslashified);

    return url;
}

/*!
    Returns the path of this URL formatted as a local file path.
*/
QString QUrl::toLocalFile() const
{
    if (!d->isParsed) d->parse();

    QString tmp;
    if (isLocalFile()) {
        tmp = d->path;
        // magic for drives on windows
        if (tmp.length() > 2 && tmp.at(0) == '/' && tmp.at(2) == ':')
            tmp.remove(0, 1);
    }

    return tmp;
}

/*!
    Returns true if this URL is a parent of \a childUrl. \a childUrl is a child
    of this URL if the two URLs share the same scheme and authority,
    and this URL's path is a parent of the path of \a childUrl.

//### currentlt only when scheme and authority are the same

*/
bool QUrl::isParentOf(const QUrl &childUrl) const
{
    if (!d->isParsed) d->parse();

    QString childPath = childUrl.path();

    return ((childUrl.scheme().isEmpty() || d->scheme == childUrl.scheme())
            && (childUrl.authority().isEmpty() || d->authority() == childUrl.authority())
            &&  childPath.startsWith(d->path)
            && ((d->path.endsWith("/") && childPath.length() > d->path.length())
                || (!d->path.endsWith("/")
                    && childPath.length() > d->path.length() && childPath.at(d->path.length()) == '/')));
}
