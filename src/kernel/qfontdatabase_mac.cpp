/****************************************************************************
** $Id$
**
** Implementation of platform specific QFontDatabase
**
** Created : 970521
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Macintosh may use this file in accordance with the Qt Commercial
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

#include "qt_mac.h"

static
void add_style( QtFontFamily *family, const QString& styleName,
                bool italic, bool lesserItalic, int weight )
{
    QString weightString;
    if ( weight <= QFont::Light ) {
        weight = QFont::Light;
        weightString = "Light";
    } else if ( weight <= QFont::Normal ) {
        weight = QFont::Normal;
        weightString = "Regular";
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

    QString sn = styleName;
    if ( sn.isEmpty() ) {
        // Not TTF, we make the name
        if ( weight != QFont::Normal || !italic && !lesserItalic ) {
            sn += weightString;
            sn += " ";
        }
        if ( italic )
            sn += "Italic ";
        if ( lesserItalic ) {
            // Windows doesn't tell the user, so we don't either
            //  sn += "Oblique ";
            sn += "Italic ";
        }
        sn = sn.left(sn.length()-1); // chomp " "
    }
    QtFontStyle *style = family->styleDict.find( sn );
    if ( !style ) {
        style = new QtFontStyle( family, sn );
        Q_CHECK_PTR( style );
        style->ital         = italic;
        style->lesserItal   = lesserItalic;
        style->weightString = weightString;
        style->weightVal    = weight;
        style->weightDirty  = FALSE;
        family->addStyle( style );
    }
    style->setSmoothlyScalable();  // cowabunga
}

void QFontDatabase::createDatabase()
{
    if ( db ) return;
    db = new QFontDatabasePrivate;

    QtFontFoundry *foundry = NULL;
    if ( !foundry ) {
	foundry = new QtFontFoundry( "Mac" ); // One foundry on Macintosh
	db->addFoundry(foundry);        // (and only one db)
    }

    FMFontFamilyIterator it;
    if(!FMCreateFontFamilyIterator(NULL, NULL, kFMUseGlobalScopeOption, &it)) {
	FMFontFamily fam;
	QString fam_name;
	while(!FMGetNextFontFamily(&it, &fam)) {

	    static Str255 n;
	    if(FMGetFontFamilyName(fam, n))
		qDebug("Whoa! %s %d", __FILE__, __LINE__);
	    if(!n[0] || n[1] == '.') //throw out ones starting with a .
		continue;

	    TextEncoding encoding;
	    FMGetFontFamilyTextEncoding( fam, &encoding);
	    TextToUnicodeInfo uni_info;
	    CreateTextToUnicodeInfoByEncoding( encoding, &uni_info);

	    unsigned long len = n[0] * 2;
	    unsigned char *buff = (unsigned char *)malloc(len);
	    ConvertFromPStringToUnicode(uni_info, n, len, &len, (UniCharArrayPtr)buff);
	    fam_name = "";
	    for(unsigned long x = 0; x < len; x+=2)
		fam_name += QChar(buff[x+1], buff[x]);

	    QtFontFamily *family = foundry->familyDict.find( fam_name.lower() );
	    if ( !family ) {
		family = new QtFontFamily( foundry, fam_name.lower() );
		Q_CHECK_PTR(family);
		foundry->addFamily( family );
	    }

	    FMFontFamilyInstanceIterator fit;
	    if(!FMCreateFontFamilyInstanceIterator(fam, &fit)) {
		FMFont font;
		FMFontStyle font_style;
		FMFontSize font_size;

		while(!FMGetNextFontFamilyInstance(&fit, &font, &font_style, &font_size)) {
		    bool italic = (bool)(font_style & ::italic);
		    int weight = ((font_style & ::bold) ? QFont::Bold : QFont::Normal);

		    if ( italic ) {
			add_style( family, QString::null, italic, FALSE, weight );
		    } else {
			add_style( family, QString::null, italic, FALSE, weight );
			add_style( family, QString::null, italic, TRUE, weight );
		    }
		    if ( weight < QFont::DemiBold ) {
			// Can make bolder
			if ( italic )
			    add_style( family, QString::null, italic, FALSE, QFont::Bold );
			else {
			    add_style( family, QString::null, FALSE, FALSE, QFont::Bold );
			    add_style( family, QString::null, FALSE, TRUE, QFont::Bold );
			}
		    }
		}
		FMDisposeFontFamilyInstanceIterator(&fit);
	    }
	}
	FMDisposeFontFamilyIterator(&it);
    }
}

