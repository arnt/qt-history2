#include <qapplication.h>
#include <qcursor.h>
#include <qbitmap.h>
#include <qwmatrix.h>
#include <qlayout.h>
#include "workspace.h"
#include "client.h"
#include "atoms.h"
#include <X11/X.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

extern Atom qt_wm_state;


static void sendClientMessage(Window w, Atom a, long x){
  XEvent ev;
  long mask;

  memset(&ev, 0, sizeof(ev));
  ev.xclient.type = ClientMessage;
  ev.xclient.window = w;
  ev.xclient.message_type = a;
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = x;
  ev.xclient.data.l[1] = CurrentTime;
  mask = 0L;
  if (w == qt_xrootwin())
    mask = SubstructureRedirectMask;        /* magic! */
  XSendEvent(qt_xdisplay(), w, False, mask, &ev);
}


static int _getprop(Window w, Atom a, Atom type, long len, unsigned char **p){
  Atom real_type;
  int format;
  unsigned long n, extra;
  int status;

  status = XGetWindowProperty(qt_xdisplay(), w, a, 0L, len, False, type, &real_type, &format, &n, &extra, p);
  if (status != Success || *p == 0)
    return -1;
  if (n == 0)
    XFree((char*) *p);
  /* could check real_type, format, extra here... */
  return n;
}
static bool getDoubleProperty(Window w, Atom a, long &result1, long &result2){
  long *p = 0;

  if (_getprop(w, a, a, 2L, (unsigned char**)&p) <= 0){
    return FALSE;
  }

  result1 = p[0];
  result2 = p[1];
  XFree((char *) p);
  return TRUE;
}
QPixmap KWM::miniIcon(Window w, int width, int height){
  QPixmap result;
  Pixmap p = None;
  Pixmap p_mask = None;

  long tmp[2] = {None, None};
  if (!getDoubleProperty(w, atoms->kwm_win_icon, tmp[0], tmp[1])){
    XWMHints *hints = XGetWMHints(qt_xdisplay(), w);
    if (hints && (hints->flags & IconPixmapHint)){
      p = hints->icon_pixmap;
    }
    if (hints && (hints->flags & IconMaskHint)){
      p_mask = hints->icon_mask;
    }
    if (hints)
      XFree((char*)hints);
  }
  else {
    p = (Pixmap) tmp[0];
    p_mask = (Pixmap) tmp[1];
  }

  if (p != None){
    Window root;
    int x, y;
    unsigned int w = 0;
    unsigned int h = 0;
    unsigned int border_w, depth;
    XGetGeometry(qt_xdisplay(), p,
		 &root,
		 &x, &y, &w, &h, &border_w, &depth);
    if (w > 0 && h > 0){
      QPixmap pm(w, h, depth);
      XCopyArea(qt_xdisplay(), p, pm.handle(),
		qt_xget_temp_gc(depth==1),
		0, 0, w, h, 0, 0);
      if (p_mask != None){
	QBitmap bm(w, h);
	XCopyArea(qt_xdisplay(), p_mask, bm.handle(),
		  qt_xget_temp_gc(TRUE),
		  0, 0, w, h, 0, 0);
	pm.setMask(bm);
      }
      if (width > 0 && height > 0 && (w > (unsigned int)width
				      || h > (unsigned int) height)){
	// scale
	QWMatrix m;
	m.scale(width/(float)w, height/(float)h);
	result = pm.xForm(m);
      }
      else
	result = pm;
    }
  }
  else {
    XWMHints *hints = XGetWMHints(qt_xdisplay(),  w);
    if (hints &&
	(hints->flags & WindowGroupHint)
	&& hints->window_group != None
	&& hints->window_group != w){
	Window wg = hints->window_group;
	XFree((char*)hints);
	return miniIcon(wg, width, height);
    }
    if (hints)
      XFree((char*)hints);
    Window trans = None;
    if (XGetTransientForHint(qt_xdisplay(), w, &trans)){
      if (trans != w)
	return miniIcon(trans, width, height);
    }
  }
  return result;
}

QPixmap KWM::icon(Window w, int width, int height){
  QPixmap result;
  Pixmap p = None;
  Pixmap p_mask = None;

  XWMHints *hints = XGetWMHints(qt_xdisplay(), w);
  if (hints && (hints->flags & IconPixmapHint)){
    p = hints->icon_pixmap;
  }
  if (hints && (hints->flags & IconMaskHint)){
    p_mask = hints->icon_mask;
  }
  if (hints)
    XFree((char*)hints);

  if (p != None){
    Window root;
    int x, y;
    unsigned int w = 0;
    unsigned int h = 0;
    unsigned int border_w, depth;
    XGetGeometry(qt_xdisplay(), p,
		 &root,
		 &x, &y, &w, &h, &border_w, &depth);
    if (w > 0 && h > 0){
      QPixmap pm(w, h, depth);
      XCopyArea(qt_xdisplay(), p, pm.handle(),
		qt_xget_temp_gc(depth==1),
		0, 0, w, h, 0, 0);
      if (p_mask != None){
	QBitmap bm(w, h);
	XCopyArea(qt_xdisplay(), p_mask, bm.handle(),
		  qt_xget_temp_gc(TRUE),
		  0, 0, w, h, 0, 0);
	pm.setMask(bm);
      }
      if (width > 0 && height > 0 && (w > (unsigned int)width
				      || h > (unsigned int) height)){
	// scale
	QWMatrix m;
	m.scale(width/(float)w, height/(float)h);
	result = pm.xForm(m);
      }
      else
	result = pm;
    }
  }
  else {
    XWMHints *hints = XGetWMHints(qt_xdisplay(),  w);
    if (hints &&
	(hints->flags & WindowGroupHint)
	&& hints->window_group != None
	&& hints->window_group != w){
      XFree((char*)hints);
      return icon(hints->window_group, width, height);
    }
    if (hints)
      XFree((char*)hints);
    Window trans = None;
    if (XGetTransientForHint(qt_xdisplay(), w, &trans)){
      if (trans != w)
	return icon(trans, width, height);
    }
  }
  return result;
}

/*!
  \class WindowWrapper client.h

  \brief The WindowWrapper class encapsulates a client's managed
  window.

  There's not much to know about this class, it's completely handled by
  the abstract class Client. You get access to the window wrapper with
  Client:.windowWrapper(). The big advantage of WindowWrapper is,
  that you can use just like a normal QWidget, although it encapsulates
  an X window that belongs to another application.

  In particular, this means adding a client's windowWrapper() to a
  QLayout for the geometry management. See StdClient for an example
  on how to do this.

 */


WindowWrapper::WindowWrapper( WId w, Client *parent, const char* name)
    : QWidget( parent, name )
{
    win = w;
    setMouseTracking( TRUE );

    // we don't want the window to be destroyed when we are destroyed
    XAddToSaveSet(qt_xdisplay(), win );

    // overwrite Qt-defaults because we need SubstructureNotifyMask
    XSelectInput( qt_xdisplay(), winId(),
  		 KeyPressMask | KeyReleaseMask |
  		 ButtonPressMask | ButtonReleaseMask |
  		 KeymapStateMask |
   		 ButtonMotionMask |
   		 PointerMotionMask | // need this, too!
  		 EnterWindowMask | LeaveWindowMask |
  		 FocusChangeMask |
  		 ExposureMask |
		 StructureNotifyMask |
		 SubstructureRedirectMask |
		 SubstructureNotifyMask
  		 );

    XSelectInput( qt_xdisplay(), w,
		  FocusChangeMask |
		  PropertyChangeMask
		  );

    // set the border width to 0
    XWindowChanges wc;
    wc.border_width = 0;
    XConfigureWindow( qt_xdisplay(), win, CWBorderWidth, &wc );

    // finally, get the window
    XReparentWindow( qt_xdisplay(), win, winId(), 0, 0 );

    // install a passive grab to catch mouse button events
    XGrabButton(qt_xdisplay(), AnyButton, AnyModifier, winId(), FALSE,
		ButtonPressMask, GrabModeSync, GrabModeSync,
		None, None );

}

WindowWrapper::~WindowWrapper()
{
    releaseWindow();
}

QSize WindowWrapper::sizeHint() const
{
    return size();
}

QSizePolicy WindowWrapper::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
}


void WindowWrapper::resizeEvent( QResizeEvent * )
{
    if ( win )
	XMoveResizeWindow( qt_xdisplay(), win,
			   0, 0, width(), height() );
}

void WindowWrapper::showEvent( QShowEvent* )
{
    if ( win ) {
	XMoveResizeWindow( qt_xdisplay(), win,
			   0, 0, width(), height() );
	XMapRaised( qt_xdisplay(), win );
    }
}
void WindowWrapper::hideEvent( QHideEvent* )
{
    if ( win )
	XUnmapWindow( qt_xdisplay(), win );
}

void WindowWrapper::invalidateWindow()
{
    win = 0;
}

/*!
  Releases the window. The client has done its job and the window is still existing.
 */
void WindowWrapper::releaseWindow()
{
    if ( win ) {
	qDebug("release to %d %d", parentWidget()->x(), parentWidget()->y() );
	XReparentWindow( qt_xdisplay(), win,
			 ((Client*)parentWidget())->workspace()->rootWin(),
			 parentWidget()->x(),
			 parentWidget()->y() );
	XRemoveFromSaveSet(qt_xdisplay(), win );
	invalidateWindow();
    }
}



void WindowWrapper::mousePressEvent( QMouseEvent*  )
{
    XAllowEvents( qt_xdisplay(), ReplayPointer, lastMouseEventTime );
}

void WindowWrapper::mouseReleaseEvent( QMouseEvent*  )
{
    XAllowEvents( qt_xdisplay(), ReplayPointer, lastMouseEventTime );
}

void WindowWrapper::mouseMoveEvent( QMouseEvent* )
{
    //#### we sometimes get these but shouldn't..... Maybe a Qt problem
    //XAllowEvents( qt_xdisplay(), ReplayPointer, lastMouseEventTime );
}


bool WindowWrapper::x11Event( XEvent * e)
{
    switch ( e->type ) {
    case ButtonPress:
    case ButtonRelease:
	lastMouseEventTime = e->xbutton.time;
	if ( e->xbutton.button > 3 ) {
	    XAllowEvents( qt_xdisplay(), ReplayPointer, lastMouseEventTime );
	    return TRUE;
	}
	break;
    case MotionNotify:
	lastMouseEventTime = e->xmotion.time;
	break;
    default:
	break;
    }
    return FALSE;
}


/*!
  \class Client client.h

  \brief The Client class encapsulates a window decoration frame.

  TODO

*/

/*!
  Creates a client on workspace \a ws for window \a w.
 */
Client::Client( Workspace *ws, WId w, QWidget *parent, const char *name, WFlags f )
    : QWidget( parent, name, f | WStyle_Customize | WStyle_NoBorder )
{

    wspace = ws;
    win = w;
    XWindowAttributes attr;
    if (XGetWindowAttributes(qt_xdisplay(), win, &attr)){
	original_geometry.setRect(attr.x, attr.y, attr.width, attr.height );
    }
    wwrap = new WindowWrapper( w, this );
    wwrap->installEventFilter( this );

    // set the initial mapping state
    setMappingState( WithdrawnState );
    desk = -1; // and no desktop yet

    mode = Nowhere;
    buttonDown = FALSE;
    setMouseTracking( TRUE );

    active = FALSE;
    shaded = FALSE;
    transient_for = None;

    ignore_unmap = 0;
}

/*!
  Destructor, nothing special.
 */
Client::~Client()
{
    releaseWindow();
}


/*!
  Manages the clients. This means handling the very first maprequest:
  reparenting, initial geometry, initial state, placement, etc.
 */
void Client::manage( bool isMapped )
{
    getIcons();
    getWindowProtocols();
    layout()->setResizeMode( QLayout::Minimum );
    QRect geom( original_geometry );
    getWmNormalHints(); // get xSizeHint
    fetchName();

    if ( !XGetTransientForHint( qt_xdisplay(), (Window) win, (Window*) &transient_for ) )
	transient_for = None;

    bool placementDone = FALSE;

    if ( isMapped )
	placementDone = TRUE;
    else {
	if ( xSizeHint.flags & PPosition || xSizeHint.flags & USPosition ) {
	    // support for obsolete hints
	    if ( xSizeHint.x != 0 && geom.x() == 0 )
		geom.setRect( xSizeHint.x, geom.y(), geom.width(), geom.height() );
	    if ( xSizeHint.y != 0 && geom.y() == 0 )
		geom.setRect( geom.x(), xSizeHint.y, geom.width(), geom.height() );
	    placementDone = TRUE;
	}
	if ( xSizeHint.flags & USSize || xSizeHint.flags & PSize ) {
	    if ( xSizeHint.width != 0 )
		geom.setWidth( xSizeHint.width );
	    if ( xSizeHint.height != 0 )
		geom.setHeight( xSizeHint.height );
	}
    }

    // the clever activate() trick is necessary
    layout()->activate();
    resize ( sizeForWindowSize( geom.size() ) );
    layout()->activate();

    move( geom.x(), geom.y() );
    gravitate( FALSE );

    if ( !placementDone ) {
	workspace()->doPlacement( this );
	placementDone = TRUE;
    }

    // ### TODO check XGetWMHints() for initial mapping state, icon, etc. pp.
    // assume window wants to be visible on the current desktop
    desk = workspace()->currentDesktop();
    setMappingState( NormalState );
    show();

    if ( options->focusPolicyIsReasonable() )
	workspace()->requestFocus( this );

    // ignore unmap notify send by the xserver cause the window was already mapped
    if ( isMapped )
	ignore_unmap++;

}


/*!
  Gets the client's normal WM hints and reconfigures itself respectively.
 */
void Client::getWmNormalHints()
{
    // TODO keep in mind changing of fix size! if !isWithdrawn()!
    long msize;
    if (XGetWMNormalHints(qt_xdisplay(), win, &xSizeHint, &msize) == 0
	|| xSizeHint.flags == 0)
	xSizeHint.flags = PSize;      /* not specified */
}

/*!
  Fetches the window's caption (WM_NAME property). It will be
  stored in the client's caption().
 */
void Client::fetchName()
{
    char* name = 0;
    if ( XFetchName( qt_xdisplay(), win, &name ) && name ) {
	QString s = QString::fromLatin1( name );
	if ( s != caption() ) {
	    setCaption( QString::fromLatin1( name ) );
	    if ( !isWithdrawn() )
		captionChange( caption() );
	}
	XFree( name );
    }
}

/*!
  Sets the client window's mapping state. Possible values are
  WithdrawnState, IconicState, NormalState.
 */
void Client::setMappingState(int s){
    if ( !win)
	return;
  unsigned long data[2];
  data[0] = (unsigned long) s;
  data[1] = (unsigned long) None;

  state = s;
  XChangeProperty(qt_xdisplay(), win, qt_wm_state, qt_wm_state, 32,
		  PropModeReplace, (unsigned char *)data, 2);
}


/*!
  General handler for XEvents concerning the client window
 */
bool Client::windowEvent( XEvent * e)
{
    switch (e->type) {
    case UnmapNotify:
	return unmapNotify( e->xunmap );
    case MapRequest:
	return mapRequest( e->xmaprequest );
    case ConfigureRequest:
	return configureRequest( e->xconfigurerequest );
    case PropertyNotify:
	return propertyNotify( e->xproperty );
    case ButtonPress:
    case ButtonRelease:
	XAllowEvents( qt_xdisplay(), e->xbutton.time, ReplayPointer );
	break;
    case FocusIn:
	if ( e->xfocus.mode == NotifyUngrab )
	    break; // we don't care
	if ( e->xfocus.detail == NotifyPointer )
	    break;  // we don't care
	setActive( TRUE );
	break;
    case FocusOut:
	if ( e->xfocus.mode == NotifyGrab )
	    break; // we don't care
	if ( isShade() )
	    break; // we neither
	if ( e->xfocus.detail != NotifyNonlinear )
	    return TRUE; // hack for motif apps like netscape
	setActive( FALSE );
	break;
    default:
	break;
    }

    return TRUE; // we accept everything :-)
}


/*!
  Handles map requests of the client window
 */
bool Client::mapRequest( XMapRequestEvent& /* e */  )
{
    switch ( mappingState() ) {
    case WithdrawnState:
	manage();
	break;
    case IconicState:
	show();
	break;
    case NormalState:
	show(); // for safety
	break;
    }

    return TRUE;
}


/*!
  Handles unmap notify events of the client window
 */
bool Client::unmapNotify( XUnmapEvent& e )
{

    if ( ignore_unmap ) {
	ignore_unmap--;
	return TRUE;
    }


    switch ( mappingState() ) {
    case IconicState:
	// only react on sent events, all others are produced by us
	if ( e.send_event )
	    withdraw();
	break;
    case NormalState:
	if ( !windowWrapper()->isVisible() && !e.send_event )
	    return TRUE; // this event was produced by us as well
	
	// maybe we will be destroyed soon. Check this first.
	XEvent ev;
	QApplication::syncX();
	if  ( XCheckTypedWindowEvent (qt_xdisplay(), windowWrapper()->winId(),
				      DestroyNotify, &ev) ){
	    workspace()->destroyClient( this );
	    return TRUE;
	}
	
	// fall through
    case WithdrawnState: // however that has been possible....
	withdraw();
	break;
    }
    return TRUE;
}

/*!
  Withdraws the window and destroys the client afterwards
 */
void Client::withdraw()
{
    setMappingState( WithdrawnState );
    releaseWindow();
    workspace()->destroyClient( this );
}

/*!
  Handles configure  requests of the client window
 */
bool Client::configureRequest( XConfigureRequestEvent& e )
{
    if ( isShade() )
	setShade( FALSE );

    if ( e.value_mask & CWBorderWidth ) {
	// first, get rid of a window border
	XWindowChanges wc;
	unsigned int value_mask = 0;
	
	wc.border_width = 0;
	value_mask = CWBorderWidth;
	XConfigureWindow( qt_xdisplay(), win, value_mask, & wc );
    }

    if ( e.value_mask & (CWX | CWY ) ) {
	int nx = x();
	int ny = y();
	if ( e.value_mask & CWX )
	    nx = e.x;
	if ( e.value_mask & CWY )
	    ny = e.y;
	move( nx, ny );
    }

    if ( e.value_mask & (CWWidth | CWHeight ) ) {
	int nw = windowWrapper()->width();
	int nh = windowWrapper()->height();
	if ( e.value_mask & CWWidth )
	    nw = e.width;
	if ( e.value_mask & CWHeight )
	    nh = e.height;
	resize( sizeForWindowSize( QSize( nw, nh ) ) );
    }

    // TODO handle stacking!

    sendSynteticConfigureNotify();
    return TRUE;
}


/*!
  Handles property changes of the client window
 */
bool Client::propertyNotify( XPropertyEvent& e )
{
    switch ( e.atom ) {
    case XA_WM_NORMAL_HINTS:
	getWmNormalHints();
	break;
    case XA_WM_NAME:
	fetchName();
	break;
    case XA_WM_TRANSIENT_FOR:
	if ( !XGetTransientForHint( qt_xdisplay(), (Window) win, (Window*) &transient_for ) )
	    transient_for = None;
	break;
    case XA_WM_HINTS:
	getIcons();
	break;
    default:
	if ( e.atom == atoms->wm_protocols )
	    getWindowProtocols();
	else if ( e.atom == atoms->kwm_win_icon ) {
	    getIcons();
	}
	    
	break;
    }
    return TRUE;
}


/*!
  Auxiliary function to inform the client about the current window
  configuration.

 */
void Client::sendSynteticConfigureNotify()
{
    XConfigureEvent c;
    c.type = ConfigureNotify;
    c.event = win;
    c.window = win;
    c.x = x() + windowWrapper()->x();
    c.y = y() + windowWrapper()->y();
    c.width = windowWrapper()->width();
    c.height = windowWrapper()->height();
    c.border_width = 0;
    XSendEvent( qt_xdisplay(), c.event, TRUE, NoEventMask, (XEvent*)&c );
}



/*!
  Adjust the frame size \a frame according to he window's size hints.
 */
QSize Client::adjustedSize( const QSize& frame) const
{
    // first, get the window size for the given frame size s

    QSize wsize( frame.width() - ( width() - wwrap->width() ),
	     frame.height() - ( height() - wwrap->height() ) );

    return sizeForWindowSize( wsize );
}

/*!
  Calculate the appropriate frame size for the given window size \a
  wsize.

  \a wsize is adapted according to the window's size hints (minimum,
  maximum and incremental size changes).

 */
QSize Client::sizeForWindowSize( const QSize& wsize, bool ignore_height) const
{
    int w = wsize.width();
    int h = wsize.height();
    if (w<1) w = 1;
    if (h<1) h = 1;

    int bw = 0;
    int bh = 0;

    if (xSizeHint.flags & PBaseSize) {
	bw = xSizeHint.base_width;
	bh = xSizeHint.base_height;
	if (w < xSizeHint.base_width)
	    w = xSizeHint.base_width;
	if (h < xSizeHint.base_height)
	    h = xSizeHint.base_height;
    }

    if (xSizeHint.flags & PResizeInc) {
	if (  xSizeHint.width_inc > 0 ) {
	    int sx = (w - bw) / xSizeHint.width_inc;
	    w = bw + sx * xSizeHint.width_inc;
	}
	if (  xSizeHint.height_inc > 0 ) {
	    int sy = (h - bh) / xSizeHint.height_inc;
	    h = bh + sy * xSizeHint.height_inc;
	}
    }

    int ww = wwrap->width();
    int wh = 0;
    if ( !wwrap->testWState( WState_ForceHide ) )
	wh = wwrap->height();

    return QSize( QMIN( QMAX( width() - ww + w, minimumWidth() ),
			maximumWidth() ),
		  ignore_height? height()-wh : QMIN( QMAX( height() - wh + h, minimumHeight() ),
						     maximumHeight() ) );
}


/*!
  Reimplemented to provide move/resize
 */
void Client::mousePressEvent( QMouseEvent * e)
{
    if ( e->button() == LeftButton ) {
	qDebug("mouse press event %d %d ", e->pos().x(), e->pos().y() );
	if ( options->focusPolicyIsReasonable() )
	    workspace()->requestFocus( this );
	workspace()->raiseClient( this );
	mouseMoveEvent( e );
	buttonDown = TRUE;
	moveOffset = e->pos();
	invertedMoveOffset = rect().bottomRight() - e->pos();
    }
    else if ( e->button() == RightButton ) {
	workspace()->showPopup( e->globalPos(), this );
    }
}

/*!
  Reimplemented to provide move/resize
 */
void Client::mouseReleaseEvent( QMouseEvent * e)
{
    if ( e->button() == LeftButton ) {
	buttonDown = FALSE;
    }
}

/*!
  Reimplemented to provide move/resize
 */
void Client::mouseMoveEvent( QMouseEvent * e)
{
    if ( !buttonDown ) {
	mode = mousePosition( e->pos() );
	setMouseCursor( mode );
	return;
    }

    if ( mode !=  Center && shaded ) {
	wwrap->show();
	workspace()->requestFocus( this );
	shaded = FALSE;
    }

    qDebug("x=%d, y=%d", x(), y() );
    QPoint globalPos = e->pos() + geometry().topLeft();

    // TODO for MDI this has to be based on the parent window!
//     QPoint p = parentWidget()->mapFromGlobal( e->globalPos() );

//     if ( !parentWidget()->rect().contains(p) ) {
// 	if ( p.x() < 0 )
// 	    p.rx() = 0;
// 	if ( p.y() < 0 )
// 	    p.ry() = 0;
// 	if ( p.x() > parentWidget()->width() )
// 	    p.rx() = parentWidget()->width();
// 	if ( p.y() > parentWidget()->height() )
// 	    p.ry() = parentWidget()->height();
//     }

//     if ( testWState(WState_ConfigPending) )
// 	return;

    QPoint p = globalPos + invertedMoveOffset;

    QPoint pp = globalPos - moveOffset;

    QSize mpsize( geometry().right() - pp.x() + 1, geometry().bottom() - pp.y() + 1 );
    mpsize = adjustedSize( mpsize );
    QPoint mp( geometry().right() - mpsize.width() + 1,
	       geometry().bottom() - mpsize.height() + 1 );

    QRect geom = geometry();
    switch ( mode ) {
    case TopLeft:
	geom =  QRect( mp, geometry().bottomRight() ) ;
	break;
    case BottomRight:
	geom =  QRect( geometry().topLeft(), p ) ;
	break;
    case BottomLeft:
	geom =  QRect( QPoint(mp.x(), geometry().y() ), QPoint( geometry().right(), p.y()) ) ;
	break;
    case TopRight:
	geom =  QRect( QPoint(geometry().x(), mp.y() ), QPoint( p.x(), geometry().bottom()) ) ;
	break;
    case Top:
	geom =  QRect( QPoint( geometry().left(), mp.y() ), geometry().bottomRight() ) ;
	break;
    case Bottom:
	geom =  QRect( geometry().topLeft(), QPoint( geometry().right(), p.y() ) ) ;
	break;
    case Left:
	geom =  QRect( QPoint( mp.x(), geometry().top() ), geometry().bottomRight() ) ;
	break;
    case Right:
	geom =  QRect( geometry().topLeft(), QPoint( p.x(), geometry().bottom() ) ) ;
	break;
    case Center:
	geom.moveTopLeft( pp );
	break;
    default:
	break;
    }

    if ( geom.size() != size() ) {
	geom.setSize( adjustedSize( geom.size() ) );
	setGeometry( geom );
    }
    else if ( geom.topLeft() != geometry().topLeft() )
	move( geom.topLeft() );

}

/*!
  Reimplemented to provide move/resize
 */
void Client::enterEvent( QEvent * )
{
}

/*!
  Reimplemented to provide move/resize
 */
void Client::leaveEvent( QEvent * )
{
    if ( !buttonDown )
	setCursor( arrowCursor );
}


/*!
  Reimplemented to inform the client about the new window position.
 */
void Client::moveEvent( QMoveEvent * )
{
    sendSynteticConfigureNotify();
}


/*!\reimp
 */
void Client::showEvent( QShowEvent* )
{
    setMappingState( NormalState );
    windowWrapper()->show();// ########## hack for qt < 2.1
 }

/*!
  Reimplemented to hide the window wrapper as well. Also informs the
  workspace.
 */
void Client::hideEvent( QHideEvent* )
{
    windowWrapper()->hide();// ########## hack for qt < 2.1
    workspace()->clientHidden( this );
}



/*!
  Late initialize the client after the window has been managed.
  
  Ensure to call the superclasses init() implementation when subclassing.
 */
void Client::init()
{
}

/*!\fn captionChange( const QString& name )

  Indicates that the caption (the window title) changed to \a
  name. Subclasses shall then repaint the title string in a clever,
  fast mannor. The default implementation calls repaint( FALSE );
 */
void Client::captionChange( const QString& )
{
    repaint( FALSE );
}


/*!\fn activeChange( bool act )

  Indicates that the activation state changed to \a act.  Subclasses
  may want to indicate the new state graphically in a clever, fast
  mannor.  The default implementation calls repaint( FALSE );
 */
void Client::activeChange( bool )
{
    repaint( FALSE );
}

/*!
  Indicates that the application's icon changed to \a act.  Subclasses
  may want to indicate the new state graphically in a clever, fast
  mannor.  The default implementation calls repaint( FALSE );
 */
void Client::iconChange()
{
    repaint( FALSE );
}


/*!\fn maximizeChange( bool max )

  Indicates that the window was maximized or demaximized. \a max is
  set respectively. Subclasses may want to indicate the new state
  graphically, for example with a different icon.
 */
void Client::maximizeChange( bool )
{
}


/*!
  Paints a client window.

  The default implementation does nothing. To be implemented by subclasses.
 */
void Client::paintEvent( QPaintEvent * )
{
}



/*!
  Releases the window. The client has done its job and the window is still existing.
 */
void Client::releaseWindow()
{
    if ( win ) {
	gravitate( TRUE );
	windowWrapper()->releaseWindow();
	win = 0;
    }
}

/*!
  Invalidates the window to avoid the client accessing it again.

  This function is called by the workspace when the window has been
  destroyed.
 */
void Client::invalidateWindow()
{
    win = 0;
    windowWrapper()->invalidateWindow();
}

/*!
  Returns the minimum size. This function differs from QWidget::minimumSize()
  and is to be preferred
 */
QSize Client::minimumSize() const
{
    return QSize( minimumWidth(), minimumHeight() );
}
/*!
  Returns the minimum width. This function differs from QWidget::minimumWidth()
  and is to be preferred
 */
int Client::minimumWidth() const
{
    if (xSizeHint.flags & PMinSize)
	return QMAX( width() - wwrap->width() + xSizeHint.min_width,
		     QWidget::minimumWidth() );
    else
	return QWidget::minimumWidth();
}
/*!
  Returns the minimum height. This function differs from QWidget::minimumHeight()
  and is to be preferred
 */
int Client::minimumHeight() const
{
    if (xSizeHint.flags & PMinSize)
	return QMAX( height() - wwrap->height() + xSizeHint.min_height,
		     QWidget::minimumHeight() );
    else
	return QWidget::minimumHeight();
}

/*!
  Returns the maximum size. This function differs from QWidget::maximumSize()
  and is to be preferred
 */
QSize Client::maximumSize() const
{
    return QSize( maximumWidth(), maximumHeight() );
}
/*!
  Returns the maximum width. This function differs from QWidget::maximumWidth()
  and is to be preferred
 */
int Client::maximumWidth() const
{
    if (xSizeHint.flags & PMaxSize)
	return QMIN( width() - wwrap->width() + xSizeHint.max_width,
		     QWidget::maximumWidth() );
    else
	return QWidget::maximumWidth();
}
/*!
  Returns the maximum height. This function differs from QWidget::maximumHeight()
  and is to be preferred
 */
int Client::maximumHeight() const
{
    if (xSizeHint.flags & PMaxSize)
	return QMIN( height() - wwrap->height() + xSizeHint.max_height,
		     QWidget::maximumHeight() );
    else
	return QWidget::maximumHeight();
}


void Client::iconify()
{
    setMappingState( IconicState );
    hide();
    // TODO animation (virtual function)
}

void Client::closeWindow()
{
  if ( Pdeletewindow ){
      sendClientMessage( win, atoms->wm_protocols, atoms->wm_delete_window);
  }
  else {
    // client will not react on wm_delete_window. We have not choice
    // but destroy his connection to the XServer.
      XKillClient(qt_xdisplay(), win );
      workspace()->destroyClient( this );
  }
}

void Client::maximize( MaximizeMode /*m*/)
{
    if ( geom_restore.isNull() ) {
	geom_restore = geometry();
	setGeometry( workspace()->geometry() );
	maximizeChange( TRUE );
    }
    else {
	setGeometry( geom_restore );
	QRect invalid;
	geom_restore = invalid;
	maximizeChange( FALSE );
    }
}

void Client::maximize()
{
    maximize( MaximizeFull );
}


void Client::fullScreen()
{
    workspace()->makeFullScreen( this );
}



/*!
  Catch events of the WindowWrapper
 */
bool Client::eventFilter( QObject *o, QEvent * e)
{
    if ( o != wwrap )
	return FALSE;
    switch ( e->type() ) {
    case QEvent::MouseButtonPress:
	if ( options->focusPolicyIsReasonable() )
	    workspace()->requestFocus( this );
	workspace()->raiseClient( this );
	break;
    case QEvent::MouseButtonRelease:
	break;
    default:
	break;
    }

    return FALSE;
}

void Client::gravitate( bool invert ){
  int gravity, dx, dy;
  dx = dy = 0;

  gravity = NorthWestGravity;
  if ( xSizeHint.flags & PWinGravity)
      gravity = xSizeHint.win_gravity;

  switch (gravity) {
  case NorthWestGravity:
    dx = 0;
    dy = 0;
    break;
  case NorthGravity:
    dx = -windowWrapper()->x();
    dy = 0;
    break;
  case NorthEastGravity:
    dx = -( width() - windowWrapper()->width() );
    dy = 0;
    break;
  case WestGravity:
    dx = 0;
    dy = -windowWrapper()->y();
    break;
  case CenterGravity:
  case StaticGravity:
    dx = -windowWrapper()->x();
    dy = -windowWrapper()->y();
    break;
  case EastGravity:
    dx = -( width() - windowWrapper()->width() );
    dy = -windowWrapper()->y();
    break;
  case SouthWestGravity:
    dx = 0;
    dy = -( height() - windowWrapper()->height() );
    break;
  case SouthGravity:
    dx = -windowWrapper()->x();
    dy = -( height() - windowWrapper()->height() );
    break;
  case SouthEastGravity:
    dx = -( width() - windowWrapper()->width() - 1 );
    dy = -( height() - windowWrapper()->height() - 1 );
    break;
  }
  if (invert)
      move( x() - dx, y() - dy );
  else
      move( x() + dx, y() + dy );
}

/*!
  Reimplement to handle crossing events (qt should provide xroot, yroot)
*/
bool Client::x11Event( XEvent * e)
{
    if ( e->type == EnterNotify ) {
	if ( options->focusPolicy != Options::ClickToFocus )
	    workspace()->requestFocus( this );
	return TRUE;
    }
    if ( e->type == LeaveNotify ) {
	if ( !buttonDown )
	    setCursor( arrowCursor );
	if ( options->focusPolicy == Options::FocusStricklyUnderMouse ) {
	    if ( isActive() && !rect().contains( QPoint( e->xcrossing.x, e->xcrossing.y ) ) )
		workspace()->requestFocus( 0 ) ;
	}
	return TRUE;
    }
    return FALSE;
}


/*!
  Returns a logical mouse position for the cursor position \a
  p. Possible positions are: Nowhere, TopLeft , BottomRight,
  BottomLeft, TopRight, Top, Bottom, Left, Right, Center
 */
Client::MousePosition Client::mousePosition( const QPoint& p ) const
{
    const int range = 16;
    const int border = 4;

    MousePosition m = Nowhere;


    if ( ( p.x() > border && p.x() < width() - border )
	 && ( p.y() > border && p.y() < height() - border ) )
	return Center;

    if ( p.y() <= range && p.x() <= range)
	m = TopLeft;
    else if ( p.y() >= height()-range && p.x() >= width()-range)
	m = BottomRight;
    else if ( p.y() >= height()-range && p.x() <= range)
	m = BottomLeft;
    else if ( p.y() <= range && p.x() >= width()-range)
	m = TopRight;
    else if ( p.y() <= border )
	m = Top;
    else if ( p.y() >= height()-border )
	m = Bottom;
    else if ( p.x() <= border )
	m = Left;
    else if ( p.x() >= width()-border )
	m = Right;
    else
	m = Center;
    return m;
}

/*!
  Sets an appropriate cursor shape for the logical mouse position \a m

  \sa QWidget::setCursor()
 */
void Client::setMouseCursor( MousePosition m )
{
    switch ( m ) {
    case TopLeft:
    case BottomRight:
	setCursor( sizeFDiagCursor );
	break;
    case BottomLeft:
    case TopRight:
	setCursor( sizeBDiagCursor );
	break;
    case Top:
    case Bottom:
	setCursor( sizeVerCursor );
	break;
    case Left:
    case Right:
	setCursor( sizeHorCursor );
	break;
    default:
	setCursor( arrowCursor );
	break;
    }
}


bool Client::isShade() const
{
    return shaded;
}

void Client::setShade( bool s )
{
    if ( shaded == s )
	return;

    shaded = s;

    if (shaded ) {
	QSize s( sizeForWindowSize( QSize( windowWrapper()->width(), 0), TRUE ) );
	windowWrapper()->hide();
	resize (s );
    } else {
	QSize s( sizeForWindowSize( windowWrapper()->size() )  );
	windowWrapper()->show();
	resize (  s );
	if ( isActive() )
	    workspace()->requestFocus( this );
    }
}


/*!
  Sets the client's active state to \a act.

  This function does only change the visual appearance of the client,
  it does not change the focus setting. Use
  Workspace::activateClient() or Workspace::requestFocus() instead.

  If a client receives or looses the focus, it calls setActive() on
  its own.

 */
void Client::setActive( bool act)
{
    if ( active == act )
	return;

    active = act;
    if ( active )
	workspace()->setActiveClient( this );
    activeChange( active );
}


void Client::getIcons()
{
    icon_pix = KWM::icon( win, 32, 32 ); // TODO sizes from workspace
    miniicon_pix = KWM::miniIcon( win, 16, 16 );
    if ( !isWithdrawn() )
	iconChange();
}

void Client::getWindowProtocols(){
  Atom *p;
  int i,n;

  Pdeletewindow = 0;
  Ptakefocus = 0;

  if (XGetWMProtocols(qt_xdisplay(), win, &p, &n)){
      for (i = 0; i < n; i++)
	  if (p[i] == atoms->wm_delete_window)
	      Pdeletewindow = 1;
	  else if (p[i] == atoms->wm_take_focus)
	      Ptakefocus = 1;
      if (n>0)
	  XFree(p);
  }
}


/*!
  Puts the focus on this window. Clients should never calls this
  themselves, instead they should use Workspace::requestFocus().
 */
void Client::takeFocus()
{
    if ( !Ptakefocus )
	XSetInputFocus( qt_xdisplay(), win, RevertToPointerRoot, CurrentTime );
    else
	sendClientMessage(win, atoms->wm_protocols, atoms->wm_take_focus);
}


NoBorderClient::NoBorderClient( Workspace *ws, WId w, QWidget *parent=0, const char *name=0 )
    : Client( ws, w, parent, name )
{
    QGridLayout* g = new QGridLayout( this, 0, 0, 0 );
    g->addWidget( windowWrapper(), 0, 0 );
}

NoBorderClient::~NoBorderClient()
{
}
