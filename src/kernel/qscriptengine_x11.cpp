// --------------------------------------------------------------------------------------------------------------------------------------------
//
// Continuation of middle eastern languages
//
// --------------------------------------------------------------------------------------------------------------------------------------------


static void syriac_shape( int script, const QString &string, int from, int len, QScriptItem *item )
{
#ifndef QT_NO_XFTFREETYPE
    QOpenType *openType = item->fontEngine->openType();

    if ( openType && openType->supportsScript( QFont::Syriac ) ) {
	openTypeShape( QFont::Syriac, openType, string, from, len, item );
	return;
    }
#endif
    basic_shape( script, string, from, len, item );
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

static inline Form form( const QChar &ch ) {
    ushort uc = ch.unicode();
    if ( uc < 0x900 || uc > 0xdff ) {
	if ( uc == 0x25cc )
	    return Consonant;
	return Other;
    }
    return (Form)indicForms[uc-0x900];
}

static inline bool isRa( const QChar &ch ) {
    ushort uc = ch.unicode();
    if ( uc < 0x900 || uc > 0xd80 )
	return false;
    return ((uc & 0x7f) == 0x30 );
}

enum Position {
    None,
    Pre,
    Above,
    Below,
    Post,
    Split
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

    None, None, None, None,
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
    None, None, None, None,

    None, None, None, None,
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
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
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

    None, None, None, None,
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
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
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
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
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

static inline Position indic_position( const QChar &ch ) {
    unsigned short uc = ch.unicode();
    if ( uc < 0x900 && uc > 0xdff )
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

/* syllables are of the form:

   (Consonant Nukta? Halant)* Consonant Matra? VowelMark? StressMark?
   (Consonant Nukta? Halant)* Consonant Halant
   IndependentVowel VowelMark? StressMark?

   // ### check the above is correct

   We return syllable boundaries on invalid combinations aswell
*/
static int indic_nextSyllableBoundary( int script, const QString &s, int start, int end, bool *invalid )
{
    *invalid = FALSE;
//     qDebug("indic_nextSyllableBoundary: start=%d, end=%d", start, end );
    const QChar *uc = s.unicode()+start;

    int pos = 0;
    Form state = form( uc[pos] );
//     qDebug("state[%d]=%d (uc=%4x)", pos, state, uc[pos].unicode() );
    pos++;

    if ( state != Consonant && state != IndependentVowel ) {
	if ( state != Other )
	    *invalid = TRUE;
	goto finish;
    }

    while ( pos < end - start ) {
	Form newState = form( uc[pos] );
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

static QChar *indic_reorder( int script, const QString &string, int start, int end, unsigned short *featuresToApply,
				GlyphAttributes *attributes, bool invalid, QChar *reordered )
{
    int len = end - start;
    unsigned char properties = scriptProperties[script-QFont::Devanagari];

    if ( invalid ) {
	*reordered = QChar( 0x25cc );
    }
    memcpy( reordered + (invalid ? 1 : 0), string.unicode() + start, len*sizeof( QChar ) );

    QChar *uc = (QChar *)reordered;

    if ( properties & HasSplit ) {
	// We can do this rule at the beginning, as it doesn't interact with later operations.
	// Rule 4: split two part matras into parts
	// This could be done better, but works for now.
	for ( int i = 0; i < len; i++ ) {
	    const QChar *split = 0;
	    if ( script == QFont::Bengali ) {
		if ( uc[i].unicode() == 0x9cb )
		    split = (const QChar *)bengali_o;
		else if ( uc[i].unicode() == 0x9cc )
		    split = (const QChar *)bengali_au;
	    } else if ( script == QFont::Oriya ) {
		if ( uc[i].unicode() == 0xb48 )
		    split = (const QChar *)oriya_ai;
		else if ( uc[i].unicode() == 0xb4b )
		    split = (const QChar *)oriya_o;
		else if ( uc[i].unicode() == 0xb4c )
		    split = (const QChar *)oriya_au;
	    } else if ( script == QFont::Tamil ) {
		if ( uc[i].unicode() == 0xbca )
		    split = (const QChar *)tamil_o;
		else if ( uc[i].unicode() == 0xbcb )
		    split = (const QChar *)tamil_oo;
		else if ( uc[i].unicode() == 0xbcc )
		    split = (const QChar *)tamil_au;
	    } else if ( script == QFont::Telugu ) {
		if ( uc[i].unicode() == 0xc48 )
		    split = (const QChar *)telugu_ai;
	    } else if ( script == QFont::Kannada ) {
		if ( uc[i].unicode() == 0xcc0 )
		    split = (const QChar *)kannada_ii;
		else if ( uc[i].unicode() == 0xcc7 )
		    split = (const QChar *)kannada_ee;
		else if ( uc[i].unicode() == 0xcc8 )
		    split = (const QChar *)kannada_ai;
		else if ( uc[i].unicode() == 0xcca )
		    split = (const QChar *)kannada_o;
		else if ( uc[i].unicode() == 0xccb )
		    split = (const QChar *)kannada_oo;
	    } else if ( script == QFont::Malayalam ) {
		if ( uc[i].unicode() == 0xd4a )
		    split = (const QChar *)malayalam_o;
		else if ( uc[i].unicode() == 0xd4b )
		    split = (const QChar *)malayalam_oo;
		else if ( uc[i].unicode() == 0xd4c )
		    split = (const QChar *)malayalam_au;
	    } else if ( script == QFont::Sinhala ) {
		if ( uc[i].unicode() == 0xdda )
		    split = (const QChar *)sinhala_ee;
		else if ( uc[i].unicode() == 0xddc )
		    split = (const QChar *)sinhala_o;
		else if ( uc[i].unicode() == 0xddd )
		    split = (const QChar *)sinhala_oo;
		else if ( uc[i].unicode() == 0xdde )
		    split = (const QChar *)sinhala_au;
	    }
	    if ( split ) {
		memmove( reordered + i + 1, reordered + i, (len-i)*sizeof( QChar ) );
		reordered[i] = split[0];
		reordered[i+1] = split[1];
		len++;
		break;
	    }
	}
    }

//     qDebug("length=%d",  len );
    int i;
    for ( i = 0; i < len; i++ )
	featuresToApply[i] = 0;

    // nothing to do in this case!
    if ( len == 1 ) {
	attributes[0].mark = (category( *reordered ) == QChar::Mark_NonSpacing);
	attributes[0].clusterStart = TRUE;
	reordered++;
	return reordered;
    }

    int base = 0;
    if ( form( *uc ) == Consonant ) {
	bool reph = FALSE;
	if ( (properties & HasReph) && len > 2 && isRa( uc[0] ) && form( uc[1] ) == Halant ) {
	    reph = TRUE;
// 	    qDebug("Reph");
	}

	// Rule 1: find base consonant
	int lastConsonant = 0;
	for ( int i = len-1; i > 0; i-- ) {
	    if ( form( uc[i] ) == Consonant ) {
		if ( !lastConsonant )
		    lastConsonant = i;
		if ( !(properties & HasReph) || !isRa( uc[i] ) ) {
		    base = i;
		    break;
		}
	    }
	}
	if ( (properties & HasReph) && reph && base == 0 )
	    base = lastConsonant;

// 	qDebug("base consonant at %d skipped=%s", base, lastConsonant != base ? "true" :"false" );

	// Rule 2: move halant of base consonant to last one. Only
	// possible if we skipped consonants while finding the base
	if ( lastConsonant != base && form( uc[base+1] ) == Halant ) {
// 	    qDebug("moving halant from %d to %d!", base+1, lastConsonant);
	    QChar halant = uc[base+1];
	    for ( int i = base+1; i < lastConsonant; i++ )
		uc[i] = uc[i+1];
	    uc[lastConsonant] = halant;

	}

	// Rule 3: Move reph to follow post base matra
	if ( reph ) {
	    int toPos = base+1;
	    if ( toPos < len && form( uc[toPos] ) == Nukta )
		toPos++;
	    // doing this twice takes care of split matras.
	    if ( toPos < len && form( uc[toPos] ) == Matra )
		toPos++;
	    if ( toPos < len && form( uc[toPos] ) == Matra )
		toPos++;
// 	    qDebug("moving reph from %d to %d", 0, toPos );
	    QChar ra = uc[0];
	    QChar halant = uc[1];
	    for ( int i = 2; i < toPos; i++ )
		uc[i-2] = uc[i];
	    uc[toPos-2] = ra;
	    uc[toPos-1] = halant;
	    featuresToApply[toPos-2] |= RephFeature;
	    featuresToApply[toPos-1] |= RephFeature;
	    base -= 2;
	}
    }

    // Rule 5: identify matra position. there are no post/below base consonats
    // in devanagari except for [Ra Halant]_Vattu, but these are already at the
    // right position

    // all reordering happens now to the chars after (base+(reph halant)_vattu?)
    // so we move base to there
    int fixed = base+1;
    if ( fixed < len && form( uc[fixed] ) == Nukta )
	fixed++;
    if ( fixed < len - 1 && isRa( uc[fixed] ) && form( uc[fixed+1] ) == Halant )
	fixed += 2;


    // we continuosly position the matras and vowel marks and increase the fixed
    // until we reached the end.
    static struct {
	Form form;
	Position position;
    } finalOrder [] = {
	{ Matra, Pre },
	{ Matra, Below },
	{ VowelMark, Below },
	{ StressMark, Below },
	{ Matra, Above },
	{ Matra, Post },
	{ Consonant, None },
	{ Halant, None },
	{ VowelMark, Above },
	{ StressMark, Above },
	{ VowelMark, Post },
	{ (Form)0, None }
    };

//      qDebug("base=%d fixed=%d", base, fixed );
    int toMove = 0;
    while ( fixed < len ) {
//  	qDebug("fixed = %d", fixed );
	for ( int i = fixed; i < len; i++ ) {
	    if ( form( uc[i] ) == finalOrder[toMove].form &&
		 indic_position( uc[i] ) == finalOrder[toMove].position ) {
		// need to move this glyph
		int to = fixed;
		if ( finalOrder[toMove].position == Pre )
		    to = (properties&MovePreToFront) ? 0 : base;
//  		qDebug("moving from %d to %d", i,  to );
		QChar ch = uc[i];
		unsigned short feature = featuresToApply[i];
		for ( int j = i; j > to; j-- ) {
		    uc[j] = uc[j-1];
		    featuresToApply[j] = featuresToApply[j-1];
		}
		uc[to] = ch;
		switch( finalOrder[toMove].position ) {
		case Pre:
// 		    feature |= PreSubstFeature;
		    break;
		case Above:
// 		    feature |= AboveSubstFeature;
		    break;
		case Below:
		    feature |= BelowFormFeature;//|BelowSubstFeature;
		    break;
		case Post:
		    feature |= PostSubstFeature;//|PostFormFeature;
		    break;
		case None:
		    break;
		case Split:
		    break;
		}
		featuresToApply[to] = feature;
		fixed++;
	    }
	}
	toMove++;
	if ( finalOrder[toMove].form == 0 )
	    break;
    }

    bool halantForm = base < len-1 && (form( uc[base+1] ) == Halant);
    if ( halantForm ) {
	// #### we have to take care this doesn't get applied to Akhant ligatures,
	// but that's currently rather hard (without a bigger rewrite of the open type
	// API (ftx*.c)
	featuresToApply[base] |= HalantFeature;
	featuresToApply[base+1] |= HalantFeature;
    }

    // set the features we need to apply in OT
    int state = form( uc[0] );
    bool lastWasBase = (base == 0);
    if ( state == Consonant )
	featuresToApply[0] |= AkhantFeature|NuktaFeature;

    for ( i = 1; i < len; i++ ) {
	int newState = form( uc[i] );
	switch( newState ) {
	case Consonant:
	    lastWasBase = (i == base);
	    featuresToApply[i] |= AkhantFeature|NuktaFeature;
	    break;
	case Halant:
	    if ( state == Nukta || state == Consonant ) {
		// vattu or halant feature
		if ( (properties & HasReph) && isRa( uc[i-1] ) && len > 2 ) {
		    if ( !(featuresToApply[i] & RephFeature) ) {
			featuresToApply[i-1] |= BelowFormFeature|VattuFeature;
			featuresToApply[i] |= BelowFormFeature|VattuFeature;
			int j = i-2;
			while ( j >= 0 ) {
			    int f = form( uc[j] );
			    featuresToApply[j] |= VattuFeature;
			    if ( f == Consonant )
				break;
			    j--;
			}
		    }
		}
		else if ( !lastWasBase  ) {
		    if ( state == Nukta )
			featuresToApply[i-2] |= HalfFormFeature;
		    featuresToApply[i-1] |= HalfFormFeature;
		    featuresToApply[i] |= HalfFormFeature;
		}
	    }
	    break;
	case Nukta:
	    if ( state == Consonant ) {
		featuresToApply[i-1] |= NuktaFeature;
		featuresToApply[i] |= NuktaFeature;
	    }
	case StressMark:
	case VowelMark:
	case Matra:
	case LengthMark:
	case IndependentVowel:
	case Invalid:
	case Other:
	    break;
	}
	state = newState;
    }

    for ( i = 0; i < len; i++ ) {
	attributes[i].mark = (category( *reordered ) == QChar::Mark_NonSpacing);
	attributes[i].clusterStart = FALSE;
	reordered++;
    }
    attributes[0].clusterStart = TRUE;

//     qDebug("reordered:");
//     for ( int i = 0; i < (int)reordered.length(); i++ )
// 	qDebug("    %d: %4x apply=%4x clusterStart=%d", i, reordered[i].unicode(), featuresToApply[i], attributes[i].clusterStart );

    return reordered;
}


static QChar *analyzeSyllables( int script, const QString &string, int from, int length,
				 unsigned short *featuresToApply, GlyphAttributes *attributes, QChar *reordered ) {

    int sstart = from;
    int end = sstart + length;
    QChar *current = reordered;
    while ( sstart < end ) {
	bool invalid;
	int send = indic_nextSyllableBoundary( script, string, sstart, end, &invalid );
// 	qDebug("syllable from %d, length %d, invalid=%s", sstart, send-sstart,
// 	       invalid ? "true" : "false" );
	assert( script >= QFont::Devanagari && script <= QFont::Sinhala );
	current = indic_reorder( script, string, sstart, send, featuresToApply+(current-reordered),
				    attributes+(current-reordered), invalid, current );
	sstart = send;
    }
    return current;
}


static void indic_shape( int script, const QString &string, int from, int len, QScriptItem *item )
{
//      qDebug("QScriptEngineDevanagari::shape( from=%d, len=%d)",  from,  len);
    QShapedItem *shaped = item->shaped;

    unsigned short fa[256];
    unsigned short *featuresToApply = fa;
    if ( len > 127 )
	featuresToApply = new unsigned short[ 2*len ];


    shaped->glyphAttributes = (GlyphAttributes *)realloc( shaped->glyphAttributes, ( 3 * len + 1 ) * sizeof( GlyphAttributes )  );

    QChar *reordered = (QChar *)malloc( (3*len + 1) * sizeof( QChar ) );

    QChar *end = analyzeSyllables( script, string, from, len, featuresToApply, shaped->glyphAttributes, reordered );
    shaped->num_glyphs = end - reordered;

    shaped->logClusters = (unsigned short *) realloc( shaped->logClusters, shaped->num_glyphs * sizeof( unsigned short ) );
    int pos = 0;
    for ( int i = 0; i < shaped->num_glyphs; i++ ) {
	if ( shaped->glyphAttributes[i].clusterStart )
	    pos = i;
	shaped->logClusters[i] = pos;
    }

    convertToCMap( reordered, shaped->num_glyphs, item );

#ifndef QT_NO_XFTFREETYPE
    QOpenType *openType = item->fontEngine->openType();

    if ( openType && openType->supportsScript( script ) ) {
	((QOpenType *) openType)->apply( script, featuresToApply, item, len );
    }
#endif

    if ( len > 127 )
	delete [] featuresToApply;
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


static void tibetan_shape( int script, const QString &string, int from, int len, QScriptItem *item )
{
#ifdef QT_NO_XFTFREETYPE
    Q_UNUSED( script );
#endif
//     qDebug("tibetan_shape()");
    QShapedItem *shaped = item->shaped;

    unsigned short fa[256];
    unsigned short *featuresToApply = fa;
    if ( len > 127 )
	featuresToApply = new unsigned short[ len ];

    shaped->glyphAttributes = (GlyphAttributes *)realloc( shaped->glyphAttributes, len * sizeof( GlyphAttributes ) );
    shaped->num_glyphs = len;
    shaped->logClusters = (unsigned short *) realloc( shaped->logClusters, shaped->num_glyphs * sizeof( unsigned short ) );

    tibetan_analyzeSyllables( string, from, len, featuresToApply, shaped->glyphAttributes );

    int pos = 0;
    for ( int i = 0; i < shaped->num_glyphs; i++ ) {
	if ( shaped->glyphAttributes[i].clusterStart )
	    pos = i;
	shaped->logClusters[i] = pos;
    }

    convertToCMap( string.unicode() + from, shaped->num_glyphs, item );

#ifndef QT_NO_XFTFREETYPE
    QOpenType *openType = item->fontEngine->openType();

    if ( openType && openType->supportsScript( script ) ) {
	((QOpenType *) openType)->apply( script, featuresToApply, item, len );
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
    { basic_shape, basic_attributes },
	// Hiragana,
    { basic_shape, basic_attributes },
	// Katakana,
    { basic_shape, basic_attributes },
	// Hangul,
    { basic_shape, basic_attributes },
	// Bopomofo,
    { basic_shape, basic_attributes },
	// Yi,
    { basic_shape, basic_attributes },

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
