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

/* utility functions */
static inline void qstring_to_pstring( QString s, int len, Str255 str, TextEncoding encoding )
{
    UnicodeMapping mapping;
    UnicodeToTextInfo info;
    mapping.unicodeEncoding = CreateTextEncoding( kTextEncodingUnicodeDefault,
						  kTextEncodingDefaultVariant, kUnicode16BitFormat );
    mapping.otherEncoding = encoding;
    mapping.mappingVersion = kUnicodeUseLatestMapping;

    if ( CreateUnicodeToTextInfo( &mapping, &info  ) != noErr )
      Q_ASSERT( 0 );

    const int unilen = len * 2;
    const UniChar *unibuf = (UniChar *)s.unicode(); //don't use pos here! FIXME
    ConvertFromUnicodeToPString( info, unilen, unibuf, str  );
}

/* font information */
class QMacFontInfo
{
    short fi_fnum, fi_face;
    int fi_size;
    TextEncoding fi_enc;
public:
    inline QMacFontInfo() : fi_fnum(0), fi_face(0), fi_size(0), fi_enc(0) { }
    inline QMacFontInfo &operator=( const QMacFontInfo &rhs ) {
	setEncoding(rhs.encoding());
	setFont(rhs.font());
	setStyle(rhs.style());
	setSize(rhs.size());
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
    if(d->request.dirty)
	((QFontPrivate *)d)->load();

    QMacFontInfo *fi = d->fin->internal_fi;
    if(!fi) {
	d->fin->internal_fi = fi = new QMacFontInfo();

	//face
	Str255 str;
	// encoding == 1, yes it is strange the names of fonts are encoded in MacJapanese
	TextEncoding encoding = CreateTextEncoding( kTextEncodingMacJapanese,
						    kTextEncodingDefaultVariant, kTextEncodingDefaultFormat );
	qstring_to_pstring( d->request.family, d->request.family.length(), str, encoding );
	short fnum;
	GetFNum(str, &fnum);
	fi->setFont(fnum);

	//style
	short face = normal;
	if(d->request.italic)
	    face |= italic;
	if(d->request.underline) 
	    face |= underline;
	//strikeout? FIXME
	if(d->request.weight == QFont::Bold)
	    face |= bold;
	fi->setStyle(face);
	
	//size
	fi->setSize(d->request.pointSize / 10);

	//encoding
	TextEncoding enc;
	UpgradeScriptInfoToTextEncoding( FontToScript( fnum ), kTextLanguageDontCare, 
					 kTextRegionDontCare, NULL, &enc );
	fi->setEncoding(enc);
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

static QMAC_PASCAL OSStatus macFallbackChar(UniChar *, ByteCount, ByteCount *oSrcConvLen, TextPtr oStr,
					    ByteCount iDestLen, ByteCount *oDestConvLen, void *, 
					    ConstUnicodeMappingPtr map)
{
    UnicodeToTextInfo tuni;
    CreateUnicodeToTextInfo(map, &tuni);
    const short flbk = 0x25A1; //square
    const ByteCount flbklen = sizeof(flbk);
    OSStatus err = ConvertFromUnicodeToText( tuni, flbklen,(UniChar *)&flbk, 0,
					     0, NULL, NULL, NULL, iDestLen, oSrcConvLen,
					     oDestConvLen, oStr);
    DisposeUnicodeToTextInfo(&tuni);
    return err == noErr ? noErr : kTECUnmappableElementErr;
}

const unsigned char * p_str(const QString &); //qglobal.cpp
enum text_task { GIMME_WIDTH=0x01, GIMME_DRAW=0x02 };
static int do_text_task( const QFontPrivate *d, QString s, int pos, int len, uchar task, bool no_optim=FALSE)
{
    QMacSetFontInfo fi(d);
    if(!no_optim) //latin1 optimization
    {
	uint is_latin = 1;
	const QChar *chs = s.unicode() + pos;
	for(int i = 0; i < len; i++) {
	    if(chs[i].row() || (chs[i].cell() & (1 << 7))) {
		is_latin = 0;
		break;
	    } 
	}
	if(is_latin) {
	    int ret = 0;
	    if(task & GIMME_WIDTH)
		ret = TextWidth(chs, 0, len * 2);
	    if(task & GIMME_DRAW) {
		TextFace(fi.style() & ~(underline)); //do my own underlining
		DrawText(chs, 0, len * 2);
	    }
	    return ret;
	}
    }

    //set the grafport font
    FontInfo setfi; GetFontInfo(&setfi);
    OSStatus err;

    //convert qt to mac unibuffer
    const int unilen = len * 2;
    const UniChar *unibuf = (UniChar *)(s.unicode() + pos); //don't use pos here! FIXME

    //create converter
    UnicodeToTextRunInfo runi;
    ItemCount scpts = 1 << 31; //high bit
    short scpt[1]; 
    scpt[0] = FontToScript( fi.font() );
    err =  CreateUnicodeToTextRunInfoByScriptCode(scpts, scpt, &runi);
    if(err != noErr) {
	qDebug("unlikely error %d %s:%d", (int)err, __FILE__, __LINE__);
	return 0;
    }
    SetFallbackUnicodeToTextRun( runi, NewUnicodeToTextFallbackUPP(macFallbackChar), 
				 kUnicodeFallbackCustomFirst, NULL);

    //now convert
    int buf_len = 2056;     //buffer
    uchar *buf = (uchar *)malloc(buf_len);
    ItemCount run_len = 20; //runs
    ScriptCodeRun runs[run_len];
    ByteCount read, converted; //returns
    int read_so_far = 0;
    const int flags = kUnicodeUseFallbacksMask | kUnicodeTextRunMask;
    int ret = 0, sz = fi.size();
    ScriptCode sc = FontToScript(fi.font());
    while(1) {
	err = ConvertFromUnicodeToScriptCodeRun( runi, unilen-read_so_far, unibuf+read_so_far, flags,
						 0, NULL, NULL, NULL, buf_len, &read, 
						 &converted, buf, run_len, &run_len, runs);
	if(err != noErr && err != kTECUsedFallbacksStatus && 
	   err != kTECArrayFullErr && err != kTECOutputBufferFullStatus)  {
	    qDebug("unlikely error %d %s:%d", (int)err, __FILE__, __LINE__);
	    DisposeUnicodeToTextRunInfo(&runi);
	    free(buf);
	    return 0;
	}
	read_so_far += read;

	for(ItemCount i = 0; i < run_len; i++) {
	    //set the font
	    short fn = runs[i].script == sc ? fi.font() : GetScriptVariable(runs[i].script, smScriptSysFond);
	    TextFont(fn);

	    //crap font scaling
	    FontInfo info;
	    GetFontInfo(&info);
	    int msz = sz;
	    while( (info.ascent + info.descent) > (setfi.ascent + setfi.descent)) {
		TextSize(msz--);
		GetFontInfo(&info);
	    }

	    //calculate string offsets
	    ByteOffset off = runs[i].offset;
	    int rlen = ((i == run_len - 1) ? converted : runs[i+1].offset) - off;

	    //do the requested task
	    if(task & GIMME_WIDTH)
		ret += TextWidth(buf, off, rlen);
	    if(task & GIMME_DRAW) {
		TextFace(fi.style() & ~(underline)); //do my own underlining
		DrawText(buf, off, rlen);
		TextFace(fi.style());
	    }

	    //restore the scale
	    if(msz != sz)
		TextSize(sz);
	}

	if( err != kTECArrayFullErr && err != kTECOutputBufferFullStatus)
	    break;
    }
    DisposeUnicodeToTextRunInfo(&runi);
    free(buf);
    return ret;
}
static inline int do_text_task( const QFontPrivate *d, const QChar &c, uchar task, bool no_optim=FALSE)
{
    if(!no_optim || c.row() || (c.cell() & (1 << 7)))
	return do_text_task(d, QString(c), 0, 1, task, TRUE);
    QMacSetFontInfo fi(d);
    int ret = 0; //latin1 optimization
    if(task & GIMME_WIDTH)
	ret = CharWidth((char)c.cell());
    if(task & GIMME_DRAW) {
	TextFace(fi.style() & ~(underline)); //do my own underlining
	DrawChar((char)c.cell());
    }
    return ret;
}


/* Qt platform dependant functions */
int QFontMetrics::lineSpacing() const
{
    return leading()+height();
}

int QFontMetrics::lineWidth() const
{
    return 1;
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

int QFontMetrics::width(QChar c) const
{
    return do_text_task(FI, c, GIMME_WIDTH);
}

int QFontMetrics::charWidth( const QString &s, int pos ) const
{
    return do_text_task(FI, s, pos, 1, GIMME_WIDTH);
}

int QFontMetrics::width(const QString &s,int len) const
{
    return do_text_task(FI, s, 0, len < 1 ? s.length() : len, GIMME_WIDTH);
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


QRect QFontMetrics::boundingRect( const QString &str, int len ) const
{
    return QRect( 0,-(ascent()),width(str,len),height());
}

void QFont::cleanup()
{
    delete QFontPrivate::fontCache;
}

Qt::HANDLE QFont::handle() const
{
    if(d->request.dirty) {
	d->load();
    }
    return 0;
}

void QFont::macSetFont(QPaintDevice *v)
{
    d->macSetFont(v);
}

void QFontPrivate::macSetFont(QPaintDevice *v)
{
    if(v && !v->paintingActive()) {
	qDebug("I was really hoping it would never come to this...");
	Q_ASSERT(0); //we need to figure out if this can really happen
    }
    QMacSetFontInfo::setMacFont(this);
}

void QFontPrivate::drawText( int x, int y, QString s, int len )
{
    MoveTo(x, y);
    if(len < 1)
	len = s.length();
    uchar task = GIMME_DRAW;
    if(request.underline || request.strikeOut) 
	task |= GIMME_WIDTH;
    int w = do_text_task(this, s, 0, len, task);
    if(task & GIMME_WIDTH) { //I need the width for these..
	if(request.underline) {
	    MoveTo(x, y + 2);
	    LineTo(x + w, y + 2);
	}
	if(request.strikeOut) {
	    int spos = fin->ascent() / 3;
	    if(!spos)
		spos = 1;
	    MoveTo(x, y - spos);
	    LineTo(x + w, y - spos);
	}
    } 
}

void QFontPrivate::load()
{
    if(!request.dirty)
	return;
    request.dirty=FALSE;

    QString k = key();
    QFontStruct* qfs = fontCache->find(k);
    if ( !qfs ) {
	qfs = new QFontStruct(request);
	fontCache->insert(k, qfs, 1);
    }
    qfs->ref();

    if(fin) 
	fin->deref();
    fin=qfs;
    if(!fin->info) {
	fin->info = (FontInfo *)malloc(sizeof(FontInfo));
	QMacSetFontInfo fi(this);
	GetFontInfo(fin->info);
    }

    // Our 'handle' is actually a structure with the information needed to load
    // the font into the current grafport
}

void QFont::initialize()
{
    if(!QFontPrivate::fontCache)
	QFontPrivate::fontCache = new QFontCache();
    Q_CHECK_PTR( QFontPrivate::fontCache );
    if(qApp)
	qApp->setFont(QFont("Helvetica", 14));
}

void QFont::setPixelSizeFloat( float pixelSize )
{
    setPointSizeFloat( pixelSize );
}

int QFont::pixelSize() const
{
    return d->request.pointSize/10;
}

QString QFontPrivate::defaultFamily() const
{
    switch( request.styleHint ) {
	case QFont::Times:
	    return QString::fromLatin1("Times New Roman");
	case QFont::Courier:
	    return QString::fromLatin1("Courier New");
	case QFont::Decorative:
	    return QString::fromLatin1("Bookman Old Style");
	case QFont::Helvetica:
	    return QString::fromLatin1("Arial");
	case QFont::System:
	default:
	    return QString::fromLatin1("MS Sans Serif");
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

QRect QFontPrivate::boundingRect( const QChar &ch )
{
    return QRect( 0,-(fin->ascent()), do_text_task(this, ch, GIMME_WIDTH),
		  fin->ascent() + fin->descent());
}

int QFontPrivate::textWidth( const QString &s, int p, int l)
{
    return do_text_task(this, s, p, l, GIMME_WIDTH);
}
