#include "qwidget.h"
#include "qpaintdevicemetrics.h"
#include "qapplication.h"
#include "qpainter.h"
#include "qobjectlist.h"
#include "qfocusdata.h"
#include "qaccel.h"
#include "qpixmap.h"
#include "qwidgetintdict.h"
#include <stdio.h>

#include "qt_mac.h"

WId parentw,destroyw=0;

QWidget * mac_pre=0;

extern bool ignorecliprgn;
extern RgnHandle cliprgn;

WId myactive=-1;

const unsigned char * p_str(const char * c)
{
  const unsigned char * ret=new const unsigned char[strlen(c)+2];
  ((unsigned char *)ret)[0]=strlen(c);
  strcpy(((char *)ret)+1,c);
  return ret;
}

void QWidget::setBackgroundPixmapDirect( const QPixmap &pixmap )
{
  //printf("%s %d\n",__FILE__,__LINE__);
  back_type=2;
  if(bg_pix)
    delete bg_pix;
  bg_pix=new QPixmap();
  *bg_pix=pixmap;
}

/*
void QWidget::setMouseTracking( bool enable )
{
}
*/

void show_children(QWidget * w,int show)
{
  const QObjectList * foo=w->children();
  if(foo) {
    QObjectListIt it(*foo);
    QObject * bar;
    bar=it.toLast();
    do {
      if(bar->inherits("QWidget")) {
        if(!bar->inherits("QMenuBar")) {
          if(show) {
            ((QWidget *)bar)->show();
	  } else {
            ((QWidget *)bar)->hide();
	  }
	}
      }
      bar=--it;
    } while(bar!=0);
  }
}

void redraw_children(QWidget * w)
{
  w->erase(0,0,w->width(),w->height());
  const QObjectList * foo=w->children();
  if(foo) {
    QObjectListIt it(*foo);
    QObject * bar;
    bar=it.toLast();
    do {
      if(bar->inherits("QWidget")) {
        redraw_children((QWidget *)bar);
      }
      bar=--it;
    } while(bar!=0);
  }
}

void QWidget::drawText( int x, int y, const QString &str )
{
  //printf("QWidget::drawText %s %d\n",__FILE__,__LINE__);
  //printf("Text is %s\n",str.ascii());
  if(testWState(WState_Visible)) {
    QPainter paint;
    paint.begin(this);
    paint.drawText(x,y,str);
    paint.end();
  }
}

/*
void QWidget::setSizeGrip( bool sizegrip )
{
  ////printf("%s %d\n",__FILE__,__LINE__);
}
*/

void QWidget::setMaximumSize( int maxw, int maxh )
{
  //printf("%s %d\n",__FILE__,__LINE__);
#if defined(CHECK_RANGE)
    if ( maxw > QCOORD_MAX || maxh > QCOORD_MAX )
        warning("QWidget::setMaximumSize: (%s/%s) "
                "The largest allowed size is (%d,%d)",
                 name( "unnamed" ), className(), QCOORD_MAX, QCOORD_MAX );
    if ( maxw < 0 || maxh < 0 )
        warning("QWidget::setMaximumSize: (%s/%s) Negative sizes (%d,%d) "
                "are not possible",
                name( "unnamed" ), className(), maxw, maxh );
#endif
    createExtra();
    if ( extra->maxw == maxw && extra->maxh == maxh )
        return;
    extra->maxw = maxw;
    extra->maxh = maxh;
    if ( maxw < width() || maxh < height() )
        resize( QMIN(maxw,width()), QMIN(maxh,height()) );
    //updateGeometry();
}

void QWidget::createSysExtra()
{
  //printf("%s %d\n",__FILE__,__LINE__);
}

void QWidget::scroll( int dx, int dy )
{
  //printf("%s %d\n",__FILE__,__LINE__);
  // Temporary cheat?
  bitBlt(this,dx,dy,this,0,0,width(),height());
  const QObjectList * foo=children();
  if(foo) {
    QObjectListIt it(*foo);
    QObject * bar;
    QWidget * frobnitz;
    bar=it.toLast();
    do {
      if(bar->inherits("QWidget")) {
        frobnitz=(QWidget *)bar;
        frobnitz->move(frobnitz->x()+dx,frobnitz->y()+dy);
      }
      bar=--it;
    } while(bar!=0);
  }
}

QWidget * get_top(QWidget * widg)
{
  // Return top-level window
  if(!widg)
    return 0;
  QWidget * ret=widg;
  while(ret->parentWidget()!=0) {
    ret=ret->parentWidget();
  }
  return ret;
}

void make_top(QWidget * widg,int & x,int & y)
{
  // Convert from local window to global coords
  //printf("Values here %d %d\n",x,y);
  // First get top left of actual window
  Rect oggy;
  oggy=((GrafPtr)widg->handle())->portRect;
  int xfoo;
  int yfoo;
  //xfoo=oggy.left;
  //yfoo=oggy.top;
  xfoo=0;
  yfoo=0;

  if(!widg || !widg->parentWidget())
    return;
  //printf("Top %d %d\n",xfoo,yfoo);
  QWidget * ret=widg;
  while(ret->parentWidget()) {
    //printf("Adding %d %d\n",ret->x(),ret->y());
    xfoo+=ret->x();
    yfoo+=ret->y();
    ret=ret->parentWidget();
  }
  //printf("Total to add %d %d\n",xfoo,yfoo);
  x=x+xfoo;
  y=y+yfoo;
}

void QWidget::create( WId window, bool initializeWindow, bool destroyOldWindow)
{
  //printf("QWidget::create %s %d\n",__FILE__,__LINE__);
  //printf("Parent %d\n",parentWidget());
  /*
  if ( testWState(WState_Created) && window == 0 ) {
    //printf("Already created top-level\n");
    //getchar();
    return;
  }
  */
  bg_pix=0;
  back_type=1;
  WId root_win=0;
  setWState( WState_Created );                        // set created flag
//   clearWState( WState_USPositionX ); // #### Matthias

  if ( !parentWidget() )
    setWFlags( WType_TopLevel );            // top-level widget

  static int sw = -1, sh = -1;                // screen size

  bool   topLevel = testWFlags(WType_TopLevel);
  bool   popup    = testWFlags(WType_Popup);
  bool   modal    = testWFlags(WType_Modal);
  bool   desktop  = testWFlags(WType_Desktop);
  WId    id;

  if ( !window )                              // always initialize
    initializeWindow=TRUE;

  //printf("Size is %d %d\n",sw,sh);

  if(sw<0) {
    sw=1024;    // Make it up
    sh=768;
  }

  bg_col = pal.normal().background();

  if ( modal || popup || desktop ) {          // these are top-level, too
    topLevel = TRUE;
    setWFlags( WType_TopLevel );
  }

  Rect boundsRect;

  if ( desktop ) {                            // desktop widget
    modal = popup = FALSE;                  // force these flags off
    crect.setRect( 0, 0, sw, sh );
  } else if ( topLevel ) {                    // calc pos/size from screen
    crect.setRect( sw/4, 3*sh/10, sw/2, 4*sh/10 );
  } else {                                    // child widget
    crect.setRect( 0, 0, 100, 30 );
  }
  fpos = crect.topLeft();                     // default frame rect

  parentw = topLevel ? root_win : parentWidget()->winId();

  //if(crect.bottom()-crect.top()<10) {
  //  SetRect(&boundsRect,crect.left(),crect.top(),crect.right(),
  //          crect.top()+200);
  //} else {
    SetRect(&boundsRect,crect.left(),crect.top(),crect.right(),crect.bottom());
  //}
  char title[2];
  title[0]=0;
  title[1]='\0';
  unsigned char visible=0;
  //short procid=kWindowFullZoomGrowDocumentProc;
  short procid;
  if(popup) {
    procid=plainDBox;
  } else {
    procid=zoomDocProc;
  }
  GrafPort * behind=(GrafPort *)-1;
  unsigned char goaway=true;

  if(!parentWidget() || (popup || modal)) {
    mytop=this;
    id=(WId)NewCWindow((void *)0,&boundsRect,(const unsigned char *)title,
                       visible,procid,behind,goaway,0);
    hd=(void *)id;
  } else {
    //printf("Calling get_top\n");
    mytop=get_top(this);
    //id=(WId)NewCWindow((void *)0,&boundsRect,(const unsigned char *)title,
    //                   visible,procid,behind,goaway,0);
    //hd=(void *)id;
    id=(WId)mytop->hd;
    hd=mytop->hd;
  }
  //printf("Made window\n");
  bg_col=pal.normal().background();
  //printf("Done background\n");
  if(!parentWidget()) {
    setWinId(id);
  } else {
    winid=id;
  }
  //printf("Done winid %d\n",id);

  const char * c=name();
  if(!parentWidget()) {
    if(c) {
      setCaption(QString(c));
    }
  }
  //printf("Done caption\n");

  setWState( WState_MouseTracking );
  setMouseTracking( FALSE );                  // also sets event mask
  clearWState(WState_Visible);
  //printf("All done\n");
  getchar();
}

int QWidget::metric( int m ) const
{
  WindowPtr p=(WindowPtr)winid;
  //printf("QWidget::metric: %s %d\n",__FILE__,__LINE__);
  //printf("  Portrect %d %d %d %d\n",p->portRect.left,p->portRect.top,
  //       p->portRect.right,p->portRect.bottom);
  if(m==QPaintDeviceMetrics::PdmWidth) {
    if(parentWidget()) {
      //printf("Width here %d\n",crect.width());
      return crect.width();
    } else {
      return p->portRect.right;
    }
  } else if(m==QPaintDeviceMetrics::PdmHeight) {
    if(parentWidget()) {
      return crect.height();
    } else {
      return p->portRect.bottom;
    }
  } else if(m==QPaintDeviceMetrics::PdmWidthMM) {
    return metric(QPaintDeviceMetrics::PdmWidth);
  } else if(m==QPaintDeviceMetrics::PdmHeightMM) {
    return metric(QPaintDeviceMetrics::PdmHeight);
  } else if(m==QPaintDeviceMetrics::PdmNumColors) {
    return 16;
  } else if(m==QPaintDeviceMetrics::PdmDepth) {
    // FIXME : this is a lie in most cases
    return 16;
  } else {
    //printf("Heeelp! QWidget::metric %d\n",m);
    getchar();
  }
  return 0;
}

void QWidget::update()
{
  //printf("%s %d\n",__FILE__,__LINE__);
  update(0,0,width(),height());
}

void QWidget::update( int x, int y, int w, int h )
{
  if(testWFlags(WState_Created)) {
    if(!isVisible()) {
      return;
    }
    erase(x,y,w,h);
    Rect r;
    int x1=x;
    int y1=y;
    int x2=x+w;
    int y2=y+h;
    make_top(this,x1,y1);
    make_top(this,x2,y2);
    SetRect(&r,x1,y1,x2,y2);
    InvalRect(&r);
  }
}

void QWidget::setBackgroundColorDirect( const QColor &color )
{
  back_type=1;
  QColor old=bg_col;
  bg_col=color;
  if(bg_pix)
    delete bg_pix;
  //backgroundColorChange(old);
  erase(0,0,width(),height());
  //printf("QWidget::setBackgroundColorDirect: %s %d\n",__FILE__,__LINE__);
  //printf("  %d %d %d\n",color.red(),color.green(),color.blue());

  /*
  if(!parentWidget()) {
    WCTabPtr wtp=new WinCTab;
    WCTabHandle * wth=new WCTabHandle;
    PtrToHand((const void *)wtp,(char ***)wth,(unsigned long)sizeof(WinCTab));
    HLock((char **)(*wth));
    wtp->ctSize=12;
    int loopc;
    for(loopc=0;loopc<12;loopc++) {
      wtp->ctTable[loopc].value=wContentColor;
      wtp->ctTable[loopc].rgb.red=color.red()*256;
      wtp->ctTable[loopc].rgb.green=color.green()*256;
      wtp->ctTable[loopc].rgb.blue=color.blue()*256;
    }
    SetWinColor((WindowPtr)winid,*wth);
  }
  */
}

void QWidget::erase( int x, int y, int w, int h )
{
  //printf("QWidget::erase: %s %d\n",__FILE__,__LINE__);
  if(back_type==1) {
    // solid background
    Rect r;
    RGBColor rc;
    rc.red=bg_col.red()*256;
    rc.green=bg_col.green()*256;
    rc.blue=bg_col.blue()*256;
    this->fixport();
    RGBForeColor(&rc);
    x-=1;
    y-=1;
    w+=2;
    h+=2;
    if(x<0)
      x=0;
    if(y<0)
      y=0;
    if(w>width()) {
      w=width();
    }
    if(h>height()) {
      h=height();
    }
    SetRect(&r,x,y,x+w,y+h);
    PaintRect(&r);
  } else if(back_type==2) {
    // pixmap
    if(bg_pix) {
      QPainter p;
      p.begin(this);
      p.drawTiledPixmap(x,y,w,h,*bg_pix,0,0);
      p.end();
    }
  } else {
    // nothing
  }
}

void QWidget::erase( const QRegion& reg )
{
  //printf("%s %d\n",__FILE__,__LINE__);
  RGBColor rc;
  this->fixport();
  rc.red=bg_col.red()*256;
  rc.green=bg_col.green()*256;
  rc.blue=bg_col.blue()*256;
  RGBForeColor(&rc);
  PaintRgn((RgnHandle)reg.handle());
}

void qt_leave_modal( QWidget * );

void QWidget::destroy( bool destroyWindow, bool destroySubWindows )
{
  //printf("QWidget::destroy: %s %d\n",__FILE__,__LINE__);
    if ( testWState(WState_Created) ) {
        clearWState( WState_Created );
        if ( children() ) {
            QObjectListIt it(*children());
            register QObject *obj;
            while ( (obj=it.current()) ) {      // destroy all widget children
                ++it;
                if ( obj->isWidgetType() )
                    ((QWidget*)obj)->destroy(destroySubWindows,
                                             destroySubWindows);
            }
        }
        if ( testWFlags(WType_Modal) )          // just be sure we leave modal
            qt_leave_modal( this );
        else if ( testWFlags(WType_Popup) )
            qApp->closePopup( this );
    }
  QWidget * mya;
  mya=QWidget::find(myactive);
  if(mya==this) {
    myactive=-1;
  }
  if(destroyWindow && !testWFlags(WType_Desktop)) {
    //printf("Dispose\n");
    if(!parentWidget() && hd) {
      DisposeWindow((WindowPtr)hd);
    }
    //printf("Disposed\n");
  }
  //printf("Set winid\n");
  hd=0;
}

void QWidget::showNormal()
{
  //printf("QWidget::showNormal: %s %d\n",__FILE__,__LINE__);
  showWindow();
}

void QWidget::showWindow()
{
  //printf("QWidget::showWindow: %s %d\n",__FILE__,__LINE__);
  setWState( WState_Visible );
  clearWState( WState_ForceHide );
  //printf("Doing show event\n");
  QShowEvent e(FALSE);
  QApplication::sendEvent( this, &e );
  //printf("Ptr is %d\n",winid);
  //getchar();
  if(!parentWidget()) {
    ShowHide((WindowPtr)winid,1);
  }
  setActiveWindow();
  QApplication::postEvent( this, new QPaintEvent( rect() ) );
  mac_pre=0;
  if(parentWidget()) {
    parentWidget()->update();
  }
  erase(0,0,width(),height());
  show_children(this,1);
}

void QWidget::showMinimized()
{
  //printf("QWidget::showMinimized: %s %d\n",__FILE__,__LINE__);
  hideWindow();    // Hmm
}

void QWidget::showMaximized()
{
  //printf("QWidget::showMaximized: %s %d\n",__FILE__,__LINE__);
  showNormal();    // Hmm
}

void QWidget::setMinimumSize( int minw, int minh )
{
  //printf("%s %d\n",__FILE__,__LINE__);
#if defined(CHECK_RANGE)
    if ( minw < 0 || minh < 0 )
        warning("QWidget::setMinimumSize: The smallest allowed size is (0,0)");
#endif
    createExtra();
    if ( extra->minw == minw && extra->minh == minh )
        return;
    extra->minw = minw;
    extra->minh = minh;
    if ( minw > width() || minh > height() )
        resize( QMAX(minw,width()), QMAX(minh,height()) );
    //updateGeometry();
}

// I don't think it's possible to make MacOS do the right thing here
// when dragging the window

void QWidget::setSizeIncrement( int w, int h )
{
  //printf("%s %d\n",__FILE__,__LINE__);
    createTLExtra();
    QTLWExtra* x = extra->topextra;
    if ( x->incw == w && x->inch == h )
        return;
    x->incw = w;
    x->inch = h;
}

void QWidget::repaint( int x, int y, int w, int h, bool erase )
{
  //printf("%s %d\n",__FILE__,__LINE__);
  if ((widget_state & (WState_Visible|WState_BlockUpdates))==WState_Visible) {
    if ( w < 0 )
      w=crect.width()  - x;
    if ( h < 0 )
      h=crect.height() - y;
    QPaintEvent e( QRect(x,y,w,h), erase );
    if ( erase && w != 0 && h != 0 )
      this->erase( x, y, w, h );
    QApplication::sendEvent( this, &e );
  }
}

void QWidget::repaint( const QRegion& reg, bool erase )
{
  //printf("%s %d\n",__FILE__,__LINE__);
  repaint(0,0,width(),height(),erase);
}

void QWidget::deleteSysExtra()
{
  //printf("%s %d\n",__FILE__,__LINE__);
}

void QWidget::setActiveWindow()
{
  //printf("QWidget::setActiveWindow: %s %d\n",__FILE__,__LINE__);
  QWidget * poppy;
  poppy=QWidget::find(myactive);
  // This is likely to flicker
  if(poppy) {
    if(!poppy->isPopup()) {
      poppy=0;
    }
  }
  if(!parentWidget()) {
    SelectWindow((WindowPtr)winid);  // Also brings to front - naughty?
    erase(0,0,width(),height());
    myactive=winid;
  }
  if(!isPopup() && poppy) {
    poppy->setActiveWindow();
  }
}

void QWidget::createTLSysExtra()
{
  //printf("%s %d\n",__FILE__,__LINE__);
}

void QWidget::deleteTLSysExtra()
{
  //printf("%s %d\n",__FILE__,__LINE__);
}

void QWidget::internalSetGeometry( int x, int y, int w, int h, bool isMove )
{
  if ( testWFlags(WType_Desktop) ) {
    //printf("Desktop\n");
    return;
  }
    if ( w < 1 )                                // invalid size
        w = width();
    if ( h < 1 )
        h = 100;
    QPoint oldp = pos();
    QSize  olds = size();
    // Deal with size increment
    if(extra) {
      if(extra->topextra) {
        if(extra->topextra->incw) {
          w=w/extra->topextra->incw;
          w=w*extra->topextra->incw;
	}
        if(extra->topextra->inch) {
          h=h/extra->topextra->inch;
          h=h*extra->topextra->inch;
	}
      }
    }
    QRect  r( x, y, w, h );
    if ( r.size() == olds && oldp == r.topLeft() &&
         (isTopLevel() == FALSE /* || testWState(WState_USPositionX)*/ )  ) { // #### Matthias
        //printf("USpositionX thingy\n");
        //getchar();
        return;
    }

    setCRect( r );

  if(!parentWidget()) {
    if(isMove) {
      if(winid) {
        MoveWindow((WindowPtr)winid,x,y,1);
      }
    }
  }
  bool isResize=olds!=r.size();
  if(!parentWidget()) {
    if(winid) {
      SizeWindow((WindowPtr)winid,w,h,1);
    }
  }
    if ( isVisible() ) {
        if ( isMove ) {
            QMoveEvent e( r.topLeft(), oldp );
            QApplication::sendEvent( this, &e );
            repaint(TRUE);
        }
        if ( isResize ) {
            QResizeEvent e( r.size(), olds );
            QApplication::sendEvent( this, &e );
            if ( !testWFlags(WResizeNoErase) )
                repaint( TRUE );
        }
    } else {
        if ( isMove )
            QApplication::postEvent( this,
                                     new QMoveEvent( r.topLeft(), oldp ) );
        if ( isResize )
            QApplication::postEvent( this,
                                     new QResizeEvent( r.size(), olds ) );
    }

    if(parentWidget()) {
      mac_pre=0;
      redraw_children(parentWidget());
    } else {
      redraw_children(this);
    }
}

void QWidget::setBackgroundEmpty()
{
  //printf("%s %d\n",__FILE__,__LINE__);
  back_type=3;
  if(bg_pix) {
    delete bg_pix;
  }
}

void QWidget::setMask( const QRegion& region )
{
  //printf("%s %d\n",__FILE__,__LINE__);
  // Hmm - how do we do this sort of thing?
}

void QWidget::setMask( const QBitmap &bitmap )
{
  //printf("%s %d\n",__FILE__,__LINE__);
}

void QWidget::clearMask()
{
  //printf("%s %d\n",__FILE__,__LINE__);
}

/*
void QWidget::setCaret(int x,int y,int w ,int h)
{
  // FIXME : Obsoleted?
  //printf("%s %d\n",__FILE__,__LINE__);
}
*/

QPoint QWidget::mapToGlobal( const QPoint &pos ) const
{
  //printf("%s %d\n",__FILE__,__LINE__);
  PixMapHandle pmh=((CGrafPtr)handle())->portPixMap;
  ((QWidget *)this)->fixport();
  mac_pre=0;
  int x2=pos.x();
  int y2=pos.y();
  x2=x2-(**pmh).bounds.left;
  y2=y2-(**pmh).bounds.top;
  //printf("Toglobal %d %d\n",x2,y2);
  QPoint p2(x2,y2);
  return p2;
}

QPoint QWidget::mapFromGlobal( const QPoint &pos ) const
{
  //printf("%s %d\n",__FILE__,__LINE__);
  PixMapHandle pmh=((CGrafPtr)handle())->portPixMap;
  ((QWidget *)this)->fixport();
  mac_pre=0;
  int x2=pos.x();
  int y2=pos.y();
  x2=x2+(**pmh).bounds.left;
  y2=y2+(**pmh).bounds.top;
  //printf("Tolocal %d %d\n",x2,y2);
  QPoint p2(x2,y2);
  return p2;
}

void QWidget::raise()
{
  //printf("%s %d\n",__FILE__,__LINE__);
  SelectWindow((WindowPtr)winid);  // Bring to front and activate
  if(parentWidget()) {
    QObjectList * qol=(QObjectList *)parentWidget()->children();
    qol->remove((QObject *)this);
    qol->insert(0,(QObject *)this);
    redraw_children(this);
    mac_pre=0;
  }
}

void QWidget::lower()
{
  //printf("%s %d\n",__FILE__,__LINE__);
  SendBehind((WindowPtr)winid,0);
  if(parentWidget()) {
    QObjectList * qol=(QObjectList *)children();
    qol->remove((QObject *)this);
    qol->append((QObject *)this);
    redraw_children(this);
    mac_pre=0;
  }
}

void QWidget::setCaption( const QString &caption )
{
  //printf("QWidget::setCaption %s %d\n",__FILE__,__LINE__);
  if ( QWidget::caption() == caption )
      return; // for less flicker
  topData()->caption = caption;
  if(!parentWidget()) {
    const char * b=caption.ascii();
    //printf("  %s\n",b);
    const unsigned char * c=p_str(b);
    SetWTitle((WindowPtr)winid,c);
    delete[] c;
  }
}

// Individual windows simply don't have icons on MacOS
// Need to think about how to set app icon though

void QWidget::setIcon( const QPixmap &pixmap )
{
  //printf("%s %d\n",__FILE__,__LINE__);
}

void QWidget::setIconText( const QString &iconText )
{
  //printf("%s %d\n",__FILE__,__LINE__);
}

void QWidget::reparent( QWidget *parent, WFlags f, const QPoint &p,
                        bool showIt )
{
  //printf("%s %d\n",__FILE__,__LINE__);
  //extern void qPRCreate( const QWidget *, Window );
  //Display *dpy = x11Display();
  //WId old_winid = winid;
  //if ( testWFlags(WType_Desktop) )
  // old_winid = 0;
    setWinId( 0 );
    reparentFocusWidgets( parent );             // fix focus chains

    if ( parentObj ) {                          // remove from parent
        parentObj->removeChild( this );
    }
    if ( parent ) {                             // insert into new parent
        parentObj = parent;                     // avoid insertChild warning
        parent->insertChild( this );
    } else {
        qApp->noteTopLevel(this);
    }
    bool     enable = isEnabled();              // remember status
    FocusPolicy fp = focusPolicy();
    QSize    s      = size();
    QPixmap *bgp    = (QPixmap *)backgroundPixmap();
    QColor   bgc    = bg_col;                   // save colors
    QString capt= caption();
    widget_flags = f;
    clearWState( WState_Created | WState_Visible );
    create();
    const QObjectList *chlist = children();
    if ( chlist ) {                             // reparent children
        QObjectListIt it( *chlist );
        QObject *obj;
        while ( (obj=it.current()) ) {
            if ( obj->isWidgetType() ) {
                QWidget *w = (QWidget *)obj;
                //XReparentWindow( x11Display(), w->winId(), winId(),
                //                 w->geometry().x(), w->geometry().y() );
            }
            ++it;
        }
    }
    //qPRCreate( this, old_winid );
    /*
    if ( bgp )
        XSetWindowBackgroundPixmap( dpy, winid, bgp->handle() );
    else
        XSetWindowBackground( dpy, winid, bgc.pixel() );
    */
    setGeometry( p.x(), p.y(), s.width(), s.height() );
    setEnabled( enable );
    setFocusPolicy( fp );
    if ( !capt.isNull() ) {
        extra->topextra->caption = QString::null;
        setCaption( capt );
    }
    if ( showIt )
        show();
    //if ( old_winid )
    //    qt_XDestroyWindow( this, dpy, old_winid );

    QObjectList *accelerators = queryList( "QAccel" );
    QObjectListIt it( *accelerators );
    QObject *obj;
    while ( (obj=it.current()) != 0 ) {
        ++it;
        ((QAccel*)obj)->repairEventFilter();
    }
    delete accelerators;
    if ( !parent ) {
        QFocusData *fd = focusData( TRUE );
        if ( fd->focusWidgets.findRef(this) < 0 )
            fd->focusWidgets.append( this );
    }
}

void QWidget::hideWindow()
{
  //printf("QWidget::hideWindow: %s %d\n",__FILE__,__LINE__);
  if(!parentWidget()) {
    ShowHide((WindowPtr)winid,0);
  }
  clearWState(WState_Visible);
  setWState( WState_ForceHide );
  mac_pre=0;
  if(parentWidget()) {
    parentWidget()->update();
  }
  show_children(this,0);
}

QWidget * the_grabbed=0;

void QWidget::grabMouse()
{
  //printf("%s %d\n",__FILE__,__LINE__);
  the_grabbed=this;
}

void QWidget::releaseMouse()
{
  //printf("%s %d\n",__FILE__,__LINE__);
  the_grabbed=0;
}

void QWidget::setCursor( const QCursor &cursor )
{
  //printf("%s %d\n",__FILE__,__LINE__);
    if ( cursor.handle() != arrowCursor.handle()
         || (extra && extra->curs) ) {
        createExtra();
        extra->curs = new QCursor(cursor);
    }
    setWState( WState_OwnCursor );
    QCursor *oc = QApplication::overrideCursor();
    //XDefineCursor( x11Display(), winId(),
    //               oc ? oc->handle() : cursor.handle() );
    //XFlush( x11Display() );

}

void QWidget::unsetCursor()
{
  //printf("%s %d\n",__FILE__,__LINE__);
    if ( !isTopLevel() ) {
        if (extra ) {
            delete extra->curs;
            extra->curs = 0;
        }
        clearWState( WState_OwnCursor );
        //XDefineCursor( x11Display(), winId(), None );
        //XFlush( x11Display() );
    }
}

void QWidget::setAcceptDrops( bool on )
{
  //printf("%s %d\n",__FILE__,__LINE__);
}

bool QWidget::acceptDrops() const
{
  return testWState(WState_DND);
}

void QWidget::propagateUpdates(int x,int y,int x2,int y2)
{
  //if(!isVisible()) {
  //  return;
  //}
  /*
  if(!testWFlags(WState_Created)) {
    return;
  }
  */
  this->fixport();
  //printf("Updates %d %d %d %d\n",x,y,x2,y2);
  erase(x,y,x2,y2);
  QRect paintRect(x,y,x2,y2);
  //QRect paintRect(0,0,width(),height());
  QRegion paintRegion(paintRect);
  QPaintEvent e(paintRegion);
  setWState( WState_InPaintEvent );
  QApplication::sendEvent( this, &e );
  clearWState( WState_InPaintEvent );

  int o1,o2;
  int a,b,c,d;
  const QObjectList * foo=children();
  if(foo) {
    QObjectListIt it(*foo);
    QObject * bar;
    QWidget * frobnitz;
    bar=it.toLast();
    do {
      if(bar->inherits("QWidget")) {
        frobnitz=(QWidget *)bar;
        a=x;
        b=y;
        c=x2;
        d=y2;
        a-=frobnitz->x();
        b-=frobnitz->y();
        c-=frobnitz->x();
        d-=frobnitz->y();
        //printf("X %d Y %d width %d height %d\n",frobnitz->x(),
        //       frobnitz->y(),frobnitz->width(),frobnitz->height());
        if(a<frobnitz->width() && b<frobnitz->height() &&
          c>0 && d>0) {
	  //if(1) {
          frobnitz->propagateUpdates(a,b,c,d);
	}
      }
      bar=--it;
    } while(bar!=0);
  }
}

void QWidget::fixport()
{
  //if(this==mac_pre)
  //  return;
  if(!hd)
    return;
  mac_pre=this;

  // FIXME: need to add painter clip too

  int xx=0;
  int yy=0;
  SetPort((WindowPtr)hd);
  SetOrigin(0,0);
  make_top(this,xx,yy);
  // Need to set clipping
  const QObjectList * foo=children();
  RgnHandle myclip;
  RgnHandle myrgn;
  RgnHandle bigrgn;
  RgnHandle morergn;
  RgnHandle toprgn;
  RgnHandle grr;
  myclip=NewRgn();
  myrgn=NewRgn();
  bigrgn=NewRgn();
  morergn=NewRgn();
  grr=NewRgn();

  int x1,y1,x2,y2;

  if(!isVisible()) {
    Rect rr;
    SetRect(&rr,0,0,0,0);
    OpenRgn();
    FrameRect(&rr);
    CloseRgn(myclip);
    SetClip(myclip);
    DisposeRgn(myrgn);
    DisposeRgn(myclip);
    DisposeRgn(bigrgn);
    DisposeRgn(morergn);
    DisposeRgn(grr);
    return;
  }

  Rect rect;

  // Get top-level
  QWidget * super_parent=parentWidget();
  if(!super_parent) {
    super_parent=this;
  } else {
    while(super_parent->parentWidget()) {
      super_parent=super_parent->parentWidget();
    }
  }

  x1=xx;
  y1=yy;
  x2=x1+width();
  y2=y1+height();

  if(x1<0)
    x1=0;
  if(y1<0)
    y1=0;
  if(x2>(super_parent->width()-xx))
    x2=super_parent->width();
  if(y2>(super_parent->height()-yy))
    y2=super_parent->height();

  // This is buggy and probably needs to do clever recursive things
  // It mostly works for now though, and the redraw/update code makes
  // sure things are drawn in Z order so there's just some flickering

  OpenRgn();
  SetRect(&rect,x1,y1,x2,y2);
  FrameRect(&rect);
  CloseRgn(myclip);

  OpenRgn();
  SetRect(&rect,x1,y1,x2,y2);
  FrameRect(&rect);
  CloseRgn(grr);

  bool bingo=false;

  //foo=0;
  if(foo) {
    QObjectListIt it(*foo);
    QObject * bar;
    QWidget * frobnitz;
    bar=it.toFirst();
    do {
      if(bar->inherits("QWidget")) {
        frobnitz=(QWidget *)bar;
        if(frobnitz->isVisible() && frobnitz->back_type!=3) {
          DisposeRgn(myrgn);
          myrgn=NewRgn();
          x1=0;
          y1=0;
          SetOrigin(0,0);
          make_top(frobnitz,x1,y1);
          x2=frobnitz->width();
          y2=frobnitz->height();
          make_top(frobnitz,x2,y2);
          OpenRgn();
          SetRect(&rect,x1,y1,x2,y2);
          FrameRect(&rect);
          if(!bingo) {
            CloseRgn(bigrgn);
            //FrameRgn(bigrgn);
            bingo=true;
          } else {
            CloseRgn(myrgn);
            //FrameRgn(myrgn);
            UnionRgn(myrgn,bigrgn,bigrgn);
	  }
	}
      }
    bar=++it;
    } while(bar!=0);
    SectRgn(bigrgn,myclip,bigrgn);
    XorRgn(bigrgn,myclip,myclip);
  } else {
    // Just /setting/ the clip doesn't work. It's bizarre. Does
    // SetClip do something other than /setting/ the clip?
  }
  // Myclip contains our clipping region
  //GetClip(bigrgn);
  //UnionRgn(bigrgn,myclip,myrgn);

  if(parentWidget()) {
    if(parentWidget()->parentWidget()) {
      if(parentWidget()->isVisible() && parentWidget()->parentWidget()->
         isVisible()) {
        x1=0;
        y1=0;
        make_top(parentWidget(),x1,y1);
        x2=x1+parentWidget()->width();
        y2=y1+parentWidget()->height();
        Rect r;
        SetRect(&r,x1,y1,x2,y2);
        OpenRgn();
        FrameRect(&r);
        CloseRgn(morergn);
        SectRgn(morergn,myclip,myclip);
      }
    }

    // Now chop siblings
    // First item in list is top of Z order

    const QObjectList * foo=parentWidget()->children();
    QObjectListIt it(*foo);
    QObject * bar;
    QWidget * frobnitz;
    bar=it.toFirst();
    while(bar!=this && bar!=0) {
      if(bar->inherits("QWidget")) {
        frobnitz=(QWidget *)bar;
        if(frobnitz->isVisible()) {
          x1=0;
          y1=0;
          make_top(frobnitz,x1,y1);
          x2=x1+frobnitz->width();
          y2=y1+frobnitz->height();
          Rect r;
          SetRect(&r,x1,y1,x2,y2);
          OpenRgn();
          FrameRect(&r);
          CloseRgn(morergn);
          DiffRgn(myclip,morergn,myclip);
	}
      }
      bar=++it;
    }
  }

  SectRgn(myclip,grr,myclip);
  //if(ignorecliprgn==false) {
  //  SectRgn(cliprgn,myclip,myclip);
  //}
  OffsetRgn(myclip,-(xx),-(yy));

  SetClip(myclip);

  SetOrigin(-(xx),-(yy));

  DisposeRgn(myrgn);
  DisposeRgn(myclip);
  DisposeRgn(bigrgn);
  DisposeRgn(morergn);
  DisposeRgn(grr);

}

void QWidget::setMicroFocusHint(int x,int y,int width,int height,bool text)
{
}

void QWidget::setName( const char * name )
{
}

void QWidget::setFontSys()
{
}

void QWidget::grabKeyboard()
{
}

void QWidget::releaseKeyboard()
{
}

void QWidget::scroll( int dx, int dy, const QRect& r )
{
}
