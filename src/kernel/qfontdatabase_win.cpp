/****************************************************************************
** $Id$
**
** Implementation of QFontDatabase class for Win32
**
** Created : 970521
**
** Copyright (C) 1997-2001 Trolltech AS.  All rights reserved.
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

static void newWinFont( void * p );

static
int CALLBACK
storeFont( ENUMLOGFONTEX* f, TEXTMETRIC*, int /*type*/, LPARAM /*p*/ )
{
    // QFontDatabasePrivate* d = (QFontDatabasePrivate*)p;
    newWinFont( (void*) f );
    return 1; // Keep enumerating.
}

static
void add_style( QtFontFamily *family,
                bool italic, bool lesserItalic, int weight )
{
    QString weightString;
    if ( weight <= QFont::Light ) {
        weight = QFont::Light;
        weightString = "Light";
    } else if ( weight <= QFont::Normal ) {
        weight = QFont::Normal;
        weightString = "Normal";
    } else if ( weight <= QFont::DemiBold ) {
        weight = QFont::DemiBold;
        weightString = "DemiBold";
    } else if ( weight <= QFont::Bold ) {
        weight = QFont::Bold;
        weightString = "Bold";
    } else {
        weight = QFont::Black;
        weightString = "Black";
    }

    QString sn;
    
    if ( weight != QFont::Normal || (!italic && !lesserItalic) ) {
	sn = weightString;
	sn += " ";
    }
    if ( italic || lesserItalic ) {
        sn += "Italic";
    }
#if 0
    if ( lesserItalic ) {
        // Windows doesn't tell the user, so we don't either
        //  sn += "Oblique ";
        sn += "Italic";
    }
#endif
    sn = sn.stripWhiteSpace();
    QtFontStyle *style = family->styleDict.find( sn );
    if ( !style ) {
        // qWarning( "New style[%s] for [%s][%s][%s]",
        // (const char*)styleName, (const char*)charSetName,
        // (const char*)familyName, (const char *)foundryName );
        style = new QtFontStyle( family, sn );
        Q_CHECK_PTR( style );
        style->ital         = italic;
        style->lesserItal   = lesserItalic;
        style->weightString = weightString;
        style->weightVal    = weight;
        style->weightDirty  = FALSE;
        family->addStyle( style );
    }

    //#### eiriken?
#if 0
else
qDebug("Already got it");
#endif

    style->setSmoothlyScalable();  // cowabunga
}


static
void newWinFont( void * p )
{
    ENUMLOGFONTEX* f = (ENUMLOGFONTEX*)p;

    static QtFontFoundry *foundry = 0;

    if ( !foundry ) {
        foundry = new QtFontFoundry( "MS" ); // One foundry on Windows
        // (and only one db)
        db->addFoundry(foundry);
    }

    const TCHAR* tc = f->elfLogFont.lfFaceName;

    QString familyName;
    QT_WA( {
        familyName = QString::fromUcs2( (ushort*)tc );
    } , {
        familyName = QString::fromLocal8Bit((const char*)tc);
    } );

    // the "@family" fonts are just the same as "family". Ignore them.
    if ( familyName[0] == '@' )
	return;

    QtFontFamily *family = foundry->familyDict.find( familyName.lower() );
    if ( !family ) {
        //qWarning( "New font family [%s]", (const char*) familyName );
        family = new QtFontFamily( foundry, familyName );
        Q_CHECK_PTR(family);
        foundry->addFamily( family );
    }
    bool italic = f->elfLogFont.lfItalic;
    int weight = f->elfLogFont.lfWeight/10;

    tc = (TCHAR*)f->elfStyle;

    QString styleName;
    QT_WA( {
        styleName = QString::fromUcs2( (ushort*)tc );
    } , {
        styleName = QString::fromLocal8Bit((const char*)tc);
    } );

    if ( styleName.isEmpty() ) {
        // Not TTF, we enumerate the
        // transformed fonts that Windows can generate.

#if 0
qDebug("%s with quality %x",familyName.latin1(),f->elfLogFont.lfQuality);
#endif

        add_style( family, FALSE, FALSE, weight );
        add_style( family, FALSE, TRUE, weight );

        if ( weight < QFont::DemiBold ) {
            // Can make bolder
            add_style( family, FALSE, FALSE, QFont::Bold );
            add_style( family, FALSE, TRUE, QFont::Bold );
        }
    } else {
        if ( italic ) {
            add_style( family, italic, FALSE, weight );
        } else {
            add_style( family, italic, FALSE, weight );
            add_style( family, italic, TRUE, weight );
        }
        if ( weight < QFont::DemiBold ) {
            // Can make bolder
            if ( italic )
                add_style( family, italic, FALSE, QFont::Bold );
            else {
                add_style( family, FALSE, FALSE, QFont::Bold );
                add_style( family, FALSE, TRUE, QFont::Bold );
            }
        }
    }
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

    // ##### Should add Italic if none already
    // ##### Should add Bold and Bold Italic if any less-than-bold exists
}

void QFontDatabase::createDatabase()
{
    if ( db ) return;
    db = new QFontDatabasePrivate;
    populate_database(QString::null);
}

