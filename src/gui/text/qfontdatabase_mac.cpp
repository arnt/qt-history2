/****************************************************************************
**
** Implementation of platform specific QFontDatabase.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
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

int qt_mac_pixelsize(const QFontDef &def, QPaintDevice *pdev); //qfont_mac.cpp
int qt_mac_pointsize(const QFontDef &def, QPaintDevice *pdev); //qfont_mac.cpp

static void initializeDb()
{
    if(db)
        return;
    db = new QFontDatabasePrivate;
    qfontdatabase_cleanup.set(&db);

    FMFontFamilyIterator it;
    QString foundry_name = "ATSUI";
    if(!FMCreateFontFamilyIterator(NULL, NULL, kFMUseGlobalScopeOption, &it)) {
        FMFontFamily fam;
        QString fam_name;
        while(!FMGetNextFontFamily(&it, &fam)) {
            static Str255 n;
            if(FMGetFontFamilyName(fam, n) != noErr)
                qDebug("Qt: internal: WH0A, %s %d", __FILE__, __LINE__);
            if(!n[0] || n[1] == '.') //throw out ones starting with a .
                continue;

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

            //sanity check the font, and see if we can use it at all! --Sam
            if(!ATSFontFindFromName(QCFStringHelper(fam_name), kATSOptionFlagsDefault))
                continue;

            QtFontFamily *family = db->family(fam_name, true);
            for(int script = 0; script < QFont::LastPrivateScript; ++script)
                family->scripts[script] = QtFontFamily::Supported;
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
                    styleKey.italic = italic;
                    styleKey.oblique = false;
                    styleKey.weight = weight;

                    QtFontStyle *style = foundry->style(styleKey, true);
                    style->pixelSize(font_size, true);
                    style->smoothScalable = true;
#if 0
                    if(!italic) {
                        styleKey.oblique = true;
                        style = foundry->style(styleKey, true);
                        style->smoothScalable = true;
                        styleKey.oblique = false;
                    }
                    if(weight < QFont::DemiBold) {
                        // Can make bolder
                        styleKey.weight = QFont::Bold;
                        if(italic) {
                            style = foundry->style(styleKey, true);
                            style->smoothScalable = true;
                        } else {
                            styleKey.oblique = true;
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
QFontEngine *loadEngine(QFont::Script, const QFontPrivate *, const QFontDef &request,
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
            if(ATSFontRef fontref = ATSFontFindFromName(QCFStringHelper(*it),
                                                        kATSOptionFlagsDefault)) {
                QCFStringHelper actualName;
                if(ATSFontGetName(fontref, kATSOptionFlagsDefault, &actualName) == noErr) {
                    if(static_cast<QString>(actualName) == *it) {
                        engine->fontref = fontref;
                        break;
                    }
                }
                if(!engine->fontref) //just take one if it isn't set yet
                    engine->fontref = fontref;
            }
        }
    }
    if(engine->fontref) { //fill in actual name
        QCFStringHelper actualName;
        if(ATSFontGetName(engine->fontref, kATSOptionFlagsDefault, &actualName) == noErr)
            engine->fontDef.family = actualName;
    }
    return engine;
}



