#include "qtextdata.h"


const unsigned char QTextData::otherScripts [118] = {
#define SCRIPTS_02 0
    0xaf, QFont::Latin, 0xff, QFont::SpacingModifiers, 			// row 0x02, index 0
#define SCRIPTS_03 4
    0x6f, QFont::CombiningMarks, 0xff, QFont::Greek, 			// row 0x03, index 4
#define SCRIPTS_05 8
    0x2f, QFont::Cyrillic, 0x8f, QFont::Armenian, 0xff, QFont::Hebrew,	// row 0x05, index 8
#define SCRIPTS_07 14
    0x4f, QFont::Syriac, 0x7f, QFont::Unicode, 0xbf, QFont::Thaana,
    0xff, QFont::Unicode, 						// row 0x07, index 14
#define SCRIPTS_10 22
    0x9f, QFont::Myanmar, 0xff, QFont::Georgian,			// row 0x10, index 20
#define SCRIPTS_13 26
    0x7f, QFont::Ethiopic, 0x9f, QFont::Unicode, 0xff, QFont::Cherokee,	// row 0x13, index 24
#define SCRIPTS_16 32
    0x7f, QFont::CanadianAboriginal, 0x9f, QFont::Ogham,
    0xff, QFont::Runic, 						// row 0x16 index 30
#define SCRIPTS_17 38
//     0x1f, QFont::Tagalog, 0x3f, QFont::Hanunoo, 0x5f, QFont::Buhid,
//     0x7f, QFont::Tagbanwa, 0xff, QFont::Khmer,				// row 0x17, index 36
    0x1f, QFont::Unicode, 0x3f, QFont::Unicode, 0x5f, QFont::Unicode,
    0x7f, QFont::Unicode, 0xff, QFont::Khmer,				// row 0x17, index 36
#define SCRIPTS_18 48
    0xaf, QFont::Mongolian, 0xff, QFont::Unicode,		       	// row 0x18, index 46
#define SCRIPTS_20 52
    0x6f, QFont::Unicode, 0x9f, QFont::NumberForms,
    0xab, QFont::CurrencySymbols, 0xac, QFont::LatinExtendedA_15,
    0xcf, QFont::CurrencySymbols, 0xff, QFont::CombiningMarks,		// row 0x20, index 50
#define SCRIPTS_21 64
    0x4f, QFont::LetterlikeSymbols, 0x8f, QFont::NumberForms,
    0xff, QFont::MathematicalOperators,					// row 0x21, index 62
#define SCRIPTS_24 70
    0x5f, QFont::TechnicalSymbols, 0xff, QFont::EnclosedAndSquare,	// row 0x24, index 68
#define SCRIPTS_2e 74
    0x7f, QFont::Unicode, 0xff, QFont::Han,				// row 0x2e, index 72
#define SCRIPTS_30 78
    0x3f, QFont::Han, 0x9f, QFont::Hiragana, 0xff, QFont::Katakana,	// row 0x30, index 76
#define SCRIPTS_31 84
    0x2f, QFont::Bopomofo, 0x8f, QFont::Hangul, 0x9f, QFont::Han,
    0xff, QFont::Unicode,						// row 0x31, index 82
#define SCRIPTS_fb 92
    0x06, QFont::Latin, 0x1c, QFont::Unicode, 0x4f, QFont::Hebrew,
    0xff, QFont::Arabic,						// row 0xfb, index 90
#define SCRIPTS_fe 100
    0x1f, QFont::Unicode, 0x2f, QFont::CombiningMarks, 0x6f, QFont::Unicode,
    0xff, QFont::Arabic,						// row 0xfe, index 98
#define SCRIPTS_ff 108
    0x5e, QFont::Katakana, 0x60, QFont::Unicode,        		// row 0xff, index 106
    0x9f, QFont::KatakanaHalfWidth, 0xfe, QFont::UnknownScript, 0xfe, QFont::Unicode
};

// (uc-0x0900)>>7
const unsigned char QTextData::indicScripts [] =
{
    QFont::Devanagari, QFont::Bengali,
    QFont::Gurmukhi, QFont::Gujarati,
    QFont::Oriya, QFont::Tamil,
    QFont::Telugu, QFont::Kannada,
    QFont::Malayalam, QFont::Sinhala,
    QFont::Thai, QFont::Lao
};


// 0x80 + x: x is the offset into the otherScripts table
const unsigned char QTextData::scriptTable[256] =
{
    QFont::Latin, QFont::Latin, 0x80+SCRIPTS_02, 0x80+SCRIPTS_03,
    QFont::Cyrillic, 0x80+SCRIPTS_05, QFont::Arabic, 0x80+SCRIPTS_07,
    QFont::Unicode, SCRIPTS_INDIC, SCRIPTS_INDIC, SCRIPTS_INDIC,
    SCRIPTS_INDIC, SCRIPTS_INDIC, SCRIPTS_INDIC, QFont::Tibetan,

    0x80+SCRIPTS_10, QFont::Hangul, QFont::Ethiopic, 0x80+SCRIPTS_13,
    QFont::CanadianAboriginal, QFont::CanadianAboriginal, 0x80+SCRIPTS_16, 0x80+SCRIPTS_17,
    0x80+SCRIPTS_18, QFont::Unicode, QFont::Unicode, QFont::Unicode,
    QFont::Unicode, QFont::Unicode, QFont::Latin, QFont::Greek,

    0x80+SCRIPTS_20, 0x80+SCRIPTS_21, QFont::MathematicalOperators, QFont::TechnicalSymbols,
    0x80+SCRIPTS_24, QFont::GeometricSymbols, QFont::MiscellaneousSymbols, QFont::MiscellaneousSymbols,
    QFont::Braille, QFont::Unicode, QFont::Unicode, QFont::Unicode,
    QFont::Unicode, QFont::Unicode, 0x80+SCRIPTS_2e, QFont::Han,

    0x80+SCRIPTS_30, 0x80+SCRIPTS_31, QFont::EnclosedAndSquare, QFont::EnclosedAndSquare,
    QFont::Han, QFont::Han, QFont::Han, QFont::Han,
    QFont::Han, QFont::Han, QFont::Han, QFont::Han,
    QFont::Han, QFont::Han, QFont::Han, QFont::Han,

    QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han,
    QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han,

    QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han,
    QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han,

    QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han,
    QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han,

    QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han,
    QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han,


    QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han,
    QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han,

    QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han,
    QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han, QFont::Han,

    QFont::Yi, QFont::Yi, QFont::Yi, QFont::Yi, QFont::Yi, QFont::Unicode, QFont::Unicode, QFont::Unicode,
    QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul,

    QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul,
    QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul,

    QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul,
    QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul,

    QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul, QFont::Hangul,
    QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode,

    QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode,
    QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode,

    QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode, QFont::Unicode,
    QFont::Unicode, QFont::Han, QFont::Han, 0x80+SCRIPTS_fb, QFont::Arabic, QFont::Arabic, 0x80+SCRIPTS_fe, 0x80+SCRIPTS_ff
};

