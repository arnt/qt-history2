#include "qregion.h"
#include <stdio.h>

QRegion::QRegion()
{
  printf("QRegion::QRegion(): %s %d\n",__FILE__,__LINE__);
  data=new QRegionData;
  data->rgn=(void *)NewRgn();
}

QRegion::QRegion( bool is_null )
{
  printf("QRegion::QRegion(bool): %s %d\n",__FILE__,__LINE__);
  data=new QRegionData;
  data->rgn=(void *)NewRgn();
  data->is_null=is_null;
}

QRegion::QRegion( const QRect &r, RegionType t )
{
  data=new QRegionData;
  printf("QRegion::QRegion(rect): %s %d\n",__FILE__,__LINE__);
  data->rgn=(void *)NewRgn();
  Rect re;
  OpenRgn();
  SetRect(&re,r.left(),r.top(),r.right(),r.bottom());
  FrameRect(&re);
  CloseRgn((RgnHandle)data->rgn);
}

QRegion::QRegion( const QPointArray &a, bool winding )
{
  printf("QRegion::QRegion(points): %s %d\n",__FILE__,__LINE__);
  data=new QRegionData;
  data->rgn=(void *)NewRgn();
}

QRegion::QRegion( const QRegion &r )
{
  printf("QRegion::QRegion(QRegion): %s %d\n",__FILE__,__LINE__);
  data=r.data;
  data->ref();
}

QRegion::QRegion( const QBitmap & bm )
{
  printf("QRegion::QRegion(bitmap): %s %d\n",__FILE__,__LINE__);
  data=new QRegionData;
  data->rgn=(void *)NewRgn();
}

QRegion::~QRegion()
{
  printf("QRegion::~QRegion: %s %d\n",__FILE__,__LINE__);
  if ( data->deref() ) {
    if ( data->rgn ) {
      DisposeRgn((RgnHandle)data->rgn );
    }
    delete data;
  }
  printf("Dunregionin\n");
}

QRegion &QRegion::operator=( const QRegion &r )
{
  printf("QRegion::= %s %d\n",__FILE__,__LINE__);
  r.data->ref();
  data=r.data;
  return *this;
}

QRegion QRegion::copy() const
{
  printf("QRegion::copy: %s %d\n",__FILE__,__LINE__);
  QRegion r(data->is_null);
  RgnHandle rr=(RgnHandle)data->rgn;
  RgnHandle r2=NewRgn();
  **r2=**rr;  // Copy data. I hope :)
  r.data->rgn=(void *)r2;
  return r;
}

bool QRegion::isNull() const
{
  printf("QRegion::isNull: %s %d\n",__FILE__,__LINE__);
  return data->is_null;
}

bool QRegion::isEmpty() const
{
  printf("QRegion::isEmpty: %s %d\n",__FILE__,__LINE__);
  if(EmptyRgn((RgnHandle)data->rgn)) {
    return true;
  }
  return false;
}

bool QRegion::contains( const QPoint &p ) const
{
  printf("QRegion::contains: %s %d\n",__FILE__,__LINE__);
  Point pp;
  //pp.x=p.x();  How the hell do you set a point?
  //pp.y=p.y();
  if(PtInRgn(pp,(RgnHandle)data->rgn))
    return true;
  return false;
}

void QRegion::translate( int dx, int dy )
{
  printf("QRegion::translate: %s %d\n",__FILE__,__LINE__);
  OffsetRgn((RgnHandle)data->rgn,dx,dy);
}

QRegion QRegion::unite( const QRegion &r ) const
{
  printf("QRegion::unite: %s %d\n",__FILE__,__LINE__);
  RgnHandle ret=NewRgn();
  if(data->rgn && r.data->rgn) {
    UnionRgn((RgnHandle)data->rgn,(RgnHandle)r.data->rgn,
             ret);
  }
  return (void *)ret;
}

QRegion QRegion::intersect( const QRegion &r ) const
{
  printf("QRegion::intersect: %s %d\n",__FILE__,__LINE__);
  RgnHandle ret=NewRgn();
  if(data->rgn && r.data->rgn) {
    SectRgn((RgnHandle)data->rgn,(RgnHandle)r.data->rgn,
             ret);
  }
  return (void *)ret;
}

QRegion QRegion::subtract( const QRegion &r ) const
{
  // Is this right? What's the difference of two regions?
  printf("QRegion::subtract: %s %d\n",__FILE__,__LINE__);
  RgnHandle ret=NewRgn();
  if(data->rgn && r.data->rgn) {
    DiffRgn((RgnHandle)data->rgn,(RgnHandle)r.data->rgn,
             ret);
  }
  return (void *)ret;
}

QRegion QRegion::eor( const QRegion &r ) const
{
  printf("QRegion::eor: %s %d\n",__FILE__,__LINE__);
  RgnHandle ret=NewRgn();
  if(data->rgn && r.data->rgn) {
    XorRgn((RgnHandle)data->rgn,(RgnHandle)r.data->rgn,
             ret);
  }
  return (void *)ret;
}

QRect QRegion::boundingRect() const
{
  printf("QRegion::boundingRect: %s %d\n",__FILE__,__LINE__);
  Region * r=*((RgnHandle)data->rgn);
  return QRect(r->rgnBBox.left,r->rgnBBox.top,
               r->rgnBBox.right-r->rgnBBox.left,
               r->rgnBBox.bottom-r->rgnBBox.top);
}

QArray<QRect> QRegion::rects() const
{
  printf("QRegion::rects: %s %d\n",__FILE__,__LINE__);
  return QArray<QRect>();
}

bool QRegion::operator==( const QRegion &r ) const
{
  printf("QRegion::== %s %d\n",__FILE__,__LINE__);
  return false;
}



