
#ifdef QT_THREAD_SUPPORT
#  include <private/qmutexpool_p.h>
#endif // QT_THREAD_SUPPORT
#include "qlibrary.h"


// these defines are from usp10.h
typedef void *SCRIPT_CACHE;
typedef struct tag_SCRIPT_CONTROL {
    DWORD   uDefaultLanguage    :16;
    DWORD   fContextDigits      :1;
    DWORD   fInvertPreBoundDir  :1;
    DWORD   fInvertPostBoundDir :1;
    DWORD   fLinkStringBefore   :1;
    DWORD   fLinkStringAfter    :1;
    DWORD   fNeutralOverride    :1;
    DWORD   fNumericOverride    :1;
    DWORD   fLegacyBidiClass    :1;
    DWORD   fReserved           :8;
} SCRIPT_CONTROL;

typedef struct tag_SCRIPT_STATE {
    WORD    uBidiLevel         :5;
    WORD    fOverrideDirection :1;
    WORD    fInhibitSymSwap    :1;
    WORD    fCharShape         :1;
    WORD    fDigitSubstitute   :1;
    WORD    fInhibitLigate     :1;
    WORD    fDisplayZWG        :1;
    WORD    fArabicNumContext  :1;
    WORD    fGcpClusters       :1;
    WORD    fReserved          :1;
    WORD    fEngineReserved    :2;
} SCRIPT_STATE;

typedef struct tag_SCRIPT_ITEM {
    int              iCharPos;
    QScriptAnalysis  a;
} SCRIPT_ITEM;

typedef GlyphAttributes SCRIPT_VISATTR;
typedef qoffset_t GOFFSET;

#define USP_E_SCRIPT_NOT_IN_FONT   \
        MAKE_HRESULT(SEVERITY_ERROR,FACILITY_ITF,0x200)    // Script doesn't exist in font

typedef struct {
  DWORD   langid              :16;
  DWORD   fNumeric            :1;
  DWORD   fComplex            :1;
  DWORD   fNeedsWordBreaking  :1;
  DWORD   fNeedsCaretInfo     :1;
  DWORD   bCharSet            :8;
  DWORD   fControl            :1;
  DWORD   fPrivateUseArea     :1;
  DWORD   fNeedsCharacterJustify :1;
  DWORD   fInvalidGlyph       :1;
  DWORD   fInvalidLogAttr     :1;
  DWORD   fCDM                :1;
  DWORD   fAmbiguousCharSet   :1;
  DWORD   fClusterSizeVaries  :1;
  DWORD   fRejectInvalid      :1;
} SCRIPT_PROPERTIES;

#ifdef Q_OS_TEMP
typedef struct {
  int     abcA;
  UINT    abcB;
  int     abcC;
} ABC;
#endif

typedef HRESULT (WINAPI *fScriptFreeCache)( SCRIPT_CACHE *);
typedef HRESULT (WINAPI *fScriptItemize)( const WCHAR *, int, int, const SCRIPT_CONTROL *,
					  const SCRIPT_STATE *, SCRIPT_ITEM *, int *);
typedef HRESULT (WINAPI *fScriptShape)( HDC hdc, SCRIPT_CACHE *, const WCHAR *, int, int,
				        QScriptAnalysis *, WORD *, WORD *, SCRIPT_VISATTR *, int *);
typedef HRESULT (WINAPI *fScriptPlace)( HDC, SCRIPT_CACHE *, const WORD *, int, const SCRIPT_VISATTR *, QScriptAnalysis *, int *,
					GOFFSET *, ABC * );
typedef HRESULT (WINAPI *fScriptTextOut)( const HDC, SCRIPT_CACHE *, int, int, UINT, const RECT *, const QScriptAnalysis *,
					 const WCHAR *, int, const WORD *, int, const int *, const int *, const GOFFSET *);
typedef HRESULT (WINAPI *fScriptBreak)( const WCHAR *, int, const QScriptAnalysis *, QCharAttributes * );
//typedef HRESULT (WINAPI *fScriptGetFontProperties)( HDC, SCRIPT_CACHE *, SCRIPT_FONTPROPERTIES * );
typedef HRESULT (WINAPI *fScriptGetProperties)( const SCRIPT_PROPERTIES ***, int *);

fScriptFreeCache ScriptFreeCache = 0;
static fScriptItemize ScriptItemize = 0;
static fScriptShape ScriptShape = 0;
static fScriptPlace ScriptPlace = 0;
fScriptTextOut ScriptTextOut = 0;
static fScriptBreak ScriptBreak = 0;
//static fScriptGetFontProperties ScriptGetFontProperties = 0;
static fScriptGetProperties ScriptGetProperties = 0;

static bool resolvedUsp10 = FALSE;
bool hasUsp10 = FALSE;

const SCRIPT_PROPERTIES **script_properties = 0;
int num_scripts = 0;
int usp_latin_script = 0;


const QFont::Script japanese_tryScripts[] = {
    QFont::Han_Japanese,
    QFont::Hangul,
    QFont::Han_SimplifiedChinese,
    QFont::Han_TraditionalChinese
};

const QFont::Script korean_tryScripts[] = {
    QFont::Hangul,
    QFont::Han_Japanese,
    QFont::Han_SimplifiedChinese,
    QFont::Han_TraditionalChinese
};

const QFont::Script simplifiedChinese_tryScripts[] = {
    QFont::Han_SimplifiedChinese,
    QFont::Han_TraditionalChinese,
    QFont::Han_Japanese,
    QFont::Hangul
};

const QFont::Script traditionalChinese_tryScripts[] = {
    QFont::Han_TraditionalChinese,
    QFont::Han_SimplifiedChinese,
    QFont::Han_Japanese,
    QFont::Hangul
};

const QFont::Script *tryScripts = japanese_tryScripts;

static void uspAppendItems(QTextEngine *engine, int &start, int &stop, BidiControl &control, QChar::Direction dir);

static void resolveUsp10()
{
#ifndef QT_NO_COMPONENT
    if ( !resolvedUsp10 ) {
	// need to resolve the security info functions

#ifdef QT_THREAD_SUPPORT
	// protect initialization
	QMutexLocker locker( qt_global_mutexpool ?
			     qt_global_mutexpool->get( &resolveUsp10 ) : 0 );
	// check triedResolve again, since another thread may have already
	// done the initialization
	if ( resolvedUsp10 ) {
	    // another thread did initialize the security function pointers,
	    // so we shouldn't do it again.
	    return;
	}
#endif

	resolvedUsp10 = TRUE;
	QLibrary lib("usp10");
	lib.setAutoUnload( FALSE );

	ScriptFreeCache = (fScriptFreeCache) lib.resolve( "ScriptFreeCache" );
	ScriptItemize = (fScriptItemize) lib.resolve( "ScriptItemize" );
	ScriptShape = (fScriptShape) lib.resolve( "ScriptShape" );
	ScriptPlace = (fScriptPlace) lib.resolve( "ScriptPlace" );
	ScriptTextOut = (fScriptTextOut) lib.resolve( "ScriptTextOut" );
	ScriptBreak = (fScriptBreak) lib.resolve( "ScriptBreak" );
	ScriptGetProperties = (fScriptGetProperties) lib.resolve( "ScriptGetProperties" );

	if ( !ScriptFreeCache )
	    return;

	hasUsp10 = TRUE;
	ScriptGetProperties( &script_properties, &num_scripts );

	// get the usp script for western
	for( int i = 0; i < num_scripts; i++ ) {
	    if (script_properties[i]->langid == LANG_ENGLISH &&
		!script_properties[i]->fAmbiguousCharSet ) {
		usp_latin_script = i;
		break;
	    }
	}

	// initialize tryScripts according to locale
	LANGID lid = GetUserDefaultLangID();
	switch( lid&0xff ) {
	case LANG_CHINESE: // Chinese (Taiwan)
	    if ( lid == 0x0804 ) // Taiwan
		tryScripts = traditionalChinese_tryScripts;
	    else
	    	tryScripts = simplifiedChinese_tryScripts;
	    break;
	case LANG_JAPANESE:
	    // japanese is already the default
	    break;
	case LANG_KOREAN:
		tryScripts = korean_tryScripts;
		break;
	default:
	    break;
	}

	appendItems = uspAppendItems;
    }
#endif
}

static unsigned char script_for_win_language[ 0x80 ] = {
    //0x00 LANG_NEUTRAL Neutral
    QFont::Latin,
    //0x01 LANG_ARABIC Arabic
    QFont::Arabic,
    //0x02 LANG_BULGARIAN Bulgarian
    QFont::NScripts,
    //0x03 LANG_CATALAN Catalan
    QFont::NScripts,
    //0x04 LANG_CHINESE Chinese
    QFont::Han,
    //0x05 LANG_CZECH Czech
    QFont::NScripts,
    //0x06 LANG_DANISH Danish
    QFont::NScripts,
    //0x07 LANG_GERMAN German
    QFont::NScripts,
    //0x08 LANG_GREEK Greek
    QFont::Greek,
    //0x09 LANG_ENGLISH English
    QFont::Latin,
    //0x0a LANG_SPANISH Spanish
    QFont::NScripts,
    //0x0b LANG_FINNISH Finnish
    QFont::NScripts,
    //0x0c LANG_FRENCH French
    QFont::NScripts,
    //0x0d LANG_HEBREW Hebrew
    QFont::Hebrew,
    //0x0e LANG_HUNGARIAN Hungarian
    QFont::NScripts,
    //0x0f LANG_ICELANDIC Icelandic
    QFont::NScripts,

    //0x10 LANG_ITALIAN Italian
    QFont::NScripts,
    //0x11 LANG_JAPANESE Japanese
    QFont::Hiragana,
    //0x12 LANG_KOREAN Korean
    QFont::Hangul,
    //0x13 LANG_DUTCH Dutch
    QFont::NScripts,
    //0x14 LANG_NORWEGIAN Norwegian
    QFont::NScripts,
    //0x15 LANG_POLISH Polish
    QFont::NScripts,
    //0x16 LANG_PORTUGUESE Portuguese
    QFont::NScripts,
    QFont::NScripts,
    //0x18 LANG_ROMANIAN Romanian
    QFont::NScripts,
    //0x19 LANG_RUSSIAN Russian
    QFont::Cyrillic,
    //0x1a LANG_CROATIAN Croatian
    //0x1a LANG_SERBIAN Serbian
    QFont::NScripts,
    //0x1b LANG_SLOVAK Slovak
    QFont::NScripts,
    //0x1c LANG_ALBANIAN Albanian
    QFont::NScripts,
    //0x1d LANG_SWEDISH Swedish
    QFont::NScripts,
    //0x1e LANG_THAI Thai
    QFont::Thai,
    //0x1f LANG_TURKISH Turkish
    QFont::NScripts,

    //0x20 LANG_URDU Urdu
    QFont::NScripts,
    //0x21 LANG_INDONESIAN Indonesian
    QFont::NScripts,
    //0x22 LANG_UKRAINIAN Ukrainian
    QFont::NScripts,
    //0x23 LANG_BELARUSIAN Belarusian
    QFont::NScripts,
    //0x24 LANG_SLOVENIAN Slovenian
    QFont::NScripts,
    //0x25 LANG_ESTONIAN Estonian
    QFont::NScripts,
    //0x26 LANG_LATVIAN Latvian
    QFont::NScripts,
    //0x27 LANG_LITHUANIAN Lithuanian
    QFont::NScripts,
    QFont::NScripts,
    //0x29 LANG_FARSI Farsi
    QFont::NScripts,
    //0x2a LANG_VIETNAMESE Vietnamese
    QFont::Latin, // ##### maybe use QFont::CombiningMarks instead?
    //0x2b LANG_ARMENIAN Armenian
    QFont::Armenian,
    //0x2c LANG_AZERI Azeri
    QFont::NScripts,
    //0x2d LANG_BASQUE Basque
    QFont::NScripts,
    QFont::NScripts,
    //0x2f LANG_MACEDONIAN FYRO Macedonian
    QFont::NScripts,

    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    //0x36 LANG_AFRIKAANS Afrikaans
    QFont::NScripts,
    //0x37 LANG_GEORGIAN Georgian
    QFont::NScripts,
    //0x38 LANG_FAEROESE Faeroese
    QFont::NScripts,
    //0x39 LANG_HINDI Hindi
    QFont::Devanagari,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    //0x3e LANG_MALAY Malay
    QFont::NScripts,
    //0x3f LANG_KAZAK Kazak
    QFont::NScripts,

    //0x40 LANG_KYRGYZ Kyrgyz
    QFont::NScripts,
    //0x41 LANG_SWAHILI Swahili
    QFont::NScripts,
    QFont::NScripts,
    //0x43 LANG_UZBEK Uzbek
    QFont::NScripts,
    //0x44 LANG_TATAR Tatar
    QFont::NScripts,
    //0x45 LANG_BENGALI Not supported.
    QFont::Bengali,
    //0x46 LANG_PUNJABI Punjabi
    QFont::NScripts,
    //0x47 LANG_GUJARATI Gujarati
    QFont::Gujarati,
    //0x48 LANG_ORIYA Not supported.
    QFont::Oriya,
    //0x49 LANG_TAMIL Tamil
    QFont::Tamil,
    //0x4a LANG_TELUGU Telugu
    QFont::Telugu,
    //0x4b LANG_KANNADA Kannada
    QFont::Kannada,
    //0x4c LANG_MALAYALAM Not supported.
    QFont::Malayalam,
    //0x4d LANG_ASSAMESE Not supported.
    QFont::NScripts,
    //0x4e LANG_MARATHI Marathi
    QFont::NScripts,
    //0x4f LANG_SANSKRIT Sanskrit
    QFont::NScripts,

    //0x50 LANG_MONGOLIAN Mongolian
    QFont::Mongolian,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    //0x56 LANG_GALICIAN Galician
    QFont::NScripts,
    //0x57 LANG_KONKANI Konkani
    QFont::NScripts,
    //0x58 LANG_MANIPURI Not supported.
    QFont::NScripts,
    //0x59 LANG_SINDHI Not supported.
    QFont::NScripts,
    //0x5a LANG_SYRIAC Syriac
    QFont::Syriac,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,

    //0x60 LANG_KASHMIRI Not supported.
    QFont::NScripts,
    //0x61 LANG_NEPALI Not supported.
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    //0x65 LANG_DIVEHI Divehi
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,

    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    QFont::NScripts,
    //0x7f LANG_INVARIANT
    QFont::NScripts,
};

static inline QFont::Script scriptForWinLanguage( DWORD langid )
{
    QFont::Script script = (QFont::Script)script_for_win_language[langid];
    if ( script == QFont::NScripts )
	qWarning( "Qt Uniscribe support: Encountered unhandled language id %x", langid );
    return script;
}

static inline bool isAsian( unsigned short ch )
{
    return (ch > 0x2dff && ch < 0xfb00) || ((ch & 0xff00) == 0x1100);
}


// we're not using Uniscribe's BiDi algorithm, since it is (a) not 100% Unicode compliant and
// (b) seems to work wrongly when trying to use it with a base level != 0.
//
// This function does uses Uniscribe to do the script analysis and creates items from this.
static void uspAppendItems(QTextEngine *engine, int &start, int &stop, BidiControl &control, QChar::Direction dir)
{
    QScriptItemArray &items = engine->items;
    const QChar *text = engine->string.unicode();

    if ( start > stop ) {
	// #### the algorithm is currently not really safe against this. Still needs fixing.
// 	qWarning( "Bidi: appendItems() internal error" );
	return;
    }

    int level = control.level();

    if(dir != QChar::DirON) {
	// add level of run (cases I1 & I2)
	if( level % 2 ) {
	    if(dir == QChar::DirL || dir == QChar::DirAN || dir == QChar::DirEN )
		level++;
	} else {
	    if( dir == QChar::DirR )
		level++;
	    else if( dir == QChar::DirAN || dir == QChar::DirEN )
		level += 2;
	}
    }

    SCRIPT_ITEM s_items[256];
    SCRIPT_ITEM *usp_items = s_items;

    int numItems;
    HRESULT res = ScriptItemize( (WCHAR *)(text+start), stop-start+1, 255, 0, 0, usp_items, &numItems );

    if ( res == E_OUTOFMEMORY ) {
	int alloc = 256;
	usp_items = 0;
	while( res == E_OUTOFMEMORY ) {
	    alloc *= 2;
	    usp_items = (SCRIPT_ITEM *)realloc( usp_items, alloc * sizeof( SCRIPT_ITEM ) );
	    res = ScriptItemize( (WCHAR *)(text+start), stop-start+1, alloc-1, 0, 0, usp_items, &numItems );
	}
    }
    items.resize( items.size() + numItems );
    int i;
    if ( control.singleLine ) {
	for( i = 0; i < numItems; i++ ) {
	    QScriptItem item;
	    item.analysis = usp_items[i].a;
	    item.position = usp_items[i].iCharPos+start;
	    item.analysis.bidiLevel = level;
	    item.analysis.override = control.override();
	    item.analysis.reserved = 0;
	    items.append( item );
	}
    } else {
	for( i = 0; i < numItems; i++ ) {
	    QScriptItem item;
	    item.analysis = usp_items[i].a;
	    item.position = usp_items[i].iCharPos+start;
	    item.analysis.bidiLevel = level;
	    item.analysis.override = control.override();

	    int rstart = usp_items[i].iCharPos;
	    int rstop = usp_items[i+1].iCharPos-1;
	    bool b = TRUE;
	    for ( int j = rstart; j <= rstop; j++ ) {

		unsigned short uc = text[j+start].unicode();
		QChar::Category category = ::category( uc );
		if ( uc == 0xfffcU || uc == 0x2028U ) {
		    item.analysis.script = usp_latin_script;
		    item.isObject = TRUE;
		    b = TRUE;
		} else if ((uc >= 9 && uc <=13) ||
			   (category >= QChar::Separator_Space && category <= QChar::Separator_Paragraph)) {
		    item.analysis.script = usp_latin_script;
		    item.isSpace = TRUE;
		    item.isTab = (uc == '\t');
		    if (item.isTab)
			item.analysis.bidiLevel = control.baseLevel();
		    b = TRUE;
		} else if (b) {
		    b = FALSE;
		} else {
		    continue;
		}

		item.position = j+start;
		items.append( item );
		item.analysis = usp_items[i].a;
		item.analysis.bidiLevel = level;
		item.analysis.override = control.override();
		item.isSpace = item.isTab = item.isObject = FALSE;
	    }
	}
    }

    QCharAttributes *charAttributes = (QCharAttributes *)engine->memory;
    for ( i = 0; i < numItems; i++ ) {
	int from = usp_items[i].iCharPos;
	int len = usp_items[i+1].iCharPos - from;
	// Something I consider a Uniscribe bug: They don't set the softBreak property for asian text (it's just set after
	// spaces in asian text runs. We work around it by calling our implementation for asian text
	if (isAsian(text[from].unicode()))
	    scriptEngines[QFont::Han].charAttributes(QFont::Han, engine->string, from + start, len, charAttributes);
	else
	    ScriptBreak( (const WCHAR *)text + from + start, len, &(usp_items[i].a), charAttributes + from + start);

    }

    if ( usp_items != s_items )
	free( usp_items );

    ++stop;
    start = stop;
}
// -----------------------------------------------------------------------------------------------------
//
// Text engine classes
//
// -----------------------------------------------------------------------------------------------------


QScriptItemArray::~QScriptItemArray()
{
    clear();
    free( d );
}

void QScriptItemArray::clear()
{
    if ( d ) {
	for ( unsigned int i = 0; i < d->size; i++ ) {
	    QScriptItem &si = d->items[i];
	    if ( si.fontEngine )
		si.fontEngine->deref();
	}
	d->size = 0;
    }
}

void QScriptItemArray::resize( int s )
{
    int alloc = (s + 8) >> 3 << 3;
    d = (QScriptItemArrayPrivate *)realloc( d, sizeof( QScriptItemArrayPrivate ) +
		 sizeof( QScriptItem ) * alloc );
    d->alloc = alloc;
}


void QTextEngine::shape( int item ) const
{
    QScriptItem &si = items[item];

    if ( si.num_glyphs )
	return;

    QFont::Script script = (QFont::Script)si.analysis.script;
    int from = si.position;
    int len = length( item );

    Q_ASSERT( len > 0 );

    si.glyph_data_offset = used;

    if ( !si.fontEngine ) {
	if ( hasUsp10 ) {
	    const SCRIPT_PROPERTIES *script_prop = script_properties[si.analysis.script];
	    script = scriptForWinLanguage( script_prop->langid );
	    if ( script == QFont::Latin && script_prop->fAmbiguousCharSet ) {
		// maybe some asian language
		int i;
		for( i = 0; i < 4; i++ ) {
		    QFontEngine *fe = fnt->engineForScript( tryScripts[i] );
		    if ( fe->type() == QFontEngine::Box )
			continue;

		    if ( fe->canRender( string.unicode()+from, len ) ) {
			script = tryScripts[i];
    			break;
		    }
		}
	    }
	}
	si.fontEngine = fnt->engineForScript( script );
	if ( si.fontEngine->type() == QFontEngine::Box )
	    si.fontEngine = fnt->engineForScript( QFont::NoScript );
	si.fontEngine->ref();
    }

    if ( hasUsp10 ) {
	int l = len;
	si.analysis.logicalOrder = TRUE;
	HRESULT res = E_OUTOFMEMORY;
	HDC hdc = 0;

	do {
	    ensureSpace( l );

	    res = ScriptShape( hdc, &si.fontEngine->script_cache, (WCHAR *)string.unicode() + from, len,
			       l, &si.analysis, glyphs( &si ), logClusters( &si ), glyphAttributes( &si ),
			       &si.num_glyphs );
	    if ( res == E_PENDING ) {
		hdc = si.fontEngine->dc();
		SelectObject( hdc, si.fontEngine->hfont );
	    } else if ( res == USP_E_SCRIPT_NOT_IN_FONT ) {
		si.analysis.script = 0;
		hdc = 0;
	    } else if (res == E_OUTOFMEMORY) {
		l += 32;
	    } else if ( res != S_OK ) {
		Q_ASSERT( FALSE );
	    }
	} while( res != S_OK );


	ABC abc;
	res = ScriptPlace( hdc, &si.fontEngine->script_cache, glyphs( &si ), si.num_glyphs,
		           glyphAttributes( &si ), &si.analysis, advances( &si ), offsets( &si ), &abc );
	if ( res == E_PENDING ) {
	    hdc = si.fontEngine->dc();
	    SelectObject( hdc, si.fontEngine->hfont );
	    ScriptPlace( hdc, &si.fontEngine->script_cache, glyphs( &si ), si.num_glyphs,
		         glyphAttributes( &si ), &si.analysis, advances( &si ), offsets( &si ), &abc );
	}
	si.width = abc.abcA + abc.abcB + abc.abcC;
    } else {
	Q_ASSERT( script < QFont::NScripts );
	scriptEngines[script].shape( script, string, from, len, (QTextEngine*)this, &si );
	si.width = 0;
	advance_t *advances = this->advances( &si );
	for ( int i = 0; i < si.num_glyphs; i++ )
	    si.width += advances[i];
    }
    si.ascent = si.fontEngine->ascent();
    si.descent = si.fontEngine->descent();

    ((QTextEngine *)this)->used += si.num_glyphs;
}

