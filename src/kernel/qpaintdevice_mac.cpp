#include "qpaintdevice.h"
#include "qpaintdevicedefs.h"
#include <stdio.h>

QPaintDevice::QPaintDevice(uint devflags)
{
  printf("%s %d\n",__FILE__,__LINE__);
  devFlags=devflags;
  painters=0;
  hd=0;
}

QPaintDevice::~QPaintDevice()
{
  printf("%s %d\n",__FILE__,__LINE__);
}

bool QPaintDevice::cmd(int,QPainter *,QPDevCmdParam *)
{
  printf("%s %d\n",__FILE__,__LINE__);
  return true;
}

int QPaintDevice::metric(int) const
{
  printf("%s %d\n",__FILE__,__LINE__);
  return 0;
}

int QPaintDevice::fontMet(QFont *,int,const char *,int) const
{
  printf("%s %d\n",__FILE__,__LINE__);
  return 0;
}

int QPaintDevice::fontInf(QFont *,int) const
{
  printf("%s %d\n",__FILE__,__LINE__);
  return 0;
}

void bitBlt( QPaintDevice *dst, int dx, int dy,
             const QPaintDevice *src, int sx, int sy, int sw, int sh,
             Qt::RasterOp rop, bool ignoreMask )
{
  printf("%s %d\n",__FILE__,__LINE__);
  printf("  Blitting %d %d %d %d %d %d\n",dx,dy,sx,sy,sw,sh);
  if(dx+sw>dst->metric(PDM_WIDTH)) {
    sw=dst->metric(PDM_WIDTH)-dx;
  }
  if(dy+sh>dst->metric(PDM_HEIGHT)) {
    sh=dst->metric(PDM_HEIGHT)-dy;
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
  printf("%s %d\n",__FILE__,__LINE__);
  return hd;
}

void QPaintDevice::fixport()
{
  if(handle())
    SetPort((WindowPtr)handle());
}













