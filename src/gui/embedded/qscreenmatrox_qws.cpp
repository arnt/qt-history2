/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qgfxmatrox_qws.h"

#ifndef QT_NO_QWS_MATROX

#include "qgfxmatroxdefs_qws.h"
#include "qgfxraster_qws.h"
#include "qimage.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>

// This is the least featureful of the accelerated drivers - use the
// Voodoo3 or Mach64 drivers as better examples. The main interesting
// feature of this driver is the rather primitive way Matrox cards
// (at least early ones) handle providing the offset and linestep of
// data. Since this seems to differ from card to card in ways I've not
// yet deciphered, on-card pixmaps are not supported

static unsigned char *matrox_regbase=0;

//#define DEBUG_INIT

#define LASTOP_LINE 1
#define LASTOP_RECT 2
#define LASTOP_BLT 3
#define LASTOP_BLTPEN 4
#define LASTOP_STRETCHBLT 5
#define LASTOP_RESET 6
#define LASTOP_POLYGON 7
#define LASTOP_TEXT 8
#define LASTOP_ALPHA 9
#define LASTOP_TILEDBLT 10
#define LASTOP_TILEDBLTPEN 11

inline unsigned int matrox_regr(volatile unsigned int regindex)
{
    unsigned long int val;
    val=*((volatile unsigned long *)(matrox_regbase+regindex));
    return val;
}

inline void matrox_regw(volatile unsigned int regindex,unsigned long val)
{
    *((volatile unsigned long int *)(matrox_regbase+regindex))=val;
}

inline void matrox_regw8(volatile unsigned int regindex,unsigned char val)
{
    *((volatile unsigned char *)(matrox_regbase+regindex))=val;
}

inline void matrox_regwx(int index,unsigned char val)
{
    matrox_regw8(PALWTADD,index);
    matrox_regw8(X_DATAREG,val);
}

// Older Matrox hardware has no hardware cursor; leave it for now

template <const int depth, const int type>
class QGfxMatrox : public QGfxRaster<depth,type> {

public:

    QGfxMatrox(unsigned char *,int w,int h);

    virtual void fillRect(int,int,int,int);
    virtual void blt(int,int,int,int,int,int);
    virtual void drawLine(int,int,int,int);
    virtual void sync();

private:

    // Convert colour into what the hardware needs
    unsigned int get_color(unsigned int);

    bool checkSourceDest();
    void setDest();
    void do_scissors(QRect &);

    unsigned int src_pixel_offset;

};

template<const int depth,const int type>
inline void QGfxMatrox<depth,type>::do_scissors(QRect & r)
{
    int t=linestep();
    t=(t*8)/depth;

    matrox_regw(CXLEFT,r.left());
    matrox_regw(CXRIGHT,r.right());
    matrox_regw(YTOP,r.top()*t);
    matrox_regw(YBOT,r.bottom()*t);
}

template<const int depth,const int type>
inline unsigned int QGfxMatrox<depth,type>::get_color(unsigned int i)
{
    unsigned int ret;
    if(depth==32) {
        ret=i;
    } else if(depth==16) {
        i=i & 0xffff;
        ret=(i << 16) | i;
    } else {
        // 8
        i=i & 0xff;
        ret=(i << 24) | (i << 16) | (i << 8) | i;
    }
    return ret;
}

template<const int depth,const int type>
inline void QGfxMatrox<depth,type>::sync()
{
    int loopc;
    for(loopc=0;loopc<1000000;loopc++) {
        unsigned int stat=matrox_regr(MATROX_STATUS);
        if((!(stat & 0x10000)) && loopc>20) {
            return;
        }
    }
    qDebug("Idle timeout!");
}

template<const int depth,const int type>
inline void QGfxMatrox<depth,type>::setDest()
{
    ulong buffer_offset;
    gfx_screen->onCard(buffer,buffer_offset);

    unsigned int b=(buffer_offset >> 6) << 6;
    b=(b*8)/depth;

    int t=linestep();
    t=(t*8)/depth;
    if(t & 0x1f) {
        qDebug("Unaligned pixel linestep %d",t);
        return;
    }

    // fillRect stops working if this isn't done here
    matrox_regw(YTOP,b);
    matrox_regw(YBOT,(height*t)+b);

    QLinuxFb_Shared * tmp=(QLinuxFb_Shared *)shared_data;
    if(tmp->buffer_offset==buffer_offset && tmp->linestep==linestep()) {
      return;
    } else {
      tmp->buffer_offset=buffer_offset;
      tmp->linestep=linestep();
    }

    int d;
    if(depth==16) {
        d=0x1;
    } else if(depth==32) {
        d=0x2;
    } else {
        // 8bpp
        d=0;
    }

    matrox_regw(YDSTORG,b);
    matrox_regw(PITCH,0x8000 | t);
    matrox_regw(MACCESS,d);
    matrox_regw(PLNWT,0xffffffff);

}

template<const int depth,const int type>
inline bool QGfxMatrox<depth,type>::checkSourceDest()
{
    setDest();

    ulong src_buffer_offset;
    if (srctype == SourcePen) {
        src_buffer_offset = -1;
    } else {
        if (!gfx_screen->onCard(srcbits,src_buffer_offset)) {
            return false;
        }
        if(src_buffer_offset & 0x7) {
            qDebug("Unaligned offset %lx",src_buffer_offset);
            return false;
        }
        int srcstep;
        if (srclinestep==0) {
            srcstep=(width*srcdepth)/8;
        } else {
            srcstep=srclinestep;
        }

        src_pixel_offset=(src_buffer_offset * 8)/depth;

    }
    return true;
}

template<const int depth,const int type>
QGfxMatrox<depth,type>::QGfxMatrox(unsigned char * a,int b,int c)
    : QGfxRaster<depth,type>(a,b,c)
{
}

template<const int depth,const int type>
void QGfxMatrox<depth,type>::fillRect(int rx,int ry,int w,int h)
{
    if(ncliprect<1) {
        return;
    }

    if((cbrush.style()!=Qt::NoBrush) && (cbrush.style()!=Qt::SolidPattern)) {
        QGfxRaster<depth,type>::fillRect(rx,ry,w,h);
        return;
    }

    GFX_START(QRect(rx+xoffs, ry+yoffs, w+1, h+1))
    setDest();

    (*gfx_optype)=1;
    (*gfx_lastop)=LASTOP_RECT;

    int loopc;

    int xp=rx+xoffs;
    int yp=ry+yoffs;

    int x2,y2;

    x2=xp+(w-1);
    y2=yp+(h-1);

    int x3,y3,x4,y4;

    QColor tmp=cbrush.color();

    int tmpcol=brushPixel;
    if(((QLinuxFb_Shared *)shared_data)->forecol!=tmpcol) {
        matrox_regw(FCOL,get_color(tmpcol));
        ((QLinuxFb_Shared *)shared_data)->forecol=tmpcol;
    }

    (*gfx_optype)=1;

    QRect tmprect(0,0,width,height);
    do_scissors(tmprect);

    // Last in 1d00-1dff range

    unsigned int tmprop= DWG_REPLACE;

    if(cbrush.style()!=Qt::NoBrush) {
        int p=ncliprect;
        for(loopc=0;loopc<p;loopc++) {
            QRect r=cliprect[loopc];
            if(xp<=r.right() && yp<=r.bottom() &&
               x2>=r.left() && y2>=r.top()) {
                x3 = qMax(r.left(), xp);
                y3 = qMax(r.top(), yp);
                x4 = qMin(r.right(), x2);
                y4 = qMin(r.bottom(), y2);
                int hh=(y4-y3)+1;
                int p=y3;
                int t=linestep();
                t=(t*8)/depth;
                //t&=0x1f;
                p*=(t >> 5);
                matrox_regw(DWGCTL,DWG_TRAP | DWG_SOLID | DWG_ARZERO |
                            DWG_SGNZERO | DWG_SHIFTZERO | DWG_TRANSC | tmprop);
                matrox_regw(FXBNDRY,x3 | ((x4+1) << 16));
                matrox_regw(YDST,p);
                matrox_regw(LEN | EXEC,hh);
            }
        }
    }
    GFX_END
}

template<const int depth,const int type>
void QGfxMatrox<depth,type>::drawLine(int x1,int y1,int x2,int y2)
{
    if(ncliprect<1) {
        return;
    }

    unsigned int tmprop= DWG_REPLACE;

    int dx,dy;
    dx=abs(x2-x1);
    dy=abs(y2-y1);

    GFX_START(QRect(x1, y1 < y2 ? y1 : y2, dx+1, qAbs(dy)+1))
    setDest();

    (*gfx_optype)=1;
    (*gfx_lastop)=LASTOP_LINE;

    x1+=xoffs;
    y1+=yoffs;
    x2+=xoffs;
    y2+=yoffs;

    int loopc;

    int b=dy<dx ? dy : dx;   // min
    int a=dy<dx ? dx : dy;   // max

    unsigned int sgn=0;

    if(dx>dy) {
        sgn |= 0x1;
    }
    if(x2<x1) {
        sgn |= 0x2;
    }
    if(y2<y1) {
        sgn |= 0x4;
    }

    for(loopc=0;loopc<ncliprect;loopc++) {
        do_scissors(cliprect[loopc]);
        QColor tmp=cpen.color();
        int tmp2=penPixel;

        if(((QLinuxFb_Shared *)shared_data)->forecol!=tmp2) {
            matrox_regw(FCOL,tmp2);
            ((QLinuxFb_Shared *)shared_data)->forecol=tmp2;
        }

        if(dashedLines) {
            unsigned int tmp=0;
            for(int loopc=0;loopc<numDashes;loopc++) {
                // Fill unsigned int with up to 32 bits
                tmp &= dashes[loopc] << ((loopc % 4) * 8);
                if((loopc % 4)==3) {
                    // When 4th byte is filled, fill SRC1-3
                    // with it (these registers are contiguous and
                    // 4 bytes apart)
                    matrox_regw(SRC0+(loopc-3),tmp);
                    tmp=0;
                }
            }
            if(numDashes<4) {
                matrox_regw(SRC0,tmp);
            }
            matrox_regw(SHIFT,((numDashes*8)-1) << 16);
            matrox_regw(DWGCTL,DWG_LINE_CLOSE | tmprop | DWG_BFCOL);
        } else {
            matrox_regw(DWGCTL,DWG_LINE_CLOSE | tmprop | DWG_SHIFTZERO |
                    DWG_SOLID | DWG_BFCOL);
        }

        matrox_regw(AR0,b*2);
        matrox_regw(AR1,(b*2)-a-(y2-y1));
        matrox_regw(AR2,(b*2)-(a*2));
        matrox_regw(SGN,sgn);
        matrox_regw(XDST,x1);
        int p=y1;
        int t=linestep();
        t=(t*8)/depth;
        //t&=0x1f;
        p*=(t >> 5);
        matrox_regw(YDST,p);
        matrox_regw(LEN | EXEC,a);  // Vector length
    }

    GFX_END
}

template<const int depth,const int type>
inline void QGfxMatrox<depth,type>::blt(int rx,int ry,int w,int h,int sx,int sy)
{
    if(ncliprect<1)
        return;

    bool canaccel=false;

    if((srcdepth==32 || srcdepth==16 || srcdepth==8) &&
       alphatype==IgnoreAlpha) {
        canaccel=true;
    }

    if(srctype==SourceImage && canaccel==false) {
        QGfxRaster<depth,type>::blt(rx,ry,w,h,sx,sy);
        return;
    }

    if(srctype==SourcePen) {
        QGfxRaster<depth,type>::blt(rx,ry,w,h,sx,sy);
        return;
    }

    int xp=xoffs+rx;
    int yp=yoffs+ry;
    QRect cursRect(xp, yp, w+1, h+1);
    GFX_START(cursRect)

    if(checkSourceDest()) {

        unsigned int tmprop= DWG_REPLACE;
        int xp2=srcwidgetoffs.x() + sx;
        int yp2=srcwidgetoffs.y() + sy;

        int mx = qMin(xp,xp2);
        if (mx < 0) {
            //Matrox does not like blt to/from negative X coords
            //so we clip against left edge of screen.
            xp -= mx;
            xp2 -= mx;
            w += mx;
        }


        (*gfx_optype)=1;
        (*gfx_lastop)=LASTOP_BLT;

        bool rev = (yp > yp2) || ((yp == yp2) && (xp > xp2));
        int dy = (yp > yp2) ? -1 : 1;
        int dx = (xp > xp2) ? -1 : 1;

        int loopc = (dy<0) ? ncliprect-1 : 0;

        while (loopc >=0 && loopc < ncliprect) {

            int ylevel = cliprect[loopc].y();
            if (dx != dy) {
                // find other end of strip
                while (loopc >=0 && loopc < ncliprect
                        && cliprect[loopc].y() == ylevel)
                    loopc -= dx;
                loopc += dx;
            }
            int end_of_strip = loopc;
            do {

                do_scissors(cliprect[loopc]);

                int tw=w;
                int th=h;
                int start,end;
                int typ=yp;

                int src_pixel_linestep=(srclinestep*8)/srcdepth;

                if (!rev) {
                    matrox_regw(AR5,src_pixel_linestep);
                    matrox_regw(DWGCTL,DWG_BITBLT | DWG_SHIFTZERO |
                                DWG_SGNZERO | DWG_BFCOL | tmprop);
                    tw--;
                    start=yp2*src_pixel_linestep+xp2+src_pixel_offset;
                    end=start+tw;
                } else {
                    matrox_regw(SGN,5);
                    matrox_regw(AR5,-src_pixel_linestep);
                    matrox_regw(DWGCTL,DWG_BITBLT | DWG_SHIFTZERO | DWG_BFCOL |
                                tmprop);
                    tw--;
                    end=(yp2+th-1)*src_pixel_linestep+xp2+src_pixel_offset;
                    start=end+tw;
                    typ+=th-1;
                }
                matrox_regw(AR0,end);
                matrox_regw(AR3,start);
                matrox_regw(FXBNDRY,((xp+tw)<<16) | xp);
                int p=typ;
                int t=linestep();
                t=(t*8)/depth;
                //t&=0x1f;
                p*=(t >> 5);
                matrox_regw(YDST,p);
                matrox_regw(LEN | EXEC,th);

                loopc += dx;
            } while (loopc >= 0 && loopc < ncliprect &&
                      cliprect[loopc].y() == ylevel);

            //find next strip
            if (dx != dy)
                loopc = end_of_strip - dx;

        }
        QRect r(0,0,width,height);
        do_scissors(r);

        GFX_END

    } else {

        GFX_END

        QGfxRaster<depth,type>::blt(rx,ry,w,h,sx,sy);
    }
}

class QMatroxCursor : public QScreenCursor
{
public:
    QMatroxCursor();
    ~QMatroxCursor();

    virtual void init(SWCursorData *,bool=false);

    virtual void set(const QImage &image, int hotx, int hoty);
    virtual void move(int x, int y);
    virtual void show();
    virtual void hide();

    virtual bool restoreUnder(const QRect &)
                { return false; }
    virtual void saveUnder() {}
    virtual void drawCursor() {}
    virtual void draw() {}
    virtual bool supportsAlphaCursor() { return false; }

    static bool enabled() { return false; }

private:

    int hotx;
    int hoty;

};


QMatroxScreen::QMatroxScreen(int display_id )
    : QLinuxFbScreen(display_id)
{
}

bool QMatroxScreen::useOffscreen() { return false; }
int QMatroxScreen::pixmapOffsetAlignment() { return 256; }
int QMatroxScreen::pixmapLinestepAlignment() { return 256; }

// #define DEBUG_INIT

bool QMatroxScreen::connect(const QString &spec)
{
    if (!QLinuxFbScreen::connect(spec)) {
        qDebug("Matrox driver couldn't connect to framebuffer");
        return false;
    }

    canaccel=false;

    const unsigned char* config = qt_probe_bus();

    if(!config)
        return false;

    unsigned short int * manufacturer=(unsigned short int *)config;
    if(*manufacturer!=0x102b) {
        qDebug("This does not appear to be a Matrox card");
        qDebug("Are you sure QWS_CARD_SLOT is pointing to the right entry in "
               "/proc/bus/pci?");
        return false;
    }

    const unsigned char * bar=config+0x14;
    const unsigned long int * addr=(const unsigned long int *)bar;
    unsigned long int s=*(addr+0);
    unsigned long int olds=s;
    if(s & 0x1) {
#ifdef DEBUG_INIT
        printf("IO space - not right\n");
#endif
        return false;
    } else {
#ifdef DEBUG_INIT
        qDebug("First address thing look right");
#endif
        s=s >> 1;
        s=s >> 2;
        s=olds;
        unsigned char * membase;
        int aperturefd;
        aperturefd=open("/dev/mem",O_RDWR);
        if(aperturefd==-1) {
#ifdef DEBUG_INIT
            qDebug("Can't open /dev/mem");
#endif
            return false;
        }
        s=(s >> 4) << 4;
#ifdef DEBUG_INIT
        qDebug("Using physical address %lx, mapping %d",s,0x3fff);
#endif
        membase=(unsigned char *)mmap(0,0x3fff,PROT_READ |
                                      PROT_WRITE,MAP_SHARED,
                                      aperturefd,s);
        if(membase==0 || membase==(unsigned char *)-1) {
#ifdef DEBUG_INIT
            qDebug("Failure to mmap /dev/mem, offset %ld, %s",s,
                   strerror(errno));
#endif
            close(aperturefd);
            return false;
        }
        matrox_regbase=membase;
    }
#ifdef DEBUG_INIT
    qDebug("Detected Matrox card");
#endif
    canaccel=true;

    return true;
}


QMatroxScreen::~QMatroxScreen()
{
}

bool QMatroxScreen::initDevice()
{
    QLinuxFbScreen::initDevice();
    return true;
}

void QMatroxScreen::shutdownDevice()
{
    qt_screencursor->hide();
    QLinuxFbScreen::shutdownDevice();
}

QGfx * QMatroxScreen::createGfx(unsigned char * b,int w,int h,int d,
                                int linestep)
{
    QGfx * ret=0;
    if(onCard(b)) {
        if (false) {
            //Just to simplify the ifdeffery
#ifndef QT_NO_QWS_DEPTH_16
        } else if(d==16) {
            ret = new QGfxMatrox<16,0>(b,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_32
        } else if (d==32) {
            ret = new QGfxMatrox<32,0>(b,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_8
        } else if (d==8) {
            ret = new QGfxMatrox<8,0>(b,w,h);
#endif
        }
        if(ret) {
            ret->setShared(shared);
            ret->setLineStep(linestep);
            return ret;
        }
    }
    return QLinuxFbScreen::createGfx(b,w,h,d,linestep);
}

int QMatroxScreen::initCursor(void* e, bool init)
{
#ifndef QT_NO_QWS_CURSOR
    extern bool qws_sw_cursor;

    if(qws_sw_cursor==true) {
        return QLinuxFbScreen::initCursor(e,init);
    }
    qt_screencursor=new QMatroxCursor();
    qt_screencursor->init(0,false);
#endif
    return 0;
}

extern bool qws_accel;

extern "C" QScreen * qt_get_screen_matrox(int display_id)
{
    return new QMatroxScreen(display_id);
}

QMatroxCursor::QMatroxCursor()
{

}

QMatroxCursor::~QMatroxCursor()
{
}

void QMatroxCursor::init(SWCursorData *,bool)
{
    myoffset=(qt_screen->width()*qt_screen->height()*qt_screen->depth())/8;
    myoffset+=8;
    fb_start=qt_screen->base();
}

int matrox_ngval(QRgb r)
{
    if(qAlpha(r)<255) {
        return 0;        // Transparent
    } else if(qBlue(r)>240) {
        return 3;        // White
    } else {
        return 2;        // Black
    }
}

void QMatroxCursor::set(const QImage& image,int hx,int hy)
{
    cursor=(QImage*)&image;
    hotx=hx;
    hoty=hy;

    matrox_regwx(XCURCTL,0x3);  // X-style cursor

    if(cursor->isNull()) {
        qDebug("Null cursor image!");
        abort();
        return;
    }

    // 64-bit align it
    unsigned int offset=myoffset;

    while(offset & 0x7ff)
        offset++;

    int loopc,loopc2;

    unsigned char * tmp;

    //offset-=4;

    // 3=white, 1=black?

    unsigned long * tmp2;

    tmp2=(unsigned long *)(fb_start+offset);

    for(loopc=0;loopc<64;loopc++) {
        for(loopc2=0;loopc2<4;loopc2++) {
            *(tmp2++)=0;
        }
    }

    for(loopc=0;loopc<cursor->height();loopc++) {
        tmp=fb_start+offset+(loopc*16);
        int count=1;
        for(loopc2=0;loopc2<(cursor->width()/8);loopc2++) {
            unsigned int v1,v2,v3,v4,v5,v6,v7,v8;
            unsigned int pos=loopc2*8;
            v8=matrox_ngval(cursor->pixel(pos,loopc)) & 1;
            v7=matrox_ngval(cursor->pixel(pos+1,loopc)) & 1;
            v6=matrox_ngval(cursor->pixel(pos+2,loopc)) & 1;
            v5=matrox_ngval(cursor->pixel(pos+3,loopc)) & 1;
            v4=matrox_ngval(cursor->pixel(pos+4,loopc)) & 1;
            v3=matrox_ngval(cursor->pixel(pos+5,loopc)) & 1;
            v2=matrox_ngval(cursor->pixel(pos+6,loopc)) & 1;
            v1=matrox_ngval(cursor->pixel(pos+7,loopc)) & 1;
            unsigned char put=(v8 << 7) | (v7 << 6) | (v6 << 5) |
                              (v5 << 4) | (v4 << 3) | (v3 << 2) |
                              (v2 << 1) | v1;
            *(tmp+count)=put;
            count--;
            if(count==-1) {
                count=1;
                tmp+=2;
            }
        }
        tmp=fb_start+offset+(loopc*16)+8;
        count=1;
        for(loopc2=0;loopc2<(cursor->width()/8);loopc2++) {
            unsigned int v1,v2,v3,v4,v5,v6,v7,v8;
            unsigned int pos=loopc2*8;
            v8=matrox_ngval(cursor->pixel(pos,loopc)) >> 1;
            v7=matrox_ngval(cursor->pixel(pos+1,loopc)) >> 1;
            v6=matrox_ngval(cursor->pixel(pos+2,loopc)) >> 1;
            v5=matrox_ngval(cursor->pixel(pos+3,loopc)) >> 1;
            v4=matrox_ngval(cursor->pixel(pos+4,loopc)) >> 1;
            v3=matrox_ngval(cursor->pixel(pos+5,loopc)) >> 1;
            v2=matrox_ngval(cursor->pixel(pos+6,loopc)) >> 1;
            v1=matrox_ngval(cursor->pixel(pos+7,loopc)) >> 1;
            unsigned char put=(v8 << 7) | (v7 << 6) | (v6 << 5) |
                              (v5 << 4) | (v4 << 3) | (v3 << 2) |
                              (v2 << 1) | v1;
            *(tmp+count)=put;
            count--;
            if(count==-1) {
                count=1;
                tmp+=2;
            }
        }
    }

    matrox_regwx(XCURADDL,(offset >> 10) & 0xff);
    matrox_regwx(XCURADDH,(offset >> 18) & 0xff);
    matrox_regwx(XCURCOL0RED,0);
    matrox_regwx(XCURCOL0GREEN,0);
    matrox_regwx(XCURCOL0BLUE,0);
    matrox_regwx(XCURCOL1RED,0xff);
    matrox_regwx(XCURCOL1GREEN,0xff);
    matrox_regwx(XCURCOL1BLUE,0xff);
}

void QMatroxCursor::hide()
{
    matrox_regwx(XCURCTL,0);
}

void QMatroxCursor::show()
{
    matrox_regwx(XCURCTL,0x3);
}

void QMatroxCursor::move(int x,int y)
{
    x-=hotx;
    y-=hoty;
    x+=cursor->width();
    y+=64;
    matrox_regw(CURPOS,(y << 16) | x);  // bottom left?
}

#endif // QT_NO_QWS_MATROX
