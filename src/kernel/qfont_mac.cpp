#include "qstring.h"
#include "qfontdata_p.h"
#include "qfontdatabase.h"
#include "qfontmetrics.h"
#include "qfontinfo.h"
#include "qstrlist.h"
#include "qutfcodec.h"
#include "qt_mac.h"

class QFontInternal {

public:

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

int QFontMetrics::leading() const
{
    FontInfo fi;
    GetFontInfo(&fi);
    return fi.leading;
}

int QFontMetrics::ascent() const
{
    FontInfo fi;
    GetFontInfo(&fi);
    return fi.ascent;
}

int QFontMetrics::descent() const
{
    FontInfo fi;
    GetFontInfo(&fi);
    return fi.descent;
}

int sponges[256];
bool spongy=false;

int QFontMetrics::width(QChar c) const
{
    // Grr. How do we force the Mac to speak Unicode?
    // This currently won't work outside of ASCII
    if(!spongy) {
	for(int loopc=0;loopc<256;loopc++) {
	    sponges[loopc]=-1;
	}
	spongy=true;
    }
    unsigned char f;
    f=c;
    if(f<0 || f>255) {
	return 0;
    }
    if(sponges[f]!=-1) {
	return sponges[f];
    }
    sponges[f]=CharWidth(f);
    return sponges[f];
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
    FontInfo fi;
    GetFontInfo(&fi);
    return fi.widMax;
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

int QFontMetrics::underlinePos() const
{
    return 0;
}

QRect QFontMetrics::boundingRect( const QString &str, int len ) const
{
    if(len<1) {
	len=str.length();
    }
    return QRect(0,0,len*maxWidth(),height());  // Temporary
}

HANDLE QFont::handle() const
{
    if(d->req.dirty) {
	load();
    }
    return 0;
}

void QFont::macSetFont(void * v)
{
    if(v) {
	SetPort((WindowPtr)v);
	TextSize(pointSize());
	short foo;
	char buf[300];
	qstrcpy(buf+1,family().ascii());
	buf[0]=qstrlen(buf+1);
	GetFNum((unsigned char *)buf,&foo);
	TextFont(foo);
    }
}

void QFont::load() const
{
    d->req.dirty=FALSE;
    d->fin=new QFontInternal;
    d->fin->psize=pointSize();
    // Our 'handle' is actually a structure with the information needed to load
    // the font into the current grafport
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
