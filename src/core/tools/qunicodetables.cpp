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

#include "qunicodetables_p.h"
#include "qunicodedata.cpp"

#define CURRENT_VERSION QChar::Unicode_4_0

static inline const UC_Properties *qGetProp(uint ucs4)
{
    int index = GET_PROP_INDEX(ucs4);
    return uc_properties + index;
}

Q_CORE_EXPORT QChar::Category QUnicodeTables::category(uint ucs4)
{
    return (QChar::Category) qGetProp(ucs4)->category;
}

Q_CORE_EXPORT unsigned char QUnicodeTables::combiningClass(uint ucs4)
{
    return (unsigned char) qGetProp(ucs4)->combiningClass;
}

Q_CORE_EXPORT QChar::Direction QUnicodeTables::direction(uint ucs4)
{
    return (QChar::Direction) qGetProp(ucs4)->direction;
}

Q_CORE_EXPORT QChar::Joining QUnicodeTables::joining(uint ucs4)
{
    return (QChar::Joining) qGetProp(ucs4)->joining;
}

Q_CORE_EXPORT QChar::UnicodeVersion QUnicodeTables::unicodeVersion(uint ucs4)
{
    return (QChar::UnicodeVersion) qGetProp(ucs4)->unicode_version;
}

Q_CORE_EXPORT int QUnicodeTables::digitValue(uint ucs4)
{
    int val = qGetProp(ucs4)->digit_value;
    return val == 0xf ? -1 : val;
}

Q_CORE_EXPORT bool QUnicodeTables::mirrored(uint ucs4)
{
    return qGetProp(ucs4)->mirrorDiff != 0;
}

Q_CORE_EXPORT int QUnicodeTables::mirroredChar(uint ucs4)
{
    return ucs4 + qGetProp(ucs4)->mirrorDiff;
}

Q_CORE_EXPORT QUnicodeTables::LineBreakClass QUnicodeTables::lineBreakClass(uint ucs4)
{
    return (QUnicodeTables::LineBreakClass) qGetProp(ucs4)->line_break_class;
}

Q_CORE_EXPORT int QUnicodeTables::upper(uint ucs4)
{
    const UC_Properties *p = qGetProp(ucs4);
    if (p->category == QChar::Letter_Lowercase)
        return ucs4 + p->caseDiff;
    return ucs4;
}

Q_CORE_EXPORT int QUnicodeTables::lower(uint ucs4)
{
    const UC_Properties *p = qGetProp(ucs4);
    if (p->category == QChar::Letter_Uppercase || p->category == QChar::Letter_Titlecase)
        return ucs4 + p->caseDiff;
    return ucs4;
}

enum {
    Hangul_SBase = 0xac00,
    Hangul_LBase = 0x1100,
    Hangul_VBase = 0x1161,
    Hangul_TBase = 0x11a7,
    Hangul_SCount = 11172,
    Hangul_LCount = 19,
    Hangul_VCount = 21,
    Hangul_TCount = 28,
    Hangul_NCount = 21*28
};

// buffer has to have a length of 3. It's needed for Hangul decomposition
static const unsigned short * QT_FASTCALL decomposition(uint ucs4, int *length, int *tag, unsigned short *buffer)
{
    if (ucs4 >= Hangul_SBase && ucs4 < Hangul_SBase + Hangul_SCount) {
        int SIndex = ucs4 - Hangul_SBase;
        buffer[0] = Hangul_LBase + SIndex / Hangul_NCount; // L
        buffer[1] = Hangul_VBase + (SIndex % Hangul_NCount) / Hangul_TCount; // V
        buffer[2] = Hangul_TBase + SIndex % Hangul_TCount; // T
        *length = buffer[2] == Hangul_TBase ? 2 : 3;
        *tag = QChar::Canonical;
        return buffer;
    }

    const unsigned short index = GET_DECOMPOSITION_INDEX(ucs4);
    if (index == 0xffff)
        return 0;
    const unsigned short *decomposition = uc_decomposition_map+index;
    *tag = (*decomposition) & 0xff;
    *length = (*decomposition) >> 8;
    return decomposition+1;
}

QString QUnicodeTables::decomposition(uint ucs4)
{
    unsigned short buffer[3];
    int length;
    int tag;
    const unsigned short *d = ::decomposition(ucs4, &length, &tag, buffer);
    return QString::fromUtf16(d, length);
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
    // hangul L-V pair
    int LIndex = u1 - Hangul_LBase;
    if (0 <= LIndex && LIndex < Hangul_LCount) {
        int VIndex = u2 - Hangul_VBase;
        if (0 <= VIndex && VIndex < Hangul_VCount)
            return Hangul_SBase + (LIndex * Hangul_VCount + VIndex) * Hangul_TCount;
    }

    // hangul LV-T pair
    int SIndex = u1 - Hangul_SBase;
    if (0 <= SIndex && SIndex < Hangul_SCount && (SIndex % Hangul_TCount) == 0) {
        int TIndex = u2 - Hangul_TBase;
        if (0 <= TIndex && TIndex <= Hangul_TCount)
            return u1 + TIndex;
    }

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


static QString decompose(const QString &str, bool canonical, QChar::UnicodeVersion version)
{
    unsigned short buffer[3];

    QString s = str;

    const unsigned short *utf16 = s.utf16();
    const unsigned short *uc = utf16 + s.length();
    while (uc != utf16) {
        uint ucs4 = *(--uc);
        if (QUnicodeTables::isLowSurrogate(ucs4) && uc != utf16) {
            ushort high = *(uc - 1);
            if (QUnicodeTables::isHighSurrogate(high)) {
                --uc;
                ucs4 = QUnicodeTables::surrogateToUcs4(high, ucs4);
            }
        }
        if (QUnicodeTables::unicodeVersion(ucs4) > version)
            continue;
        int length;
        int tag;
        const unsigned short *d = decomposition(ucs4, &length, &tag, buffer);
        if (!d || (canonical && tag != QChar::Canonical))
            continue;

        s.replace(uc - utf16, ucs4 > 0x10000 ? 2 : 1, (const QChar *)d, length);
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

    if (s.length() < 2)
        return s;

    // the loop can partly ignore high Unicode as all ligatures are in the BMP
    int starter = 0;
    int lastCombining = 0;
    int pos = 0;
    while (pos < s.length()) {
        uint uc = s.utf16()[pos];
        if (QUnicodeTables::isHighSurrogate(uc) && pos < s.length()-1) {
            ushort low = s.utf16()[pos+1];
            if (QUnicodeTables::isLowSurrogate(low)) {
                uc = QUnicodeTables::surrogateToUcs4(uc, low);
                ++pos;
            }
        }
        int combining = QUnicodeTables::combiningClass(uc);
        if (starter == pos - 1 || combining != lastCombining) {
            // allowed to form ligature with S
            QChar ligature = QUnicodeTables::ligature(s.utf16()[starter], uc);
            if (ligature.unicode()) {
                s[starter] = ligature;
                s.remove(pos, 1);
                continue;
            }
        }
        if (!combining)
            starter = pos;
        lastCombining = combining;
        ++pos;
    }
    return s;
}


static QString canonicalOrder(const QString &str, QChar::UnicodeVersion version)
{
    QString s = str;
    const int l = s.length()-1;
    int pos = 0;
    while (pos < l) {
        int p2 = pos+1;
        uint u1 = s.at(pos).unicode();
        if (QUnicodeTables::isHighSurrogate(u1)) {
            ushort low = s.at(pos+1).unicode();
            if (QUnicodeTables::isLowSurrogate(low)) {
                p2++;
                u1 = QUnicodeTables::surrogateToUcs4(u1, low);
                if (p2 >= l)
                    break;
            }
        }
        uint u2 = s.at(p2).unicode();
        if (QUnicodeTables::isHighSurrogate(u2) && p2 < l-1) {
            ushort low = s.at(p2+1).unicode();
            if (QUnicodeTables::isLowSurrogate(low)) {
                p2++;
                u2 = QUnicodeTables::surrogateToUcs4(u2, low);
            }
        }

        int c2 = QUnicodeTables::combiningClass(u2);
        if (QUnicodeTables::unicodeVersion(u2) > version)
            c2 = 0;

        if (c2 == 0) {
            pos = p2+1;
            continue;
        }
        int c1 = QUnicodeTables::combiningClass(u1);
        if (QUnicodeTables::unicodeVersion(u1) > version)
            c1 = 0;

        if (c1 > c2) {
            QChar *uc = s.data();
            int p = pos;
            // exchange characters
            if (u2 < 0x10000) {
                uc[p++] = u2;
            } else {
                uc[p++] = QUnicodeTables::highSurrogate(u2);
                uc[p++] = QUnicodeTables::lowSurrogate(u2);
            }
            if (u1 < 0x10000) {
                uc[p++] = u1;
            } else {
                uc[p++] = QUnicodeTables::highSurrogate(u1);
                uc[p++] = QUnicodeTables::lowSurrogate(u1);
            }
            if (pos > 0)
                --pos;
            if (pos > 0 && QUnicodeTables::isLowSurrogate(s.at(pos).unicode()))
                --pos;
        } else {
            ++pos;
            if (u1 > 0x10000)
                ++pos;
        }
    }
    return s;
}

QString QUnicodeTables::normalize(const QString &str, QString::NormalizationForm mode)
{
    return normalize(str, mode, CURRENT_VERSION);
}

QString QUnicodeTables::normalize(const QString &str, QString::NormalizationForm mode, QChar::UnicodeVersion version)
{
    QString s = str;
    if (version != CURRENT_VERSION) {
        for (int i = 0; i < NumNormalizationCorrections; ++i) {
            const NormalizationCorrection n = uc_normalization_corrections[i];
            if (n.version > version) {
                QString orig;
                orig += QChar(highSurrogate(n.ucs4));
                orig += QChar(lowSurrogate(n.ucs4));
                QString replacement;
                replacement += QChar(highSurrogate(n.old_mapping));
                replacement += QChar(lowSurrogate(n.old_mapping));
                s.replace(orig, replacement);
            }
        }
    }
    s = decompose(s, mode < QString::NormalizationForm_KD, version);

    s = canonicalOrder(s, version);

    if (mode == QString::NormalizationForm_D || mode == QString::NormalizationForm_KD)
        return s;

    return compose(s);

}

int QUnicodeTables::script(uint uc)
{
    if (uc > 0xffff)
        return Common;
    int script = uc_scripts[uc >> 7];
    if (script < ScriptSentinel)
        return script;
    script = (((script - ScriptSentinel) * UnicodeBlockSize) + UnicodeBlockCount);
    return uc_scripts[script + (uc & 0x7f)];
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
    0x1f, Unicode, 0x2f, CombiningMarks, 0x6f, Han,
    0xff, Arabic,                                                // row 0xfe, index 98
#define SCRIPTS_ff 120
    0x60, Han,                        // row 0xff, index 106
    0x9f, KatakanaHalfWidth, 0xef, Han, 0xff, Unicode
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

int qt_scriptForChar(ushort uc)
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
