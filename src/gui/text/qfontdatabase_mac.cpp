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
#include <qendian.h>

int qt_mac_pixelsize(const QFontDef &def, int dpi); //qfont_mac.cpp
int qt_mac_pointsize(const QFontDef &def, int dpi); //qfont_mac.cpp

static void initWritingSystems(QtFontFamily *family, ATSFontRef atsFont)
{
    ByteCount length = 0;
    if (ATSFontGetTable(atsFont, MAKE_TAG('O', 'S', '/', '2'), 0, 0, 0, &length) != noErr)
        return;
    QVarLengthArray<uchar> os2Table(length);
    if (length < 86
        || ATSFontGetTable(atsFont, MAKE_TAG('O', 'S', '/', '2'), 0, length, os2Table.data(), &length) != noErr)
        return;

    quint32 unicodeRange[4] = {
        qFromBigEndian<quint32>(os2Table.data() + 42),
        qFromBigEndian<quint32>(os2Table.data() + 46),
        qFromBigEndian<quint32>(os2Table.data() + 50),
        qFromBigEndian<quint32>(os2Table.data() + 54)
    };
    quint32 codePageRange[2] = { qFromBigEndian<quint32>(os2Table.data() + 78), qFromBigEndian<quint32>(os2Table.data() + 82) };
    QList<QFontDatabase::WritingSystem> systems = determineWritingSystemsFromTrueTypeBits(unicodeRange, codePageRange);
#if 0
    QCFString name;
    ATSFontGetName(atsFont, kATSOptionFlagsDefault, &name);
    qDebug() << systems.count() << "writing systems for" << QString(name);
qDebug() << "first char" << hex << unicodeRange[0];
    for (int i = 0; i < systems.count(); ++i)
        qDebug() << QFontDatabase::writingSystemName(systems.at(i));
#endif
    for (int i = 0; i < systems.count(); ++i)
        family->writingSystems[systems.at(i)] = QtFontFamily::Supported;
}

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
        if(QCFType<CFDictionaryRef> styles = (CFDictionaryRef)CTFontDescriptorCopyAttribute(font, kCTFontTraitsAttribute)) {
            if(CFNumberRef weight = (CFNumberRef)CFDictionaryGetValue(styles, kCTFontWeightTrait)) {
                Q_ASSERT(CFNumberIsFloatType(weight));
                double d;
                if(CFNumberGetValue(weight, kCFNumberDoubleType, &d)) {
                    //qDebug() << "BOLD" << (QString)family_name << d;
                    styleKey.weight = (d > 0.0) ? QFont::Bold : QFont::Normal;
                }
            }
            if(CFNumberRef italic = (CFNumberRef)CFDictionaryGetValue(styles, kCTFontSlantTrait)) {
                Q_ASSERT(CFNumberIsFloatType(italic));
                double d;
                if(CFNumberGetValue(italic, kCFNumberDoubleType, &d)) {
                    //qDebug() << "ITALIC" << (QString)family_name << d;
                    if (d > 0.0)
                        styleKey.style = QFont::StyleItalic;
                }
            }
        }

        QtFontStyle *style = foundry->style(styleKey, true);
        style->smoothScalable = true;
        if(QCFType<CFNumberRef> size = (CFNumberRef)CTFontDescriptorCopyAttribute(font, kCTFontSizeAttribute)) {
            //qDebug() << "WHEE";
            int pixel_size=0;
            if(CFNumberIsFloatType(size)) {
                double d;
                CFNumberGetValue(size, kCFNumberDoubleType, &d);
                pixel_size = d;
            } else {
                CFNumberGetValue(size, kCFNumberIntType, &pixel_size);
            }
            //qDebug() << "SIZE" << (QString)family_name << pixel_size;
            if(pixel_size)
                style->pixelSize(pixel_size, true);
        } else {
            //qDebug() << "WTF?";
        }
    }
#else
    FMFontIterator it;
    if (!FMCreateFontIterator(0, 0, kFMUseGlobalScopeOption, &it)) {
        while (true) {
            FMFont fmFont;
            if (FMGetNextFont(&it, &fmFont) != noErr)
                break;

            FMFontFamily fmFamily;
            FMFontStyle fmStyle;
            QString familyName;

            QtFontStyle::Key styleKey;

            ATSFontRef atsFont = FMGetATSFontRefFromFont(fmFont);

            if (!FMGetFontFamilyInstanceFromFont(fmFont, &fmFamily, &fmStyle)) {
                { //sanity check the font, and see if we can use it at all! --Sam
                    ATSUFontID fontID;
                    if(ATSUFONDtoFontID(fmFamily, 0, &fontID) != noErr)
                        continue;
                }

                if (fmStyle & ::italic)
                    styleKey.style = QFont::StyleItalic;
                if (fmStyle & ::bold)
                    styleKey.weight = QFont::Bold;

                ATSFontFamilyRef familyRef = FMGetATSFontFamilyRefFromFontFamily(fmFamily);
                QCFString cfFamilyName;;
                ATSFontFamilyGetName(familyRef, kATSOptionFlagsDefault, &cfFamilyName);
                familyName = cfFamilyName;
            } else {
                QCFString cfFontName;
                ATSFontGetName(atsFont, kATSOptionFlagsDefault, &cfFontName);
                familyName = cfFontName;
                quint16 macStyle = 0;
                {
                    uchar data[4];
                    ByteCount len = 4;
                    if (ATSFontGetTable(atsFont, MAKE_TAG('h', 'e', 'a', 'd'), 44, 4, &data, &len) == noErr)
                        macStyle = qFromBigEndian<quint16>(data);
                }
                if (macStyle & 1)
                    styleKey.weight = QFont::Bold;
                if (macStyle & 2)
                    styleKey.style = QFont::StyleItalic;
            }

            QtFontFamily *family = db->family(familyName, true);
            QtFontFoundry *foundry = family->foundry(QString(), true);
            QtFontStyle *style = foundry->style(styleKey, true);
            style->pixelSize(0, true);
            style->smoothScalable = true;

            initWritingSystems(family, atsFont);
        }
        FMDisposeFontIterator(&it);
    }
#endif
}

static inline void load(const QString & = QString(), int = -1)
{
    initializeDb();
}

static const char *styleHint(const QFontDef &request)
{
    const char *stylehint = 0;
    switch (request.styleHint) {
    case QFont::SansSerif:
        stylehint = "Arial";
        break;
    case QFont::Serif:
        stylehint = "Times New Roman";
        break;
    case QFont::TypeWriter:
        stylehint = "Courier New";
        break;
    default:
        if (request.fixedPitch)
            stylehint = "Courier New";
        break;
    }
    return stylehint;
}

void QFontDatabase::load(const QFontPrivate *d, int script)
{
    // sanity checks
    if(!qApp)
        qWarning("QFont: Must construct a QApplication before a QFont");
    Q_ASSERT(script >= 0 && script < QUnicodeTables::ScriptCount);
    Q_UNUSED(script);

    QFontDef req = d->request;
    req.pixelSize = qt_mac_pixelsize(req, d->dpi);

    // set the point size to 0 to get better caching
    req.pointSize = 0;
    QFontCache::Key key = QFontCache::Key(req, QUnicodeTables::Common, d->screen);

    if(!(d->engineData = QFontCache::instance()->findEngineData(key))) {
        d->engineData = new QFontEngineData;
        QFontCache::instance()->insertEngineData(key, d->engineData);
    } else {
        d->engineData->ref.ref();
    }
    if(d->engineData->engine) // already loaded
        return;

    // set it to the actual pointsize, so QFontInfo will do the right thing
    req.pointSize = qRound(qt_mac_pointsize(d->request, d->dpi));

    QFontEngine *e = QFontCache::instance()->findEngine(key);
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

    //find the font
    QStringList family_list = req.family.split(QLatin1Char(','));
    // append the substitute list for each family in family_list
    {
	    QStringList subs_list;
	    for(QStringList::ConstIterator it = family_list.begin(); it != family_list.end(); ++it)
		    subs_list += QFont::substitutes(*it);
	    family_list += subs_list;
    }

    const char *stylehint = styleHint(req);
    if (stylehint)
        family_list << QLatin1String(stylehint);

    // add QFont::defaultFamily() to the list, for compatibility with
    // previous versions
    family_list << QApplication::font().defaultFamily();

    ATSUFontID fontID = 0;
    QFontDatabasePrivate *db = privateDb();
    if (!db->count)
        initializeDb();
    for(int i = 0; i < family_list.size(); ++i) {
        bool found = false;
        for (int k = 0; k < db->count; ++k) {
            if (db->families[k]->name.compare(family_list.at(i), Qt::CaseInsensitive) == 0) {
                QByteArray family_name = db->families[k]->name.toUtf8();
                if(ATSFontRef atsfontref = ATSFontFindFromName(QCFString(db->families[k]->name),
                                                               kATSOptionFlagsDefault)) {
                    found = true;
                    fontID = FMGetFontFromATSFontRef(atsfontref);
                    break;
                }
            }
        }
        if (found)
            break;
    }

    //fill in the engine's font definition
    QFontDef fontDef = d->request; //copy..
    if(fontDef.pointSize < 0)
	fontDef.pointSize = qt_mac_pointsize(fontDef, d->dpi);
    else
	fontDef.pixelSize = qt_mac_pixelsize(fontDef, d->dpi);
#if 0
    ItemCount name_count;
    if(ATSUCountFontNames(fontID, &name_count) == noErr && name_count) {
        ItemCount actualName_size;
        if(ATSUGetIndFontName(fontID, 0, 0, 0, &actualName_size, 0, 0, 0, 0) == noErr && actualName_size) {
            QByteArray actualName(actualName_size);
            if(ATSUGetIndFontName(fontID, 0, actualName_size, actualName.data(), &actualName_size, 0, 0, 0, 0) == noErr && actualName_size)
                fontDef.family = QString::fromUtf8(actualName);
        }
    }
#else
    {
        QCFString actualName;
        if(ATSFontGetName(FMGetATSFontRefFromFont(fontID), kATSOptionFlagsDefault, &actualName) == noErr)
            fontDef.family = actualName;
    }
#endif

    QFontEngine *engine = new QFontEngineMacMulti(fontID, fontDef, d->kerning);
    d->engineData->engine = engine;
    engine->ref.ref(); //a ref for the engineData->engine
    QFontCache::instance()->insertEngine(key, engine);
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

