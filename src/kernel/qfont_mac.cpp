#include "qstring.h"
#include "qfontdata.h"
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
  //printf("%s %d\n",__FILE__,__LINE__);
  // Grr. This all works within the current grafport.
  return leading()+height();
}

int QFontMetrics::lineWidth() const
{
  //printf("%s %d\n",__FILE__,__LINE__);
  return 1;
}

int QFontMetrics::leading() const
{
  //printf("%s %d\n",__FILE__,__LINE__);
  FontInfo fi;
  GetFontInfo(&fi);
  return fi.leading;
}

int QFontMetrics::ascent() const
{
  //printf("%s %d\n",__FILE__,__LINE__);
  FontInfo fi;
  GetFontInfo(&fi);
  return fi.ascent;
}

int QFontMetrics::descent() const
{
  //printf("%s %d\n",__FILE__,__LINE__);
  FontInfo fi;
  GetFontInfo(&fi);
  return fi.descent;
}

int sponges[256];
bool spongy=false;

int QFontMetrics::width(QChar c) const
{
  //printf("%s %d\n",__FILE__,__LINE__);
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
  //printf("%s %d\n",__FILE__,__LINE__);
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
  //printf("%s %d\n",__FILE__,__LINE__);
  FontInfo fi;
  GetFontInfo(&fi);
  return fi.widMax;
}

int QFontMetrics::height() const
{
  //printf("QFontMetrics::height: %s %d\n",__FILE__,__LINE__);
  return ascent()+descent()+1;
}

int QFontMetrics::minRightBearing() const
{
  //printf("%s %d\n",__FILE__,__LINE__);
  return 0;
}

int QFontMetrics::minLeftBearing() const
{
  //printf("%s %d\n",__FILE__,__LINE__);
  return 0;
}

int QFontMetrics::underlinePos() const
{
  //printf("%s %d\n",__FILE__,__LINE__);
  return 0;
}

QRect QFontMetrics::boundingRect( const QString &str, int len ) const
{
  //printf("%s %d\n",__FILE__,__LINE__);
  if(len<1) {
    len=str.length();
  }
  return QRect(0,0,len*maxWidth(),height());  // Temporary
}

  /*
  QStrList * qsl=new QStrList();
  unsigned char buf[256];
  int loopc=0;
  //printf("Font dump:\n");
  char oldbuf[256];
  strcpy(oldbuf,"Wibble!");
  do {
    GetFontName(loopc,buf);
    buf[buf[0]+1]='\0';
    if(strcmp((char *)buf,oldbuf) && buf[0]) {
      //printf("%d: %s\n",loopc,buf+1);
      qsl->append((char *)buf+1);
      strcpy(oldbuf,(char *)buf);
    }
    loopc++;
    //} while(buf[0]);
  } while(loopc<200);
  return *qsl;
  */

/*
QFont * QFont::defFont=0;
bool QFont::defRezAdj=FALSE;
*/

/*
QFont::QFont(Internal)
{
  //printf("%s %d\n",__FILE__,__LINE__);
  init();
  d->req.family="Charcoal";
  d->req.pointSize=11*10;
  d->req.weight=QFont::Normal;
  d->req.rawMode=FALSE;
  d->req.dirty=TRUE;
  d->fin=0;
}
*/

HANDLE QFont::handle() const
{
  //printf("%s %d\n",__FILE__,__LINE__);
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
    strcpy(buf+1,family().ascii());
    buf[0]=strlen(buf+1);
    GetFNum((unsigned char *)buf,&foo);
    //printf("Setting fnum %d for %s\n",foo,family().ascii());
    TextFont(foo);
  }
}

void QFont::load() const
{
  //printf("QFont::load: %s %d\n",__FILE__,__LINE__);
  //printf("Name %s\n",family().ascii());
  //printf("Size %d\n",pointSize());
  d->req.dirty=FALSE;
  d->fin=new QFontInternal;
  d->fin->psize=pointSize();
  // Our 'handle' is actually a structure with the information needed to load
  // the font into the current grafport
}

//

const QFontDef *QFontInfo::spec() const
{
  //printf("%s %d\n",__FILE__,__LINE__);
  return 0;
}

/*
QFont QFontStyle::font(int) const
{
  return QFont();
}

bool QFontStyle::isNull() const
{
  return false;
}

bool QFontCharSet::isNull() const
{
  return false;
}

bool QFontFamily::isNull() const
{
  return false;
}

const QFontStyle& QFontCharSet::style(const QString &) const
{
  return QFontStyle();
}

const QFontFamily& QFontDatabase::family(const QString &) const
{
  return QFontFamily();
}

const QFontCharSet& QFontFamily::charSet(const QString &) const
{
  return QFontCharSet();
}

QUtf8Codec * quc=0;

const QTextCodec * QFontData::mapper() const
{
  if(!quc) {
    quc=new QUtf8Codec();
  }
  return quc;
}
*/