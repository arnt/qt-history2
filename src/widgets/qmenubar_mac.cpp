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

void qt_event_request_menubarupdate(); //qapplication_mac.cpp

class QMenuBar::MacPrivate {
public:
    MacPrivate() : commands(NULL), popups(NULL), mac_menubar(NULL), dirty(1) { }
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
    uint dirty;

    void clear() {
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

/* utility functions */
static const CFStringRef no_ampersands(QString i) {
    for(int w = 0; (w=i.find('&', w)) != -1; )
	i.remove(w, 1);
    return CFStringCreateWithCharacters(NULL, (UniChar *)i.unicode(), i.length());
}

#if 0
QMAC_PASCAL void macMenuItemProc(SInt16 msg, MenuRef mr, Rect *menuRect, Point pt, SInt16 *idx)
{
    qDebug("foo.. %d", msg);
    switch(msg) {
    case kMenuInvalMsg:
    case kMenuSizeMsg:
    case kMenuPopUpMsg:
    case kMenuCalcItemMsg:
    case kMenuThemeSavvyMsg:
	*(idx) = kThemeSavvyMenuResponse;
    }

    if(pdict) {
	short id = (short)mr;
	if(QMenuBar::MacPrivate::PopupBinding *mpb = pdict->find((int)id)) {
	    QPopupMenu *qp = mpb->qpopup;
	    switch(msg) {
	    case kMenuSizeMsg:
	    {
		int width=0, height=0;
		QMenuItem *item;
		for(int x = 0; x < (int)qp->count(); x++) {
		    short w, h;
		    item = qp->findItem(qp->idAt(x));
		    if(item->custom()) {
			QSize sz = item->custom()->sizeHint();
			w = sz.width();
			h = sz.height();
		    } else if(item->isSeparator()) {
			w = 0;
			GetThemeMenuSeparatorHeight(&h);
		    } else {
			GetThemeMenuItemExtra(item->popup() ? kThemeMenuItemHierarchical : kThemeMenuItemPlain,
					      &h, &w);
			w = 100;
			h = 100;
		    }
		    if(w > width)
			width = w;
		    height += h;
		}
		qDebug("For size msg I got %d %d", width, height);
		SetRect(menuRect, 0, 0, width, height);
	    }
	    break;
#if 0
	    QMenuItem *item = mpb->qpopup->findItem((int)*(idx));
	    if(item && item->custom()) {
		qDebug("blah..");
		QCustomMenuItem *cst = item->custom();
		switch(msg) {
		case kMenuHiliteItemMsg:
		case kMenuDrawItemsMsg:
		    break;
		case mCalcItemMsg:
		    break;
		}
	    }
#endif
	    }
	}
    }
}
#endif

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
#if 0
    if(t.find("about", 0, FALSE) == 0 && t.find(QRegExp("qt$", FALSE)) == -1) 
	ret = kHICommandAbout;
#endif
    if(t.find("config", 0, FALSE) == 0 || t.find("preference", 0, FALSE) == 0 || 
       t.find("options", 0, FALSE) == 0 || t.find("setting", 0, FALSE) == 0) 
	ret = kHICommandPreferences;
    if(t.find("quit", 0, FALSE) == 0 || t.find("exit", 0, FALSE) == 0) 
	ret = kHICommandQuit;
    //shall we?
    if(ret && activeMenuBar && (!activeMenuBar->mac_d->commands || 
				!activeMenuBar->mac_d->commands->find(ret))) {
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
	
	    if(item->custom()) {
		//qDebug("Ooops, don't think I can handle that yet! %s:%d %d", __FILE__, __LINE__, x);
		continue;
	    }
	    if(item->widget()) {
		//qDebug("Ooops, don't think I can handle that yet! %s:%d %d", __FILE__, __LINE__, x);
		continue;
	    }

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
			if(toupper(keycode) >= 'A' && toupper(keycode) <= 'Z') {
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
#if 0
    MenuDefSpec spec;
    spec.defType = kMenuDefProcPtr;
    spec.u.defProc = NewMenuDefUPP(macMenuItemProc);
    SetMenuDefinition(ret, &spec);
#endif

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
    if(MenuRef r = GetMenuHandle(0)) {
	qDebug("doing it..");
	InsertMenuItemTextWithCFString(r, no_ampersands("About"), 0, 0, kHICommandAbout);
	DisableMenuItem(r, kHICommandAbout);
    }
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
