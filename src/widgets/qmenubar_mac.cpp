#include <qglobal.h>
#if defined(Q_WS_MAC) && !defined(QMAC_QMENUBAR_NO_NATIVE)

#include <ctype.h>
#include "qt_mac.h"

#define INCLUDE_MENUITEM_DEF
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qintdict.h>
#include <qstring.h>
#include <qapplication.h>
#include <qaccel.h>

static MenuRef createPopup(QPopupMenu *d, bool);
static bool syncPopup(MenuRef ret, QPopupMenu *d);

/* utility functions */
class MacPopupBinding {
public:
    MacPopupBinding(QPopupMenu *m, MenuRef r) : qpopup(m), macpopup(r) { }
    ~MacPopupBinding() { DisposeMenu(macpopup); }

    QPopupMenu *qpopup;
    MenuRef macpopup;
};
static QIntDict<MacPopupBinding> *pdict = NULL;

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
    case kMenuDrawMsg:
    case kMenuSizeMsg:
    case kMenuPopUpMsg:
    case kMenuCalcItemMsg:
    case kMenuThemeSavvyMsg:
	*(idx) = kThemeSavvyMenuResponse;
    }

    if(pdict) {
	short id = (short)mr;
	if(MacPopupBinding *mpb = pdict->find((int)id)) {
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

static bool syncPopup(MenuRef ret, QPopupMenu *d)
{
    if(d) {
	for(int id = 1, x = 0; x < (int)d->count(); x++) {
	    QMenuItem *item = d->findItem(d->idAt(x));
	
	    if(item->custom()) {
		//FIXME
		//qDebug("Ooops, don't think I can handle that yet! %s:%d %d", __FILE__, __LINE__, x);
		continue;
	    }
	    if(item->widget()) {
		qDebug("Ooops, don't think I can handle that yet! %s:%d %d", __FILE__, __LINE__, x);
		continue;
	    }

	    QString text = "empty", accel; //Yes I need this, stupid!
	    if(!item->isSeparator()) {
		text = item->text();
		int st = text.findRev('\t');
		if(st != -1) {
		    accel = text.right(text.length()-(st+1));
		    text.remove(st, text.length()-st);
		}
	    }

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
		    SetMenuItemHierarchicalMenu(ret, id, createPopup(item->popup(), FALSE));
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

static MenuRef createPopup(QPopupMenu *d, bool do_sync) 
{
    MenuRef ret;
    if(CreateNewMenu(0, 0, &ret) != noErr)
	return NULL;

    if(!pdict) {
	pdict = new QIntDict<MacPopupBinding>();
	pdict->setAutoDelete(TRUE);
    }
    short mid = (short)ret;
    SetMenuID(ret, mid);
    pdict->insert((int)mid, new MacPopupBinding(d, ret));

#if 0
    MenuDefSpec spec;
    spec.defType = kMenuDefProcPtr;
    spec.u.defProc = NewMenuDefUPP(macMenuItemProc);
    SetMenuDefinition(ret, &spec);
#endif

    if(1 || do_sync)
	syncPopup(ret, d);
    return ret;
}
	
static bool updateMenuBar(QMenuBar *mbar) 
{
    ClearMenuBar();
    InvalMenuBar();
    if(pdict)
	pdict->clear();
    for(int x = 0; x < (int)mbar->count(); x++) {
	QMenuItem *item = mbar->findItem(mbar->idAt(x));
	if(item->isSeparator()) //mac doesn't support these
	    continue;

	MenuRef mp = createPopup(item->popup(), FALSE);
	SetMenuTitleWithCFString(mp, no_ampersands(item->text()));
	InsertMenu(mp, x);
    }
    InvalMenuBar();
    return TRUE;
}

/* qmenubar functions */

/*!
  Internal function..
*/
bool QMenuBar::activate(MenuRef menu, short idx, bool highlight)
{
    if(!pdict) {
	HiliteMenu(0);
	return FALSE;
    }

    if(MacPopupBinding *mpb = pdict->find((int)((short)menu))) {
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
    mac_dirty_menubar = 1;
    QWidget *p = parentWidget();
    if(p && (!menubars || !menubars->find((int)topLevelWidget())) && 
       ((p->inherits("QMainWindow") && !p->parentWidget()) || p->inherits("QToolBar"))) {
	mac_eaten_menubar = 1;
	if(!menubars)
	    menubars = new QIntDict<QMenuBar>();
	menubars->insert((int)topLevelWidget(), this);
    } else {
	mac_eaten_menubar = 0;
    }
}
void QMenuBar::macRemoveNativeMenubar()
{
    if(menubars)
	menubars->remove((int)topLevelWidget());
}

void QMenuBar::cleanup()
{
    ClearMenuBar();
    InvalMenuBar();
    delete pdict;
    pdict = NULL;
    delete menubars;
    menubars = NULL;
}

void QMenuBar::macUpdateMenuBar()
{
    if(!menubars) 
	return;

    static bool first = TRUE;
    if(QWidget *w = qApp->activeWindow()) {
	if(QMenuBar *mb = menubars->find((int)w)) {
	    if(!mb->mac_eaten_menubar || (!first && !mb->mac_dirty_menubar))
		return;
	    first = FALSE;
	    mb->mac_dirty_menubar = 0;
	    updateMenuBar(mb);
	} else if (!first) {
	    first = TRUE;
	    if(!w->testWFlags(WType_Dialog) && !w->testWFlags(WType_Popup) )
		ClearMenuBar();
	} 
    } 
}

void QMenuBar::macUpdatePopup(MenuRef mr)
{
    if(!mr || !pdict)
	return;

    short id = (short)mr;
    if(MacPopupBinding *mpb = pdict->find((int)id)) {
	emit mpb->qpopup->aboutToShow();
	if(1 || mpb->qpopup->mac_dirty_popup) {
	    mpb->qpopup->mac_dirty_popup = 0;
	    DeleteMenuItems(mr, 1, CountMenuItems(mr));
	    syncPopup(mr, mpb->qpopup);
	}
    } 
}

#endif //WS_MAC
