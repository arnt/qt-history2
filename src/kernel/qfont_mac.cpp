#include <stdlib.h>
#include "qstring.h"
#include "qfont.h"
#include "qfontdata_p.h"
#include "qfontdatabase.h"
#include "qfontmetrics.h"
#include "qfontinfo.h"
#include "qstrlist.h"
#include "qutfcodec.h"
#include "qt_mac.h"
#include "qpaintdevice.h"
#include "qpainter.h"

/* utility functions */
extern const unsigned char * p_str(const char * c);
static bool qstring_to_pstring( QString s, int len, Str255 str, TextEncoding encoding )
{
    UnicodeMapping mapping;
    UnicodeToTextInfo info;
    mapping.unicodeEncoding = CreateTextEncoding( kTextEncodingUnicodeDefault,
						  kTextEncodingDefaultVariant, kUnicode16BitFormat );
    mapping.otherEncoding = encoding;
    mapping.mappingVersion = kUnicodeUseLatestMapping;

    if ( CreateUnicodeToTextInfo( &mapping, &info  ) != noErr )
      Q_ASSERT( 0 );

    const QChar *uc = s.unicode();
    UniChar *buf = new UniChar[len];
    for ( int i = 0; i < len; ++i )
        buf[i] = uc[i].row() << 8 | uc[i].cell();

    ConvertFromUnicodeToPString( info, len*2, buf, str  );
    delete buf;
    return TRUE;
}

/* font information */
class QMacSetFontInfo : public QMacSavedFontInfo 
{
    short setfnum, setface;
    int setsize;
    TextEncoding enc;
public:
    //create this for temporary font settting
    inline QMacSetFontInfo(const QFontPrivate *d) : QMacSavedFontInfo(), enc(0) { setMacFont(d, this); }
    inline TextEncoding encoding() const { return enc; }
    inline short font() const { return setfnum; }
    inline short style() const { return setface; }
    inline int size() const { return setsize; }

    //you can use this to cause font setting, without restoring old
    static bool setMacFont(const QFontPrivate *d, QMacSetFontInfo *sfi=NULL);
};

inline bool QMacSetFontInfo::setMacFont(const QFontPrivate *d, QMacSetFontInfo *sfi)
{
    //face
    Str255 str;
    // encoding == 1, yes it is strange the names of fonts are encoded in MacJapanese
    TextEncoding encoding = CreateTextEncoding( kTextEncodingMacJapanese,
						kTextEncodingDefaultVariant, kTextEncodingDefaultFormat );
    qstring_to_pstring( d->request.family, d->request.family.length(), str, encoding );
    short fnum;
    GetFNum(str, &fnum);
    if(!sfi || fnum != sfi->tfont)
	TextFont(fnum);
    if(sfi) 
	sfi->setfnum = fnum;

    //style
    short face = normal;
    if(d->request.italic)
	face |= italic;
    if(d->request.underline) 
	face |= underline;
    //strikeout? FIXME
    if(d->request.weight == QFont::Bold)
	face |= bold;
    if(!sfi || face != sfi->tface)
	TextFace(face);
    if(sfi) 
	sfi->setface = face;
	
    //size
    int size = d->request.pointSize / 10;
    if(!sfi || size != sfi->tsize)
	TextSize( size );
    if(sfi) 
	sfi->setsize = size;

    if(sfi) {
	if (UpgradeScriptInfoToTextEncoding( FontToScript( fnum ), 
					     kTextLanguageDontCare, kTextRegionDontCare, NULL,
					     &sfi->enc ) != noErr)
	{
	    Q_ASSERT(0);
	}
    }

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

enum text_task { GIMME_WIDTH=1, GIMME_DRAW=2 };
static int do_text_task( const QFontPrivate *d, QString s, int pos, int len, text_task task)
{
    QMacSetFontInfo fi(d);
    FontInfo setfi; GetFontInfo(&setfi);
    OSStatus err;

    const QChar *uc = s.unicode();
    int unilen = len;
    UniChar *unibuf = new UniChar[unilen];
    for ( int i = 0; i < unilen; ++i ) //don't use pos here! FIXME
        unibuf[i] = uc[i+pos].row() << 8 | uc[i+pos].cell();

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
    const int flags = kUnicodeUseFallbacksMask | kUnicodeTextRunMask;
    err = ConvertFromUnicodeToScriptCodeRun( runi, unilen * 2, unibuf, flags,
					     0, NULL, NULL, NULL, buf_len, &read, 
					     &converted, buf, run_len, &run_len, runs);
    if(err != noErr && err != kTECUsedFallbacksStatus) {
	qDebug("unlikely error %d %s:%d", (int)err, __FILE__, __LINE__);
	DisposeUnicodeToTextRunInfo(&runi);
	delete unibuf;
	free(buf);
	return 0;
    }

    int ret = 0, sz = fi.size();
    ScriptCode sc = FontToScript(fi.font());
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
	if(task == GIMME_WIDTH) 
	    ret += TextWidth(buf, off, rlen);
	else if(task == GIMME_DRAW)
	    DrawText(buf, off, rlen);
	else 
	    qDebug("that can't be!");

	//restore the scale
	if(msz != sz)
	    TextSize(sz);
    }
    DisposeUnicodeToTextRunInfo(&runi);
    delete unibuf;
    free(buf);
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

int QFontMetrics::charWidth( const QString &s, int pos ) const
{
    return do_text_task(FI, s, pos, 1, GIMME_WIDTH);
}

int QFontMetrics::width(QChar c) const
{
    return  width( QString( c ) );
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

void QFontPrivate::drawText( QString s, int len )
{
    do_text_task(this, s, 0, len < 1 ? s.length() : len, GIMME_DRAW);
}

void QFontPrivate::load()
{
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

    fin->info = (FontInfo *)malloc(sizeof(FontInfo));
    QMacSetFontInfo fi(this);
    GetFontInfo(fin->info);
    // Our 'handle' is actually a structure with the information needed to load
    // the font into the current grafport
}

void QFont::initialize()
{
    if(!QFontPrivate::fontCache)
	QFontPrivate::fontCache = new QFontCache();
    Q_CHECK_PTR( QFontPrivate::fontCache );
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
    QMacSetFontInfo fi(this);
    return QRect( 0,-(fin->ascent()),CharWidth(ch),fin->ascent() + fin->descent());
}

int QFontPrivate::textWidth( const QString &, int, int )
{
    return 1000;
}
