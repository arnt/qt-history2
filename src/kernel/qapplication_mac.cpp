#include "qapplication.h"
#include "qt_mac.h"
#include "q1xcompatibility.h"
#include "qpaintdevicemetrics.h"
#include "qkeycode.h"
#include "qobjectlist.h"
#include "qwidgetlist.h"
#include "qwidgetintdict.h"
#include "qpainter.h"
#include "qsocketnotifier.h"
#include "qevent.h"
#include <stdio.h>
#include <Math64.h>
//#include <OpenTransport.h>
//#include <OpenTptInternet.h>
#include "qsessionmanager.h"
#include "qstringlist.h"
#include <Files.h>

class SockRec {

public:

    int sockfd;
    int type;
    QObject * obj;

};

typedef QList<QCursor> QCursorList;
typedef QList<SockRec> SockList;

SockList * thesocs=0;

static bool app_do_modal=FALSE;
static bool app_exit_loop=FALSE;
extern QWidgetList * qt_modal_stack;
static QWidget * popupOfPopupButtonFocus=0;
static QWidget * popupButtonFocus=0;
static bool popupCloseDownMode=FALSE;
static QCursorList * cursorStack=0;

extern QWidget * the_grabbed;

QObject * mywibble=0;
int myinterval=0;

void QApplication::setMainWidget(QWidget * mainWidget)
{
    main_widget=mainWidget;
}

unsigned int time_diff(UnsignedWide time1,UnsignedWide time2)
{
    UInt64 ui,ui2;
    int ret;
    ui=UnsignedWideToUInt64(time1);
    ui2=UnsignedWideToUInt64(time2);
    ret=U64Subtract(ui2,ui);
    return ret;
}

int QApplication::exec()
{
    quit_now=FALSE;
    quit_code=0;
    enter_loop();
    return quit_code;
}

bool mouse_down=false;

bool QApplication::processNextEvent(bool canWait)
{
    EventRecord event;
    sendPostedEvents();
    do {
	WaitNextEvent(everyEvent,&event,15L,nil);
	if(event.what==nullEvent) {
	    macProcessEvent((MSG *)(&event));
	}
    } while(event.what==nullEvent);
    if(macProcessEvent((MSG *)(&event))==1)
	return TRUE;
    if(quit_now || app_exit_loop)
	return FALSE;
    sendPostedEvents();
    return TRUE;
}


extern WId myactive;

static const int KeyTbl[]={
    144667,          Qt::Key_Escape,         // misc keys
    143369,             Qt::Key_Tab,
    144136,       Qt::Key_Backspace,
    140301,          Qt::Key_Return,
    160513,            Qt::Key_Home,           // cursor movement
    162588,            Qt::Key_Left,
    163358,              Qt::Key_Up,
    162845,           Qt::Key_Right,
    163103,            Qt::Key_Down,
    160779,           Qt::Key_Prior,
    162060,            Qt::Key_Next,
    /*
      0,         Qt::Key_Shift,          // modifiers
      0,       Qt::Key_Control,
      0,       Qt::Key_Control,
      0,          Qt::Key_Meta,
      0,          Qt::Key_Meta,
      0,           Qt::Key_Alt,
      0,           Qt::Key_Alt,
      0,       Qt::Key_CapsLock,
      0,        Qt::Key_NumLock,
      0,     Qt::Key_ScrollLock,
    */
    143648,        Qt::Key_Space,          // numeric keypad
    150531,        Qt::Key_Enter,
    151869,        Qt::Key_Equal,
    148266,     Qt::Key_Asterisk,
    148779,          Qt::Key_Plus,
    151085,     Qt::Key_Minus,
    147758,      Qt::Key_Period,
    150319,       Qt::Key_Slash,
    162320, Qt::Key_F1,       // Function keys
    161808, Qt::Key_F2,
    156432, Qt::Key_F3,
    161296, Qt::Key_F4,
    155664, Qt::Key_F5,
    155920, Qt::Key_F6,
    156176, Qt::Key_F7,
    156688, Qt::Key_F8,
    156944, Qt::Key_F9,
    158992, Qt::Key_F10,
    157456, Qt::Key_F11,
    159504, Qt::Key_F12,
    149275, Qt::Key_NumLock,
    133796, Qt::Key_plusminus,
    0,0
};

int get_key(int key)
{
    int i=0;
    while(KeyTbl[i]) {
	if(key==KeyTbl[i]) {
	    return KeyTbl[i+1];
	}
	i+=2;
    }
    return 0;
}

void QApplication::do_mouse_down(void * es)
{
    qDebug("do_mouse_down");
    EventRecord * er=(EventRecord *)es;
    WindowPtr wp;
    short part;
    Point wizzle=er->where;
    part=FindWindow(er->where,&wp);
    QWidget * widget;
    switch(part) {
    case inGoAway:
	widget=QWidget::find((WId)wp);
	if(widget) {
	    if(widget->close(FALSE)) {
		widget->hide();
	    } else {
	    }
	} else {
	    qWarning("Close for unknown widget");
	}
	break;
    case inDrag:
	DragWindow(wp,er->where,&qd.screenBits.bounds);
	break;
    case inContent:
	Point pp=er->where;
	Point pp2=er->where;
	GlobalToLocal(&pp2);
	widget=QApplication::widgetAt(pp2.h,pp2.v,true);
	if(widget) {
	    mouse_down=true;
	    if(myactive!=(WId)wp) {
		SelectWindow(wp);
		QWidget * qw=QWidget::find(myactive);
		QFocusEvent qf(QEvent::FocusOut);
		QApplication::sendEvent(qw,&qf);
		QFocusEvent qf2(QEvent::FocusIn);
		QApplication::sendEvent(widget,&qf2);
		myactive=(WId)wp;
	    }
	    if(the_grabbed)
		widget=the_grabbed;
	    GrafPort * g=(GrafPort *)wp;
	    int x,y;
	    SetPort(wp);
	    QPoint p(er->where.h,er->where.v);
	    QPoint p2;
	    p2=widget->mapFromGlobal(p);
	    QMouseEvent qme(QEvent::MouseButtonPress,p2,
			    QPoint(er->where.h,er->where.v),
			    QMouseEvent::LeftButton,0);
	    QApplication::sendEvent(widget,&qme);
	    //QMouseEvent qme2(QEvent::MouseButtonRelease,p2,
	    //                QPoint(er->where.h,er->where.v),
	    //                QMouseEvent::LeftButton,0);
	    //QApplication::sendEvent(widget,&qme2);
	} else {
	    qWarning("Couldn't find mouse down!");
	}
	break;
    case inGrow:
	qDebug("inGrow");
	Rect limits;
	widget=QWidget::find((WId)wp);
	SetRect(&limits,20,20,50000,50000);
	if(widget) {
	    qDebug("Widget found");
	    if(widget->extra) {
		SetRect(&limits,widget->extra->minw,widget->extra->minh,
			widget->extra->maxw,widget->extra->maxh);
	    }
	}
	int thesize=0;
	thesize=GrowWindow(wp,wizzle,&limits);
	if(thesize) {
	    if(LoWord(thesize)<1000 && LoWord(thesize)>0 &&
	       HiWord(thesize)<1000 && HiWord(thesize)>0) {
		if(widget) {
		    int ow,oh;
		    ow=widget->width();
		    oh=widget->height();
		    int nw=LoWord(thesize);
		    int nh=HiWord(thesize);
		    widget->resize(nw,nh);
		    // nw/nh might not match the actual size if setSizeIncrement
		    // is used
		    QResizeEvent qre(QSize(widget->width(),widget->height()),
				     QSize(ow,oh));
		    QApplication::sendEvent(widget,&qre);
		    widget->resizeEvent(&qre);
		}
	    }
	}
	break;
    case inZoomIn:
    case inZoomOut:
	if(TrackBox(wp,er->where,part)==true) {
	    SetPort(wp);
	    EraseRect(&wp->portRect);
	    ZoomWindow(wp,part,false);
	    InvalRect(&wp->portRect);
	}
	break;
    }
}

extern QWidget * mac_pre;

RgnHandle cliprgn=0;
bool ignorecliprgn=true;

UnsignedWide thesecs;

int QApplication::macProcessEvent(MSG * m)
{
    mac_pre=0;
    WindowPtr wp;
    EventRecord * er=(EventRecord *)m;
    QWidget * twidget=QWidget::find((WId)er->message);
    Point p2=er->where;
    QWidget * widget=QApplication::widgetAt(p2.h,p2.v,true);
    if(er->what==updateEvt) {
	wp=(WindowPtr)er->message;
	SetPort(wp);
	SetOrigin(0,0);
	mac_pre=0;
	BeginUpdate(wp);
	//DrawGrowIcon(wp);
	if(!cliprgn) {
	    cliprgn=NewRgn();
	}
	GetClip(cliprgn);
	EndUpdate(wp);
	if(!twidget) {
	    qWarning("Couldn't find paint widget for %d!",wp);
	} else {
	    twidget->crect.setWidth(twidget->metric(PDM_WIDTH)-1);
	    twidget->crect.setHeight(twidget->metric(PDM_HEIGHT)-1);
	    ignorecliprgn=false;
	    twidget->propagateUpdates(0,0,twidget->width(),twidget->height());
	    ignorecliprgn=true;
	}
    } else if(er->what==keyDown) {
	short mychar;
	mychar=er->message & charCodeMask;
	char buf[2];
	buf[0]=mychar;
	buf[1]='\0';
	QKeyEvent ke(QEvent::KeyPress,get_key(er->message),mychar,0,QString(buf));
	if(!widget) {
	    qWarning("Can't find %d!",myactive);
	    return 0;
	}
	QApplication::sendEvent(twidget,&ke);
	QApplication::sendEvent(widget,&ke);
    } else if(er->what==nullEvent) {
	short part;
	part=FindWindow(er->where,&wp);
	GrafPort * gp;
	if(mywibble) {
	    UnsignedWide xtime;
	    Microseconds(&xtime);
	    unsigned int splung=time_diff(thesecs,xtime);
	    if((splung/1000)>myinterval) {
		QTimerEvent qte(0);
		QApplication::sendEvent(mywibble,&qte);
		thesecs=xtime;
	    }

	}
	if(part==inContent) {
	    if(widget) {
		if(the_grabbed)
		    widget=the_grabbed;
		GrafPort * g=(GrafPort *)wp;
		int x,y;
		SetPort(wp);
		QPoint p(er->where.h,er->where.v);
		QPoint p2;
		p2=widget->mapFromGlobal(p);
		if(StillDown()) {
		    QMouseEvent qme(QEvent::MouseMove,p2,
				    p,
				    QMouseEvent::LeftButton,QMouseEvent::LeftButton);
		    QApplication::sendEvent(widget,&qme);
		} else {
		    QMouseEvent qme(QEvent::MouseMove,p2,
				    p,
				    0,0);
		    QApplication::sendEvent(widget,&qme);
		}
	    }
	}
	/*
	  if(thesocs) {
	  // Check for socket activity
	  SockRec * sr;
	  sr=thesocs->first();
	  while(sr) {
	  OTResult otr;
	  otr=OTLook(((EndpointRef)sr->sockfd));
	  if(otr==T_LISTEN) {
          if(sr->type==QSocketNotifier::Write) {
	  QEvent event(QEvent::SockAct);
	  QApplication::sendEvent( sr->obj, &event );
	  }
	  } else if(otr==T_CONNECT) {
	  // Ignore?
	  } else if(otr==T_DISCONNECT) {
          if(sr->type==QSocketNotifier::Exception) {
	  QEvent event(QEvent::SockAct);
	  QApplication::sendEvent( sr->obj, &event );
	  }
	  } else if(otr==T_UDERR) {
          if(sr->type==QSocketNotifier::Exception) {
	  QEvent event(QEvent::SockAct);
	  QApplication::sendEvent( sr->obj, &event );
	  }
	  } else if(otr==T_DATA) {
          if(sr->type==QSocketNotifier::Read) {
	  QEvent event(QEvent::SockAct);
	  QApplication::sendEvent( sr->obj, &event );
	  }
	  } else if(otr==T_ORDREL) {
          if(sr->type==QSocketNotifier::Exception) {
	  QEvent event(QEvent::SockAct);
	  QApplication::sendEvent( sr->obj, &event );
	  }
	  } else if(otr==T_PASSCON) {
          if(sr->type==QSocketNotifier::Write) {
	  QEvent event(QEvent::SockAct);
	  QApplication::sendEvent( sr->obj, &event );
	  }
	  } else if(otr==kOTProviderIsClosed) {
          if(sr->type==QSocketNotifier::Exception) {
	  QEvent event(QEvent::SockAct);
	  QApplication::sendEvent( sr->obj, &event );
	  }
	  } else if(otr==kOTNoError) {
	  } else {
	  }
	  sr=thesocs->next();
	  }
	  }
	*/
    } else if(er->what==keyUp) {
	short mychar;
	mychar=er->message & charCodeMask;
	char buf[2];
	buf[0]=mychar;
	buf[1]='\0';
	QKeyEvent ke(QEvent::KeyRelease,get_key(er->message),mychar,
		     0,QString(buf));
	wp=(WindowPtr)er->message;
	if(!widget) {
	    qWarning("Can't find %d!",myactive);
	}
	QApplication::sendEvent(widget,&ke);
	QApplication::sendEvent(twidget,&ke);
    } else if(er->what==mouseDown) {
	do_mouse_down(er);
    } else if(er->what==mouseUp) {
	short part;
	part=FindWindow(er->where,&wp);
	GrafPort * gp;
	if(part==inContent) {
	    if(the_grabbed) {
		widget=the_grabbed;
	    } else {
	        Point pp2=er->where;
		GlobalToLocal(&pp2);
		widget=QApplication::widgetAt(pp2.h,pp2.v,true);
	    }
	    if(widget) {
		GrafPort * g=(GrafPort *)wp;
		int x,y;
		SetPort(wp);
		QPoint p(er->where.h,er->where.v);
		QPoint p2;
		p2=widget->mapFromGlobal(p);
		QMouseEvent qme(QEvent::MouseButtonRelease,p2,
				QPoint(er->where.h,er->where.v),
				QMouseEvent::LeftButton,0);
		QApplication::sendEvent(widget,&qme);
	    }
	}
    } else {
	qWarning("  Type %d",er->what);
    }
    return 0;
}

void QApplication::processEvents(int maxtime)
{
    UnsignedWide myStartTime;
    Microseconds(&myStartTime);
    UInt64 ui,ui2;
    ui=UnsignedWideToUInt64(myStartTime);
    UnsignedWide myEndTime;
    EventRecord event;
    sendPostedEvents();
    int ret;
    do {
	WaitNextEvent(everyEvent,&event,15L,nil);
	if(event.what==nullEvent && mouse_down) {
	    macProcessEvent((MSG *)(&event));
	}
	Microseconds(&myEndTime);
	ui2=UnsignedWideToUInt64(myEndTime);
	ret=U64Subtract(ui,ui2);
    } while((ret/1000)<maxtime);
    sendPostedEvents();
}

static QWidget * recursive_match(QWidget * widg,int x,int y)
{
    // Keep looking until we find ourselves in a widget with no kiddies
    // where the x,y is
    if(!widg)
	return 0;
    const QObjectList * foo=widg->children();
    if(!foo) {
	// No children
	return widg;
    }
    QObjectListIt it(*foo);
    QObject * bar;
    QWidget * frobnitz;
    bar=it.toFirst();
    do {
	if(bar->inherits("QWidget")) {
	    frobnitz=(QWidget *)bar;
	    int wx,wy,wx2,wy2;
	    wx=frobnitz->x();
	    wy=frobnitz->y();
	    wx2=wx+frobnitz->width();
	    wy2=wy+frobnitz->height();
	    qDebug("recursive_match %d %d  %d %d  %d %d",frobnitz->x(),
		    frobnitz->y(),wx,wy,wx2,wy2);
	    if(x>=wx && y>=wy && x<=wx2 && y<=wy2) {
		return recursive_match(frobnitz,x-wx,y-wy);
	    }
	}
	bar=++it;
    } while(bar!=0);
    // If we get here, it's within a widget that has children, but isn't in any
    // of the children
    return widg;
}

QWidget * QApplication::widgetAt(int x,int y,bool child)
{
    // Need to handle child/top-level stuff - right now only do top-levels
    mac_pre=0;
    Point p;
    p.h=x;
    p.v=y;
    WindowPtr wp;
    FindWindow(p,&wp);
    QWidget * widget;
    QWidget * wodget;
    widget=QWidget::find((WId)wp);
    if(!widget) {
	qWarning("Couldn't find %d",wp);
	return 0;
    }
    if(!child) {
	return widget;
    } else {
	wodget=recursive_match(widget,x,y);
	return wodget;
    }
}

void QApplication::openPopup(QWidget * popup)
{
    if ( !popupWidgets ) {                      // create list
        popupWidgets = new QWidgetList;
        CHECK_PTR( popupWidgets );
    }
    popupWidgets->append( popup );              // add to end of list
    // Should grab/ungrab keyboard and stuff
    active_window = popup;
    QFocusEvent::setReason( QFocusEvent::ActiveWindow );
    if (active_window->focusWidget())
        active_window->focusWidget()->setFocus();
    else
        active_window->setFocus();
    QFocusEvent::resetReason();
}

void QApplication::closePopup(QWidget * popup)
{
    if ( !popupWidgets )
        return;

    popupWidgets->removeRef( popup );
    if (popup == popupOfPopupButtonFocus) {
        popupButtonFocus = 0;
        popupOfPopupButtonFocus = 0;
    }
    if ( popupWidgets->count() == 0 ) {         // this was the last popup
        popupCloseDownMode = TRUE;              // control mouse events
        delete popupWidgets;
        popupWidgets = 0;
        active_window = 0;
    } else {
	// popups are not focus-handled by the window system (the
	// first popup grabbed the keyboard), so we have to do that
	// manually: A popup was closed, so the previous popup gets
	// the focus.
	QFocusEvent::setReason( QFocusEvent::ActiveWindow );
	active_window = popupWidgets->getLast();
	if (active_window->focusWidget())
	    active_window->focusWidget()->setFocus();
	else
	    active_window->setFocus();
	QFocusEvent::resetReason();
    }
}

#ifndef QT_NO_CURSOR

void QApplication::setOverrideCursor(const QCursor &cursor, bool replace)
{
    if ( !cursorStack ) {
        cursorStack = new QCursorList;
        CHECK_PTR( cursorStack );
        cursorStack->setAutoDelete( TRUE );
    }
    app_cursor = new QCursor( cursor );
    CHECK_PTR( app_cursor );
    if ( replace )
        cursorStack->removeLast();
    cursorStack->append( app_cursor );
    QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
    register QWidget *w;
    while ( (w=it.current()) ) {                // for all widgets that have
        if ( w->testWState(WState_OwnCursor) )  //   set a cursor
	    ++it;
    }
}

void QApplication::restoreOverrideCursor()
{
    if ( !cursorStack )                         // no cursor stack
        return;
    cursorStack->removeLast();
    app_cursor = cursorStack->last();
    QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
    register QWidget *w;
    while ( (w=it.current()) ) {         // set back to original cursors
        if ( w->testWState(WState_OwnCursor) )
	    ++it;
    }
    if ( !app_cursor ) {
        delete cursorStack;
        cursorStack = 0;
    }
}

#endif

void * qt_xdisplay()
{
    return 0;
}

void qAddPostRoutine(Q_CleanUpFunction p)
{
}

void QApplication::setGlobalMouseTracking( bool enable )
{
    bool tellAllWidgets;
    if ( enable ) {
        tellAllWidgets = (++app_tracking == 1);
    } else {
        tellAllWidgets = (--app_tracking == 0);
    }
    if ( tellAllWidgets ) {
        QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
        register QWidget *w;
        while ( (w=it.current()) ) {
            if ( app_tracking > 0 ) {           // switch on
                if ( !w->testWState(WState_MouseTracking) ) {
                    w->setMouseTracking( TRUE );
                    w->clearWState(WState_MouseTracking);
                }
            } else {                            // switch off
                if ( !w->testWState(WState_MouseTracking) ) {
                    w->setWState(WState_MouseTracking);
                    w->setMouseTracking( FALSE );
                }
            }
            ++it;
        }
    }
}

bool QApplication::macEventFilter(void **)
{
    return false;
}

bool qKillTimer( int id )
{
    mywibble=0;
    myinterval=0;
    return true;
}

bool qKillTimer( QObject *obj )
{
    mywibble=0;
    myinterval=0;
    return true;
}

int qStartTimer( int interval, QObject *obj )
{
    mywibble=obj;
    myinterval=interval;
    return 1;
}

void qt_enter_modal( QWidget *widget )
{
    if ( !qt_modal_stack ) {                       // create modal stack
        qt_modal_stack = new QWidgetList;
        CHECK_PTR( qt_modal_stack );
    }
    qt_modal_stack->insert( 0, widget );
    app_do_modal = TRUE;
}

bool qt_modal_state()
{
    return app_do_modal;
}

void qt_leave_modal( QWidget *widget )
{
    if ( qt_modal_stack && qt_modal_stack->removeRef(widget) ) {
        if ( qt_modal_stack->isEmpty() ) {
            delete qt_modal_stack;
            qt_modal_stack = 0;
        }
    }
    app_do_modal = qt_modal_stack != 0;
}

void qt_init( int *argcptr, char **argv, QApplication::Type )
{
    InitGraf(&qd.thePort);
    InitFonts();
    InitWindows();
    InitMenus();
    TEInit();
    InitDialogs(0L);
    FlushEvents(everyEvent,0);
    InitCursor();
    SetPort(qd.thePort);
    //InitOpenTransport();
    Microseconds(&thesecs);
}

void qt_cleanup()
{
    //CloseOpenTransport();
}

bool qt_set_socket_handler( int sockfd, int type, QObject *obj, bool enable )
{
    if(!thesocs) {
	thesocs=new SockList();
    }
    if(enable) {
	SockRec * sr;
	sr=new SockRec();
	sr->sockfd=sockfd;
	sr->type=type;
	sr->obj=obj;
	thesocs->append(sr);
    } else {
	SockRec * sr;
	sr=thesocs->first();
	while(sr) {
	    if(sr->sockfd==sockfd && sr->type==type && sr->obj==obj) {
		thesocs->remove();
		return true;
	    }
	    sr=thesocs->next();
	}
	return false;
    }
    return true;
}

QSessionManager::QSessionManager( QApplication * app,QString &session ) :
    QObject(app)
{
}

QSessionManager::~QSessionManager()
{
}

QString QSessionManager::sessionId() const
{
    return QString();
}

bool QSessionManager::allowsInteraction()
{
    return false;
}

bool QSessionManager::allowsErrorInteraction()
{
    return false;
}

void QSessionManager::release()
{
}

void QSessionManager::cancel()
{
}

void QSessionManager::setRestartHint( RestartHint )
{
}

QSessionManager::RestartHint QSessionManager::restartHint() const
{
    return QSessionManager::RestartIfRunning;
}

void QSessionManager::setRestartCommand( const QStringList& )
{
}

QStringList QSessionManager::restartCommand() const
{
    return QStringList();
}

void QSessionManager::setDiscardCommand( const QStringList& )
{
}

QStringList QSessionManager::discardCommand() const
{
    return QStringList();
}

void QSessionManager::setProperty( const QString& name, const QString& value )
{
}

void QSessionManager::setProperty( const QString& name,
				   const QStringList& value )
{
}

bool QSessionManager::isPhase2() const
{
    return false;
}

void QSessionManager::requestPhase2()
{
}

int QApplication::cursorFlashTime()
{
    return 10;
}

int QApplication::doubleClickInterval()
{
    return 10;
}

QObject * qt_clipboard=0;


