/****************************************************************************
** $Id$
**
** Implementation of QFont/QFontMetrics class for mac
**
** Created : 001019
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
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
#include <stdlib.h>

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
    int logicalDpi = 80; //FIXME
    if(pdev) {
	QPaintDeviceMetrics pm(pdev);
	logicalDpi = pm.logicalDpiY();
    } else {
	short vr, hr;
	ScreenRes(&hr, &vr);
	logicalDpi = vr;
    }
    int pointSize = ((def->pointSize != -1) ? (def->pointSize / 10) : (def->pixelSize * logicalDpi /72)); 
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
#ifdef MACOSX_102
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
	int feat_guess=5, feats=0;
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
    if(!mac_fe->internal_fi) 
	mac_fe->internal_fi = createFontInfo(fe, &mac_fe->fdef, pdev);
    return setMacFont(mac_fe->internal_fi, sfi);
}

bool 
QMacSetFontInfo::setMacFont(const QFontPrivate *d, QMacSetFontInfo *sfi, QPaintDevice *pdev)
{
    ((QFontPrivate *)d)->load();
    Q_ASSERT(d->fin->type() == QFontEngine::Mac);
    QMacFontInfo *fi = ((QFontEngineMac*)d->fin)->internal_fi;
    if(!fi) 
	((QFontEngineMac*)d->fin)->internal_fi = fi = createFontInfo(d->fin, &d->request, pdev);
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
QCString p2qstring(const unsigned char *c); //qglobal.cpp
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
int QFontMetrics::lineSpacing() const
{
    return leading()+height();
}

int QFontMetrics::lineWidth() const
{
    // lazy computation of linewidth
    d->computeLineWidth();
    return d->lineWidth;
}


int QFontMetrics::leading() const
{
    return d->fin->leading();
}

int QFontMetrics::ascent() const
{
    return d->fin->ascent();
}

int QFontMetrics::descent() const
{
    return d->fin->descent();
}

bool QFontMetrics::inFont(QChar ch) const
{
    return d->fin->canRender(&ch, 1);
}

int QFontMetrics::width(QChar c) const
{
    Q_ASSERT(d->fin->type() == QFontStruct::Mac);
    return ((QFontEngineMac*)d->fin)->doTextTask(&c, 0, 1, 1, QFontEngineMac::WIDTH);
}

int QFontMetrics::charWidth(const QString &str, int pos) const
{
    QTextEngine layout(str, d);
    layout.itemize(FALSE);
    return layout.width(pos, 1);
}

int QFontMetrics::width(const QString &str,int len) const
{
    if(len < 0)
	len = str.length();
    if(len == 0)
	return 0;

    QTextEngine layout(str, d);
    layout.itemize(FALSE);
    return layout.width(0, len);
}

int QFontMetrics::maxWidth() const
{
    return d->fin->maxCharWidth();
}

int QFontMetrics::height() const
{
    return ascent()+descent();
}

int QFontMetrics::minRightBearing() const
{
    return 0;
}

int QFontMetrics::minLeftBearing() const
{
    return 0;
}

int QFontMetrics::leftBearing(QChar) const
{
    return 0;
}

int QFontMetrics::rightBearing(QChar) const
{
  return 0;
}

int QFontMetrics::strikeOutPos() const
{
    int pos = ascent() / 3;
    return pos ? pos : 1;
}

int QFontMetrics::underlinePos() const
{
    int pos = ((lineWidth() * 2) + 3) / 6;
    return pos ? pos : 1;
}

QRect QFontMetrics::boundingRect(QChar ch) const
{
    glyph_t glyphs[10];
    int nglyphs = 9;
    advance_t advances[10];
    d->fin->stringToCMap(&ch, 1, glyphs, advances, &nglyphs);
    glyph_metrics_t gi = d->fin->boundingBox(glyphs[0]);
    return QRect(gi.x, gi.y, gi.width, gi.height);
}

QRect QFontMetrics::boundingRect(const QString &str, int len) const
{
    if(len < 0)
	len = str.length();
    if(len == 0)
	return QRect();

    QTextEngine layout(str, d);
    layout.itemize(FALSE);
    glyph_metrics_t gm = layout.boundingBox(0, len);
    return QRect(gm.x, gm.y, gm.width, gm.height);
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
    delete QFontPrivate::fontCache;
    QFontPrivate::fontCache = NULL;
}

Qt::HANDLE QFont::handle() const
{
    if(d->request.dirty || d->actual.dirty) 
	d->load();
    return d;
}

void QFont::macSetFont(QPaintDevice *v)
{
    QMacSavedPortInfo::setPaintDevice(v);
    QMacSetFontInfo::setMacFont(d, NULL, v);
}

// Computes the line width (underline,strikeout)
void QFontPrivate::computeLineWidth()
{
    int weight = actual.weight;
    int pSize  = actual.pixelSize;

    // ad hoc algorithm
    int score = pSize * weight;
    int nlw = (score) / 700;

    // looks better with thicker line for small pointsizes
    if(nlw < 2 && score >= 1050) 
	nlw = 2;
    if(nlw == 0) 
	nlw = 1;

    if(nlw > lineWidth) 
	lineWidth = nlw;
}

void QFontPrivate::load()
{
    if(request.dirty) {
	request.dirty=FALSE;

	QString k = key();
	QFontEngine* qfs = fontCache->find(k);
	if(!qfs) {
	    qfs = new QFontEngineMac(request);
	    fontCache->insert(k, qfs, 1);
	}
	qfs->ref();
	if(fin) 
	    fin->deref();
	fin=qfs;
	Q_ASSERT(fin->type() == QFontStruct::Mac);
	QFontEngineMac *mac_fin = (QFontEngineMac*)fin;

	if(mac_fin->fnum == -1) {
	    Str255 str;
	    // encoding == 1, yes it is strange the names of fonts are encoded in MacJapanese
	    TextEncoding encoding = CreateTextEncoding(kTextEncodingMacJapanese,
							kTextEncodingDefaultVariant, 
							kTextEncodingDefaultFormat);
	    qstring_to_pstring(request.family, request.family.length(), str, encoding);
#if 0
	    mac_fin->fnum = FMGetFontFamilyFromName(str);
#else
	    GetFNum(str, &mac_fin->fnum);
#endif
	}
	if(!mac_fin->info) {
#if 0
	    mac_fin->info = (ATSFontMetrics*)malloc(sizeof(ATSFontMetrics));
	    const unsigned char *p = p_str(request.family);
	    ATSFontGetVerticalMetrics(ATSFontFamilyFindFromQuickDrawName(p),
				      kATSOptionFlagsDefault, mac_fin->info);
	    free(p);
#else
	    mac_fin->info = (FontInfo *)malloc(sizeof(FontInfo));
	    QMacSetFontInfo fi(this, paintdevice);
	    GetFontInfo(mac_fin->info);
#endif
	}
	actual.dirty = TRUE;
    }
    if(actual.dirty) {
	actual = request;
	actual.dirty = FALSE;
	int logicalDpi = 80; //FIXME
	if(paintdevice) {
	    QPaintDeviceMetrics pm(paintdevice);
	    logicalDpi = pm.logicalDpiY();
	} else {
	    short vr, hr;
	    ScreenRes(&hr, &vr);
	    logicalDpi = vr;
	}
	if(actual.pointSize == -1)
	    actual.pointSize = int((actual.pixelSize * 10 * logicalDpi) / 72. + 0.5);
	else
	    actual.pixelSize = (actual.pointSize * 72 / (10 * logicalDpi));

	Str255 font;
	Q_ASSERT(fin->type() == QFontStruct::Mac);
	GetFontName(((QFontEngineMac*)fin)->fnum, font);
	actual.family = p2qstring(font);

	exactMatch = (actual.family == request.family && 
		      (request.pointSize == -1 || (actual.pointSize == request.pointSize)) && 
		      (request.pixelSize == -1 || (actual.pixelSize == request.pixelSize))); 
    }
}

void QFont::initialize()
{
    if(!QFontPrivate::fontCache) 
	QFontPrivate::fontCache = new QFontCache();
    Q_CHECK_PTR(QFontPrivate::fontCache);
    if(qApp) {
	Str255 f_name;
	SInt16 f_size;
	Style f_style;
	GetThemeFont(kThemeApplicationFont, smSystemScript, f_name, &f_size, &f_style);
	qApp->setFont(QFont(p2qstring(f_name), f_size, 
			    (f_style & ::bold) ? QFont::Bold : QFont::Normal,
			    (bool)(f_style & ::italic)));
    }
}

bool QFont::dirty() const
{
    return d->request.dirty;
}

QString QFontPrivate::defaultFamily() const
{
    switch(request.styleHint) {
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

QString QFontPrivate::lastResortFamily() const
{
    return QString::fromLatin1("helvetica");
}

QString QFontPrivate::lastResortFont() const
{
    return QString::fromLatin1("arial");
}
