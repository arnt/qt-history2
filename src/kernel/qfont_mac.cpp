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

class QFontInternal {
public:
    FontInfo info;;
    short fnum;
    int psize;
};

int QFontMetrics::lineSpacing() const
{
    return leading()+height();
}

int QFontMetrics::lineWidth() const
{
    return 1;
}

#undef FI
#if 1
#define FI (painter ? painter->cfont.d->fin->info : d->fin->info)
#else
#define FI foo()
FontInfo foo() { FontInfo bar; GetFontInfo(&bar); return bar; }
#endif

int QFontMetrics::leading() const
{
    return FI.leading;
}

int QFontMetrics::ascent() const
{
    return FI.ascent+2; //2?? fixme!
}

int QFontMetrics::descent() const
{
    return FI.descent; //2?? fixme!
}

int char_widths[256];
bool chars_init=false;

int QFontMetrics::width(QChar c) const
{
    // Grr. How do we force the Mac to speak Unicode?
    // This currently won't work outside of ASCII
    if(!chars_init) {
	for(int loopc=0;loopc<256;loopc++) {
	    char_widths[loopc]=-1;
	}
	chars_init=true;
    }
    unsigned char f=c;
    if(char_widths[f]!=-1) {
	return char_widths[f];
    }
    char_widths[f]=CharWidth(f);
    return char_widths[f];
}

int QFontMetrics::width(const QString &s,int len) const
{
    if(len<1) {
	len=s.length();
    }
    // Need to make a Pascal string
    char * buf=new char[len+1];
    strncpy(buf,s.ascii(),len);
    int ret;
    ret=TextWidth(buf,0,len);
    delete[] buf;
    return ret;
}

int QFontMetrics::maxWidth() const
{
    return FI.widMax;
}

int QFontMetrics::height() const
{
    return ascent()+descent()+1;
}

int QFontMetrics::minRightBearing() const
{
    return 0;
}

int QFontMetrics::minLeftBearing() const
{
    return 0;
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
    if(len<1) 
	len=str.length();
    return QRect(0,0,len*maxWidth(),height());  // Temporary
}

Qt::HANDLE QFont::handle() const
{
    if(d->req.dirty) {
	load();
    }
    return 0;
}

void QFont::macSetFont(QPaintDevice *v)
{
    if(v && !v->paintingActive()) {
	qDebug("I was really hoping it would never come to this...");
	ASSERT(0); //we need to figure out if this can really happen
    }

    TextSize(pointSize());
    short fnum;
    GetFNum(p_str(family().ascii()),&fnum);
    TextFont(fnum);

    if(d && d->fin)
	d->fin->fnum = fnum;
}

void QFont::load() const
{
    d->req.dirty=FALSE;
    d->fin=new QFontInternal;
    d->fin->psize=pointSize();
    ((QFont *)this)->macSetFont(NULL);
    GetFontInfo(&d->fin->info);

    // Our 'handle' is actually a structure with the information needed to load
    // the font into the current grafport
}

int qFontGetWeight( const QCString &weightString, bool adjustScore )
{
    // Test in decreasing order of commonness
    //
    if ( weightString == "medium" )       return QFont::Normal;
    else if ( weightString == "bold" )    return QFont::Bold;
    else if ( weightString == "demibold") return QFont::DemiBold;
    else if ( weightString == "black" )   return QFont::Black;
    else if ( weightString == "light" )   return QFont::Light;

    QCString s = weightString;
    s = s.lower();
    if ( s.contains("bold") ) {
	if ( adjustScore )
	    return (int) QFont::Bold - 1;  // - 1, not sure that this IS bold
	else
	    return (int) QFont::Bold;
    }
    if ( s.contains("light") ) {
	if ( adjustScore )
	    return (int) QFont::Light - 1; // - 1, not sure that this IS light
       else
	    return (int) QFont::Light;
    }
    if ( s.contains("black") ) {
	if ( adjustScore )
	    return (int) QFont::Black - 1; // - 1, not sure this IS black
	else
	    return (int) QFont::Black;
    }
    if ( adjustScore )
	return (int) QFont::Normal - 2;	   // - 2, we hope it's close to normal

    return (int) QFont::Normal;
}

/*!
  Internal function that initializes the font system.

  \internal
  The font cache and font dict do not alloc the keys. The key is a QString
  which is shared between QFontInternal and QXFontName.
*/

void QFont::initialize()
{
#if 0
    if ( fontCache )
	return;
    fontCache = new QFontCache( fontCacheSize, 29 );
    CHECK_PTR( fontCache );
    fontDict  = new QFontDict( 29 );
    CHECK_PTR( fontDict );
    fontNameDict = new QFontNameDict( 29 );
    CHECK_PTR( fontNameDict );
    fontNameDict->setAutoDelete( TRUE );
#endif
}

void QFont::setPixelSizeFloat( float pixelSize )
{
  //    setPointSizeFloat( pixelSize * 72.0 / QPaintDevice::x11AppDpiY() );
}

//

const QFontDef *QFontInfo::spec() const
{
    return 0;
}

QUtf8Codec * quc=0;

const QTextCodec * QFontData::mapper() const
{
    if(!quc) {
	quc=new QUtf8Codec();
    }
    return quc;
}

QFont::CharSet QFont::defaultCharSet=QFont::AnyCharSet;










