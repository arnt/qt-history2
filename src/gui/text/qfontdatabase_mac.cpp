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
            for(unsigned long x = 0; x < len; x+=2)
                fam_name += QChar(buff[x+1], buff[x]);
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

static inline void load(const QString & = QString::null,  int = -1)
{
}

static
QFontEngine *loadEngine(int, const QFontPrivate *, const QFontDef &request,
                        QtFontFamily *family, QtFontFoundry *, QtFontStyle *)
{
    QFontEngineMac *engine = new QFontEngineMac;
    { //find the font
        QStringList family_list;
        if(family)
            family_list += family->name;
        family_list += request.family;
        {   // append the substitute list for each family in family_list
            QStringList subs_list;
            QStringList::ConstIterator it = family_list.begin(), end = family_list.end();
            for (; it != end; ++it)
                subs_list += QFont::substitutes(*it);
            family_list += subs_list;
        }
        family_list << QApplication::font().defaultFamily();         // add defaultFamily (compatibility)
        for(QStringList::ConstIterator it = family_list.begin(); it !=  family_list.end(); ++it) {
            if(ATSFontFamilyRef familyref = ATSFontFamilyFindFromName(QCFString(*it),
                                                                      kATSOptionFlagsDefault)) {
                QCFString actualName;
                if(ATSFontFamilyGetName(familyref, kATSOptionFlagsDefault, &actualName) == noErr) {
                    if(static_cast<QString>(actualName) == *it) {
                        engine->familyref = familyref;
                        break;
                    }
                }
                if(!engine->familyref) //just take one if it isn't set yet
                    engine->familyref = familyref;
            }
        }
    }
    if(engine->familyref) { //fill in actual name
        QCFString actualName;
        if(ATSFontFamilyGetName(engine->familyref, kATSOptionFlagsDefault, &actualName) == noErr)
            engine->fontDef.family = actualName;
    }
    return engine;
}
