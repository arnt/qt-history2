/****************************************************************************
** $Id$
**
** Implementation of QMenuBar class for mac
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Macintosh may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qplatformdefs.h"
#if defined(Q_WS_MAC) && !defined(QMAC_QMENUBAR_NO_NATIVE)
#include "qt_mac.h"

#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qintdict.h>
#include <qstring.h>
#include <qapplication.h>
#include <qaccel.h>
#include <qregexp.h>
#include <qguardedptr.h>
#include <qmessagebox.h>
#include <qdockwindow.h>
#include <qdockarea.h>

#include <ctype.h>

/*****************************************************************************
  QMenubar debug facilities
 *****************************************************************************/
//#define DEBUG_MENUBAR_ACTIVATE

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
static QGuardedPtr<QMenuBar> activeMenuBar; //The current global menubar

#if !defined(QMAC_QMENUBAR_NO_EVENT)
//event callbacks
QMAC_PASCAL OSStatus
QMenuBar::qt_mac_menubar_event(EventHandlerCallRef er, EventRef event, void *)
{
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    bool handled_event = TRUE;
    switch(eclass) {
    case kEventClassMenu: {
	qDebug("happened %d", ekind);
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
	    if(!it->custom()) {
		handled_event = FALSE;
		break;
	    }
	    qDebug("made it here..");

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
void no_ampersands(QString i, CFStringRef *ret) {
    for(int w = 0; (w=i.find('&', w)) != -1; )
	i.remove(w, 1);
    *ret = CFStringCreateWithCharacters(NULL, (UniChar *)i.unicode(), i.length());
}

#if !defined(QMAC_QMENUBAR_NO_MERGE)
uint QMenuBar::isCommand(QMenuItem *it, bool just_check)
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
    if(t.find(tr("about"), 0, FALSE) == 0) {
	if(t.find(QRegExp("qt$", FALSE)) == -1)
	    ret = kHICommandAbout;
	else
	    ret = 'CUTE';
    } else if(t.find(tr("config"), 0, FALSE) == 0 || t.find(tr("preference"), 0, FALSE) == 0 ||
	      t.find(tr("options"), 0, FALSE) == 0 || t.find(tr("setting"), 0, FALSE) == 0 ||
	      t.find(tr("setup"), 0, FALSE) == 0 ) {
	ret = kHICommandPreferences;
    } else if(t.find(tr("quit"), 0, FALSE) == 0 || t.find(tr("exit"), 0, FALSE) == 0) {
	ret = kHICommandQuit;
    }
    //shall we?
    if(just_check) {
	//do nothing, we already checked
    } else if(ret && activeMenuBar &&
	      (!activeMenuBar->mac_d->commands || !activeMenuBar->mac_d->commands->find(ret))) {
	if(ret == kHICommandAbout || ret == 'CUTE') {
	    if(activeMenuBar->mac_d->apple_menu) {
		QString text = it->text();
		for(int w = 0; (w=text.find('&', w)) != -1; )
		    text.remove(w, 1);
		int st = text.findRev('\t');
		if(st != -1)
		    text.remove(st, text.length()-st);
		text.replace(QRegExp("\\.*$"), ""); //no ellipses
#ifdef Q_WS_MACX
		if(ret == kHICommandAbout && text.lower() == tr("about")) {
		    QString prog = qApp->argv()[0];
		    text += " " + prog.section('/', -1, -1);;
		}
#endif
		CFStringRef cfref;
		no_ampersands(text, &cfref);
		InsertMenuItemTextWithCFString(activeMenuBar->mac_d->apple_menu,
					       cfref, activeMenuBar->mac_d->in_apple++,
					       kMenuItemAttrAutoRepeat, ret);
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
		     isCommand(d->findItem(d->idAt(x+1)), TRUE)))
		continue;
#endif

	    MenuItemAttributes attr = kMenuItemAttrAutoRepeat;
#if !defined(QMAC_QMENUBAR_NO_EVENT)
	    if(item->custom())
		attr |= kMenuItemAttrCustomDraw;
#endif
	    
	    //figure out an accelerator
	    int accel_key = Qt::Key_unknown;
	    if(accel.isEmpty() && item->key().count() > 1)
		text += " (****)"; //just to denote a multi stroke accelerator
	    if(item->key().count() == 1 && (int)item->key() != accel_key)
		accel_key = (int)item->key();
	    else if(!accel.isEmpty())
		accel_key = QAccel::stringToKey(accel);

	    CFStringRef cfref;
	    no_ampersands(text, &cfref);
	    InsertMenuItemTextWithCFString(ret, cfref, id,  attr, item->id());
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
		} else if(accel_key != Qt::Key_unknown) {
		    char mod = 0;
		    if ((accel_key & Qt::CTRL) != Qt::CTRL)
			mod |= kMenuNoCommandModifier;
		    if ((accel_key & Qt::META) == Qt::META)
			mod |= kMenuControlModifier;
		    if ((accel_key & Qt::ALT) == Qt::ALT)
			mod |= kMenuOptionModifier;
		    if ((accel_key & Qt::SHIFT) == Qt::SHIFT)
			mod |= kMenuShiftModifier;
		    int keycode = (accel_key & ~(Qt::MODIFIER_MASK | Qt::UNICODE_ACCEL));
		    if(keycode) {
			SetMenuItemModifiers(ret, id, mod);
			bool do_glyph = TRUE;
			if(keycode == Qt::Key_Return)
			    keycode = kMenuReturnGlyph;
			else if(keycode == Qt::Key_Enter)
			    keycode = kMenuEnterGlyph;
			else if(keycode == Qt::Key_Tab)
			    keycode = kMenuTabRightGlyph;
			else if(keycode == Qt::Key_Backspace)
			    keycode = kMenuDeleteLeftGlyph;
			else if(keycode == Qt::Key_Delete)
			    keycode = kMenuDeleteRightGlyph;
			else if(keycode == Qt::Key_Escape)
			    keycode = kMenuEscapeGlyph;
			else if(keycode == Qt::Key_PageUp)
			    keycode = kMenuPageUpGlyph;
			else if(keycode == Qt::Key_PageDown)
			    keycode = kMenuPageDownGlyph;
			else if(keycode == Qt::Key_Up)
			    keycode = kMenuUpArrowGlyph;
			else if(keycode == Qt::Key_Down)
			    keycode = kMenuDownArrowGlyph;
			else if(keycode == Qt::Key_Left)
			    keycode = kMenuLeftArrowGlyph;
			else if(keycode == Qt::Key_Right)
			    keycode = kMenuRightArrowGlyph;
			else if(keycode == Qt::Key_CapsLock)
			    keycode = kMenuCapsLockGlyph;
			else if(keycode >= Qt::Key_F1 && keycode <= Qt::Key_F15)
			    keycode = (keycode - Qt::Key_F1) + kMenuF1Glyph;
			else {
			    do_glyph = FALSE;
			    if(keycode < 127) { //regular ascii accel
				SetItemCmd(ret, id, (CharParameter)keycode );
			    } else { //I guess I missed one, fix above if this happens
				QKeySequence key(keycode);
				qDebug("QMenuBar: Not sure how to handle accelerator 0x%04x (%s)",
				       keycode, ((QString)key).latin1());
			    }
			}
			if(do_glyph) //"special" accelerator..
			    SetMenuItemKeyGlyph(ret, id, (SInt16)keycode);
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
    MenuAttributes attr = 0;
#if 0
    attr |= kMenuAttrAutoDisable;
#endif
#if !defined(QMAC_QMENUBAR_NO_EVENT)
    attr |= kMenuItemAttrCustomDraw;
#endif
    MenuRef ret;
    if(CreateNewMenu(0, attr, &ret) != noErr)
	return NULL;

    if(!activeMenuBar->mac_d->popups) {
	activeMenuBar->mac_d->popups = new QIntDict<QMenuBar::MacPrivate::PopupBinding>();
	activeMenuBar->mac_d->popups->setAutoDelete(TRUE);
    }
    SetMenuID(ret, ++mid);
#if !defined(QMAC_QMENUBAR_NO_EVENT)
    qt_mac_install_menubar_event(ret);
#endif
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
	CFStringRef cfref;
	no_ampersands(QString(QChar(0x14)), &cfref);
	SetMenuTitleWithCFString(mac_d->apple_menu, cfref);
	InsertMenu(mac_d->apple_menu, 0);
    }

    for(int x = 0; x < (int)count(); x++) {
	QMenuItem *item = findItem(idAt(x));
	if(item->isSeparator()) //mac doesn't support these
	    continue;
	MenuRef mp = createMacPopup(item->popup(), FALSE, TRUE);
	CFStringRef cfref;
	no_ampersands(item->text(), &cfref);
	SetMenuTitleWithCFString(mp, cfref);
	InsertMenu(mp, 0);
    }
    return TRUE;
}

/* qmenubar functions */

/*!
    \internal
*/
bool QMenuBar::activateCommand(uint cmd)
{
#if !defined(QMAC_QMENUBAR_NO_MERGE)
    if(activeMenuBar && activeMenuBar->mac_d->commands) {
	if(MacPrivate::CommandBinding *mcb = activeMenuBar->mac_d->commands->find(cmd)) {
#ifdef DEBUG_MENUBAR_ACTIVATE
	    qDebug("ActivateCommand: activating internal merging '%s'",
		   mcb->qpopup->text(mcb->qpopup->idAt(mcb->index)).latin1());
#endif
	    mcb->qpopup->activateItemAt(mcb->index);
	    HiliteMenu(0);
	    return TRUE;
	}
    }
#endif
    HiliteMenu(0);
    return FALSE;
}

/*!
    \internal
*/
bool QMenuBar::activate(MenuRef menu, short idx, bool highlight, bool by_accel)
{
    if(!activeMenuBar) {
	HiliteMenu(0);
	return FALSE;
    }

    int mid = GetMenuID(menu);
    if(MacPrivate::PopupBinding *mpb = activeMenuBar->mac_d->popups->find(mid)) {
	MenuCommand cmd;
	GetMenuItemCommandID(mpb->macpopup, idx, &cmd);
	if(by_accel) {
	    int key = mpb->qpopup->findItem(cmd)->key();
	    if(key == Qt::Key_unknown) {
#ifdef DEBUG_MENUBAR_ACTIVATE
		qDebug("ActivateMenuitem: ignored due to fake accelerator '%s' %d",
		       mpb->qpopup->text(cmd).latin1(), highlight);
#endif
		return FALSE;
	    }
	}
#ifdef DEBUG_MENUBAR_ACTIVATE
	qDebug("ActivateMenuitem: activating internal menubar binding '%s' %d",
	       mpb->qpopup->text(cmd).latin1(), highlight);
#endif
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


static QIntDict<QMenuBar> *menubars = NULL;
/*!
  \internal
  Internal function that cleans up the menubar.
*/
void QMenuBar::macCreateNativeMenubar()
{
    macDirtyNativeMenubar();
    QWidget *p = parentWidget();
    if(p && (!menubars || !menubars->find((int)topLevelWidget())) &&
       (((p->isDialog() || p->inherits("QMainWindow")) && p->isTopLevel()) || 
	p->inherits("QToolBar") || 
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
    if(this == activeMenuBar) {
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

/*!
    \internal
*/
void QMenuBar::initialize()
{
}

/*!
    \internal
*/
void QMenuBar::cleanup()
{
    delete menubars;
    menubars = NULL;
}

void QMenuBar::macUpdateMenuBar()
{
    static bool first = TRUE;
    if(!menubars) {
	if(first) {
	    first = FALSE;
	    ClearMenuBar();
	    InvalMenuBar();
	}
	return;
    }
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
    if(w) {
	QMenuBar *mb = menubars->find((int)w);
	if(!mb && (!w->parentWidget() || w->parentWidget()->isDesktop()) && w->inherits("QDockWindow")) {
	    if(QWidget *area = ((QDockWindow*)w)->area()) {
		QWidget *areaTL = area->topLevelWidget();
		if((mb = menubars->find((int)areaTL))) 
		    w = areaTL;
	    }
	}
	while(w && !w->testWFlags(WShowModal) && !mb) 
	    mb = menubars->find((int)(w = w->parentWidget()));
  	if(mb) {
	    if(!mb->mac_eaten_menubar || (!first && !mb->mac_d->dirty && (mb == activeMenuBar))) 
		return;
	    first = FALSE;
	    activeMenuBar = mb;
	    if(mb->mac_d->dirty || !mb->mac_d->mac_menubar) {
		mb->mac_d->dirty = 0;
		mb->updateMenuBar();
		mb->mac_d->mac_menubar = GetMenuBar();
	    } else {
		SetMenuBar(mb->mac_d->mac_menubar);
		InvalMenuBar();
	    }
	} else {
	    if(first || !w || 
		(!w->testWFlags(WStyle_Tool) && !w->testWFlags(WType_Popup))) {
	    	first = FALSE;
		activeMenuBar = NULL;
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

void QMenuBar::macWidgetChangedWindow()
{
    int was_eaten = mac_eaten_menubar;
    macRemoveNativeMenubar();
    macCreateNativeMenubar();
    if(was_eaten)
	menuContentsChanged();
}

#endif //WS_MAC
