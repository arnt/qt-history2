/****************************************************************************
** $Id: $
**
** Implementation of QFont/QFontMetrics class for mac
**
** Created : 001019
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

#include <stdlib.h>
#include "qstring.h"
#include "qfont.h"
#include "qfontdata_p.h"
#include "qfontdatabase.h"
#include "qfontmetrics.h"
#include "qfontinfo.h"
#include "qt_mac.h"
#include "qpaintdevice.h"
#include <qdict.h>
#include <qapplication.h>
#include <qpainter.h>
#ifdef Q_WS_MACX
# define QMAC_FONT_ANTIALIAS
#endif
#ifdef QMAC_FONT_ANTIALIAS
# include <ApplicationServices/ApplicationServices.h>
#endif

/*****************************************************************************
  QFont debug facilities
 *****************************************************************************/
//#define DEBUG_FONTMETRICS

#ifdef DEBUG_FONTMETRICS
#include <qregexp.h>
//QT_METRICS_DEBUG="48x68,48x102,48x100,48x107"
static bool debug_metrics_first = TRUE, debug_metrics_all = FALSE;
static QValueList<QChar> debug_metrics_list;
static bool qt_mac_debug_metrics(const QChar &c) {
    if(debug_metrics_first) {
	debug_metrics_first = FALSE;
	if(char *en = getenv("QT_METRICS_DEBUG")) {
	    qDebug("%s", en);
	    QStringList l(QStringList::split(',', QString(en)));
	    for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
		(*it) = (*it).stripWhiteSpace().simplifyWhiteSpace();
		if((*it) == "all") {
		    debug_metrics_all = TRUE;
		    break;
		}
		short k;
		if((*it).contains('x')) {
		    QRegExp re("^([0-9]*)x([0-9]*)$");
		    if(re.exactMatch((*it))) {
			QString row = re.cap(1), col = re.cap(2);
			bool ok;
			short l;
			//row
			l = row.toShort(&ok);
			if(!ok || l > 0xFF) {
			    qDebug("Invalid row %d: %s", l, (*it).latin1());
			    continue;
			}
			k = l << 8;
			//column
			l = col.toShort(&ok);
			if(!ok || l > 0xFF) {
			    qDebug("Invalid colum %d: %s", l, (*it).latin1());
			    continue;
			}
			k |= l;
		    } else {
			qDebug("%d: Invalid metrics debug: %s", __LINE__, (*it).latin1());
			continue;
		    }
		} else {
		    bool ok = TRUE;
		    k = (*it).toShort(&ok);
		    if(!ok) {
			qDebug("%d: Invalid metrics debug: %s", __LINE__, (*it).latin1());
			continue;
		    }
		}
		qDebug("Valid metrics debug: %s", (*it).latin1());
		debug_metrics_list.append(QChar(k));
	    }
	} else {
	    qDebug("Must specify a QT_METRICS_DEBUG to use DEBUG_METRICS");
	}
    }
    if(debug_metrics_all)
	return TRUE;
    for(QValueList<QChar>::Iterator it = debug_metrics_list.begin(); it != debug_metrics_list.end(); ++it) {
	if((*it) == c)
	    return TRUE;
    }
    return FALSE;
}
bool qt_mac_debug_metrics(const QString s, int len) {
    if(len == -1)
	len = s.length();
    if(!len)
	return FALSE;
    for(int i = 0; i < len; i++) {
	if(!qt_mac_debug_metrics(s.at(i)))
	    return FALSE;
    }
    return TRUE;
}
#endif

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

    if(CreateUnicodeToTextInfo(&mapping, &info) != noErr)
      Q_ASSERT(0);

    const int unilen = len * 2;
    const UniChar *unibuf = (UniChar *)s.unicode(); //don't use pos here! FIXME
    ConvertFromUnicodeToPString(info, unilen, unibuf, str);
}

/* font information */
#if defined( QMAC_FONT_ATSUI )
struct QATSUStyle : public QShared {
    ATSUStyle style;
    RGBColor rgb;
};
#endif
class QMacFontInfo
{
    short fi_fnum, fi_face;
    int fi_size;
    TextEncoding fi_enc;
#if defined( QMAC_FONT_ATSUI )
    QATSUStyle *fi_astyle;
#endif
public:
    inline QMacFontInfo() : fi_fnum(0), fi_face(0), fi_size(0), fi_enc(0)
#if defined( QMAC_FONT_ATSUI )
	,fi_astyle(0)
#endif
	{ }
#if defined( QMAC_FONT_ATSUI )
    inline ~QMacFontInfo() 
	{ if(fi_astyle && fi_astyle->deref()) {
	    ATSUDisposeStyle(fi_astyle->style);
	    delete fi_astyle;
	} }
#endif
    inline QMacFontInfo &operator=(QMacFontInfo &rhs) {
	setEncoding(rhs.encoding());
	setFont(rhs.font());
	setStyle(rhs.style());
	setSize(rhs.size());
#if defined( QMAC_FONT_ATSUI )
	if(rhs.atsuStyle()) {
	    rhs.atsuStyle()->ref();
	    setATSUStyle(rhs.atsuStyle());
	} else {
	    if(fi_astyle && fi_astyle->deref()) {
		ATSUDisposeStyle(fi_astyle->style);
		delete fi_astyle;
	    }
	    setStyle(NULL);
	}
#endif
	return *this;
    }

    inline TextEncoding encoding() const { return fi_enc; }
    inline void setEncoding(TextEncoding f) { fi_enc = f; }

    inline short font() const { return fi_fnum; }
    inline void setFont(short f) { fi_fnum = f; }

    inline short style() const { return fi_face; }
    inline void setStyle(short f) { fi_face = f; }

    inline int size() const { return fi_size; }
    inline void setSize(int f) { fi_size = f; }

#if defined( QMAC_FONT_ATSUI )
    inline QATSUStyle *atsuStyle() { return fi_astyle; }
    inline void setATSUStyle(QATSUStyle *s) { fi_astyle = s; }
#endif
};

class QMacSetFontInfo : public QMacSavedFontInfo, public QMacFontInfo 
{
public:
    //create this for temporary font settting
    inline QMacSetFontInfo(const QFontPrivate *d) : QMacSavedFontInfo(), QMacFontInfo() { setMacFont(d, this); }

    //you can use this to cause font setting, without restoring old
    static bool setMacFont(const QFontPrivate *d, QMacSetFontInfo *sfi=NULL);
};

inline bool QMacSetFontInfo::setMacFont(const QFontPrivate *d, QMacSetFontInfo *sfi)
{
    ((QFontPrivate *)d)->load();

    QMacFontInfo *fi = d->fin->internal_fi;
    if(!fi) {
	d->fin->internal_fi = fi = new QMacFontInfo();

	//face
	fi->setFont(d->fin->fnum);

	//style
	short face = normal;
	if(d->request.italic)
	    face |= italic;
	if(d->request.weight == QFont::Bold)
	    face |= bold;
	fi->setStyle(face);
	
	//size
	int pointSize = d->request.pointSize != -1 ? d->request.pointSize / 10 : 
			      d->request.pixelSize *80 /72; 
	fi->setSize(pointSize);

	//encoding
	TextEncoding enc;
	UpgradeScriptInfoToTextEncoding(FontToScript(d->fin->fnum), kTextLanguageDontCare, 
					kTextRegionDontCare, NULL, &enc);
	fi->setEncoding(enc);

#if defined( QMAC_FONT_ATSUI )
	//Create a cacheable ATSUStyle
	const int arr_guess = 5;
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
	ATSUFONDtoFontID(d->fin->fnum, face, &fond);
	valueSizes[arr] = sizeof(fond);
	values[arr] = &fond;
	arr++;
	if(arr > arr_guess) //this won't really happen, just so I will not miss the case
	    qDebug("%d: Whoa!! you forgot to increase arr_guess! %d", __LINE__, arr);

	//create style
	QATSUStyle *st = new QATSUStyle;
	st->rgb.red = st->rgb.green = st->rgb.blue = 0;
	ATSUCreateStyle(&st->style);
	if(OSStatus e = ATSUSetAttributes(st->style, arr, tags, valueSizes, values)) {
	    qDebug("%ld: This shouldn't happen %s:%d", e, __FILE__, __LINE__);
	    delete st;
	    st = NULL;
	}
	fi->setATSUStyle(st);
#endif
    }
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


QCString p2qstring(const unsigned char *c); //qglobal.cpp
const unsigned char * p_str(const QString &); //qglobal.cpp
enum text_task { GIMME_WIDTH=0x01, GIMME_DRAW=0x02, GIMME_EXISTS=0x04 };
#if defined( QMAC_FONT_ATSUI )
static int do_text_task(const QFontPrivate *d, const QChar *s, int pos,
			int use_len, int len, uchar task, int =-1, int y=-1)
{
    int ret = 0;
    QMacSetFontInfo fi(d);
    QATSUStyle *st = fi.atsuStyle();
    if(!st) 
	return 0;
    if(task & GIMME_DRAW) {
	RGBColor fcolor;
	GetForeColor(&fcolor);
	if(st->rgb.red != fcolor.red || st->rgb.green != fcolor.green ||
	   st->rgb.blue != fcolor.blue) {
	    st->rgb = fcolor;
	    const ATSUAttributeTag tag = kATSUColorTag;
	    ByteCount size = sizeof(fcolor);
	    ATSUAttributeValuePtr value = &fcolor;
	    if(OSStatus e = ATSUSetAttributes(st->style, 1, &tag, &size, &value)) {
		qDebug("%ld: This shouldn't happen %s:%d", e, __FILE__, __LINE__);
		return 0;
	    }
	}
    }

    //create layout
    ATSUTextLayout alayout;
    const UniCharCount count = use_len;
#if 0
    if(OSStatus e = ATSUCreateTextLayoutWithTextPtr((UniChar *)(s), pos, 
						    count, len, 1, &count, 
						    &st->style, &alayout)) {
	qDebug("%ld: This shouldn't happen %s:%d", e, __FILE__, __LINE__);
	return 0;
    }
#else
    Q_UNUSED(len);
    if(OSStatus e = ATSUCreateTextLayoutWithTextPtr((UniChar *)(s)+pos, 0, 
						    count, use_len, 1, &count, 
						    &st->style, &alayout)) {
	qDebug("%ld: This shouldn't happen %s:%d", e, __FILE__, __LINE__);
	return 0;
    }
#endif
    const int arr_guess = 5;
    int arr = 0;
    ATSUAttributeTag tags[arr_guess];
    ByteCount valueSizes[arr_guess];
    ATSUAttributeValuePtr values[arr_guess];
    tags[arr] = kATSULineLayoutOptionsTag;
    ATSLineLayoutOptions layopts = kATSLineIsDisplayOnly | kATSLineHasNoOpticalAlignment | 
				   kATSLineFractDisable;
    valueSizes[arr] = sizeof(layopts);
    values[arr] = &layopts;
    arr++;
#ifdef QMAC_FONT_ANTIALIAS
    tags[arr] = kATSUCGContextTag; //cgcontext
    Rect clipr;
    CGrafPtr port;
    RgnHandle clip = NewRgn();
    CGContextRef ctx;
    GetPort(&port);
    GetPortClipRegion(port, clip);
    GetPortBounds(port, &clipr);
    QDBeginCGContext(port, &ctx);
    ClipCGContextToRegion(ctx, &clipr, clip);
    valueSizes[arr] = sizeof(ctx);
    values[arr] = &ctx;
    arr++;
#endif
    if(arr > arr_guess) //this won't really happen, just so I will not miss the case
	qDebug("%d: Whoa!! you forgot to increase arr_guess! %d", __LINE__, arr);
    if(OSStatus e = ATSUSetLayoutControls(alayout, arr, tags, valueSizes, values)) {
	qDebug("%ld: This shouldn't happen %s:%d", e, __FILE__, __LINE__);
#ifdef QMAC_FONT_ANTIALIAS
	QDEndCGContext(port, &ctx);
#endif
	return 0;
    }
    ATSUSetTransientFontMatching(alayout, true);

    //do required task now
    if(task & GIMME_DRAW) {
	ATSUDrawText(alayout, kATSUFromTextBeginning, kATSUToTextEnd, 
#ifdef QMAC_FONT_ANTIALIAS
		     kATSUUseGrafPortPenLoc, FixRatio((clipr.bottom-clipr.top)-y, 1)
#else
		     kATSUUseGrafPortPenLoc, kATSUUseGrafPortPenLoc
#endif
	    );
    }
    if(task & GIMME_WIDTH) {
	ATSUTextMeasurement left, right, bottom, top;
	ATSUMeasureText(alayout, kATSUFromTextBeginning, kATSUToTextEnd,
			&left, &right, &bottom, &top);
#if 0
	qDebug("(%s) (%s): %p %d %d %d (%d %d == %d)", 
	       QString(s+pos, use_len).latin1(), QString(s, len).latin1(),
	       s, pos, use_len, len, FixRound(left), FixRound(right), 
	       FixRound(right) - FixRound(left));
#endif
	ret = FixRound(right-left);
    }
    //cleanup
    ATSUDisposeTextLayout(alayout);
#ifdef QMAC_FONT_ANTIALIAS
    QDEndCGContext(port, &ctx);
#endif
    return ret;
}
static inline int do_text_task(const QFontPrivate *d, QString s, int pos, int len, uchar task,
    int x=-1, int y=-1)
{
    if(!len)
	return 0;
    else if(pos + len > (int)s.length())
	len = s.length() - pos;
    return do_text_task(d, s.unicode(), pos, len, s.length(), task, x, y);
}

static inline int do_text_task(const QFontPrivate *d, const QChar &c, uchar task,
    int x=-1, int y=-1)
{
    return do_text_task(d, &c, 0, 1, 1, task, x, y);
}

#else
static QMAC_PASCAL OSStatus macFallbackChar(UniChar *, ByteCount, ByteCount *oSrcConvLen, TextPtr oStr,
					    ByteCount iDestLen, ByteCount *oDestConvLen, void *, 
					    ConstUnicodeMappingPtr map)
{
    UnicodeToTextInfo tuni;
    CreateUnicodeToTextInfo(map, &tuni);
    const short flbk = 0x25A1; //square
    const ByteCount flbklen = sizeof(flbk);
    OSStatus err = ConvertFromUnicodeToText(tuni, flbklen,(UniChar *)&flbk, 0,
					     0, NULL, NULL, NULL, iDestLen, oSrcConvLen,
					     oDestConvLen, oStr);
    DisposeUnicodeToTextInfo(&tuni);
    return err == noErr ? noErr : kTECUnmappableElementErr;
}
static UnicodeToTextFallbackUPP qt_macFallbackCharUPP = NULL;
static void cleanup_font_fallbackUPP() 
{
    if(qt_macFallbackCharUPP) {
	DisposeUnicodeToTextFallbackUPP(qt_macFallbackCharUPP);
	qt_macFallbackCharUPP = NULL;
    }
}    
static const UnicodeToTextFallbackUPP make_font_fallbackUPP() 
{
    if(qt_macFallbackCharUPP)
	return qt_macFallbackCharUPP;
    qAddPostRoutine(cleanup_font_fallbackUPP);
    return qt_macFallbackCharUPP = NewUnicodeToTextFallbackUPP(macFallbackChar);
}

static int do_text_task(const QFontPrivate *d, const QChar *s, int len, uchar task,
			int x=-1, int y=-1)
{
    //set the grafport font
    QMacSetFontInfo fi(d);
    FontInfo setfi; GetFontInfo(&setfi);
    OSStatus err;

    //convert qt to mac unibuffer
    const int unilen = len * 2;
    const UniChar *unibuf = (UniChar *)(s);

    //create converter
    UnicodeToTextRunInfo runi;
    ItemCount scpts = 1 << 31; //high bit
    short scpt = FontToScript(fi.font());
    err =  CreateUnicodeToTextRunInfoByScriptCode(scpts, &scpt, &runi);
    if(err != noErr) {
	qDebug("unlikely error %d %s:%d", (int)err, __FILE__, __LINE__);
	return 0;
    }
    SetFallbackUnicodeToTextRun(runi, make_font_fallbackUPP(), kUnicodeFallbackCustomFirst, NULL);

    //now convert
    int buf_len = 2056;     //buffer
    uchar *buf = (uchar *)malloc(buf_len);
#define DEFAULT_RUNS 20
    ItemCount run_len = DEFAULT_RUNS; //runs
    ScriptCodeRun runs[DEFAULT_RUNS];
#undef DEFAULT_RUNS
    ByteCount read, converted; //returns
    int read_so_far = 0;
    const int flags = kUnicodeUseFallbacksMask | kUnicodeTextRunMask;
    int ret = 0, sz = fi.size();
    ScriptCode sc = FontToScript(fi.font());
    for (;;) {
	err = ConvertFromUnicodeToScriptCodeRun(runi, unilen-read_so_far, 
						unibuf+read_so_far, flags,
						0, NULL, NULL, NULL, buf_len, &read, 
						&converted, buf, run_len, &run_len, runs);
	if(err != noErr && err != kTECUsedFallbacksStatus && 
	   err != kTECArrayFullErr && err != kTECOutputBufferFullStatus)  {
	    qDebug("unlikely error %d %s:%d", (int)err, __FILE__, __LINE__);
	    DisposeUnicodeToTextRunInfo(&runi);
	    free(buf);
	    return 0;
	}
	if(task & GIMME_EXISTS) {
	    if(task != GIMME_EXISTS)
		qWarning("GIMME_EXISTS must appear by itself!");
	    ret = (err != kTECUsedFallbacksStatus);
	    break;
	}
	read_so_far += read;

	for(ItemCount i = 0; i < run_len; i++) {
	    //set the font
	    short fn = runs[i].script == sc ? fi.font() : 
		       GetScriptVariable(runs[i].script, smScriptSysFond);
	    TextFont(fn);

	    //font scaling
	    FontInfo info;
	    GetFontInfo(&info);
	    int msz = sz;
	    while((info.ascent + info.descent) > (setfi.ascent + setfi.descent)) {
		TextSize(--msz);
		GetFontInfo(&info);
	    }

	    //calculate string offsets
	    ByteOffset off = runs[i].offset;
	    int rlen = ((i == run_len - 1) ? converted : runs[i+1].offset) - off;

	    //do the requested task
	    if(task & GIMME_DRAW) 
		DrawText(buf, off, rlen);
	    if(task & GIMME_WIDTH)
		ret += TextWidth(buf, off, rlen);

	    //restore the scale
	    if(msz != sz)
		TextSize(sz);
	}

	if(err != kTECArrayFullErr && err != kTECOutputBufferFullStatus)
	    break;
    }
    DisposeUnicodeToTextRunInfo(&runi);
    free(buf);
    return ret;
}

static inline int do_text_task(const QFontPrivate *d, QString s, int pos, int len, uchar task,
			       int x=-1, int y=-1)
{
#if 1
    if(!len)
	return 0;
    else if(pos + len > (int)s.length())
	len = s.length() - pos;
#else
    if(!len || (pos+len) > (int)s.length()) {
	if(len)
	    qDebug("This should never happen %d > %d", pos+len, s.length());
	return 0;
    }
#endif
    bool is_latin = TRUE;
    const QChar *chs = s.unicode() + pos; //don't use pos here
    for(int i = 0; i < len; i++) {
	if(chs[i].row() || (chs[i].cell() & (1 << 7))) {
	    is_latin = FALSE;
	    break;
	} 
    }
    if(is_latin) {
	QMacSetFontInfo fi(d);
	if(task & GIMME_EXISTS) {
	    if(task != GIMME_EXISTS)
		qWarning("GIMME_EXISTS must appear by itself!");
	    return TRUE;
	}
	int ret = 0;
	const int maxlen = 255;
	int curlen = len, curpos = pos;
	while (curlen > 0) {
	    const unsigned char *str = p_str(s.mid(curpos, QMIN(curlen,maxlen)));
	    if(task & GIMME_DRAW) 
		DrawString(str);
	    if(task & GIMME_WIDTH)
		ret += StringWidth(str);
	    curlen -= maxlen;
	    curpos += maxlen;
	}
	return ret;
    }
    return do_text_task(d, s.unicode()+pos, len, task, x, y); //don't use pos like this
}

static inline int do_text_task(const QFontPrivate *d, const QChar &c, uchar task,
			       int x=-1, int y=-1)
{
    if(c.row() || (c.cell() & (1 << 7)))
	return do_text_task(d, &c, 1, task, x, y);

    QMacSetFontInfo fi(d);
    int ret = 0; //latin1 optimization
    if(task & GIMME_EXISTS) {
	if(task != GIMME_EXISTS)
	    qWarning("GIMME_EXISTS must appear by itself!");
	return TRUE;
    }
    if(task & GIMME_WIDTH)
	ret = CharWidth((char)c.cell());
    if(task & GIMME_DRAW) 
	DrawChar((char)c.cell());
    return ret;
}
#endif

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

#undef FI
#define FI (painter ? painter->cfont.d : d)

int QFontMetrics::leading() const
{
    return FI->fin->leading();
}

int QFontMetrics::ascent() const
{
    return FI->fin->ascent();
}

int QFontMetrics::descent() const
{
    return FI->fin->descent();
}

bool QFontMetrics::inFont(QChar ch) const
{
    return do_text_task(FI, ch, GIMME_EXISTS);
}

int QFontMetrics::width(QChar c) const
{
    int width = do_text_task(FI, c, GIMME_WIDTH);
#ifdef DEBUG_FONTMETRICS
    if(qt_mac_debug_metrics(c)) {
	qDebug("width(QChar) %d %d:%d==%d %d:%d %dx%d", width, d->request.pointSize, d->request.pixelSize,
	       d->request.pointSize != -1 ? d->request.pointSize / 10 : d->request.pixelSize *80 /72,
	       d->actual.pointSize, d->actual.pixelSize, c.row(), c.cell());
    }
#endif
    return width;
}

int QFontMetrics::charWidth(const QString &s, int pos) const
{
    int width = do_text_task(FI, s, pos, 1, GIMME_WIDTH);
#ifdef DEBUG_FONTMETRICS
    QChar c = s.at(pos);
    if(qt_mac_debug_metrics(c)) {
	qDebug("width(string, char) %d %d:%d==%d %d:%d %dx%d", width, d->request.pointSize, d->request.pixelSize,
	       d->request.pointSize != -1 ? d->request.pointSize / 10 : d->request.pixelSize *80 /72,
	       d->actual.pointSize, d->actual.pixelSize, c.row(), c.cell());
    }
#endif
    return width;
}

int QFontMetrics::width(const QString &s,int len) const
{
    int width = do_text_task(FI, s, 0, len < 1 ? s.length() : len, GIMME_WIDTH);
#ifdef DEBUG_FONTMETRICS
    if(qt_mac_debug_metrics(s, len)) {
	qDebug("width(string, %d) %d %d:%d==%d %d:%d", len, width, d->request.pointSize, d->request.pixelSize,
	       d->request.pointSize != -1 ? d->request.pointSize / 10 : d->request.pixelSize *80 /72,
	       d->actual.pointSize, d->actual.pixelSize);
    }
#endif
   return width;
}

int QFontMetrics::maxWidth() const
{
    return FI->fin->maxWidth();
}

int QFontMetrics::height() const
{
    return ascent()+descent();
}

int QFontMetrics::minRightBearing() const
{
    return FI->fin->minRightBearing();
}

int QFontMetrics::minLeftBearing() const
{
    return FI->fin->minLeftBearing();
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


QRect QFontMetrics::boundingRect(const QString &str, int len) const
{
    return QRect(0,-(ascent()),width(str,len),height());
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
    return 0;
}

void QFont::macSetFont(QPaintDevice *v)
{
    d->macSetFont(v);
}

void QFontPrivate::macSetFont(QPaintDevice *v)
{
    QMacSavedPortInfo::setPaintDevice(v);
    QMacSetFontInfo::setMacFont(this);
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
    if (nlw < 2 && score >= 1050) 
	nlw = 2;
    if (nlw == 0) 
	nlw = 1;

    if (nlw > lineWidth) 
	lineWidth = nlw;
}

void QFontPrivate::drawText(int x, int y, QString s, int len)
{
    MoveTo(x, y);
    if(len < 1)
	len = s.length();
#ifdef DEBUG_FONTMETRICS
    bool do_debug = qt_mac_debug_metrics(s, len);
#endif

    uchar task = GIMME_DRAW;
    if(request.underline || request.strikeOut) 
	task |= GIMME_WIDTH; //I need the width for these..
#ifdef DEBUG_FONTMETRICS
    else if(do_debug)
	task |= GIMME_WIDTH;
#endif
    int w = do_text_task(this, s, 0, len, task, x, y);
    
#ifdef DEBUG_FONTMETRICS
    if(do_debug) {
	qDebug("drawText %d::%dx%d,%d %d:%d==%d %d:%d %d %d", w, len, x, y, request.pointSize, request.pixelSize,
	       request.pointSize != -1 ? request.pointSize / 10 : request.pixelSize *80 /72,
	       actual.pointSize, actual.pixelSize, request.underline, request.strikeOut);
    }
#endif
    if(request.underline || request.strikeOut) { 
	computeLineWidth();
	if(request.underline) {
	    Rect r;
	    SetRect(&r, x, (y + 2) - (lineWidth / 2), 
		    x + w, (y + 2) + (lineWidth / 2));
	    if(!(r.bottom - r.top))
		r.bottom++;
	    PaintRect(&r);
	}
	if(request.strikeOut) {
	    int spos = fin->ascent() / 3;
	    if(!spos)
		spos = 1;
	    Rect r;
	    SetRect(&r, x, (y - spos) - (lineWidth / 2), 
		    x + w, (y - spos) + (lineWidth / 2));
	    if(!(r.bottom - r.top))
		r.bottom++;
	    PaintRect(&r);
	}
    } 
}

void QFontPrivate::load()
{
    if(request.dirty) {
	request.dirty=FALSE;

	QString k = key();
	QFontStruct* qfs = fontCache->find(k);
	if (!qfs) {
	    qfs = new QFontStruct();
	    fontCache->insert(k, qfs, 1);
	}
	qfs->ref();

	if(fin) 
	    fin->deref();
	fin=qfs;
	if(fin->fnum == -1) {
	    Str255 str;
	    // encoding == 1, yes it is strange the names of fonts are encoded in MacJapanese
	    TextEncoding encoding = CreateTextEncoding(kTextEncodingMacJapanese,
							kTextEncodingDefaultVariant, 
							kTextEncodingDefaultFormat);
	    qstring_to_pstring(request.family, request.family.length(), str, encoding);
	    GetFNum(str, &fin->fnum);
	}
	if(!fin->info) {
#if defined( QMAC_FONT_ATSUI ) && 0
	    fin->info = (ATSFontMetrics*)malloc(sizeof(ATSFontMetrics));
	    ATSFontGetVerticalMetrics(ATSFontFamilyFindFromQuickDrawName(p_str(request.family)),
				      kATSOptionFlagsDefault, fin->info);
#else
	    fin->info = (FontInfo *)malloc(sizeof(FontInfo));
	    QMacSetFontInfo fi(this);
	    GetFontInfo(fin->info);
#endif
	}
	actual.dirty = TRUE;
    }
    if(actual.dirty) {
	actual = request;
	actual.dirty = FALSE;
	if (actual.pointSize == -1)
	    actual.pointSize = int((actual.pixelSize * 10 * 80) / 72. + 0.5);
	else
	    actual.pixelSize = (actual.pointSize * 72 / (10 * 80));

	Str255 font;
	GetFontName(fin->fnum, font);
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

QRect QFontPrivate::boundingRect(const QChar &ch)
{
    return QRect(0,-(fin->ascent()), do_text_task(this, ch, GIMME_WIDTH),
		  fin->ascent() + fin->descent());
}

int QFontPrivate::textWidth(const QString &s, int p, int l)
{
    return do_text_task(this, s, p, l, GIMME_WIDTH);
}
