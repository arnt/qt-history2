// --------------------------------------------------------------------------------------------------------------------------------------------
//
// Continuation of middle eastern languages
//
// --------------------------------------------------------------------------------------------------------------------------------------------


static void syriac_shape( int script, const QString &string, int from, int len,
			  QTextEngine *engine, QScriptItem *item )
{
#ifndef QT_NO_XFTFREETYPE
    QOpenType *openType = item->fontEngine->openType();

    if ( openType && openType->supportsScript( QFont::Syriac ) ) {
	arabicSyriacOpenTypeShape( QFont::Syriac, openType, string, from, len, engine, item );
	return;
    }
#endif
    basic_shape( script, string, from, len, engine, item );
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//
// Indic languages
//
// --------------------------------------------------------------------------------------------------------------------------------------------

enum Form {
    Invalid = 0x0,
    Unknown = Invalid,
    Consonant,
    Nukta,
    Halant,
    Matra,
    VowelMark,
    StressMark,
    IndependentVowel,
    LengthMark,
    Other
};

static const unsigned char indicForms[0xe00-0x900] = {
    // Devangari
    Invalid, VowelMark, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,

    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Nukta, Other, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Matra, Matra, Matra, Matra,
    Matra, Matra, Matra, Matra,
    Matra, Halant, Unknown, Unknown,

    Other, StressMark, StressMark, StressMark,
    StressMark, Unknown, Unknown, Unknown,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Other, Other, VowelMark, VowelMark,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Bengali
    Invalid, VowelMark, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, Invalid, Invalid, IndependentVowel,

    IndependentVowel, Invalid, Invalid, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Invalid, Consonant, Invalid,
    Invalid, Invalid, Consonant, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Nukta, Other, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Matra, Invalid, Invalid, Matra,
    Matra, Invalid, Invalid, Matra,
    Matra, Halant, Unknown, Unknown,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, VowelMark,
    Invalid, Invalid, Invalid, Invalid,
    Consonant, Consonant, Invalid, Consonant,

    Other, Other, VowelMark, VowelMark,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Gurmukhi
    Invalid, Invalid, VowelMark, Invalid,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, Invalid,
    Invalid, Invalid, Invalid, IndependentVowel,

    IndependentVowel, Invalid, Invalid, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Invalid, Consonant, Consonant,
    Invalid, Consonant, Consonant, Invalid,
    Consonant, Consonant, Unknown, Unknown,
    Nukta, Other, Matra, Matra,

    Matra, Matra, Matra, Invalid,
    Invalid, Invalid, Invalid, Matra,
    Matra, Invalid, Invalid, Matra,
    Matra, Halant, Unknown, Unknown,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Unknown, Unknown, Unknown,
    Invalid, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Invalid,

    Other, Other, Invalid, Invalid,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    StressMark, StressMark, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Gujarati
    Invalid, VowelMark, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    Invalid, IndependentVowel, Invalid, IndependentVowel,

    IndependentVowel, IndependentVowel, Invalid, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Invalid, Consonant, Consonant,
    Invalid, Consonant, Consonant, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Nukta, Other, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Matra, Matra, Invalid, Matra,
    Matra, Matra, Invalid, Matra,
    Matra, Halant, Unknown, Unknown,

    Other, Unknown, Unknown, Unknown,
    Unknown, Unknown, Unknown, Unknown,
    Unknown, Unknown, Unknown, Unknown,
    Unknown, Unknown, Unknown, Unknown,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Oriya
    Invalid, VowelMark, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, Invalid, Invalid, IndependentVowel,

    IndependentVowel, Invalid, Invalid, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Invalid, Consonant, Consonant,
    Invalid, Invalid, Consonant, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Nukta, Other, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Invalid, Invalid, Invalid, Matra,
    Matra, Invalid, Invalid, Matra,
    Matra, Halant, Unknown, Unknown,

    Other, Invalid, Invalid, Invalid,
    Invalid, Unknown, LengthMark, LengthMark,
    Invalid, Invalid, Invalid, Invalid,
    Consonant, Consonant, Invalid, Consonant,

    Other, Other, Invalid, Invalid,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    //Tamil
    Invalid, Invalid, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, Invalid,
    Invalid, Invalid, IndependentVowel, IndependentVowel,

    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,
    IndependentVowel, Consonant, Invalid, Invalid,
    Invalid, Consonant, Consonant, Invalid,
    Consonant, Invalid, Consonant, Consonant,

    Invalid, Invalid, Invalid, Consonant,
    Consonant, Invalid, Invalid, Invalid,
    Consonant, Consonant, Consonant, Invalid,
    Invalid, Invalid, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Invalid, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Invalid, Invalid, Matra, Matra,

    Matra, Matra, Matra, Invalid,
    Invalid, Invalid, Matra, Matra,
    Matra, Invalid, Matra, Matra,
    Matra, Halant, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, LengthMark,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Telugu
    Invalid, VowelMark, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,

    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Invalid, Consonant, Consonant, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Invalid, Invalid, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Matra, Invalid, Matra, Matra,
    Matra, Invalid, Matra, Matra,
    Matra, Halant, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, LengthMark, LengthMark, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Kannada
    Invalid, Invalid, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,

    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Invalid, Consonant, Consonant, Consonant,
    Consonant, Consonant, Unknown, Unknown,
    Invalid, Invalid, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Matra, Invalid, Matra, Matra,
    Matra, Invalid, Matra, Matra,
    Matra, Halant, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, LengthMark, LengthMark, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Consonant, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Malayalam
    Invalid, Invalid, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,

    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Unknown, Unknown,
    Invalid, Invalid, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Invalid, Invalid, Matra, Matra,
    Matra, Invalid, Matra, Matra,
    Matra, Halant, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, LengthMark,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Sinhala
    Invalid, Invalid, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,

    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, Invalid,
    Invalid, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Invalid, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Invalid, Consonant, Invalid, Invalid,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Invalid, Invalid,
    Invalid, Invalid, Halant, Invalid,
    Invalid, Invalid, Invalid, Matra,

    Matra, Matra, Matra, Matra,
    Matra, Invalid, Matra, Invalid,
    Matra, Matra, Matra, Matra,
    Matra, Matra, Matra, Matra,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Matra, Matra,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
};

enum Position {
    None,
    Pre,
    Above,
    Below,
    Post,
    Split,
    Base,
    Reph,
    Vattu,
    Inherit
};

static const unsigned char indicPosition[0xe00-0x900] = {
    // Devanagari
    None, Above, Above, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    Below, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, Post, Pre,

    Post, Below, Below, Below,
    Below, Above, Above, Above,
    Above, Post, Post, Post,
    Post, None, None, None,

    None, Above, Below, Above,
    Above, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, Below, Below,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Bengali
    None, Above, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    Below, None, None, Post,

    Below, None, None, None,
    None, None, None, None,
    None, None, None, None,
    Below, None, Post, Pre,

    Post, Below, Below, Below,
    Below, None, None, Pre,
    Pre, None, None, Split,
    Split, Below, None, None,

    None, None, None, None,
    None, None, None, Post,
    None, None, None, None,
    None, None, None, None,

    None, None, Below, Below,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Gurmukhi
    None, None, Above, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, Post,

    Below, None, None, None,
    None, Below, None, None,
    None, None, None, None,
    Below, None, Post, Pre,

    Post, Below, Below, None,
    None, None, None, Above,
    Above, None, None, Above,
    Above, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    Above, Above, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Gujarati
    None, Above, Above, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    Below, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, Post, Pre,

    Post, Below, Below, Below,
    Below, Above, None, Above,
    Above, Post, None, Post,
    Post, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Oriya
    None, Above, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    Below, None, None, None,
    Below, None, None, None,
    Below, Below, Below, Post,

    Below, None, Below, Below,
    None, None, None, None,
    None, None, None, None,
    None, None, Post, Above,

    Post, Below, Below, Below,
    None, None, None, Pre,
    Split, None, None, Split,
    Split, None, None, None,

    None, None, None, None,
    None, None, Above, Post,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Tamil
    None, None, Above, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, Post, Post,

    Above, Below, Below, None,
    None, None, Pre, Pre,
    Pre, None, Split, Split,
    Split, Halant, None, None,

    None, None, None, None,
    None, None, None, Post,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Telugu
    None, Post, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, Below, Below, Below,
    Below, Below, Below, Below,
    Below, Below, Below, Below,

    Below, Below, Below, Below,
    Below, Below, Below, Below,
    Below, None, Below, Below,
    Below, Below, Below, Below,

    Below, None, Below, Below,
    None, Below, Below, Below,
    Below, Below, None, None,
    None, None, Post, Above,

    Above, Post, Post, Post,
    Post, None, Above, Above,
    Split, None, Post, Above,
    Above, Halant, None, None,

    None, None, None, None,
    None, Above, Below, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Kannada
    None, None, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, Below, Below, Below,
    Below, Below, Below, Below,
    Below, Below, Below, Below,

    Below, Below, Below, Below,
    Below, Below, Below, Below,
    Below, Below, Below, Below,
    Below, Below, Below, Below,

    Below, None, Below, Below,
    None, Below, Below, Below,
    Below, Below, None, None,
    None, None, Post, Above,

    Split, Post, Post, Post,
    Post, None, Above, Split,
    Split, None, Split, Split,
    Above, Halant, None, None,

    None, None, None, None,
    None, Post, Post, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Malayalam
    None, None, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, Below,

    None, None, None, Below,
    None, Below, Below, None,
    None, None, None, None,
    None, None, None, Post,

    Post, None, Below, None,
    None, Post, None, None,
    None, None, None, None,
    None, None, Post, Post,

    Post, Post, Post, Post,
    None, None, Pre, Pre,
    Pre, None, Split, Split,
    Split, Halant, None, None,

    None, None, None, None,
    None, None, None, Post,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Sinhala
    None, None, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, Post,

    Post, Post, Above, Above,
    Below, None, Below, None,
    Post, Pre, Split, Pre,
    Split, Split, Split, Post,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None
};

static inline Form form( unsigned short uc ) {
    if ( uc < 0x900 || uc > 0xdff ) {
	if ( uc == 0x25cc )
	    return Consonant;
	return Other;
    }
    return (Form)indicForms[uc-0x900];
}

static inline Position indic_position( unsigned short uc ) {
    if ( uc < 0x900 || uc > 0xdff )
	return None;
    return (Position) indicPosition[uc-0x900];
}


enum IndicScriptProperties {
    HasReph = 0x01,
    MovePreToFront = 0x02,
    HasSplit = 0x04
};

const uchar scriptProperties[10] = {
    // Devanagari,
    HasReph|MovePreToFront,
    // Bengali,
    HasReph|MovePreToFront|HasSplit,
    // Gurmukhi,
    MovePreToFront,
    // Gujarati,
    HasReph|MovePreToFront,
    // Oriya,
    HasReph|MovePreToFront|HasSplit,
    // Tamil,
    HasSplit,
    // Telugu,
    HasSplit|MovePreToFront,
    // Kannada,
    HasSplit,
    // Malayalam,
    HasSplit,
    // Sinhala,
    HasSplit
};

struct IndicOrdering {
    Form form;
    Position position;
};

static const IndicOrdering devanagari_order [] = {
    { Consonant, Below },
    { Matra, Below },
    { VowelMark, Below },
    { StressMark, Below },
    { Matra, Above },
    { Matra, Post },
    { Consonant, Reph },
    { VowelMark, Above },
    { StressMark, Above },
    { VowelMark, Post },
    { (Form)0, None }
};

static const IndicOrdering bengali_order [] = {
    { Consonant, Below },
    { Matra, Below },
    { Matra, Above },
    { Consonant, Reph },
    { VowelMark, Above },
    { Consonant, Post },
    { Matra, Post },
    { VowelMark, Post },
    { (Form)0, None }
};

static const IndicOrdering gurmukhi_order [] = {
    { Consonant, Below },
    { Matra, Below },
    { Matra, Above },
    { Consonant, Post },
    { Matra, Post },
    { VowelMark, Above },
    { (Form)0, None }
};

static const IndicOrdering tamil_order [] = {
    { Matra, Above },
    { Matra, Post },
    { VowelMark, Post },
    { (Form)0, None }
};

static const IndicOrdering telugu_order [] = {
    { Matra, Above },
    { Matra, Below },
    { Matra, Post },
    { Consonant, Below },
    { Consonant, Post },
    { VowelMark, Post },
    { (Form)0, None }
};

static const IndicOrdering kannada_order [] = {
    { Matra, Above },
    { Matra, Post },
    { Consonant, Below },
    { Consonant, Post },
    { LengthMark, Post },
    { Consonant, Reph },
    { VowelMark, Post },
    { (Form)0, None }
};

static const IndicOrdering malayalam_order [] = {
    { Consonant, Below },
    { Matra, Below },
    { Consonant, Reph },
    { Consonant, Post },
    { Matra, Post },
    { VowelMark, Post },
    { (Form)0, None }
};

static const IndicOrdering * const indic_order[] = {
    devanagari_order, // Devanagari
    bengali_order, // Bengali
    // Gurmukhi
    devanagari_order, // Gujarati
    bengali_order, // Oriya
    tamil_order, // Tamil
    telugu_order, // Telugu
    kannada_order, // Kannada
    malayalam_order, // Malayalam
    devanagari_order // Sinhala // ### no OT specs available, we use devanagari
};



// vowel matras that have to be split into two parts.
static const unsigned short bengali_o[2]  = { 0x9c7, 0x9be };
static const unsigned short bengali_au[2] = { 0x9c7, 0x9d7 };

static const unsigned short oriya_ai[2]    = { 0xb47, 0xb56 };
static const unsigned short oriya_o[2]    = { 0xb47, 0xb3e };
static const unsigned short oriya_au[2]    = { 0xb47, 0xb57 };

static const unsigned short tamil_o[2]    = { 0xbc6, 0xbbe };
static const unsigned short tamil_oo[2]   = { 0xbc7, 0xbbe };
static const unsigned short tamil_au[2]   = { 0xbc6, 0xbd7 };

static const unsigned short telugu_ai[2]   = { 0xc46, 0xc56 };

static const unsigned short kannada_ii[2]   = { 0xcbf, 0xcd5 };
static const unsigned short kannada_ee[2]   = { 0xcc6, 0xcd5 };
static const unsigned short kannada_ai[2]   = { 0xcc6, 0xcd6 };
static const unsigned short kannada_o[2]    = { 0xcc6, 0xcc2 };
static const unsigned short kannada_oo[2]   = { 0xcca, 0xcd5 };

static const unsigned short malayalam_o[2]   = { 0xd46, 0xd3e };
static const unsigned short malayalam_oo[2]   = { 0xd47, 0xd3e };
static const unsigned short malayalam_au[2]   = { 0xd46, 0xd57 };

static const unsigned short sinhala_ee[2]   = { 0xdd9, 0xdca };
static const unsigned short sinhala_o[2]   = { 0xdd9, 0xdcf };
static const unsigned short sinhala_oo[2]   = { 0xddc, 0xdca };
static const unsigned short sinhala_au[2]   = { 0xdd9, 0xddf };

inline void splitMatra( int script, unsigned short *reordered, int matra, int &len, int &base)
{
    const unsigned short *split = 0;
    unsigned short matra_uc = reordered[matra];
    if ( script == QFont::Bengali ) {
	if ( matra_uc == 0x9cb )
	    split = bengali_o;
	else if ( matra_uc == 0x9cc )
	    split = bengali_au;
    } else if ( script == QFont::Oriya ) {
	if ( matra_uc == 0xb48 )
	    split = oriya_ai;
	else if ( matra_uc == 0xb4b )
	    split = oriya_o;
	else if ( matra_uc == 0xb4c )
	    split = oriya_au;
    } else if ( script == QFont::Tamil ) {
	if ( matra_uc == 0xbca )
	    split = tamil_o;
	else if ( matra_uc == 0xbcb )
	    split = tamil_oo;
	else if ( matra_uc == 0xbcc )
	    split = tamil_au;
    } else if ( script == QFont::Telugu ) {
	if ( matra_uc == 0xc48 )
	    split = telugu_ai;
    } else if ( script == QFont::Kannada ) {
	if ( matra_uc == 0xcc0 )
	    split = kannada_ii;
	else if ( matra_uc == 0xcc7 )
	    split = kannada_ee;
	else if ( matra_uc == 0xcc8 )
	    split = kannada_ai;
	else if ( matra_uc == 0xcca )
	    split = kannada_o;
	else if ( matra_uc == 0xccb )
	    split = kannada_oo;
    } else if ( script == QFont::Malayalam ) {
	if ( matra_uc == 0xd4a )
	    split = malayalam_o;
	else if ( matra_uc == 0xd4b )
	    split = malayalam_oo;
	else if ( matra_uc == 0xd4c )
	    split = malayalam_au;
    } else if ( script == QFont::Sinhala ) {
	if ( matra_uc == 0xdda )
	    split = sinhala_ee;
	else if ( matra_uc == 0xddc )
	    split = sinhala_o;
	else if ( matra_uc == 0xddd )
	    split = sinhala_oo;
	else if ( matra_uc == 0xdde )
	    split = sinhala_au;
    }
    assert(split);
    if (indic_position(*split) == Pre) {
	reordered[matra] = split[1];
	memmove(reordered + 1, reordered, len*sizeof(unsigned short));
	reordered[0] = split[0];
	base++;
    } else {
	memmove(reordered + matra + 1, reordered + matra, (len-matra)*sizeof(unsigned short));
	reordered[matra] = split[0];
	reordered[matra+1] = split[1];
    }
    len++;
}

// #define INDIC_DEBUG
#ifdef INDIC_DEBUG
#define IDEBUG qDebug
#else
#define IDEBUG if(0) qDebug
#endif

static void indic_shape( int script, const QString &string, int from, int syllableLength,
			 QTextEngine *engine, QScriptItem *si, QOpenType *openType, bool invalid )
{
    assert( script >= QFont::Devanagari && script <= QFont::Sinhala );
    const unsigned short script_base = 0x0900 + 0x80*(script-QFont::Devanagari);
    const unsigned short ra = script_base + 0x30;
    const unsigned short halant = script_base + 0x4d;
    const unsigned short nukta = script_base + 0x3c;

    int len = syllableLength;
    IDEBUG(">>>>> devanagari shape: from=%d, len=%d invalid=%d", from, len, invalid);

    unsigned short r[64];
    unsigned short *reordered = r;
    GlyphAttributes ga[64];
    GlyphAttributes *glyphAttributes = ga;
    glyph_t gl[64];
    glyph_t *glyphs = gl;
    unsigned char p[64];
    unsigned char *position = p;
    if ( len > 60 ) {
	reordered = (unsigned short *)malloc((len+4)*sizeof(unsigned short));
	glyphAttributes = (GlyphAttributes *)malloc((len+4)*sizeof(GlyphAttributes));
	glyphs = (glyph_t *)malloc((len+4)*sizeof(glyph_t));
	position = (unsigned char*)malloc((len+4)*sizeof(unsigned char));
    }

    unsigned char properties = scriptProperties[script-QFont::Devanagari];

    unsigned short *copyTo = reordered;
    if ( invalid ) {
	*reordered = 0x25cc;
	len++;
	copyTo++;
    }
    memcpy( copyTo, string.unicode() + from, len*sizeof( QChar ) );

    int base = 0;
    int reph = -1;

#ifdef INDIC_DEBUG
    IDEBUG("original:");
    for ( int i = 0; i < len; i++ ) {
	IDEBUG("    %d: %4x", i, reordered[i]);
    }
#endif

    if ( len != 1 ) {
	unsigned short *uc = reordered;
	bool beginsWithRa = FALSE;

	// Rule 1: find base consonant
	//
	// The shaping engine finds the base consonant of the
	// syllable, using the following algorithm: starting from the
	// end of the syllable, move backwards until a consonant is
	// found that does not have a below-base or post-base form
	// (post-base forms have to follow below-base forms), or
	// arrive at the first consonant. The consonant stopped at
	// will be the base.
	//
 	//  * If the syllable starts with Ra + H (in a script that has
 	//    'Reph'), Ra is excluded from candidates for base
 	//    consonants.
	//
 	// * In Kannada and Telugu, the base consonant cannot be
 	//   farther than 3 consonants from the end of the syllable.
	if (form(*uc) == Consonant) {
	    beginsWithRa = ((len > 2) && *uc == ra && *(uc+1) == halant);
	    base = (beginsWithRa && (properties & HasReph) ? 2 : 0);
	    IDEBUG("    length = %d, beginsWithRa = %d, base=%d", len, beginsWithRa, base );

	    int lastConsonant = 0;
	    int matra = -1;
	    int skipped = 0;
	    Position pos = Post;
	    // we remember:
	    // * the last consonant since we need it for rule 2
	    // * the matras position for rule 3 and 4

	    // figure out possible base glyphs
	    memset(position, 0, len);
	    if (script == QFont::Devanagari || script == QFont::Gujarati) {
		bool vattu = FALSE;
		for (int i = base; i < len; ++i) {
		    position[i] = form(uc[i]);
		    if (position[i] == Consonant) {
			lastConsonant = i;
			vattu = (!vattu && uc[i] == ra);
			if (vattu) {
			    IDEBUG("excluding vattu glyph at %d from base candidates", i);
			    position[i] = Vattu;
			}
		    } else if (position[i] == Matra) {
			matra = i;
		    }
		}
	    } else {
		for (int i = base; i < len; ++i) {
		    position[i] = form(uc[i]);
		    if (position[i] == Consonant)
			lastConsonant = i;
		    else if (position[i] == Matra)
			matra = i;
		}
	    }
	    for (int i = len-1; i > base; i--) {
		if (position[i] != Consonant)
		    continue;

		Position charPosition = indic_position(uc[i]);
		if (pos == Post && charPosition == Post) {
		    pos = Below;
		} else if ((pos == Post || pos == Below) && charPosition == Below) {
		    pos = None;
		    if (script == QFont::Devanagari || script == QFont::Gujarati)
			base = i;
		} else {
		    base = i;
		    break;
		}
		if (skipped == 2 && (script == QFont::Kannada || script == QFont::Telugu)) {
		    base = i;
		    break;
		}
		++skipped;
	    }

	    IDEBUG("    base consonant at %d skipped=%d, lastConsonant=%d", base, skipped, lastConsonant );

	    // Rule 2:
	    //
	    // If the base consonant is not the last one, Uniscribe
	    // moves the halant from the base consonant to the last
	    // one.
	    if ( lastConsonant != base && uc[base+1] == halant ) {
		IDEBUG("    moving halant from %d to %d!", base+1, lastConsonant);
		for ( int i = base+1; i < lastConsonant; i++ )
		    uc[i] = uc[i+1];
		uc[lastConsonant] = halant;

	    }

	    // Rule 3:
	    //
	    // If the syllable starts with Ra + H, Uniscribe moves
	    // this combination so that it follows either:

	    // * the post-base 'matra' (if any) or the base consonant
	    //   (in scripts that show similarity to Devanagari, i.e.,
	    //   Devanagari, Gujarati, Bengali)
	    // * the base consonant (other scripts)
	    // * the end of the syllable (Kannada)

	    Position matra_position = None;
	    if (matra > 0)
		matra_position = indic_position( uc[matra] );
	    IDEBUG("    matra at %d with form %d, base=%d", matra, matra_position, base);

 	    if (beginsWithRa && base != 0) {
		int toPos = base+1;
		if ( toPos < len && uc[toPos] == nukta )
		    toPos++;
		if ( toPos < len-1 && uc[toPos] == ra && uc[toPos+1] == halant )
		    toPos += 2;
		if (script == QFont::Devanagari || script == QFont::Gujarati || script == QFont::Bengali) {
		    if (matra_position == Post || matra_position == Split) {
			toPos = matra + 1;
			matra -= 2;
		    }
		} else if (script == QFont::Kannada) {
		    toPos = len - 1;
		    matra -= 2;
		}

		IDEBUG("moving leading ra+halant to position %d", toPos);
		for ( int i = 2; i < toPos; i++ )
		    uc[i-2] = uc[i];
		uc[toPos-2] = ra;
		uc[toPos-1] = halant;
		base -= 2;
		if (properties & HasReph)
		    reph = toPos-2;
	    }

	    // Rule 4:

	    // Uniscribe splits two- or three-part matras into their
	    // parts. This splitting is a character-to-character
	    // operation).
	    //
	    //      Uniscribe describes some moving operations for these
	    //      matras here. For shaping however all pre matras need
	    //      to be at the begining of the syllable, so we just move
	    //      them there now.
	    if (matra_position == Split)
		splitMatra(script, uc, matra, len, base);
	    else if (matra_position == Pre) {
		unsigned short m = uc[matra];
		while (matra--)
		    uc[matra+1] = uc[matra];
		uc[0] = m;
		base++;
	    }
	}

	// Rule 5:
	//
	// Uniscribe classifies consonants and 'matra' parts as
	// pre-base, above-base (Reph), below-base or post-base. This
	// classification exists on the character code level and is
	// language-dependent, not font-dependent.
	for (int i = 0; i < base; ++i)
	    position[i] = Pre;
	position[base] = Base;
	for (int i = base+1; i < len; ++i) {
	    position[i] = indic_position(uc[i]);
	    // #### replace by adjusting table
	    if (uc[i] == nukta || uc[i] == halant)
		position[i] = Inherit;
	}
	if (reph > 0) {
	    position[reph] = Reph;
	    position[reph+1] = Inherit;
	}

	// all reordering happens now to the chars after the base
	int fixed = base+1;
	if ( fixed < len && uc[fixed] == nukta )
	    fixed++;

#ifdef INDIC_DEBUG
	for (int i = fixed; i < len; ++i)
	    IDEBUG("position[%d] = %d, form=%d", i, position[i],  form(uc[i]));
#endif
	// we continuosly position the matras and vowel marks and increase the fixed
	// until we reached the end.
	const IndicOrdering *finalOrder = indic_order[script-QFont::Devanagari];

	IDEBUG("    reordering pass:");
	//IDEBUG("        base=%d fixed=%d", base, fixed );
	int toMove = 0;
	while ( finalOrder[toMove].form && fixed < len-1 ) {
	    //IDEBUG("        fixed = %d, moving form %d with pos %d", fixed, finalOrder[toMove].form, finalOrder[toMove].position );
	    for ( int i = fixed; i < len; i++ ) {
		if ( form( uc[i] ) == finalOrder[toMove].form &&
		     position[i] == finalOrder[toMove].position ) {
		    // need to move this glyph
		    int to = fixed;
		    if (i < len-1 && position[i+1] == Inherit) {
			IDEBUG("         moving two chars from %d to %d", i,  to );
			unsigned short ch = uc[i];
			unsigned short ch2 = uc[i+1];
			unsigned char pos = position[i];
			for ( int j = i+1; j > to+1; j-- ) {
			    uc[j] = uc[j-2];
			    position[j] = uc[j-2];
			}
			uc[to] = ch;
			uc[to+1] = ch2;
			position[to] = pos;
			position[to+1] = pos;
			fixed += 2;
		    } else {
			IDEBUG("         moving one char from %d to %d", i,  to );
			unsigned short ch = uc[i];
			unsigned char pos = position[i];
			for ( int j = i; j > to; j-- ) {
			    uc[j] = uc[j-1];
			    position[j] = position[j-1];
			}
			uc[to] = ch;
			position[to] = pos;
			fixed++;
		    }
		}
	    }
	    toMove++;
	}

    }
    IDEBUG("reordered:");
    for ( int i = 0; i < len; i++ ) {
	glyphAttributes[i].mark = FALSE;
	glyphAttributes[i].clusterStart = FALSE;
	IDEBUG("    %d: %4x", i, reordered[i]);
    }
    IDEBUG("  base=%d, reph=%d", base, reph);
    glyphAttributes[0].clusterStart = TRUE;

    // now we have the syllable in the right order, and can start running it through open type.

    int firstGlyph = si->num_glyphs;

#ifndef QT_NO_XFTFREETYPE
    if (openType) {
	int error = si->fontEngine->stringToCMap((QChar *)reordered, len, glyphs, 0, &len,
						 (si->analysis.bidiLevel %2));
	assert (!error);

	// we need to keep track of where the base glyph is for some scripts and abuse the logcluster feature for this.
	// This also means we have to correct the logCluster output from the open type engine manually afterwards.
	// for indic this is rather simple, as all chars just point to the first glyph in the syllable.
	unsigned short lc[64];
	unsigned short *logClusters = lc;
	bool w[64];
	bool *where = w;
	if (len > 63) {
	    where = (bool *)malloc(len*sizeof(bool));
	    logClusters = (unsigned short *)malloc((len+4)*sizeof(unsigned short));
	}
	memset(where, 0, len*sizeof(bool));
	for (int i = 0; i < len; ++i)
	    logClusters[i] = i;

	openType->init(glyphs, glyphAttributes, len, logClusters, len);

	// substitutions

	openType->applyGSUBFeature(FT_MAKE_TAG( 'c', 'c', 'm', 'p' ));

	where[0] = TRUE;
	openType->applyGSUBFeature(FT_MAKE_TAG( 'i', 'n', 'i', 't' ), where);
	openType->applyGSUBFeature(FT_MAKE_TAG( 'n', 'u', 'k', 't' ));

	for (int i = 0; i <= base; ++i)
	    where[i] = TRUE;
	openType->applyGSUBFeature(FT_MAKE_TAG( 'a', 'k', 'h', 'n' ), where);

	memset(where, 0, len*sizeof(bool));
	if (reph >= 0) {
	    where[reph] = where[reph+1] = TRUE;
	    openType->applyGSUBFeature(FT_MAKE_TAG( 'r', 'p', 'h', 'f' ), where);
	    where[reph] = where[reph+1] = FALSE;
	}

	for (int i = base+1; i < len; ++i)
	    where[i] = TRUE;
	if (script == QFont::Devanagari || script == QFont::Gujarati) {
	    // vattu glyphs need this aswell
	    bool vattu = FALSE;
	    for (int i = base-2; i > 1; --i) {
		if (form(reordered[i]) == Consonant) {
		    vattu = (!vattu && reordered[i] == ra);
		    if (vattu) {
			IDEBUG("forming vattu ligature at %d", i);
			where[i] = where[i+1] = TRUE;
		    }
		}
	    }
	}
	openType->applyGSUBFeature(FT_MAKE_TAG( 'b', 'l', 'w', 'f' ), where);
	memset(where, 0, len*sizeof(bool));
	for (int i = 0; i < base; ++i)
	    where[i] = TRUE;
	openType->applyGSUBFeature(FT_MAKE_TAG( 'h', 'a', 'l', 'f' ), where);
	openType->applyGSUBFeature(FT_MAKE_TAG( 'p', 's', 'b', 'f' ));
	openType->applyGSUBFeature(FT_MAKE_TAG( 'v', 'a', 't', 'u' ));

	// Conjunkts and typographical forms
	openType->applyGSUBFeature(FT_MAKE_TAG( 'p', 'r', 'e', 's' ));
	openType->applyGSUBFeature(FT_MAKE_TAG( 'b', 'l', 'w', 's' ));
	openType->applyGSUBFeature(FT_MAKE_TAG( 'a', 'b', 'v', 's' ));
	openType->applyGSUBFeature(FT_MAKE_TAG( 'p', 's', 't', 's' ));

	// halant forms
	if (reordered[len-1] == halant) {
	    where[len-1] = where[len-2] = TRUE;
	    openType->applyGSUBFeature(FT_MAKE_TAG( 'h', 'a', 'l', 'n' ));
	}

	int newLen;
	const int *char_map = openType->mapping(newLen);

	// move the left matra back to it's correct position in malayalam and tamil
	if ((script == QFont::Malayalam || script == QFont::Tamil) && (form(reordered[0]) == Matra)) {
	    // need to find the base in the shaped string and move the matra there
	    int prebase = 0;
	    while (prebase < newLen && char_map[prebase] < base)
		prebase++;
	    if (prebase == newLen)
		prebase = 0;
	    if (prebase != 0) {
		unsigned short *g = openType->glyphs();
		unsigned short m = g[0];
		for (int i = 0; i < prebase; ++i)
		    g[i] = g[i+1];
		g[prebase] = m;
	    }
	}

	openType->applyGPOSFeatures();

	GlyphAttributes *ga = engine->glyphAttributes(si)+si->num_glyphs;

	for (int i = 0; i < newLen; ++i)
	    ga[i] = glyphAttributes[char_map[i]];


	openType->appendTo(engine, si, FALSE);

	if (w != where) {
	    free(where);
	    free(logClusters);
	}
    } else
#endif
    {
	Q_UNUSED(openType);
	// can't do any shaping, copy the stuff to the script item.
	engine->ensureSpace(len);

	glyph_t *glyphs = engine->glyphs(si)+si->num_glyphs;
	advance_t *advances = engine->advances(si)+si->num_glyphs;
	GlyphAttributes *ga = engine->glyphAttributes(si)+si->num_glyphs;

	int error = si->fontEngine->stringToCMap((QChar *)reordered, len, glyphs, advances, &len,
						 (si->analysis.bidiLevel %2));
	assert (!error);

	memcpy(ga, glyphAttributes, len*sizeof(GlyphAttributes));

	si->num_glyphs += len;
    }

    // fix logcluster array
    unsigned short *logClusters = engine->logClusters(si)+from-si->position;
    for (int i = 0; i < syllableLength; ++i)
	logClusters[i] = firstGlyph;

    if (r != reordered) {
	free(reordered);
	free(glyphAttributes);
	free(glyphs);
	free(position);
    }
    IDEBUG("<<<<<<");
}


/* syllables are of the form:

   (Consonant Nukta? Halant)* Consonant Matra? VowelMark? StressMark?
   (Consonant Nukta? Halant)* Consonant Halant
   IndependentVowel VowelMark? StressMark?

   We return syllable boundaries on invalid combinations aswell
*/
static int indic_nextSyllableBoundary( int script, const QString &s, int start, int end, bool *invalid )
{
    *invalid = FALSE;
//     qDebug("indic_nextSyllableBoundary: start=%d, end=%d", start, end );
    const QChar *uc = s.unicode()+start;

    int pos = 0;
    Form state = form( uc[pos].unicode() );
//     qDebug("state[%d]=%d (uc=%4x)", pos, state, uc[pos].unicode() );
    pos++;

    if ( state != Consonant && state != IndependentVowel ) {
	if ( state != Other )
	    *invalid = TRUE;
	goto finish;
    }

    while ( pos < end - start ) {
	Form newState = form( uc[pos].unicode() );
// 	qDebug("state[%d]=%d (uc=%4x)", pos, newState, uc[pos].unicode() );
	switch( newState ) {
	case Consonant:
	    if ( state == Halant )
		break;
	    goto finish;
	case Halant:
	    if ( state == Nukta || state == Consonant )
		break;
	    goto finish;
	case Nukta:
	    if ( state == Consonant )
		break;
	    goto finish;
	case StressMark:
	    if ( state == VowelMark )
		break;
	    // fall through
	case VowelMark:
	    if ( state == Matra || state == IndependentVowel )
		break;
	    // fall through
	case Matra:
	    if ( state == Consonant || state == Nukta )
		break;
	    // ### not sure if this is correct. If it is, does it apply only to Bengali or should
	    // it work for all Indic languages?
	    // the combination Independent_A + Vowel Sign AA is allowed.
	    if ( script == QFont::Bengali && uc[pos].unicode() == 0x9be && uc[pos-1].unicode() == 0x985 )
		break;
	    if ( script == QFont::Tamil && state == Matra ) {
		if ( uc[pos-1].unicode() == 0x0bc6 &&
		     ( uc[pos].unicode() == 0xbbe || uc[pos].unicode() == 0xbd7 ) )
		    break;
		if ( uc[pos-1].unicode() == 0x0bc7 && uc[pos].unicode() == 0xbbe )
		    break;
	    }
	    goto finish;

	case LengthMark:
	case IndependentVowel:
	case Invalid:
	case Other:
	    goto finish;
	}
	state = newState;
	pos++;
    }
 finish:
    return pos+start;
}


static void indic_shape( int script, const QString &string, int from, int len, QTextEngine *engine, QScriptItem *si )
{
    assert( script >= QFont::Devanagari && script <= QFont::Sinhala );
    si->num_glyphs = 0;
    int sstart = from;
    int end = sstart + len;
#ifndef QT_NO_XFTFREETYPE
    QOpenType *openType = si->fontEngine->openType();
    if (openType && !openType->supportsScript(script))
	openType = 0;
#else
    QOpenType *openType = 0;
#endif

    while ( sstart < end ) {
	bool invalid;
	int send = indic_nextSyllableBoundary( script, string, sstart, end, &invalid );
 	IDEBUG("syllable from %d, length %d, invalid=%s", sstart, send-sstart,
 	       invalid ? "true" : "false" );
	indic_shape(script, string, sstart, send-sstart, engine, si, openType, invalid);
	sstart = send;
    }
}


static void indic_attributes( int script, const QString &text, int from, int len, QCharAttributes *attributes )
{
    int end = from + len;
    const QChar *uc = text.unicode() + from;
    attributes += from;
    int i = 0;
    while ( i < len ) {
	bool invalid;
	int boundary = indic_nextSyllableBoundary( script, text, from+i, end, &invalid ) - from;

	attributes[i].whiteSpace = ::isSpace( *uc ) && (uc->unicode() != 0xa0);
	attributes[i].softBreak = FALSE;
	attributes[i].charStop = TRUE;
	attributes[i].wordStop = FALSE;
	attributes[i].invalid = invalid;

	if ( boundary > len-1 ) boundary = len;
	i++;
	while ( i < boundary ) {
	    attributes[i].whiteSpace = ::isSpace( *uc ) && (uc->unicode() != 0xa0);
	    attributes[i].softBreak = FALSE;
	    attributes[i].charStop = FALSE;
	    attributes[i].wordStop = FALSE;
	    attributes[i].invalid = invalid;
	    ++uc;
	    ++i;
	}
	assert( i == boundary );
    }


}

// --------------------------------------------------------------------------------------------------------------------------------------------
//
// Tibetan
//
// --------------------------------------------------------------------------------------------------------------------------------------------

// tibetan syllables are of the form:
//    head position consonant
//    first sub-joined consonant
//    ....intermediate sub-joined consonants (if any)
//    last sub-joined consonant
//    sub-joined vowel (a-chung U+0F71)
//    standard or compound vowel sign (or 'virama' for devanagari transliteration)

enum TibetanForm {
    TibetanOther,
    TibetanHeadConsonant,
    TibetanSubjoinedConsonant,
    TibetanSubjoinedVowel,
    TibetanVowel
};

// this table starts at U+0f40
static unsigned char tibetanForm[0x80] = {
    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,
    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,
    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,
    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,

    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,
    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,
    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,
    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,

    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,
    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,
    TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant, TibetanHeadConsonant,
    TibetanOther, TibetanOther, TibetanOther, TibetanOther,

    TibetanOther, TibetanVowel, TibetanVowel, TibetanVowel,
    TibetanVowel, TibetanVowel, TibetanVowel, TibetanVowel,
    TibetanVowel, TibetanVowel, TibetanVowel, TibetanVowel,
    TibetanVowel, TibetanVowel, TibetanVowel, TibetanVowel,

    TibetanVowel, TibetanVowel, TibetanVowel, TibetanVowel,
    TibetanVowel, TibetanVowel, TibetanVowel, TibetanVowel,
    TibetanOther, TibetanOther, TibetanOther, TibetanOther,
    TibetanOther, TibetanOther, TibetanOther, TibetanOther,

    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,
    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,
    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,
    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,

    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,
    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,
    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,
    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,

    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,
    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,
    TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant, TibetanSubjoinedConsonant,
    TibetanSubjoinedConsonant, TibetanOther, TibetanOther, TibetanOther
};


static inline TibetanForm tibetan_form( const QChar &c )
{
    return (TibetanForm)tibetanForm[ c.unicode() - 0x0f40 ];
}

static int tibetan_nextSyllableBoundary( const QString &s, int start, int end, bool *invalid,
					 unsigned short *featuresToApply = 0, GlyphAttributes *attributes = 0 )
{
    const QChar *uc = s.unicode() + start;

    int pos = 0;
    TibetanForm state = tibetan_form( *uc );

    if ( attributes ) {
	attributes[0].justification = 0;
	attributes[0].clusterStart = TRUE;
	attributes[0].mark = FALSE;
	attributes[0].zeroWidth = FALSE;
    }
    if ( featuresToApply ) {
	featuresToApply[0] = AboveSubstFeature|BelowSubstFeature;
    }
//     qDebug("state[%d]=%d (uc=%4x)", pos, state, uc[pos].unicode() );
    pos++;

    if ( state != TibetanHeadConsonant ) {
	if ( state != TibetanOther )
	    *invalid = TRUE;
	goto finish;
    }

    while ( pos < end - start ) {
	TibetanForm newState = tibetan_form( uc[pos] );
	switch( newState ) {
	case TibetanSubjoinedConsonant:
	case TibetanSubjoinedVowel:
	    if ( state != TibetanHeadConsonant &&
		 state != TibetanSubjoinedConsonant )
		goto finish;
	    state = newState;
	    break;
	case TibetanVowel:
	    if ( state != TibetanHeadConsonant &&
		 state != TibetanSubjoinedConsonant &&
		 state != TibetanSubjoinedVowel )
		goto finish;
	    break;
	case TibetanOther:
	case TibetanHeadConsonant:
	    goto finish;
	}
	if ( attributes ) {
	    attributes[pos].justification = 0;
	    attributes[pos].clusterStart = FALSE;
	    attributes[pos].mark = TRUE;
	    attributes[pos].zeroWidth = TRUE;
	}
	if ( featuresToApply ) {
	    featuresToApply[pos] = AboveSubstFeature|BelowSubstFeature;
	}
	pos++;
    }

finish:
    *invalid = false;
    return start+pos;
}

static void tibetan_analyzeSyllables( const QString &string, int from, int length,
				 unsigned short *featuresToApply, GlyphAttributes *attributes ) {
    int sstart = from;
    int end = sstart + length;
    while ( sstart < end ) {
	bool invalid;
	int send = tibetan_nextSyllableBoundary( string, sstart, end,
						 &invalid, featuresToApply + sstart - from,
						 attributes + sstart - from );
// 	qDebug("tibetan syllable from %d, length %d, invalid=%s", sstart, send-sstart,
// 	       invalid ? "true" : "false" );
	sstart = send;
    }
}


static void tibetan_shape( int script, const QString &string, int from, int len, QTextEngine *engine, QScriptItem *si )
{
#ifdef QT_NO_XFTFREETYPE
    Q_UNUSED( script );
#endif
//     qDebug("tibetan_shape()");
    si->hasPositioning = TRUE;

    unsigned short fa[256];
    unsigned short *featuresToApply = fa;
    if ( len > 127 )
	featuresToApply = new unsigned short[ len ];

    GlyphAttributes *glyphAttributes = engine->glyphAttributes( si );
    unsigned short *logClusters = engine->logClusters( si );

    si->num_glyphs = len;
    engine->ensureSpace( si->num_glyphs );

    tibetan_analyzeSyllables( string, from, len, featuresToApply, glyphAttributes );

    int pos = 0;
    for ( int i = 0; i < si->num_glyphs; i++ ) {
	if ( glyphAttributes[i].clusterStart )
	    pos = i;
	logClusters[i] = pos;
    }

    convertToCMap( string.unicode() + from, si->num_glyphs, engine, si );

#ifndef QT_NO_XFTFREETYPE
    QOpenType *openType = si->fontEngine->openType();

    if ( openType && openType->supportsScript( script ) ) {
	//	((QOpenType *) openType)->apply( script, featuresToApply, engine, si, len );
    }
#endif

    if ( len > 127 )
	delete featuresToApply;


}

static void tibetan_attributes( int script, const QString &text, int from, int len, QCharAttributes *attributes )
{
#if 0
    int end = from + len;
    const QChar *uc = text.unicode() + from;
    attributes += from;
    int i = 0;
    while ( i < len ) {
	bool invalid;
	int boundary = tibetan_nextSyllableBoundary( text, from+i, end, &invalid ) - from;

	attributes[i].whiteSpace = ::isSpace( *uc ) && (uc->unicode() != 0xa0);
	attributes[i].softBreak = FALSE;
	attributes[i].charStop = TRUE;
	attributes[i].wordStop = FALSE;
	attributes[i].invalid = invalid;

	if ( boundary > len-1 ) boundary = len;
	i++;
	while ( i < boundary ) {
	    attributes[i].whiteSpace = ::isSpace( *uc ) && (uc->unicode() != 0xa0);
	    attributes[i].softBreak = FALSE;
	    attributes[i].charStop = FALSE;
	    attributes[i].wordStop = FALSE;
	    attributes[i].invalid = invalid;
	    ++uc;
	    ++i;
	}
	assert( i == boundary );
    }
#else
    basic_attributes( script, text, from, len, attributes );
#endif
}

// --------------------------------------------------------------------------------------------------------------------------------------------
//
// The script engine jump table
//
// --------------------------------------------------------------------------------------------------------------------------------------------

const q_scriptEngine scriptEngines[] = {
	// Latin,
    { basic_shape, basic_attributes },
	// Greek,
    { basic_shape, basic_attributes },
	// Cyrillic,
    { basic_shape, basic_attributes },
	// Armenian,
    { basic_shape, basic_attributes },
	// Georgian,
    { basic_shape, basic_attributes },
	// Runic,
    { basic_shape, basic_attributes },
	// Ogham,
    { basic_shape, basic_attributes },
	// SpacingModifiers,
    { basic_shape, basic_attributes },
	// CombiningMarks,
    { basic_shape, basic_attributes },

	// // Middle Eastern Scripts
	// Hebrew,
    { basic_shape, basic_attributes },
	// Arabic,
    { arabic_shape, arabic_attributes },
	// Syriac,
    { syriac_shape, arabic_attributes },
	// Thaana,
    { basic_shape, basic_attributes },

	// // South and Southeast Asian Scripts
	// Devanagari,
    { indic_shape, indic_attributes },
	// Bengali,
    { indic_shape, indic_attributes },
	// Gurmukhi,
    { indic_shape, indic_attributes },
	// Gujarati,
    { indic_shape, indic_attributes },
	// Oriya,
    { indic_shape, indic_attributes },
	// Tamil,
    { indic_shape, indic_attributes },
	// Telugu,
    { indic_shape, indic_attributes },
	// Kannada,
    { indic_shape, indic_attributes },
	// Malayalam,
    { indic_shape, indic_attributes },
	// Sinhala,
    { indic_shape, indic_attributes },
	// Thai,
    { basic_shape, basic_attributes },
	// Lao,
    { basic_shape, basic_attributes },
	// Tibetan,
    { tibetan_shape, tibetan_attributes },
	// Myanmar,
    { basic_shape, basic_attributes },
	// Khmer,
    { basic_shape, basic_attributes },

	// // East Asian Scripts
	// Han,
    { basic_shape, asian_attributes },
	// Hiragana,
    { basic_shape, asian_attributes },
	// Katakana,
    { basic_shape, asian_attributes },
	// Hangul,
    { basic_shape, asian_attributes },
	// Bopomofo,
    { basic_shape, asian_attributes },
	// Yi,
    { basic_shape, asian_attributes },

	// // Additional Scripts
	// Ethiopic,
    { basic_shape, basic_attributes },
	// Cherokee,
    { basic_shape, basic_attributes },
	// CanadianAboriginal,
    { basic_shape, basic_attributes },
	// Mongolian,
    { basic_shape, basic_attributes },

	// // Symbols
	// CurrencySymbols,
    { basic_shape, basic_attributes },
	// LetterlikeSymbols,
    { basic_shape, basic_attributes },
	// NumberForms,
    { basic_shape, basic_attributes },
	// MathematicalOperators,
    { basic_shape, basic_attributes },
	// TechnicalSymbols,
    { basic_shape, basic_attributes },
	// GeometricSymbols,
    { basic_shape, basic_attributes },
	// MiscellaneousSymbols,
    { basic_shape, basic_attributes },
	// EnclosedAndSquare,
    { basic_shape, basic_attributes },
	// Braille,
    { basic_shape, basic_attributes },

	// Unicode,
    { basic_shape, basic_attributes },
    //Tagalog,
    { basic_shape, basic_attributes },
    //Hanunoo,
    { basic_shape, basic_attributes },
    //Buhid,
    { basic_shape, basic_attributes },
    //Tagbanwa,
    { basic_shape, basic_attributes },
    // KatakanaHalfWidth
    { basic_shape, basic_attributes }
};
