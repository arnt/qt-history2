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

#define QFONTDATABASE_DEBUG

static
int CALLBACK
storeFont( ENUMLOGFONTEX* f, NEWTEXTMETRICEX *textmetric, int /*type*/, LPARAM /*p*/ )
{
    const int script = QFont::Unicode;
    const QString foundryName;
    const bool smoothScalable = TRUE;
    const bool oblique = FALSE;

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

	QtFontFamily *family = db->scripts[script].family( familyName, TRUE );
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
	}

	family->fixedPitch = fixed;
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

void QFontDatabase::createDatabase()
{
    if ( db ) return;

    db = new QFontDatabasePrivate;
    populate_database( QString::null );

#ifdef QFONTDATABASE_DEBUG
    // print the database
    for ( int i = 0; i < QFont::NScripts; i++ ) {
	QtFontScript &script = db->scripts[i];
	qDebug("Script %s", QFontDatabase::scriptName( (QFont::Script)i ).latin1() );
	if ( !script.count ) {
	    qDebug("    No fonts found!");
	    continue;
	}
	for ( int f = 0; f < script.count; f++ ) {
	    QtFontFamily *family = script.families[f];
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

