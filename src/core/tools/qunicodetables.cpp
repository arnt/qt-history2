/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "qunicodetables_p.h"
#include "qunicodedata.cpp"

#define GET_PROP(ucs4) (uc_properties + GET_PROP_INDEX(ucs4))

QChar::Category QUnicodeTables::category(uint ucs4)
{
    return (QChar::Category) GET_PROP(ucs4)->category;
}

unsigned char QUnicodeTables::combiningClass(uint ucs4)
{
    return (unsigned char) GET_PROP(ucs4)->combiningClass;
}

QChar::Direction QUnicodeTables::direction(uint ucs4)
{
    return (QChar::Direction) GET_PROP(ucs4)->direction;
}

QChar::Joining QUnicodeTables::joining(uint ucs4)
{
    return (QChar::Joining) GET_PROP(ucs4)->joining;
}

QChar::UnicodeVersion QUnicodeTables::unicodeVersion(uint ucs4)
{
    return (QChar::UnicodeVersion) GET_PROP(ucs4)->unicode_version;
}

int QUnicodeTables::digitValue(uint ucs4)
{
    int val = GET_PROP(ucs4)->digit_value;
    return val == 0xf ? -1 : val;
}

bool QUnicodeTables::mirrored(uint ucs4)
{
    return GET_PROP(ucs4)->mirrorDiff != 0;
}

int QUnicodeTables::mirroredChar(uint ucs4)
{
    return ucs4 + GET_PROP(ucs4)->mirrorDiff;
}

QUnicodeTables::LineBreakClass QUnicodeTables::lineBreakClass(uint ucs4)
{
    return (QUnicodeTables::LineBreakClass) GET_PROP(ucs4)->line_break_class;
}

int QUnicodeTables::upper(uint ucs4)
{
    const UC_Properties *p = GET_PROP(ucs4);
    if (p->category == QChar::Letter_Lowercase)
        return ucs4 + p->caseDiff;
    return ucs4;
}

int QUnicodeTables::lower(uint ucs4)
{
    const UC_Properties *p = GET_PROP(ucs4);
    if (p->category == QChar::Letter_Uppercase || p->category == QChar::Letter_Titlecase)
        return ucs4 + p->caseDiff;
    return ucs4;
}

QString QUnicodeTables::decomposition(uint ucs4)
{
    const unsigned short index = GET_DECOMPOSITION_INDEX(ucs4);
    if (index == 0xffff)
        return QString();
    const unsigned short *decomposition = uc_decomposition_map+index;
    uint length = (*decomposition) >> 8;
    QString str;
    str.resize(length);
    QChar *c = str.data();
    for (uint i = 0; i < length; ++i)
        *(c++) = *(++decomposition);
    return str;
}

QChar::Decomposition QUnicodeTables::decompositionTag(uint ucs4)
{
    const unsigned short index = GET_DECOMPOSITION_INDEX(ucs4);
    if (index == 0xffff)
        return QChar::NoDecomposition;
    return (QChar::Decomposition)(uc_decomposition_map[index] & 0xff);
}

ushort QUnicodeTables::ligature(ushort u1, ushort u2)
{
    const unsigned short index = GET_LIGATURE_INDEX(u2);
    if (index == 0xffff)
        return 0;
    const unsigned short *ligatures = uc_ligature_map+index;
    ushort length = *ligatures;
    ++ligatures;
    // ### use bsearch
    for (uint i = 0; i < length; ++i)
        if (ligatures[2*i] == u1)
            return ligatures[2*i+1];
    return 0;
}

static QString decompose(const QString &str, bool canonical)
{
    QString s = str;

    const unsigned short *utf16 = s.utf16();
    const unsigned short *uc = utf16 + s.length();
    while (uc != utf16) {
        uint ucs4 = *(--uc);
        if (ucs4 >= 0xd800 && ucs4 < 0xdc00 && uc != utf16) {
            ushort low = *(uc - 1);
            if (low >= 0xdc00 && low < 0xe000) {
                --uc;
                ucs4 = (ucs4 - 0xd800)*0x400 + (low - 0xdc00) + 0x10000;
            }
        }
        const unsigned short index = GET_DECOMPOSITION_INDEX(ucs4);
        if (index == 0xffff)
            continue;
        const unsigned short *decomposition = uc_decomposition_map+index;
        if (canonical && ((*decomposition) & 0xff) != QChar::Canonical)
            continue;

        uint length = (*decomposition) >> 8;
        s.replace(uc - utf16, ucs4 > 0x10000 ? 2 : 1, (const QChar *)(decomposition+1), length);
        // since the insert invalidates the pointers and we do decomposition recursive
        int pos = uc - utf16;
        utf16 = s.utf16();
        uc = utf16 + pos + length;
    }

    return s;
}

static QString compose(const QString &str)
{
    QString s = str;

    // the loop can ignore high Unicode as all ligatures are in the BMP
    const unsigned short *uc = s.utf16();
    const unsigned short *end = uc + s.length();
    ushort last = *(uc++);
    while (uc != end) {
        ushort utf16 = *uc;
        QChar ligature = QUnicodeTables::ligature(last, utf16);
        if (ligature.unicode()) {
            int pos = uc - s.utf16() - 1;
            s.replace(pos, 2, &ligature, 1);
            // since the insert invalidates the pointers and we do decomposition recursive
            uc = s.utf16() + pos;
            end = s.utf16() + s.length();
            utf16 = ligature.unicode();
        }
        last = utf16;
        uc++;
    }
    return s;
}


static QString canonicalOrder(const QString &str)
{
    // #####
    return str;
}

QString QUnicodeTables::normalize(const QString &str, QUnicodeTables::NormalizationMode mode)
{
    // first decomposition
    QString s = decompose(str, mode < QUnicodeTables::NormalizationMode_KD);

    // now canonical ordering
    s = canonicalOrder(s);

    if (mode == QUnicodeTables::NormalizationMode_D || mode == QUnicodeTables::NormalizationMode_KD)
        return s;

    // last composition
    return compose(s);
}

enum Script {
    // European Alphabetic Scripts
    Latin,
    Greek,
    Cyrillic,
    Armenian,
    Georgian,
    Runic,
    Ogham,
    SpacingModifiers,
    CombiningMarks,

    // Middle Eastern Scripts
    Hebrew,
    Arabic,
    Syriac,
    Thaana,

    // South and Southeast Asian Scripts
    Devanagari,
    Bengali,
    Gurmukhi,
    Gujarati,
    Oriya,
    Tamil,
    Telugu,
    Kannada,
    Malayalam,
    Sinhala,
    Thai,
    Lao,
    Tibetan,
    Myanmar,
    Khmer,

    // East Asian Scripts
    Han,
    Hiragana,
    Katakana,
    Hangul,
    Bopomofo,
    Yi,

    // Additional Scripts
    Ethiopic,
    Cherokee,
    CanadianAboriginal,
    Mongolian,

    // Symbols
    CurrencySymbols,
    LetterlikeSymbols,
    NumberForms,
    MathematicalOperators,
    TechnicalSymbols,
    GeometricSymbols,
    MiscellaneousSymbols,
    EnclosedAndSquare,
    Braille,

    Unicode,

    // some scripts added in Unicode 3.2
    Tagalog,
    Hanunoo,
    Buhid,
    Tagbanwa,

    KatakanaHalfWidth,                // from JIS X 0201

    // from Unicode 4.0
    Limbu,
    TaiLe,

    // End
    NScripts,
    UnknownScript = NScripts
};

enum { SCRIPTS_INDIC = 0x7e };

// copied form qfont.h, as we can't include it in tools. Do not modify without
// changing the script enum in qfont.h aswell.
const unsigned char qt_otherScripts [128] = {
#define SCRIPTS_02 0
    0xaf, Latin, 0xff, SpacingModifiers,                         // row 0x02, index 0
#define SCRIPTS_03 4
    0x6f, CombiningMarks, 0xff, Greek,                         // row 0x03, index 4
#define SCRIPTS_05 8
    0x2f, Cyrillic, 0x8f, Armenian, 0xff, Hebrew,        // row 0x05, index 8
#define SCRIPTS_07 14
    0x4f, Syriac, 0x7f, Unicode, 0xbf, Thaana,
    0xff, Unicode,                                                 // row 0x07, index 14
#define SCRIPTS_10 22
    0x9f, Myanmar, 0xff, Georgian,                        // row 0x10, index 20
#define SCRIPTS_13 26
    0x7f, Ethiopic, 0x9f, Unicode, 0xff, Cherokee,        // row 0x13, index 24
#define SCRIPTS_16 32
    0x7f, CanadianAboriginal, 0x9f, Ogham,
    0xff, Runic,                                                 // row 0x16 index 30
#define SCRIPTS_17 38
    0x1f, Tagalog, 0x3f, Hanunoo, 0x5f, Buhid,
    0x7f, Tagbanwa, 0xff, Khmer,                                // row 0x17, index 36
#define SCRIPTS_18 48
    0xaf, Mongolian, 0xff, Unicode,                        // row 0x18, index 46
#define SCRIPTS_19 52
    0x4f, Limbu, 0x7f, TaiLe, 0xdf, Unicode, 0xff, Khmer,
#define SCRIPTS_20 60
    0x0b, Unicode, 0x0d, UnknownScript, 0x6f, Unicode, 0x9f, NumberForms,
    0xab, CurrencySymbols, 0xac, Latin,
    0xcf, CurrencySymbols, 0xff, CombiningMarks,                // row 0x20, index 50
#define SCRIPTS_21 76
    0x4f, LetterlikeSymbols, 0x8f, NumberForms,
    0xff, MathematicalOperators,                                        // row 0x21, index 62
#define SCRIPTS_24 82
    0x5f, TechnicalSymbols, 0xff, EnclosedAndSquare,        // row 0x24, index 68
#define SCRIPTS_2e 86
    0x7f, Unicode, 0xff, Han,                                // row 0x2e, index 72
#define SCRIPTS_30 90
    0x3f, Han, 0x9f, Hiragana, 0xff, Katakana,        // row 0x30, index 76
#define SCRIPTS_31 96
    0x2f, Bopomofo, 0x8f, Hangul, 0x9f, Han,
    0xff, Unicode,                                                // row 0x31, index 82
#define SCRIPTS_fb 104
    0x06, Latin, 0x1c, Unicode, 0x4f, Hebrew,
    0xff, Arabic,                                                // row 0xfb, index 90
#define SCRIPTS_fe 112
    0x1f, Unicode, 0x2f, CombiningMarks, 0x6f, Unicode,
    0xff, Arabic,                                                // row 0xfe, index 98
#define SCRIPTS_ff 120
    0x5e, Katakana, 0x60, Unicode,                        // row 0xff, index 106
    0x9f, KatakanaHalfWidth, 0xff, Unicode
};

// (uc-0x0900)>>7
const unsigned char qt_indicScripts [] =
{
    Devanagari, Bengali,
    Gurmukhi, Gujarati,
    Oriya, Tamil,
    Telugu, Kannada,
    Malayalam, Sinhala,
    Thai, Lao
};


// 0x80 + x: x is the offset into the otherScripts table
const unsigned char qt_scriptTable[256] =
{
    Latin, Latin, 0x80+SCRIPTS_02, 0x80+SCRIPTS_03,
    Cyrillic, 0x80+SCRIPTS_05, Arabic, 0x80+SCRIPTS_07,
    Unicode, SCRIPTS_INDIC, SCRIPTS_INDIC, SCRIPTS_INDIC,
    SCRIPTS_INDIC, SCRIPTS_INDIC, SCRIPTS_INDIC, Tibetan,

    0x80+SCRIPTS_10, Hangul, Ethiopic, 0x80+SCRIPTS_13,
    CanadianAboriginal, CanadianAboriginal, 0x80+SCRIPTS_16, 0x80+SCRIPTS_17,
    0x80+SCRIPTS_18, 0x80+SCRIPTS_19, Unicode, Unicode,
    Unicode, Unicode, Latin, Greek,

    0x80+SCRIPTS_20, 0x80+SCRIPTS_21, MathematicalOperators, TechnicalSymbols,
    0x80+SCRIPTS_24, GeometricSymbols, MiscellaneousSymbols, MiscellaneousSymbols,
    Braille, Unicode, Unicode, Unicode,
    Unicode, Unicode, 0x80+SCRIPTS_2e, Han,

    0x80+SCRIPTS_30, 0x80+SCRIPTS_31, EnclosedAndSquare, EnclosedAndSquare,
    Han, Han, Han, Han,
    Han, Han, Han, Han,
    Han, Han, Han, Han,

    Han, Han, Han, Han, Han, Han, Han, Han,
    Han, Han, Han, Han, Han, Han, Han, Han,

    Han, Han, Han, Han, Han, Han, Han, Han,
    Han, Han, Han, Han, Han, Han, Han, Han,

    Han, Han, Han, Han, Han, Han, Han, Han,
    Han, Han, Han, Han, Han, Han, Han, Han,

    Han, Han, Han, Han, Han, Han, Han, Han,
    Han, Han, Han, Han, Han, Han, Han, Han,


    Han, Han, Han, Han, Han, Han, Han, Han,
    Han, Han, Han, Han, Han, Han, Han, Han,

    Han, Han, Han, Han, Han, Han, Han, Han,
    Han, Han, Han, Han, Han, Han, Han, Han,

    Yi, Yi, Yi, Yi, Yi, Unicode, Unicode, Unicode,
    Unicode, Unicode, Unicode, Unicode, Hangul, Hangul, Hangul, Hangul,

    Hangul, Hangul, Hangul, Hangul, Hangul, Hangul, Hangul, Hangul,
    Hangul, Hangul, Hangul, Hangul, Hangul, Hangul, Hangul, Hangul,

    Hangul, Hangul, Hangul, Hangul, Hangul, Hangul, Hangul, Hangul,
    Hangul, Hangul, Hangul, Hangul, Hangul, Hangul, Hangul, Hangul,

    Hangul, Hangul, Hangul, Hangul, Hangul, Hangul, Hangul, Hangul,
    Unicode, Unicode, Unicode, Unicode, Unicode, Unicode, Unicode, Unicode,

    Unicode, Unicode, Unicode, Unicode, Unicode, Unicode, Unicode, Unicode,
    Unicode, Unicode, Unicode, Unicode, Unicode, Unicode, Unicode, Unicode,

    Unicode, Unicode, Unicode, Unicode, Unicode, Unicode, Unicode, Unicode,
    Unicode, Han, Han, 0x80+SCRIPTS_fb, Arabic, Arabic, 0x80+SCRIPTS_fe, 0x80+SCRIPTS_ff
};

int scriptForChar(ushort uc)
{
    unsigned char script = qt_scriptTable[(uc>>8)];
    if (script >= SCRIPTS_INDIC) {
        if (script == SCRIPTS_INDIC) {
            script = qt_indicScripts[(uc-0x0900)>>7];
        } else {
            // 0x80 + SCRIPTS_xx
            unsigned char index = script-0x80;
            unsigned char cell = uc &0xff;
            while(qt_otherScripts[index++] < cell)
                index++;
            script = qt_otherScripts[index];
        }
    }
    return script;
}
