#include <QtCore/QLocale>

#include "localemodel.h"

static const int g_model_cols = 1;

struct LocaleListItem
{
    int language;
    int country;
};

const LocaleListItem g_locale_list[] = {
    {      1,     0 }, // C/AnyCountry
    {      5,   195 }, // Afrikaans/SouthAfrica
    {      6,     2 }, // Albanian/Albania
    {      8,   186 }, // Arabic/SaudiArabia
    {      8,     3 }, // Arabic/Algeria
    {      8,    17 }, // Arabic/Bahrain
    {      8,    64 }, // Arabic/Egypt
    {      8,   103 }, // Arabic/Iraq
    {      8,   109 }, // Arabic/Jordan
    {      8,   115 }, // Arabic/Kuwait
    {      8,   119 }, // Arabic/Lebanon
    {      8,   122 }, // Arabic/LibyanArabJamahiriya
    {      8,   145 }, // Arabic/Morocco
    {      8,   162 }, // Arabic/Oman
    {      8,   175 }, // Arabic/Qatar
    {      8,   207 }, // Arabic/SyrianArabRepublic
    {      8,   216 }, // Arabic/Tunisia
    {      8,   223 }, // Arabic/UnitedArabEmirates
    {      8,   237 }, // Arabic/Yemen
    {      9,    11 }, // Armenian/Armenia
    {     12,    15 }, // Azerbaijani/Azerbaijan
    {     14,   197 }, // Basque/Spain
    {     20,    33 }, // Bulgarian/Bulgaria
    {     22,    20 }, // Byelorussian/Belarus
    {     24,   197 }, // Catalan/Spain
    {     25,    44 }, // Chinese/China
    {     25,    97 }, // Chinese/HongKong
    {     25,   126 }, // Chinese/Macau
    {     25,   190 }, // Chinese/Singapore
    {     25,   208 }, // Chinese/Taiwan
    {     27,    54 }, // Croatian/Croatia
    {     28,    57 }, // Czech/CzechRepublic
    {     29,    58 }, // Danish/Denmark
    {     30,   151 }, // Dutch/Netherlands
    {     30,    21 }, // Dutch/Belgium
    {     31,   225 }, // English/UnitedStates
    {     31,    13 }, // English/Australia
    {     31,    22 }, // English/Belize
    {     31,    38 }, // English/Canada
    {     31,   104 }, // English/Ireland
    {     31,   107 }, // English/Jamaica
    {     31,   154 }, // English/NewZealand
    {     31,   170 }, // English/Philippines
    {     31,   195 }, // English/SouthAfrica
    {     31,   215 }, // English/TrinidadAndTobago
    {     31,   224 }, // English/UnitedKingdom
    {     31,   240 }, // English/Zimbabwe
    {     33,    68 }, // Estonian/Estonia
    {     34,    71 }, // Faroese/FaroeIslands
    {     36,    73 }, // Finnish/Finland
    {     37,    74 }, // French/France
    {     37,    21 }, // French/Belgium
    {     37,    38 }, // French/Canada
    {     37,   125 }, // French/Luxembourg
    {     37,   142 }, // French/Monaco
    {     37,   206 }, // French/Switzerland
    {     40,   197 }, // Galician/Spain
    {     41,    81 }, // Georgian/Georgia
    {     42,    82 }, // German/Germany
    {     42,    14 }, // German/Austria
    {     42,   123 }, // German/Liechtenstein
    {     42,   125 }, // German/Luxembourg
    {     42,   206 }, // German/Switzerland
    {     43,    85 }, // Greek/Greece
    {     46,   100 }, // Gujarati/India
    {     48,   105 }, // Hebrew/Israel
    {     49,   100 }, // Hindi/India
    {     50,    98 }, // Hungarian/Hungary
    {     51,    99 }, // Icelandic/Iceland
    {     52,   101 }, // Indonesian/Indonesia
    {     58,   106 }, // Italian/Italy
    {     58,   206 }, // Italian/Switzerland
    {     59,   108 }, // Japanese/Japan
    {     61,   100 }, // Kannada/India
    {     63,   110 }, // Kazakh/Kazakhstan
    {     65,   116 }, // Kirghiz/Kyrgyzstan
    {     66,   114 }, // Korean/RepublicOfKorea
    {     71,   118 }, // Latvian/Latvia
    {     73,   124 }, // Lithuanian/Lithuania
    {     74,   127 }, // Macedonian/Macedonia
    {     76,   130 }, // Malay/Malaysia
    {     76,    32 }, // Malay/BruneiDarussalam
    {     80,   100 }, // Marathi/India
    {     82,   143 }, // Mongolian/Mongolia
    {     85,   161 }, // Norwegian/Norway
    {     89,   102 }, // Persian/Iran
    {     90,   172 }, // Polish/Poland
    {     91,   173 }, // Portuguese/Portugal
    {     91,    30 }, // Portuguese/Brazil
    {     92,   100 }, // Punjabi/India
    {     95,   177 }, // Romanian/Romania
    {     96,   178 }, // Russian/RussianFederation
    {     99,   100 }, // Sanskrit/India
    {    108,   191 }, // Slovak/Slovakia
    {    109,   192 }, // Slovenian/Slovenia
    {    111,   197 }, // Spanish/Spain
    {    111,    10 }, // Spanish/Argentina
    {    111,    26 }, // Spanish/Bolivia
    {    111,    43 }, // Spanish/Chile
    {    111,    47 }, // Spanish/Colombia
    {    111,    52 }, // Spanish/CostaRica
    {    111,    61 }, // Spanish/DominicanRepublic
    {    111,    63 }, // Spanish/Ecuador
    {    111,    65 }, // Spanish/ElSalvador
    {    111,    90 }, // Spanish/Guatemala
    {    111,    96 }, // Spanish/Honduras
    {    111,   139 }, // Spanish/Mexico
    {    111,   155 }, // Spanish/Nicaragua
    {    111,   166 }, // Spanish/Panama
    {    111,   168 }, // Spanish/Paraguay
    {    111,   169 }, // Spanish/Peru
    {    111,   174 }, // Spanish/PuertoRico
    {    111,   227 }, // Spanish/Uruguay
    {    111,   231 }, // Spanish/Venezuela
    {    113,   111 }, // Swahili/Kenya
    {    114,   205 }, // Swedish/Sweden
    {    114,    73 }, // Swedish/Finland
    {    117,   100 }, // Tamil/India
    {    119,   100 }, // Telugu/India
    {    120,   211 }, // Thai/Thailand
    {    125,   217 }, // Turkish/Turkey
    {    129,   222 }, // Ukrainian/Ukraine
    {    130,   163 }, // Urdu/Pakistan
    {    131,   228 }, // Uzbek/Uzbekistan
    {    132,   232 }, // Vietnamese/VietNam
    {    141,   161 }  // Nynorsk/Norway
};
static const int g_locale_list_count = sizeof(g_locale_list)/sizeof(g_locale_list[0]);

LocaleModel::LocaleModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    m_data_list.append(1234.5678);
}

QVariant LocaleModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()
        || role != Qt::DisplayRole
        || index.column() >= g_model_cols
        || index.row() >= g_locale_list_count)
        return QVariant();

    LocaleListItem item = g_locale_list[index.row()];
    QLocale locale((QLocale::Language)item.language, (QLocale::Country)item.country);
    QVariant data = m_data_list.at(index.column());

    switch (index.row()) {
        case 0:
            return locale.toString(data.toDouble());
        default:
            break;
    }

    return QVariant();
}

QVariant LocaleModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
            case 0:
                return QLatin1String("Double");
            default:
                break;
        }
    } else {
        if (section >= g_locale_list_count)
            return QVariant();
        LocaleListItem item = g_locale_list[section];
        return QLocale::languageToString((QLocale::Language)item.language)
                + QLatin1Char('/')
                + QLocale::countryToString((QLocale::Country)item.country);
    }

    return QVariant();
}

QModelIndex LocaleModel::index(int row, int column,
                    const QModelIndex &parent) const
{
    if (parent.isValid()
        || row >= g_locale_list_count
        || column >= g_model_cols)
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex LocaleModel::parent(const QModelIndex&) const
{
    return QModelIndex();
}

int LocaleModel::columnCount(const QModelIndex&) const
{
    return g_model_cols;
}

int LocaleModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return g_locale_list_count;
}

