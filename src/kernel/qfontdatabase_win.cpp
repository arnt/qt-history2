/****************************************************************************
** $Id$
**
** Implementation of QFontDatabase class for Win32
**
** Created : 970521
**
** Copyright (C) 1997-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qt_windows.h"
#include "qapplication_p.h"
#include "qfontdata_p.h"
#include "qfontengine_p.h"
extern HDC   shared_dc;		// common dc for all fonts
extern HFONT stock_sysfont  = 0;

// #define QFONTDATABASE_DEBUG
// #define FONT_MATCH_DEBUG

// see the Unicode subset bitfields in the MSDN docs
static int requiredUnicodeBits[QFont::NScripts][2] = {
    //Latin,
    { 0, 1 },
    //Greek,
    { 7, 127 },
    //Cyrillic,
    { 9, 127 },
    //Armenian,
    { 10, 127 },
    //Georgian,
    { 26, 127 },
    //Runic,
    { 79, 127 },
    //Ogham,
    { 78, 127 },
    //SpacingModifiers,
    { 5, 127 },
    //CombiningMarks,
    { 6, 127 },

    // Middle Eastern Scripts
    //Hebrew,
    { 11, 127 },
    //Arabic,
    { 13, 67 },
    //Syriac,
    { 71, 127 },
    //Thaana,
    { 72, 127 },

    // South and Southeast Asian Scripts
    //Devanagari,
    { 15, 127 },
    //Bengali,
    { 16, 127 },
    //Gurmukhi,
    { 17, 127 },
    //Gujarati,
    { 18, 127 },
    //Oriya,
    { 19, 127 },
    //Tamil,
    { 20, 127 },
    //Telugu,
    { 21, 127 },
    //Kannada,
    { 22, 127 },
    //Malayalam,
    { 23, 127 },
    //Sinhala,
    { 73, 127 },
    //Thai,
    { 24, 127 },
    //Lao,
    { 25, 127 },
    //Tibetan,
    { 70, 127 },
    //Myanmar,
    { 74, 127 },
    //Khmer,
    { 80, 127 },

    // East Asian Scripts
    //Han,
    { 59, 127 },
    //Hiragana,
    { 49, 127 },
    //Katakana,
    { 50, 127 },
    //Hangul,
    { 56, 127 },
    //Bopomofo,
    { 51, 127 },
    //Yi,
    { 83, 127 },

    // Additional Scripts
    //Ethiopic,
    { 75, 127 },
    //Cherokee,
    { 76, 127 },
    //CanadianAboriginal,
    { 77, 127 },
    //Mongolian,
    { 81, 127 },

    // Symbols
    //CurrencySymbols,
    { 33, 127 },
    //LetterlikeSymbols,
    { 35, 127 },
    //NumberForms,
    { 36, 127 },
    //MathematicalOperators,
    { 38, 127 },
    //TechnicalSymbols,
    { 39, 127 },
    //GeometricSymbols,
    { 43, 127 },
    //MiscellaneousSymbols,
    { 46, 127 },
    //EnclosedAndSquare,
    { 42, 127 },
    //Braille,
    { 82, 127 },

    //Unicode,
    { 126, 126 },

    // some scripts added in Unicode 3.2
    //Tagalog,
    { 84, 127 },
    //Hanunoo,
    { 84, 127 },
    //Buhid,
    { 84, 127 },
    //Tagbanwa,
    { 84, 127 }
};


static
int CALLBACK
storeFont( ENUMLOGFONTEX* f, NEWTEXTMETRICEX *textmetric, int type, LPARAM /*p*/ )
{
    const int script = QFont::Unicode;
    const QString foundryName;
    const bool smoothScalable = TRUE;
    const bool oblique = FALSE;
    Q_UNUSED( script );
    Q_UNUSED( smoothScalable );

    bool italic = FALSE;
    QString familyName;
    int weight;
    bool fixed;

    // ### make non scalable fonts work

    QT_WA( {
	familyName = QString::fromUcs2( (ushort*)f->elfLogFont.lfFaceName );
	italic = f->elfLogFont.lfItalic;
	weight = f->elfLogFont.lfWeight;
	TEXTMETRIC *tm = (TEXTMETRIC *)textmetric;
	fixed = (tm->tmPitchAndFamily & TMPF_FIXED_PITCH);
    } , {
	ENUMLOGFONTEXA* fa = (ENUMLOGFONTEXA *)f;
	familyName = QString::fromLocal8Bit( fa->elfLogFont.lfFaceName );
	italic = fa->elfLogFont.lfItalic;
	weight = fa->elfLogFont.lfWeight;
	TEXTMETRICA *tm = (TEXTMETRICA *)textmetric;
	fixed = (tm->tmPitchAndFamily & TMPF_FIXED_PITCH);
    } );
    // the "@family" fonts are just the same as "family". Ignore them.
    if ( familyName[0] != '@' ) {
	QtFontStyle::Key styleKey;
	styleKey.italic = italic;
	styleKey.oblique = oblique;
	if ( weight < 400 )
	    styleKey.weight = QFont::Light;
	else if ( weight < 600 )
	    styleKey.weight = QFont::Normal;
	else if ( weight < 700 )
	    styleKey.weight = QFont::DemiBold;
	else if ( weight < 800 )
	    styleKey.weight = QFont::Bold;
	else
	    styleKey.weight = QFont::Black;

	QtFontFamily *family = db->family( familyName, TRUE );
	QtFontFoundry *foundry = family->foundry( foundryName,  TRUE );
	QtFontStyle *style = foundry->style( styleKey,  TRUE );
	style->smoothScalable = TRUE;

	// add fonts windows can generate for us:
	if ( styleKey.weight <= QFont::DemiBold ) {
	    QtFontStyle::Key key( styleKey );
	    key.weight = QFont::Bold;
	    QtFontStyle *style = foundry->style( key,  TRUE );
	    style->smoothScalable = TRUE;
	}
	if ( !styleKey.italic ) {
	    QtFontStyle::Key key( styleKey );
	    key.italic = TRUE;
	    QtFontStyle *style = foundry->style( key,  TRUE );
	    style->smoothScalable = TRUE;
	}
	if ( styleKey.weight <= QFont::DemiBold && !styleKey.italic ) {
	    QtFontStyle::Key key( styleKey );
	    key.weight = QFont::Bold;
	    key.italic = TRUE;
	    QtFontStyle *style = foundry->style( key,  TRUE );
	    style->smoothScalable = TRUE;
	    style->pixelSize( SMOOTH_SCALABLE, TRUE );
	}

	family->fixedPitch = fixed;
	
	bool hasScript = false;
	if ( type & TRUETYPE_FONTTYPE ) {
	    FONTSIGNATURE signature;
	    QT_WA( {
		signature = textmetric->ntmFontSig;
	    }, {
		signature = ((NEWTEXTMETRICEXA *)textmetric)->ntmFontSig;
	    } );
	    int i;
	    //qDebug("family %s:", familyName.latin1() );
	    for( i = 0; i < QFont::Unicode; i++ ) {
		int bit = requiredUnicodeBits[i][0];
		int index = bit/32;
		int flag =  1 << (bit&31);
		if ( bit != 126 && signature.fsUsb[index] & flag ) {
		    bit = requiredUnicodeBits[i][1];
		    index = bit/32;
		    flag =  1 << (bit&31);
		    if ( bit == 127 || signature.fsUsb[index] & flag ) {
			family->scripts[i] = TRUE;
			hasScript = true;
			//qDebug( "i=%d, flag=%8x font supports script %d", index, flag, i );
		    }
		}
	    }
	}
	if( !hasScript )
	    family->scripts[QFont::Unicode] = TRUE;
    }

    // keep on enumerating
    return 1;
}

static
void populate_database(const QString& fam)
{
    HDC dummy = GetDC(0);

    QT_WA( {
        LOGFONT lf;
        lf.lfCharSet = DEFAULT_CHARSET;
        if ( fam.isNull() ) {
            lf.lfFaceName[0] = 0;
        } else {
            memcpy( lf.lfFaceName, fam.ucs2(), sizeof(TCHAR)*QMIN(fam.length()+1,32));  // 32 = Windows hard-coded
        }
        lf.lfPitchAndFamily = 0;

        EnumFontFamiliesEx( dummy, &lf,
            (FONTENUMPROC)storeFont, (LPARAM)db, 0 );
    } , {
        LOGFONTA lf;
        lf.lfCharSet = DEFAULT_CHARSET;
        if ( fam.isNull() ) {
            lf.lfFaceName[0] = 0;
        } else {
            QCString lname = fam.local8Bit();
            memcpy(lf.lfFaceName,lname.data(),
                QMIN(lname.length()+1,32));  // 32 = Windows hard-coded
        }
        lf.lfPitchAndFamily = 0;

	EnumFontFamiliesExA( dummy, &lf,
            (FONTENUMPROCA)storeFont, (LPARAM)db, 0 );
    } );

    ReleaseDC(0, dummy);
}

static void initializeDb()
{
    if ( db ) return;

    db = new QFontDatabasePrivate;
    populate_database( QString::null );

#ifdef QFONTDATABASE_DEBUG
    // print the database
    for ( int i = 0; i < QFont::NScripts; i++ ) {
	for ( int f = 0; f < db->count; f++ ) {
	    QtFontFamily *family = db->families[f];
	    qDebug("    %s", family->name.latin1() );
	    populate_database( family->name );

	    for ( int fd = 0; fd < family->count; fd++ ) {
		QtFontFoundry *foundry = family->foundries[fd];
		qDebug("        %s", foundry->name.latin1() );
		for ( int s = 0; s < foundry->count; s++ ) {
		    QtFontStyle *style = foundry->styles[s];
		    qDebug("            style: italic=%d oblique=%d weight=%d",  style->key.italic,
			   style->key.oblique, style->key.weight );
		}
	    }
	}
    }
#endif // QFONTDATABASE_DEBUG

}

void QFontDatabase::createDatabase()
{
    initializeDb();
}

static inline void load(const QString &family = QString::null,  int = -1 )
{
    populate_database( family );
}





// --------------------------------------------------------------------------------------
// font loader
// --------------------------------------------------------------------------------------



#if 0
void QFontPrivate::initFontInfo()
{
    lineWidth = 1;
    actual = request;				// most settings are equal
    QT_WA( {
	TCHAR n[64];
	GetTextFaceW( fin->dc(), 64, n );
	actual.family = QString::fromUcs2((ushort*)n);
	actual.fixedPitch = !(fin->tm.w.tmPitchAndFamily & TMPF_FIXED_PITCH);
    } , {
	char an[64];
	GetTextFaceA( fin->dc(), 64, an );
	actual.family = QString::fromLocal8Bit(an);
	actual.fixedPitch = !(fin->tm.a.tmPitchAndFamily & TMPF_FIXED_PITCH);
    } );
    if ( actual.pointSize == -1 ) {
	if ( paintdevice )
	    actual.pointSize = actual.pixelSize * 720 / QPaintDeviceMetrics( paintdevice ).logicalDpiY();
	else {
	    actual.pointSize = actual.pixelSize * 720 / GetDeviceCaps( fin->dc(), LOGPIXELSY );
	}
    } else if ( actual.pixelSize == -1 ) {
	if ( paintdevice )
	    actual.pixelSize = actual.pointSize * QPaintDeviceMetrics( paintdevice ).logicalDpiY() / 720;
	else
	    actual.pixelSize = actual.pointSize * GetDeviceCaps( fin->dc(), LOGPIXELSY ) / 720;
    }

    actual.dirty = FALSE;
    exactMatch = ( actual.family == request.family &&
		   ( request.pointSize == -1 || ( actual.pointSize == request.pointSize ) ) &&
		   ( request.pixelSize == -1 || ( actual.pixelSize == request.pixelSize ) ) &&
		   actual.fixedPitch == request.fixedPitch );
}

#endif


static inline HFONT systemFont()
{
    if ( stock_sysfont == 0 )
	stock_sysfont = (HFONT)GetStockObject(SYSTEM_FONT);
    return stock_sysfont;
}

#if !defined(DEFAULT_GUI_FONT)
#define DEFAULT_GUI_FONT 17
#endif


static inline
QFontEngine *loadEngine( QFont::Script script, const QFontDef &request,
			 QtFontFamily *family, QtFontFoundry *foundry,
			 QtFontStyle *style )
{
    Q_UNUSED( script );
    Q_UNUSED( foundry );
    Q_UNUSED( style );
    // ########
    QPaintDevice *paintdevice = 0;

    HDC hdc;
    if ( paintdevice ) {
	hdc = paintdevice->handle();
    } else if ( qt_winver & Qt::WV_NT_based ) {
	hdc = GetDC( 0 );
    } else {
	hdc = shared_dc;
    }

    bool stockFont = FALSE;

    HFONT hfont = 0;

    if ( request.mask & QFontDef::RawMode ) {			// will choose a stock font
	int f, deffnt;
	// ### why different?
	if ( (qt_winver & Qt::WV_NT_based) || qt_winver == Qt::WV_32s )
	    deffnt = SYSTEM_FONT;
	else
	    deffnt = DEFAULT_GUI_FONT;
	QString fam = family->name.lower();
	if ( fam == "default" )
	    f = deffnt;
	else if ( fam == "system" )
	    f = SYSTEM_FONT;
#ifndef Q_OS_TEMP
	else if ( fam == "system_fixed" )
	    f = SYSTEM_FIXED_FONT;
	else if ( fam == "ansi_fixed" )
	    f = ANSI_FIXED_FONT;
	else if ( fam == "ansi_var" )
	    f = ANSI_VAR_FONT;
	else if ( fam == "device_default" )
	    f = DEVICE_DEFAULT_FONT;
	else if ( fam == "oem_fixed" )
	    f = OEM_FIXED_FONT;
#endif
	else if ( fam[0] == '#' )
	    f = fam.right(fam.length()-1).toInt();
	else
	    f = deffnt;
	hfont = (HFONT)GetStockObject( f );
	if ( !hfont ) {
#ifndef QT_NO_DEBUG
	    qSystemWarning( "GetStockObject failed" );
#endif
	    hfont = systemFont();
	}
	stockFont = TRUE;
    } else {

	int hint = FF_DONTCARE;
	switch ( request.styleHint ) {
	    case QFont::Helvetica:
		hint = FF_SWISS;
		break;
	    case QFont::Times:
		hint = FF_ROMAN;
		break;
	    case QFont::Courier:
		hint = FF_MODERN;
		break;
	    case QFont::OldEnglish:
		hint = FF_DECORATIVE;
		break;
	    case QFont::System:
		hint = FF_MODERN;
		break;
	    default:
		break;
	}

	LOGFONT lf;
	memset( &lf, 0, sizeof(LOGFONT) );

	lf.lfHeight = -request.pixelSize;
#ifdef Q_OS_TEMP
	lf.lfHeight		+= 3;
#endif
	lf.lfWidth		= 0;
	lf.lfEscapement	= 0;
	lf.lfOrientation	= 0;
	if ( request.weight == 50 )
	    lf.lfWeight = FW_DONTCARE;
	else
	    lf.lfWeight = (request.weight*900)/99;
	lf.lfItalic		= request.italic;
	lf.lfUnderline	= request.underline;
	lf.lfStrikeOut	= request.strikeOut;
	lf.lfCharSet	= DEFAULT_CHARSET;

	int strat = OUT_DEFAULT_PRECIS;
	if (  request.styleStrategy & QFont::PreferBitmap ) {
	    strat = OUT_RASTER_PRECIS;
#ifndef Q_OS_TEMP
	} else if ( request.styleStrategy & QFont::PreferDevice ) {
	    strat = OUT_DEVICE_PRECIS;
	} else if ( request.styleStrategy & QFont::PreferOutline ) {
	    QT_WA( {
		strat = OUT_OUTLINE_PRECIS;
	    } , {
		strat = OUT_TT_PRECIS;
	    } );
	} else if ( request.styleStrategy & QFont::ForceOutline ) {
	    strat = OUT_TT_ONLY_PRECIS;
#endif
	}

	lf.lfOutPrecision   = strat;

	int qual = DEFAULT_QUALITY;

	if ( request.styleStrategy & QFont::PreferMatch )
	    qual = DRAFT_QUALITY;
#ifndef Q_OS_TEMP
	else if ( request.styleStrategy & QFont::PreferQuality )
	    qual = PROOF_QUALITY;
#endif

	if ( request.styleStrategy & QFont::PreferAntialias ) {
	    if ( qt_winver >= Qt::WV_XP )
		qual = 5; // == CLEARTYPE_QUALITY;
	    else
		qual = ANTIALIASED_QUALITY;
	} else if ( request.styleStrategy & QFont::NoAntialias ) {
	    qual = NONANTIALIASED_QUALITY;
	}

	lf.lfQuality	= qual;

	lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
	lf.lfPitchAndFamily = DEFAULT_PITCH | hint;

	QString fam = family->name;
	if ( (fam == "MS Sans Serif") && (request.italic || (-lf.lfHeight > 18 && -lf.lfHeight != 24)) )
	    fam = "Arial"; // MS Sans Serif has bearing problems in italic, and does not scale

	QT_WA( {
	    memcpy(lf.lfFaceName, fam.ucs2(), sizeof(TCHAR)*QMIN(fam.length()+1,32));  // 32 = Windows hard-coded
	    hfont = CreateFontIndirect( &lf );
	} , {
	    // LOGFONTA and LOGFONTW are binary compatible
	    QCString lname = fam.local8Bit();
	    memcpy(lf.lfFaceName,lname.data(),
		QMIN(lname.length()+1,32));  // 32 = Windows hard-coded
	    hfont = CreateFontIndirectA( (LOGFONTA*)&lf );
	} );
#ifndef QT_NO_DEBUG
	if ( !hfont )
	    qSystemWarning( "CreateFontIndirect failed" );
#endif

	stockFont = (hfont == 0);

	if ( hfont && request.stretch != 100 ) {
	    HGDIOBJ oldObj = SelectObject( hdc, hfont );
	    BOOL res;
	    int avWidth = 0;
	    QT_WA( {
		TEXTMETRICW tm;
		res = GetTextMetricsW( hdc, &tm );
		avWidth = tm.tmAveCharWidth;
	    } , {
		TEXTMETRICA tm;
		res = GetTextMetricsA( hdc, &tm);
		avWidth = tm.tmAveCharWidth;
	    } );
#ifndef QT_NO_DEBUG
	    if ( res )
		qSystemWarning( "QFontPrivate: GetTextMetrics failed" );
#endif

	    SelectObject( hdc, oldObj );
	    DeleteObject( hfont );

	    lf.lfWidth = avWidth * request.stretch/100;
	    QT_WA( {
		hfont = CreateFontIndirect( &lf );
	    } , {
		hfont = CreateFontIndirectA( (LOGFONTA*)&lf );
	    } );
#ifndef QT_NO_DEBUG
	    if ( !hfont )
		qSystemWarning( "CreateFontIndirect with stretch failed" );
#endif
	}    

#ifndef Q_OS_TEMP
	if ( hfont == 0 ) {
	    hfont = (HFONT)GetStockObject( ANSI_VAR_FONT );
	    stockFont = TRUE;
	}
#endif

    }

    QFontEngine *fe = new QFontEngineWin( family->name, hdc, hfont, stockFont );
    if ( paintdevice ) 
	fe->paintDevice = TRUE;
    return fe;
}


static const unsigned short sample_chars[QFont::NScripts] =
{
    // European Alphabetic Scripts
    // Latin,
    0x00c0,
    // Greek,
    0x0390,
    // Cyrillic,
    0x0410,
    // Armenian,
    0x0540,
    // Georgian,
    0x10a0,
    // Runic,
    0x16a0,
    // Ogham,
    0x1680,
    // SpacingModifiers,
    0x0000,
    // CombiningMarks,
    0x0300,

    // Middle Eastern Scripts
    // Hebrew,
    0x05d0,
    // Arabic,
    0x0630,
    // Syriac,
    0x0710,
    // Thaana,
    0x0780,

    // South and Southeast Asian Scripts
    // Devanagari,
    0x0910,
    // Bengali,
    0x0990,
    // Gurmukhi,
    0x0a10,
    // Gujarati,
    0x0a90,
    // Oriya,
    0x0b10,
    // Tamil,
    0x0b90,
    // Telugu,
    0x0c10,
    // Kannada,
    0x0c90,
    // Malayalam,
    0x0d10,
    // Sinhala,
    0x0d90,
    // Thai,
    0x0e10,
    // Lao,
    0x0e81,
    // Tibetan,
    0x0f00,
    // Myanmar,
    0x1000,
    // Khmer,
    0x1780,

    // East Asian Scripts
    // Han,
    0x4e00,
    // Hiragana,
    0x3050,
    // Katakana,
    0x30b0,
    // Hangul,
    0xac00,
    // Bopomofo,
    0x3110,
    // Yi,
    0xa000,

    // Additional Scripts
    // Ethiopic,
    0x1200,
    // Cherokee,
    0x13a0,
    // CanadianAboriginal,
    0x1410,
    // Mongolian,
    0x1800,

    // Symbols
    // CurrencySymbols,
    0x20aa,
    // LetterlikeSymbols,
    0x2122,
    // NumberForms,
    0x215b,
    // MathematicalOperators,
    0x222b,
    // TechnicalSymbols,
    0x2440,
    // GeometricSymbols,
    0x2500,
    // MiscellaneousSymbols,
    0x2600,
    // EnclosedAndSquare,
    0x2460,
    // Braille,
    0x2800,

    // Unicode,
    0xfffd,

    // some scripts added in Unicode 3.2
    // Tagalog,
    0x1700,
    // Hanunoo,
    0x1720,
    // Buhid,
    0x1740,
    // Tagbanwa,
    0x1770
};

// returns a sample unicode character for the specified script
static QChar sampleCharacter(QFont::Script script)
{
    return QChar(sample_chars[script]);
}

static inline bool canRender( QFontEngine *fe, const QChar &sample )
{
    if ( !fe ) return FALSE;

    QChar chs[2] = { QChar(0xfffe), sample };
    bool hasChar = !fe->canRender( chs, 1 ) && fe->canRender( chs+1, 1 );

#ifdef FONT_MATCH_DEBUG
    if (hasChar) {
	qDebug("    unicode font has char 0x%04x", sample.unicode() );
    }
#endif

    return hasChar;
}


static
unsigned int bestFoundry( QFont::Script script, unsigned int score, int styleStrategy,
			  const QtFontFamily *family, const QString &foundry_name,
			  QtFontStyle::Key styleKey, int pixelSize, char pitch,
			  QtFontFoundry **best_foundry, QtFontStyle **best_style,
			  QtFontSize **best_size
#ifdef Q_WS_X11
			  , QtFontEncoding **best_encoding 
#endif
			  )
{
    Q_UNUSED( script );
    Q_UNUSED( pitch );
    for ( int x = 0; x < family->count; ++x ) {
	QtFontFoundry *foundry = family->foundries[x];
	if ( ! foundry_name.isEmpty() &&
	     ucstricmp( foundry->name, foundry_name ) != 0 )
	    continue;

#ifdef FONT_MATCH_DEBUG
	qDebug( "  REMARK: looking for matching style in foundry '%s'",
		foundry->name.isEmpty() ? "-- none --" : foundry->name.latin1() );
#endif // FONT_MATCH_DEBUG

	QtFontStyle *style = foundry->style( styleKey );
	if ( !style && styleKey.italic ) {
#ifdef FONT_MATCH_DEBUG
	    qDebug( "          italic not available, looking for oblique" );
#endif // FONT_MATCH_DEBUG
	    styleKey.italic = FALSE;
	    styleKey.oblique = TRUE;
	    style = foundry->style( styleKey );
	}

	if ( !style ) {
#ifdef FONT_MATCH_DEBUG
	    qDebug( "          style not available, looking for closest match" );
#endif // FONT_MATCH_DEBUG

	    int best = 0;
	    int dist = 0xffff;

	    for ( int i = 0; i < foundry->count; i++ ) {
		style = foundry->styles[i];

		int d = QABS( styleKey.weight - style->key.weight );

		if ( styleKey.stretch > 0 && styleKey.stretch != 100 &&
		     style->key.stretch != 0 ) {
		    d += QABS( styleKey.stretch - style->key.stretch );
		}

		if ( styleKey.italic ) {
		    if ( !style->key.italic )
			d += style->key.oblique ? 0x0800 : 0x1000;
		} else if ( styleKey.oblique ) {
		    if (!style->key.oblique )
			d += style->key.italic ? 0x0800 : 0x1000;
		} else if ( style->key.italic || style->key.oblique ) {
		    d += 0x1000;
		}

		if ( d < dist ) {
		    best = i;
		    dist = d;
		}
	    }

#ifdef FONT_MATCH_DEBUG
	    qDebug( "          best style has distance 0x%x", dist );
#endif // FONT_MATCH_DEBUG

	    style = foundry->styles[best];
	}

	int px = pixelSize;
	if ( style->smoothScalable )
	    px = SMOOTH_SCALABLE;
	else if ( style->bitmapScalable && ( styleStrategy & QFont::PreferMatch ) )
	    px = 0; // scale it to the required size

	QtFontSize *size = style->pixelSize( px );
	if ( px != SMOOTH_SCALABLE && !size ) {
	    // find closest size match
	    unsigned int distance = ~0;
	    for ( int x = 0; x < style->count; ++x ) {
		unsigned int d = QABS( style->pixelSizes[x].pixelSize - pixelSize);
		if ( d < distance ) {
		    distance = d;
		    size = style->pixelSizes + x;
		}
	    }

	    if ( style->bitmapScalable &&
		 ! ( styleStrategy & QFont::PreferQuality ) &&
		 ( distance * 10 / pixelSize ) > 2 ) {
		px = 0;
		size = style->pixelSize( px );
	    }
	    Q_ASSERT( size != 0 );
	}

	if ( style->smoothScalable || ( style->bitmapScalable && px == 0 ) )
	    px = pixelSize;
	else
	    px = size->pixelSize;

#ifdef Q_WS_X11
	QtFontEncoding *encoding = size->encodingID( -1 ); // -1 == prefer Xft
	if ( ! encoding ) {
	    for ( int x = 0; ! encoding && x < size->count; ++x ) {
		if ( scripts_for_xlfd_encoding[size->encodings[x].encoding][script] ) {
		    encoding = &size->encodings[x];
		    break;
		}
	    }
	}

	if ( ! encoding ) {
#  ifdef FONT_MATCH_DEBUG
	    qDebug( "          foundry doesn't support the script we want" );
#  endif // FONT_MATCH_DEBUG

	    continue;
	}
#endif // Q_WS_X11

	unsigned int this_score = 0x0000;
#ifdef Q_WS_X11
	if ( encoding->encoding != -1 )
	    this_score += 1;
	if ( !( pitch == 'm' && encoding->pitch == 'c' ) && pitch != encoding->pitch )
	    this_score += 200;
#endif
	if ( styleKey != style->key )
	    this_score += 100;
	if ( !style->smoothScalable && px != size->pixelSize ) // bitmap scaled
	    this_score += 50;

	if ( this_score < score ) {
#ifdef FONT_MATCH_DEBUG
	    qDebug( "          found a match: score %u best score so far %u",
		    this_score, score );
#endif // FONT_MATCH_DEBUG

	    score = this_score;
	    *best_foundry = foundry;
	    *best_style = style;
	    *best_size = size;
#ifdef Q_WS_X11
	    *best_encoding = encoding;
#endif
	}
    }

    return score;
}

/*!
    \internal
*/
QFontEngine *
QFontDatabase::findFont( QFont::Script script, const QFontDef &request, int x11Screen )
{
    Q_UNUSED( x11Screen );
    if ( !db )
	initializeDb();

    QString family_name, foundry_name;
    QtFontStyle::Key styleKey;
    styleKey.italic = request.italic;
    styleKey.weight = request.weight;
    styleKey.stretch = request.stretch;
    char pitch = request.fixedPitch ? 'm' : 'p';

    QFontEngine *fe = 0;

    parseFontName( request.family, foundry_name, family_name );

#ifdef FONT_MATCH_DEBUG
    qDebug( "QFontDatabase::findFont\n"
	    "  REQUEST:\n"
	    "    family: %s [%s], script: %d (%s)\n"
	    "    weight: %d, italic: %d\n"
	    "    stretch: %d\n"
	    "    pixelSize: %d\n"
	    "    pitch: %c",
	    family_name.isEmpty() ? "-- first in script --" : family_name.latin1(),
	    foundry_name.isEmpty() ? "-- any --" : foundry_name.latin1(),
	    script, scriptName( script ).latin1(),
	    request.weight, request.italic, request.stretch, request.pixelSize, pitch );
#endif // FONT_MATCH_DEBUG

    QtFontFamily *best_family = 0;
    QtFontFoundry *best_foundry = 0;
    QtFontStyle *best_style = 0;
    QtFontSize *best_size = 0;
#ifdef Q_WS_X11
    QtFontEncoding *best_encoding = 0;
#endif

    unsigned int score = ~0;

    int loop = 0;
    while ( loop++ < 2 ) {
	if ( loop == 2 || !family_name.isEmpty() )
	    load( family_name, script );

	for ( int x = 0; x < db->count; ++x ) {
	    QtFontFamily *family = db->families[x];
	    if ( !family_name.isEmpty() &&
		 ucstricmp( family->name, family_name ) != 0 )
		continue;

	    if ( loop == 2 && family_name.isNull() )
		load( family->name, script );

	    if ( !( family->scripts[script] & QtFontFamily::Supported ||
		    family->scripts[QFont::UnknownScript] &
		    QtFontFamily::Supported ) )
		continue;

	    // as we know the script is supported, we can be sure
	    // to find a matching font here.
	    unsigned int newscore =
		bestFoundry( script, score, request.styleStrategy,
			     family, foundry_name, styleKey, request.pixelSize, pitch,
			     &best_foundry, &best_style, &best_size
#ifdef Q_WS_X11
			     , &best_encoding 
#endif
			     );
	    if ( best_foundry == 0 ) {
		// the specific foundry was not found, so look for
		// any foundry matching our requirements
		newscore = bestFoundry( script, score, request.styleStrategy, family,
					QString::null, styleKey, request.pixelSize,
					pitch, &best_foundry, &best_style, &best_size
#ifdef Q_WS_X11
					, &best_encoding 
#endif
					);
	    }

	    if ( newscore < score ) {
		score = newscore;
		best_family = family;
	    }
	    if ( newscore < 10 ) // xlfd instead of xft... just accept it
		break;
	}
	if ( score < 10 )  // xlfd instead of xft... just accept it
	    break;
    }

    if ( best_family == 0 || best_foundry == 0 || best_style == 0 
#ifdef Q_WS_X11
	 || best_size == 0 || best_encoding == 0 
#endif
	 ) {
#ifdef FONT_MATCH_DEBUG
	qDebug( "  No matching font found" );
#endif
	return 0;
    }
#ifdef FONT_MATCH_DEBUG
    qDebug( "  BEST:\n"
	    "    family: %s [%s]\n"
	    "    weight: %d, italic: %d, oblique: %d\n"
	    "    stretch: %d\n"
	    "    pixelSize: %d\n"
	    "    pitch: %c",
	    best_family->name.latin1(),
	    best_foundry->name.isEmpty() ? "-- none --" : best_foundry->name.latin1(),
	    best_style->key.weight, best_style->key.italic, best_style->key.oblique,
	    best_style->key.stretch, best_size ? best_size->pixelSize : 0xffff, 
#ifdef Q_WS_X11
	    best_encoding->pitch 
#else
	    'p'
#endif
	    );
#endif // FONT_MATCH_DEBUG

    fe = loadEngine( script, request, best_family, best_foundry, best_style
#ifdef Q_WS_X11
		     , best_size, best_encoding, x11Screen 
#endif
		     );

    if ( fe ) {
	QChar sample = sampleCharacter( script );
	if ( ! canRender( fe, sample ) ) {
#ifdef FONT_MATCH_DEBUG
	    qDebug( "  WARN: font loaded cannot render sample 0x%04x",
		    sample.unicode() );
#endif // FONT_MATCH_DEBUG
	    delete fe;
	    return 0;
	}

	fe->fontDef.family = best_family->name;
	if ( ! best_foundry->name.isEmpty() ) {
	    fe->fontDef.family += QString::fromLatin1( " [" );
	    fe->fontDef.family += best_foundry->name;
	    fe->fontDef.family += QString::fromLatin1( "]" );
	}

	if ( best_style->smoothScalable )
	    fe->fontDef.pixelSize = request.pixelSize;
	else if ( best_style->bitmapScalable &&
		  ( request.styleStrategy & QFont::PreferMatch ) )
	    fe->fontDef.pixelSize = request.pixelSize;
	else
	    fe->fontDef.pixelSize = best_size->pixelSize;

#if defined ( Q_WS_X11 )
	fe->fontDef.pointSize     = int( double( fe->fontDef.pixelSize ) * 720.0 /
					 QPaintDevice::x11AppDpiY( x11Screen ) );
#elif defined ( Q_WS_WIN )
	fe->fontDef.pointSize     = int( double( fe->fontDef.pixelSize ) * 720.0 /
					 GetDeviceCaps(shared_dc,LOGPIXELSY) );
#else
	fe->fontDef.pointSize     = int( double( fe->fontDef.pixelSize ) * 720.0 / 96. ) );
#endif
	fe->fontDef.styleHint     = request.styleHint;
	fe->fontDef.styleStrategy = request.styleStrategy;

	fe->fontDef.weight        = best_style->key.weight;
	fe->fontDef.italic        = best_style->key.italic;
	fe->fontDef.underline     = request.underline;
	fe->fontDef.strikeOut     = request.strikeOut;
	fe->fontDef.fixedPitch    = best_family->fixedPitch;
	fe->fontDef.stretch       = best_style->key.stretch;
    }

    return fe;
}
