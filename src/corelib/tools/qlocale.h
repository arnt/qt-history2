/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QLOCALE_H
#define QLOCALE_H

#include <QtCore/qstring.h>

QT_BEGIN_HEADER

QT_MODULE(Core)

class QDataStream;
class QDate;
class QTime;
struct QLocalePrivate;
class QVariant;

class Q_CORE_EXPORT QLocale
{
    friend class QString;
    friend class QByteArray;

public:
    enum Language {
        C = 1,
        Abkhazian = 2,
        Afan = 3,
        Afar = 4,
        Afrikaans = 5,
        Albanian = 6,
        Amharic = 7,
        Arabic = 8,
        Armenian = 9,
        Assamese = 10,
        Aymara = 11,
        Azerbaijani = 12,
        Bashkir = 13,
        Basque = 14,
        Bengali = 15,
        Bhutani = 16,
        Bihari = 17,
        Bislama = 18,
        Breton = 19,
        Bulgarian = 20,
        Burmese = 21,
        Byelorussian = 22,
        Cambodian = 23,
        Catalan = 24,
        Chinese = 25,
        Corsican = 26,
        Croatian = 27,
        Czech = 28,
        Danish = 29,
        Dutch = 30,
        English = 31,
        Esperanto = 32,
        Estonian = 33,
        Faroese = 34,
        FijiLanguage = 35,
        Finnish = 36,
        French = 37,
        Frisian = 38,
        Gaelic = 39,
        Galician = 40,
        Georgian = 41,
        German = 42,
        Greek = 43,
        Greenlandic = 44,
        Guarani = 45,
        Gujarati = 46,
        Hausa = 47,
        Hebrew = 48,
        Hindi = 49,
        Hungarian = 50,
        Icelandic = 51,
        Indonesian = 52,
        Interlingua = 53,
        Interlingue = 54,
        Inuktitut = 55,
        Inupiak = 56,
        Irish = 57,
        Italian = 58,
        Japanese = 59,
        Javanese = 60,
        Kannada = 61,
        Kashmiri = 62,
        Kazakh = 63,
        Kinyarwanda = 64,
        Kirghiz = 65,
        Korean = 66,
        Kurdish = 67,
        Kurundi = 68,
        Laothian = 69,
        Latin = 70,
        Latvian = 71,
        Lingala = 72,
        Lithuanian = 73,
        Macedonian = 74,
        Malagasy = 75,
        Malay = 76,
        Malayalam = 77,
        Maltese = 78,
        Maori = 79,
        Marathi = 80,
        Moldavian = 81,
        Mongolian = 82,
        NauruLanguage = 83,
        Nepali = 84,
        Norwegian = 85,
        Occitan = 86,
        Oriya = 87,
        Pashto = 88,
        Persian = 89,
        Polish = 90,
        Portuguese = 91,
        Punjabi = 92,
        Quechua = 93,
        RhaetoRomance = 94,
        Romanian = 95,
        Russian = 96,
        Samoan = 97,
        Sangho = 98,
        Sanskrit = 99,
        Serbian = 100,
        SerboCroatian = 101,
        Sesotho = 102,
        Setswana = 103,
        Shona = 104,
        Sindhi = 105,
        Singhalese = 106,
        Siswati = 107,
        Slovak = 108,
        Slovenian = 109,
        Somali = 110,
        Spanish = 111,
        Sundanese = 112,
        Swahili = 113,
        Swedish = 114,
        Tagalog = 115,
        Tajik = 116,
        Tamil = 117,
        Tatar = 118,
        Telugu = 119,
        Thai = 120,
        Tibetan = 121,
        Tigrinya = 122,
        TongaLanguage = 123,
        Tsonga = 124,
        Turkish = 125,
        Turkmen = 126,
        Twi = 127,
        Uigur = 128,
        Ukrainian = 129,
        Urdu = 130,
        Uzbek = 131,
        Vietnamese = 132,
        Volapuk = 133,
        Welsh = 134,
        Wolof = 135,
        Xhosa = 136,
        Yiddish = 137,
        Yoruba = 138,
        Zhuang = 139,
        Zulu = 140,
        Nynorsk = 141,
        Bosnian = 142,
        Divehi = 143,
        Manx = 144,
        Cornish = 145,
        LastLanguage = Cornish
    };

    enum Country {
        AnyCountry = 0,
        Afghanistan = 1,
        Albania = 2,
        Algeria = 3,
        AmericanSamoa = 4,
        Andorra = 5,
        Angola = 6,
        Anguilla = 7,
        Antarctica = 8,
        AntiguaAndBarbuda = 9,
        Argentina = 10,
        Armenia = 11,
        Aruba = 12,
        Australia = 13,
        Austria = 14,
        Azerbaijan = 15,
        Bahamas = 16,
        Bahrain = 17,
        Bangladesh = 18,
        Barbados = 19,
        Belarus = 20,
        Belgium = 21,
        Belize = 22,
        Benin = 23,
        Bermuda = 24,
        Bhutan = 25,
        Bolivia = 26,
        BosniaAndHerzegowina = 27,
        Botswana = 28,
        BouvetIsland = 29,
        Brazil = 30,
        BritishIndianOceanTerritory = 31,
        BruneiDarussalam = 32,
        Bulgaria = 33,
        BurkinaFaso = 34,
        Burundi = 35,
        Cambodia = 36,
        Cameroon = 37,
        Canada = 38,
        CapeVerde = 39,
        CaymanIslands = 40,
        CentralAfricanRepublic = 41,
        Chad = 42,
        Chile = 43,
        China = 44,
        ChristmasIsland = 45,
        CocosIslands = 46,
        Colombia = 47,
        Comoros = 48,
        DemocraticRepublicOfCongo = 49,
        PeoplesRepublicOfCongo = 50,
        CookIslands = 51,
        CostaRica = 52,
        IvoryCoast = 53,
        Croatia = 54,
        Cuba = 55,
        Cyprus = 56,
        CzechRepublic = 57,
        Denmark = 58,
        Djibouti = 59,
        Dominica = 60,
        DominicanRepublic = 61,
        EastTimor = 62,
        Ecuador = 63,
        Egypt = 64,
        ElSalvador = 65,
        EquatorialGuinea = 66,
        Eritrea = 67,
        Estonia = 68,
        Ethiopia = 69,
        FalklandIslands = 70,
        FaroeIslands = 71,
        FijiCountry = 72,
        Finland = 73,
        France = 74,
        MetropolitanFrance = 75,
        FrenchGuiana = 76,
        FrenchPolynesia = 77,
        FrenchSouthernTerritories = 78,
        Gabon = 79,
        Gambia = 80,
        Georgia = 81,
        Germany = 82,
        Ghana = 83,
        Gibraltar = 84,
        Greece = 85,
        Greenland = 86,
        Grenada = 87,
        Guadeloupe = 88,
        Guam = 89,
        Guatemala = 90,
        Guinea = 91,
        GuineaBissau = 92,
        Guyana = 93,
        Haiti = 94,
        HeardAndMcDonaldIslands = 95,
        Honduras = 96,
        HongKong = 97,
        Hungary = 98,
        Iceland = 99,
        India = 100,
        Indonesia = 101,
        Iran = 102,
        Iraq = 103,
        Ireland = 104,
        Israel = 105,
        Italy = 106,
        Jamaica = 107,
        Japan = 108,
        Jordan = 109,
        Kazakhstan = 110,
        Kenya = 111,
        Kiribati = 112,
        DemocraticRepublicOfKorea = 113,
        RepublicOfKorea = 114,
        Kuwait = 115,
        Kyrgyzstan = 116,
        Lao = 117,
        Latvia = 118,
        Lebanon = 119,
        Lesotho = 120,
        Liberia = 121,
        LibyanArabJamahiriya = 122,
        Liechtenstein = 123,
        Lithuania = 124,
        Luxembourg = 125,
        Macau = 126,
        Macedonia = 127,
        Madagascar = 128,
        Malawi = 129,
        Malaysia = 130,
        Maldives = 131,
        Mali = 132,
        Malta = 133,
        MarshallIslands = 134,
        Martinique = 135,
        Mauritania = 136,
        Mauritius = 137,
        Mayotte = 138,
        Mexico = 139,
        Micronesia = 140,
        Moldova = 141,
        Monaco = 142,
        Mongolia = 143,
        Montserrat = 144,
        Morocco = 145,
        Mozambique = 146,
        Myanmar = 147,
        Namibia = 148,
        NauruCountry = 149,
        Nepal = 150,
        Netherlands = 151,
        NetherlandsAntilles = 152,
        NewCaledonia = 153,
        NewZealand = 154,
        Nicaragua = 155,
        Niger = 156,
        Nigeria = 157,
        Niue = 158,
        NorfolkIsland = 159,
        NorthernMarianaIslands = 160,
        Norway = 161,
        Oman = 162,
        Pakistan = 163,
        Palau = 164,
        PalestinianTerritory = 165,
        Panama = 166,
        PapuaNewGuinea = 167,
        Paraguay = 168,
        Peru = 169,
        Philippines = 170,
        Pitcairn = 171,
        Poland = 172,
        Portugal = 173,
        PuertoRico = 174,
        Qatar = 175,
        Reunion = 176,
        Romania = 177,
        RussianFederation = 178,
        Rwanda = 179,
        SaintKittsAndNevis = 180,
        StLucia = 181,
        StVincentAndTheGrenadines = 182,
        Samoa = 183,
        SanMarino = 184,
        SaoTomeAndPrincipe = 185,
        SaudiArabia = 186,
        Senegal = 187,
        Seychelles = 188,
        SierraLeone = 189,
        Singapore = 190,
        Slovakia = 191,
        Slovenia = 192,
        SolomonIslands = 193,
        Somalia = 194,
        SouthAfrica = 195,
        SouthGeorgiaAndTheSouthSandwichIslands = 196,
        Spain = 197,
        SriLanka = 198,
        StHelena = 199,
        StPierreAndMiquelon = 200,
        Sudan = 201,
        Suriname = 202,
        SvalbardAndJanMayenIslands = 203,
        Swaziland = 204,
        Sweden = 205,
        Switzerland = 206,
        SyrianArabRepublic = 207,
        Taiwan = 208,
        Tajikistan = 209,
        Tanzania = 210,
        Thailand = 211,
        Togo = 212,
        Tokelau = 213,
        TongaCountry = 214,
        TrinidadAndTobago = 215,
        Tunisia = 216,
        Turkey = 217,
        Turkmenistan = 218,
        TurksAndCaicosIslands = 219,
        Tuvalu = 220,
        Uganda = 221,
        Ukraine = 222,
        UnitedArabEmirates = 223,
        UnitedKingdom = 224,
        UnitedStates = 225,
        UnitedStatesMinorOutlyingIslands = 226,
        Uruguay = 227,
        Uzbekistan = 228,
        Vanuatu = 229,
        VaticanCityState = 230,
        Venezuela = 231,
        VietNam = 232,
        BritishVirginIslands = 233,
        USVirginIslands = 234,
        WallisAndFutunaIslands = 235,
        WesternSahara = 236,
        Yemen = 237,
        Yugoslavia = 238,
        Zambia = 239,
        Zimbabwe = 240,
        SerbiaAndMontenegro = 241,
        LastCountry = SerbiaAndMontenegro
    };

    enum FormatType { LongFormat, ShortFormat };

    QLocale();
    QLocale(const QString &name);
    QLocale(Language language, Country country = AnyCountry);
    QLocale(const QLocale &other);

    QLocale &operator=(const QLocale &other);

    Language language() const;
    Country country() const;
    QString name() const;

    short toShort(const QString &s, bool *ok = 0, int base = 0) const;
    ushort toUShort(const QString &s, bool *ok = 0, int base = 0) const;
    int toInt(const QString &s, bool *ok = 0, int base = 0) const;
    uint toUInt(const QString &s, bool *ok = 0, int base = 0) const;
    qlonglong toLongLong(const QString &s, bool *ok = 0, int base = 0) const;
    qlonglong toULongLong(const QString &s, bool *ok = 0, int base = 0) const;
    float toFloat(const QString &s, bool *ok = 0) const;
    double toDouble(const QString &s, bool *ok = 0) const;

    QString toString(qlonglong i) const;
    QString toString(qulonglong i) const;
    inline QString toString(short i) const;
    inline QString toString(ushort i) const;
    inline QString toString(int i) const;
    inline QString toString(uint i) const;
    QString toString(double i, char f = 'g', int prec = 6) const;
    inline QString toString(float i, char f = 'g', int prec = 6) const;
    QString toString(const QDate &date, const QString &formatStr) const;
    QString toString(const QDate &date, FormatType format = LongFormat) const;
    QString toString(const QTime &time, const QString &formatStr) const;
    QString toString(const QTime &time, FormatType format = LongFormat) const;

    QString dateFormat(FormatType format = LongFormat) const;
    QString timeFormat(FormatType format = LongFormat) const;

    QChar decimalPoint() const;
    QChar groupSeparator() const;
    QChar percent() const;
    QChar zeroDigit() const;
    QChar negativeSign() const;
    QChar exponential() const;

    QString monthName(int, FormatType format = LongFormat) const;
    QString dayName(int, FormatType format = LongFormat) const;

    inline bool operator==(const QLocale &other) const;
    inline bool operator!=(const QLocale &other) const;

    static QString languageToString(Language language);
    static QString countryToString(Country country);
    static void setDefault(const QLocale &locale);

    static QLocale c() { return QLocale(C); }
    static QLocale system();

private:
    friend struct QLocalePrivate;
    const QLocalePrivate *d;
};
Q_DECLARE_TYPEINFO(QLocale, Q_MOVABLE_TYPE);

inline QString QLocale::toString(short i) const
    { return toString(qlonglong(i)); }
inline QString QLocale::toString(ushort i) const
    { return toString(qulonglong(i)); }
inline QString QLocale::toString(int i) const
    { return toString(qlonglong(i)); }
inline QString QLocale::toString(uint i) const
    { return toString(qulonglong(i)); }
inline QString QLocale::toString(float i, char f, int prec) const
    { return toString(double(i), f, prec); }
inline bool QLocale::operator==(const QLocale &other) const
    { return d == other.d; }
inline bool QLocale::operator!=(const QLocale &other) const
    { return d != other.d; }

#ifndef QT_NO_DATASTREAM
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QLocale &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QLocale &);
#endif

#ifndef QT_NO_SYSTEMLOCALE
class Q_CORE_EXPORT QSystemLocale
{
public:
    QSystemLocale();
    virtual ~QSystemLocale();

    enum QueryType {
        LanguageId, // uint
        CountryId, // uint
        DecimalPoint, // QString
        GroupSeparator, // QString
        ZeroDigit, // QString
        NegativeSign, // QString
        DateFormatLong, // QString
        DateFormatShort, // QString
        TimeFormatLong, // QString
        TimeFormatShort, // QString
        DayNameLong, // QString, in: int
        DayNameShort, // QString, in: int
        MonthNameLong, // QString, in: int
        MonthNameShort, // QString, in: int
        DateToStringLong, // QString, in: QDate
        DateToStringShort, // QString in: QDate
        TimeToStringLong, // QString in: QTime
        TimeToStringShort // QString in: QTime
    };
    virtual QVariant query(QueryType type, QVariant in) const;
    virtual QLocale fallbackLocale() const;
};
#endif


QT_END_HEADER

#endif // QLOCALE_H
