/****************************************************************************
**
** Implementation of QFont/QFontMetrics class for mac.
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

#include "qstring.h"
#include "qfont.h"
#include "qfontdata_p.h"
#include "qfontengine_p.h"
#include "qpaintdevicemetrics.h"
#include "qfontmetrics.h"
#include "qfontinfo.h"
#include "qt_mac.h"
#include "qpaintdevice.h"
#include <qdict.h>
#include <qapplication.h>
#include <qpainter.h>
#include <private/qunicodetables_p.h>
#include <stdlib.h>

QFont::Script QFontPrivate::defaultScript = QFont::UnknownScript;

static inline int qt_mac_pixelsize(const QFontDef &def, QPaintDevice *pdev)
{
    float ret;
    if(def.pixelSize == -1) {
	if(pdev) {
	    ret = def.pointSize *  QPaintDeviceMetrics(pdev).logicalDpiY() / 720.;
	} else {
	    short vr, hr;
	    ScreenRes(&hr, &vr);
	    ret = def.pointSize * vr / 720.;
	}
    } else {
	ret = def.pixelSize;
    }
    return (int)(ret + .5);
}
static inline int qt_mac_pointsize(const QFontDef &def, QPaintDevice *pdev)
{
    float ret;
    if(def.pointSize == -1) {
	if(pdev) {
	    ret = def.pixelSize * 720. / QPaintDeviceMetrics(pdev).logicalDpiY();
	} else {
	    short vr, hr;
	    ScreenRes(&hr, &vr);
	    ret = def.pixelSize * 720. / vr;
	}
    } else {
	ret = def.pointSize;
    }
    return (int)(ret + .5);
}


QMacFontInfo *
QMacSetFontInfo::createFontInfo(const QFontEngine *fe, const QFontDef *def, QPaintDevice *pdev)
{
    Q_ASSERT(fe->type() == QFontEngine::Mac);
    const QFontEngineMac *mac_fe = (QFontEngineMac*)fe;

    QMacFontInfo *ret = new QMacFontInfo();
    //face
    ret->setFont(mac_fe->fnum);

    //style
    short face = normal;
    if(def->italic)
	face |= italic;
    if(def->weight == QFont::Bold)
	face |= bold;
    ret->setStyle(face);

    //size
    int pointSize = qt_mac_pointsize(*def, pdev) / 10;
    ret->setSize(pointSize);

    //encoding
    TextEncoding enc;
    UpgradeScriptInfoToTextEncoding(FontToScript(mac_fe->fnum), kTextLanguageDontCare,
				    kTextRegionDontCare, NULL, &enc);
    ret->setEncoding(enc);

    //Create a cacheable ATSUStyle
    const int arr_guess = 7;
    int arr = 0;
    ATSUAttributeTag tags[arr_guess];
    ByteCount valueSizes[arr_guess];
    ATSUAttributeValuePtr values[arr_guess];
    tags[arr] = kATSUSizeTag; //font size
    Fixed fsize = FixRatio(pointSize, 1);
    valueSizes[arr] = sizeof(fsize);
    values[arr] = &fsize;
    arr++;
    tags[arr] = kATSUFontTag;  //font
    ATSUFontID fond;
    ATSUFONDtoFontID(mac_fe->fnum, NULL, &fond);
    valueSizes[arr] = sizeof(fond);
    values[arr] = &fond;
    arr++;
    tags[arr] = kATSUQDItalicTag;
    valueSizes[arr] = sizeof(Boolean);
    Boolean italicBool = def->italic ? true : false;
    values[arr] = &italicBool;
    arr++;
    tags[arr] = kATSUQDBoldfaceTag;
    valueSizes[arr] = sizeof(Boolean);
    Boolean boldBool = ((def->weight == QFont::Bold) ? true : false);
    values[arr] = &boldBool;
    arr++;
#if QT_MACOSX_VERSION >= 0x1020
    CGAffineTransform tf = CGAffineTransformIdentity;
    if(def->stretch != 100) {
	tf = CGAffineTransformMakeScale(def->stretch/100, 1);
	tags[arr] = kATSUFontMatrixTag;
	valueSizes[arr] = sizeof(tf);
	values[arr] = &tf;
	arr++;
    }
#endif
    if(arr > arr_guess) //this won't really happen, just so I will not miss the case
	qDebug("Qt: internal: %d, WH0A %d: arr_guess overflow", __LINE__, arr);

    //create style
    QATSUStyle *st = new QATSUStyle;
    st->rgb.red = st->rgb.green = st->rgb.blue = 0;
    ATSUCreateStyle(&st->style);
    if(OSStatus e = ATSUSetAttributes(st->style, arr, tags, valueSizes, values)) {
	qDebug("Qt: internal: %ld: unexpected condition reached %s:%d", e, __FILE__, __LINE__);
	delete st;
	st = NULL;
    } else {
	const int feat_guess=5;
	int feats=0;
	ATSUFontFeatureType feat_types[feat_guess];
	ATSUFontFeatureSelector feat_values[feat_guess];
	feat_types[feats] = kLigaturesType;
	feat_values[feats] = kRareLigaturesOffSelector;
	feats++;
	feat_types[feats] = kLigaturesType;
	feat_values[feats] = kCommonLigaturesOffSelector;
	feats++;
	if(feats > feat_guess) //this won't really happen, just so I will not miss the case
	    qDebug("Qt: internal: %d: WH0A feat_guess underflow %d", __LINE__, feats);
	if(OSStatus e = ATSUSetFontFeatures(st->style, feats, feat_types, feat_values))
	    qDebug("Qt: internal: %ld: unexpected condition reached %s:%d", e, __FILE__, __LINE__);
    }
    ret->setATSUStyle(st);
    return ret;
}

bool
QMacSetFontInfo::setMacFont(const QFontEngine *fe, QMacSetFontInfo *sfi, QPaintDevice *pdev)
{
    Q_ASSERT(fe->type() == QFontEngine::Mac);
    QFontEngineMac *mac_fe = (QFontEngineMac*)fe;
    if(!mac_fe->internal_fi) {
	mac_fe->internal_fi = createFontInfo(fe, &fe->fontDef, pdev);
	mac_fe->calculateCost();
    }
    return setMacFont(mac_fe->internal_fi, sfi);
}

bool
QMacSetFontInfo::setMacFont(const QFontPrivate *d, QMacSetFontInfo *sfi, QPaintDevice *pdev)
{
    QFontEngine *eng = d->engineForScript( QFont::NoScript );
    Q_ASSERT( eng->type() == QFontEngine::Mac );
    QFontEngineMac *engine = (QFontEngineMac *) eng;
    QMacFontInfo *fi = engine->internal_fi;
    if(!fi) {
	engine->internal_fi = fi = createFontInfo(engine, &eng->fontDef, pdev);
	engine->calculateCost();
    }
    return setMacFont(fi, sfi);
}

bool
QMacSetFontInfo::setMacFont(const QMacFontInfo *fi, QMacSetFontInfo *sfi)
{
    if(!sfi || fi->font() != sfi->tfont)
	TextFont(fi->font());
    if(!sfi || fi->style() != sfi->tface)
	TextFace(fi->style());
    if(!sfi || fi->size() != sfi->tsize)
	TextSize(fi->size());
    if(sfi)
	*((QMacFontInfo*)sfi) = *(fi);
    return TRUE;
}

/* utility functions */
QByteArray p2qstring(const unsigned char *c); //qglobal.cpp
static inline void qstring_to_pstring(QString s, int len, Str255 str, TextEncoding encoding)
{
    UnicodeMapping mapping;
    UnicodeToTextInfo info;
    mapping.unicodeEncoding = CreateTextEncoding(kTextEncodingUnicodeDefault,
						  kTextEncodingDefaultVariant,
						 kUnicode16BitFormat);
    mapping.otherEncoding = encoding;
    mapping.mappingVersion = kUnicodeUseLatestMapping;

    if(CreateUnicodeToTextInfo(&mapping, &info) != noErr) {
	qDebug("Qt: internal: Unexpected condition reached %s:%d", __FILE__, __LINE__);
	return;
    }
    const int unilen = len * 2;
    const UniChar *unibuf = (UniChar *)s.unicode();
    ConvertFromUnicodeToPString(info, unilen, unibuf, str);
    DisposeUnicodeToTextInfo(&info);
}

/* Qt platform dependant functions */

int QFontMetrics::width(QChar c) const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
    Q_ASSERT( engine != 0 );
    Q_ASSERT( engine->type() == QFontEngine::Mac );

    return ((QFontEngineMac*) engine)->doTextTask(&c, 0, 1, 1,
						  QFontEngineMac::WIDTH);
}

int QFontMetrics::charWidth(const QString &str, int pos) const
{
    QTextEngine layout(str, d);
    layout.itemize(QTextEngine::WidthOnly);
    return layout.width(pos, 1);
}

QString QFont::rawName() const
{
    return family();
}

void QFont::setRawName(const QString &name)
{
    setFamily(name);
}

void QFont::cleanup()
{
    delete QFontCache::instance;
}

// Return a font family ID
Qt::HANDLE QFont::handle() const
{
    if ( ! d->engineData )
        d->load( QFont::NoScript );
    if (d->engineData && d->engineData->engine)
	return (Qt::HANDLE)((UInt32)((QFontEngineMac*)d->engineData->engine)->fnum);
    return 0;
}

void QFont::macSetFont(QPaintDevice *v)
{
    QMacSavedPortInfo::setPaintDevice(v);
    QMacSetFontInfo::setMacFont(d, NULL, v);
}

void QFontPrivate::load(QFont::Script script)
{
    // sanity checks
    if (!QFontCache::instance)
	qWarning("Must construct a QApplication before a QFont");
    Q_ASSERT(script >= 0 && script < QFont::LastPrivateScript);

    QFontEngineMac *engine = NULL;

    QFontDef req = request;
    req.pixelSize = qt_mac_pixelsize(request, paintdevice);
    req.pointSize = 0;
    QFontCache::Key key = QFontCache::Key(req, QFont::NoScript, screen);

    if(!(engineData = QFontCache::instance->findEngineData(key))) {
	engineData = new QFontEngineData;
	QFontCache::instance->insertEngineData(key, engineData);
    } else {
	engineData->ref();
    }

    if ( engineData->engine ) // already loaded
	return;

    if(QFontEngine *e = QFontCache::instance->findEngine(key)) {
	Q_ASSERT(e->type() == QFontEngine::Mac);
	e->ref();
	engineData->engine = e;
	engine = (QFontEngineMac*)e;
	return; // the font info and fontdef should already be filled
    }

    engineData->engine = engine = new QFontEngineMac;
    if(engine->fnum == -1) {
	//find the font
	QStringList family_list = QStringList::split( ',', request.family );
	// append the substitute list for each family in family_list
	{
	    QStringList subs_list;
	    QStringList::ConstIterator it = family_list.begin(), end = family_list.end();
	    for ( ; it != end; ++it )
		subs_list += QFont::substitutes( *it );
	    family_list += subs_list;
	}
	// add QFont::defaultFamily() to the list, for compatibility with
	// previous versions
	family_list << QApplication::font().defaultFamily();
	for(QStringList::ConstIterator it = family_list.begin(); it !=  family_list.end(); ++it) {
	    Str255 request_str, actual_str;
	    // encoding == 1, yes it is strange the names of fonts are encoded in MacJapanese
	    TextEncoding encoding = CreateTextEncoding(kTextEncodingMacJapanese,
						       kTextEncodingDefaultVariant,
						       kTextEncodingDefaultFormat);
	    qstring_to_pstring((*it), (*it).length(), request_str, encoding);

#if 0
	    short fnum = FMGetFontFamilyFromName(request_str);
#else
	    short fnum;
	    GetFNum(request_str, &fnum);
#endif
	    GetFontName(fnum, actual_str);
	    if(actual_str[0] == request_str[0] &&
	       !strncasecmp((const char *)actual_str+1, (const char *)request_str+1, actual_str[0])) {
		engine->fnum = fnum;
		break;
	    } else if(engine->fnum == -1) {
		engine->fnum = fnum; //take the first one for now..
	    }
	}
    }
    { //fill in the engine's font definition
	engineData->engine->fontDef = request; //copy..
	if(engineData->engine->fontDef.pointSize == -1)
	    engineData->engine->fontDef.pointSize = qt_mac_pointsize(engineData->engine->fontDef, paintdevice);
	else
	    engineData->engine->fontDef.pixelSize = qt_mac_pixelsize(engineData->engine->fontDef, paintdevice);
	Str255 font;
	Q_ASSERT(engineData->engine->type() == QFontEngine::Mac);
	GetFontName(((QFontEngineMac*)engineData->engine)->fnum, font);
	engineData->engine->fontDef.family = p2qstring(font);
    }
    if(!engine->info) {
#if 0
	engine->info = (ATSFontMetrics*)malloc(sizeof(ATSFontMetrics));
	const unsigned char *p = p_str(request.family);
	ATSFontGetVerticalMetrics(ATSFontFamilyFindFromQuickDrawName(p),
				  kATSOptionFlagsDefault, engine->info);
	free(p);
#else
	engine->info = (FontInfo *)malloc(sizeof(FontInfo));
	QMacSetFontInfo fi(this, paintdevice);
	GetFontInfo(engine->info);
	engine->calculateCost();
#endif
    }

    engine->ref();
    QFontCache::instance->insertEngine(key, engine);
}

void QFont::initialize()
{
    if(!QFontCache::instance)
        new QFontCache();
}

bool QFont::dirty() const
{
    return d->engineData == 0;
}

QString QFont::defaultFamily() const
{
    switch(d->request.styleHint) {
	case QFont::Times:
	    return QString::fromLatin1("Times New Roman");
	case QFont::Courier:
	    return QString::fromLatin1("Courier New");
	case QFont::Decorative:
	    return QString::fromLatin1("Bookman Old Style");
	case QFont::Helvetica:
	case QFont::System:
	default:
	    return QString::fromLatin1("Helvetica");
    }
}

QString QFont::lastResortFamily() const
{
    return QString::fromLatin1("Helvetica");
}

QString QFont::lastResortFont() const
{
    return QString::fromLatin1("Arial");
}
