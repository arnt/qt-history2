#! /usr/bin/python

import os
import xml.dom.minidom

language_list = {
    1 : [ "C",                    "  " ],
    2 : [ "Abkhazian",            "ab" ],
    3 : [ "Afan",                 "om" ],
    4 : [ "Afar",                 "aa" ],
    5 : [ "Afrikaans",            "af" ],
    6 : [ "Albanian",             "sq" ],
    7 : [ "Amharic",              "am" ],
    8 : [ "Arabic",               "ar" ],
    9 : [ "Armenian",             "hy" ],
    10 : [ "Assamese",            "as" ],
    11 : [ "Aymara",              "ay" ],
    12 : [ "Azerbaijani",         "az" ],
    13 : [ "Bashkir",             "ba" ],
    14 : [ "Basque",              "eu" ],
    15 : [ "Bengali",             "bn" ],
    16 : [ "Bhutani",             "dz" ],
    17 : [ "Bihari",              "bh" ],
    18 : [ "Bislama",             "bi" ],
    19 : [ "Breton",              "br" ],
    20 : [ "Bulgarian",           "bg" ],
    21 : [ "Burmese",             "my" ],
    22 : [ "Byelorussian",        "be" ],
    23 : [ "Cambodian",           "km" ],
    24 : [ "Catalan",             "ca" ],
    25 : [ "Chinese",             "zh" ],
    26 : [ "Corsican",            "co" ],
    27 : [ "Croatian",            "hr" ],
    28 : [ "Czech",               "cs" ],
    29 : [ "Danish",              "da" ],
    30 : [ "Dutch",               "nl" ],
    31 : [ "English",             "en" ],
    32 : [ "Esperanto",           "eo" ],
    33 : [ "Estonian",            "et" ],
    34 : [ "Faroese",             "fo" ],
    35 : [ "FijiLanguage",        "fj" ],
    36 : [ "Finnish",             "fi" ],
    37 : [ "French",              "fr" ],
    38 : [ "Frisian",             "fy" ],
    39 : [ "Gaelic",              "gd" ],
    40 : [ "Galician",            "gl" ],
    41 : [ "Georgian",            "ka" ],
    42 : [ "German",              "de" ],
    43 : [ "Greek",               "el" ],
    44 : [ "Greenlandic",         "kl" ],
    45 : [ "Guarani",             "gn" ],
    46 : [ "Gujarati",            "gu" ],
    47 : [ "Hausa",               "ha" ],
    48 : [ "Hebrew",              "he" ],
    49 : [ "Hindi",               "hi" ],
    50 : [ "Hungarian",           "hu" ],
    51 : [ "Icelandic",           "is" ],
    52 : [ "Indonesian",          "id" ],
    53 : [ "Interlingua",         "ia" ],
    54 : [ "Interlingue",         "ie" ],
    55 : [ "Inuktitut",           "iu" ],
    56 : [ "Inupiak",             "ik" ],
    57 : [ "Irish",               "ga" ],
    58 : [ "Italian",             "it" ],
    59 : [ "Japanese",            "ja" ],
    60 : [ "Javanese",            "jv" ],
    61 : [ "Kannada",             "kn" ],
    62 : [ "Kashmiri",            "ks" ],
    63 : [ "Kazakh",              "kk" ],
    64 : [ "Kinyarwanda",         "rw" ],
    65 : [ "Kirghiz",             "ky" ],
    66 : [ "Korean",              "ko" ],
    67 : [ "Kurdish",             "ku" ],
    68 : [ "Kurundi",             "rn" ],
    69 : [ "Laothian",            "lo" ],
    70 : [ "Latin",               "la" ],
    71 : [ "Latvian",             "lv" ],
    72 : [ "Lingala",             "ln" ],
    73 : [ "Lithuanian",          "lt" ],
    74 : [ "Macedonian",          "mk" ],
    75 : [ "Malagasy",            "mg" ],
    76 : [ "Malay",               "ms" ],
    77 : [ "Malayalam",           "ml" ],
    78 : [ "Maltese",             "mt" ],
    79 : [ "Maori",               "mi" ],
    80 : [ "Marathi",             "mr" ],
    81 : [ "Moldavian",           "mo" ],
    82 : [ "Mongolian",           "mn" ],
    83 : [ "NauruLanguage",       "na" ],
    84 : [ "Nepali",              "ne" ],
    85 : [ "Norwegian",           "no" ],
    86 : [ "Occitan",             "oc" ],
    87 : [ "Oriya",               "or" ],
    88 : [ "Pashto",              "ps" ],
    89 : [ "Persian",             "fa" ],
    90 : [ "Polish",              "pl" ],
    91 : [ "Portuguese",          "pt" ],
    92 : [ "Punjabi",             "pa" ],
    93 : [ "Quechua",             "qu" ],
    94 : [ "RhaetoRomance",       "rm" ],
    95 : [ "Romanian",            "ro" ],
    96 : [ "Russian",             "ru" ],
    97 : [ "Samoan",              "sm" ],
    98 : [ "Sangho",              "sg" ],
    99 : [ "Sanskrit",            "sa" ],
    100 : [ "Serbian",            "sr" ],
    101 : [ "SerboCroatian",      "sh" ],
    102 : [ "Sesotho",            "st" ],
    103 : [ "Setswana",           "tn" ],
    104 : [ "Shona",              "sn" ],
    105 : [ "Sindhi",             "sd" ],
    106 : [ "Singhalese",         "si" ],
    107 : [ "Siswati",            "ss" ],
    108 : [ "Slovak",             "sk" ],
    109 : [ "Slovenian",          "sl" ],
    110 : [ "Somali",             "so" ],
    111 : [ "Spanish",            "es" ],
    112 : [ "Sundanese",          "su" ],
    113 : [ "Swahili",            "sw" ],
    114 : [ "Swedish",            "sv" ],
    115 : [ "Tagalog",            "tl" ],
    116 : [ "Tajik",              "tg" ],
    117 : [ "Tamil",              "ta" ],
    118 : [ "Tatar",              "tt" ],
    119 : [ "Telugu",             "te" ],
    120 : [ "Thai",               "th" ],
    121 : [ "Tibetan",            "bo" ],
    122 : [ "Tigrinya",           "ti" ],
    123 : [ "TongaLanguage",      "to" ],
    124 : [ "Tsonga",             "ts" ],
    125 : [ "Turkish",            "tr" ],
    126 : [ "Turkmen",            "tk" ],
    127 : [ "Twi",                "tw" ],
    128 : [ "Uigur",              "ug" ],
    129 : [ "Ukrainian",          "uk" ],
    130 : [ "Urdu",               "ur" ],
    131 : [ "Uzbek",              "uz" ],
    132 : [ "Vietnamese",         "vi" ],
    133 : [ "Volapuk",            "vo" ],
    134 : [ "Welsh",              "cy" ],
    135 : [ "Wolof",              "wo" ],
    136 : [ "Xhosa",              "xh" ],
    137 : [ "Yiddish",            "yi" ],
    138 : [ "Yoruba",             "yo" ],
    139 : [ "Zhuang",             "za" ],
    140 : [ "Zulu",               "zu" ],
    141 : [ "Nynorsk",            "nn" ],
    142 : [ "Bosnian",            "bs" ],
    143 : [ "Divehi",             "dv" ],
    144 : [ "Manx",               "gv" ],
    145 : [ "Cornish",            "kw" ]
}

country_list = {
    0 : [ "AnyCountry",                                 "  "  ],
    1 : [ "Afghanistan",                                "AF"  ],
    2 : [ "Albania",                                    "AL"  ],
    3 : [ "Algeria",                                    "DZ"  ],
    4 : [ "AmericanSamoa",                              "AS"  ],
    5 : [ "Andorra",                                    "AD"  ],
    6 : [ "Angola",                                     "AO"  ],
    7 : [ "Anguilla",                                   "AI"  ],
    8 : [ "Antarctica",                                 "AQ"  ],
    9 : [ "AntiguaAndBarbuda",                          "AG"  ],
    10 : [ "Argentina",                                 "AR"  ],
    11 : [ "Armenia",                                   "AM"  ],
    12 : [ "Aruba",                                     "AW"  ],
    13 : [ "Australia",                                 "AU"  ],
    14 : [ "Austria",                                   "AT"  ],
    15 : [ "Azerbaijan",                                "AZ"  ],
    16 : [ "Bahamas",                                   "BS"  ],
    17 : [ "Bahrain",                                   "BH"  ],
    18 : [ "Bangladesh",                                "BD"  ],
    19 : [ "Barbados",                                  "BB"  ],
    20 : [ "Belarus",                                   "BY"  ],
    21 : [ "Belgium",                                   "BE"  ],
    22 : [ "Belize",                                    "BZ"  ],
    23 : [ "Benin",                                     "BJ"  ],
    24 : [ "Bermuda",                                   "BM"  ],
    25 : [ "Bhutan",                                    "BT"  ],
    26 : [ "Bolivia",                                   "BO"  ],
    27 : [ "BosniaAndHerzegowina",                      "BA"  ],
    28 : [ "Botswana",                                  "BW"  ],
    29 : [ "BouvetIsland",                              "BV"  ],
    30 : [ "Brazil",                                    "BR"  ],
    31 : [ "BritishIndianOceanTerritory",               "IO"  ],
    32 : [ "BruneiDarussalam",                          "BN"  ],
    33 : [ "Bulgaria",                                  "BG"  ],
    34 : [ "BurkinaFaso",                               "BF"  ],
    35 : [ "Burundi",                                   "BI"  ],
    36 : [ "Cambodia",                                  "KH"  ],
    37 : [ "Cameroon",                                  "CM"  ],
    38 : [ "Canada",                                    "CA"  ],
    39 : [ "CapeVerde",                                 "CV"  ],
    40 : [ "CaymanIslands",                             "KY"  ],
    41 : [ "CentralAfricanRepublic",                    "CF"  ],
    42 : [ "Chad",                                      "TD"  ],
    43 : [ "Chile",                                     "CL"  ],
    44 : [ "China",                                     "CN"  ],
    45 : [ "ChristmasIsland",                           "CX"  ],
    46 : [ "CocosIslands",                              "CC"  ],
    47 : [ "Colombia",                                  "CO"  ],
    48 : [ "Comoros",                                   "KM"  ],
    49 : [ "DemocraticRepublicOfCongo",                 "CD"  ],
    50 : [ "PeoplesRepublicOfCongo",                    "CG"  ],
    51 : [ "CookIslands",                               "CK"  ],
    52 : [ "CostaRica",                                 "CR"  ],
    53 : [ "IvoryCoast",                                "CI"  ],
    54 : [ "Croatia",                                   "HR"  ],
    55 : [ "Cuba",                                      "CU"  ],
    56 : [ "Cyprus",                                    "CY"  ],
    57 : [ "CzechRepublic",                             "CZ"  ],
    58 : [ "Denmark",                                   "DK"  ],
    59 : [ "Djibouti",                                  "DJ"  ],
    60 : [ "Dominica",                                  "DM"  ],
    61 : [ "DominicanRepublic",                         "DO"  ],
    62 : [ "EastTimor",                                 "TL"  ],
    63 : [ "Ecuador",                                   "EC"  ],
    64 : [ "Egypt",                                     "EG"  ],
    65 : [ "ElSalvador",                                "SV"  ],
    66 : [ "EquatorialGuinea",                          "GQ"  ],
    67 : [ "Eritrea",                                   "ER"  ],
    68 : [ "Estonia",                                   "EE"  ],
    69 : [ "Ethiopia",                                  "ET"  ],
    70 : [ "FalklandIslands",                           "FK"  ],
    71 : [ "FaroeIslands",                              "FO"  ],
    72 : [ "FijiCountry",                               "FJ"  ],
    73 : [ "Finland",                                   "FI"  ],
    74 : [ "France",                                    "FR"  ],
    75 : [ "MetropolitanFrance",                        "FX"  ],
    76 : [ "FrenchGuiana",                              "GF"  ],
    77 : [ "FrenchPolynesia",                           "PF"  ],
    78 : [ "FrenchSouthernTerritories",                 "TF"  ],
    79 : [ "Gabon",                                     "GA"  ],
    80 : [ "Gambia",                                    "GM"  ],
    81 : [ "Georgia",                                   "GE"  ],
    82 : [ "Germany",                                   "DE"  ],
    83 : [ "Ghana",                                     "GH"  ],
    84 : [ "Gibraltar",                                 "GI"  ],
    85 : [ "Greece",                                    "GR"  ],
    86 : [ "Greenland",                                 "GL"  ],
    87 : [ "Grenada",                                   "GD"  ],
    88 : [ "Guadeloupe",                                "GP"  ],
    89 : [ "Guam",                                      "GU"  ],
    90 : [ "Guatemala",                                 "GT"  ],
    91 : [ "Guinea",                                    "GN"  ],
    92 : [ "GuineaBissau",                              "GW"  ],
    93 : [ "Guyana",                                    "GY"  ],
    94 : [ "Haiti",                                     "HT"  ],
    95 : [ "HeardAndMcDonaldIslands",                   "HM"  ],
    96 : [ "Honduras",                                  "HN"  ],
    97 : [ "HongKong",                                  "HK"  ],
    98 : [ "Hungary",                                   "HU"  ],
    99 : [ "Iceland",                                   "IS"  ],
    100 : [ "India",                                    "IN"  ],
    101 : [ "Indonesia",                                "ID"  ],
    102 : [ "Iran",                                     "IR"  ],
    103 : [ "Iraq",                                     "IQ"  ],
    104 : [ "Ireland",                                  "IE"  ],
    105 : [ "Israel",                                   "IL"  ],
    106 : [ "Italy",                                    "IT"  ],
    107 : [ "Jamaica",                                  "JM"  ],
    108 : [ "Japan",                                    "JP"  ],
    109 : [ "Jordan",                                   "JO"  ],
    110 : [ "Kazakhstan",                               "KZ"  ],
    111 : [ "Kenya",                                    "KE"  ],
    112 : [ "Kiribati",                                 "KI"  ],
    113 : [ "DemocraticRepublicOfKorea",                "KP"  ],
    114 : [ "RepublicOfKorea",                          "KR"  ],
    115 : [ "Kuwait",                                   "KW"  ],
    116 : [ "Kyrgyzstan",                               "KG"  ],
    117 : [ "Lao",                                      "LA"  ],
    118 : [ "Latvia",                                   "LV"  ],
    119 : [ "Lebanon",                                  "LB"  ],
    120 : [ "Lesotho",                                  "LS"  ],
    121 : [ "Liberia",                                  "LR"  ],
    122 : [ "LibyanArabJamahiriya",                     "LY"  ],
    123 : [ "Liechtenstein",                            "LI"  ],
    124 : [ "Lithuania",                                "LT"  ],
    125 : [ "Luxembourg",                               "LU"  ],
    126 : [ "Macau",                                    "MO"  ],
    127 : [ "Macedonia",                                "MK"  ],
    128 : [ "Madagascar",                               "MG"  ],
    129 : [ "Malawi",                                   "MW"  ],
    130 : [ "Malaysia",                                 "MY"  ],
    131 : [ "Maldives",                                 "MV"  ],
    132 : [ "Mali",                                     "ML"  ],
    133 : [ "Malta",                                    "MT"  ],
    134 : [ "MarshallIslands",                          "MH"  ],
    135 : [ "Martinique",                               "MQ"  ],
    136 : [ "Mauritania",                               "MR"  ],
    137 : [ "Mauritius",                                "MU"  ],
    138 : [ "Mayotte",                                  "YT"  ],
    139 : [ "Mexico",                                   "MX"  ],
    140 : [ "Micronesia",                               "FM"  ],
    141 : [ "Moldova",                                  "MD"  ],
    142 : [ "Monaco",                                   "MC"  ],
    143 : [ "Mongolia",                                 "MN"  ],
    144 : [ "Montserrat",                               "MS"  ],
    145 : [ "Morocco",                                  "MA"  ],
    146 : [ "Mozambique",                               "MZ"  ],
    147 : [ "Myanmar",                                  "MM"  ],
    148 : [ "Namibia",                                  "NA"  ],
    149 : [ "NauruCountry",                             "NR"  ],
    150 : [ "Nepal",                                    "NP"  ],
    151 : [ "Netherlands",                              "NL"  ],
    152 : [ "NetherlandsAntilles",                      "AN"  ],
    153 : [ "NewCaledonia",                             "NC"  ],
    154 : [ "NewZealand",                               "NZ"  ],
    155 : [ "Nicaragua",                                "NI"  ],
    156 : [ "Niger",                                    "NE"  ],
    157 : [ "Nigeria",                                  "NG"  ],
    158 : [ "Niue",                                     "NU"  ],
    159 : [ "NorfolkIsland",                            "NF"  ],
    160 : [ "NorthernMarianaIslands",                   "MP"  ],
    161 : [ "Norway",                                   "NO"  ],
    162 : [ "Oman",                                     "OM"  ],
    163 : [ "Pakistan",                                 "PK"  ],
    164 : [ "Palau",                                    "PW"  ],
    165 : [ "PalestinianTerritory",                     "PS"  ],
    166 : [ "Panama",                                   "PA"  ],
    167 : [ "PapuaNewGuinea",                           "PG"  ],
    168 : [ "Paraguay",                                 "PY"  ],
    169 : [ "Peru",                                     "PE"  ],
    170 : [ "Philippines",                              "PH"  ],
    171 : [ "Pitcairn",                                 "PN"  ],
    172 : [ "Poland",                                   "PL"  ],
    173 : [ "Portugal",                                 "PT"  ],
    174 : [ "PuertoRico",                               "PR"  ],
    175 : [ "Qatar",                                    "QA"  ],
    176 : [ "Reunion",                                  "RE"  ],
    177 : [ "Romania",                                  "RO"  ],
    178 : [ "RussianFederation",                        "RU"  ],
    179 : [ "Rwanda",                                   "RW"  ],
    180 : [ "SaintKittsAndNevis",                       "KN"  ],
    181 : [ "StLucia",                                  "LC"  ],
    182 : [ "StVincentAndTheGrenadines",                "VC"  ],
    183 : [ "Samoa",                                    "WS"  ],
    184 : [ "SanMarino",                                "SM"  ],
    185 : [ "SaoTomeAndPrincipe",                       "ST"  ],
    186 : [ "SaudiArabia",                              "SA"  ],
    187 : [ "Senegal",                                  "SN"  ],
    188 : [ "Seychelles",                               "SC"  ],
    189 : [ "SierraLeone",                              "SL"  ],
    190 : [ "Singapore",                                "SG"  ],
    191 : [ "Slovakia",                                 "SK"  ],
    192 : [ "Slovenia",                                 "SI"  ],
    193 : [ "SolomonIslands",                           "SB"  ],
    194 : [ "Somalia",                                  "SO"  ],
    195 : [ "SouthAfrica",                              "ZA"  ],
    196 : [ "SouthGeorgiaAndTheSouthSandwichIslands",   "GS"  ],
    197 : [ "Spain",                                    "ES"  ],
    198 : [ "SriLanka",                                 "LK"  ],
    199 : [ "StHelena",                                 "SH"  ],
    200 : [ "StPierreAndMiquelon",                      "PM"  ],
    201 : [ "Sudan",                                    "SD"  ],
    202 : [ "Suriname",                                 "SR"  ],
    203 : [ "SvalbardAndJanMayenIslands",               "SJ"  ],
    204 : [ "Swaziland",                                "SZ"  ],
    205 : [ "Sweden",                                   "SE"  ],
    206 : [ "Switzerland",                              "CH"  ],
    207 : [ "SyrianArabRepublic",                       "SY"  ],
    208 : [ "Taiwan",                                   "TW"  ],
    209 : [ "Tajikistan",                               "TJ"  ],
    210 : [ "Tanzania",                                 "TZ"  ],
    211 : [ "Thailand",                                 "TH"  ],
    212 : [ "Togo",                                     "TG"  ],
    213 : [ "Tokelau",                                  "TK"  ],
    214 : [ "TongaCountry",                             "TO"  ],
    215 : [ "TrinidadAndTobago",                        "TT"  ],
    216 : [ "Tunisia",                                  "TN"  ],
    217 : [ "Turkey",                                   "TR"  ],
    218 : [ "Turkmenistan",                             "TM"  ],
    219 : [ "TurksAndCaicosIslands",                    "TC"  ],
    220 : [ "Tuvalu",                                   "TV"  ],
    221 : [ "Uganda",                                   "UG"  ],
    222 : [ "Ukraine",                                  "UA"  ],
    223 : [ "UnitedArabEmirates",                       "AE"  ],
    224 : [ "UnitedKingdom",                            "GB"  ],
    225 : [ "UnitedStates",                             "US"  ],
    226 : [ "UnitedStatesMinorOutlyingIslands",         "UM"  ],
    227 : [ "Uruguay",                                  "UY"  ],
    228 : [ "Uzbekistan",                               "UZ"  ],
    229 : [ "Vanuatu",                                  "VU"  ],
    230 : [ "VaticanCityState",                         "VA"  ],
    231 : [ "Venezuela",                                "VE"  ],
    232 : [ "VietNam",                                  "VN"  ],
    233 : [ "BritishVirginIslands",                     "VG"  ],
    234 : [ "USVirginIslands",                          "VI"  ],
    235 : [ "WallisAndFutunaIslands",                   "WF"  ],
    236 : [ "WesternSahara",                            "EH"  ],
    237 : [ "Yemen",                                    "YE"  ],
    238 : [ "Yugoslavia",                               "YU"  ],
    239 : [ "Zambia",                                   "ZM"  ],
    240 : [ "Zimbabwe",                                 "ZW"  ],
    241 : [ "SerbiaAndMontenegro",                      "CS"  ]
}

current_keys = [
    [      1,     0    ],
    [      5,   195    ],
    [      6,     2    ],
    [      8,   186    ],
    [      8,     3    ],
    [      8,    17    ],
    [      8,    64    ],
    [      8,   103    ],
    [      8,   109    ],
    [      8,   115    ],
    [      8,   119    ],
    [      8,   122    ],
    [      8,   145    ],
    [      8,   162    ],
    [      8,   175    ],
    [      8,   207    ],
    [      8,   216    ],
    [      8,   223    ],
    [      8,   237    ],
    [      9,    11    ],
    [     12,    15    ],
    [     14,   197    ],
    [     20,    33    ],
    [     22,    20    ],
    [     24,   197    ],
    [     25,    44    ],
    [     25,    97    ],
    [     25,   126    ],
    [     25,   190    ],
    [     25,   208    ],
    [     27,    54    ],
    [     28,    57    ],
    [     29,    58    ],
    [     30,   151    ],
    [     30,    21    ],
    [     31,   225    ],
    [     31,    13    ],
    [     31,    22    ],
    [     31,    38    ],
    [     31,   104    ],
    [     31,   107    ],
    [     31,   154    ],
    [     31,   170    ],
    [     31,   195    ],
    [     31,   215    ],
    [     31,   224    ],
    [     31,   240    ],
    [     33,    68    ],
    [     34,    71    ],
    [     36,    73    ],
    [     37,    74    ],
    [     37,    21    ],
    [     37,    38    ],
    [     37,   125    ],
    [     37,   142    ],
    [     37,   206    ],
    [     40,   197    ],
    [     41,    81    ],
    [     42,    82    ],
    [     42,    14    ],
    [     42,   123    ],
    [     42,   125    ],
    [     42,   206    ],
    [     43,    85    ],
    [     46,   100    ],
    [     48,   105    ],
    [     49,   100    ],
    [     50,    98    ],
    [     51,    99    ],
    [     52,   101    ],
    [     58,   106    ],
    [     58,   206    ],
    [     59,   108    ],
    [     61,   100    ],
    [     63,   110    ],
    [     65,   116    ],
    [     66,   114    ],
    [     71,   118    ],
    [     73,   124    ],
    [     74,   127    ],
    [     76,   130    ],
    [     76,    32    ],
    [     80,   100    ],
    [     82,   143    ],
    [     85,   161    ],
    [     89,   102    ],
    [     90,   172    ],
    [     91,   173    ],
    [     91,    30    ],
    [     92,   100    ],
    [     95,   177    ],
    [     96,   178    ],
    [     99,   100    ],
    [    108,   191    ],
    [    109,   192    ],
    [    111,   197    ],
    [    111,    10    ],
    [    111,    26    ],
    [    111,    43    ],
    [    111,    47    ],
    [    111,    52    ],
    [    111,    61    ],
    [    111,    63    ],
    [    111,    65    ],
    [    111,    90    ],
    [    111,    96    ],
    [    111,   139    ],
    [    111,   155    ],
    [    111,   166    ],
    [    111,   168    ],
    [    111,   169    ],
    [    111,   174    ],
    [    111,   227    ],
    [    111,   231    ],
    [    113,   111    ],
    [    114,   205    ],
    [    114,    73    ],
    [    117,   100    ],
    [    119,   100    ],
    [    120,   211    ],
    [    125,   217    ],
    [    129,   222    ],
    [    130,   163    ],
    [    131,   228    ],
    [    132,   232    ],
    [    141,   161    ]
]

file_list = [
    "aa_DJ",
    "aa_ER",
    "aa_ET",
    "af_ZA",
    "am_ET",
    "ar_AE",
    "ar_BH",
    "ar_DZ",
    "ar_EG",
    "ar_IQ",
    "ar_JO",
    "ar_KW",
    "ar_LB",
    "ar_LY",
    "ar_MA",
    "ar_OM",
    "ar_QA",
    "ar_SA",
    "ar_SD",
    "ar_SY",
    "ar_TN",
    "ar_YE",
    "as_IN",
    "az_AZ",
    "be_BY",
    "bg_BG",
    "bn_IN",
    "bs_BA",
    "ca_ES",
    "cs_CZ",
    "cy_GB",
    "da_DK",
    "de_AT",
    "de_BE",
    "de_CH",
    "de_DE",
    "de_LI",
    "de_LU",
    "dv_MV",
    "dz_BT",
    "el_CY",
    "el_GR",
    "en_AS",
    "en_AU",
    "en_BE",
    "en_BW",
    "en_BZ",
    "en_CA",
    "en_GB",
    "en_GU",
    "en_HK",
    "en_IE",
    "en_IN",
    "en_JM",
    "en_MH",
    "en_MP",
    "en_MT",
    "en_NZ",
    "en_PH",
    "en_PK",
    "en_SG",
    "en_TT",
    "en_UM",
    "en_US",
    "en_VI",
    "en_ZA",
    "en_ZW",
    "es_AR",
    "es_BO",
    "es_CL",
    "es_CO",
    "es_CR",
    "es_DO",
    "es_EC",
    "es_ES",
    "es_GT",
    "es_HN",
    "es_MX",
    "es_NI",
    "es_PA",
    "es_PE",
    "es_PR",
    "es_PY",
    "es_SV",
    "es_US",
    "es_UY",
    "es_VE",
    "et_EE",
    "eu_ES",
    "fa_AF",
    "fa_IR",
    "fi_FI",
    "fo_FO",
    "fr_BE",
    "fr_CA",
    "fr_CH",
    "fr_FR",
    "fr_LU",
    "fr_MC",
    "ga_IE",
    "gl_ES",
    "gu_IN",
    "gv_GB",
    "he_IL",
    "hi_IN",
    "hr_HR",
    "hu_HU",
    "hy_AM",
    "id_ID",
    "is_IS",
    "it_CH",
    "it_IT",
    "ja_JP",
    "ka_GE",
    "kk_KZ",
    "kl_GL",
    "km_KH",
    "kn_IN",
    "ko_KR",
    "kw_GB",
    "ky_KG",
    "lo_LA",
    "lt_LT",
    "lv_LV",
    "mk_MK",
    "ml_IN",
    "mn_MN",
    "mr_IN",
    "ms_BN",
    "ms_MY",
    "mt_MT",
    "nl_BE",
    "nl_NL",
    "nn_NO",
    "no_NO",
    "om_ET",
    "om_KE",
    "or_IN",
    "pa_IN",
    "pl_PL",
    "ps_AF",
    "pt_BR",
    "pt_PT",
    "ro_RO",
    "ru_RU",
    "ru_UA",
    "sa_IN",
    "sh_BA",
    "sh_CS",
    "sh_YU",
    "sk_SK",
    "sl_SI",
    "so_DJ",
    "so_ET",
    "so_KE",
    "so_SO",
    "sq_AL",
    "sr_BA",
    "sr_CS",
    "sr_YU",
    "sv_FI",
    "sv_SE",
    "sw_KE",
    "sw_TZ",
    "ta_IN",
    "te_IN",
    "th_TH",
    "ti_ER",
    "ti_ET",
    "tr_TR",
    "tt_RU",
    "uk_UA",
    "ur_PK",
    "uz_AF",
    "uz_UZ",
    "vi_VN",
    "zh_CN",
    "zh_HK",
    "zh_MO",
    "zh_SG",
    "zh_TW"
]

def countryCodeToId(code):
    for country_id in country_list:
        if country_list[country_id][1] == code:
            return country_id
    return -1

def languageCodeToId(code):
    for language_id in language_list:
        if language_list[language_id][1] == code:
            return language_id
    return -1

doc_cache = {}
xml_dir = "/home/jsurazsk/src/icu/common/main"

def findChild(parent, tag_name, arg_value):
    for node in parent.childNodes:
        if node.nodeType != node.ELEMENT_NODE:
            continue
        if node.nodeName != tag_name:
            continue
        if arg_value:
            if not node.attributes.has_key('type'):
                continue
            if node.attributes['type'].nodeValue != arg_value:
                continue
        return node
    return False

def _findEntry(base, path):
    doc = False
    if doc_cache.has_key(base):
        doc = doc_cache[base]
    else:
        file = xml_dir + "/" + base + ".xml"
        doc = xml.dom.minidom.parse(file)
        doc_cache[base] = doc

    elt = doc.documentElement
    tag_spec_list = path.split("/")
    for tag_spec in tag_spec_list:
        tag_name = tag_spec
        arg_value = ''
        left_bracket = tag_spec.find('[')
        if left_bracket != -1:
            tag_name = tag_spec[:left_bracket]
            arg_value = tag_spec[left_bracket+1:-1]
        elt = findChild(elt, tag_name, arg_value)
        if not elt:
            return ""

    return elt.firstChild.nodeValue

def findEntry(base, path):
    result = _findEntry(base, path)
    if result:
        return result
    base = base[:2]
    result = _findEntry(base, path)
    if result:
        return result
    base = "root"
    result = _findEntry(base, path)
    return result

def ordStr(c):
    if len(c) == 1:
        return str(ord(c))
    return "##########"

def generateLocaleInfo(language_id, country_id):
    language = language_list[language_id][0]
    language_code = language_list[language_id][1].strip()
    country = country_list[country_id][0]
    country_code = country_list[country_id][1].strip()

    if not language_code or not country_code:
        return {}

    base = language_code + "_" + country_code

    result = {}
    result['base'] = base

    result['language'] = language
    result['country'] = country
    result['decimal'] = findEntry(base, "numbers/symbols/decimal")
    result['group'] = findEntry(base, "numbers/symbols/group")
    result['list'] = findEntry(base, "numbers/symbols/list")
    result['percent'] = findEntry(base, "numbers/symbols/percentSign")
    result['zero'] = findEntry(base, "numbers/symbols/nativeZeroDigit")
    result['minus'] = findEntry(base, "numbers/symbols/minusSign")
    result['exp'] = findEntry(base, "numbers/symbols/exponential").lower()
    result['longDateFormat'] = findEntry(base, "dates/calendars/calendar[gregorian]/dateFormats/dateFormatLength[full]/dateFormat/pattern")
    result['shortDateFormat'] = findEntry(base, "dates/calendars/calendar[gregorian]/dateFormats/dateFormatLength[medium]/dateFormat/pattern")
    result['longTimeFormat'] = findEntry(base, "dates/calendars/calendar[gregorian]/timeFormats/timeFormatLength[full]/timeFormat/pattern")
    result['shortTimeFormat'] = findEntry(base, "dates/calendars/calendar[gregorian]/timeFormats/timeFormatLength[medium]/timeFormat/pattern")

    long_month_path = "dates/calendars/calendar[gregorian]/months/monthContext[format]/monthWidth[wide]/month"
    result['longMonths'] \
        = findEntry(base, long_month_path + "[1]") + ";" \
        + findEntry(base, long_month_path + "[2]") + ";" \
        + findEntry(base, long_month_path + "[3]") + ";" \
        + findEntry(base, long_month_path + "[4]") + ";" \
        + findEntry(base, long_month_path + "[5]") + ";" \
        + findEntry(base, long_month_path + "[6]") + ";" \
        + findEntry(base, long_month_path + "[7]") + ";" \
        + findEntry(base, long_month_path + "[8]") + ";" \
        + findEntry(base, long_month_path + "[9]") + ";" \
        + findEntry(base, long_month_path + "[10]") + ";" \
        + findEntry(base, long_month_path + "[11]") + ";" \
        + findEntry(base, long_month_path + "[12]") + ";"

    short_month_path = "dates/calendars/calendar[gregorian]/months/monthContext[format]/monthWidth[abbreviated]/month"
    result['shortMonths'] \
        = findEntry(base, short_month_path + "[1]") + ";" \
        + findEntry(base, short_month_path + "[2]") + ";" \
        + findEntry(base, short_month_path + "[3]") + ";" \
        + findEntry(base, short_month_path + "[4]") + ";" \
        + findEntry(base, short_month_path + "[5]") + ";" \
        + findEntry(base, short_month_path + "[6]") + ";" \
        + findEntry(base, short_month_path + "[7]") + ";" \
        + findEntry(base, short_month_path + "[8]") + ";" \
        + findEntry(base, short_month_path + "[9]") + ";" \
        + findEntry(base, short_month_path + "[10]") + ";" \
        + findEntry(base, short_month_path + "[11]") + ";" \
        + findEntry(base, short_month_path + "[12]") + ";"

    long_day_path = "dates/calendars/calendar[gregorian]/days/dayContext[format]/dayWidth[wide]/day"
    result['longDays'] \
        = findEntry(base, long_day_path + "[sun]") + ";" \
        + findEntry(base, long_day_path + "[mon]") + ";" \
        + findEntry(base, long_day_path + "[tue]") + ";" \
        + findEntry(base, long_day_path + "[wed]") + ";" \
        + findEntry(base, long_day_path + "[thu]") + ";" \
        + findEntry(base, long_day_path + "[fri]") + ";" \
        + findEntry(base, long_day_path + "[sat]") + ";"

    short_day_path = "dates/calendars/calendar[gregorian]/days/dayContext[format]/dayWidth[abbreviated]/day"
    result['shortDays'] \
        = findEntry(base, short_day_path + "[sun]") + ";" \
        + findEntry(base, short_day_path + "[mon]") + ";" \
        + findEntry(base, short_day_path + "[tue]") + ";" \
        + findEntry(base, short_day_path + "[wed]") + ";" \
        + findEntry(base, short_day_path + "[thu]") + ";" \
        + findEntry(base, short_day_path + "[fri]") + ";" \
        + findEntry(base, short_day_path + "[sat]") + ";"

    return result

locale_database = {}
for file in file_list:
    language_code = file[:2]
    country_code = file[3:]

    language_id = languageCodeToId(language_code)
    country_id = countryCodeToId(country_code)

    if language_id == -1 or country_id == -1:
        print "################ bad id:" + file
        continue

    l = generateLocaleInfo(language_id, country_id)
    if not l:
        print "################ failed to generate:" + file
        continue

    locale_database[(language_id, country_id)] = l

def addEscapes(s):
    result = ''
    for c in s:
        n = ord(c)
        if n < 128:
            result += c
        else:
            result += "\\x"
            result += "%02x" % (n)
    return result

def unicodeStr(s):
    utf8 = s.encode('utf-8')
    return "<size>" + str(len(utf8)) + "</size><data>" + addEscapes(utf8) + "</data>"

locale_keys = locale_database.keys()
locale_keys.sort()

for key in locale_keys:
    l = locale_database[key]

    print "        <locale>"
    print "            <language>" + l['language']        + "</language>"
    print "            <country>"  + l['country']         + "</country>"
    print "            <decimal>"  + ordStr(l['decimal']) + "</decimal>"
    print "            <group>"    + ordStr(l['group'])   + "</group>"
    print "            <list>"     + ordStr(l['list'])    + "</list>"
    print "            <percent>"  + ordStr(l['percent']) + "</percent>"
    print "            <zero>"     + ordStr(l['zero'])    + "</zero>"
    print "            <minus>"    + ordStr(l['minus'])   + "</minus>"
    print "            <exp>"      + ordStr(l['exp'])     + "</exp>"
    print "            <longDateFormat>"  + l['longDateFormat'].encode('utf-8')  + "</longDateFormat>"
    print "            <shortDateFormat>" + l['shortDateFormat'].encode('utf-8') + "</shortDateFormat>"
    print "            <longTimeFormat>"  + l['longTimeFormat'].encode('utf-8')  + "</longTimeFormat>"
    print "            <shortTimeFormat>" + l['shortTimeFormat'].encode('utf-8') + "</shortTimeFormat>"
    print "            <longMonths>"      + l['longMonths'].encode('utf-8')      + "</longMonths>"
    print "            <shortMonths>"     + l['shortMonths'].encode('utf-8')     + "</shortMonths>"
    print "            <longDays>"        + l['longDays'].encode('utf-8')        + "</longDays>"
    print "            <shortDays>"       + l['shortDays'].encode('utf-8')       + "</shortDays>"
    print "        </locale>"

def show_locales():
    for file in file_list:
        language_code = file[:2]
        country_code = file[3:]

        language_id = languageCodeToId(language_code)
        country_id = countryCodeToId(country_code)
        language_name = "########"
        country_name = "########"
        if language_id != -1:
            language_name = language_list[language_id][0]
        if country_id != -1:
            country_name = country_list[country_id][0]

        print file + ": " + str(language_id) + "/" + str(country_id) + " " + language_name + "/" + country_name
