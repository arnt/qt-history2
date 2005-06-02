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

/*! \class QUrl

    \brief The QUrl class provides a convenient interface for working
    with URLs.

    \reentrant
    \ingroup io
    \ingroup misc
    \mainclass

    It can parse and construct URLs in both encoded and unencoded
    form. QUrl also has support for internationalized domain names
    (IDNs).

    The most common way to use QUrl is to initialize it via the
    constructor by passing a QString. Otherwise, setUrl() and
    setEncodedUrl() can also be used.

    A URL can also be constructed piece by piece by calling
    setScheme(), setUserName(), setPassword(), setHost(), setPort(),
    setPath(), setEncodedQuery() and setFragment(). Some convenience
    functions are also available: setAuthority() sets the user name,
    password, host and port. setUserInfo() sets the user name and
    password at once.

    Call isValid() to check if the URL is valid. This can be done at
    any point during the constructing of a URL.

    Constructing a query is particularily convenient through the use
    of setQueryItems(), addQueryItem() and removeQueryItem(). Use
    setQueryDelimiters() to customize the delimiters used for
    generating the query string.

    For the convenience of generating encoded URL strings or query
    strings, there are two static functions called
    fromPercentEncoding() and toPercentEncoding() which deal with
    percent encoding and decoding of QStrings.

    Calling isRelative() will tell whether or not the URL is
    relative. A relative URL can be resolved by passing it as argument
    to resolved(), which returns an absolute URL. isParentOf() is used
    for determining whether one URL is a parent of another.

    isLocalFile() tells whether the URL represents the path to a local
    file or not. fromLocalFile() constructs a QUrl by parsing a local
    file path. toLocalFile() converts a URL to a local file path.

    The human readable representation of the URL is fetched with
    toString(). This representation is appropriate for displaying a
    URL to a user in unencoded form. The encoded form however, as
    returned by toEncoded(), is for internal use, passing to web
    servers, mail clients and so on.
*/

/*!
    \enum QUrl::FormattingOption

    The formatting options define how the URL is formatted when written out
    as text.

    \value None          The URL is left unchanged.
    \value RemoveScheme  The scheme is removed from the URL.
    \value RemovePassword  Any password in the URL is removed.
    \value RemoveUserInfo  Any user information in the URL is removed.
    \value RemovePort      Any specified port is removed from the URL.
    \value RemoveAuthority
    \value RemovePath   The URL's path is removed, leaving only the scheme,
                        host address, and port (if present).
    \value RemoveQuery  The query part of the URL (following a '?' character)
                        is removed.
    \value RemoveFragment
    \value StripTrailingSlash  The trailing slash is removed if one is present.
*/

#include "qplatformdefs.h"

#include <private/qunicodetables_p.h>
#include <qatomic.h>
#include <qbytearray.h>
#include <qlist.h>
#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qstack.h>
#include <qvarlengtharray.h>
#include <qdebug.h>
#include "qurl.h"

#if defined QT3_SUPPORT
#include <qfileinfo.h>
#endif

//#define QURL_DEBUG

// needed by the punycode encoder/decoder
#define Q_MAXINT ((uint)((uint)(-1)>>1))
static const uint base = 36;
static const uint tmin = 1;
static const uint tmax = 26;
static const uint skew = 38;
static const uint damp = 700;
static const uint initial_bias = 72;
static const uint initial_n = 128;

#define QURL_SETFLAG(a, b) { (a) |= (b); }
#define QURL_UNSETFLAG(a, b) { (a) &= ~(b); }
#define QURL_HASFLAG(a, b) (((a) & (b)) == (b))

class QUrlPrivate
{
public:
    QUrlPrivate();
    QUrlPrivate(const QUrlPrivate &other);

    bool setUrl(const QString &url);

    QString authority(QUrl::FormattingOptions options = QUrl::None) const;
    void setAuthority(const QString &auth);
    void setUserInfo(const QString &userInfo);
    QString userInfo(QUrl::FormattingOptions options = QUrl::None) const;

    QString mergePaths(const QString &relativePath) const;

    static QString removeDotsFromPath(const QString &path);

    enum ParseOptions {
        ParseAndSet,
        ParseOnly
    };

    void validate() const;
    void parse(ParseOptions parseOptions = ParseAndSet) const;
    void clear();

    QByteArray toEncoded(QUrl::FormattingOptions options = QUrl::None) const;

    QAtomic ref;

    QString scheme;
    QString userName;
    QString password;
    QString host;
    int port;
    QString path;
    QByteArray query;
    QString fragment;

    QByteArray encodedOriginal;

    bool isValid;

    char valueDelimiter;
    char pairDelimiter;

    enum State {
        Parsed = 0x1,
        Validated = 0x2,
        Normalized = 0x4
    };
    int stateFlags;

    QByteArray encodedNormalized;
    const QByteArray & normalized();
};

static bool QT_FASTCALL _char(char **ptr, char expected)
{
    if (*((*ptr)) == expected) {
        ++(*ptr);
        return true;
    }

    return false;
}

static bool QT_FASTCALL _HEXDIG(char **ptr, char *dig)
{
    char ch = **ptr;
    if ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')) {
        *dig = ch;
        ++(*ptr);
        return true;
    }

    return false;
}

// pct-encoded = "%" HEXDIG HEXDIG
static bool QT_FASTCALL _pctEncoded(char **ptr, char pct[])
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

#if 0
// gen-delims  = ":" / "/" / "?" / "#" / "[" / "]" / "@"
static bool QT_FASTCALL _genDelims(char **ptr, char *c)
{
    char ch = **ptr;
    switch (ch) {
    case ':': case '/': case '?': case '#':
    case '[': case ']': case '@':
        *c = ch;
        ++(*ptr);
        return true;
    default:
        return false;
    }
}
#endif

// sub-delims  = "!" / "$" / "&" / "'" / "(" / ")"
//             / "*" / "+" / "," / ";" / "="
static bool QT_FASTCALL _subDelims(char **ptr, char *c)
{
    char ch = **ptr;
    switch (ch) {
    case '!': case '$': case '&': case '\'':
    case '(': case ')': case '*': case '+':
    case ',': case ';': case '=':
        *c = ch;
        ++(*ptr);
        return true;
    default:
        return false;
    }
}

static bool QT_FASTCALL _ALPHA_(char **ptr, char *c)
{
    char ch = **ptr;
    if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
        *c = ch;
        ++(*ptr);
        return true;
    }

    return false;
}

static bool QT_FASTCALL _DIGIT_(char **ptr, char *c)
{
    char ch = **ptr;
    if (ch >= '0' && ch <= '9') {
        *c = ch;
        ++(*ptr);
        return true;
    }

    return false;
}

// unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~"
static bool QT_FASTCALL _unreserved(char **ptr, char *c)
{
    if (_ALPHA_(ptr, c) || _DIGIT_(ptr, c))
        return true;

    char ch = **ptr;
    switch (ch) {
    case '-': case '.': case '_': case '~':
        *c = ch;
        ++(*ptr);
        return true;
    default:
        return false;
    }
}

// scheme      = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
static bool QT_FASTCALL _scheme(char **ptr, QByteArray *scheme)
{
    bool first = true;

    for (;;) {
        char ch = **ptr;
        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
            *scheme += ch;
        } else if (!first && ((ch >= '0' && ch <= '9') || ch == '+' || ch == '-' || ch == '.')) {
            *scheme += ch;
        } else {
            break;
        }

        ++(*ptr);
        first = false;
    }

    return true;
}

// IPvFuture  = "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )
static bool QT_FASTCALL _IPvFuture(char **ptr, QByteArray *host)
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
static bool QT_FASTCALL _h16(char **ptr, QByteArray *c)
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
static bool QT_FASTCALL _decOctet(char **ptr, QByteArray *octet)
{
    char c1 = **ptr;

    if (c1 < '0' || c1 > '9')
	return false;

    *octet += c1;

    ++(*ptr);

    if (c1 == '0')
        return true;

    char c2 = **ptr;

    if (c2 < '0' || c2 > '9')
        return true;

    *octet += c2;

    ++(*ptr);

    char c3 = **ptr;
    if (c3 < '0' || c3 > '9')
        return true;

    *octet += c3;

    // If there is a three digit number larger than 255, reject the
    // whole token.
    if (c1 >= '2' && c2 >= '5' && c3 > '5')
        return false;

    ++(*ptr);

    return true;
}

// IPv4address = dec-octet "." dec-octet "." dec-octet "." dec-octet
static bool QT_FASTCALL _IPv4Address(char **ptr, QByteArray *c)
{
    char *ptrBackup = *ptr;
    QByteArray tmp1; tmp1.reserve(32);

    if (!_decOctet(ptr, &tmp1))
        return false;

    for (int i = 0; i < 3; ++i) {
        if (*((*ptr)++) != '.') {
            *ptr = ptrBackup;
            return false;
        }

        tmp1 += '.';

        if (!_decOctet(ptr, &tmp1))
            return false;
    }

    *c += tmp1;

    return true;
}

// ls32        = ( h16 ":" h16 ) / IPv4address
//             ; least-significant 32 bits of address
static bool QT_FASTCALL _ls32(char **ptr, QByteArray *c)
{
    char *ptrBackup = *ptr;
    QByteArray tmp1;
    QByteArray tmp2;
    if (_h16(ptr, &tmp1) && _char(ptr, ':') && _h16(ptr, &tmp2)) {
        *c += tmp1;
        *c += ':';
        *c += tmp2;
        return true;
    }

    *ptr = ptrBackup;
    return _IPv4Address(ptr, c);
}

// IPv6address =                            6( h16 ":" ) ls32 // case 1
//             /                       "::" 5( h16 ":" ) ls32 // case 2
//             / [               h16 ] "::" 4( h16 ":" ) ls32 // case 3
//             / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32 // case 4
//             / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32 // case 5
//             / [ *3( h16 ":" ) h16 ] "::"    h16 ":"   ls32 // case 6
//             / [ *4( h16 ":" ) h16 ] "::"              ls32 // case 7
//             / [ *5( h16 ":" ) h16 ] "::"              h16  // case 8
//             / [ *6( h16 ":" ) h16 ] "::"                   // case 9
static bool QT_FASTCALL _IPv6Address(char **ptr, QByteArray *host)
{
    char *ptrBackup = *ptr;

    QByteArray tmp;

    // count of (h16 ":") to the left of and including ::
    int leftHexColons = 0;
    // count of (h16 ":") to the right of ::
    int rightHexColons = 0;

    // first count the number of (h16 ":") on the left of ::
    while (_h16(ptr, &tmp)) {

	// an h16 not followed by a colon is considered an
	// error.
        if (!_char(ptr, ':')) {
	    *ptr = ptrBackup;
	    return false;
	}
	tmp += ':';
	++leftHexColons;

	// check for case 1, the only time when there can be no ::
	if (leftHexColons == 6 && _ls32(ptr, &tmp)) {
	    *host += tmp;
	    return true;
	}
    }

    // check for case 2 where the address starts with a :
    if (leftHexColons == 0 && _char(ptr, ':'))
	tmp += ':';


    // check for the second colon in ::
    if (!_char(ptr, ':')) {
	*ptr = ptrBackup;
	return false;
    }
    tmp += ':';

    int canBeCase = -1;
    bool ls32WasRead = false;

    QByteArray tmp2;
    char *tmpBackup = *ptr;

    // count the number of (h16 ":") on the right of ::
    for (;;) {
	tmpBackup = *ptr;
        if (!_h16(ptr, &tmp2)) {
	    if (!_ls32(ptr, &tmp)) {
		if (rightHexColons != 0) {
		    *ptr = ptrBackup;
		    return false;
		}

		// the address ended with :: (case 9)
		// only valid if 1 <= leftHexColons <= 7
		canBeCase = 9;
	    } else {
		ls32WasRead = true;
	    }
	    break;
	}
	++rightHexColons;
	if (!_char(ptr, ':')) {
	    // no colon could mean that what was read as an h16
	    // was in fact the first part of an ls32. we backtrack
	    // and retry.
	    char *pb = *ptr;
	    *ptr = tmpBackup;
	    if (_ls32(ptr, &tmp)) {
		ls32WasRead = true;
		--rightHexColons;
	    } else {
		*ptr = pb;
		// address ends with only 1 h16 after :: (case 8)
		if (rightHexColons == 1)
		    canBeCase = 8;
		tmp += tmp2;
	    }
	    break;
	}
	tmp += tmp2 + ':';
	tmp2.truncate(0);
    }

    // determine which case it is based on the number of rightHexColons
    if (canBeCase == -1) {

	// check if a ls32 was read. If it wasn't and rightHexColons >= 2 then the
	// last 2 HexColons are in fact a ls32
	if (!ls32WasRead && rightHexColons >= 2)
	    rightHexColons -= 2;

	canBeCase = 7 - rightHexColons;
    }

    // based on the case we need to check that the number of leftHexColons is valid
    if (leftHexColons > (canBeCase - 2)) {
	*ptr = ptrBackup;
	return false;
    }

    *host += tmp;
    return true;
}

// IP-literal = "[" ( IPv6address / IPvFuture  ) "]"
static bool QT_FASTCALL _IPLiteral(char **ptr, QByteArray *host)
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

// reg-name    = *( unreserved / pct-encoded / sub-delims )
static bool QT_FASTCALL _regName(char **ptr, QByteArray *host)
{
    char pctTmp[4];
    for (;;) {
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
static bool QT_FASTCALL _host(char **ptr, QByteArray *host)
{
    return (_IPLiteral(ptr, host) || _IPv4Address(ptr, host) || _regName(ptr, host));
}

// userinfo    = *( unreserved / pct-encoded / sub-delims / ":" )
static bool QT_FASTCALL _userInfo(char **ptr, QByteArray *userInfo)
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
static bool QT_FASTCALL _port(char **ptr, int *port)
{
    bool first = true;

    for (;;) {
        char *ptrBackup = *ptr;
        char ch = *((*ptr)++);
        if (ch < '0' || ch > '9') {
            *ptr = ptrBackup;
            break;
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
static bool QT_FASTCALL _authority(char **ptr, QByteArray *userInfo, QByteArray *host, int *port)
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
static bool QT_FASTCALL _pchar(char **ptr, char pc[])
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
static bool QT_FASTCALL _segment(char **ptr, QByteArray *segment)
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
static bool QT_FASTCALL _segmentNZ(char **ptr, QByteArray *segment)
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
static bool QT_FASTCALL _pathAbEmpty(char **ptr, QByteArray *path)
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
static bool QT_FASTCALL _pathAbs(char **ptr, QByteArray *path)
{
    char *ptrBackup = *ptr;
    char ch = *((*ptr)++);
    if (ch != '/') {
        *ptr = ptrBackup;
        return false;
    }

    *path += '/';

    // we might be able to unnest this to gain some performance.
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

        // we might be able to unnest this to gain some
        // performance.
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
static bool QT_FASTCALL _pathRootless(char **ptr, QByteArray *path)
{
    // we might be able to unnest this to gain some performance.
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

        // we might be able to unnest this to gain some performance.
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
static bool QT_FASTCALL _pathEmpty(char **, QByteArray *path)
{
    path->truncate(0);
    return true;
}

// hier-part   = "//" authority path-abempty
//             / path-abs
//             / path-rootless
//             / path-empty
static bool QT_FASTCALL _hierPart(char **ptr, QByteArray *userInfo, QByteArray *host, int *port, QByteArray *path)
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
static bool QT_FASTCALL _query(char **ptr, QByteArray *query)
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
static bool QT_FASTCALL _fragment(char **ptr, QByteArray *fragment)
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

QUrlPrivate::QUrlPrivate()
{
    ref = 1;
    port = -1;
    isValid = false;
    valueDelimiter = '=';
    pairDelimiter = '&';
    stateFlags = 0;
}

QUrlPrivate::QUrlPrivate(const QUrlPrivate &copy)
    : scheme(copy.scheme),
      userName(copy.userName),
      password(copy.password),
      host(copy.host),
      port(copy.port),
      path(copy.path),
      query(copy.query),
      fragment(copy.fragment),
      encodedOriginal(copy.encodedOriginal),
      isValid(copy.isValid),
      valueDelimiter(copy.valueDelimiter),
      pairDelimiter(copy.pairDelimiter),
      stateFlags(copy.stateFlags),
      encodedNormalized(copy.encodedNormalized)
{ ref = 1; }

QString QUrlPrivate::authority(QUrl::FormattingOptions options) const
{
    if ((options & QUrl::RemoveAuthority) == QUrl::RemoveAuthority)
        return QString();

    QString tmp = userInfo(options);
    if (!tmp.isEmpty()) tmp += QLatin1Char('@');
    tmp += host;
    if (!(options & QUrl::RemovePort) && port != -1)
        tmp += QLatin1Char(':') + QString::number(port);

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

    int userInfoIndex = auth.indexOf(QLatin1Char('@'));
    if (userInfoIndex != -1 && (portIndex == -1 || userInfoIndex < portIndex))
        setUserInfo(auth.left(userInfoIndex));

    int hostIndex = 0;
    if (userInfoIndex != -1)
        hostIndex = userInfoIndex + 1;
    int hostLength = auth.length() - hostIndex;
    if (portIndex != -1)
        hostLength -= (auth.length() - portIndex);

    host = auth.mid(hostIndex, hostLength).trimmed();
}

void QUrlPrivate::setUserInfo(const QString &userInfo)
{
    int delimIndex = userInfo.indexOf(QLatin1Char(':'));
    if (delimIndex == -1) {
        userName = userInfo;
        password.clear();
        return;
    }
    userName = userInfo.left(delimIndex);
    password = userInfo.right(userInfo.length() - delimIndex - 1);
}

QString QUrlPrivate::userInfo(QUrl::FormattingOptions options) const
{
    if ((options & QUrl::RemoveUserInfo) == QUrl::RemoveUserInfo)
        return QString();

    QString tmp;
    tmp += userName;

    if (!(options & QUrl::RemovePassword) && !password.isEmpty())
        tmp += QLatin1Char(':') + password;

    return tmp;
}

/*
    From http://www.ietf.org/rfc/rfc3986.txt, 5.2.3: Merge paths

    Returns a merge of the current path with the relative path passed
    as argument.
*/
QString QUrlPrivate::mergePaths(const QString &relativePath) const
{
    // If the base URI has a defined authority component and an empty
    // path, then return a string consisting of "/" concatenated with
    // the reference's path; otherwise,
    if (!authority().isEmpty() && path.isEmpty())
        return QLatin1Char('/') + relativePath;

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
    From http://www.ietf.org/rfc/rfc3986.txt, 5.2.4: Remove dot segments

    Removes unnecessary ../ and ./ from the path. Used for normalizing
    the URL.
*/
QString QUrlPrivate::removeDotsFromPath(const QString &dottedPath)
{
    // The input buffer is initialized with the now-appended path
    // components and the output buffer is initialized to the empty
    // string.
    QString origPath = dottedPath;
    QString path;
    path.reserve(origPath.length());

    const QLatin1String Dot(".");
    const QLatin1Char Slash('/');
    const QLatin1String DotDot("..");
    const QLatin1String DotSlash("./");
    const QLatin1String SlashDot("/.");
    const QLatin1String DotDotSlash("../");
    const QLatin1String SlashDotSlash("/./");
    const QLatin1String SlashDotDotSlash("/../");
    const QLatin1String SlashDotDot("/..");

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

    return path;
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
        QURL_SETFLAG(that->stateFlags, Validated | Parsed);
        return;
    }

    QByteArray __scheme; __scheme.reserve(8);
    QByteArray __userInfo; __userInfo.reserve(32);
    QByteArray __host; __host.reserve(32);
    int __port = -1;

    QByteArray __path; __path.reserve(32);
    QByteArray __query; __query.reserve(64);
    QByteArray __fragment; __fragment.reserve(32);

    char *pptr = (char *) encodedOriginal.data();
    char **ptr = &pptr;

#if defined (QURL_DEBUG)
    qDebug("QUrlPrivate::parse(), parsing \"%s\"", pptr);
#endif

    // optional scheme
    char *ptrBackup = *ptr;
    if (_scheme(ptr, &__scheme)) {
        char ch = *((*ptr)++);
        if (ch != ':') {
            *ptr = ptrBackup;
            __scheme.clear();
        }
    }

    // hierpart, fails on syntax error
    if (!_hierPart(ptr, &__userInfo, &__host, &__port, &__path)) {
        that->isValid = false;
        QURL_SETFLAG(that->stateFlags, Validated | Parsed);
        return;
    }

    // optional query
    char ch = *((*ptr)++);
    if (ch == '?' && _query(ptr, &__query))
        ch = *((*ptr)++);

    // optional fragment
    if (ch == '#') {
        (void) _fragment(ptr, &__fragment);
    } else if (ch != '\0') {
        that->isValid = false;
        QURL_SETFLAG(that->stateFlags, Validated | Parsed);
#if defined (QURL_DEBUG)
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
        that->scheme = QUrl::fromPercentEncoding(__scheme);
        that->setUserInfo(QUrl::fromPercentEncoding(__userInfo));
        that->host = QUrl::fromPercentEncoding(__host);
        that->port = __port;
        that->path = QUrl::fromPercentEncoding(__path);
        that->query = __query;
        that->fragment = QUrl::fromPercentEncoding(__fragment);
    }

    that->isValid = true;
    QURL_SETFLAG(that->stateFlags, Validated | Parsed);

#if defined (QURL_DEBUG)
    qDebug("QUrl::setUrl(), scheme = %s", QUrl::fromPercentEncoding(__scheme).toLatin1().constData());
    qDebug("QUrl::setUrl(), userInfo = %s", QUrl::fromPercentEncoding(__userInfo).toLatin1().constData());
    qDebug("QUrl::setUrl(), host = %s", QUrl::fromPercentEncoding(__host).toLatin1().constData());
    qDebug("QUrl::setUrl(), port = %i", __port);
    qDebug("QUrl::setUrl(), path = %s", QUrl::fromPercentEncoding(__path).toLatin1().constData());
    qDebug("QUrl::setUrl(), query = %s", __query.constData());
    qDebug("QUrl::setUrl(), fragment = %s", QUrl::fromPercentEncoding(__fragment).toLatin1().constData());
#endif
}

void QUrlPrivate::clear()
{
    scheme.clear();
    userName.clear();
    password.clear();
    host.clear();
    port = -1;
    path.clear();
    query.clear();
    fragment.clear();

    encodedOriginal.clear();

    isValid = false;

    valueDelimiter = '=';
    pairDelimiter = '&';

    QURL_UNSETFLAG(stateFlags, Parsed | Validated);
}

QByteArray QUrlPrivate::toEncoded(QUrl::FormattingOptions options) const
{
    if (!QURL_HASFLAG(stateFlags, Parsed)) parse();

    QByteArray url;

    if (!(options & QUrl::RemoveScheme) && !scheme.isEmpty()) {
        url += scheme.toAscii();
        url += ":";
    }
    QString auth = authority();
    if ((options & QUrl::RemoveAuthority) != QUrl::RemoveAuthority && !auth.isEmpty()) {

	url += "//";

        if ((options & QUrl::RemoveUserInfo) != QUrl::RemoveUserInfo) {
            if (!userName.isEmpty()) {
                // userinfo = *( unreserved / pct-encoded / sub-delims / ":" )
                url += QUrl::toPercentEncoding(userName, "!$&'()*+,;=:");
                if (!(options & QUrl::RemovePassword) && !password.isEmpty())
                    // userinfo = *( unreserved / pct-encoded / sub-delims / ":" )
                    url += ":" + QUrl::toPercentEncoding(password, "!$&'()*+,;=:");
                url += "@";
            }
        }

        // IDNA / rfc3490 describes these four delimiters used for
        // separating labels in unicode international domain
        // names.
        const unsigned short delimiters[] = {'[', 0x2e, 0x3002, 0xff0e, 0xff61, ']', 0};
        QStringList labels = host.split(QRegExp(QString::fromUtf16(delimiters)));
        for (int i = 0; i < labels.count(); ++i) {
            if (i != 0) url += '.';

            QString label = QUnicodeTables::normalize(labels.at(i), QString::NormalizationForm_KC, QChar::Unicode_3_1);
            url += QUrl::toPunycode(label);
        }

        if (!(options & QUrl::RemovePort) && port != -1) {
            url += ":";
            url += QString::number(port).toAscii();
        }
    }

    if (!(options & QUrl::RemovePath)) {
    // check if we need to insert a slash
        if (!path.isEmpty() && path.at(0) != QLatin1Char('/') && !auth.isEmpty())
            url += '/';
        // pchar = unreserved / pct-encoded / sub-delims / ":" / "@" ... alos "/"
        url += QUrl::toPercentEncoding(path, "!$&'()*+,;=:@/");
    }

    if (!(options & QUrl::RemoveQuery) && !query.isEmpty())
        url += "?" + query;
    if (!(options & QUrl::RemoveFragment) && !fragment.isEmpty())
        // fragment      = *( pchar / "/" / "?" )
        url += "#" + QUrl::toPercentEncoding(fragment, "!$&'()*+,;=:@/?");

    return url;
}

#define qToLower(ch) (((ch|32) >= 'a' && (ch|32) <= 'z') ? (ch|32) : ch)

const QByteArray & QUrlPrivate::normalized()
{
    if (QURL_HASFLAG(stateFlags, QUrlPrivate::Normalized))
        return encodedNormalized;

    QURL_SETFLAG(stateFlags, QUrlPrivate::Normalized);

    QUrlPrivate tmp = *this;
    tmp.scheme = tmp.scheme.toLower();
    tmp.host = tmp.host.toLower();
    if (!tmp.scheme.isEmpty()) // relative test
        tmp.path = QUrlPrivate::removeDotsFromPath(tmp.path);

    int qLen = tmp.query.length();
    for (int i = 0; i < qLen; i++) {
        if (qLen - i > 2 && tmp.query.at(i) == '%') {
            ++i;
            tmp.query[i] = qToLower(tmp.query.at(i));
            ++i;
            tmp.query[i] = qToLower(tmp.query.at(i));
        }
    }
    encodedNormalized = tmp.toEncoded();

    return encodedNormalized;
}


/*!
    \fn QUrl::QUrl(QUrlPrivate &d)

    \internal
*/

/*!
    Constructs a URL by parsing \a url. \a url is assumed to be in
    human readable representation, with no percent encoding. Any
    percent symbols '%' will be interpreted as they are.

    \code
        QUrl url("http://www.example.com/List of holidays.xml");
    \endcode

    To construct a URL from an encoded string, call fromEncoded():

    \code
        QUrl url = QUrl::fromEncoded("http://www.trolltech.com/List%20of%20holidays.xml");
    \endcode
*/
QUrl::QUrl(const QString &url) : d(new QUrlPrivate)
{
    if (!url.isEmpty())
        setUrl(url);
}

/*!
    Constructs an empty QUrl object.
*/
QUrl::QUrl() : d(new QUrlPrivate)
{
}

/*!
    Constructs a copy of \a other.
*/
QUrl::QUrl(const QUrl &other) : d(other.d)
{
    d->ref.ref();
}

/*!
    Destructor; called immediately before the object is deleted.
*/
QUrl::~QUrl()
{
    if (!d->ref.deref())
        delete d;
}

/*!
    Returns true if the URL is valid; otherwise returns false.

    The URL is run through a conformance test. Every part of the URL
    must conform to the standard encoding rules of the URI standard
    for the URL to be reported as valid.

    \code
        bool checkUrl(const QUrl &url) {
            if (!url.isValid()) {
                qDebug(QString("Invalid URL: %1").arg(url.toString()));
                return false;
            }

            return true;
        }
    \endcode
*/
bool QUrl::isValid() const
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Validated)) d->validate();

    return d->isValid;
}

/*!
    Returns true if the URL has no data; otherwise returns false.
*/
bool QUrl::isEmpty() const
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed))
        return d->encodedOriginal.isEmpty();
    else
        return d->scheme.isEmpty()
        && d->userName.isEmpty()
        && d->password.isEmpty()
        && d->host.isEmpty()
        && d->port == -1
        && d->path.isEmpty()
        && d->query.isEmpty()
        && d->fragment.isEmpty();
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

    \a url is assumed to be in unicode format, with no percent
    encoding.

    Calling isValid() will tell whether or not a valid URL was
    constructed.

    \sa setEncodedUrl()
*/
void QUrl::setUrl(const QString &url)
{
    // reserved      = gen-delims / sub-delims
    setEncodedUrl(QUrl::toPercentEncoding(url, ":/?#[]@!$&'()*+,;="));
}

/*!
    Constructs a URL by parsing the contents of \a encodedUrl.

    \a encodedUrl is assumed to be a URL string in percent encoded
    form, containing only ASCII characters.

    Use isValid() to determine if a valid URL was constructed.

    \sa setUrl()
*/
void QUrl::setEncodedUrl(const QByteArray &encodedUrl)
{
    clear();
    d->encodedOriginal = encodedUrl;
}

/*!
    Sets the scheme of the URL to \a scheme. As a scheme can only
    contain ASCII characters, no conversion or encoding is done on the
    input.

    The scheme describes the type (or protocol) of the URL. It's
    represented by one or more ASCII characters at the start the URL,
    and is followed by a ':'. The following example shows a URL where
    the scheme is "ftp":

    \img qurl-authority2.png

    The scheme can also be empty, in which case the URL is interpreted
    as relative.

    \sa scheme(), isRelative()
*/
void QUrl::setScheme(const QString &scheme)
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();
    detach();
    QURL_UNSETFLAG(d->stateFlags, QUrlPrivate::Validated | QUrlPrivate::Normalized);

    d->scheme = scheme;
}

/*!
    Returns the scheme of the URL. If an empty string is returned,
    this means the scheme is undefined and the URL is then relative.

    \sa setScheme(), isRelative()
*/
QString QUrl::scheme() const
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();

    return d->scheme;
}

/*!
    Sets the authority of the URL to \a authority.

    The authority of a URL is the combination of user info, a host
    name and a port. All of these elements are optional; an empty
    authority is therefore valid.

    The user info and host are separated by a '@', and the host and
    port are separated by a ':'. If the user info is empty, the '@'
    must be omitted; although a stray ':' is permitted if the port is
    empty.

    The following example shows a valid authority string:

    \img qurl-authority.png
*/
void QUrl::setAuthority(const QString &authority)
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();
    detach();
    QURL_UNSETFLAG(d->stateFlags, QUrlPrivate::Validated | QUrlPrivate::Normalized);

    d->setAuthority(authority);
}

/*!
    Returns the authority of the URL if it is defined; otherwise
    an empty string is returned.

    \sa setAuthority()
*/
QString QUrl::authority() const
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();

    return d->authority();
}

/*!
    Sets the user info of the URL to \a userInfo. The user info is an
    optional part of the authority of the URL, as described in
    setAuthority().

    The user info consists of a user name and optionally a password,
    separated by a ':'. If the password is empty, the colon must be
    omitted. The following example shows a valid user info string:

    \img qurl-authority3.png

    \sa userInfo(), setUserName(), setPassword(), setAuthority()
*/
void QUrl::setUserInfo(const QString &userInfo)
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();
    detach();
    QURL_UNSETFLAG(d->stateFlags, QUrlPrivate::Validated | QUrlPrivate::Normalized);

    d->setUserInfo(userInfo.trimmed());
}

/*!
    Returns the user info of the URL, or an empty string if the user
    info is undefined.
*/
QString QUrl::userInfo() const
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();

    return d->userInfo();
}

/*!
    Sets the URL's user name to \a userName. The \a userName is part
    of the user info element in the authority of the URL, as described
    in setUserInfo().

    \sa userName(), setUserInfo()
*/
void QUrl::setUserName(const QString &userName)
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();
    detach();
    QURL_UNSETFLAG(d->stateFlags, QUrlPrivate::Validated | QUrlPrivate::Normalized);

    d->userName = userName;
}

/*!
    Returns the user name of the URL if it is defined; otherwise
    an empty string is returned.

    \sa setUserName()
*/
QString QUrl::userName() const
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();

    return d->userName;
}

/*!
    Sets the URL's password to \a password. The \a password is part of
    the user info element in the authority of the URL, as described in
    setUserInfo().

    \sa password(), setUserInfo()
*/
void QUrl::setPassword(const QString &password)
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();
    detach();
    QURL_UNSETFLAG(d->stateFlags, QUrlPrivate::Validated | QUrlPrivate::Normalized);

    d->password = password;
}

/*!
    Returns the password of the URL if it is defined; otherwise
    an empty string is returned.

    \sa setUserName()
*/
QString QUrl::password() const
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();

    return d->password;
}

/*!
    Sets the host of the URL to \a host. The host is part of the
    authority.

    \sa host(), setAuthority()
*/
void QUrl::setHost(const QString &host)
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();
    detach();
    QURL_UNSETFLAG(d->stateFlags, QUrlPrivate::Validated | QUrlPrivate::Normalized);

    d->host = host.trimmed();
}

/*!
    Returns the host of the URL if it is defined; otherwise
    an empty string is returned.
*/
QString QUrl::host() const
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();

    return d->host;
}

/*!
    Sets the port of the URL to \a port. The port is part of the
    authority of the URL, as described in setAuthority().

    If \a port is negative or has a value larger than 65535, the port
    will be set to -1 indicating that the port is indefined.
*/
void QUrl::setPort(int port)
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();
    detach();
    QURL_UNSETFLAG(d->stateFlags, QUrlPrivate::Validated | QUrlPrivate::Normalized);

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
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();

    return d->port;
}

/*!
    Sets the path of the URL to \a path. The path is the part of the
    URL that comes after the authority but before the query string.

    \img qurl-ftppath.png

    For non-hierarchical schemes, the path will be everything
    following the scheme declaration, as in the following example:

    \img qurl-mailtopath.png

    \sa path()
*/
void QUrl::setPath(const QString &path)
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();
    detach();
    QURL_UNSETFLAG(d->stateFlags, QUrlPrivate::Validated | QUrlPrivate::Normalized);

    d->path = path;
}

/*!
    Returns the path of the URL.

    \sa setPath()
*/
QString QUrl::path() const
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();

    return d->path;
}

/*!
    Sets the characters used for delimiting between keys and values,
    and between key-value pairs in the URL's query string. The default
    value delimiter is '=' and the default pair delimiter is '&'.

    \img qurl-querystring.png

    \a valueDelimiter will be used for separating keys from values,
    and \a pairDelimiter will be used to separate key-value pairs.
    Any occurrences of these delimiting characters in the encoded
    representation of the keys and values of the query string are
    percent encoded.

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
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();
    detach();
    QURL_UNSETFLAG(d->stateFlags, QUrlPrivate::Validated | QUrlPrivate::Normalized);

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
void QUrl::setQueryItems(const QList<QPair<QString, QString> > &query)
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();
    detach();

    QByteArray alsoEncode;
    alsoEncode += d->valueDelimiter;
    alsoEncode += d->pairDelimiter;

    QByteArray queryTmp;
    for (int i = 0; i < query.size(); i++) {
        if (i) queryTmp += d->pairDelimiter;
        // query = *( pchar / "/" / "?" )
        queryTmp += QUrl::toPercentEncoding(query.at(i).first, "!$&'()*+,;=:@/?", alsoEncode);
        queryTmp += d->valueDelimiter;
        // query = *( pchar / "/" / "?" )
        queryTmp += QUrl::toPercentEncoding(query.at(i).second, "!$&'()*+,;=:@/?", alsoEncode);
    }

    d->query = queryTmp;
}

/*!
    Inserts the pair \a key = \a value into the query string of the
    URL.
*/
void QUrl::addQueryItem(const QString &key, const QString &value)
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();
    detach();

    QByteArray alsoEncode;
    alsoEncode += d->valueDelimiter;
    alsoEncode += d->pairDelimiter;

    if (!d->query.isEmpty())
        d->query += d->pairDelimiter;

    // query = *( pchar / "/" / "?" )
    d->query += QUrl::toPercentEncoding(key, "!$&'()*+,;=:@/?", alsoEncode);
    d->query += d->valueDelimiter;
    // query = *( pchar / "/" / "?" )
    d->query += QUrl::toPercentEncoding(value, "!$&'()*+,;=:@/?", alsoEncode);
}

/*!
    Returns the query string of the URL, as a map of keys and values.

    \sa setQueryItems(), setEncodedQuery()
*/
QList<QPair<QString, QString> > QUrl::queryItems() const
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();

    QList<QPair<QString, QString> > itemMap;

    QList<QByteArray> items = d->query.split(d->pairDelimiter);
    for (int i = 0; i < items.count(); ++i) {
        QList<QByteArray> keyValuePair = items.at(i).split(d->valueDelimiter);
        itemMap += qMakePair(QUrl::fromPercentEncoding(keyValuePair.at(0)),
                             QUrl::fromPercentEncoding(keyValuePair.at(1)));
    }

    return itemMap;
}

/*!
    Returns true if there is a query string pair whose key is equal
    to \a key from the URL.
*/
bool QUrl::hasQueryItem(const QString &key) const
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();

    QList<QPair<QString, QString> > items = queryItems();
    QList<QPair<QString, QString> >::ConstIterator it = items.constBegin();
    while (it != items.constEnd()) {
        if ((*it).first == key)
            return true;
        ++it;
    }

    return false;
}

/*!
    Returns the first query string value whose key is equal to \a key
    from the URL.

    \sa allQueryItemValues()
*/
QString QUrl::queryItemValue(const QString &key) const
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();

    QList<QPair<QString, QString> > items = queryItems();
    QList<QPair<QString, QString> >::ConstIterator it = items.constBegin();
    while (it != items.constEnd()) {
        if ((*it).first == key)
            return (*it).second;
        ++it;
    }

    return QString();
}

/*!
    Returns the a list of query string values whose key is equal to
    \a key from the URL.

    \sa queryItemValue()
*/
QStringList QUrl::allQueryItemValues(const QString &key) const
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();

    QStringList values;

    QList<QPair<QString, QString> > items = queryItems();
    QList<QPair<QString, QString> >::ConstIterator it = items.constBegin();
    while (it != items.constEnd()) {
        if ((*it).first == key)
            values += (*it).second;
        ++it;
    }

    return values;
}

/*!
    Removes the first query string pair whose key is equal to \a key
    from the URL.

    \sa removeAllQueryItems()
*/
void QUrl::removeQueryItem(const QString &key)
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();
    detach();

    QList<QPair<QString, QString> > items = queryItems();
    QList<QPair<QString, QString> >::Iterator it = items.begin();
    while (it != items.end()) {
        if ((*it).first == key) {
            items.erase(it);
            break;
        }
        ++it;
    }
    setQueryItems(items);
}

/*!
    Removes all the query string pairs whose key is equal to \a key
    from the URL.

   \sa removeQueryItem()
*/
void QUrl::removeAllQueryItems(const QString &key)
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();
    detach();

    QList<QPair<QString, QString> > items = queryItems();
    QList<QPair<QString, QString> >::Iterator it = items.begin();
    while (it != items.end()) {
        if ((*it).first == key) {
            it = items.erase(it);
            continue;
        }
        ++it;
    }
    setQueryItems(items);
}

/*!
    Returns the query string of the URL in percent encoded form.
*/
QByteArray QUrl::encodedQuery() const
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();

    return d->query;
}

/*!
    Sets the fragment of the URL to \a fragment. The fragment is the
    last part of the URL, represented by a '#' followed by a string of
    characters. It is typically used in HTTP for referring to a
    certain link or point on a page:

    \img qurl-fragment.png

    The fragment is sometimes also referred to as the URL "reference".

    \sa fragment()
*/
void QUrl::setFragment(const QString &fragment)
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();
    detach();
    QURL_UNSETFLAG(d->stateFlags, QUrlPrivate::Validated | QUrlPrivate::Normalized);

    d->fragment = fragment;
}

/*!
    Returns the fragment of the URL.

    \sa setFragment()
*/
QString QUrl::fragment() const
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();

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
         // prints "http://www.trolltech.com/products/solutions"
    \endcode

    Calling resolved() with ".." returns a QUrl whose directory is
    one level higher than the original. Similarly, calling resolved()
    with "../.." removes two levels from the path. If \a relative is
    "/", the path becomes "/".

    The algorithm for this merge is described

    \sa isRelative()
*/
QUrl QUrl::resolved(const QUrl &relative) const
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();

    QUrl r(relative), t;

    // be non strict and allow scheme in relative url
    if (r.scheme() == d->scheme)
        r.setScheme(QString());

    if (!r.scheme().isEmpty()) {
        t.setScheme(r.scheme());
        t.setAuthority(r.authority());
        t.setPath(r.path());
        t.d->path = QUrlPrivate::removeDotsFromPath(t.d->path);
        t.setEncodedQuery(r.encodedQuery());
    } else {
        if (!r.authority().isEmpty()) {
            t.setAuthority(r.authority());
            t.setPath(r.path());
            t.d->path = QUrlPrivate::removeDotsFromPath(t.d->path);
            t.setEncodedQuery(r.encodedQuery());
        } else {
            if (r.path().isEmpty()) {
                t.setPath(d->path);
                if (!r.encodedQuery().isEmpty())
                    t.setEncodedQuery(r.encodedQuery());
                else
                    t.setEncodedQuery(d->query);
            } else {
                if (r.path().startsWith(QLatin1Char('/'))) {
                    t.setPath(r.path());
                    t.d->path = QUrlPrivate::removeDotsFromPath(t.d->path);
                } else {
                    t.setPath(d->mergePaths(r.path()));
                    t.d->path = QUrlPrivate::removeDotsFromPath(t.d->path);
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
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();

    return d->scheme.isEmpty();
}

/*!
    Returns the human-displayable string representation of the
    URL. The output can be customized by passing flags with \a
    options.

    \sa FormattingOptions, toEncoded()
*/
QString QUrl::toString(FormattingOptions options) const
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();

    QString url;

    if (!(options & QUrl::RemoveScheme) && !d->scheme.isEmpty())
        url += d->scheme + QLatin1Char(':');
    if ((options & QUrl::RemoveAuthority) != QUrl::RemoveAuthority) {
        QString tmp = d->authority(options);
        if (!tmp.isEmpty()) {
            url += QLatin1String("//");
            url += tmp;
        }
    }
    if (!(options & QUrl::RemovePath)) {
	// check if we need to insert a slash
	if ((options & QUrl::RemoveAuthority) != QUrl::RemoveAuthority
	    && !d->authority(options).isEmpty() && !d->path.isEmpty() && d->path.at(0) != QLatin1Char('/'))
	    url += QLatin1Char('/');
	url += d->path;
    }
    if (!(options & QUrl::RemoveQuery) && !d->query.isEmpty())
        url += QLatin1Char('?') + fromPercentEncoding(d->query);
    if (!(options & QUrl::RemoveFragment) && !d->fragment.isEmpty())
        url += QLatin1Char('#') + d->fragment;

    return url;
}

/*!
    Returns the encoded representation of the URL if it's valid;
    otherwise an empty QByteArray is returned. The output can be
    customized by passing flags with \a options.

    The user info, path and fragment are all converted to UTF-8, and
    all non-ASCII characters are then percent encoded. The host name
    is encoded using Punycode.
*/
QByteArray QUrl::toEncoded(FormattingOptions options) const
{
    return d->toEncoded(options);
}

/*!
    Parses \a input and returns the corresponding QUrl. \a input is
    assumed to be in encoded form, containing only ASCII characters.
*/
QUrl QUrl::fromEncoded(const QByteArray &input)
{
    QUrl tmp;
    tmp.setEncodedUrl(input);
    return tmp;
}

/*!
    Returns a decoded copy of \a input. \a input is first decoded from
    percent encoding, then converted from UTF-8 to unicode.
*/
QString QUrl::fromPercentEncoding(const QByteArray &input)
{
    QVarLengthArray<char> tmp(input.size() + 1);

    char *data = tmp.data();
    const char *inputPtr = input.constData();

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

#if defined QURL_DEBUG
    qDebug("QUrl::fromPercentEncoding(\"%s\") == \"%s\"", input.data(), QString::fromUtf8(tmp.data()).toLatin1().constData());
#endif

    return QString::fromUtf8(tmp.data());
}

inline bool q_strchr(const char str[], char chr)
{
    if (!str) return false;

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
    to UTF-8, and all ASCII-characters that are not in the unreserved group
    are percent encoded. To prevent characters from being percent encoded
    pass them to \a exclude. To force characters to be percent encoded pass
    them to \a include.

    Unreserved is defined as:
       ALPHA / DIGIT / "-" / "." / "_" / "~"

    \code
         QByteArray ba = QUrl::toPercentEncoding("{a fishy string?}", "{}", "s");
         qDebug(ba.constData());
         // prints "{a fi%73hy %73tring%3F}"
    \endcode
*/
QByteArray QUrl::toPercentEncoding(const QString &input, const QByteArray &exclude, const QByteArray &include)
{

    QByteArray tmp = input.toUtf8();
    QVarLengthArray<char> output(tmp.size() * 3);

    int len = tmp.count();
    char *data = output.data();
    const char *inputData = tmp.constData();
    int length = 0;

    const char * dontEncode = 0;
    if (!exclude.isEmpty()) dontEncode = exclude.constData();


    if (include.isEmpty()) {
        for (int i = 0; i < len; ++i) {
            unsigned char c = *inputData++;
            if (c >= 0x61 && c <= 0x7A // ALPHA
                || c >= 0x41 && c <= 0x5A // ALPHA
                || c >= 0x30 && c <= 0x39 // DIGIT
                || c == 0x2D // -
                || c == 0x2E // .
                || c == 0x5F // _
                || c == 0x7E // ~
                || q_strchr(dontEncode, c)) {
                data[length++] = c;
            } else {
                data[length++] = '%';
                data[length++] = toHex((c & 0xf0) >> 4);
                data[length++] = toHex(c & 0xf);
            }
        }
    } else {
        const char * alsoEncode = include.constData();
        for (int i = 0; i < len; ++i) {
            unsigned char c = *inputData++;
            if ((c >= 0x61 && c <= 0x7A // ALPHA
                || c >= 0x41 && c <= 0x5A // ALPHA
                || c >= 0x30 && c <= 0x39 // DIGIT
                || c == 0x2D // -
                || c == 0x2E // .
                || c == 0x5F // _
                || c == 0x7E // ~
                || q_strchr(dontEncode, c))
                && !q_strchr(alsoEncode, c)) {
                data[length++] = c;
            } else {
                data[length++] = '%';
                data[length++] = toHex((c & 0xf0) >> 4);
                data[length++] = toHex(c & 0xf);
            }
        }
    }

#if defined QURL_DEBUG
    qDebug("QUrl::toPercentEncoding(\"%s\") == \"%s\"", input.toLatin1().constData(), QByteArray(output.data(), length).data());
#endif

    return QByteArray(output.data(), length);
}

inline uint adapt(uint delta, uint numpoints, bool firsttime)
{
    delta /= (firsttime ? damp : 2);
    delta += (delta / numpoints);

    uint k = 0;
    for (; delta > ((base - tmin) * tmax) / 2; k += base)
        delta /= (base - tmin);

    return k + (((base - tmin + 1) * delta) / (delta + skew));
}

inline char encodeDigit(uint digit)
{
  return digit + 22 + 75 * (digit < 26);
}

/*!
    Returns a \a uc in Punycode encoding.

    Punycode is a Unicode encoding used for internationalized domain
    names, as defined in RFC3492.
*/
QByteArray QUrl::toPunycode(const QString &uc)
{
    uint n = initial_n;
    uint delta = 0;
    uint bias = initial_bias;

    // assume that the size of output will be smaller than the number
    // of input characters.
    QByteArray output;

    int ucLength = uc.length();

    // copy all basic code points verbatim to output.
    for (uint j = 0; j < (uint) ucLength; ++j) {
        ushort js = uc.at(j).unicode();
        if (js < 0x80)
            output += js;
    }

    // if there were only basic code points, just return them
    // directly; don't do any encoding.
    if (output.size() == ucLength)
        return output;

    // h and b now contain the number of basic code points in input.
    uint b = output.size();
    uint h = output.size();

    // if basic code points were copied, add the delimiter character.
    if (h > 0) output += 0x2d;

    // while there are still unprocessed non-basic code points left in
    // the input string...
    while (h < (uint) ucLength) {
        // find the character in the input string with the lowest
        // unicode value.
        uint m = Q_MAXINT;
        uint j;
        for (j = 0; j < (uint) ucLength; ++j) {
            if (uc.at(j).unicode() >= n && uc.at(j).unicode() < m)
                m = (uint) uc.at(j).unicode();
        }

        // reject out-of-bounds unicode characters
        if (m - n > (Q_MAXINT - delta) / (h + 1))
            return ""; // punycode_overflow

        delta += (m - n) * (h + 1);
        n = m;

        // for each code point in the input string
        for (j = 0; j < (uint) ucLength; ++j) {

            // increase delta until we reach the character with the
            // lowest unicode code. fail if delta overflows.
            if (uc.at(j).unicode() < n) {
                ++delta;
                if (!delta)
                    return ""; // punycode_overflow
            }

            // if j is the index of the character with the lowest
            // unicode code...
            if (uc.at(j).unicode() == n) {
                uint qq;
                uint k;
                uint t;

                // insert the variable length delta integer; fail on
                // overflow.
                for (qq = delta, k = base;; k += base) {
                    // stop generating digits when the threshold is
                    // detected.
                    t = (k <= bias) ? tmin : (k >= bias + tmax) ? tmax : k - bias;
                    if (qq < t) break;

                    output += encodeDigit(t + (qq - t) % (base - t));
                    qq = (qq - t) / (base - t);
                }

                output += encodeDigit(qq);
                bias = adapt(delta, h + 1, h == b);
                delta = 0;
                ++h;
            }
        }

        ++delta;
        ++n;
    }

    // prepend ACE prefix
    output.prepend("xn--");
    return output;

}

/*!
    Returns the Punycode decoded representation of \a pc.

    Punycode is a Unicode encoding used for internationalized domain
    names, as defined in RFC3492.
*/
QString QUrl::fromPunycode(const QByteArray &pc)
{
    uint n = initial_n;
    uint i = 0;
    uint bias = initial_bias;

    // strip any ACE prefix
    QByteArray inputTrimmed = (pc.startsWith("xn--") ? pc.mid(4) : pc);

    // find the last delimiter character '-' in the input array. copy
    // all data before this delimiter directly to the output array.
    int delimiterPos = inputTrimmed.lastIndexOf(0x2d);
    QString output = QLatin1String(delimiterPos == -1 ? "" : inputTrimmed.left(delimiterPos).constData());

    // if a delimiter was found, skip to the position after it;
    // otherwise start at the front of the input string. everything
    // before the delimiter is assumed to be basic code points.
    uint cnt = delimiterPos ? delimiterPos + 1 : 0;

    // loop through the rest of the input string, inserting non-basic
    // characters into output as we go.
    while (cnt < (uint) inputTrimmed.size()) {
        uint oldi = i;
        uint w = 1;

        // find the next index for inserting a non-basic character.
        for (uint k = base; cnt < (uint) inputTrimmed.length(); k += base) {
            // grab a character from the punycode input and find its
            // delta digit (each digit code is part of the
            // variable-length integer delta)
            uint digit = inputTrimmed.at(cnt++);
            if (digit - 48 < 10) digit -= 22;
            else if (digit - 65 < 26) digit -= 65;
            else if (digit - 97 < 26) digit -= 97;
            else digit = base;

            // reject out of range digits
            if (digit >= base || digit > (Q_MAXINT - i) / w)
                return QLatin1String("");

            i += (digit * w);

            // detect threshold to stop reading delta digits
            uint t;
            if (k <= bias) t = tmin;
            else if (k >= bias + tmax) t = tmax;
            else t = k - bias;
            if (digit < t) break;

            w *= (base - t);
        }

        // find new bias and calculate the next non-basic code
        // character.
        bias = adapt(i - oldi, output.length() + 1, oldi == 0);
        n += i / (output.length() + 1);

        // allow the deltas to wrap around
        i %= (output.length() + 1);

        // insert the character n at position i
        output.insert((uint) i, QChar((ushort) n));
        ++i;
    }

    return output;
}

/*!
    \internal

    Returns true if this URL is "less than" the given \a url. This
    provides a means of ordering URLs.
*/
bool QUrl::operator <(const QUrl &url) const
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();
    if (!QURL_HASFLAG(url.d->stateFlags, QUrlPrivate::Parsed)) url.d->parse();
    return d->normalized() < url.d->normalized();
}

/*!
    Returns true if this URL and the given \a url are equal;
    otherwise returns false.
*/
bool QUrl::operator ==(const QUrl &url) const
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();
    if (!QURL_HASFLAG(url.d->stateFlags, QUrlPrivate::Parsed)) url.d->parse();
    return d->normalized() == url.d->normalized();
}

/*!
    Returns true if this URL and the given \a url are not equal;
    otherwise returns false.
*/
bool QUrl::operator !=(const QUrl &url) const
{
    return !(*this == url);
}

/*!
    Assigns the data of \a url to this class.
*/
QUrl &QUrl::operator =(const QUrl &url)
{
    qAtomicAssign(d, url.d);
    return *this;
}

/*! \internal

    Forces a detach.
*/
void QUrl::detach()
{ qAtomicDetach(d); }

/*!
    \internal
*/
bool QUrl::isDetached() const
{
    return d->ref == 1;
}


/*!
    Returns a QUrl representation of \a localFile, interpreted as a
    local file.
*/
QUrl QUrl::fromLocalFile(const QString &localFile)
{
    QUrl url;
    url.setScheme(QLatin1String("file"));
    QString deslashified = localFile;
    deslashified.replace(QLatin1Char('\\'), QLatin1Char('/'));

    // magic for drives on windows
    if (deslashified.length() > 1 && deslashified.at(1) == QLatin1Char(':') && deslashified.at(0) != QLatin1Char('/')) {
        url.setPath(QLatin1String("/") + deslashified);
    // magic for shared drive on windows
    } else if (deslashified.startsWith(QLatin1String("//"))) {
	int indexOfPath = deslashified.indexOf(QLatin1Char('/'), 2);
	url.setHost(deslashified.mid(2, indexOfPath - 2));
	if (indexOfPath > 2)
	    url.setPath(deslashified.right(deslashified.length() - indexOfPath));
    } else {
        url.setPath(deslashified);
    }

    return url;
}

/*!
    Returns the path of this URL formatted as a local file path.
*/
QString QUrl::toLocalFile() const
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();

    QString tmp;
    if (d->scheme.isEmpty() || d->scheme.toLower() == QLatin1String("file")) {

	// magic for shared drive on windows
	if (!d->host.isEmpty()) {
	    tmp = QLatin1String("//") + d->host + (d->path.length() > 0 && d->path.at(0) != QLatin1Char('/')
						  ? QLatin1String("/") + d->path :  d->path);
        } else {
	    tmp = d->path;
	    // magic for drives on windows
            if (d->path.length() > 2 && d->path.at(0) == QLatin1Char('/') && d->path.at(2) == QLatin1Char(':'))
		tmp.remove(0, 1);
	}
    }

    return tmp;
}

/*!
    Returns true if this URL is a parent of \a childUrl. \a childUrl is a child
    of this URL if the two URLs share the same scheme and authority,
    and this URL's path is a parent of the path of \a childUrl.
*/
bool QUrl::isParentOf(const QUrl &childUrl) const
{
    if (!QURL_HASFLAG(d->stateFlags, QUrlPrivate::Parsed)) d->parse();

    QString childPath = childUrl.path();

    return ((childUrl.scheme().isEmpty() || d->scheme == childUrl.scheme())
            && (childUrl.authority().isEmpty() || d->authority() == childUrl.authority())
            &&  childPath.startsWith(d->path)
            && ((d->path.endsWith(QLatin1Char('/')) && childPath.length() > d->path.length())
                || (!d->path.endsWith(QLatin1Char('/'))
                    && childPath.length() > d->path.length() && childPath.at(d->path.length()) == QLatin1Char('/'))));
}

/*!
    \fn void QUrl::setProtocol(const QString &s)

    Use setScheme() instead.
*/

/*!
    \fn void QUrl::setUser(const QString &s)

    Use setUserName() instead.
*/

/*!
    \fn bool QUrl::hasUser() const

    Use !userName().isEmpty() instead.
*/

/*!
    \fn bool QUrl::hasPassword() const

    Use !password().isEmpty() instead.
*/

/*!
    \fn bool QUrl::hasHost() const

    Use !host().isEmpty() instead.
*/

/*!
    \fn bool QUrl::hasPort() const

    Use port() != -1 instead.
*/

/*!
    \fn bool QUrl::hasPath() const

    Use !path().isEmpty() instead.
*/

/*!
    \fn void QUrl::setQuery(const QString &txt)

    Use setEncodedQuery() instead.
*/

/*!
    \fn void QUrl::setRef(const QString &txt)

    Use setFragment() instead.
*/

/*!
    \fn bool QUrl::hasRef() const

    Use !fragment().isEmpty() instead.
*/

/*!
    \fn void QUrl::addPath(const QString &p)

    Use setPath() instead.
*/

/*!
    \fn void QUrl::setFileName(const QString &txt)

    Use setPath() instead.
*/

/*!
    \fn void QUrl::decode(QString &url)

    Use fromPercentEncoding() instead.
*/

/*!
    \fn void QUrl::encode(QString &url)

    Use toPercentEncoding() instead.
*/

/*!
    \fn bool QUrl::cdUp()

    Use resolved("..") instead.

    \oldcode
        QUrl url("http://www.trolltech.com/Developer/");
        url.cdUp();
    \newcode
        QUrl url("http://www.trolltech.com/Developer/");
        url = url.resolved("..");
    \endcode
*/

/*!
    \fn bool QUrl::isRelativeUrl(const QString &url)

    Use isRelative() instead.
*/

/*!
    \fn void QUrl::reset()

    Use clear() instead.
*/

/*!
    \fn  QUrl::operator QString() const

    Use toString() instead.
*/

/*!
    \fn QString QUrl::protocol() const

    Use scheme() instead.
*/

/*!
    \fn QString QUrl::user() const

    Use userName() instead.
*/

/*!
    \fn QString QUrl::query() const

    Use encodedQuery() instead.
*/

/*!
    \fn QString QUrl::ref() const

    Use fragment() instead.
*/

/*!
    \fn QString QUrl::fileName() const

    Use QFileInfo(path()).fileName() instead.
*/

/*!
    \fn QString QUrl::dirPath() const

    Use QFileInfo(path()).absolutePath() or QFileInfo(path()) instead.
*/

#ifdef QT3_SUPPORT
void QUrl::setFileName(const QString &txt)
{
    QFileInfo fileInfo(path());
    fileInfo.setFile(txt);
    setPath(fileInfo.filePath());
}

QString QUrl::fileName() const
{
    QFileInfo fileInfo(path());
    return fileInfo.fileName();
}

QString QUrl::dirPath() const
{
    QFileInfo fileInfo(path());
    if (fileInfo.isAbsolute())
        return fileInfo.absolutePath();
    return fileInfo.path();
}
#endif


#ifndef QT_NO_DATASTREAM
/*! \relates QUrl

    Writes url \a url to the stream \a out and returns a reference
    to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
QDataStream &operator<<(QDataStream &out, const QUrl &url)
{
    QByteArray u = url.toEncoded();
    out << u;
    return out;
}

/*! \relates QUrl

    Reads a url into \a url from the stream \a in and returns a
    reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
QDataStream &operator>>(QDataStream &in, QUrl &url)
{
    QByteArray u;
    in >> u;
    url = QUrl::fromEncoded(u);
    return in;
}
#endif // QT_NO_DATASTREAM

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const QUrl &url)
{
    d.maybeSpace() << "QUrl(" << url.toString() << ")";
    return d.space();
}
#endif

