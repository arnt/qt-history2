/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <private/qt_mac_p.h>
#include "qfontengine_p.h"
#include <stdlib.h>

int qt_mac_pixelsize(const QFontDef &def, int dpi); //qfont_mac.cpp
int qt_mac_pointsize(const QFontDef &def, int dpi); //qfont_mac.cpp

static void initializeDb()
{
    QFontDatabasePrivate *db = privateDb();
    if (!db || db->count)
        return;

    FMFontFamilyIterator it;
    QString foundry_name = "ATSUI";
    if(!FMCreateFontFamilyIterator(NULL, NULL, kFMUseGlobalScopeOption, &it)) {
        FMFontFamily fam;
        QString fam_name;
        while(!FMGetNextFontFamily(&it, &fam)) {
            { //sanity check the font, and see if we can use it at all! --Sam
                ATSUFontID fontID;
                if(ATSUFONDtoFontID(fam, 0, &fontID) != noErr)
                    continue;
            }

            static Str255 fam_pstr;
            if(FMGetFontFamilyName(fam, fam_pstr) != noErr)
                qDebug("Qt: internal: WH0A, %s %d", __FILE__, __LINE__);
            if(!fam_pstr[0] || fam_pstr[1] == '.') //throw out ones starting with a .
                continue;

            TextEncoding encoding;
            FMGetFontFamilyTextEncoding(fam, &encoding);
            TextToUnicodeInfo uni_info;
            CreateTextToUnicodeInfoByEncoding(encoding, &uni_info);

            unsigned long len = fam_pstr[0] * 2;
            unsigned char *buff = (unsigned char *)malloc(len);
            ConvertFromPStringToUnicode(uni_info, fam_pstr, len, &len, (UniCharArrayPtr)buff);
            fam_name = "";
            for(unsigned long x = 0; x < len; x+=2) {
#if defined(__i386__)
		fam_name += QChar(buff[x], buff[x+1]);
#else
		fam_name += QChar(buff[x+1], buff[x]);
#endif
            }
            free(buff);
            DisposeTextToUnicodeInfo(&uni_info);

            QtFontFamily *family = db->family(fam_name, true);
            // ###
            for(int ws = 1; ws < QFontDatabase::WritingSystemsCount; ++ws)
                family->writingSystems[ws] = QtFontFamily::Supported;
            QtFontFoundry *foundry = family->foundry(foundry_name, true);

            FMFontFamilyInstanceIterator fit;
            if(!FMCreateFontFamilyInstanceIterator(fam, &fit)) {
                FMFont font;
                FMFontStyle font_style;
                FMFontSize font_size;

                while(!FMGetNextFontFamilyInstance(&fit, &font, &font_style, &font_size)) {
                    bool italic = (bool)(font_style & ::italic);
                    int weight = ((font_style & ::bold) ? QFont::Bold : QFont::Normal);
                    QtFontStyle::Key styleKey;
                    styleKey.style = italic ? QFont::StyleItalic : QFont::StyleNormal;
                    styleKey.weight = weight;

                    QtFontStyle *style = foundry->style(styleKey, true);
                    style->pixelSize(font_size, true);
                    style->smoothScalable = true;
#if 0
                    if(!italic) {
                        styleKey.style = QFont::StyleOblique;
                        style = foundry->style(styleKey, true);
                        style->smoothScalable = true;
                        styleKey.style = QFont::StyleNormal;
                    }
                    if(weight < QFont::DemiBold) {
                        // Can make bolder
                        styleKey.weight = QFont::Bold;
                        if(italic) {
                            style = foundry->style(styleKey, true);
                            style->smoothScalable = true;
                        } else {
                            styleKey.style = QFont::StyleOblique;
                            style = foundry->style(styleKey, true);
                            style->smoothScalable = true;
                        }
                    }
#endif
                }
                FMDisposeFontFamilyInstanceIterator(&fit);
            }
        }
        FMDisposeFontFamilyIterator(&it);
    }
}

static inline void load(const QString & = QString(), int = -1)
{
}

void QFontDatabase::load(const QFontPrivate *d, int script)
{
    // sanity checks
    if (!QFontCache::instance)
        qWarning("QFont: Must construct a QApplication before a QFont");
    Q_ASSERT(script >= 0 && script < QUnicodeTables::ScriptCount);
    Q_UNUSED(script);

    QFontDef req = d->request;
    req.pixelSize = qt_mac_pixelsize(req, d->dpi);

    // set the point size to 0 to get better caching
    req.pointSize = 0;
    QFontCache::Key key = QFontCache::Key(req, QUnicodeTables::Common, d->screen);

    if(!(d->engineData = QFontCache::instance->findEngineData(key))) {
        d->engineData = new QFontEngineData;
        QFontCache::instance->insertEngineData(key, d->engineData);
    } else {
        d->engineData->ref.ref();
    }
    if(d->engineData->engine) // already loaded
        return;

    // set it to the actual pointsize, so QFontInfo will do the right thing
    req.pointSize = qRound(qt_mac_pointsize(d->request, d->dpi));

    if(QFontEngine *e = QFontCache::instance->findEngine(key)) {
        Q_ASSERT(e->type() == QFontEngine::Multi);
        e->ref.ref();
        d->engineData->engine = e;
        return; // the font info and fontdef should already be filled
    }

    ATSFontFamilyRef familyRef = 0;

    //find the font
    QStringList family_list = req.family.split(',');
    // append the substitute list for each family in family_list
    {
	    QStringList subs_list;
	    QStringList::ConstIterator it = family_list.begin(), end = family_list.end();
	    for (; it != end; ++it)
		    subs_list += QFont::substitutes(*it);
	    family_list += subs_list;
    }
    // add QFont::defaultFamily() to the list, for compatibility with
    // previous versions
    family_list << QApplication::font().defaultFamily();

    //find it!
    QHash<QString, ATSFontFamilyRef> mac_families;
    {
	ATSFontFamilyIterator iterator;
	if(!ATSFontFamilyIteratorCreate(kATSFontContextGlobal, 0, 0,
		                       kATSOptionFlagsRestrictedScope, &iterator)) {
	    for(ATSFontFamilyRef family; ATSFontFamilyIteratorNext(iterator, &family) == noErr;) {
		QCFString actualName;
		if(ATSFontFamilyGetName(family, kATSOptionFlagsDefault, &actualName) == noErr)
		    mac_families.insert(static_cast<QString>(actualName).toLower(), family);
	    }
	    ATSFontFamilyIteratorRelease(&iterator);
	}
    }
    for(QStringList::ConstIterator it = family_list.constBegin(); it !=  family_list.constEnd(); ++it) {
	if(mac_families.contains((*it).toLower())) {
	    familyRef = mac_families.value((*it).toLower());
	    break;
	}
	if(ATSFontFamilyRef family = ATSFontFamilyFindFromName(QCFString(*it), kATSOptionFlagsDefault)) {
	    QCFString actualName;
	    if(ATSFontFamilyGetName(family, kATSOptionFlagsDefault, &actualName) == noErr) {
		if(static_cast<QString>(actualName) == (*it)) {
		    familyRef = family;
		    break;
		}
	    }
	    familyRef = family; //just take one if it isn't set yet
	}
    }

    //fill in the engine's font definition
    QFontDef fontDef = d->request; //copy..
    if(fontDef.pointSize < 0)
	fontDef.pointSize = qt_mac_pointsize(fontDef, d->dpi);
    else
	fontDef.pixelSize = qt_mac_pixelsize(fontDef, d->dpi);
    {
	QCFString actualName;
	if (ATSFontFamilyGetName(familyRef, kATSOptionFlagsDefault, &actualName) == noErr)
	    fontDef.family = actualName;
    }

    QFontEngine *engine = new QFontEngineMacMulti(familyRef, fontDef, d->kerning);
    d->engineData->engine = engine;
    engine->ref.ref(); //a ref for the engineData->engine

    QFontCache::instance->insertEngine(key, engine);
}

