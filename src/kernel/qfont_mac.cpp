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
    int fnum;
    TextEncoding enc;
public:
    //create this for temporary font settting
    inline QMacSetFontInfo(const QFontPrivate *d) : QMacSavedFontInfo(), enc(0) { setMacFont(d, this); }
    inline TextEncoding encoding() const { return enc; }
    inline short font() const { return fnum; }

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
	sfi->fnum = fnum;

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
	
    //size
    int size = d->request.pointSize / 10;
    if(!sfi || size != sfi->tsize)
	TextSize( size );

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

enum text_task { GIMME_WIDTH, GIMME_DRAW };
static int do_text_task( const QFontPrivate *d, QString s, int pos, int len, text_task task)
{
    QMacSetFontInfo fi(d);
    OSStatus err;

    const QChar *uc = s.unicode();
    UniChar *unibuf = new UniChar[len];
    for ( int i = pos; i < len; ++i ) //don't use pos there FIXME!!
        unibuf[i] = uc[i].row() << 8 | uc[i].cell();

    //create converter
    UnicodeToTextRunInfo runi;
    ItemCount scpts = 1 << 31; //hight bit
    short scpt[1]; 
    scpt[0] = FontToScript( fi.font() );
    err =  CreateUnicodeToTextRunInfoByScriptCode(scpts, scpt, &runi);
    if(err != noErr) {
	qDebug("unlikely error %d %s:%d", (int)err, __FILE__, __LINE__);
	return 0;
    }

    //now convert
    int buf_len = 2056;     //buffer
    uchar *buf = (uchar *)malloc(buf_len);
    ItemCount run_len = 20; //runs
    ScriptCodeRun runs[run_len];
    ByteCount read, converted; //returns
    err = ConvertFromUnicodeToScriptCodeRun( runi, len * 2, unibuf, kUnicodeUseFallbacksMask,
					     0, NULL, NULL, NULL, buf_len, &read, 
					     &converted, buf, run_len, &run_len, runs);
    if(err != noErr && err != kTECUsedFallbacksStatus) {
	qDebug("unlikely error %d %s:%d", (int)err, __FILE__, __LINE__);
	delete unibuf;
	free(buf);
	return 0;
    }

    //now do the task
    int ret = 0;
    if(run_len) {
	for(ItemCount i = 0; i < run_len; i++) {
	    if(runs[i].script)
		TextFont(GetScriptVariable(runs[i].script, smScriptSysFond));
	    else
		TextFont(fi.font());
	    ByteOffset off = runs[i].offset;
	    int len = ((i == run_len - 1) ? converted : runs[i+1].offset) - off;
	    if(task == GIMME_WIDTH) 
		ret += TextWidth(buf, off, len);
	    else if(task == GIMME_DRAW) 
		DrawText(buf, off, len);
	}
    }
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
    //FIXME: This may not work correctly if str contains double byte characters
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
    return 0;
}

int QFontMetrics::underlinePos() const
{
    return 0;
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
