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
short QFontStruct::currentFnum = 0;
int QFontStruct::currentFsize = 0;

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

int QFontMetrics::charWidth( const QString &str, int pos ) const
{
    return width(str.at(pos));
}

int QFontMetrics::width(QChar c) const
{
    // Grr. How do we force the Mac to speak Unicode?
    // This currently won't work outside of ASCII
    TextFont(FI->fin->fnum);
    TextSize(d->request.pointSize / 10);
    int char_width=CharWidth(c);
    TextFont(QFontStruct::currentFnum);
    TextSize(QFontStruct::currentFsize);
    return char_width;
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
    // Need to make a Pascal string
    char * buf=new char[len+1];
    strncpy(buf,s.ascii(),len);
    int ret;
    TextFont(FI->fin->fnum);
    TextSize(FI->request.pointSize / 10);
    ret=TextWidth(buf,0,len);
    TextFont(QFontStruct::currentFnum);
    TextSize(QFontStruct::currentFsize);
    delete[] buf;
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

int QFontMetrics::leftBearing(QChar ch) const
{
    return 0;
}

int QFontMetrics::rightBearing(QChar ch) const
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

    TextSize(request.pointSize / 10);
    short fnum;
    GetFNum(p_str(request.family.ascii()),&fnum);
    TextFont(fnum);
    QFontStruct::currentFnum = fnum;
    QFontStruct::currentFsize = request.pointSize / 10;

    if(fin)
	fin->fnum = fnum;
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

