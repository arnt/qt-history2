#include "qregion.h"
#include <stdio.h>

QRegion::QRegion()
{
  data=new QRegionData;
  data->rgn=(void *)NewRgn();
}

QRegion::QRegion( bool is_null )
{
  data=new QRegionData;
  data->rgn=(void *)NewRgn();
  data->is_null=is_null;
}

QRegion::QRegion( const QRect &r, RegionType t )
{
  data=new QRegionData;
  data->rgn=(void *)NewRgn();
  Rect re;
  OpenRgn();
  SetRect(&re,r.left(),r.top(),r.right(),r.bottom());
  FrameRect(&re);
  CloseRgn((RgnHandle)data->rgn);
}

QRegion::QRegion( const QPointArray &a, bool winding )
{
  data=new QRegionData;
  data->rgn=(void *)NewRgn();
}

QRegion::QRegion( const QRegion &r )
{
  data=r.data;
  data->ref();
}

QRegion::QRegion( const QBitmap & bm )
{
  data=new QRegionData;
  data->rgn=(void *)NewRgn();
}

QRegion::~QRegion()
{
  if ( data->deref() ) {
    if ( data->rgn ) {
      DisposeRgn((RgnHandle)data->rgn );
    }
    delete data;
  }
}

QRegion &QRegion::operator=( const QRegion &r )
{
  r.data->ref();
  if ( data->deref() ) {
    if ( data->rgn ) {
      DisposeRgn((RgnHandle)data->rgn );
    }
    delete data;
  }
  data=r.data;
  return *this;
}

QRegion QRegion::copy() const
{
  QRegion r(data->is_null);
  RgnHandle rr=(RgnHandle)data->rgn;
  RgnHandle r2=NewRgn();
  **r2=**rr;  // Copy data. I hope :)
  r.data->rgn=(void *)r2;
  return r;
}

bool QRegion::isNull() const
{
  return data->is_null;
}

bool QRegion::isEmpty() const
{
  if(EmptyRgn((RgnHandle)data->rgn)) {
    return true;
  }
  return false;
}

bool QRegion::contains( const QPoint &p ) const
{
  Point pp;
  pp.h=p.x();
  pp.v=p.y();
  if(PtInRgn(pp,(RgnHandle)data->rgn))
    return true;
  return false;
}

void QRegion::translate( int dx, int dy )
{
  OffsetRgn((RgnHandle)data->rgn,dx,dy);
}

QRegion QRegion::unite( const QRegion &r ) const
{
  QRegion qr(false);
  RgnHandle ret=(RgnHandle)qr.data->rgn;
  if(data->rgn && r.data->rgn) {
    UnionRgn((RgnHandle)data->rgn,(RgnHandle)r.data->rgn,
             ret);
  }
  return qr;
}

QRegion QRegion::intersect( const QRegion &r ) const
{
  QRegion qr(false);
  RgnHandle ret=(RgnHandle)qr.data->rgn;
  if(data->rgn && r.data->rgn) {
    SectRgn((RgnHandle)data->rgn,(RgnHandle)r.data->rgn,
             ret);
  }
  return qr;
}

QRegion QRegion::subtract( const QRegion &r ) const
{
  // Is this right? What's the difference of two regions?
  QRegion qr(false);
  RgnHandle ret=(RgnHandle)qr.data->rgn;
  if(data->rgn && r.data->rgn) {
    DiffRgn((RgnHandle)data->rgn,(RgnHandle)r.data->rgn,
             ret);
  }
  return qr;
}

QRegion QRegion::eor( const QRegion &r ) const
{
  QRegion qr(false);
  RgnHandle ret=(RgnHandle)qr.data->rgn;
  if(data->rgn && r.data->rgn) {
    XorRgn((RgnHandle)data->rgn,(RgnHandle)r.data->rgn,
             ret);
  }
  return qr;
}

QRect QRegion::boundingRect() const
{
  Region * r=*((RgnHandle)data->rgn);
  return QRect(r->rgnBBox.left,r->rgnBBox.top,
               r->rgnBBox.right-r->rgnBBox.left,
               r->rgnBBox.bottom-r->rgnBBox.top);
}

QArray<QRect> QRegion::rects() const
{
  // I'm not sure how we can break this down into more accurate rects
  // so for now we just return the one
  QArray<QRect> foo(1);
  foo[0]=boundingRect();
  return foo;
}

void QRegion::setRects( const QRect *rects, int num )
{
    // Could be optimized
    *this = QRegion();
    for (int i=0; i<num; i++)
	*this |= rects[i];
}

bool QRegion::operator==( const QRegion &r ) const
{
  if(EqualRgn((RgnHandle)data->rgn,(RgnHandle)r.data->rgn)) {
    return true;
  } else {
    return false;
  }
}



