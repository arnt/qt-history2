/****************************************************************************
**
** Implementation of platform specific QFontDatabase.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qt_mac.h"
#include "qfontengine_p.h"
#include <stdlib.h>

void QFontDatabase::createDatabase()
{
    if(db)
	return;
    db = new QFontDatabasePrivate;
    qfontdatabase_cleanup.set(&db);

    FMFontFamilyIterator it;
    QString foundry_name = "Mac";
    if(!FMCreateFontFamilyIterator(NULL, NULL, kFMUseGlobalScopeOption, &it)) {
	FMFontFamily fam;
	QString fam_name;
	while(!FMGetNextFontFamily(&it, &fam)) {
	    static Str255 n;
	    if(FMGetFontFamilyName(fam, n) != noErr)
		qDebug("Qt: internal: WH0A, %s %d", __FILE__, __LINE__);
	    if(!n[0] || n[1] == '.') //throw out ones starting with a .
		continue;
#ifndef QMAC_USE_ATSUFONT
	    {
		short fnum;
		ATSUFontID fond;
		GetFNum(n, &fnum);
		if(ATSUFONDtoFontID(fnum, NULL, &fond) != noErr)
		    continue;
	    }
#endif
	    TextEncoding encoding;
	    FMGetFontFamilyTextEncoding(fam, &encoding);
	    TextToUnicodeInfo uni_info;
	    CreateTextToUnicodeInfoByEncoding(encoding, &uni_info);

	    unsigned long len = n[0] * 2;
	    unsigned char *buff = (unsigned char *)malloc(len);
	    ConvertFromPStringToUnicode(uni_info, n, len, &len, (UniCharArrayPtr)buff);
	    fam_name = "";
	    for(unsigned long x = 0; x < len; x+=2)
		fam_name += QChar(buff[x+1], buff[x]);
	    free(buff);
	    DisposeTextToUnicodeInfo(&uni_info);

	    QtFontFamily *family = db->family( fam_name, TRUE );
	    for(int script = 0; script < QFont::LastPrivateScript; ++script)
		family->scripts[script] = QtFontFamily::Supported;
	    QtFontFoundry *foundry = family->foundry( foundry_name, TRUE );

	    FMFontFamilyInstanceIterator fit;
	    if(!FMCreateFontFamilyInstanceIterator(fam, &fit)) {
		FMFont font;
		FMFontStyle font_style;
		FMFontSize font_size;

		while(!FMGetNextFontFamilyInstance(&fit, &font, &font_style, &font_size)) {
		    bool italic = (bool)(font_style & ::italic);
		    int weight = ((font_style & ::bold) ? QFont::Bold : QFont::Normal);
		    QtFontStyle::Key styleKey;
		    styleKey.italic = italic;
		    styleKey.oblique = false;
		    styleKey.weight = weight;

		    QtFontStyle *style = foundry->style( styleKey, TRUE );
		    style->smoothScalable = TRUE;
		    if( !italic ) {
			styleKey.oblique = TRUE;
			style = foundry->style( styleKey, TRUE );
			style->smoothScalable = TRUE;
			styleKey.oblique = FALSE;
		    }
		    if(weight < QFont::DemiBold) {
			// Can make bolder
			styleKey.weight = QFont::Bold;
			if(italic) {
			    style = foundry->style( styleKey, TRUE );
			    style->smoothScalable = TRUE;
			} else {
			    styleKey.oblique = TRUE;
			    style = foundry->style( styleKey, TRUE );
			    style->smoothScalable = TRUE;
			}
		    }
		}
		FMDisposeFontFamilyInstanceIterator(&fit);
	    }
	}
	FMDisposeFontFamilyIterator(&it);
    }
}

static inline void load(const QString & = QString::null,  int = -1 )
{
}
