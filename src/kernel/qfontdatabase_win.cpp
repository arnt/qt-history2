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


#define SimplifiedChineseCsbBit 18
#define TraditionalChineseCsbBit 20
#define JapaneseCsbBit 17


static
int CALLBACK
#ifndef Q_OS_TEMP
storeFont( ENUMLOGFONTEX* f, NEWTEXTMETRICEX *textmetric, int type, LPARAM /*p*/ )
#else
storeFont( ENUMLOGFONTEX* f, NEWTEXTMETRIC *textmetric, int type, LPARAM /*p*/ )
#endif
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
	if ( !family->scriptCheck && type & TRUETYPE_FONTTYPE ) {
	    FONTSIGNATURE signature;
#ifndef Q_OS_TEMP
	    QT_WA( {
		signature = textmetric->ntmFontSig;
	    }, {
		// the textmetric structure we get from EnumFontFamiliesEx on Win9x has
		// a FONTSIGNATURE, but that one is uninitialized and doesn't work. Have to go
		// the hard way and load the font to find out.
		HDC hdc = GetDC( 0 );
		LOGFONTA lf;
		memset( &lf, 0, sizeof( LOGFONTA ) );
		QCString lfam = familyName.local8Bit();
		memcpy( lf.lfFaceName, familyName.local8Bit(), QMIN( LF_FACESIZE, familyName.local8Bit().length() ) );
		HFONT hfont = CreateFontIndirectA( &lf );
		HGDIOBJ oldobj = SelectObject( hdc, hfont );
		GetTextCharsetInfo( hdc, &signature, 0 );
		SelectObject( hdc, oldobj );
		DeleteObject( hfont );
	    } );
#else
	    CHARSETINFO csi;
	    DWORD charset = textmetric->tmCharSet;
	    TranslateCharsetInfo( &charset, &csi, TCI_SRCCHARSET);
	    signature = csi.fs;
#endif

	    int i;
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
			//qDebug( "font %s: index=%d, flag=%8x supports script %d", familyName.latin1(), index, flag, i );
		    }
		}
	    }
	    if( signature.fsCsb[0] & (1 << SimplifiedChineseCsbBit) ) {
		family->scripts[QFont::Han_SimplifiedChinese] = TRUE;
		//qDebug("font %s supports Simplified Chinese", familyName.latin1() );
	    }
	    if( signature.fsCsb[0] & (1 << TraditionalChineseCsbBit) ) {
		family->scripts[QFont::Han_TraditionalChinese] = TRUE;
		//qDebug("font %s supports Traditional Chinese", familyName.latin1() );
	    }
	    if( signature.fsCsb[0] & (1 << JapaneseCsbBit) ) {
		family->scripts[QFont::Han_Japanese] = TRUE;
		//qDebug("font %s supports Japanese", familyName.latin1() );
	    }
	    family->scriptCheck = true;
	    //qDebug( "usb=%08x %08x csb=%08x for %s", signature.fsUsb[0], signature.fsUsb[1], signature.fsCsb[0], familyName.latin1() );
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

#ifndef Q_OS_TEMP
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
#else
        LOGFONT lf;
        lf.lfCharSet = DEFAULT_CHARSET;
        if ( fam.isNull() ) {
            lf.lfFaceName[0] = 0;
        } else {
            memcpy( lf.lfFaceName, fam.ucs2(), sizeof(TCHAR)*QMIN(fam.length()+1,32));  // 32 = Windows hard-coded
        }
        lf.lfPitchAndFamily = 0;

        EnumFontFamilies( dummy, lf.lfFaceName,
            (FONTENUMPROC)storeFont, (LPARAM)db );
#endif


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


static
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
