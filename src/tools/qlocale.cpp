#define BAD_DOUBLE_COMPARISON

#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <limits.h>
#include <ctype.h>
#include <ctype.h>
#include <float.h>

#include "qlocale.h"
#include "qlocale_p.h"
#include "qcleanuphandler.h"
#include "qmutex.h"

#ifdef QT_THREAD_SUPPORT
#  include <private/qmutexpool_p.h>
#endif // QT_THREAD_SUPPORT

class QLocaleStaticData
{
public:
    static const double &inf() { init(); return d->m_inf; }
    static const double &nan() { init(); return d->m_nan; }
    static bool bigEndian() { init(); return d->m_big_endian; }
    static const QString &nanStr() { init(); return d->m_nan_str; }
    static const QString &infStr() { init(); return d->m_inf_str; }
    static const void init() {
    	if (d == 0)
	    initStaticData();
    }

private:
    QLocaleStaticData();
    static void initStaticData();

    double m_inf, m_nan;
    bool m_big_endian;
    QString m_nan_str, m_inf_str;
    
    static QLocaleStaticData *d;
};

QLocaleStaticData *QLocaleStaticData::d = 0;

static QCleanupHandler<QLocaleStaticData> g_static_data_cleanup_handler;

void QLocaleStaticData::initStaticData()
{
#ifdef QT_THREAD_SUPPORT
    QMutexLocker locker( qt_global_mutexpool ?
                         qt_global_mutexpool->get(&d) : 0 );
    if (d != 0)
    	return;
#endif // QT_THREAD_SUPPORT

    d = new QLocaleStaticData;
    g_static_data_cleanup_handler.add(&d);
}

QLocaleStaticData::QLocaleStaticData() 
{
    int word_size;
    qSysInfo(&word_size, &m_big_endian);
    
    const unsigned char be_inf_bytes[] = { 0x7f, 0xf0, 0, 0, 0, 0, 0, 0 };
    const unsigned char le_inf_bytes[] = { 0, 0, 0, 0, 0, 0, 0xf0, 0x7f };
    const unsigned char be_nan_bytes[] = { 0x7f, 0xf8, 0, 0, 0, 0, 0, 0 };
    const unsigned char le_nan_bytes[] = { 0, 0, 0, 0, 0, 0, 0xf8, 0x7f };
    
    if (m_big_endian) {
    	m_inf = *(const double*)be_inf_bytes;
    	m_nan = *(const double*)be_nan_bytes;
    } else {
    	m_inf = *(const double*)le_inf_bytes;
    	m_nan = *(const double*)le_nan_bytes;
    }

    m_nan_str = "nan";
    m_inf_str = "inf";
}

#ifdef Q_WS_WIN
#   define isinf(d) (!_finite(d) && !_isnan(d))
#   define isnan(d) _isnan(d)
#   define ULLONG_MAX _UI64_MAX
#   define LLONG_MAX _I64_MAX
#   define LLONG_MIN _I64_MIN
#endif

#if !defined(INFINITY)
#   define INFINITY (QLocaleStaticData::inf())
#endif

#if !defined(NAN)
#   define NAN (QLocaleStaticData::nan())
#endif


// Sizes as defined by the ISO C99 standard - fallback
#ifndef LLONG_MAX
#   define LLONG_MAX Q_INT64_C(9223372036854775807)
#endif
#ifndef LLONG_MIN
#   define LLONG_MIN (-LLONG_MAX - Q_INT64_C(1))
#endif
#ifndef ULLONG_MAX
#   define ULLONG_MAX Q_UINT64_C(18446744073709551615)
#endif

static char *qdtoa(double d, int mode, int ndigits, int *decpt,
    	    	    int *sign, char **rve, char **digits_str);
static double qstrtod(const char *s00, char const **se, bool *ok);
static Q_LLONG qstrtoll(const char *nptr, const char **endptr, register int base, bool *ok);
static Q_ULLONG qstrtoull(const char *nptr, const char **endptr, register int base, bool *ok);

static uint locale_index[] = {
     0, // Default
     0, // C
     0, // Abkhazian
     0, // Afan
     0, // Afar
     1, // Afrikaans
     2, // Albanian
     0, // Amharic
     3, // Arabic
    19, // Armenian
     0, // Assamese
     0, // Aymara
    20, // Azerbaijani
     0, // Bashkir
    21, // Basque
     0, // Bengali
     0, // Bhutani
     0, // Bihari
     0, // Bislama
     0, // Breton
    22, // Bulgarian
     0, // Burmese
    23, // Byelorussian
     0, // Cambodian
    24, // Catalan
    25, // Chinese
     0, // Corsican
    30, // Croatian
    31, // Czech
    32, // Danish
    33, // Dutch
    35, // English
     0, // Esperanto
    47, // Estonian
    48, // Faroese
     0, // Fiji
    49, // Finnish
    50, // French
     0, // Frisian
     0, // Gaelic
    56, // Galician
    57, // Georgian
    58, // German
    63, // Greek
     0, // Greenlandic
     0, // Guarani
    64, // Gujarati
     0, // Hausa
    65, // Hebrew
    66, // Hindi
    67, // Hungarian
    68, // Icelandic
    69, // Indonesian
     0, // Interlingua
     0, // Interlingue
     0, // Inuktitut
     0, // Inupiak
     0, // Irish
    70, // Italian
    72, // Japanese
     0, // Javanese
    73, // Kannada
     0, // Kashmiri
    74, // Kazakh
     0, // Kinyarwanda
    75, // Kirghiz
    76, // Korean
     0, // Kurdish
     0, // Kurundi
     0, // Laothian
     0, // Latin
    77, // Latvian
     0, // Lingala
    78, // Lithuanian
    79, // Macedonian
     0, // Malagasy
    80, // Malay
     0, // Malayalam
     0, // Maltese
     0, // Maori
    82, // Marathi
     0, // Moldavian
    83, // Mongolian
     0, // Nauru
     0, // Nepali
    84, // Norwegian
     0, // Occitan
     0, // Oriya
     0, // Pashto
    85, // Persian
    86, // Polish
    87, // Portuguese
    89, // Punjabi
     0, // Quechua
     0, // RhaetoRomance
    90, // Romanian
    91, // Russian
     0, // Samoan
     0, // Sangho
    92, // Sanskrit
     0, // Serbian
     0, // SerboCroatian
     0, // Sesotho
     0, // Setswana
     0, // Shona
     0, // Sindhi
     0, // Singhalese
     0, // Siswati
    93, // Slovak
    94, // Slovenian
     0, // Somali
    95, // Spanish
     0, // Sundanese
   114, // Swahili
   115, // Swedish
     0, // Tagalog
     0, // Tajik
   117, // Tamil
     0, // Tatar
   118, // Telugu
   119, // Thai
     0, // Tibetan
     0, // Tigrinya
     0, // Tonga
     0, // Tsonga
   120, // Turkish
     0, // Turkmen
     0, // Twi
     0, // Uigur
   121, // Ukrainian
   122, // Urdu
   123, // Uzbek
   124, // Vietnamese
     0, // Volapuk
     0, // Welsh
     0, // Wolof
     0, // Xhosa
     0, // Yiddish
     0, // Yoruba
     0, // Zhuang
     0, // Zulu
     0  // trailing 0
};

static QLocalePrivate locale_data[] = {
//      lang   terr    dec  group   list  prcnt   zero  minus    exp
    {      1,     0,    46,    44,    59,    37,    48,    45,   101 }, // C/Default
    {      5,   195,    46,    44,    44,    37,    48,    45,   101 }, // Afrikaans/SouthAfrica
    {      6,     2,    44,    46,    59,    37,    48,    45,   101 }, // Albanian/Albania
    {      8,   186,    46,    44,    59,    37,  1632,    45,   101 }, // Arabic/SaudiArabia
    {      8,     3,    46,    44,    59,    37,    48,    45,   101 }, // Arabic/Algeria
    {      8,    17,    46,    44,    59,    37,  1632,    45,   101 }, // Arabic/Bahrain
    {      8,    64,    46,    44,    59,    37,  1632,    45,   101 }, // Arabic/Egypt
    {      8,   103,    46,    44,    59,    37,  1632,    45,   101 }, // Arabic/Iraq
    {      8,   109,    46,    44,    59,    37,  1632,    45,   101 }, // Arabic/Jordan
    {      8,   115,    46,    44,    59,    37,  1632,    45,   101 }, // Arabic/Kuwait
    {      8,   119,    46,    44,    59,    37,  1632,    45,   101 }, // Arabic/Lebanon
    {      8,   122,    46,    44,    59,    37,    48,    45,   101 }, // Arabic/LibyanArabJamahiriya
    {      8,   145,    46,    44,    59,    37,    48,    45,   101 }, // Arabic/Morocco
    {      8,   162,    46,    44,    59,    37,  1632,    45,   101 }, // Arabic/Oman
    {      8,   175,    46,    44,    59,    37,  1632,    45,   101 }, // Arabic/Qatar
    {      8,   207,    46,    44,    59,    37,  1632,    45,   101 }, // Arabic/SyrianArabRepublic
    {      8,   216,    46,    44,    59,    37,    48,    45,   101 }, // Arabic/Tunisia
    {      8,   223,    46,    44,    59,    37,  1632,    45,   101 }, // Arabic/UnitedArabEmirates
    {      8,   237,    46,    44,    59,    37,  1632,    45,   101 }, // Arabic/Yemen
    {      9,    11,    46,    44,    44,    37,    48,    45,   101 }, // Armenian/Armenia
    {     12,    15,    44,   160,    59,    37,    48,    45,   101 }, // Azerbaijani/Azerbaijan
    {     14,   197,    44,    46,    59,    37,    48,    45,   101 }, // Basque/Spain
    {     20,    33,    44,   160,    59,    37,    48,    45,   101 }, // Bulgarian/Bulgaria
    {     22,    20,    44,   160,    59,    37,    48,    45,   101 }, // Byelorussian/Belarus
    {     24,   197,    44,    46,    59,    37,    48,    45,   101 }, // Catalan/Spain
    {     25,    44,    46,    44,    44,    37,    48,    45,   101 }, // Chinese/China
    {     25,    97,    46,    44,    44,    37,    48,    45,   101 }, // Chinese/HongKong
    {     25,   126,    46,    44,    44,    37,    48,    45,   101 }, // Chinese/Macau
    {     25,   190,    46,    44,    44,    37,    48,    45,   101 }, // Chinese/Singapore
    {     25,   208,    46,    44,    44,    37,    48,    45,   101 }, // Chinese/Taiwan
    {     27,    54,    44,    46,    59,    37,    48,    45,   101 }, // Croatian/Croatia
    {     28,    57,    44,   160,    59,    37,    48,    45,   101 }, // Czech/CzechRepublic
    {     29,    58,    44,    46,    59,    37,    48,    45,   101 }, // Danish/Denmark
    {     30,   151,    44,    46,    59,    37,    48,    45,   101 }, // Dutch/Netherlands
    {     30,    21,    44,    46,    59,    37,    48,    45,   101 }, // Dutch/Belgium
    {     31,   225,    46,    44,    44,    37,    48,    45,   101 }, // English/UnitedStates
    {     31,    13,    46,    44,    44,    37,    48,    45,   101 }, // English/Australia
    {     31,    22,    46,    44,    59,    37,    48,    45,   101 }, // English/Belize
    {     31,    38,    46,    44,    44,    37,    48,    45,   101 }, // English/Canada
    {     31,   104,    46,    44,    44,    37,    48,    45,   101 }, // English/Ireland
    {     31,   107,    46,    44,    44,    37,    48,    45,   101 }, // English/Jamaica
    {     31,   154,    46,    44,    44,    37,    48,    45,   101 }, // English/NewZealand
    {     31,   170,    46,    44,    44,    37,    48,    45,   101 }, // English/Philippines
    {     31,   195,    46,    44,    44,    37,    48,    45,   101 }, // English/SouthAfrica
    {     31,   215,    46,    44,    59,    37,    48,    45,   101 }, // English/TrinidadAndTobago
    {     31,   224,    46,    44,    44,    37,    48,    45,   101 }, // English/UnitedKingdom
    {     31,   240,    46,    44,    44,    37,    48,    45,   101 }, // English/Zimbabwe
    {     33,    68,    44,   160,    59,    37,    48,    45,   101 }, // Estonian/Estonia
    {     34,    71,    44,    46,    59,    37,    48,    45,   101 }, // Faroese/FaroeIslands
    {     36,    73,    44,   160,    59,    37,    48,    45,   101 }, // Finnish/Finland
    {     37,    74,    44,   160,    59,    37,    48,    45,   101 }, // French/France
    {     37,    21,    44,    46,    59,    37,    48,    45,   101 }, // French/Belgium
    {     37,    38,    44,   160,    59,    37,    48,    45,   101 }, // French/Canada
    {     37,   125,    44,   160,    59,    37,    48,    45,   101 }, // French/Luxembourg
    {     37,   142,    44,   160,    59,    37,    48,    45,   101 }, // French/Monaco
    {     37,   206,    46,    39,    59,    37,    48,    45,   101 }, // French/Switzerland
    {     40,   197,    44,    46,    44,    37,    48,    45,   101 }, // Galician/Spain
    {     41,    81,    44,   160,    59,    37,    48,    45,   101 }, // Georgian/Georgia
    {     42,    82,    44,    46,    59,    37,    48,    45,   101 }, // German/Germany
    {     42,    14,    44,    46,    59,    37,    48,    45,   101 }, // German/Austria
    {     42,   123,    46,    39,    59,    37,    48,    45,   101 }, // German/Liechtenstein
    {     42,   125,    44,    46,    59,    37,    48,    45,   101 }, // German/Luxembourg
    {     42,   206,    46,    39,    59,    37,    48,    45,   101 }, // German/Switzerland
    {     43,    85,    44,    46,    59,    37,    48,    45,   101 }, // Greek/Greece
    {     46,   100,    46,    44,    44,    37,  2790,    45,   101 }, // Gujarati/India
    {     48,   105,    46,    44,    44,    37,    48,    45,   101 }, // Hebrew/Israel
    {     49,   100,    46,    44,    44,    37,    48,    45,   101 }, // Hindi/India
    {     50,    98,    44,   160,    59,    37,    48,    45,   101 }, // Hungarian/Hungary
    {     51,    99,    44,    46,    59,    37,    48,    45,   101 }, // Icelandic/Iceland
    {     52,   101,    44,    46,    59,    37,    48,    45,   101 }, // Indonesian/Indonesia
    {     58,   106,    44,    46,    59,    37,    48,    45,   101 }, // Italian/Italy
    {     58,   206,    46,    39,    59,    37,    48,    45,   101 }, // Italian/Switzerland
    {     59,   108,    46,    44,    44,    37,    48,    45,   101 }, // Japanese/Japan
    {     61,   100,    46,    44,    44,    37,  3302,    45,   101 }, // Kannada/India
    {     63,   110,    44,   160,    59,    37,    48,    45,   101 }, // Kazakh/Kazakhstan
    {     65,   116,    44,   160,    59,    37,    48,    45,   101 }, // Kirghiz/Kyrgyzstan
    {     66,   114,    46,    44,    44,    37,    48,    45,   101 }, // Korean/RepublicOfKorea
    {     71,   118,    44,   160,    59,    37,    48,    45,   101 }, // Latvian/Latvia
    {     73,   124,    44,    46,    59,    37,    48,    45,   101 }, // Lithuanian/Lithuania
    {     74,   127,    44,    46,    59,    37,    48,    45,   101 }, // Macedonian/Macedonia
    {     76,   130,    44,    46,    59,    37,    48,    45,   101 }, // Malay/Malaysia
    {     76,    32,    44,    46,    59,    37,    48,    45,   101 }, // Malay/BruneiDarussalam
    {     80,   100,    46,    44,    44,    37,  2406,    45,   101 }, // Marathi/India
    {     82,   143,    44,   160,    59,    37,    48,    45,   101 }, // Mongolian/Mongolia
    {     85,   161,    44,   160,    59,    37,    48,    45,   101 }, // Norwegian/Norway
    {     89,   102,    46,    44,    59,    37,  1776,    45,   101 }, // Persian/Iran
    {     90,   172,    44,   160,    59,    37,    48,    45,   101 }, // Polish/Poland
    {     91,   173,    44,    46,    59,    37,    48,    45,   101 }, // Portuguese/Portugal
    {     91,    30,    44,    46,    59,    37,    48,    45,   101 }, // Portuguese/Brazil
    {     92,   100,    46,    44,    44,    37,  2662,    45,   101 }, // Punjabi/India
    {     95,   177,    44,    46,    59,    37,    48,    45,   101 }, // Romanian/Romania
    {     96,   178,    44,   160,    59,    37,    48,    45,   101 }, // Russian/RussianFederation
    {     99,   100,    46,    44,    44,    37,  2406,    45,   101 }, // Sanskrit/India
    {    108,   191,    44,   160,    59,    37,    48,    45,   101 }, // Slovak/Slovakia
    {    109,   192,    44,    46,    59,    37,    48,    45,   101 }, // Slovenian/Slovenia
    {    111,   197,    44,    46,    59,    37,    48,    45,   101 }, // Spanish/Spain
    {    111,    10,    44,    46,    44,    37,    48,    45,   101 }, // Spanish/Argentina
    {    111,    26,    44,    46,    44,    37,    48,    45,   101 }, // Spanish/Bolivia
    {    111,    43,    44,    46,    44,    37,    48,    45,   101 }, // Spanish/Chile
    {    111,    47,    44,    46,    44,    37,    48,    45,   101 }, // Spanish/Colombia
    {    111,    52,    44,    46,    44,    37,    48,    45,   101 }, // Spanish/CostaRica
    {    111,    61,    46,    44,    44,    37,    48,    45,   101 }, // Spanish/DominicanRepublic
    {    111,    63,    44,    46,    44,    37,    48,    45,   101 }, // Spanish/Ecuador
    {    111,    65,    46,    44,    44,    37,    48,    45,   101 }, // Spanish/ElSalvador
    {    111,    90,    46,    44,    44,    37,    48,    45,   101 }, // Spanish/Guatemala
    {    111,    96,    46,    44,    44,    37,    48,    45,   101 }, // Spanish/Honduras
    {    111,   139,    46,    44,    44,    37,    48,    45,   101 }, // Spanish/Mexico
    {    111,   155,    46,    44,    44,    37,    48,    45,   101 }, // Spanish/Nicaragua
    {    111,   166,    46,    44,    44,    37,    48,    45,   101 }, // Spanish/Panama
    {    111,   168,    44,    46,    44,    37,    48,    45,   101 }, // Spanish/Paraguay
    {    111,   169,    46,    44,    44,    37,    48,    45,   101 }, // Spanish/Peru
    {    111,   174,    46,    44,    44,    37,    48,    45,   101 }, // Spanish/PuertoRico
    {    111,   227,    44,    46,    44,    37,    48,    45,   101 }, // Spanish/Uruguay
    {    111,   231,    44,    46,    44,    37,    48,    45,   101 }, // Spanish/Venezuela
    {    113,   111,    46,    44,    44,    37,    48,    45,   101 }, // Swahili/Kenya
    {    114,   205,    44,   160,    59,    37,    48,    45,   101 }, // Swedish/Sweden
    {    114,    73,    44,   160,    59,    37,    48,    45,   101 }, // Swedish/Finland
    {    117,   100,    46,    44,    44,    37,    48,    45,   101 }, // Tamil/India
    {    119,   100,    46,    44,    44,    37,  3174,    45,   101 }, // Telugu/India
    {    120,   211,    46,    44,    44,    37,  3664,    45,   101 }, // Thai/Thailand
    {    125,   217,    44,    46,    59,    37,    48,    45,   101 }, // Turkish/Turkey
    {    129,   222,    44,   160,    59,    37,    48,    45,   101 }, // Ukrainian/Ukraine
    {    130,   163,    46,    44,    59,    37,  1776,    45,   101 }, // Urdu/Pakistan
    {    131,   228,    44,   160,    59,    37,    48,    45,   101 }, // Uzbek/Uzbekistan
    {    132,   232,    44,    46,    44,    37,    48,    45,   101 }, // Vietnamese/VietNam
    {      0,     0,     0,     0,     0,     0,     0,     0,     0 }  // trailing 0s
};

const QLocalePrivate *QLocale::default_d = 0;

const QString &QLocalePrivate::infinity() const
{
    return QLocaleStaticData::infStr();
}

const QString &QLocalePrivate::nan() const
{
    return QLocaleStaticData::nanStr();
}

static QLocalePrivate *findLocale(QLocale::Language language,
    	    	    	    	    QLocale::Country country)
{
    unsigned language_id = (unsigned)language;
    unsigned country_id = (unsigned)country;

    uint idx = locale_index[language_id];

    QLocalePrivate *d = locale_data + idx;

    if (idx == 0) // default language has no associated country
    	return d;

    if (country == QLocale::DefaultCountry)
    	return d;

    Q_ASSERT(d->languageId() == language_id);

    while (d->languageId() == language_id
    	    	&& d->countryId() != country_id)
	++d;

    if (d->countryId() == country_id
    	    && d->languageId() == language_id)
	return d;

    return locale_data + idx;
}

/*!
    \class QLocale

    \brief The QLocale class converts between numbers and their
    string representations in various languages.

    It is initialized with a country/language pair in its constructor
    and offers number-to-string and string-to-number conversion
    functions simmilar to those in QString.

    \code
    QLocale l(QLocale::Arabic, QLocale::Egypt);
    QString s1 = l.toString(1.571429E+07, 'e');
    QString s2 = l.toString(10);

    double d = l.toDouble(s1);
    int s2 = l.toInt(s2);
    \endcode

    Additionally, QLocale supports
    the concept of a default locale, which can be set with
    the static member setDefaultLocale(). This allows a locale to
    be set globally for the entire application.

    \code
    QLocale::setDefaultLocale(QLocale::German, QLocale::Switzerland);

    QLocale l; // Ctor parms default to DefaultLanguage/DefaultCountry
    QString s1 = l.toString(15714.3, 'e');
    \endcode

    QLocale uses a database which currently contains 125 language/country
    pairs. When a langauge/country pair is specified in the constructor,
    one of three things may happen:

    \list
    \i if DefaultLanguage/DefaultCountry are specified, QLocale uses values
    	set using setDefaultLocale(). If no default values are set, QLocale defaults
	to "C",
    \i if the language/country pair is found in the database, it is used,
    \i if the language is found but the country is not, or if the country
    	is DefaultCountry, the language is used with the most appropriate
	available country,
    \i if neither the language nor the country are found, QLocale
    	defaults to "C".
    \endlist

    All the methods in QLocale, with the exception of setDefaultLocale(),
    are reentrant.
*/

/*!
    \enum QLocale::Language

    This enumerated type is used to specify a language.

    \value DefaultLanguage
    \value C
    \value Abkhazian
    \value Afan
    \value Afar
    \value Afrikaans
    \value Albanian
    \value Amharic
    \value Arabic
    \value Armenian
    \value Assamese
    \value Aymara
    \value Azerbaijani
    \value Bashkir
    \value Basque
    \value Bengali
    \value Bhutani
    \value Bihari
    \value Bislama
    \value Breton
    \value Bulgarian
    \value Burmese
    \value Byelorussian
    \value Cambodian
    \value Catalan
    \value Chinese
    \value Corsican
    \value Croatian
    \value Czech
    \value Danish
    \value Dutch
    \value English
    \value Esperanto
    \value Estonian
    \value Faroese
    \value FijiLanguage
    \value Finnish
    \value French
    \value Frisian
    \value Gaelic
    \value Galician
    \value Georgian
    \value German
    \value Greek
    \value Greenlandic
    \value Guarani
    \value Gujarati
    \value Hausa
    \value Hebrew
    \value Hindi
    \value Hungarian
    \value Icelandic
    \value Indonesian
    \value Interlingua
    \value Interlingue
    \value Inuktitut
    \value Inupiak
    \value Irish
    \value Italian
    \value Japanese
    \value Javanese
    \value Kannada
    \value Kashmiri
    \value Kazakh
    \value Kinyarwanda
    \value Kirghiz
    \value Korean
    \value Kurdish
    \value Kurundi
    \value Laothian
    \value Latin
    \value Latvian
    \value Lingala
    \value Lithuanian
    \value Macedonian
    \value Malagasy
    \value Malay
    \value Malayalam
    \value Maltese
    \value Maori
    \value Marathi
    \value Moldavian
    \value Mongolian
    \value NauruLanguage
    \value Nepali
    \value Norwegian
    \value Occitan
    \value Oriya
    \value Pashto
    \value Persian
    \value Polish
    \value Portuguese
    \value Punjabi
    \value Quechua
    \value RhaetoRomance
    \value Romanian
    \value Russian
    \value Samoan
    \value Sangho
    \value Sanskrit
    \value Serbian
    \value SerboCroatian
    \value Sesotho
    \value Setswana
    \value Shona
    \value Sindhi
    \value Singhalese
    \value Siswati
    \value Slovak
    \value Slovenian
    \value Somali
    \value Spanish
    \value Sundanese
    \value Swahili
    \value Swedish
    \value Tagalog
    \value Tajik
    \value Tamil
    \value Tatar
    \value Telugu
    \value Thai
    \value Tibetan
    \value Tigrinya
    \value TongaLanguage
    \value Tsonga
    \value Turkish
    \value Turkmen
    \value Twi
    \value Uigur
    \value Ukrainian
    \value Urdu
    \value Uzbek
    \value Vietnamese
    \value Volapuk
    \value Welsh
    \value Wolof
    \value Xhosa
    \value Yiddish
    \value Yoruba
    \value Zhuang
    \value Zulu
*/

/*!
    \enum QLocale::Country

    This enumerated type is used to specify a country.

    \value DefaultCountry
    \value Afghanistan
    \value Albania
    \value Algeria
    \value AmericanSamoa
    \value Andorra
    \value Angola
    \value Anguilla
    \value Antarctica
    \value AntiguaAndBarbuda
    \value Argentina
    \value Armenia
    \value Aruba
    \value Australia
    \value Austria
    \value Azerbaijan
    \value Bahamas
    \value Bahrain
    \value Bangladesh
    \value Barbados
    \value Belarus
    \value Belgium
    \value Belize
    \value Benin
    \value Bermuda
    \value Bhutan
    \value Bolivia
    \value BosniaAndHerzegowina
    \value Botswana
    \value BouvetIsland
    \value Brazil
    \value BritishIndianOceanTerritory
    \value BruneiDarussalam
    \value Bulgaria
    \value BurkinaFaso
    \value Burundi
    \value Cambodia
    \value Cameroon
    \value Canada
    \value CapeVerde
    \value CaymanIslands
    \value CentralAfricanRepublic
    \value Chad
    \value Chile
    \value China
    \value ChristmasIsland
    \value CocosIslands
    \value Colombia
    \value Comoros
    \value DemocraticRepublicOfCongo
    \value PeoplesRepublicOfCongo
    \value CookIslands
    \value CostaRica
    \value IvoryCoast
    \value Croatia
    \value Cuba
    \value Cyprus
    \value CzechRepublic
    \value Denmark
    \value Djibouti
    \value Dominica
    \value DominicanRepublic
    \value EastTimor
    \value Ecuador
    \value Egypt
    \value ElSalvador
    \value EquatorialGuinea
    \value Eritrea
    \value Estonia
    \value Ethiopia
    \value FalklandIslands
    \value FaroeIslands
    \value FijiCountry
    \value Finland
    \value France
    \value MetropolitanFrance
    \value FrenchGuiana
    \value FrenchPolynesia
    \value FrenchSouthernTerritories
    \value Gabon
    \value Gambia
    \value Georgia
    \value Germany
    \value Ghana
    \value Gibraltar
    \value Greece
    \value Greenland
    \value Grenada
    \value Guadeloupe
    \value Guam
    \value Guatemala
    \value Guinea
    \value GuineaBissau
    \value Guyana
    \value Haiti
    \value HeardAndMcDonaldIslands
    \value Honduras
    \value HongKong
    \value Hungary
    \value Iceland
    \value India
    \value Indonesia
    \value Iran
    \value Iraq
    \value Ireland
    \value Israel
    \value Italy
    \value Jamaica
    \value Japan
    \value Jordan
    \value Kazakhstan
    \value Kenya
    \value Kiribati
    \value DemocraticRepublicOfKorea
    \value RepublicOfKorea
    \value Kuwait
    \value Kyrgyzstan
    \value Lao
    \value Latvia
    \value Lebanon
    \value Lesotho
    \value Liberia
    \value LibyanArabJamahiriya
    \value Liechtenstein
    \value Lithuania
    \value Luxembourg
    \value Macau
    \value Macedonia
    \value Madagascar
    \value Malawi
    \value Malaysia
    \value Maldives
    \value Mali
    \value Malta
    \value MarshallIslands
    \value Martinique
    \value Mauritania
    \value Mauritius
    \value Mayotte
    \value Mexico
    \value Micronesia
    \value Moldova
    \value Monaco
    \value Mongolia
    \value Montserrat
    \value Morocco
    \value Mozambique
    \value Myanmar
    \value Namibia
    \value NauruCountry
    \value Nepal
    \value Netherlands
    \value NetherlandsAntilles
    \value NewCaledonia
    \value NewZealand
    \value Nicaragua
    \value Niger
    \value Nigeria
    \value Niue
    \value NorfolkIsland
    \value NorthernMarianaIslands
    \value Norway
    \value Oman
    \value Pakistan
    \value Palau
    \value PalestinianTerritory
    \value Panama
    \value PapuaNewGuinea
    \value Paraguay
    \value Peru
    \value Philippines
    \value Pitcairn
    \value Poland
    \value Portugal
    \value PuertoRico
    \value Qatar
    \value Reunion
    \value Romania
    \value RussianFederation
    \value Rwanda
    \value SaintKittsAndNevis
    \value StLucia
    \value StVincentAndTheGrenadines
    \value Samoa
    \value SanMarino
    \value SaoTomeAndPrincipe
    \value SaudiArabia
    \value Senegal
    \value Seychelles
    \value SierraLeone
    \value Singapore
    \value Slovakia
    \value Slovenia
    \value SolomonIslands
    \value Somalia
    \value SouthAfrica
    \value SouthGeorgiaAndTheSouthSandwichIslands
    \value Spain
    \value SriLanka
    \value StHelena
    \value StPierreAndMiquelon
    \value Sudan
    \value Suriname
    \value SvalbardAndJanMayenIslands
    \value Swaziland
    \value Sweden
    \value Switzerland
    \value SyrianArabRepublic
    \value Taiwan
    \value Tajikistan
    \value Tanzania
    \value Thailand
    \value Togo
    \value Tokelau
    \value TongaCountry
    \value TrinidadAndTobago
    \value Tunisia
    \value Turkey
    \value Turkmenistan
    \value TurksAndCaicosIslands
    \value Tuvalu
    \value Uganda
    \value Ukraine
    \value UnitedArabEmirates
    \value UnitedKingdom
    \value UnitedStates
    \value UnitedStatesMinorOutlyingIslands
    \value Uruguay
    \value Uzbekistan
    \value Vanuatu
    \value VaticanCityState
    \value Venezuela
    \value VietNam
    \value BritishVirginIslands
    \value USVirginIslands
    \value WallisAndFutunaIslands
    \value WesternSahara
    \value Yemen
    \value Yugoslavia
    \value Zambia
    \value Zimbabwe
*/

/*!
    Constructs a QLocale object with specified \a language and \a country.
    If \a language is DefaultLanguage and \a country is
    DefaultCountry, the values set with setDefaultLocale() are
    used. If setDefaultLocale() was not previously called, the "C" locale
    is used.

    QLocale might use different language/country values than specified in
    the constructor, depending on
    what's available in the database. The language and country actually
    used may be queried with language() and country().

    \sa setDefaultLocale() language() country()
*/

QLocale::QLocale(Language language, Country country)
{
    if (default_d == 0)
    	default_d = locale_data;

    if (language == DefaultLanguage) {
    	d = default_d;
	return;
    }

    d = findLocale(language, country);
}

/*!
    Sets the global default locale to \a language / \a country. These values
    are used when a QLocale object is constructed with DefaultLanguage/DefaultCountry.

    \warning This method is not reentrant. The default locale should be
    set at application startup, before any new threads are created.

    \sa QLocale()
*/

void QLocale::setDefaultLocale(Language language, Country country)
{
    if (language == DefaultLanguage)
	return;

    default_d = findLocale(language, country);
}

/*!
    Returns the language of this locale.

    \sa QLocale()
*/
QLocale::Language QLocale::language() const
{
    return (Language)d->languageId();
}

/*!
    Returns the country of this locale.

    \sa QLocale()
*/
QLocale::Country QLocale::country() const
{
    return (Country)d->countryId();
}

/*!
    Returns a QString containing the name of \a language.
*/

QString QLocale::languageToString(Language language)
{
    switch (language) {
	case DefaultLanguage: return "DefaultLanguage";
	case C: return "C";
	case Abkhazian: return "Abkhazian";
	case Afan: return "Afan";
	case Afar: return "Afar";
	case Afrikaans: return "Afrikaans";
	case Albanian: return "Albanian";
	case Amharic: return "Amharic";
	case Arabic: return "Arabic";
	case Armenian: return "Armenian";
	case Assamese: return "Assamese";
	case Aymara: return "Aymara";
	case Azerbaijani: return "Azerbaijani";
	case Bashkir: return "Bashkir";
	case Basque: return "Basque";
	case Bengali: return "Bengali";
	case Bhutani: return "Bhutani";
	case Bihari: return "Bihari";
	case Bislama: return "Bislama";
	case Breton: return "Breton";
	case Bulgarian: return "Bulgarian";
	case Burmese: return "Burmese";
	case Byelorussian: return "Byelorussian";
	case Cambodian: return "Cambodian";
	case Catalan: return "Catalan";
	case Chinese: return "Chinese";
	case Corsican: return "Corsican";
	case Croatian: return "Croatian";
	case Czech: return "Czech";
	case Danish: return "Danish";
	case Dutch: return "Dutch";
	case English: return "English";
	case Esperanto: return "Esperanto";
	case Estonian: return "Estonian";
	case Faroese: return "Faroese";
	case FijiLanguage: return "FijiLanguage";
	case Finnish: return "Finnish";
	case French: return "French";
	case Frisian: return "Frisian";
	case Gaelic: return "Gaelic";
	case Galician: return "Galician";
	case Georgian: return "Georgian";
	case German: return "German";
	case Greek: return "Greek";
	case Greenlandic: return "Greenlandic";
	case Guarani: return "Guarani";
	case Gujarati: return "Gujarati";
	case Hausa: return "Hausa";
	case Hebrew: return "Hebrew";
	case Hindi: return "Hindi";
	case Hungarian: return "Hungarian";
	case Icelandic: return "Icelandic";
	case Indonesian: return "Indonesian";
	case Interlingua: return "Interlingua";
	case Interlingue: return "Interlingue";
	case Inuktitut: return "Inuktitut";
	case Inupiak: return "Inupiak";
	case Irish: return "Irish";
	case Italian: return "Italian";
	case Japanese: return "Japanese";
	case Javanese: return "Javanese";
	case Kannada: return "Kannada";
	case Kashmiri: return "Kashmiri";
	case Kazakh: return "Kazakh";
	case Kinyarwanda: return "Kinyarwanda";
	case Kirghiz: return "Kirghiz";
	case Korean: return "Korean";
	case Kurdish: return "Kurdish";
	case Kurundi: return "Kurundi";
	case Laothian: return "Laothian";
	case Latin: return "Latin";
	case Latvian: return "Latvian";
	case Lingala: return "Lingala";
	case Lithuanian: return "Lithuanian";
	case Macedonian: return "Macedonian";
	case Malagasy: return "Malagasy";
	case Malay: return "Malay";
	case Malayalam: return "Malayalam";
	case Maltese: return "Maltese";
	case Maori: return "Maori";
	case Marathi: return "Marathi";
	case Moldavian: return "Moldavian";
	case Mongolian: return "Mongolian";
	case NauruLanguage: return "NauruLanguage";
	case Nepali: return "Nepali";
	case Norwegian: return "Norwegian";
	case Occitan: return "Occitan";
	case Oriya: return "Oriya";
	case Pashto: return "Pashto";
	case Persian: return "Persian";
	case Polish: return "Polish";
	case Portuguese: return "Portuguese";
	case Punjabi: return "Punjabi";
	case Quechua: return "Quechua";
	case RhaetoRomance: return "RhaetoRomance";
	case Romanian: return "Romanian";
	case Russian: return "Russian";
	case Samoan: return "Samoan";
	case Sangho: return "Sangho";
	case Sanskrit: return "Sanskrit";
	case Serbian: return "Serbian";
	case SerboCroatian: return "SerboCroatian";
	case Sesotho: return "Sesotho";
	case Setswana: return "Setswana";
	case Shona: return "Shona";
	case Sindhi: return "Sindhi";
	case Singhalese: return "Singhalese";
	case Siswati: return "Siswati";
	case Slovak: return "Slovak";
	case Slovenian: return "Slovenian";
	case Somali: return "Somali";
	case Spanish: return "Spanish";
	case Sundanese: return "Sundanese";
	case Swahili: return "Swahili";
	case Swedish: return "Swedish";
	case Tagalog: return "Tagalog";
	case Tajik: return "Tajik";
	case Tamil: return "Tamil";
	case Tatar: return "Tatar";
	case Telugu: return "Telugu";
	case Thai: return "Thai";
	case Tibetan: return "Tibetan";
	case Tigrinya: return "Tigrinya";
	case TongaLanguage: return "TongaLanguage";
	case Tsonga: return "Tsonga";
	case Turkish: return "Turkish";
	case Turkmen: return "Turkmen";
	case Twi: return "Twi";
	case Uigur: return "Uigur";
	case Ukrainian: return "Ukrainian";
	case Urdu: return "Urdu";
	case Uzbek: return "Uzbek";
	case Vietnamese: return "Vietnamese";
	case Volapuk: return "Volapuk";
	case Welsh: return "Welsh";
	case Wolof: return "Wolof";
	case Xhosa: return "Xhosa";
	case Yiddish: return "Yiddish";
	case Yoruba: return "Yoruba";
	case Zhuang: return "Zhuang";
	case Zulu: return "Zulu";
    }

    return "Unknown";
}

/*!
    Returns a QString containing the name of \a country.
*/

QString QLocale::countryToString(Country country)
{
    switch (country) {
    	case DefaultCountry: return "DefaultCountry";
	case Afghanistan: return "Afghanistan";
	case Albania: return "Albania";
	case Algeria: return "Algeria";
	case AmericanSamoa: return "AmericanSamoa";
	case Andorra: return "Andorra";
	case Angola: return "Angola";
	case Anguilla: return "Anguilla";
	case Antarctica: return "Antarctica";
	case AntiguaAndBarbuda: return "AntiguaAndBarbuda";
	case Argentina: return "Argentina";
	case Armenia: return "Armenia";
	case Aruba: return "Aruba";
	case Australia: return "Australia";
	case Austria: return "Austria";
	case Azerbaijan: return "Azerbaijan";
	case Bahamas: return "Bahamas";
	case Bahrain: return "Bahrain";
	case Bangladesh: return "Bangladesh";
	case Barbados: return "Barbados";
	case Belarus: return "Belarus";
	case Belgium: return "Belgium";
	case Belize: return "Belize";
	case Benin: return "Benin";
	case Bermuda: return "Bermuda";
	case Bhutan: return "Bhutan";
	case Bolivia: return "Bolivia";
	case BosniaAndHerzegowina: return "BosniaAndHerzegowina";
	case Botswana: return "Botswana";
	case BouvetIsland: return "BouvetIsland";
	case Brazil: return "Brazil";
	case BritishIndianOceanTerritory: return "BritishIndianOceanTerritory";
	case BruneiDarussalam: return "BruneiDarussalam";
	case Bulgaria: return "Bulgaria";
	case BurkinaFaso: return "BurkinaFaso";
	case Burundi: return "Burundi";
	case Cambodia: return "Cambodia";
	case Cameroon: return "Cameroon";
	case Canada: return "Canada";
	case CapeVerde: return "CapeVerde";
	case CaymanIslands: return "CaymanIslands";
	case CentralAfricanRepublic: return "CentralAfricanRepublic";
	case Chad: return "Chad";
	case Chile: return "Chile";
	case China: return "China";
	case ChristmasIsland: return "ChristmasIsland";
	case CocosIslands: return "CocosIslands";
	case Colombia: return "Colombia";
	case Comoros: return "Comoros";
	case DemocraticRepublicOfCongo: return "DemocraticRepublicOfCongo";
	case PeoplesRepublicOfCongo: return "PeoplesRepublicOfCongo";
	case CookIslands: return "CookIslands";
	case CostaRica: return "CostaRica";
	case IvoryCoast: return "IvoryCoast";
	case Croatia: return "Croatia";
	case Cuba: return "Cuba";
	case Cyprus: return "Cyprus";
	case CzechRepublic: return "CzechRepublic";
	case Denmark: return "Denmark";
	case Djibouti: return "Djibouti";
	case Dominica: return "Dominica";
	case DominicanRepublic: return "DominicanRepublic";
	case EastTimor: return "EastTimor";
	case Ecuador: return "Ecuador";
	case Egypt: return "Egypt";
	case ElSalvador: return "ElSalvador";
	case EquatorialGuinea: return "EquatorialGuinea";
	case Eritrea: return "Eritrea";
	case Estonia: return "Estonia";
	case Ethiopia: return "Ethiopia";
	case FalklandIslands: return "FalklandIslands";
	case FaroeIslands: return "FaroeIslands";
	case FijiCountry: return "FijiCountry";
	case Finland: return "Finland";
	case France: return "France";
	case MetropolitanFrance: return "MetropolitanFrance";
	case FrenchGuiana: return "FrenchGuiana";
	case FrenchPolynesia: return "FrenchPolynesia";
	case FrenchSouthernTerritories: return "FrenchSouthernTerritories";
	case Gabon: return "Gabon";
	case Gambia: return "Gambia";
	case Georgia: return "Georgia";
	case Germany: return "Germany";
	case Ghana: return "Ghana";
	case Gibraltar: return "Gibraltar";
	case Greece: return "Greece";
	case Greenland: return "Greenland";
	case Grenada: return "Grenada";
	case Guadeloupe: return "Guadeloupe";
	case Guam: return "Guam";
	case Guatemala: return "Guatemala";
	case Guinea: return "Guinea";
	case GuineaBissau: return "GuineaBissau";
	case Guyana: return "Guyana";
	case Haiti: return "Haiti";
	case HeardAndMcDonaldIslands: return "HeardAndMcDonaldIslands";
	case Honduras: return "Honduras";
	case HongKong: return "HongKong";
	case Hungary: return "Hungary";
	case Iceland: return "Iceland";
	case India: return "India";
	case Indonesia: return "Indonesia";
	case Iran: return "Iran";
	case Iraq: return "Iraq";
	case Ireland: return "Ireland";
	case Israel: return "Israel";
	case Italy: return "Italy";
	case Jamaica: return "Jamaica";
	case Japan: return "Japan";
	case Jordan: return "Jordan";
	case Kazakhstan: return "Kazakhstan";
	case Kenya: return "Kenya";
	case Kiribati: return "Kiribati";
	case DemocraticRepublicOfKorea: return "DemocraticRepublicOfKorea";
	case RepublicOfKorea: return "RepublicOfKorea";
	case Kuwait: return "Kuwait";
	case Kyrgyzstan: return "Kyrgyzstan";
	case Lao: return "Lao";
	case Latvia: return "Latvia";
	case Lebanon: return "Lebanon";
	case Lesotho: return "Lesotho";
	case Liberia: return "Liberia";
	case LibyanArabJamahiriya: return "LibyanArabJamahiriya";
	case Liechtenstein: return "Liechtenstein";
	case Lithuania: return "Lithuania";
	case Luxembourg: return "Luxembourg";
	case Macau: return "Macau";
	case Macedonia: return "Macedonia";
	case Madagascar: return "Madagascar";
	case Malawi: return "Malawi";
	case Malaysia: return "Malaysia";
	case Maldives: return "Maldives";
	case Mali: return "Mali";
	case Malta: return "Malta";
	case MarshallIslands: return "MarshallIslands";
	case Martinique: return "Martinique";
	case Mauritania: return "Mauritania";
	case Mauritius: return "Mauritius";
	case Mayotte: return "Mayotte";
	case Mexico: return "Mexico";
	case Micronesia: return "Micronesia";
	case Moldova: return "Moldova";
	case Monaco: return "Monaco";
	case Mongolia: return "Mongolia";
	case Montserrat: return "Montserrat";
	case Morocco: return "Morocco";
	case Mozambique: return "Mozambique";
	case Myanmar: return "Myanmar";
	case Namibia: return "Namibia";
	case NauruCountry: return "NauruCountry";
	case Nepal: return "Nepal";
	case Netherlands: return "Netherlands";
	case NetherlandsAntilles: return "NetherlandsAntilles";
	case NewCaledonia: return "NewCaledonia";
	case NewZealand: return "NewZealand";
	case Nicaragua: return "Nicaragua";
	case Niger: return "Niger";
	case Nigeria: return "Nigeria";
	case Niue: return "Niue";
	case NorfolkIsland: return "NorfolkIsland";
	case NorthernMarianaIslands: return "NorthernMarianaIslands";
	case Norway: return "Norway";
	case Oman: return "Oman";
	case Pakistan: return "Pakistan";
	case Palau: return "Palau";
	case PalestinianTerritory: return "PalestinianTerritory";
	case Panama: return "Panama";
	case PapuaNewGuinea: return "PapuaNewGuinea";
	case Paraguay: return "Paraguay";
	case Peru: return "Peru";
	case Philippines: return "Philippines";
	case Pitcairn: return "Pitcairn";
	case Poland: return "Poland";
	case Portugal: return "Portugal";
	case PuertoRico: return "PuertoRico";
	case Qatar: return "Qatar";
	case Reunion: return "Reunion";
	case Romania: return "Romania";
	case RussianFederation: return "RussianFederation";
	case Rwanda: return "Rwanda";
	case SaintKittsAndNevis: return "SaintKittsAndNevis";
	case StLucia: return "StLucia";
	case StVincentAndTheGrenadines: return "StVincentAndTheGrenadines";
	case Samoa: return "Samoa";
	case SanMarino: return "SanMarino";
	case SaoTomeAndPrincipe: return "SaoTomeAndPrincipe";
	case SaudiArabia: return "SaudiArabia";
	case Senegal: return "Senegal";
	case Seychelles: return "Seychelles";
	case SierraLeone: return "SierraLeone";
	case Singapore: return "Singapore";
	case Slovakia: return "Slovakia";
	case Slovenia: return "Slovenia";
	case SolomonIslands: return "SolomonIslands";
	case Somalia: return "Somalia";
	case SouthAfrica: return "SouthAfrica";
	case SouthGeorgiaAndTheSouthSandwichIslands: return "SouthGeorgiaAndTheSouthSandwichIslands";
	case Spain: return "Spain";
	case SriLanka: return "SriLanka";
	case StHelena: return "StHelena";
	case StPierreAndMiquelon: return "StPierreAndMiquelon";
	case Sudan: return "Sudan";
	case Suriname: return "Suriname";
	case SvalbardAndJanMayenIslands: return "SvalbardAndJanMayenIslands";
	case Swaziland: return "Swaziland";
	case Sweden: return "Sweden";
	case Switzerland: return "Switzerland";
	case SyrianArabRepublic: return "SyrianArabRepublic";
	case Taiwan: return "Taiwan";
	case Tajikistan: return "Tajikistan";
	case Tanzania: return "Tanzania";
	case Thailand: return "Thailand";
	case Togo: return "Togo";
	case Tokelau: return "Tokelau";
	case TongaCountry: return "TongaCountry";
	case TrinidadAndTobago: return "TrinidadAndTobago";
	case Tunisia: return "Tunisia";
	case Turkey: return "Turkey";
	case Turkmenistan: return "Turkmenistan";
	case TurksAndCaicosIslands: return "TurksAndCaicosIslands";
	case Tuvalu: return "Tuvalu";
	case Uganda: return "Uganda";
	case Ukraine: return "Ukraine";
	case UnitedArabEmirates: return "UnitedArabEmirates";
	case UnitedKingdom: return "UnitedKingdom";
	case UnitedStates: return "UnitedStates";
	case UnitedStatesMinorOutlyingIslands: return "UnitedStatesMinorOutlyingIslands";
	case Uruguay: return "Uruguay";
	case Uzbekistan: return "Uzbekistan";
	case Vanuatu: return "Vanuatu";
	case VaticanCityState: return "VaticanCityState";
	case Venezuela: return "Venezuela";
	case VietNam: return "VietNam";
	case BritishVirginIslands: return "BritishVirginIslands";
	case USVirginIslands: return "USVirginIslands";
	case WallisAndFutunaIslands: return "WallisAndFutunaIslands";
	case WesternSahara: return "WesternSahara";
	case Yemen: return "Yemen";
	case Yugoslavia: return "Yugoslavia";
	case Zambia: return "Zambia";
	case Zimbabwe: return "Zimbabwe";
    }

    return "Unknown";
}

/*!
    Converts a localized string representation \a s of a number
    to a short int. If \a ok is not 0, reports failure by setting
    *ok to false and success by setting *ok to true.

    \sa toString()
*/

short QLocale::toShort(const QString &s, bool *ok) const
{
    Q_LLONG i = toLongLong(s, ok);
    if (i < SHRT_MIN || i > SHRT_MAX) {
    	if (ok != 0)
	    *ok = false;
	return 0;
    }
    return (short) i;
}

/*!
    Converts a localized string representation \a s of a number
    to an unsigned short int. If \a ok is not 0, reports failure by setting
    *ok to false and success by setting *ok to true.

    \sa toString()
*/

ushort QLocale::toUShort(const QString &s, bool *ok) const
{
    Q_ULLONG i = toULongLong(s, ok);
    if (i > USHRT_MAX) {
    	if (ok != 0)
	    *ok = false;
	return 0;
    }
    return (ushort) i;
}

/*!
    Converts a localized string representation \a s of a number
    to an int. If \a ok is not 0, reports failure by setting
    *ok to false and success by setting *ok to true.

    \sa toString()
*/

int QLocale::toInt(const QString &s, bool *ok) const
{
    Q_LLONG i = toLongLong(s, ok);
    if (i < INT_MIN || i > INT_MAX) {
    	if (ok != 0)
	    *ok = false;
	return 0;
    }
    return (int) i;
}

/*!
    Converts a localized string representation \a s of a number
    to an unsigned int. If \a ok is not 0, reports failure by setting
    *ok to false and success by setting *ok to true.

    \sa toString()
*/

uint QLocale::toUInt(const QString &s, bool *ok) const
{
    Q_ULLONG i = toULongLong(s, ok);
    if (i > UINT_MAX) {
    	if (ok != 0)
	    *ok = false;
	return 0;
    }
    return (uint) i;
}

/*!
    Converts a localized string representation \a s of a number
    to a long int. If \a ok is not 0, reports failure by setting
    *ok to false and success by setting *ok to true.

    \sa toString()
*/

Q_LONG QLocale::toLong(const QString &s, bool *ok) const
{
    Q_LLONG i = toLongLong(s, ok);
    if (i < LONG_MIN || i > LONG_MAX) {
    	if (ok != 0)
	    *ok = false;
	return 0;
    }
    return (Q_LONG) i;
}

/*!
    Converts a localized string representation \a s of a number
    to an unsigned long int. If \a ok is not 0, reports failure by setting
    *ok to false and success by setting *ok to true.

    \sa toString()
*/

Q_ULONG QLocale::toULong(const QString &s, bool *ok) const
{
    Q_ULLONG i = toULongLong(s, ok);
    if (i > ULONG_MAX) {
    	if (ok != 0)
	    *ok = false;
	return 0;
    }
    return (Q_ULONG) i;
}

/*!
    Converts a localized string representation \a s of a number
    to a Q_LLONG int. If \a ok is not 0, reports failure by setting
    *ok to false and success by setting *ok to true.

    \sa toString()
*/


Q_LLONG QLocale::toLongLong(const QString &s, bool *ok) const
{
    return d->stringToLongLong(s, 0, ok);
}

/*!
    Converts a localized string representation \a s of a number
    to an Q_ULLONG int. If \a ok is not 0, reports failure by setting
    *ok to false and success by setting *ok to true.

    \sa toString()
*/


Q_ULLONG QLocale::toULongLong(const QString &s, bool *ok) const
{
    return d->stringToUnsLongLong(s, 0, ok);
}

/*!
    Converts a localized string representation \a s of a
    floating point number
    to a float. If \a ok is not 0, reports failure by setting
    *ok to false and success by setting *ok to true.

    \sa toString()
*/

float QLocale::toFloat(const QString &s, bool *ok) const
{
    return (float) toDouble(s, ok);
}

/*!
    Converts a localized string representation \a s of a
    floating point number
    to a double. If \a ok is not 0, reports failure by setting
    *ok to false and success by setting *ok to true.

    \sa toString()
*/

double QLocale::toDouble(const QString &s, bool *ok) const
{
    return d->stringToDouble(s, ok);
}

/*!
    Converts \a i to a string containing its localized representation.

    \sa toLongLong()
*/

QString QLocale::toString(Q_LLONG i) const
{
    return d->longLongToString(i, -1, 10, QLocalePrivate::ThousandsGroup);
}

/*!
    \overload

    \sa toULongLong()
*/

QString QLocale::toString(Q_ULLONG i) const
{
    return d->unsLongLongToString(i, -1, 10, QLocalePrivate::ThousandsGroup);
}

static bool qIsUpper(char c)
{
    return c >= 'A' && c <= 'Z';
}

static char qToLower(char c)
{
    if (c >= 'A' && c <= 'Z')
        return c - 'A' + 'a';
    else
        return c;
}

/*!
    \overload

    \a f and \a prec have the same meaning as in QString::number(double, char, int).

    \sa toDouble()
*/

QString QLocale::toString(double i, char f, int prec) const
{
    QLocalePrivate::DoubleForm form = QLocalePrivate::DFDecimal;
    uint flags = 0;

    if (qIsUpper(f))
        flags = QLocalePrivate::CapitalEorX;
    f = qToLower(f);

    switch (f) {
        case 'f':
            form = QLocalePrivate::DFDecimal;
            break;
        case 'e':
            form = QLocalePrivate::DFExponent;
            break;
        case 'g':
            form = QLocalePrivate::DFSignificantDigits;
            break;
        default:
            break;
    }

    flags |= QLocalePrivate::ThousandsGroup;
    return d->doubleToString(i, prec, form, -1, flags);
}

/*!
\fn QString QLocale::toString(short i) const

\overload

\sa toShort()
*/

/*!
\fn QString QLocale::toString(ushort i) const

\overload

\sa toUShort()
*/

/*!
\fn QString QLocale::toString(int i) const

\overload

\sa toInt()
*/

/*!
\fn QString QLocale::toString(uint i) const

\overload

\sa toUInt()
*/

/*!
\fn QString QLocale::toString(Q_LONG i) const

\overload

\sa  toLong()
*/

/*!
\fn QString QLocale::toString(Q_ULONG i) const

\overload

\sa toULong()
*/

/*!
\fn QString QLocale::toString(float i, char f = 'g', int prec = 6) const

\overload

\a f and \a prec have the same meaning as in QString::number(double, char, int).

\sa toDouble()
*/

QChar QLocalePrivate::digitFromCLocale(char c) const
{
    if (c >= '0' && c <= '9')
    	return QChar(zero().unicode() + (c - '0'));

    qWarning("QLocalePrivate::digitFromCLocale(): bad digit: %c", c);
    return 0;
}

bool QLocalePrivate::isDigit(QChar d) const
{
    return zero().unicode() <= d.unicode()
	    && zero().unicode() + 10 > d.unicode();
}

char QLocalePrivate::digitToCLocale(QChar d) const
{
    if (zero().unicode() <= d.unicode()
	    && zero().unicode() + 10 > d.unicode())
	return '0' + d.unicode() - zero().unicode();

    qWarning("QLocalePrivate::digitToCLocale(): bad digit: row=%d, cell=%d", d.row(), d.cell());
    return 0;
}

static QString qulltoa(Q_ULLONG l, int base, const QLocalePrivate &locale)
{
    QChar buff[65]; // length of MAX_ULLONG in base 2
    QChar *p = buff + 65;

    if (base != 10 || locale.zero().unicode() == '0') {
	while (l != 0) {
            int c = l % base;

    	    --p;

            if (c < 10)
        	*p = '0' + c;
            else
        	*p = c - 10 + 'a';

            l /= base;
	}
    }
    else {
	while (l != 0) {
            int c = l % base;

    	    *(--p) = locale.zero().unicode() + c;

            l /= base;
	}
    }

    return QString(p, 65 - (p - buff));
}

static QString qlltoa(Q_LLONG l, int base, const QLocalePrivate &locale)
{
    return qulltoa(l < 0 ? -l : l, base, locale);
}

enum PrecisionMode {
    PMDecimalDigits = 	    0x01,
    PMSignificantDigits =   0x02,
    PMChopTrailingZeros =   0x03
};

static QString &decimalForm(QString &digits, int decpt, uint precision,
    	    	    	    PrecisionMode pm,
			    bool always_show_decpt,
			    bool thousands_group,
			    const QLocalePrivate &locale)
{
    if (decpt < 0) {
	for (int i = 0; i < -decpt; ++i)
    	    digits.prepend(locale.zero());
    	decpt = 0;
    }
    else if (decpt > digits.length()) {
    	for (uint i = digits.length(); i < (uint)decpt; ++i)
	    digits.append(locale.zero());
    }

    if (pm == PMDecimalDigits) {
	uint decimal_digits = digits.length() - decpt;
	for (uint i = decimal_digits; i < precision; ++i)
	    digits.append(locale.zero());
    }
    else if (pm == PMSignificantDigits) {
    	for (uint i = digits.length(); i < precision; ++i)
	    digits.append(locale.zero());
    }
    else { // pm == PMChopTrailingZeros
    }

    if (always_show_decpt || decpt < digits.length())
    	digits.insert(decpt, locale.decimal());

    if (thousands_group) {
	for (int i = decpt - 3; i > 0; i -= 3)
    	    digits.insert(i, locale.group());
    }

    if (decpt == 0)
    	digits.prepend(locale.zero());

    return digits;
}

static QString &exponentForm(QString &digits, int decpt, uint precision,
    	    	    	    	PrecisionMode pm,
			    	bool always_show_decpt,
				const QLocalePrivate &locale)
{
    int exp = decpt - 1;

    if (pm == PMDecimalDigits) {
    	for (uint i = digits.length(); i < precision + 1; ++i)
	    digits.append(locale.zero());
    }
    else if (pm == PMSignificantDigits) {
    	for (uint i = digits.length(); i < precision; ++i)
	    digits.append(locale.zero());
    }
    else { // pm == PMChopTrailingZeros
    }

    if (always_show_decpt || digits.length() > 1)
    	digits.insert(1, locale.decimal());

    digits.append(locale.exponential());
    digits.append(locale.longLongToString(exp, 2, 10,
    	    	    	    -1, QLocalePrivate::AlwaysShowSign));

    return digits;
}

QString QLocalePrivate::doubleToString(double d,
    	    int precision,
	    DoubleForm form,
    	    int width,
	    unsigned flags) const
{
    if (precision == -1)
	precision = 6;
    if (width == -1)
    	width = 0;

    bool negative = false;
    bool special_number = false; // nan, +/-inf
    QString num_str;

    // Detect special numbers (nan, +/-inf)
    if (isinf(d)) {
    	num_str = infinity();
	special_number = true;
        negative = d < 0;
    } else if (isnan(d)) {
    	num_str = nan();
	special_number = true;
    }

    // Handle normal numbers
    if (!special_number) {
	char *buff = 0;
	char *rve = 0;
	int decpt, sign;

	int mode;
	if (form == DFDecimal)
	    mode = 3;
	else
	    mode = 2;

	/* This next bit is a bit quirky. In DFExponent form, the precision
	   is the number of digits after decpt. So that would suggest using
	   mode=3 for qdtoa. But qdtoa behaves strangely when mode=3 and
	   precision=0. So we get around this by using mode=2 and reasoning
	   that we want precision+1 significant digits, since the decimal
	   point in this mode is always after the first digit. */
    	int pr = precision;
	if (form == DFExponent)
	    ++pr;

	QString digits = qdtoa(d, mode, pr, &decpt, &sign, &rve, &buff);

    	if (zero().unicode() != '0') {
	    for (int i = 0; i < digits.length(); ++i)
	    	digits.ref(i).unicode() += zero().unicode() - '0';
	}

	if (buff != 0)
    	    free(buff);

	bool always_show_decpt = flags & Alternate;
	switch (form) {
    	    case DFExponent: {
    	    	num_str = exponentForm(digits, decpt, precision, PMDecimalDigits,
		    	    	    	always_show_decpt, *this);
		break;
    	    }
	    case DFDecimal: {
		num_str = decimalForm(digits, decpt, precision, PMDecimalDigits,
		    	    	    	always_show_decpt, flags & ThousandsGroup,
					*this);
    		break;
    	    }
	    case DFSignificantDigits: {
	    	PrecisionMode mode = (flags & Alternate) ?
		    	    PMSignificantDigits : PMChopTrailingZeros;

    		if (decpt != (int)digits.length() && (decpt <= -4 || decpt > (int)precision))
	    	    num_str = exponentForm(digits, decpt, precision, mode,
		    	    	    	    always_show_decpt, *this);
		else
	    	    num_str = decimalForm(digits, decpt, precision, mode,
		    	    	    	    always_show_decpt, flags & ThousandsGroup,
					    *this);
		break;
	    }
	}

	negative = sign != 0;
    }

    // pad with zeros. LeftAdjusted overrides this flag). Also, we don't
    // pad special numbers
    if (flags & QLocalePrivate::ZeroPadded
    	    && !(flags & QLocalePrivate::LeftAdjusted)
	    && !special_number) {
    	int num_pad_chars = width - (int)num_str.length();
	// leave space for the sign
	if (negative
	    	|| flags & QLocalePrivate::AlwaysShowSign
		|| flags & QLocalePrivate::BlankBeforePositive)
	    --num_pad_chars;

	for (int i = 0; i < num_pad_chars; ++i)
	    num_str.prepend(zero());
    }

    // add sign
    if (negative)
    	num_str.prepend(minus());
    else if (flags & QLocalePrivate::AlwaysShowSign)
    	num_str.prepend(plus());
    else if (flags & QLocalePrivate::BlankBeforePositive)
    	num_str.prepend(' ');

    if (flags & QLocalePrivate::CapitalEorX)
    	num_str = num_str.upper();

    return num_str;
}

QString QLocalePrivate::longLongToString(Q_LLONG l, int precision,
	    	    	    	int base, int width,
				unsigned flags) const
{
    bool precision_not_specified = false;
    if (precision == -1) {
    	precision_not_specified = true;
	precision = 1;
    }

    bool negative = l < 0;
    if (base != 10) {
    	// these are not suported by sprintf for octal and hex
	flags &= ~AlwaysShowSign;
	flags &= ~BlankBeforePositive;
	negative = false; // neither are negative numbers
    }

    QString num_str;
    if (base == 10)
    	num_str = qlltoa(l, base, *this);
    else
    	num_str = qulltoa(l, base, *this);

    uint cnt_thousand_sep = 0;
    if (flags & ThousandsGroup && base == 10) {
    	for (int i = (int)num_str.length() - 3; i > 0; i -=3) {
	    num_str.insert(i, group());
	    ++cnt_thousand_sep;
	}
    }

    for (int i = num_str.length()/* - cnt_thousand_sep*/; i < precision; ++i)
    	num_str.prepend(base == 10 ? zero() : QChar('0'));

    if (flags & Alternate
    	    && base == 8
	    && (num_str.isEmpty()
	    	    || num_str[0].unicode() != '0'))
    	num_str.prepend('0');

    // LeftAdjusted overrides this flag ZeroPadded. sprintf only padds
    // when precision is not specified in the format string
    bool zero_padded = flags & ZeroPadded
    	    	    	&& !(flags & LeftAdjusted)
			&& precision_not_specified;

    if (zero_padded) {
    	int num_pad_chars = width - (int)num_str.length();

	// leave space for the sign
	if (negative
	    	|| flags & AlwaysShowSign
		|| flags & BlankBeforePositive)
	    --num_pad_chars;

	// leave space for optional '0x' in hex form
	if (base == 16
	    	&& flags & Alternate
	    	&& l != 0)
	    num_pad_chars -= 2;

	for (int i = 0; i < num_pad_chars; ++i)
	    num_str.prepend(base == 10 ? zero() : QChar('0'));
    }

    if (base == 16
	    && flags & Alternate
	    && l != 0)
    	num_str.prepend("0x");

    // add sign
    if (negative)
    	num_str.prepend(minus());
    else if (flags & AlwaysShowSign)
    	num_str.prepend(base == 10 ? plus() : QChar('+'));
    else if (flags & BlankBeforePositive)
    	num_str.prepend(' ');

    if (flags & CapitalEorX)
    	num_str = num_str.upper();

    return num_str;
}

QString QLocalePrivate::unsLongLongToString(Q_ULLONG l, int precision,
	    	    	    	int base, int width,
				unsigned flags) const
{
    bool precision_not_specified = false;
    if (precision == -1) {
    	precision_not_specified = true;
	precision = 1;
    }

    QString num_str = qulltoa(l, base, *this);

    uint cnt_thousand_sep = 0;
    if (flags & ThousandsGroup && base == 10) {
    	for (int i = (int)num_str.length() - 3; i > 0; i -=3) {
	    num_str.insert(i, group());
	    ++cnt_thousand_sep;
	}
    }

    for (int i = num_str.length()/* - cnt_thousand_sep*/; i < precision; ++i)
    	num_str.prepend(base == 10 ? zero() : QChar('0'));

    if (flags & Alternate
    	    && base == 8
	    && (num_str.isEmpty()
	    	    || num_str[0].unicode() != '0'))
    	num_str.prepend('0');

    // LeftAdjusted overrides this flag ZeroPadded. sprintf only padds
    // when precision is not specified in the format string
    bool zero_padded = flags & ZeroPadded
    	    	    	&& !(flags & LeftAdjusted)
			&& precision_not_specified;

    if (zero_padded) {
    	int num_pad_chars = width - (int)num_str.length();

	// leave space for optional '0x' in hex form
	if (base == 16
	    	&& flags & Alternate
	    	&& l != 0)
	    num_pad_chars -= 2;

	for (int i = 0; i < num_pad_chars; ++i)
	    num_str.prepend(base == 10 ? zero() : QChar('0'));
    }

    if (base == 16
	    && flags & Alternate
	    && l != 0)
    	num_str.prepend("0x");

    if (flags & CapitalEorX)
    	num_str = num_str.upper();

    return num_str;
}

static bool compareSubstr(const QString &s1, int idx, const QString &s2)
{
    int i = 0;
    for (; i + idx < s1.length() && i < s2.length(); ++i) {
        if (s1.unicode()[i + idx] != s2.unicode()[i])
            return false;
    }

    return i == s2.length();
}

// Converts a number in locale to its representation in the C locale.
// If it fails, returns number unconverted.
QString &QLocalePrivate::numberToCLocale(QString &l_num) const
{
    if (languageId() == QLocale::C)
	return l_num;

    int idx = 0;

    // skip leading white space
    while (idx < l_num.length() && l_num.unicode()[idx].isSpace())
    	++idx;

    do {

	if (compareSubstr(l_num, idx, nan())) {
	    idx += nan().length();
	    break;
	}
	else if (compareSubstr(l_num, idx, nan().upper())) {
	    for (int i = idx; i < idx + nan().length(); ++i)
	    	l_num.ref(i) = l_num.unicode()[i].lower();
	    idx += nan().length();
	    break;
	}
	QChar &c = l_num.ref(idx);

	if (c == plus()) {
    	    c.unicode() = '+';
    	    ++idx;
    	}
	else if (c == minus()) {
            c.unicode() = '-';
    	    ++idx;
	}

	if (compareSubstr(l_num, idx, infinity())) {
	    idx += infinity().length();
	    break;
	}
	else if (compareSubstr(l_num, idx, infinity().upper())) {
	    for (int i = idx; i < idx + infinity().length(); ++i)
	    	l_num.ref(i) = l_num.unicode()[i].lower();
	    idx += infinity().length();
	    break;
	}

	while (idx < l_num.length()) {
            QChar &c = l_num.ref(idx);

            if (isDigit(c))
		c = digitToCLocale(c);
	    else if (c == plus())
		c = '+';
            else if (c == minus())
		c = '-';
            else if (c == decimal())
        	c = '.';
            else if (c == group())
	    	c = ',';
	    else if (c == exponential() || c == exponential().upper())
    		c = 'e';
            else if (c.unicode() == 'x' || c.unicode() == 'X') // hex number
		c = 'x';
            else if (c == list())
    		c = ';';
            else if (c == percent())
		c = '%';
	    else
		break;

	    ++idx;
	}

    } while (false);

    return l_num;
}

double QLocalePrivate::stringToDouble(QString num,
                                        bool *ok) const
{
//    num = num.stripWhiteSpace();
    if (num.isEmpty()) {
	if (ok != 0)
            *ok = false;
	return 0.0;
    }

    if (ok != 0)
        *ok = true;

    num = numberToCLocale(num);

    if (num == "nan")
        return NAN;

    if (num == "+inf"
    	    || num == "inf")
        return INFINITY;

    if (num == "-inf")
        return -INFINITY;

    bool _ok;
    const char *endptr;
    const char *num_buff = num.latin1();
    double d = qstrtod(num_buff, &endptr, &_ok);

    if (!_ok || *endptr != '\0') {
        if (ok != 0)
            *ok = false;
        return 0.0;
    }
    else
        return d;
}

Q_LLONG QLocalePrivate::stringToLongLong(QString num, int base,
                                    bool *ok) const
{
    num = num.stripWhiteSpace();
    if (num.isEmpty()) {
	if (ok != 0)
            *ok = false;
	return 0;
    }

    num = numberToCLocale(num);

    bool _ok;
    const char *endptr;
    const char *num_buff = num.latin1();
    Q_LLONG l = qstrtoll(num_buff, &endptr, base, &_ok);

    if (!_ok || *endptr != '\0') {
        if (ok != 0)
            *ok = false;
        return 0;
    }

    if (ok != 0)
        *ok = true;
    return l;
}

Q_ULLONG QLocalePrivate::stringToUnsLongLong(QString num, int base,
                                    bool *ok) const
{
    num = num.stripWhiteSpace();
    if (num.isEmpty()) {
	if (ok != 0)
            *ok = false;
	return 0;
    }

    num = numberToCLocale(num);

    bool _ok;
    const char *endptr;
    const char *num_buff = num.latin1();
    Q_ULLONG l = qstrtoull(num_buff, &endptr, base, &_ok);

    if (!_ok || *endptr != '\0') {
        if (ok != 0)
            *ok = false;
        return 0;
    }

    if (ok != 0)
        *ok = true;
    return l;
}

/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

// static char sccsid[] = "@(#)strtouq.c	8.1 (Berkeley) 6/4/93";
//  "$FreeBSD: src/lib/libc/stdlib/strtoull.c,v 1.5.2.1 2001/03/02 09:45:20 obrien Exp $";

/*
 * Convert a string to an Q_ULLONG integer.
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
static Q_ULLONG qstrtoull(const char *nptr, const char **endptr, register int base, bool *ok)
{
	register const char *s = nptr;
	register Q_ULLONG acc;
	register unsigned char c;
	register Q_ULLONG qbase, cutoff;
	register int neg, any, cutlim;

    	if (ok != 0)
	    *ok = true;

	/*
	 * See strtoq for comments as to the logic used.
	 */
	s = nptr;
	do {
		c = *s++;
	} while (isspace(c));
	if (c == '-') {
		if (ok != 0)
		    *ok = false;
		if (endptr != 0)
		    *endptr = s - 1;
		return 0;
	} else {
		neg = 0;
		if (c == '+')
			c = *s++;
	}
	if ((base == 0 || base == 16) &&
	    c == '0' && (*s == 'x' || *s == 'X')) {
		c = s[1];
		s += 2;
		base = 16;
	}
	if (base == 0)
		base = c == '0' ? 8 : 10;
	qbase = (unsigned)base;
	cutoff = (Q_ULLONG)ULLONG_MAX / qbase;
	cutlim = (Q_ULLONG)ULLONG_MAX % qbase;
	for (acc = 0, any = 0;; c = *s++) {
		if (!isascii(c))
			break;
		if (isdigit(c))
			c -= '0';
		else if (isalpha(c))
			c -= isupper(c) ? 'A' - 10 : 'a' - 10;
		else
			break;
		if (c >= base)
			break;
		if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
			any = -1;
		else {
			any = 1;
			acc *= qbase;
			acc += c;
		}
	}
	if (any < 0) {
		acc = ULLONG_MAX;
    	    	if (ok != 0)
		    *ok = false;
	}
	else if (neg)
	    acc = (~acc) + 1;
	if (endptr != 0)
		*endptr = (char *)(any ? s - 1 : nptr);
	return (acc);
}


//  "$FreeBSD: src/lib/libc/stdlib/strtoll.c,v 1.5.2.1 2001/03/02 09:45:20 obrien Exp $";


/*
 * Convert a string to a Q_LLONG integer.
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
static Q_LLONG qstrtoll(const char *nptr, const char **endptr, register int base, bool *ok)
{
	register const char *s;
	register Q_ULLONG acc;
	register unsigned char c;
	register Q_ULLONG qbase, cutoff;
	register int neg, any, cutlim;

    	if (ok != 0)
	    *ok = true;

	/*
	 * Skip white space and pick up leading +/- sign if any.
	 * If base is 0, allow 0x for hex and 0 for octal, else
	 * assume decimal; if base is already 16, allow 0x.
	 */
	s = nptr;
	do {
		c = *s++;
	} while (isspace(c));
	if (c == '-') {
		neg = 1;
		c = *s++;
	} else {
		neg = 0;
		if (c == '+')
			c = *s++;
	}
	if ((base == 0 || base == 16) &&
	    c == '0' && (*s == 'x' || *s == 'X')) {
		c = s[1];
		s += 2;
		base = 16;
	}
	if (base == 0)
		base = c == '0' ? 8 : 10;

	/*
	 * Compute the cutoff value between legal numbers and illegal
	 * numbers.  That is the largest legal value, divided by the
	 * base.  An input number that is greater than this value, if
	 * followed by a legal input character, is too big.  One that
	 * is equal to this value may be valid or not; the limit
	 * between valid and invalid numbers is then based on the last
	 * digit.  For instance, if the range for quads is
	 * [-9223372036854775808..9223372036854775807] and the input base
	 * is 10, cutoff will be set to 922337203685477580 and cutlim to
	 * either 7 (neg==0) or 8 (neg==1), meaning that if we have
	 * accumulated a value > 922337203685477580, or equal but the
	 * next digit is > 7 (or 8), the number is too big, and we will
	 * return a range error.
	 *
	 * Set any if any `digits' consumed; make it negative to indicate
	 * overflow.
	 */
	qbase = (unsigned)base;
	cutoff = neg ? (Q_ULLONG)-(LLONG_MIN + LLONG_MAX) + LLONG_MAX
	    : LLONG_MAX;
	cutlim = cutoff % qbase;
	cutoff /= qbase;
	for (acc = 0, any = 0;; c = *s++) {
		if (!isascii(c))
			break;
		if (isdigit(c))
			c -= '0';
		else if (isalpha(c))
			c -= isupper(c) ? 'A' - 10 : 'a' - 10;
		else
			break;
		if (c >= base)
			break;
		if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
			any = -1;
		else {
			any = 1;
			acc *= qbase;
			acc += c;
		}
	}
	if (any < 0) {
		acc = neg ? LLONG_MIN : LLONG_MAX;
		if (ok != 0)
		    *ok = false;
	} else if (neg) {
	    acc = (~acc) + 1;
	}
	if (endptr != 0)
		*endptr = (char *)(any ? s - 1 : nptr);
	return (acc);
}


/*	From: NetBSD: strtod.c,v 1.26 1998/02/03 18:44:21 perry Exp */
/* $FreeBSD: src/lib/libc/stdlib/netbsd_strtod.c,v 1.2.2.2 2001/03/02 17:14:15 tegge Exp $	*/

/****************************************************************
 *
 * The author of this software is David M. Gay.
 *
 * Copyright (c) 1991 by AT&T.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice
 * is included in all copies of any software which is or includes a copy
 * or modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHOR NOR AT&T MAKES ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 ***************************************************************/

/* Please send bug reports to
	David M. Gay
	AT&T Bell Laboratories, Room 2C-463
	600 Mountain Avenue
	Murray Hill, NJ 07974-2070
	U.S.A.
	dmg@research.att.com or research!dmg
 */

/* strtod for IEEE-, VAX-, and IBM-arithmetic machines.
 *
 * This strtod returns a nearest machine number to the input decimal
 * string (or sets errno to ERANGE).  With IEEE arithmetic, ties are
 * broken by the IEEE round-even rule.  Otherwise ties are broken by
 * biased rounding (add half and chop).
 *
 * Inspired loosely by William D. Clinger's paper "How to Read Floating
 * Point Numbers Accurately" [Proc. ACM SIGPLAN '90, pp. 92-101].
 *
 * Modifications:
 *
 *	1. We only require IEEE, IBM, or VAX double-precision
 *		arithmetic (not IEEE double-extended).
 *	2. We get by with floating-point arithmetic in a case that
 *		Clinger missed -- when we're computing d * 10^n
 *		for a small integer d and the integer n is not too
 *		much larger than 22 (the maximum integer k for which
 *		we can represent 10^k exactly), we may be able to
 *		compute (d*10^k) * 10^(e-k) with just one roundoff.
 *	3. Rather than a bit-at-a-time adjustment of the binary
 *		result in the hard case, we use floating-point
 *		arithmetic to determine the adjustment to within
 *		one bit; only in really hard cases do we need to
 *		compute a second residual.
 *	4. Because of 3., we don't need a large table of powers of 10
 *		for ten-to-e (just some small tables, e.g. of 10^k
 *		for 0 <= k <= 22).
 */

/*
 * #define IEEE_LITTLE_ENDIAN for IEEE-arithmetic machines where the least
 *	significant byte has the lowest address.
 * #define IEEE_BIG_ENDIAN for IEEE-arithmetic machines where the most
 *	significant byte has the lowest address.
 * #define Long int on machines with 32-bit ints and 64-bit longs.
 * #define Sudden_Underflow for IEEE-format machines without gradual
 *	underflow (i.e., that flush to zero on underflow).
 * #define IBM for IBM mainframe-style floating-point arithmetic.
 * #define VAX for VAX-style floating-point arithmetic.
 * #define Unsigned_Shifts if >> does treats its left operand as unsigned.
 * #define No_leftright to omit left-right logic in fast floating-point
 *	computation of dtoa.
 * #define Check_FLT_ROUNDS if FLT_ROUNDS can assume the values 2 or 3.
 * #define RND_PRODQUOT to use rnd_prod and rnd_quot (assembly routines
 *	that use extended-precision instructions to compute rounded
 *	products and quotients) with IBM.
 * #define ROUND_BIASED for IEEE-format with biased rounding.
 * #define Inaccurate_Divide for IEEE-format with correctly rounded
 *	products but inaccurate quotients, e.g., for Intel i860.
 * #define Just_16 to store 16 bits per 32-bit Long when doing high-precision
 *	integer arithmetic.  Whether this speeds things up or slows things
 *	down depends on the machine and the number being converted.
 * #define KR_headers for old-style C function headers.
 * #define Bad_float_h if your system lacks a float.h or if it does not
 *	define some or all of DBL_DIG, DBL_MAX_10_EXP, DBL_MAX_EXP,
 *	FLT_RADIX, FLT_ROUNDS, and DBL_MAX.
 * #define MALLOC your_malloc, where your_malloc(n) acts like malloc(n)
 *	if memory is available and otherwise does something you deem
 *	appropriate.  If MALLOC is undefined, malloc will be invoked
 *	directly -- and assumed always to succeed.
 */

#if defined(LIBC_SCCS) && !defined(lint)
__RCSID("$NetBSD: strtod.c,v 1.26 1998/02/03 18:44:21 perry Exp $");
#endif /* LIBC_SCCS and not lint */

#if defined(__m68k__)    || defined(__sparc__) || defined(__i386__) || \
     defined(__mips__)    || defined(__ns32k__) || defined(__alpha__) || \
     defined(__powerpc__) || defined(Q_OS_WIN) || defined(Q_OS_MACX)
#   	define IEEE_BIG_OR_LITTLE_ENDIAN 1
#endif

#ifdef __arm32__
/*
 * Although the CPU is little endian the FP has different
 * byte and word endianness. The byte order is still little endian
 * but the word order is big endian.
 */
#define IEEE_BIG_OR_LITTLE_ENDIAN
#endif

#ifdef vax
#define VAX
#endif

#ifdef __ppc__
# define IEEE_BIG_ENDIAN
#endif

#define Long	Q_INT32
#define ULong	Q_UINT32

#define MALLOC malloc
#define CONST const

#ifdef BSD_QDTOA_DEBUG
#include <stdio.h>
#define Bug(x) {fprintf(stderr, "%s\n", x); exit(1);}
#endif

#ifdef Unsigned_Shifts
#define Sign_Extend(a,b) if (b < 0) a |= 0xffff0000;
#else
#define Sign_Extend(a,b) /*no-op*/
#endif

#if (defined(IEEE_BIG_OR_LITTLE_ENDIAN) + defined(VAX) + defined(IBM)) != 1
#error Exactly one of IEEE_BIG_OR_LITTLE_ENDIAN, VAX, or IBM should be defined.
#endif

/*
#ifdef IEEE_LITTLE_ENDIAN
#define word0(x) ((ULong *)&x)[1]
#define word1(x) ((ULong *)&x)[0]
#else
#define word0(x) ((ULong *)&x)[0]
#define word1(x) ((ULong *)&x)[1]
#endif
*/

inline ULong &word0(double &x)
{
    if (QLocaleStaticData::bigEndian())
    	return ((ULong *)&x)[0];
    return ((ULong *)&x)[1];
}

inline ULong &word1(double &x)
{
    if (QLocaleStaticData::bigEndian())
    	return ((ULong *)&x)[1];
    return ((ULong *)&x)[0];
}

/* The following definition of Storeinc is appropriate for MIPS processors.
 * An alternative that might be better on some machines is
 * #define Storeinc(a,b,c) (*a++ = b << 16 | c & 0xffff)
 */

/* 
#if defined(IEEE_LITTLE_ENDIAN) + defined(VAX) + defined(__arm32__)
#define Storeinc(a,b,c) (((unsigned short *)a)[1] = (unsigned short)b, \
((unsigned short *)a)[0] = (unsigned short)c, a++)
#else
#define Storeinc(a,b,c) (((unsigned short *)a)[0] = (unsigned short)b, \
((unsigned short *)a)[1] = (unsigned short)c, a++)
#endif
*/

inline void Storeinc(ULong *&a, const ULong &b, const ULong &c)
{

#   if defined(VAX) + defined(__arm32__)
#   	define USE_LITTLE_ENDIAN 1
#   else
#   	define USE_LITTLE_ENDIAN 0
#   endif

#   if defined(IEEE_BIG_OR_LITTLE_ENDIAN)
#   	define USE_IEEE 1
#   else
#   	define USE_IEEE 0
#   endif

    if (!QLocaleStaticData::bigEndian() && USE_IEEE || USE_LITTLE_ENDIAN) {
	((unsigned short *)a)[1] = (unsigned short)b;
	((unsigned short *)a)[0] = (unsigned short)c;
    } else {
    	((unsigned short *)a)[0] = (unsigned short)b;
    	((unsigned short *)a)[1] = (unsigned short)c;
    }
    
    ++a;

#   undef USE_LITTLE_ENDIAN
#   undef USE_IEEE
}

/* #define P DBL_MANT_DIG */
/* Ten_pmax = floor(P*log(2)/log(5)) */
/* Bletch = (highest power of 2 < DBL_MAX_10_EXP) / 16 */
/* Quick_max = floor((P-1)*log(FLT_RADIX)/log(10) - 1) */
/* Int_max = floor(P*log(FLT_RADIX)/log(10) - 1) */

#if defined(IEEE_BIG_OR_LITTLE_ENDIAN)
#define Exp_shift  20
#define Exp_shift1 20
#define Exp_msk1    0x100000
#define Exp_msk11   0x100000
#define Exp_mask  0x7ff00000
#define P 53
#define Bias 1023
#define IEEE_Arith
#define Emin (-1022)
#define Exp_1  0x3ff00000
#define Exp_11 0x3ff00000
#define Ebits 11
#define Frac_mask  0xfffff
#define Frac_mask1 0xfffff
#define Ten_pmax 22
#define Bletch 0x10
#define Bndry_mask  0xfffff
#define Bndry_mask1 0xfffff
#define LSB 1
#define Sign_bit 0x80000000
#define Log2P 1
#define Tiny0 0
#define Tiny1 1
#define Quick_max 14
#define Int_max 14
#define Infinite(x) (word0(x) == 0x7ff00000) /* sufficient test for here */
#else
#undef  Sudden_Underflow
#define Sudden_Underflow
#ifdef IBM
#define Exp_shift  24
#define Exp_shift1 24
#define Exp_msk1   0x1000000
#define Exp_msk11  0x1000000
#define Exp_mask  0x7f000000
#define P 14
#define Bias 65
#define Exp_1  0x41000000
#define Exp_11 0x41000000
#define Ebits 8	/* exponent has 7 bits, but 8 is the right value in b2d */
#define Frac_mask  0xffffff
#define Frac_mask1 0xffffff
#define Bletch 4
#define Ten_pmax 22
#define Bndry_mask  0xefffff
#define Bndry_mask1 0xffffff
#define LSB 1
#define Sign_bit 0x80000000
#define Log2P 4
#define Tiny0 0x100000
#define Tiny1 0
#define Quick_max 14
#define Int_max 15
#else /* VAX */
#define Exp_shift  23
#define Exp_shift1 7
#define Exp_msk1    0x80
#define Exp_msk11   0x800000
#define Exp_mask  0x7f80
#define P 56
#define Bias 129
#define Exp_1  0x40800000
#define Exp_11 0x4080
#define Ebits 8
#define Frac_mask  0x7fffff
#define Frac_mask1 0xffff007f
#define Ten_pmax 24
#define Bletch 2
#define Bndry_mask  0xffff007f
#define Bndry_mask1 0xffff007f
#define LSB 0x10000
#define Sign_bit 0x8000
#define Log2P 1
#define Tiny0 0x80
#define Tiny1 0
#define Quick_max 15
#define Int_max 15
#endif
#endif

#ifndef IEEE_Arith
#define ROUND_BIASED
#endif

#ifdef RND_PRODQUOT
#define rounded_product(a,b) a = rnd_prod(a, b)
#define rounded_quotient(a,b) a = rnd_quot(a, b)
#ifdef KR_headers
extern double rnd_prod(), rnd_quot();
#else
extern double rnd_prod(double, double), rnd_quot(double, double);
#endif
#else
#define rounded_product(a,b) a *= b
#define rounded_quotient(a,b) a /= b
#endif

#define Big0 (Frac_mask1 | Exp_msk1*(DBL_MAX_EXP+Bias-1))
#define Big1 0xffffffff

#ifndef Just_16
/* When Pack_32 is not defined, we store 16 bits per 32-bit Long.
 * This makes some inner loops simpler and sometimes saves work
 * during multiplications, but it often seems to make things slightly
 * slower.  Hence the default is now to store 32 bits per Long.
 */
#ifndef Pack_32
#define Pack_32
#endif
#endif

#define Kmax 15

 struct
Bigint {
	struct Bigint *next;
	int k, maxwds, sign, wds;
	ULong x[1];
	};

 typedef struct Bigint Bigint;

 static Bigint *
Balloc
#ifdef KR_headers
	(k) int k;
#else
	(int k)
#endif
{
	int x;
	Bigint *rv;

	x = 1 << k;
	rv = (Bigint *)MALLOC(sizeof(Bigint) + (x-1)*sizeof(Long));
	rv->k = k;
	rv->maxwds = x;
	rv->sign = rv->wds = 0;
	return rv;
	}

 static void
Bfree
#ifdef KR_headers
	(v) Bigint *v;
#else
	(Bigint *v)
#endif
{
	free(v);
	}

#define Bcopy(x,y) memcpy((char *)&x->sign, (char *)&y->sign, \
y->wds*sizeof(Long) + 2*sizeof(int))

 static Bigint *
multadd
#ifdef KR_headers
	(b, m, a) Bigint *b; int m, a;
#else
	(Bigint *b, int m, int a)	/* multiply by m and add a */
#endif
{
	int i, wds;
	ULong *x, y;
#ifdef Pack_32
	ULong xi, z;
#endif
	Bigint *b1;

	wds = b->wds;
	x = b->x;
	i = 0;
	do {
#ifdef Pack_32
		xi = *x;
		y = (xi & 0xffff) * m + a;
		z = (xi >> 16) * m + (y >> 16);
		a = (int)(z >> 16);
		*x++ = (z << 16) + (y & 0xffff);
#else
		y = *x * m + a;
		a = (int)(y >> 16);
		*x++ = y & 0xffff;
#endif
		}
		while(++i < wds);
	if (a) {
		if (wds >= b->maxwds) {
			b1 = Balloc(b->k+1);
			Bcopy(b1, b);
			Bfree(b);
			b = b1;
			}
		b->x[wds++] = a;
		b->wds = wds;
		}
	return b;
	}

 static Bigint *
s2b
#ifdef KR_headers
	(s, nd0, nd, y9) CONST char *s; int nd0, nd; ULong y9;
#else
	(CONST char *s, int nd0, int nd, ULong y9)
#endif
{
	Bigint *b;
	int i, k;
	Long x, y;

	x = (nd + 8) / 9;
	for(k = 0, y = 1; x > y; y <<= 1, k++) ;
#ifdef Pack_32
	b = Balloc(k);
	b->x[0] = y9;
	b->wds = 1;
#else
	b = Balloc(k+1);
	b->x[0] = y9 & 0xffff;
	b->wds = (b->x[1] = y9 >> 16) ? 2 : 1;
#endif

	i = 9;
	if (9 < nd0) {
		s += 9;
		do b = multadd(b, 10, *s++ - '0');
			while(++i < nd0);
		s++;
		}
	else
		s += 10;
	for(; i < nd; i++)
		b = multadd(b, 10, *s++ - '0');
	return b;
	}

 static int
hi0bits
#ifdef KR_headers
	(x) ULong x;
#else
	(ULong x)
#endif
{
	int k = 0;

	if (!(x & 0xffff0000)) {
		k = 16;
		x <<= 16;
		}
	if (!(x & 0xff000000)) {
		k += 8;
		x <<= 8;
		}
	if (!(x & 0xf0000000)) {
		k += 4;
		x <<= 4;
		}
	if (!(x & 0xc0000000)) {
		k += 2;
		x <<= 2;
		}
	if (!(x & 0x80000000)) {
		k++;
		if (!(x & 0x40000000))
			return 32;
		}
	return k;
	}

 static int
lo0bits
#ifdef KR_headers
	(y) ULong *y;
#else
	(ULong *y)
#endif
{
	int k;
	ULong x = *y;

	if (x & 7) {
		if (x & 1)
			return 0;
		if (x & 2) {
			*y = x >> 1;
			return 1;
			}
		*y = x >> 2;
		return 2;
		}
	k = 0;
	if (!(x & 0xffff)) {
		k = 16;
		x >>= 16;
		}
	if (!(x & 0xff)) {
		k += 8;
		x >>= 8;
		}
	if (!(x & 0xf)) {
		k += 4;
		x >>= 4;
		}
	if (!(x & 0x3)) {
		k += 2;
		x >>= 2;
		}
	if (!(x & 1)) {
		k++;
		x >>= 1;
		if (!x & 1)
			return 32;
		}
	*y = x;
	return k;
	}

 static Bigint *
i2b
#ifdef KR_headers
	(i) int i;
#else
	(int i)
#endif
{
	Bigint *b;

	b = Balloc(1);
	b->x[0] = i;
	b->wds = 1;
	return b;
	}

 static Bigint *
mult
#ifdef KR_headers
	(a, b) Bigint *a, *b;
#else
	(Bigint *a, Bigint *b)
#endif
{
	Bigint *c;
	int k, wa, wb, wc;
	ULong carry, y, z;
	ULong *x, *xa, *xae, *xb, *xbe, *xc, *xc0;
#ifdef Pack_32
	ULong z2;
#endif

	if (a->wds < b->wds) {
		c = a;
		a = b;
		b = c;
		}
	k = a->k;
	wa = a->wds;
	wb = b->wds;
	wc = wa + wb;
	if (wc > a->maxwds)
		k++;
	c = Balloc(k);
	for(x = c->x, xa = x + wc; x < xa; x++)
		*x = 0;
	xa = a->x;
	xae = xa + wa;
	xb = b->x;
	xbe = xb + wb;
	xc0 = c->x;
#ifdef Pack_32
	for(; xb < xbe; xb++, xc0++) {
		if ((y = *xb & 0xffff) != 0) {
			x = xa;
			xc = xc0;
			carry = 0;
			do {
				z = (*x & 0xffff) * y + (*xc & 0xffff) + carry;
				carry = z >> 16;
				z2 = (*x++ >> 16) * y + (*xc >> 16) + carry;
				carry = z2 >> 16;
				Storeinc(xc, z2, z);
				}
				while(x < xae);
			*xc = carry;
			}
		if ((y = *xb >> 16) != 0) {
			x = xa;
			xc = xc0;
			carry = 0;
			z2 = *xc;
			do {
				z = (*x & 0xffff) * y + (*xc >> 16) + carry;
				carry = z >> 16;
				Storeinc(xc, z, z2);
				z2 = (*x++ >> 16) * y + (*xc & 0xffff) + carry;
				carry = z2 >> 16;
				}
				while(x < xae);
			*xc = z2;
			}
		}
#else
	for(; xb < xbe; xc0++) {
		if (y = *xb++) {
			x = xa;
			xc = xc0;
			carry = 0;
			do {
				z = *x++ * y + *xc + carry;
				carry = z >> 16;
				*xc++ = z & 0xffff;
				}
				while(x < xae);
			*xc = carry;
			}
		}
#endif
	for(xc0 = c->x, xc = xc0 + wc; wc > 0 && !*--xc; --wc) ;
	c->wds = wc;
	return c;
	}

 static Bigint *p5s;

 static Bigint *
pow5mult
#ifdef KR_headers
	(b, k) Bigint *b; int k;
#else
	(Bigint *b, int k)
#endif
{
	Bigint *b1, *p5, *p51;
	int i;
	static int p05[3] = { 5, 25, 125 };

	if ((i = k & 3) != 0)
		b = multadd(b, p05[i-1], 0);

	if (!(k >>= 2))
		return b;
	if (!(p5 = p5s)) {
		/* first time */
		p5 = p5s = i2b(625);
		p5->next = 0;
		}
	for(;;) {
		if (k & 1) {
			b1 = mult(b, p5);
			Bfree(b);
			b = b1;
			}
		if (!(k >>= 1))
			break;
		if (!(p51 = p5->next)) {
			p51 = p5->next = mult(p5,p5);
			p51->next = 0;
			}
		p5 = p51;
		}
	return b;
	}

 static Bigint *
lshift
#ifdef KR_headers
	(b, k) Bigint *b; int k;
#else
	(Bigint *b, int k)
#endif
{
	int i, k1, n, n1;
	Bigint *b1;
	ULong *x, *x1, *xe, z;

#ifdef Pack_32
	n = k >> 5;
#else
	n = k >> 4;
#endif
	k1 = b->k;
	n1 = n + b->wds + 1;
	for(i = b->maxwds; n1 > i; i <<= 1)
		k1++;
	b1 = Balloc(k1);
	x1 = b1->x;
	for(i = 0; i < n; i++)
		*x1++ = 0;
	x = b->x;
	xe = x + b->wds;
#ifdef Pack_32
	if (k &= 0x1f) {
		k1 = 32 - k;
		z = 0;
		do {
			*x1++ = *x << k | z;
			z = *x++ >> k1;
			}
			while(x < xe);
		if ((*x1 = z) != 0)
			++n1;
		}
#else
	if (k &= 0xf) {
		k1 = 16 - k;
		z = 0;
		do {
			*x1++ = *x << k  & 0xffff | z;
			z = *x++ >> k1;
			}
			while(x < xe);
		if (*x1 = z)
			++n1;
		}
#endif
	else do
		*x1++ = *x++;
		while(x < xe);
	b1->wds = n1 - 1;
	Bfree(b);
	return b1;
	}

 static int
cmp
#ifdef KR_headers
	(a, b) Bigint *a, *b;
#else
	(Bigint *a, Bigint *b)
#endif
{
	ULong *xa, *xa0, *xb, *xb0;
	int i, j;

	i = a->wds;
	j = b->wds;
#ifdef BSD_QDTOA_DEBUG
	if (i > 1 && !a->x[i-1])
		Bug("cmp called with a->x[a->wds-1] == 0");
	if (j > 1 && !b->x[j-1])
		Bug("cmp called with b->x[b->wds-1] == 0");
#endif
	if (i -= j)
		return i;
	xa0 = a->x;
	xa = xa0 + j;
	xb0 = b->x;
	xb = xb0 + j;
	for(;;) {
		if (*--xa != *--xb)
			return *xa < *xb ? -1 : 1;
		if (xa <= xa0)
			break;
		}
	return 0;
	}

 static Bigint *
diff
#ifdef KR_headers
	(a, b) Bigint *a, *b;
#else
	(Bigint *a, Bigint *b)
#endif
{
	Bigint *c;
	int i, wa, wb;
	Long borrow, y;	/* We need signed shifts here. */
	ULong *xa, *xae, *xb, *xbe, *xc;
#ifdef Pack_32
	Long z;
#endif

	i = cmp(a,b);
	if (!i) {
		c = Balloc(0);
		c->wds = 1;
		c->x[0] = 0;
		return c;
		}
	if (i < 0) {
		c = a;
		a = b;
		b = c;
		i = 1;
		}
	else
		i = 0;
	c = Balloc(a->k);
	c->sign = i;
	wa = a->wds;
	xa = a->x;
	xae = xa + wa;
	wb = b->wds;
	xb = b->x;
	xbe = xb + wb;
	xc = c->x;
	borrow = 0;
#ifdef Pack_32
	do {
		y = (*xa & 0xffff) - (*xb & 0xffff) + borrow;
		borrow = y >> 16;
		Sign_Extend(borrow, y);
		z = (*xa++ >> 16) - (*xb++ >> 16) + borrow;
		borrow = z >> 16;
		Sign_Extend(borrow, z);
		Storeinc(xc, z, y);
		}
		while(xb < xbe);
	while(xa < xae) {
		y = (*xa & 0xffff) + borrow;
		borrow = y >> 16;
		Sign_Extend(borrow, y);
		z = (*xa++ >> 16) + borrow;
		borrow = z >> 16;
		Sign_Extend(borrow, z);
		Storeinc(xc, z, y);
		}
#else
	do {
		y = *xa++ - *xb++ + borrow;
		borrow = y >> 16;
		Sign_Extend(borrow, y);
		*xc++ = y & 0xffff;
		}
		while(xb < xbe);
	while(xa < xae) {
		y = *xa++ + borrow;
		borrow = y >> 16;
		Sign_Extend(borrow, y);
		*xc++ = y & 0xffff;
		}
#endif
	while(!*--xc)
		wa--;
	c->wds = wa;
	return c;
	}

 static double
ulp
#ifdef KR_headers
	(x) double x;
#else
	(double x)
#endif
{
	Long L;
	double a;

	L = (word0(x) & Exp_mask) - (P-1)*Exp_msk1;
#ifndef Sudden_Underflow
	if (L > 0) {
#endif
#ifdef IBM
		L |= Exp_msk1 >> 4;
#endif
		word0(a) = L;
		word1(a) = 0;
#ifndef Sudden_Underflow
		}
	else {
		L = -L >> Exp_shift;
		if (L < Exp_shift) {
			word0(a) = 0x80000 >> L;
			word1(a) = 0;
			}
		else {
			word0(a) = 0;
			L -= Exp_shift;
			word1(a) = L >= 31 ? 1 : 1 << (31 - L);
			}
		}
#endif
	return a;
	}

 static double
b2d
#ifdef KR_headers
	(a, e) Bigint *a; int *e;
#else
	(Bigint *a, int *e)
#endif
{
	ULong *xa, *xa0, w, y, z;
	int k;
	double d;
#ifdef VAX
	ULong d0, d1;
#else
#define d0 word0(d)
#define d1 word1(d)
#endif

	xa0 = a->x;
	xa = xa0 + a->wds;
	y = *--xa;
#ifdef BSD_QDTOA_DEBUG
	if (!y) Bug("zero y in b2d");
#endif
	k = hi0bits(y);
	*e = 32 - k;
#ifdef Pack_32
	if (k < Ebits) {
		d0 = Exp_1 | y >> (Ebits - k);
		w = xa > xa0 ? *--xa : 0;
		d1 = y << ((32-Ebits) + k) | w >> (Ebits - k);
		goto ret_d;
		}
	z = xa > xa0 ? *--xa : 0;
	if (k -= Ebits) {
		d0 = Exp_1 | y << k | z >> (32 - k);
		y = xa > xa0 ? *--xa : 0;
		d1 = z << k | y >> (32 - k);
		}
	else {
		d0 = Exp_1 | y;
		d1 = z;
		}
#else
	if (k < Ebits + 16) {
		z = xa > xa0 ? *--xa : 0;
		d0 = Exp_1 | y << k - Ebits | z >> Ebits + 16 - k;
		w = xa > xa0 ? *--xa : 0;
		y = xa > xa0 ? *--xa : 0;
		d1 = z << k + 16 - Ebits | w << k - Ebits | y >> 16 + Ebits - k;
		goto ret_d;
		}
	z = xa > xa0 ? *--xa : 0;
	w = xa > xa0 ? *--xa : 0;
	k -= Ebits + 16;
	d0 = Exp_1 | y << k + 16 | z << k | w >> 16 - k;
	y = xa > xa0 ? *--xa : 0;
	d1 = w << k + 16 | y << k;
#endif
 ret_d:
#ifdef VAX
	word0(d) = d0 >> 16 | d0 << 16;
	word1(d) = d1 >> 16 | d1 << 16;
#else
#undef d0
#undef d1
#endif
	return d;
	}

 static Bigint *
d2b
#ifdef KR_headers
	(d, e, bits) double d; int *e, *bits;
#else
	(double d, int *e, int *bits)
#endif
{
	Bigint *b;
	int de, i, k;
	ULong *x, y, z;
#ifdef VAX
	ULong d0, d1;
	d0 = word0(d) >> 16 | word0(d) << 16;
	d1 = word1(d) >> 16 | word1(d) << 16;
#else
#define d0 word0(d)
#define d1 word1(d)
#endif

#ifdef Pack_32
	b = Balloc(1);
#else
	b = Balloc(2);
#endif
	x = b->x;

	z = d0 & Frac_mask;
	d0 &= 0x7fffffff;	/* clear sign bit, which we ignore */
#ifdef Sudden_Underflow
	de = (int)(d0 >> Exp_shift);
#ifndef IBM
	z |= Exp_msk11;
#endif
#else
	if ((de = (int)(d0 >> Exp_shift)) != 0)
		z |= Exp_msk1;
#endif
#ifdef Pack_32
	if ((y = d1) != 0) {
		if ((k = lo0bits(&y)) != 0) {
			x[0] = y | z << (32 - k);
			z >>= k;
			}
		else
			x[0] = y;
		i = b->wds = (x[1] = z) ? 2 : 1;
		}
	else {
#ifdef BSD_QDTOA_DEBUG
		if (!z)
			Bug("Zero passed to d2b");
#endif
		k = lo0bits(&z);
		x[0] = z;
		i = b->wds = 1;
		k += 32;
		}
#else
	if (y = d1) {
		if (k = lo0bits(&y))
			if (k >= 16) {
				x[0] = y | z << 32 - k & 0xffff;
				x[1] = z >> k - 16 & 0xffff;
				x[2] = z >> k;
				i = 2;
				}
			else {
				x[0] = y & 0xffff;
				x[1] = y >> 16 | z << 16 - k & 0xffff;
				x[2] = z >> k & 0xffff;
				x[3] = z >> k+16;
				i = 3;
				}
		else {
			x[0] = y & 0xffff;
			x[1] = y >> 16;
			x[2] = z & 0xffff;
			x[3] = z >> 16;
			i = 3;
			}
		}
	else {
#ifdef BSD_QDTOA_DEBUG
		if (!z)
			Bug("Zero passed to d2b");
#endif
		k = lo0bits(&z);
		if (k >= 16) {
			x[0] = z;
			i = 0;
			}
		else {
			x[0] = z & 0xffff;
			x[1] = z >> 16;
			i = 1;
			}
		k += 32;
		}
	while(!x[i])
		--i;
	b->wds = i + 1;
#endif
#ifndef Sudden_Underflow
	if (de) {
#endif
#ifdef IBM
		*e = (de - Bias - (P-1) << 2) + k;
		*bits = 4*P + 8 - k - hi0bits(word0(d) & Frac_mask);
#else
		*e = de - Bias - (P-1) + k;
		*bits = P - k;
#endif
#ifndef Sudden_Underflow
		}
	else {
		*e = de - Bias - (P-1) + 1 + k;
#ifdef Pack_32
		*bits = 32*i - hi0bits(x[i-1]);
#else
		*bits = (i+2)*16 - hi0bits(x[i]);
#endif
		}
#endif
	return b;
	}
#undef d0
#undef d1

 static double
ratio
#ifdef KR_headers
	(a, b) Bigint *a, *b;
#else
	(Bigint *a, Bigint *b)
#endif
{
	double da, db;
	int k, ka, kb;

	da = b2d(a, &ka);
	db = b2d(b, &kb);
#ifdef Pack_32
	k = ka - kb + 32*(a->wds - b->wds);
#else
	k = ka - kb + 16*(a->wds - b->wds);
#endif
#ifdef IBM
	if (k > 0) {
		word0(da) += (k >> 2)*Exp_msk1;
		if (k &= 3)
			da *= 1 << k;
		}
	else {
		k = -k;
		word0(db) += (k >> 2)*Exp_msk1;
		if (k &= 3)
			db *= 1 << k;
		}
#else
	if (k > 0)
		word0(da) += k*Exp_msk1;
	else {
		k = -k;
		word0(db) += k*Exp_msk1;
		}
#endif
	return da / db;
	}

static CONST double
tens[] = {
		1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9,
		1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19,
		1e20, 1e21, 1e22
#ifdef VAX
		, 1e23, 1e24
#endif
		};

#ifdef IEEE_Arith
static CONST double bigtens[] = { 1e16, 1e32, 1e64, 1e128, 1e256 };
static CONST double tinytens[] = { 1e-16, 1e-32, 1e-64, 1e-128, 1e-256 };
#define n_bigtens 5
#else
#ifdef IBM
static CONST double bigtens[] = { 1e16, 1e32, 1e64 };
static CONST double tinytens[] = { 1e-16, 1e-32, 1e-64 };
#define n_bigtens 3
#else
static CONST double bigtens[] = { 1e16, 1e32 };
static CONST double tinytens[] = { 1e-16, 1e-32 };
#define n_bigtens 2
#endif
#endif

#ifdef BAD_DOUBLE_COMPARISON
/* The pre-release gcc3.3 shipped with SuSE 8.2 has a bug which causes
   the comparison 1e-100 == 0.0 to return true. As a workaround, we compare
   it to a global variable containing 0.0, which produces correct assembler
   output. */
static double g_double_zero = 0.0;
#endif

static double qstrtod(CONST char *s00, CONST char **se, bool *ok)
{
	int bb2, bb5, bbe, bd2, bd5, bbbits, bs2, c, dsign,
		 e, e1, esign, i, j, k, nd, nd0, nf, nz, nz0, sign;
	CONST char *s, *s0, *s1;
	double aadj, aadj1, adj, rv, rv0;
	Long L;
	ULong y, z;
	Bigint *bb1, *bd0;
	Bigint *bb = NULL, *bd = NULL, *bs = NULL, *delta = NULL;/* pacify gcc */

/*
#ifndef KR_headers
	CONST char decimal_point = localeconv()->decimal_point[0];
#else
	CONST char decimal_point = '.';
#endif */
    	if (ok != 0)
	    *ok = true;

	CONST char decimal_point = '.';

	sign = nz0 = nz = 0;
	rv = 0.;


	for(s = s00; isspace((unsigned char) *s); s++)
		;

	if (*s == '-') {
		sign = 1;
		s++;
	} else if (*s == '+') {
		s++;
	}

	if (*s == '\0') {
		s = s00;
		goto ret;
	}

	if (*s == '0') {
		nz0 = 1;
		while(*++s == '0') ;
		if (!*s)
			goto ret;
		}
	s0 = s;
	y = z = 0;
	for(nd = nf = 0; (c = *s) >= '0' && c <= '9'; nd++, s++)
		if (nd < 9)
			y = 10*y + c - '0';
		else if (nd < 16)
			z = 10*z + c - '0';
	nd0 = nd;
	if (c == decimal_point) {
		c = *++s;
		if (!nd) {
			for(; c == '0'; c = *++s)
				nz++;
			if (c > '0' && c <= '9') {
				s0 = s;
				nf += nz;
				nz = 0;
				goto have_dig;
				}
			goto dig_done;
			}
		for(; c >= '0' && c <= '9'; c = *++s) {
 have_dig:
			nz++;
			if (c -= '0') {
				nf += nz;
				for(i = 1; i < nz; i++)
					if (nd++ < 9)
						y *= 10;
					else if (nd <= DBL_DIG + 1)
						z *= 10;
				if (nd++ < 9)
					y = 10*y + c;
				else if (nd <= DBL_DIG + 1)
					z = 10*z + c;
				nz = 0;
				}
			}
		}
 dig_done:
	e = 0;
	if (c == 'e' || c == 'E') {
		if (!nd && !nz && !nz0) {
			s = s00;
			goto ret;
			}
		s00 = s;
		esign = 0;
		switch(c = *++s) {
			case '-':
				esign = 1;
			case '+':
				c = *++s;
			}
		if (c >= '0' && c <= '9') {
			while(c == '0')
				c = *++s;
			if (c > '0' && c <= '9') {
				L = c - '0';
				s1 = s;
				while((c = *++s) >= '0' && c <= '9')
					L = 10*L + c - '0';
				if (s - s1 > 8 || L > 19999)
					/* Avoid confusion from exponents
					 * so large that e might overflow.
					 */
					e = 19999; /* safe for 16 bit ints */
				else
					e = (int)L;
				if (esign)
					e = -e;
				}
			else
				e = 0;
			}
		else
			s = s00;
		}
	if (!nd) {
		if (!nz && !nz0)
			s = s00;
		goto ret;
		}
	e1 = e -= nf;

	/* Now we have nd0 digits, starting at s0, followed by a
	 * decimal point, followed by nd-nd0 digits.  The number we're
	 * after is the integer represented by those digits times
	 * 10**e */

	if (!nd0)
		nd0 = nd;
	k = nd < DBL_DIG + 1 ? nd : DBL_DIG + 1;
	rv = y;
	if (k > 9)
		rv = tens[k - 9] * rv + z;
	bd0 = 0;
	if (nd <= DBL_DIG
#ifndef RND_PRODQUOT
		&& FLT_ROUNDS == 1
#endif
			) {
		if (!e)
			goto ret;
		if (e > 0) {
			if (e <= Ten_pmax) {
#ifdef VAX
				goto vax_ovfl_check;
#else
				/* rv = */ rounded_product(rv, tens[e]);
				goto ret;
#endif
				}
			i = DBL_DIG - nd;
			if (e <= Ten_pmax + i) {
				/* A fancier test would sometimes let us do
				 * this for larger i values.
				 */
				e -= i;
				rv *= tens[i];
#ifdef VAX
				/* VAX exponent range is so narrow we must
				 * worry about overflow here...
				 */
 vax_ovfl_check:
				word0(rv) -= P*Exp_msk1;
				/* rv = */ rounded_product(rv, tens[e]);
				if ((word0(rv) & Exp_mask)
				 > Exp_msk1*(DBL_MAX_EXP+Bias-1-P))
					goto ovfl;
				word0(rv) += P*Exp_msk1;
#else
				/* rv = */ rounded_product(rv, tens[e]);
#endif
				goto ret;
				}
			}
#ifndef Inaccurate_Divide
		else if (e >= -Ten_pmax) {
			/* rv = */ rounded_quotient(rv, tens[-e]);
			goto ret;
			}
#endif
		}
	e1 += nd - k;

	/* Get starting approximation = rv * 10**e1 */

	if (e1 > 0) {
		if ((i = e1 & 15) != 0)
			rv *= tens[i];
		if (e1 &= ~15) {
			if (e1 > DBL_MAX_10_EXP) {
 ovfl:
//				errno = ERANGE;
    	    	    	    	if (ok != 0)
				    *ok = false;
#ifdef __STDC__
				rv = HUGE_VAL;
#else
				/* Can't trust HUGE_VAL */
#ifdef IEEE_Arith
				word0(rv) = Exp_mask;
				word1(rv) = 0;
#else
				word0(rv) = Big0;
				word1(rv) = Big1;
#endif
#endif
				if (bd0)
					goto retfree;
				goto ret;
				}
			if (e1 >>= 4) {
				for(j = 0; e1 > 1; j++, e1 >>= 1)
					if (e1 & 1)
						rv *= bigtens[j];
			/* The last multiplication could overflow. */
				word0(rv) -= P*Exp_msk1;
				rv *= bigtens[j];
				if ((z = word0(rv) & Exp_mask)
				 > Exp_msk1*(DBL_MAX_EXP+Bias-P))
					goto ovfl;
				if (z > Exp_msk1*(DBL_MAX_EXP+Bias-1-P)) {
					/* set to largest number */
					/* (Can't trust DBL_MAX) */
					word0(rv) = Big0;
					word1(rv) = Big1;
					}
				else
					word0(rv) += P*Exp_msk1;
				}

			}
		}
	else if (e1 < 0) {
		e1 = -e1;
		if ((i = e1 & 15) != 0)
			rv /= tens[i];
		if (e1 &= ~15) {
			e1 >>= 4;
			if (e1 >= 1 << n_bigtens)
				goto undfl;
			for(j = 0; e1 > 1; j++, e1 >>= 1)
				if (e1 & 1)
					rv *= tinytens[j];
			/* The last multiplication could underflow. */
			rv0 = rv;
			rv *= tinytens[j];
#ifdef BAD_DOUBLE_COMPARISON
    	    	    	if (rv == g_double_zero)
#else
    	    	    	if (!rv)
#endif
			{
				rv = 2.*rv0;
				rv *= tinytens[j];
#ifdef BAD_DOUBLE_COMPARISON
    	    	    	if (rv == g_double_zero)
#else
    	    	    	if (!rv)
#endif
				{
 undfl:
					rv = 0.;
//					errno = ERANGE;
    	    	    	    	    	if (ok != 0)
					    *ok = false;
					if (bd0)
						goto retfree;
					goto ret;
					}
				word0(rv) = Tiny0;
				word1(rv) = Tiny1;
				/* The refinement below will clean
				 * this approximation up.
				 */
				}
			}
		}

	/* Now the hard part -- adjusting rv to the correct value.*/

	/* Put digits into bd: true value = bd * 10^e */

	bd0 = s2b(s0, nd0, nd, y);

	for(;;) {
		bd = Balloc(bd0->k);
		Bcopy(bd, bd0);
		bb = d2b(rv, &bbe, &bbbits);	/* rv = bb * 2^bbe */
		bs = i2b(1);

		if (e >= 0) {
			bb2 = bb5 = 0;
			bd2 = bd5 = e;
			}
		else {
			bb2 = bb5 = -e;
			bd2 = bd5 = 0;
			}
		if (bbe >= 0)
			bb2 += bbe;
		else
			bd2 -= bbe;
		bs2 = bb2;
#ifdef Sudden_Underflow
#ifdef IBM
		j = 1 + 4*P - 3 - bbbits + ((bbe + bbbits - 1) & 3);
#else
		j = P + 1 - bbbits;
#endif
#else
		i = bbe + bbbits - 1;	/* logb(rv) */
		if (i < Emin)	/* denormal */
			j = bbe + (P-Emin);
		else
			j = P + 1 - bbbits;
#endif
		bb2 += j;
		bd2 += j;
		i = bb2 < bd2 ? bb2 : bd2;
		if (i > bs2)
			i = bs2;
		if (i > 0) {
			bb2 -= i;
			bd2 -= i;
			bs2 -= i;
			}
		if (bb5 > 0) {
			bs = pow5mult(bs, bb5);
			bb1 = mult(bs, bb);
			Bfree(bb);
			bb = bb1;
			}
		if (bb2 > 0)
			bb = lshift(bb, bb2);
		if (bd5 > 0)
			bd = pow5mult(bd, bd5);
		if (bd2 > 0)
			bd = lshift(bd, bd2);
		if (bs2 > 0)
			bs = lshift(bs, bs2);
		delta = diff(bb, bd);
		dsign = delta->sign;
		delta->sign = 0;
		i = cmp(delta, bs);
		if (i < 0) {
			/* Error is less than half an ulp -- check for
			 * special case of mantissa a power of two.
			 */
			if (dsign || word1(rv) || word0(rv) & Bndry_mask)
				break;
			delta = lshift(delta,Log2P);
			if (cmp(delta, bs) > 0)
				goto drop_down;
			break;
			}
		if (i == 0) {
			/* exactly half-way between */
			if (dsign) {
				if ((word0(rv) & Bndry_mask1) == Bndry_mask1
				 &&  word1(rv) == 0xffffffff) {
					/*boundary case -- increment exponent*/
					word0(rv) = (word0(rv) & Exp_mask)
						+ Exp_msk1
#ifdef IBM
						| Exp_msk1 >> 4
#endif
						;
					word1(rv) = 0;
					break;
					}
				}
			else if (!(word0(rv) & Bndry_mask) && !word1(rv)) {
 drop_down:
				/* boundary case -- decrement exponent */
#ifdef Sudden_Underflow
				L = word0(rv) & Exp_mask;
#ifdef IBM
				if (L <  Exp_msk1)
#else
				if (L <= Exp_msk1)
#endif
					goto undfl;
				L -= Exp_msk1;
#else
				L = (word0(rv) & Exp_mask) - Exp_msk1;
#endif
				word0(rv) = L | Bndry_mask1;
				word1(rv) = 0xffffffff;
#ifdef IBM
				goto cont;
#else
				break;
#endif
				}
#ifndef ROUND_BIASED
			if (!(word1(rv) & LSB))
				break;
#endif
			if (dsign)
				rv += ulp(rv);
#ifndef ROUND_BIASED
			else {
				rv -= ulp(rv);
#ifndef Sudden_Underflow
#ifdef BAD_DOUBLE_COMPARISON
    	    	    	if (rv == g_double_zero)
#else
    	    	    	if (!rv)
#endif
					goto undfl;
#endif
				}
#endif
			break;
			}
		if ((aadj = ratio(delta, bs)) <= 2.) {
			if (dsign)
				aadj = aadj1 = 1.;
			else if (word1(rv) || word0(rv) & Bndry_mask) {
#ifndef Sudden_Underflow
				if (word1(rv) == Tiny1 && !word0(rv))
					goto undfl;
#endif
				aadj = 1.;
				aadj1 = -1.;
				}
			else {
				/* special case -- power of FLT_RADIX to be */
				/* rounded down... */

				if (aadj < 2./FLT_RADIX)
					aadj = 1./FLT_RADIX;
				else
					aadj *= 0.5;
				aadj1 = -aadj;
				}
			}
		else {
			aadj *= 0.5;
			aadj1 = dsign ? aadj : -aadj;
#ifdef Check_FLT_ROUNDS
			switch(FLT_ROUNDS) {
				case 2: /* towards +infinity */
					aadj1 -= 0.5;
					break;
				case 0: /* towards 0 */
				case 3: /* towards -infinity */
					aadj1 += 0.5;
				}
#else
			if (FLT_ROUNDS == 0)
				aadj1 += 0.5;
#endif
			}
		y = word0(rv) & Exp_mask;

		/* Check for overflow */

		if (y == Exp_msk1*(DBL_MAX_EXP+Bias-1)) {
			rv0 = rv;
			word0(rv) -= P*Exp_msk1;
			adj = aadj1 * ulp(rv);
			rv += adj;
			if ((word0(rv) & Exp_mask) >=
					Exp_msk1*(DBL_MAX_EXP+Bias-P)) {
				if (word0(rv0) == Big0 && word1(rv0) == Big1)
					goto ovfl;
				word0(rv) = Big0;
				word1(rv) = Big1;
				goto cont;
				}
			else
				word0(rv) += P*Exp_msk1;
			}
		else {
#ifdef Sudden_Underflow
			if ((word0(rv) & Exp_mask) <= P*Exp_msk1) {
				rv0 = rv;
				word0(rv) += P*Exp_msk1;
				adj = aadj1 * ulp(rv);
				rv += adj;
#ifdef IBM
				if ((word0(rv) & Exp_mask) <  P*Exp_msk1)
#else
				if ((word0(rv) & Exp_mask) <= P*Exp_msk1)
#endif
					{
					if (word0(rv0) == Tiny0
					 && word1(rv0) == Tiny1)
						goto undfl;
					word0(rv) = Tiny0;
					word1(rv) = Tiny1;
					goto cont;
					}
				else
					word0(rv) -= P*Exp_msk1;
				}
			else {
				adj = aadj1 * ulp(rv);
				rv += adj;
				}
#else
			/* Compute adj so that the IEEE rounding rules will
			 * correctly round rv + adj in some half-way cases.
			 * If rv * ulp(rv) is denormalized (i.e.,
			 * y <= (P-1)*Exp_msk1), we must adjust aadj to avoid
			 * trouble from bits lost to denormalization;
			 * example: 1.2e-307 .
			 */
			if (y <= (P-1)*Exp_msk1 && aadj >= 1.) {
				aadj1 = (double)(int)(aadj + 0.5);
				if (!dsign)
					aadj1 = -aadj1;
				}
			adj = aadj1 * ulp(rv);
			rv += adj;
#endif
			}
		z = word0(rv) & Exp_mask;
		if (y == z) {
			/* Can we stop now? */
			L = (Long) aadj;
			aadj -= L;
			/* The tolerances below are conservative. */
			if (dsign || word1(rv) || word0(rv) & Bndry_mask) {
				if (aadj < .4999999 || aadj > .5000001)
					break;
				}
			else if (aadj < .4999999/FLT_RADIX)
				break;
			}
 cont:
		Bfree(bb);
		Bfree(bd);
		Bfree(bs);
		Bfree(delta);
		}
 retfree:
	Bfree(bb);
	Bfree(bd);
	Bfree(bs);
	Bfree(bd0);
	Bfree(delta);
 ret:
	if (se)
		*se = (char *)s;
	return sign ? -rv : rv;
	}

 static int
quorem
#ifdef KR_headers
	(b, S) Bigint *b, *S;
#else
	(Bigint *b, Bigint *S)
#endif
{
	int n;
	Long borrow, y;
	ULong carry, q, ys;
	ULong *bx, *bxe, *sx, *sxe;
#ifdef Pack_32
	Long z;
	ULong si, zs;
#endif

	n = S->wds;
#ifdef BSD_QDTOA_DEBUG
	/*debug*/ if (b->wds > n)
	/*debug*/	Bug("oversize b in quorem");
#endif
	if (b->wds < n)
		return 0;
	sx = S->x;
	sxe = sx + --n;
	bx = b->x;
	bxe = bx + n;
	q = *bxe / (*sxe + 1);	/* ensure q <= true quotient */
#ifdef BSD_QDTOA_DEBUG
	/*debug*/ if (q > 9)
	/*debug*/	Bug("oversized quotient in quorem");
#endif
	if (q) {
		borrow = 0;
		carry = 0;
		do {
#ifdef Pack_32
			si = *sx++;
			ys = (si & 0xffff) * q + carry;
			zs = (si >> 16) * q + (ys >> 16);
			carry = zs >> 16;
			y = (*bx & 0xffff) - (ys & 0xffff) + borrow;
			borrow = y >> 16;
			Sign_Extend(borrow, y);
			z = (*bx >> 16) - (zs & 0xffff) + borrow;
			borrow = z >> 16;
			Sign_Extend(borrow, z);
			Storeinc(bx, z, y);
#else
			ys = *sx++ * q + carry;
			carry = ys >> 16;
			y = *bx - (ys & 0xffff) + borrow;
			borrow = y >> 16;
			Sign_Extend(borrow, y);
			*bx++ = y & 0xffff;
#endif
			}
			while(sx <= sxe);
		if (!*bxe) {
			bx = b->x;
			while(--bxe > bx && !*bxe)
				--n;
			b->wds = n;
			}
		}
	if (cmp(b, S) >= 0) {
		q++;
		borrow = 0;
		carry = 0;
		bx = b->x;
		sx = S->x;
		do {
#ifdef Pack_32
			si = *sx++;
			ys = (si & 0xffff) + carry;
			zs = (si >> 16) + (ys >> 16);
			carry = zs >> 16;
			y = (*bx & 0xffff) - (ys & 0xffff) + borrow;
			borrow = y >> 16;
			Sign_Extend(borrow, y);
			z = (*bx >> 16) - (zs & 0xffff) + borrow;
			borrow = z >> 16;
			Sign_Extend(borrow, z);
			Storeinc(bx, z, y);
#else
			ys = *sx++ + carry;
			carry = ys >> 16;
			y = *bx - (ys & 0xffff) + borrow;
			borrow = y >> 16;
			Sign_Extend(borrow, y);
			*bx++ = y & 0xffff;
#endif
			}
			while(sx <= sxe);
		bx = b->x;
		bxe = bx + n;
		if (!*bxe) {
			while(--bxe > bx && !*bxe)
				--n;
			b->wds = n;
			}
		}
	return q;
	}

/* dtoa for IEEE arithmetic (dmg): convert double to ASCII string.
 *
 * Inspired by "How to Print Floating-Point Numbers Accurately" by
 * Guy L. Steele, Jr. and Jon L. White [Proc. ACM SIGPLAN '90, pp. 92-101].
 *
 * Modifications:
 *	1. Rather than iterating, we use a simple numeric overestimate
 *	   to determine k = floor(log10(d)).  We scale relevant
 *	   quantities using O(log2(k)) rather than O(k) multiplications.
 *	2. For some modes > 2 (corresponding to ecvt and fcvt), we don't
 *	   try to generate digits strictly left to right.  Instead, we
 *	   compute with fewer bits and propagate the carry if necessary
 *	   when rounding the final digit up.  This is often faster.
 *	3. Under the assumption that input will be rounded nearest,
 *	   mode 0 renders 1e23 as 1e23 rather than 9.999999999999999e22.
 *	   That is, we allow equality in stopping tests when the
 *	   round-nearest rule will give the same floating-point value
 *	   as would satisfaction of the stopping test with strict
 *	   inequality.
 *	4. We remove common factors of powers of 2 from relevant
 *	   quantities.
 *	5. When converting floating-point integers less than 1e16,
 *	   we use floating-point arithmetic rather than resorting
 *	   to multiple-precision integers.
 *	6. When asked to produce fewer than 15 digits, we first try
 *	   to get by with floating-point arithmetic; we resort to
 *	   multiple-precision integer arithmetic only if we cannot
 *	   guarantee that the floating-point calculation has given
 *	   the correctly rounded result.  For k requested digits and
 *	   "uniformly" distributed input, the probability is
 *	   something like 10^(k-15) that we must resort to the Long
 *	   calculation.
 */

static char *
qdtoa
#ifdef KR_headers
	(d, mode, ndigits, decpt, sign, rve, resultp)
	double d; int mode, ndigits, *decpt, *sign; char **rve, **resultp;
#else
	(double d, int mode, int ndigits, int *decpt, int *sign, char **rve,
	 char **resultp)
#endif
{
 /*	Arguments ndigits, decpt, sign are similar to those
	of ecvt and fcvt; trailing zeros are suppressed from
	the returned string.  If not null, *rve is set to point
	to the end of the return value.  If d is +-Infinity or NaN,
	then *decpt is set to 9999.

	mode:
		0 ==> shortest string that yields d when read in
			and rounded to nearest.
		1 ==> like 0, but with Steele & White stopping rule;
			e.g. with IEEE P754 arithmetic , mode 0 gives
			1e23 whereas mode 1 gives 9.999999999999999e22.
		2 ==> max(1,ndigits) significant digits.  This gives a
			return value similar to that of ecvt, except
			that trailing zeros are suppressed.
		3 ==> through ndigits past the decimal point.  This
			gives a return value similar to that from fcvt,
			except that trailing zeros are suppressed, and
			ndigits can be negative.
		4-9 should give the same return values as 2-3, i.e.,
			4 <= mode <= 9 ==> same return as mode
			2 + (mode & 1).  These modes are mainly for
			debugging; often they run slower but sometimes
			faster than modes 2-3.
		4,5,8,9 ==> left-to-right digit generation.
		6-9 ==> don't try fast floating-point estimate
			(if applicable).

		Values of mode other than 0-9 are treated as mode 0.

		Sufficient space is allocated to the return value
		to hold the suppressed trailing zeros.
	*/

	int bbits, b2, b5, be, dig, i, ieps, ilim0,
		j, j1, k, k0, k_check, leftright, m2, m5, s2, s5,
		try_quick;
	int ilim = 0, ilim1 = 0, spec_case = 0;	/* pacify gcc */
	Long L;
#ifndef Sudden_Underflow
	int denorm;
	ULong x;
#endif
	Bigint *b, *b1, *delta, *mhi, *S;
	Bigint *mlo = NULL; /* pacify gcc */
	double d2, ds, eps;
	char *s, *s0;

	if (word0(d) & Sign_bit) {
		/* set sign for everything, including 0's and NaNs */
		*sign = 1;
		word0(d) &= ~Sign_bit;	/* clear sign bit */
		}
	else
		*sign = 0;

#if defined(IEEE_Arith) + defined(VAX)
#ifdef IEEE_Arith
	if ((word0(d) & Exp_mask) == Exp_mask)
#else
	if (word0(d)  == 0x8000)
#endif
		{
		/* Infinity or NaN */
		*decpt = 9999;
		s =
#ifdef IEEE_Arith
			!word1(d) && !(word0(d) & 0xfffff) ? (char*)"Infinity" :
#endif
				(char*)"NaN";
		if (rve)
			*rve =
#ifdef IEEE_Arith
				s[3] ? s + 8 :
#endif
						s + 3;
		return s;
		}
#endif
#ifdef IBM
	d += 0; /* normalize */
#endif
#ifdef BAD_DOUBLE_COMPARISON
    	if (d == g_double_zero)
#else
	if (!d)
#endif
    	    	{
		*decpt = 1;
		s = "0";
		if (rve)
			*rve = s + 1;
		return s;
		}

	b = d2b(d, &be, &bbits);
#ifdef Sudden_Underflow
	i = (int)(word0(d) >> Exp_shift1 & (Exp_mask>>Exp_shift1));
#else
	if ((i = (int)(word0(d) >> Exp_shift1 & (Exp_mask>>Exp_shift1))) != 0) {
#endif
		d2 = d;
		word0(d2) &= Frac_mask1;
		word0(d2) |= Exp_11;
#ifdef IBM
		if (j = 11 - hi0bits(word0(d2) & Frac_mask))
			d2 /= 1 << j;
#endif

		/* log(x)	~=~ log(1.5) + (x-1.5)/1.5
		 * log10(x)	 =  log(x) / log(10)
		 *		~=~ log(1.5)/log(10) + (x-1.5)/(1.5*log(10))
		 * log10(d) = (i-Bias)*log(2)/log(10) + log10(d2)
		 *
		 * This suggests computing an approximation k to log10(d) by
		 *
		 * k = (i - Bias)*0.301029995663981
		 *	+ ( (d2-1.5)*0.289529654602168 + 0.176091259055681 );
		 *
		 * We want k to be too large rather than too small.
		 * The error in the first-order Taylor series approximation
		 * is in our favor, so we just round up the constant enough
		 * to compensate for any error in the multiplication of
		 * (i - Bias) by 0.301029995663981; since |i - Bias| <= 1077,
		 * and 1077 * 0.30103 * 2^-52 ~=~ 7.2e-14,
		 * adding 1e-13 to the constant term more than suffices.
		 * Hence we adjust the constant term to 0.1760912590558.
		 * (We could get a more accurate k by invoking log10,
		 *  but this is probably not worthwhile.)
		 */

		i -= Bias;
#ifdef IBM
		i <<= 2;
		i += j;
#endif
#ifndef Sudden_Underflow
		denorm = 0;
		}
	else {
		/* d is denormalized */

		i = bbits + be + (Bias + (P-1) - 1);
		x = i > 32  ? word0(d) << (64 - i) | word1(d) >> (i - 32)
			    : word1(d) << (32 - i);
		d2 = x;
		word0(d2) -= 31*Exp_msk1; /* adjust exponent */
		i -= (Bias + (P-1) - 1) + 1;
		denorm = 1;
		}
#endif
	ds = (d2-1.5)*0.289529654602168 + 0.1760912590558 + i*0.301029995663981;
	k = (int)ds;
	if (ds < 0. && ds != k)
		k--;	/* want k = floor(ds) */
	k_check = 1;
	if (k >= 0 && k <= Ten_pmax) {
		if (d < tens[k])
			k--;
		k_check = 0;
		}
	j = bbits - i - 1;
	if (j >= 0) {
		b2 = 0;
		s2 = j;
		}
	else {
		b2 = -j;
		s2 = 0;
		}
	if (k >= 0) {
		b5 = 0;
		s5 = k;
		s2 += k;
		}
	else {
		b2 -= k;
		b5 = -k;
		s5 = 0;
		}
	if (mode < 0 || mode > 9)
		mode = 0;
	try_quick = 1;
	if (mode > 5) {
		mode -= 4;
		try_quick = 0;
		}
	leftright = 1;
	switch(mode) {
		case 0:
		case 1:
			ilim = ilim1 = -1;
			i = 18;
			ndigits = 0;
			break;
		case 2:
			leftright = 0;
			/* no break */
		case 4:
			if (ndigits <= 0)
				ndigits = 1;
			ilim = ilim1 = i = ndigits;
			break;
		case 3:
			leftright = 0;
			/* no break */
		case 5:
			i = ndigits + k + 1;
			ilim = i;
			ilim1 = i - 1;
			if (i <= 0)
				i = 1;
		}
	*resultp = (char *) malloc(i + 1);
	s = s0 = *resultp;

	if (ilim >= 0 && ilim <= Quick_max && try_quick) {

		/* Try to get by with floating-point arithmetic. */

		i = 0;
		d2 = d;
		k0 = k;
		ilim0 = ilim;
		ieps = 2; /* conservative */
		if (k > 0) {
			ds = tens[k&0xf];
			j = k >> 4;
			if (j & Bletch) {
				/* prevent overflows */
				j &= Bletch - 1;
				d /= bigtens[n_bigtens-1];
				ieps++;
				}
			for(; j; j >>= 1, i++)
				if (j & 1) {
					ieps++;
					ds *= bigtens[i];
					}
			d /= ds;
			}
		else if ((j1 = -k) != 0) {
			d *= tens[j1 & 0xf];
			for(j = j1 >> 4; j; j >>= 1, i++)
				if (j & 1) {
					ieps++;
					d *= bigtens[i];
					}
			}
		if (k_check && d < 1. && ilim > 0) {
			if (ilim1 <= 0)
				goto fast_failed;
			ilim = ilim1;
			k--;
			d *= 10.;
			ieps++;
			}
		eps = ieps*d + 7.;
		word0(eps) -= (P-1)*Exp_msk1;
		if (ilim == 0) {
			S = mhi = 0;
			d -= 5.;
			if (d > eps)
				goto one_digit;
			if (d < -eps)
				goto no_digits;
			goto fast_failed;
			}
#ifndef No_leftright
		if (leftright) {
			/* Use Steele & White method of only
			 * generating digits needed.
			 */
			eps = 0.5/tens[ilim-1] - eps;
			for(i = 0;;) {
				L = (Long)d;
				d -= L;
				*s++ = '0' + (int)L;
				if (d < eps)
					goto ret1;
				if (1. - d < eps)
					goto bump_up;
				if (++i >= ilim)
					break;
				eps *= 10.;
				d *= 10.;
				}
			}
		else {
#endif
			/* Generate ilim digits, then fix them up. */
			eps *= tens[ilim-1];
			for(i = 1;; i++, d *= 10.) {
				L = (Long)d;
				d -= L;
				*s++ = '0' + (int)L;
				if (i == ilim) {
					if (d > 0.5 + eps)
						goto bump_up;
					else if (d < 0.5 - eps) {
						while(*--s == '0');
						s++;
						goto ret1;
						}
					break;
					}
				}
#ifndef No_leftright
			}
#endif
 fast_failed:
		s = s0;
		d = d2;
		k = k0;
		ilim = ilim0;
		}

	/* Do we have a "small" integer? */

	if (be >= 0 && k <= Int_max) {
		/* Yes. */
		ds = tens[k];
		if (ndigits < 0 && ilim <= 0) {
			S = mhi = 0;
			if (ilim < 0 || d <= 5*ds)
				goto no_digits;
			goto one_digit;
			}
		for(i = 1;; i++) {
			L = (Long)(d / ds);
			d -= L*ds;
#ifdef Check_FLT_ROUNDS
			/* If FLT_ROUNDS == 2, L will usually be high by 1 */
			if (d < 0) {
				L--;
				d += ds;
				}
#endif
			*s++ = '0' + (int)L;
			if (i == ilim) {
				d += d;
				if (d > ds || (d == ds && L & 1)) {
 bump_up:
					while(*--s == '9')
						if (s == s0) {
							k++;
							*s = '0';
							break;
							}
					++*s++;
					}
				break;
				}
#ifdef BAD_DOUBLE_COMPARISON
    	    	    	if ((d *= 10.) == g_double_zero)
#else
			if (!(d *= 10.))
#endif
				break;
			}
		goto ret1;
		}

	m2 = b2;
	m5 = b5;
	mhi = mlo = 0;
	if (leftright) {
		if (mode < 2) {
			i =
#ifndef Sudden_Underflow
				denorm ? be + (Bias + (P-1) - 1 + 1) :
#endif
#ifdef IBM
				1 + 4*P - 3 - bbits + ((bbits + be - 1) & 3);
#else
				1 + P - bbits;
#endif
			}
		else {
			j = ilim - 1;
			if (m5 >= j)
				m5 -= j;
			else {
				s5 += j -= m5;
				b5 += j;
				m5 = 0;
				}
			if ((i = ilim) < 0) {
				m2 -= i;
				i = 0;
				}
			}
		b2 += i;
		s2 += i;
		mhi = i2b(1);
		}
	if (m2 > 0 && s2 > 0) {
		i = m2 < s2 ? m2 : s2;
		b2 -= i;
		m2 -= i;
		s2 -= i;
		}
	if (b5 > 0) {
		if (leftright) {
			if (m5 > 0) {
				mhi = pow5mult(mhi, m5);
				b1 = mult(mhi, b);
				Bfree(b);
				b = b1;
				}
			if ((j = b5 - m5) != 0)
				b = pow5mult(b, j);
			}
		else
			b = pow5mult(b, b5);
		}
	S = i2b(1);
	if (s5 > 0)
		S = pow5mult(S, s5);

	/* Check for special case that d is a normalized power of 2. */

	if (mode < 2) {
		if (!word1(d) && !(word0(d) & Bndry_mask)
#ifndef Sudden_Underflow
		 && word0(d) & Exp_mask
#endif
				) {
			/* The special case */
			b2 += Log2P;
			s2 += Log2P;
			spec_case = 1;
			}
		else
			spec_case = 0;
		}

	/* Arrange for convenient computation of quotients:
	 * shift left if necessary so divisor has 4 leading 0 bits.
	 *
	 * Perhaps we should just compute leading 28 bits of S once
	 * and for all and pass them and a shift to quorem, so it
	 * can do shifts and ors to compute the numerator for q.
	 */
#ifdef Pack_32
	if ((i = ((s5 ? 32 - hi0bits(S->x[S->wds-1]) : 1) + s2) & 0x1f) != 0)
		i = 32 - i;
#else
	if (i = ((s5 ? 32 - hi0bits(S->x[S->wds-1]) : 1) + s2) & 0xf)
		i = 16 - i;
#endif
	if (i > 4) {
		i -= 4;
		b2 += i;
		m2 += i;
		s2 += i;
		}
	else if (i < 4) {
		i += 28;
		b2 += i;
		m2 += i;
		s2 += i;
		}
	if (b2 > 0)
		b = lshift(b, b2);
	if (s2 > 0)
		S = lshift(S, s2);
	if (k_check) {
		if (cmp(b,S) < 0) {
			k--;
			b = multadd(b, 10, 0);	/* we botched the k estimate */
			if (leftright)
				mhi = multadd(mhi, 10, 0);
			ilim = ilim1;
			}
		}
	if (ilim <= 0 && mode > 2) {
		if (ilim < 0 || cmp(b,S = multadd(S,5,0)) <= 0) {
			/* no digits, fcvt style */
 no_digits:
			k = -1 - ndigits;
			goto ret;
			}
 one_digit:
		*s++ = '1';
		k++;
		goto ret;
		}
	if (leftright) {
		if (m2 > 0)
			mhi = lshift(mhi, m2);

		/* Compute mlo -- check for special case
		 * that d is a normalized power of 2.
		 */

		mlo = mhi;
		if (spec_case) {
			mhi = Balloc(mhi->k);
			Bcopy(mhi, mlo);
			mhi = lshift(mhi, Log2P);
			}

		for(i = 1;;i++) {
			dig = quorem(b,S) + '0';
			/* Do we yet have the shortest decimal string
			 * that will round to d?
			 */
			j = cmp(b, mlo);
			delta = diff(S, mhi);
			j1 = delta->sign ? 1 : cmp(b, delta);
			Bfree(delta);
#ifndef ROUND_BIASED
			if (j1 == 0 && !mode && !(word1(d) & 1)) {
				if (dig == '9')
					goto round_9_up;
				if (j > 0)
					dig++;
				*s++ = dig;
				goto ret;
				}
#endif
			if (j < 0 || (j == 0 && !mode
#ifndef ROUND_BIASED
							&& !(word1(d) & 1)
#endif
					)) {
				if (j1 > 0) {
					b = lshift(b, 1);
					j1 = cmp(b, S);
					if ((j1 > 0 || (j1 == 0 && dig & 1))
					&& dig++ == '9')
						goto round_9_up;
					}
				*s++ = dig;
				goto ret;
				}
			if (j1 > 0) {
				if (dig == '9') { /* possible if i == 1 */
 round_9_up:
					*s++ = '9';
					goto roundoff;
					}
				*s++ = dig + 1;
				goto ret;
				}
			*s++ = dig;
			if (i == ilim)
				break;
			b = multadd(b, 10, 0);
			if (mlo == mhi)
				mlo = mhi = multadd(mhi, 10, 0);
			else {
				mlo = multadd(mlo, 10, 0);
				mhi = multadd(mhi, 10, 0);
				}
			}
		}
	else
		for(i = 1;; i++) {
			*s++ = dig = quorem(b,S) + '0';
			if (i >= ilim)
				break;
			b = multadd(b, 10, 0);
			}

	/* Round off last digit */

	b = lshift(b, 1);
	j = cmp(b, S);
	if (j > 0 || (j == 0 && dig & 1)) {
 roundoff:
		while(*--s == '9')
			if (s == s0) {
				k++;
				*s++ = '1';
				goto ret;
				}
		++*s++;
		}
	else {
		while(*--s == '0');
		s++;
		}
 ret:
	Bfree(S);
	if (mhi) {
		if (mlo && mlo != mhi)
			Bfree(mlo);
		Bfree(mhi);
		}
 ret1:
	Bfree(b);
	if (s == s0) {				/* don't return empty string */
		*s++ = '0';
		k = 0;
	}
	*s = 0;
	*decpt = k + 1;
	if (rve)
		*rve = s;
	return s0;
	}

