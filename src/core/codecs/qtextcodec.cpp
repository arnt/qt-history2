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

#include "qplatformdefs.h"


#include "qtextcodec.h"

#ifndef QT_NO_TEXTCODEC

#include "qlist.h"
#include "qfile.h"
#include "qtextcodecfactory.h"

#include "qutfcodec_p.h"
#include "qsimplecodec_p.h"
#include "qlatincodec_p.h"
#ifndef QT_NO_CODECS
#include "qtsciicodec_p.h"
#include "qisciicodec_p.h"
#endif // QT_NO_CODECS

#include <private/qlocale_p.h>
#include <private/qmutexpool_p.h>
#include <stdlib.h>
#include <ctype.h>
#ifndef Q_OS_TEMP
#include <locale.h>
#endif
#if defined(_XOPEN_UNIX) && !defined(Q_OS_QNX6)
#include <langinfo.h>
#endif

static QList<QTextCodec*> *all = 0;
static bool destroying_is_ok = false;
static QTextCodec *localeMapper = 0;

/*!
    Deletes all the created codecs.

    \internal

    Qt calls this function just before exiting to delete any
    QTextCodec objects that may be lying around. Since various other
    classes hold pointers to QTextCodec objects, it is not safe to
    call this function earlier.
*/

static void deleteAllCodecs()
{
    if (!all)
        return;

    QMutexLocker locker(qt_global_mutexpool ?
                         qt_global_mutexpool->get(&all) : 0);
    if (!all)
        return;

    destroying_is_ok = true;

    QList<QTextCodec*> *ball = all;
    all = 0;
    QList<QTextCodec*>::Iterator it;
    for (it = ball->begin(); it != ball->end(); ++it) {
        delete *it;
        *it = 0;
    }
    ball->clear();
    delete ball;

    destroying_is_ok = false;
}

class QTextCodecCleanup
{
public:
    ~QTextCodecCleanup() { deleteAllCodecs(); }
};
static QTextCodecCleanup qtextcodec_cleanup;


static void realSetup();

static inline void setup()
{
    if (all) return;

    QMutexLocker locker(qt_global_mutexpool ?
                         qt_global_mutexpool->get(&all) : 0);
    if (all) return;

    realSetup();
}


/*!
    \class QTextCodec qtextcodec.h
    \brief The QTextCodec class provides conversions between text encodings.
    \reentrant
    \ingroup i18n

    Qt uses Unicode to store, draw and manipulate strings. In many
    situations you may wish to deal with data that uses a different
    encoding. For example, most Japanese documents are still stored in
    Shift-JIS or ISO2022, while Russian users often have their
    documents in KOI8-R or CP1251.

    Qt provides a set of QTextCodec classes to help with converting
    non-Unicode formats to and from Unicode. You can also create your
    own codec classes (\link #subclassing see later\endlink).

    The supported encodings are:
    \list
    \i Latin1
    \i Big5 -- Chinese
    \i Big5-HKSCS -- Chinese
    \i eucJP -- Japanese
    \i eucKR -- Korean
    \i GB2312 -- Chinese
    \i GBK -- Chinese
    \i GB18030 -- Chinese
    \i JIS7 -- Japanese
    \i Shift-JIS -- Japanese
    \i TSCII -- Tamil
    \i utf8 -- Unicode, 8-bit
    \i utf16 -- Unicode
    \i KOI8-R -- Russian
    \i KOI8-U -- Ukrainian
    \i ISO8859-1 -- Western
    \i ISO8859-2 -- Central European
    \i ISO8859-3 -- Central European
    \i ISO8859-4 -- Baltic
    \i ISO8859-5 -- Cyrillic
    \i ISO8859-6 -- Arabic
    \i ISO8859-7 -- Greek
    \i ISO8859-8 -- Hebrew, visually ordered
    \i ISO8859-8-i -- Hebrew, logically ordered
    \i ISO8859-9 -- Turkish
    \i ISO8859-10
    \i ISO8859-13
    \i ISO8859-14
    \i ISO8859-15 -- Western
    \i IBM 850
    \i IBM 866
    \i CP874
    \i CP1250 -- Central European
    \i CP1251 -- Cyrillic
    \i CP1252 -- Western
    \i CP1253 -- Greek
    \i CP1254 -- Turkish
    \i CP1255 -- Hebrew
    \i CP1256 -- Arabic
    \i CP1257 -- Baltic
    \i CP1258
    \i Apple Roman
    \i TIS-620 -- Thai
    \endlist

    QTextCodecs can be used as follows to convert some locally encoded
    string to Unicode. Suppose you have some string encoded in Russian
    KOI8-R encoding, and want to convert it to Unicode. The simple way
    to do it is like this:

    \code
    QByteArray locallyEncoded = "..."; // text to convert
    QTextCodec *codec = QTextCodec::codecForName("KOI8-R"); // get the codec for KOI8-R
    QString unicodeString = codec->toUnicode(locallyEncoded);
    \endcode

    After this, \c{unicodeString} holds the text converted to Unicode.
    Converting a string from Unicode to the local encoding is just as
    easy:

    \code
    QString unicodeString = "..."; // any Unicode text
    QTextCodec *codec = QTextCodec::codecForName("KOI8-R"); // get the codec for KOI8-R
    QByteArray locallyEncoded = codec->fromUnicode(unicodeString);
    \endcode

    Some care must be taken when trying to convert the data in chunks,
    for example, when receiving it over a network. In such cases it is
    possible that a multi-byte character will be split over two
    chunks. At best this might result in the loss of a character and
    at worst cause the entire conversion to fail.

    The approach to use in these situations is to create a QTextDecoder
    object for the codec and use this QTextDecoder for the whole
    decoding process, as shown below:

    \code
    QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    QTextDecoder *decoder = codec->makeDecoder();

    QString unicodeString;
    while(receiving_data) {
        QByteArray chunk = new_data;
        unicodeString += decoder->toUnicode(chunk.data(), chunk.length());
    }
    \endcode

    The QTextDecoder object maintains state between chunks and therefore
    works correctly even if a multi-byte character is split between
    chunks.

    \target subclassing
    \section1 Creating your own Codec class

    Support for new text encodings can be added to Qt by creating
    QTextCodec subclasses.

    Built-in codecs can be overridden by custom codecs since more
    recently created QTextCodec objects take precedence over earlier
    ones.

    You may find it more convenient to make your codec class available
    as a plugin; see the \link plugins-howto.html plugin
    documentation\endlink for more details.

    The abstract virtual functions describe the encoder to the
    system and the coder is used as required in the different
    text file formats supported by QTextStream, and under X11, for the
    locale-specific character input and output.

    To add support for another 8-bit encoding to Qt, make a subclass
    of QTextCodec and implement at least the following methods:

    \code
    const char* name() const
    \endcode
    Return the official name for the encoding.

    \code
    int mibEnum() const
    \endcode
    Return the MIB enum for the encoding if it is listed in the
        \link http://www.iana.org/assignments/character-sets
        IANA character-sets encoding file\endlink.

    If the encoding is multi-byte then it will have "state"; that is,
    the interpretation of some bytes will be dependent on some preceding
    bytes. For such encodings, you must implement:

    \code
    QTextDecoder* makeDecoder() const
    \endcode
    Return a QTextDecoder that remembers incomplete multi-byte sequence
    prefixes or other required state.

    If the encoding does \e not require state, you should implement:

    \code
    QString toUnicode(const char* chars, int len) const
    \endcode
    Converts \e len characters from \e chars to Unicode.

    The base QTextCodec class has default implementations of the above
    two functions, \e{but they are mutually recursive}, so you must
    re-implement at least one of them, or both for improved efficiency.

    For conversion from Unicode to 8-bit encodings, it is rarely necessary
    to maintain state. However, two functions similar to the two above
    are used for encoding:

    \code
    QTextEncoder* makeEncoder() const
    \endcode
    Return a QTextEncoder.

    \code
    QByteArray fromUnicode(const QString& uc, int& lenInOut) const
    \endcode
    Converts \e lenInOut characters (of type QChar) from the start of
    the string \e uc, returning a QByteArray result, and also returning
    the \link QByteArray::length() length\endlink of the result in
    \e lenInOut.

    Again, these are mutually recursive so only one needs to be implemented,
    or both if greater efficiency is possible.

    A QTextCodec subclass might have improved performance if you also
    re-implement:

    \code
    bool canEncode(QChar) const
    \endcode
    Test if a Unicode character can be encoded.

    \code
    bool canEncode(const QString&) const
    \endcode
    Test if a string of Unicode characters can be encoded.

    Codecs can also be created as \link plugins-howto.html plugins\endlink.
*/


/*!
    \nonreentrant

    Constructs a QTextCodec, and gives it the highest precedence. The
    QTextCodec should always be constructed on the heap (i.e. with \c
    new). Qt takes ownership and will delete it when the application
    terminates.
*/
QTextCodec::QTextCodec()
{
    setup();
    all->insert(all->begin(), this);
}


/*!
    \nonreentrant

    Destroys the QTextCodec. Note that you should not delete codecs
    yourself: once created they become Qt's responsibility.
*/
QTextCodec::~QTextCodec()
{
    if (!destroying_is_ok)
        qWarning("QTextCodec::~QTextCodec() called by application");
    if (all)
        all->removeAll(this);
}



#if 0
/*!
    A simple utility function for heuristicNameMatch(): it does some
    very minor character-skipping so that almost-exact matches score
    high. \a name is the text we're matching and \a hint is used for
    the comparison.
*/
int QTextCodec::simpleHeuristicNameMatch(const char *name, const char *hint)
{
    // if they're the same, return a perfect score
    if (qstricmp(name, hint) == 0)
        return qstrlen(hint);

    const char *n = name;
    const char *h = hint;

    // if the letters and numbers are the same, we have an almost
    // perfect match
    while (*n != '\0') {
        if (isalnum((uchar)*n)) {
            for (;;) {
                if (*h == '\0')
                    return 0;
                if (isalnum((uchar)*h))
                    break;
                ++h;
            }
            if (tolower((uchar)*n) != tolower((uchar)*h))
                return 0;
            ++h;
        }
        ++n;
    }
    return h - hint - 1;
}
#endif


/*!
    Returns the QTextCodec which matches the \link
    QTextCodec::mibEnum() MIBenum\endlink \a mib.
*/
QTextCodec* QTextCodec::codecForMib(int mib)
{
    setup();
    QList<QTextCodec*>::ConstIterator i;
    QTextCodec* result=0;
    for (i = all->begin(); i != all->end(); ++i) {
        result = *i;
        if (result->mibEnum() == mib)
            return result;
    }

#if !defined(QT_NO_COMPONENT) && !defined(QT_LITE_COMPONENT)
    if (!result || (result && result->mibEnum() != mib)) {
        QTextCodec *codec = QTextCodecFactory::createForMib(mib);
        if (codec)
            result = codec;
    }
#endif // !QT_NO_COMPONENT !QT_LITE_COMPONENT

    return result;
}


#ifdef Q_OS_WIN32
class QWindowsLocalCodec: public QTextCodec
{
public:
    QWindowsLocalCodec();
    ~QWindowsLocalCodec();

    QString convertToUnicode(const char *, int, ConverterState *) const;
    QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const;

    const char* name() const;
    int mibEnum() const;

};

QWindowsLocalCodec::QWindowsLocalCodec()
{
}

QWindowsLocalCodec::~QWindowsLocalCodec()
{
}


QString QWindowsLocalCodec::convertToUnicode(const char* chars, int len, ConverterState *state) const
{
    return qt_winMB2QString(chars, len);
}

QByteArray QWindowsLocalCodec::convertFromUnicode(const QChar *uc, int len, ConverterState *) const
{
    return qt_winQString2MB(uc, len);
}


const char* QWindowsLocalCodec::name() const
{
    return "System";
}

int QWindowsLocalCodec::mibEnum() const
{
    return 0;
}

#else

/* locale names mostly copied from XFree86 */
static const char * const iso8859_2locales[] = {
    "croatian", "cs", "cs_CS", "cs_CZ","cz", "cz_CZ", "czech", "hr",
    "hr_HR", "hu", "hu_HU", "hungarian", "pl", "pl_PL", "polish", "ro",
    "ro_RO", "rumanian", "serbocroatian", "sh", "sh_SP", "sh_YU", "sk",
    "sk_SK", "sl", "sl_CS", "sl_SI", "slovak", "slovene", "sr_SP", 0 };

static const char * const iso8859_3locales[] = {
    "eo", 0 };

static const char * const iso8859_4locales[] = {
    "ee", "ee_EE", 0 };

static const char * const iso8859_5locales[] = {
    "mk", "mk_MK", "sp", "sp_YU", 0 };

static const char * const cp_1251locales[] = {
    "be", "be_BY", "bg", "bg_BG", "bulgarian", 0 };

static const char * const pt_154locales[] = {
    "ba_RU", "ky", "ky_KG", "kk", "kk_KZ", 0 };

static const char * const iso8859_6locales[] = {
    "ar_AA", "ar_SA", "arabic", 0 };

static const char * const iso8859_7locales[] = {
    "el", "el_GR", "greek", 0 };

static const char * const iso8859_8locales[] = {
    "hebrew", "he", "he_IL", "iw", "iw_IL", 0 };

static const char * const iso8859_9locales[] = {
    "tr", "tr_TR", "turkish", 0 };

static const char * const iso8859_13locales[] = {
    "lt", "lt_LT", "lv", "lv_LV", 0 };

static const char * const iso8859_15locales[] = {
    "et", "et_EE",
    // Euro countries
    "br_FR", "ca_ES", "de", "de_AT", "de_BE", "de_DE", "de_LU", "en_IE",
    "es", "es_ES", "eu_ES", "fi", "fi_FI", "finnish", "fr", "fr_FR",
    "fr_BE", "fr_LU", "french", "ga_IE", "gl_ES", "it", "it_IT", "oc_FR",
    "nl", "nl_BE", "nl_NL", "pt", "pt_PT", "sv_FI", "wa_BE",
    0 };

static const char * const koi8_ulocales[] = {
    "uk", "uk_UA", "ru_UA", "ukrainian", 0 };

static const char * const tis_620locales[] = {
    "th", "th_TH", "thai", 0 };

static const char * const tcvnlocales[] = {
    "vi", "vi_VN", 0 };

static bool try_locale_list(const char * const locale[], const char * lang)
{
    int i;
    for(i=0; locale[i] && *locale[i] && strcmp(locale[i], lang); i++)
        ;
    return locale[i] != 0;
}

// For the probably_koi8_locales we have to look. the standard says
// these are 8859-5, but almost all Russian users use KOI8-R and
// incorrectly set $LANG to ru_RU. We'll check tolower() to see what
// totoLower() thinks ru_RU means.

// If you read the history, it seems that many Russians blame ISO and
// Perestroika for the confusion.
//
// The real bug is that some programs break if the user specifies
// ru_RU.KOI8-R.

static const char * const probably_koi8_rlocales[] = {
    "ru", "ru_SU", "ru_RU", "russian", 0 };

static QTextCodec * ru_RU_hack(const char * i) {
    QTextCodec * ru_RU_codec = 0;

    QByteArray origlocale(setlocale(LC_CTYPE, i));
    // unicode   koi8r   latin5   name
    // 0x044E    0xC0    0xEE     CYRILLIC SMALL LETTER YU
    // 0x042E    0xE0    0xCE     CYRILLIC CAPITAL LETTER YU
    int latin5 = tolower(0xCE);
    int koi8r = tolower(0xE0);
    if (koi8r == 0xC0 && latin5 != 0xEE) {
        ru_RU_codec = QTextCodec::codecForName("KOI8-R");
    } else if (koi8r != 0xC0 && latin5 == 0xEE) {
        ru_RU_codec = QTextCodec::codecForName("ISO 8859-5");
    } else {
        // something else again... let's assume... *throws dice*
        ru_RU_codec = QTextCodec::codecForName("KOI8-R");
        qWarning("QTextCodec: using KOI8-R, probe failed (%02x %02x %s)",
                  koi8r, latin5, i);
    }
    setlocale(LC_CTYPE, origlocale);

    return ru_RU_codec;
}

#endif

/*!
    Set the codec to \a c; this will be returned by codecForLocale().
    This might be needed for some applications that want to use their
    own mechanism for setting the locale.

    \sa codecForLocale()
*/
void QTextCodec::setCodecForLocale(QTextCodec *c) {
    localeMapper = c;
}

/*! Returns a pointer to the codec most suitable for this locale. */

QTextCodec* QTextCodec::codecForLocale()
{
    if (localeMapper)
        return localeMapper;

    setup();

    return localeMapper;
}


/*!
    Searches all installed QTextCodec objects and returns the one
    which best matches \a name; the match is case-insensitive. Returns
    0 if no codec matching the name \a name could be found.
*/

QTextCodec *QTextCodec::codecForName(const QByteArray &name)
{
    if (name.isEmpty())
        return 0;

    setup();
    QList<QTextCodec*>::ConstIterator i;
    for (i = all->begin(); i != all->end(); ++i) {
        QTextCodec *cursor = *i;
        if (cursor->name() == name || cursor->mimeName() == name)
            return cursor;
    }

#if !defined(QT_NO_COMPONENT) && !defined(QT_LITE_COMPONENT)
    return QTextCodecFactory::createForName(name);
#else
    return 0;
#endif // !QT_NO_COMPONENT !QT_LITE_COMPONENT
}


// #### doc: the names should match IANA.
/*!
    \fn const char* QTextCodec::name() const

    QTextCodec subclasses must reimplement this function. It returns
    the name of the encoding supported by the subclass. When choosing
    a name for an encoding, consider these points:
    \list
    \i        On X11, heuristicNameMatch(const char * hint)
        is used to test if a the QTextCodec
        can convert between Unicode and the encoding of a font
        with encoding \e hint, such as "iso8859-1" for Latin1 fonts,
        "koi8-r" for Russian KOI8 fonts.
        The default algorithm of heuristicNameMatch() uses name().
    \i        Some applications may use this function to present
        encodings to the end user.
    \endlist
    */

/*!
    \fn int QTextCodec::mibEnum() const

    Subclasses of QTextCodec must reimplement this function. It
    returns the MIBenum (see \link
    http://www.iana.org/assignments/character-sets the
    IANA character-sets encoding file\endlink for more information).
    It is important that each QTextCodec subclass returns the correct
    unique value for this function.
*/


/*!
    Returns the preferred mime name of the encoding as defined in the
    \link http://www.iana.org/assignments/character-sets
    IANA character-sets encoding file\endlink.
*/
const char* QTextCodec::mimeName() const
{
    return name();
}

#ifdef QT_COMPAT
/*!
    Creates a QTextDecoder which stores enough state to decode chunks
    of char* data to create chunks of Unicode data. The default
    implementation creates a stateless decoder, which is only
    sufficient for the simplest encodings where each byte corresponds
    to exactly one Unicode character.

    The caller is responsible for deleting the returned object.
*/
QTextDecoder* QTextCodec::makeDecoder() const
{
    return new QTextDecoder(this);
}


/*!
    Creates a QTextEncoder which stores enough state to encode chunks
    of Unicode data as char* data. The default implementation creates
    a stateless encoder, which is only sufficient for the simplest
    encodings where each Unicode character corresponds to exactly one
    character.

    The caller is responsible for deleting the returned object.
*/
QTextEncoder* QTextCodec::makeEncoder() const
{
    return new QTextEncoder(this);
}

/*!
    Returns a string representing the current language and
    sublanguage, e.g. "pt" for Portuguese, or "pt_br" for Portuguese/Brazil.
*/
const char* QTextCodec::locale()
{
    return QLocalePrivate::systemLocaleName();
}

/*!
    QTextCodec subclasses must reimplement either this function or
    makeEncoder(). It converts the first \a lenInOut characters of \a
    uc from Unicode to the encoding of the subclass. If \a lenInOut is
    negative or too large, the length of \a uc is used instead.

    Converts \a lenInOut characters (not bytes) from \a uc, producing
    a QByteArray. \a lenInOut will be set to the \link
    QByteArray::length() length\endlink of the result (in bytes).

    The default implementation makes an encoder with makeEncoder() and
    converts the input with that. Note that the default makeEncoder()
    implementation makes an encoder that simply calls this function,
    hence subclasses \e must reimplement one function or the other to
    avoid infinite recursion.
*/

QByteArray QTextCodec::fromUnicode(const QString& uc, int& lenInOut) const
{
    QByteArray result = convertFromUnicode(uc.constData(), lenInOut, 0);
    lenInOut = result.length();
    return result;
}

/*!
    \overload

    \a a contains the source characters; \a len contains the number of
    characters in \a a to use.
*/
QString QTextCodec::toUnicode(const QByteArray& a, int len) const
{
    len = qMin(a.size(), len);
    return convertToUnicode(a.constData(), len, 0);
}

/*!
    \overload

    \a chars contains the source characters.
*/
QString QTextCodec::toUnicode(const char* chars) const
{
    int len = qstrlen(chars);
    return convertToUnicode(chars, len, 0);
}
#endif

/*!
    \overload

    \a str is the unicode source string.
*/
QByteArray QTextCodec::fromUnicode(const QString& str) const
{
    return convertFromUnicode(str.constData(), str.length(), 0);
}


/*!
    \overload

    \a a contains the source characters.
*/
QString QTextCodec::toUnicode(const QByteArray& a) const
{
    return convertToUnicode(a.constData(), a.length(), 0);
}


/*!
    Returns true if the Unicode character \a ch can be fully encoded
    with this codec; otherwise returns false. The default
    implementation tests if the result of toUnicode(fromUnicode(ch))
    is the original \a ch. Subclasses may be able to provide a more
    efficient algorithm.
*/
bool QTextCodec::canEncode(QChar ch) const
{
    ConverterState state;
    state.flags = ConvertInvalidToNull;
    convertFromUnicode(&ch, 1, &state);
    return (state.invalidChars == 0);
}

/*!
    \overload

    \a s contains the string being tested for encode-ability.
*/
bool QTextCodec::canEncode(const QString& s) const
{
    ConverterState state;
    state.flags = ConvertInvalidToNull;
    convertFromUnicode(s.constData(), s.length(), &state);
    return (state.invalidChars == 0);
}

#ifdef QT_COMPAT
/*!
    \class QTextEncoder qtextcodec.h
    \brief The QTextEncoder class provides a state-based encoder.
    \reentrant
    \ingroup i18n

    The encoder converts Unicode into another format, remembering any
    state that is required between calls.

    \sa QTextCodec::makeEncoder()
*/

/*!
    Destroys the encoder.
*/
QTextEncoder::~QTextEncoder()
{
}

/*!
    \fn QByteArray QTextEncoder::fromUnicode(const QString& uc, int& lenInOut)

    Converts \a lenInOut characters (not bytes) from \a uc, producing
    a QByteArray. \a lenInOut will be set to the \link
    QByteArray::length() length\endlink of the result (in bytes).

    The encoder is free to record state to use when subsequent calls
    are made to this function (for example, it might change modes with
    escape sequences if needed during the encoding of one string, then
    assume that mode applies when a subsequent call begins).
*/
QByteArray QTextEncoder::fromUnicode(const QString& uc, int& lenInOut)
{
    QByteArray result = c->fromUnicode(uc.constData(), lenInOut, &state);
    lenInOut = result.length();
    return result;
}

/*!
    \class QTextDecoder qtextcodec.h
    \brief The QTextDecoder class provides a state-based decoder.
    \reentrant
    \ingroup i18n

    The decoder converts a text format into Unicode, remembering any
    state that is required between calls.

    \sa QTextCodec::makeEncoder()
*/


/*!
    Destroys the decoder.
*/
QTextDecoder::~QTextDecoder()
{
}

/*!
    \fn QString QTextDecoder::toUnicode(const char* chars, int len)

    Converts the first \a len bytes in \a chars to Unicode, returning
    the result.

    If not all characters are used (e.g. if only part of a multi-byte
    encoding is at the end of the characters), the decoder remembers
    enough state to continue with the next call to this function.
*/
QString QTextDecoder::toUnicode(const char* chars, int len)
{
    return c->toUnicode(chars, len, &state);
}

#endif

/* the next two functions are implicitely thread safe,
   as they are only called by setup() which uses a mutex.
*/
static void setupLocaleMapper()
{
#ifdef Q_OS_WIN32
    localeMapper = QTextCodec::codecForName("System");
#else

#if defined (_XOPEN_UNIX) && !defined(Q_OS_QNX6) && !defined(Q_OS_OSF)
    char *charset = nl_langinfo (CODESET);
    if (charset)
      localeMapper = QTextCodec::codecForName(charset);
#endif

    if (!localeMapper) {
        // Very poorly defined and followed standards causes lots of code
        // to try to get all the cases...

        // Try to determine locale codeset from locale name assigned to
        // LC_CTYPE category.

        // First part is getting that locale name.  First try setlocale() which
        // definitely knows it, but since we cannot fully trust it, get ready
        // to fall back to environment variables.
        char * ctype = qstrdup(setlocale(LC_CTYPE, 0));

        // Get the first nonempty value from $LC_ALL, $LC_CTYPE, and $LANG
        // environment variables.
        char * lang = qstrdup(qgetenv("LC_ALL"));
        if (!lang || lang[0] == 0 || strcmp(lang, "C") == 0) {
            if (lang) delete [] lang;
            lang = qstrdup(qgetenv("LC_CTYPE"));
        }
        if (!lang || lang[0] == 0 || strcmp(lang, "C") == 0) {
            if (lang) delete [] lang;
            lang = qstrdup(qgetenv("LANG"));
        }

        // Now try these in order:
        // 1. CODESET from ctype if it contains a .CODESET part (e.g. en_US.ISO8859-15)
        // 2. CODESET from lang if it contains a .CODESET part
        // 3. ctype (maybe the locale is named "ISO-8859-1" or something)
        // 4. locale (ditto)
        // 5. check for "@euro"
        // 6. guess locale from ctype unless ctype is "C"
        // 7. guess locale from lang

        // 1. CODESET from ctype if it contains a .CODESET part (e.g. en_US.ISO8859-15)
        char * codeset = ctype ? strchr(ctype, '.') : 0;
        if (codeset && *codeset == '.')
            localeMapper = QTextCodec::codecForName(codeset + 1);

        // 2. CODESET from lang if it contains a .CODESET part
        codeset = lang ? strchr(lang, '.') : 0;
        if (!localeMapper && codeset && *codeset == '.')
            localeMapper = QTextCodec::codecForName(codeset + 1);

        // 3. ctype (maybe the locale is named "ISO-8859-1" or something)
        if (!localeMapper && ctype && *ctype != 0 && strcmp (ctype, "C") != 0)
            localeMapper = QTextCodec::codecForName(ctype);

        // 4. locale (ditto)
        if (!localeMapper && lang && *lang != 0)
            localeMapper = QTextCodec::codecForName(lang);

        // 5. "@euro"
        if (ctype && strstr(ctype, "@euro") || lang && strstr(lang, "@euro"))
            localeMapper = QTextCodec::codecForName("ISO 8859-15");

        // 6. guess locale from ctype unless ctype is "C"
        // 7. guess locale from lang
        char * try_by_name = ctype;
        if (ctype && *ctype != 0 && strcmp (ctype, "C") != 0)
            try_by_name = lang;

        // Now do the guessing.
        if (lang && *lang && !localeMapper && try_by_name && *try_by_name) {
            if (try_locale_list(iso8859_15locales, lang))
                localeMapper = QTextCodec::codecForName("ISO 8859-15");
            else if (try_locale_list(iso8859_2locales, lang))
                localeMapper = QTextCodec::codecForName("ISO 8859-2");
            else if (try_locale_list(iso8859_3locales, lang))
                localeMapper = QTextCodec::codecForName("ISO 8859-3");
            else if (try_locale_list(iso8859_4locales, lang))
                localeMapper = QTextCodec::codecForName("ISO 8859-4");
            else if (try_locale_list(iso8859_5locales, lang))
                localeMapper = QTextCodec::codecForName("ISO 8859-5");
            else if (try_locale_list(iso8859_6locales, lang))
                localeMapper = QTextCodec::codecForName("ISO 8859-6");
            else if (try_locale_list(iso8859_7locales, lang))
                localeMapper = QTextCodec::codecForName("ISO 8859-7");
            else if (try_locale_list(iso8859_8locales, lang))
                localeMapper = QTextCodec::codecForName("ISO 8859-8-I");
            else if (try_locale_list(iso8859_9locales, lang))
                localeMapper = QTextCodec::codecForName("ISO 8859-9");
            else if (try_locale_list(iso8859_13locales, lang))
                localeMapper = QTextCodec::codecForName("ISO 8859-13");
            else if (try_locale_list(tis_620locales, lang))
                localeMapper = QTextCodec::codecForName("ISO 8859-11");
            else if (try_locale_list(koi8_ulocales, lang))
                localeMapper = QTextCodec::codecForName("KOI8-U");
            else if (try_locale_list(cp_1251locales, lang))
                localeMapper = QTextCodec::codecForName("CP 1251");
            else if (try_locale_list(pt_154locales, lang))
                localeMapper = QTextCodec::codecForName("PT 154");
            else if (try_locale_list(probably_koi8_rlocales, lang))
                localeMapper = ru_RU_hack(lang);
        }

        delete [] ctype;
        delete [] lang;
    }
    if (localeMapper && localeMapper->mibEnum() == 11)
        localeMapper = QTextCodec::codecForName("ISO 8859-8-I");

    // If everything failed, we default to 8859-1
    // We could perhaps default to 8859-15.
    if (!localeMapper)
        localeMapper = QTextCodec::codecForName("ISO 8859-1");
#endif
}


static void realSetup()
{
    if (destroying_is_ok)
        qWarning("QTextCodec: Creating new codec during codec cleanup");
    all = new QList<QTextCodec*>;

    (void)new QLatin1Codec;
    (void)new QLatin15Codec;
    (void)new QUtf8Codec;
    (void)new QUtf16Codec;

#ifndef QT_NO_CODECS
    for (int i = 0; i < QSimpleTextCodec::numSimpleCodecs; ++i)
        (void)new QSimpleTextCodec(i);

    (void)new QTsciiCodec;

    for (int i = 0; i < 9; ++i)
        (void)new QIsciiCodec(i);
#endif // QT_NO_CODECS

#ifdef Q_OS_WIN32
    (void) new QWindowsLocalCodec;
#endif // Q_OS_WIN32

    if (!localeMapper)
        setupLocaleMapper();
}


/*!
    \fn QTextCodec* QTextCodec::codecForTr()

    Returns the codec used by QObject::tr() on its argument. If this
    function returns 0 (the default), tr() assumes Latin1.

    \sa setCodecForTr()
*/

/*!
    \fn void QTextCodec::setCodecForTr(QTextCodec *c)
    \nonreentrant

    Sets the codec used by QObject::tr() on its argument to \a c. If
    \a c is 0 (the default), tr() assumes Latin1.

    If the literal quoted text in the program is not in the Latin1
    encoding, this function can be used to set the appropriate
    encoding. For example, software developed by Korean programmers
    might use eucKR for all the text in the program, in which case the
    main() function might look like this:

    \code
    int main(int argc, char** argv)
    {
        QApplication app(argc, argv);
        ... install any additional codecs ...
        QTextCodec::setCodecForTr(QTextCodec::codecForName("eucKR"));
        ...
    }
    \endcode

    Note that this is not the way to select the encoding that the \e
    user has chosen. For example, to convert an application containing
    literal English strings to Korean, all that is needed is for the
    English strings to be passed through tr() and for translation
    files to be loaded. For details of internationalization, see the
    \link i18n.html Qt internationalization documentation\endlink.

    \sa codecForTr(), setCodecForTr(), setCodecForCStrings()
*/


/*!
    \fn QTextCodec* QTextCodec::codecForCStrings()

    Returns the codec used by QString to convert to and from \c{const
    char*} and QByteArrays. If this function returns 0 (the default),
    QString assumes Latin1.

    \sa setCodecForCStrings()
*/

/*!
    \fn void QTextCodec::setCodecForCStrings(QTextCodec *c)
    \nonreentrant

    Sets the codec used by QString to convert to and from \c{const
    char*} and QByteArrays. If \a c is 0 (the default), QString
    assumes Latin1.

    \warning Some codecs do not preserve the characters in the ASCII
    range (0x00 to 0x7f).  For example, the Japanese Shift-JIS
    encoding maps the backslash character (0x5a) to the Yen character.
    This leads to unexpected results when using the backslash
    character to escape characters in strings used in e.g. regular
    expressions. Use QString::fromLatin1() to preserve characters in
    the ASCII range when needed.

    \sa codecForCStrings(), setCodecForTr(), setCodecForCStrings()
*/


QTextCodec *QTextCodec::cftr = 0;


#endif // QT_NO_TEXTCODEC
