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

extern const unsigned char * p_str(const char * c);

void qstring_to_pstring( QString s, int len, Str255 str, TextEncoding encoding )
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
}

short QFontStruct::currentFStyle = normal;
short QFontStruct::currentFnum = 0;
int QFontStruct::currentFsize = 0;
TextEncoding QFontStruct::currentEncoding = 0;

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

int char_widths[256];
bool chars_init=false;

int QFontMetrics::charWidth( const QString &s, int pos ) const
{
    int ret;
    Str255 str;
    qstring_to_pstring( s, s.length(), str, QFontStruct::currentEncoding );
    TextFont( FI->fin->fnum );
    TextSize( FI->request.pointSize / 10 );
    //FIXME: This may not work correctly if str contains double byte characters
    ret = TextWidth( &str[1], pos, 1 );
    TextFont( QFontStruct::currentFnum );
    TextSize( QFontStruct::currentFsize );
    return ret;
}

int QFontMetrics::width(QChar c) const
{
    return  width( QString( c ) );
}

#if 0
const QFontDef *QFontMetrics::spec() const
{
    if ( painter ) {
	painter->cfont.handle();
	return painter->cfont.d->fin->spec();
    } else {
	return d->fin->spec();
    }
}
#endif

int QFontMetrics::width(const QString &s,int len) const
{
    if(len<1) {
	len=s.length();
    }
    int ret;
    Str255 str;
    qstring_to_pstring( s, len, str, QFontStruct::currentEncoding );
    TextFont( FI->fin->fnum );
    TextSize( FI->request.pointSize / 10 );
    ret = TextWidth( &str[1], 0, str[0] );
    TextFont( QFontStruct::currentFnum );
    TextSize( QFontStruct::currentFsize );
    return ret;
}

int QFontMetrics::maxWidth() const
{
    return FI->fin->maxWidth();
}

int QFontMetrics::height() const
{
    return ascent()+descent()+1;
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
    return QRect( 0,-(ascent()),width(str,len)+5,height()+5);
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

    //style
    QFontStruct::currentFStyle = normal;
    if(request.italic)
	QFontStruct::currentFStyle |= italic;
    if(request.underline) 
	QFontStruct::currentFStyle |= underline;
    //strikeout? FIXME
    if(request.weight == QFont::Bold)
	QFontStruct::currentFStyle |= bold;

    //size
    QFontStruct::currentFsize = request.pointSize / 10;

    //font
    Str255 str;
    // encoding == 1, yes it is strange the names of fonts are encoded in MacJapanese
    TextEncoding encoding = CreateTextEncoding( kTextEncodingMacJapanese,
        kTextEncodingDefaultVariant, kTextEncodingDefaultFormat );
    qstring_to_pstring( request.family, request.family.length(), str, encoding );
    GetFNum(str, &QFontStruct::currentFnum);

    //actually set things now
    TextFont(QFontStruct::currentFnum);
    TextSize(QFontStruct::currentFsize);
    TextFace(QFontStruct::currentFStyle);

    if (UpgradeScriptInfoToTextEncoding( FontToScript( QFontStruct::currentFnum ), 
        kTextLanguageDontCare, kTextRegionDontCare, NULL,
	&QFontStruct::currentEncoding ) != noErr)
        Q_ASSERT(0);

    if(fin)
	fin->fnum = QFontStruct::currentFnum;
}

void QFontPrivate::drawText( QString s, int len )
{
    Str255 str;
    qstring_to_pstring( s, len, str, QFontStruct::currentEncoding );
    DrawString( str  );
}

void QFontPrivate::load()
{
    request.dirty=FALSE;

#if 0
    QString k = key();
    QFontStruct* qfs = fontCache->find(k);
    if ( !qfs ) {
	qfs = new QFontStruct(request);
	fontCache->insert(k, qfs, 1);
    }
    qfs->ref();
#else
    static QFontStruct *blah = NULL;
    if(!blah) 
	blah = new QFontStruct(request);
    QFontStruct *qfs = blah;
#endif	

    if(fin) 
	fin->deref();
    fin=qfs;

    macSetFont(NULL);
    fin->info = (FontInfo *)malloc(sizeof(FontInfo));
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

//

#if 0
const QFontDef *QFontInfo::spec() const
{
    if ( painter ) {
	painter->cfont.handle();
	return painter->cfont.d->fin->spec();
    } else {
	return fin->spec();
    }
}
#endif

void QFont::cacheStatistics()
{
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
    // Grr. How do we force the Mac to speak Unicode?
    // This currently won't work outside of ASCII
    TextFont(fin->fnum);
    TextSize(request.pointSize / 10);
    int char_width=CharWidth(ch);
    TextFont(QFontStruct::currentFnum);
    TextSize(QFontStruct::currentFsize);
    return QRect( 0,-(fin->ascent()),char_width+5,fin->ascent() + fin->descent()+5);
}

int QFontPrivate::textWidth( const QString &str, int pos, int len )
{
    return 1000;
}
