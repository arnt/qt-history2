#include <qglobal.h>
#if defined(Q_WS_MAC) && !defined(QMAC_QMENUBAR_NO_NATIVE)
#include <ctype.h>
#include "qt_mac.h"
#include <unistd.h>

#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qintdict.h>
#include <qstring.h>
#include <qapplication.h>
#include <qaccel.h>
#include <qregexp.h>
#include <qmessagebox.h>

QCString p2qstring(const unsigned char *); //qglobal.cpp
void qt_event_request_menubarupdate(); //qapplication_mac.cpp

//internal class
class QMenuBar::MacPrivate {
public:
    MacPrivate() : commands(NULL), popups(NULL), mac_menubar(NULL), 
	apple_menu(NULL), in_apple(0), dirty(1) { }
    ~MacPrivate() { clear(); delete popups; delete commands; }

    class CommandBinding {
    public:
	CommandBinding(QPopupMenu *m, uint i) : qpopup(m), index(i) { }
	QPopupMenu *qpopup;
	int index;
    };
    QIntDict<CommandBinding> *commands;

    class PopupBinding {
    public:
	PopupBinding(QPopupMenu *m, MenuRef r, bool b) : qpopup(m), macpopup(r), tl(b) { }
	~PopupBinding() { if(tl) DeleteMenu(GetMenuID(macpopup)); DisposeMenu(macpopup); }
	QPopupMenu *qpopup;
	MenuRef macpopup;
	bool tl;
    };
    QIntDict<PopupBinding> *popups;
    MenuBarHandle mac_menubar;
    MenuRef apple_menu;
    int in_apple;
    uint dirty;

    void clear() {
	in_apple = 0;
	if(apple_menu) {
	    DeleteMenu(GetMenuID(apple_menu));
	    DisposeMenu(apple_menu);
	}
	if(popups)
	    popups->clear();
	if(commands)
	    commands->clear();
	if(mac_menubar) {
	    DisposeMenuBar(mac_menubar);
	    mac_menubar = NULL;
	}
    }
};
static QMenuBar* activeMenuBar = NULL; //The current global menubar

#if !defined(QMAC_QMENUBAR_NO_EVENT)
//event callbacks
QMAC_PASCAL OSStatus 
QMenuBar::qt_mac_menubar_event(EventHandlerCallRef er, EventRef event, void *)
{
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    bool handled_event = TRUE;
    switch(eclass) {
    case kEventClassMenu: {
	MenuRef menu;
	GetEventParameter(event, kEventParamDirectObject, typeMenuRef, NULL,
			  sizeof(menu), NULL, &menu);
	int mid = GetMenuID(menu);
	if(MacPrivate::PopupBinding *mpb = activeMenuBar->mac_d->popups->find(mid)) {
	    short idx;
	    GetEventParameter(event, kEventParamMenuItemIndex, typeMenuItemIndex, NULL,
			      sizeof(idx), NULL, &idx);
	    MenuCommand cmd;
	    GetMenuItemCommandID(mpb->macpopup, idx, &cmd);
	    QMenuItem *it = mpb->qpopup->findItem(cmd);
	    if(ekind == kEventMenuMeasureItemHeight) {
		short h = it->custom()->sizeHint().height();
		SetEventParameter(event, kEventParamMenuItemHeight, typeShortInteger,
				  sizeof(h), &h);
	    } else if(ekind == kEventMenuMeasureItemWidth) {
		short w = it->custom()->sizeHint().width();
		SetEventParameter(event, kEventParamMenuItemWidth, typeShortInteger,
				  sizeof(w), &w);
	    } else if(ekind == kEventMenuDrawItemContent) {
		handled_event = FALSE;
	    } else {
		CallNextEventHandler(er, event);
		Rect r;
		GetEventParameter(event, kEventParamMenuTextBounds, typeQDRectangle, NULL,
				  sizeof(r), NULL, &r);
		QMacSavedPortInfo fi;
		::RGBColor f;
		f.red = 256*256;
		f.blue = f.green = 0;
		RGBForeColor( &f );
		PaintRect( &r );
		handled_event = FALSE;
	    }
	} else {
	    handled_event = FALSE;
	}
	break; }
    default:
	handled_event = FALSE;
	break;
    }
    if(!handled_event) //let the event go through
	return CallNextEventHandler(er, event);
    return noErr; //we eat the event
}
static EventHandlerRef mac_menubarEventHandler = NULL;
static EventHandlerUPP mac_menubarEventUPP = NULL;
static void qt_mac_clean_menubar_event()
{
    if(mac_menubarEventHandler) {
	RemoveEventHandler(mac_menubarEventHandler);
	mac_menubarEventHandler = NULL;
    }
    if(mac_menubarEventUPP) {
	DisposeEventHandlerUPP(mac_menubarEventUPP);
	mac_menubarEventUPP = NULL;
    }
}
void QMenuBar::qt_mac_install_menubar_event(MenuRef ref)
{
    if(mac_menubarEventHandler)
	return;
    static EventTypeSpec menu_events[] = {
	{ kEventClassMenu, kEventMenuMeasureItemWidth },
	{ kEventClassMenu, kEventMenuMeasureItemHeight },
	{ kEventClassMenu, kEventMenuDrawItemContent }
    };
    if(!mac_menubarEventUPP) 
	mac_menubarEventUPP = NewEventHandlerUPP(qt_mac_menubar_event);
    InstallMenuEventHandler(ref, mac_menubarEventUPP, 
			    GetEventTypeCount(menu_events), menu_events, NULL, 
			    &mac_menubarEventHandler);
    qAddPostRoutine( qt_mac_clean_menubar_event );
}
#endif

/* utility functions */
static const CFStringRef no_ampersands(QString i) {
    for(int w = 0; (w=i.find('&', w)) != -1; )
	i.remove(w, 1);
    return CFStringCreateWithCharacters(NULL, (UniChar *)i.unicode(), i.length());
}

#if !defined(QMAC_QMENUBAR_NO_MERGE)
uint QMenuBar::isCommand(QMenuItem *it) 
{
    if(it->popup() || it->custom() || it->isSeparator())
	return 0;

    QString t = it->text().lower();
    for(int w = 0; (w=t.find('&', w)) != -1; )
	t.remove(w, 1);
    int st = t.findRev('\t');
    if(st != -1) 
	t.remove(st, t.length()-st);
    t.replace(QRegExp("\\.*$"), ""); //no ellipses
    //now the fun part
    uint ret = 0;
    if(t.find("about", 0, FALSE) == 0) {
	if(t.find(QRegExp("qt$", FALSE)) == -1)
	    ret = kHICommandAbout;
	else 
	    ret = 'CUTE';
    }
    if(t.find("config", 0, FALSE) == 0 || t.find("preference", 0, FALSE) == 0 || 
       t.find("options", 0, FALSE) == 0 || t.find("setting", 0, FALSE) == 0) 
	ret = kHICommandPreferences;
    if(t.find("quit", 0, FALSE) == 0 || t.find("exit", 0, FALSE) == 0) 
	ret = kHICommandQuit;
    //shall we?
    if(ret && activeMenuBar && (!activeMenuBar->mac_d->commands || 
				!activeMenuBar->mac_d->commands->find(ret))) {
	if(ret == kHICommandAbout || ret == 'CUTE') {
	    if(activeMenuBar->mac_d->apple_menu) {
		QString text = it->text();
		for(int w = 0; (w=text.find('&', w)) != -1; )
		    text.remove(w, 1);
		int st = text.findRev('\t');
		if(st != -1) 
		    text.remove(st, text.length()-st);
		if(ret == kHICommandAbout || text == "About") 
		    text += QString(" ") + qApp->argv()[0];
		InsertMenuItemTextWithCFString(activeMenuBar->mac_d->apple_menu, 
					       no_ampersands(text), 
					       activeMenuBar->mac_d->in_apple++, 0, ret);
	    }
	} 
	EnableMenuCommand(NULL, ret);
    } else {
	ret = 0;
    }
    return ret;
}
#endif

bool QMenuBar::syncPopups(MenuRef ret, QPopupMenu *d)
{
    if(d) {
	SetMenuExcludesMarkColumn(ret, !d->isCheckable());
	for(int id = 1, x = 0; x < (int)d->count(); x++) {
#if !defined(QMAC_QMENUBAR_NO_MERGE)
	    if(activeMenuBar->mac_d->commands) {
		bool found = FALSE;
		QIntDictIterator<QMenuBar::MacPrivate::CommandBinding> it(*(activeMenuBar->mac_d->commands));
		for( ; it.current() && !(found = (it.current()->index == x && it.current()->qpopup == d)); ++it);
		if(found)
		    continue;
	    }
#endif
	    QMenuItem *item = d->findItem(d->idAt(x));
	
#if defined(QMAC_QMENUBAR_NO_EVENT)
	    if(item->custom()) 
		continue;
#endif
	    if(item->widget()) 
		continue;

	    QString text = "empty", accel; //Yes I need this, stupid!
	    if(!item->isSeparator()) {
#if !defined(QMAC_QMENUBAR_NO_MERGE)
		if(int cmd = isCommand(item)) {
		    if(!activeMenuBar->mac_d->commands) {
			activeMenuBar->mac_d->commands = new QIntDict<QMenuBar::MacPrivate::CommandBinding>();
			activeMenuBar->mac_d->commands->setAutoDelete(TRUE);
		    }
		    activeMenuBar->mac_d->commands->insert(cmd, 
							   new QMenuBar::MacPrivate::CommandBinding(d, x));
		    continue;
		}
#endif
		text = item->text();
		int st = text.findRev('\t');
		if(st != -1) {
		    accel = text.right(text.length()-(st+1));
		    text.remove(st, text.length()-st);
		}
	    } 
#if !defined(QMAC_QMENUBAR_NO_MERGE)
	    else if(x != (int)d->count()-1 &&
		    ((x == (int)d->count() - 2 || d->findItem(d->idAt(x+2))->isSeparator()) &&
		     isCommand(d->findItem(d->idAt(x+1)))))
		continue;
#endif

	    InsertMenuItemTextWithCFString(ret, no_ampersands(text), id, 0, item->id());
	    if(item->isSeparator()) {
		ChangeMenuItemAttributes(ret, id, kMenuItemAttrSeparator, 0);
	    } else {
		if(item->pixmap()) { 		    //handle pixmaps..
#if 0
		    CIcon *ic = (CIcon *)malloc(sizeof(CIcon));
		    PixMapHandle src = GetGWorldPixMap((GWorldPtr)item->pixmap()->handle());
		    PixMap *dst = &ic->iconPMap;
		    CopyPixMap(src, &dst);
		    SetMenuItemIconHandle(ret, id, kMenuIconRefType, (Handle)ic);
#endif
		}
#if !defined(QMAC_QMENUBAR_NO_EVENT)
		if(item->custom()) {
		    qt_mac_install_menubar_event(ret);
		    ChangeMenuItemAttributes(ret, id, kMenuItemAttrCustomDraw, 0);
		}
#endif
		if(item->isEnabled())
		    EnableMenuItem(ret, id);
		else
		    DisableMenuItem(ret, id);
		CheckMenuItem(ret, id, item->isChecked() ? true : false);
		if(item->popup()) {
		    SetMenuItemHierarchicalMenu(ret, id, createMacPopup(item->popup(), FALSE));
		} else {
		    int k = item->key();
		    if(k == Qt::Key_unknown && !accel.isEmpty()) 
			k = QAccel::stringToKey(accel);

		    if( k != Qt::Key_unknown ) {
			char mod = 0;
			if ( (k & Qt::CTRL) != Qt::CTRL ) 
			    mod |= kMenuNoCommandModifier;
			if ( (k & Qt::ALT) == Qt::ALT ) 
			    mod |= kMenuOptionModifier;
			if ( (k & Qt::SHIFT) == Qt::SHIFT ) 
			    mod |= kMenuShiftModifier;
			char keycode = (char) (k & (~(Qt::SHIFT | Qt::CTRL | Qt::ALT)));
			if(keycode & 0xFF) {
			    SetMenuItemModifiers(ret, id, mod);
			    SetItemCmd(ret, id, keycode );
			}
		    }
		}
	    }
	    id++;
 	}
    }
    return TRUE;
}

MenuRef QMenuBar::createMacPopup(QPopupMenu *d, bool do_sync, bool top_level) 
{
    static int mid = 0;
    MenuRef ret;
    if(CreateNewMenu(0, 0, &ret) != noErr)
	return NULL;

    if(!activeMenuBar->mac_d->popups) {
	activeMenuBar->mac_d->popups = new QIntDict<QMenuBar::MacPrivate::PopupBinding>();
	activeMenuBar->mac_d->popups->setAutoDelete(TRUE);
    }
    SetMenuID(ret, ++mid);
    activeMenuBar->mac_d->popups->insert((int)mid, 
					 new QMenuBar::MacPrivate::PopupBinding(d, ret, 
										top_level));
    if(1 || do_sync)
	syncPopups(ret, d);
    return ret;
}
	
bool QMenuBar::updateMenuBar() 
{
    if(this != activeMenuBar) 
	qDebug("Shouldn't have happened! %s:%d", __FILE__, __LINE__);
    ClearMenuBar();
    if(mac_d)
	mac_d->clear();
    if(!CreateNewMenu(0, 0, &mac_d->apple_menu)) {
	SetMenuTitleWithCFString(mac_d->apple_menu, 
				 no_ampersands(QString(QChar(0x14))));
	InsertMenu(mac_d->apple_menu, 0);
    }

    for(int x = 0; x < (int)count(); x++) {
	QMenuItem *item = findItem(idAt(x));
	if(item->isSeparator()) //mac doesn't support these
	    continue;
	MenuRef mp = createMacPopup(item->popup(), FALSE, TRUE);
	SetMenuTitleWithCFString(mp, no_ampersands(item->text()));
	InsertMenu(mp, 0);
    }
    return TRUE;
}

/* qmenubar functions */

/*!
  Internal function..
*/
bool QMenuBar::activateCommand(uint cmd)
{
#if !defined(QMAC_QMENUBAR_NO_MERGE)
    if(activeMenuBar && activeMenuBar->mac_d->commands) {
	if(MacPrivate::CommandBinding *mcb = activeMenuBar->mac_d->commands->find(cmd)) {
	    mcb->qpopup->activateItemAt(mcb->index);
	    HiliteMenu(0);
	    return TRUE;
	}
    }
#endif
    if(cmd == kHICommandQuit) {
	qApp->closeAllWindows();
	HiliteMenu(0);
	return TRUE;
    } else if(cmd == kHICommandAbout) {
	QMessageBox::aboutQt(NULL);
	HiliteMenu(0);
	return TRUE;
    } 
    HiliteMenu(0);
    return FALSE;
}

/*!
  Internal function..
*/
bool QMenuBar::activate(MenuRef menu, short idx, bool highlight)
{
    if(!activeMenuBar) {
	HiliteMenu(0);
	return FALSE;
    }

    int mid = GetMenuID(menu);
    if(MacPrivate::PopupBinding *mpb = activeMenuBar->mac_d->popups->find(mid)) {
	MenuCommand cmd;
	GetMenuItemCommandID(mpb->macpopup, idx, &cmd);
	if(highlight) {
	    if(mpb->qpopup->isItemEnabled(cmd)) 
		mpb->qpopup->hilitSig(cmd);
	} else {
	    mpb->qpopup->activateItemAt(mpb->qpopup->indexOf(cmd));
	    HiliteMenu(0);
	}
	return TRUE;
    }
    HiliteMenu(0);
    return FALSE;
}


/*!
  Internal function that cleans up the menubar.
*/
static QIntDict<QMenuBar> *menubars = NULL;
void QMenuBar::macCreateNativeMenubar() 
{
    macDirtyNativeMenubar();
    QWidget *p = parentWidget();
    if(p && (!menubars || !menubars->find((int)topLevelWidget())) && 
       ((p->isDialog() && p->isTopLevel()) || p->inherits("QToolBar") ||
	topLevelWidget() == qApp->mainWidget() || !qApp->mainWidget())) {
	mac_eaten_menubar = 1;
	if(!menubars)
	    menubars = new QIntDict<QMenuBar>();
	menubars->insert((int)topLevelWidget(), this);
	if(!mac_d)
	    mac_d = new MacPrivate;
    } else {
	mac_eaten_menubar = 0;
    }
}
void QMenuBar::macRemoveNativeMenubar()
{
    if(mac_eaten_menubar && menubars) {
	QMenuBar *mb = menubars->find((int)topLevelWidget());
	if(mb == this) 
	    menubars->remove((int)topLevelWidget());
    }
    mac_eaten_menubar = FALSE;
    if(activeMenuBar == this) {
	activeMenuBar = NULL;
	ClearMenuBar();
	InvalMenuBar();
    }
    if(mac_d) {
	delete mac_d;
	mac_d = NULL;
    }
}
void QMenuBar::macDirtyNativeMenubar()
{
    if(mac_eaten_menubar && mac_d) {
	mac_d->dirty = 1;
	qt_event_request_menubarupdate();
    }
}

void QMenuBar::initialize()
{
}

void QMenuBar::cleanup()
{
    delete menubars;
    menubars = NULL;
}

void QMenuBar::macUpdateMenuBar()
{
    if(!menubars) 
	return;
    QWidget *w = qApp->activeWindow();
    if(!w) {
	WindowClass c;
	for(WindowPtr wp = FrontWindow(); wp; wp = GetNextWindow(wp)) {
	    if(GetWindowClass(wp, &c))
		break;
	    if(c == kOverlayWindowClass) 
		continue;
	    w = QWidget::find((WId)wp);
	    break;
	}
    }
    if(!w) //last ditch effort
	w = qApp->mainWidget();
    static bool first = TRUE;
    if(w) {
	QMenuBar *mb = menubars->find((int)w);
	while(w && !mb && !w->testWFlags(WShowModal)) 
	    mb = menubars->find((int)(w = w->parentWidget()));
  	if(mb) {
	    if(!mb->mac_eaten_menubar || (!first && !mb->mac_d->dirty && (mb == activeMenuBar)))
		return;
	    activeMenuBar = mb;
	    first = FALSE;
	    if(mb->mac_d->dirty || !mb->mac_d->mac_menubar) {
		mb->mac_d->dirty = 0;
		mb->updateMenuBar();
		mb->mac_d->mac_menubar = GetMenuBar();
	    } else {
		SetMenuBar(mb->mac_d->mac_menubar);
		InvalMenuBar();
	    }
	} else if (!first) {
	    first = TRUE;
	    if(!w || (!w->testWFlags(WStyle_Tool) && !w->testWFlags(WType_Popup)) ) {
		ClearMenuBar();

		InvalMenuBar();
	    }
	} 
    } 
}

void QMenuBar::macUpdatePopup(MenuRef mr)
{
    if(!mr || !activeMenuBar)
	return;

    int mid = GetMenuID( mr );
    if(MacPrivate::PopupBinding *mpb = activeMenuBar->mac_d->popups->find(mid)) {
	if(mpb->qpopup) {
	    emit mpb->qpopup->aboutToShow();
	    if(1 || mpb->qpopup->mac_dirty_popup) {
		mpb->qpopup->mac_dirty_popup = 0;
		DeleteMenuItems(mr, 1, CountMenuItems(mr));
		activeMenuBar->syncPopups(mr, mpb->qpopup);
	    }
	}
    } 
}

#endif //WS_MAC
