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

static int qstring_to_text( QString s, int len, Str255 str, TextEncoding encoding )
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
    //TODO determine which errors are fatal
#if 0
    int err = ConvertFromUnicodeToPString( info, len*2, buf, str  );
    if (err != noErr) {
        qDebug( s );
        qDebug( "ConvertFromUnicodeToPString %d", err );
	//        Q_ASSERT( 0 );
    }
#endif
    delete buf;
    
    int ret = str[0];
    memcpy(str, str+1, ret);
    return ret;
}

/* font information */
class QMacSetFontInfo : public QMacSavedFontInfo 
{
    TextEncoding enc;
public:
    //create this for temporary font settting
    inline QMacSetFontInfo(const QFontPrivate *d) : QMacSavedFontInfo(), enc(0) { setMacFont(d, this); }
    inline TextEncoding encoding() { return enc; }

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
    Str255 str;
    QMacSetFontInfo fi(FI);
    //FIXME: This may not work correctly if str contains double byte characters
    qstring_to_text( s, s.length(), str, fi.encoding() );
    return TextWidth( str, pos+1, 1 );
}

int QFontMetrics::width(QChar c) const
{
    return  width( QString( c ) );
}

int QFontMetrics::width(const QString &s,int len) const
{
    Str255 str;
    QMacSetFontInfo fi(FI);
    //FIXME: This may not work correctly if str contains double byte characters
    int olen = qstring_to_text( s, len < 1 ? s.length() : len, str, fi.encoding() );
    return TextWidth( str, 1, olen);
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
    Str255 str;
    QMacSetFontInfo fi(this);
    int olen = qstring_to_text( s, len, str, fi.encoding() );
    DrawText( str, 0, olen);
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
