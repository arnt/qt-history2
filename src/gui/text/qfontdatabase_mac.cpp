/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <private/qt_mac_p.h>
#include "qfontengine_p.h"
#include <qfile.h>
#include <qabstractfileengine.h>
#include <stdlib.h>

int qt_mac_pixelsize(const QFontDef &def, int dpi); //qfont_mac.cpp
int qt_mac_pointsize(const QFontDef &def, int dpi); //qfont_mac.cpp

static void initializeDb()
{
    QFontDatabasePrivate *db = privateDb();
    if(!db || db->count)
        return;

#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5)
    QCFType<CTFontCollectionRef> collection = CTFontCollectionCreateFromAvailableFonts(0);
    if(!collection)
        return;
    QCFType<CFArrayRef> fonts = CTFontCollectionCreateMatchingFontDescriptors(collection);
    if(!fonts)
        return;
    QString foundry_name = "CoreText";
    const int numFonts = CFArrayGetCount(fonts);
    for(int i = 0; i < numFonts; ++i) {
        CTFontDescriptorRef font = (CTFontDescriptorRef)CFArrayGetValueAtIndex(fonts, i);

        QCFString family_name = (CFStringRef)CTFontDescriptorCopyAttribute(font, kCTFontFamilyNameAttribute);
        QtFontFamily *family = db->family(family_name, true);
        for(int ws = 1; ws < QFontDatabase::WritingSystemsCount; ++ws)
            family->writingSystems[ws] = QtFontFamily::Supported;
        QtFontFoundry *foundry = family->foundry(foundry_name, true);

        QtFontStyle::Key styleKey;
        if(QCFType<CFDictionaryRef> styles = (CFDictionaryRef)CTFontDescriptorCopyAttribute(font, kCTFontStyleNameAttribute)) {
            if(QCFType<CFNumberRef> traits = (CFNumberRef)CFDictionaryGetValue(styles, kCTFontSymbolicTrait)) {
                int i;
                if(CFNumberGetValue(traits, kCFNumberIntType, &i)) {
                    styleKey.style = (bool)(i & kCTFontItalicTrait);
                    styleKey.weight = (i & kCTFontBoldTrait) ? QFont::Bold : QFont::Normal;
                }
            }
        }

        QtFontStyle *style = foundry->style(styleKey, true);
        style->smoothScalable = true;
        if(QCFType<CFNumberRef> size = (CFNumberRef)CTFontDescriptorCopyAttribute(font, kCTFontSizeAttribute)) {
            int i;
            if(CFNumberGetValue(size, kCFNumberIntType, &i))
                style->pixelSize(i, true);
        }
    }
#else
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

            ATSFontFamilyRef familyRef = FMGetATSFontFamilyRefFromFontFamily(fam);
            QCFString familyStr;
            ATSFontFamilyGetName(familyRef, kATSOptionFlagsDefault, &familyStr);
            fam_name = familyStr;

            QtFontFamily *family = db->family(fam_name, true);
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
                }
                FMDisposeFontFamilyInstanceIterator(&fit);
            }
        }
        FMDisposeFontFamilyIterator(&it);
    }
#endif
}

static inline void load(const QString & = QString(), int = -1)
{
    initializeDb();
}

void QFontDatabase::load(const QFontPrivate *d, int script)
{
    // sanity checks
    if(!QFontCache::instance)
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

    QFontEngine *e = QFontCache::instance->findEngine(key);
    if(!e && qt_enable_test_font && req.family == QLatin1String("__Qt__Box__Engine__")) {
        e = new QTestFontEngine(req.pixelSize);
        e->fontDef = req;
    }

    if(e) {
        Q_ASSERT(e->type() == QFontEngine::Multi || e->type() == QFontEngine::TestFontEngine);
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
	    for(; it != end; ++it)
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
	if(ATSFontFamilyGetName(familyRef, kATSOptionFlagsDefault, &actualName) == noErr)
	    fontDef.family = actualName;
    }

    QFontEngine *engine = new QFontEngineMacMulti(familyRef, fontDef, d->kerning);
    d->engineData->engine = engine;
    engine->ref.ref(); //a ref for the engineData->engine

    QFontCache::instance->insertEngine(key, engine);
}

static void registerFont(QFontDatabasePrivate::ApplicationFont *fnt)
{
    ATSFontContainerRef handle;
    OSStatus e;

    if(fnt->data.isEmpty()) {
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5)
        extern OSErr qt_mac_create_fsref(const QString &, FSRef *); // qglobal.cpp
        FSRef ref;
        if(qt_mac_create_fsref(fnt->fileName, &ref) != noErr)
            return;

        ATSFontActivateFromFileReference(&ref, kATSFontContextLocal, kATSFontFormatUnspecified, 0, kATSOptionFlagsDefault, &handle);
#else
        extern Q_CORE_EXPORT OSErr qt_mac_create_fsspec(const QString &, FSSpec *); // global.cpp
        FSSpec spec;
        if(qt_mac_create_fsspec(fnt->fileName, &spec) != noErr)
            return;

        e = ATSFontActivateFromFileSpecification(&spec, kATSFontContextLocal, kATSFontFormatUnspecified,
                                           0, kATSOptionFlagsDefault, &handle);
#endif
    } else {
        e = ATSFontActivateFromMemory((void *)fnt->data.constData(), fnt->data.size(), kATSFontContextLocal,
                                           kATSFontFormatUnspecified, 0, kATSOptionFlagsDefault, &handle);

        fnt->data = QByteArray();
    }

    if(e != noErr)
        return;

    ItemCount fontCount = 0;
    e = ATSFontFindFromContainer(handle, kATSOptionFlagsDefault, 0, 0, &fontCount);
    if(e != noErr)
        return;

    QVarLengthArray<ATSFontRef> containedFonts(fontCount);
    e = ATSFontFindFromContainer(handle, kATSOptionFlagsDefault, fontCount, containedFonts.data(), &fontCount);
    if(e != noErr)
        return;

    fnt->families.clear();
    for(int i = 0; i < containedFonts.size(); ++i) {
        QCFString family;
        ATSFontGetName(containedFonts[i], kATSOptionFlagsDefault, &family);
        fnt->families.append(family);
    }

    fnt->handle = handle;
}

bool QFontDatabase::removeApplicationFont(int handle)
{
    QFontDatabasePrivate *db = privateDb();
    if(handle < 0 || handle >= db->applicationFonts.count())
        return false;

    OSStatus e = ATSFontDeactivate(db->applicationFonts.at(handle).handle,
                                   /*iRefCon=*/0, kATSOptionFlagsDefault);
    if(e != noErr)
        return false;

    db->applicationFonts[handle] = QFontDatabasePrivate::ApplicationFont();

    db->invalidate();
    return true;
}

bool QFontDatabase::removeAllApplicationFonts()
{
    QFontDatabasePrivate *db = privateDb();
    for(int i = 0; i < db->applicationFonts.count(); ++i) {
        if(!removeApplicationFont(i))
            return false;
    }
    return true;
}

