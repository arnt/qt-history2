#include "qpaintdevice.h"
#include "qpaintdevicemetrics.h"
#include "qt_mac.h"

QPaintDevice::QPaintDevice(uint devflags)
{
    devFlags=devflags;
    painters=0;
    hd=0;
}

QPaintDevice::~QPaintDevice()
{
}

bool QPaintDevice::cmd(int,QPainter *,QPDevCmdParam *)
{
    return true;
}

int QPaintDevice::metric(int) const
{
    return 0;
}

int QPaintDevice::fontMet(QFont *,int,const char *,int) const
{
    return 0;
}

int QPaintDevice::fontInf(QFont *,int) const
{
    return 0;
}

void bitBlt( QPaintDevice *dst, int dx, int dy,
             const QPaintDevice *src, int sx, int sy, int sw, int sh,
             Qt::RasterOp rop, bool ignoreMask )
{
    if(dx+sw>dst->metric(QPaintDeviceMetrics::PdmWidth)) {
	sw=dst->metric(QPaintDeviceMetrics::PdmWidth)-dx;
    }
    if(dy+sh>dst->metric(QPaintDeviceMetrics::PdmHeight)) {
	sh=dst->metric(QPaintDeviceMetrics::PdmHeight)-dy;
    }
    Rect r;
    SetRect(&r,sx,sy,sx+sw,sy+sh);
    Rect r2;
    SetRect(&r2,dx,dy,dx+sw,dy+sh);
    dst->fixport();
    CopyBits((BitMap *)*(((CGrafPtr)src->handle())->portPixMap),
	     (BitMap *)*(((CGrafPtr)dst->handle())->portPixMap),
	     &r,&r2,(short)srcCopy,(MacRegion **)0);
}

HANDLE QPaintDevice::handle() const
{
    return hd;
}

/*!
  \internal
  Set's QuickDraw's ugly global variable.
*/
void QPaintDevice::fixport()
{
    if(handle())
	SetPort((WindowPtr)handle());
}













