#include <qglobal.h>
#ifdef Q_WS_MAC

#ifdef QMAC_QMENUBAR_NATIVE

#include "qt_mac.h"

#define INCLUDE_MENUITEM_DEF
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qintdict.h>
#include <qstring.h>
#include <qapplication.h>

const unsigned char * p_str(const char *); //qglobal.cpp
static MenuRef createPopup(QPopupMenu *d);
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

static const unsigned char *no_ampersands(QString i) {
    int w=0;
    while((w=i.find('&', w)) != -1) 
	i.remove(w, 1);
    return p_str(i);
}

#if 0
QMAC_PASCAL void macMenuItemProc(SInt16 msg, MenuRef mr, Rect *menuRect, Point pt, SInt16 *idx)
{
    qDebug("foo.. %d", msg);

    if(pdict) {
	short id = (short)mr;
	if(MacPopupBinding *mpb = pdict->find((int)id)) {
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
	}
    }
    *(idx) = 0;
}
#endif

static bool syncPopup(MenuRef ret, QPopupMenu *d)
{
    if(d) {
	for(int id = 1, x = 0; x < (int)d->count(); x++) {
	    QMenuItem *item = d->findItem(d->idAt(x));
	
	    if(item->custom()) {
		qDebug("Ooops, don't think I can handle that yet! %s:%d %d", __FILE__, __LINE__, x);
		continue;
	    }
	    if(item->widget()) {
		qDebug("Ooops, don't think I can handle that yet! %s:%d %d", __FILE__, __LINE__, x);
		continue;
	    }

	    QString text = "empty"; //Yes I need this, stupid!
	    if(!item->isSeparator()) {
		text = item->text();
		int st = text.findRev('\t');
		if(st != -1)
		    text.remove(st, text.length()-st);
	    }
	    InsertMenuItemText(ret, no_ampersands(text), id);
	    SetMenuItemCommandID(ret, id, item->id());

	    if(item->isSeparator()) {
		ChangeMenuItemAttributes(ret, id, kMenuItemAttrSeparator, 0);
	    } else {
		if(item->pixmap()) {
		    //handle pixmaps..
		}
		if(item->isEnabled())
		    EnableMenuItem(ret, id);
		else
		    DisableMenuItem(ret, id);
		CheckMenuItem(ret, id, item->isChecked() ? true : false);
		if(item->popup()) 
		    SetMenuItemHierarchicalMenu(ret, id, createPopup(item->popup()));
	    }
	    id++;
	}
    }
    return TRUE;
}

static MenuRef createPopup(QPopupMenu *d) 
{
    MenuRef ret;
#if 0
    MenuDefSpec spec;
    spec.defType = kMenuDefProcPtr;
    spec.u.defProc = NewMenuDefUPP(macMenuItemProc);
    if(CreateCustomMenu(&spec, textMenuProc, 0, &ret) != noErr)
	return NULL;
#else
    if(CreateNewMenu(textMenuProc, 0, &ret) != noErr)
	return NULL;
#endif

    if(!pdict) {
	pdict = new QIntDict<MacPopupBinding>();
	pdict->setAutoDelete(TRUE);
    }
    short mid = (short)ret;
    SetMenuID(ret, mid);
    pdict->insert((int)mid, new MacPopupBinding(d, ret));
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

	MenuRef mp = createPopup(item->popup());
	SetMenuTitle(mp, no_ampersands(item->text()));
	InsertMenu(mp, 0);
    }
    InvalMenuBar();
    return TRUE;
}

/* qmenubar functions */

/*!
  Internal function..
*/
bool QMenuBar::activate(short id, short index)
{
    if(!pdict)
	return FALSE;

    if(MacPopupBinding *mpb = pdict->find((int)id)) {
	MenuCommand cmd;
	GetMenuItemCommandID(mpb->macpopup, index, &cmd);
	mpb->qpopup->activateItemAt(mpb->qpopup->indexOf(cmd));
	return TRUE;
    }
    return FALSE;
}


/*!
  Internal function that cleans up the menubar.
*/
void QMenuBar::cleanup()
{
    ClearMenuBar();
    InvalMenuBar();
    delete pdict;
}

void QMenuBar::macUpdateMenuBar()
{
    if(QWidget *w = qApp->activeWindow()) {
	if(QObject *mb = qApp->activeWindow()->child(0, "QMenuBar", FALSE)) {
	    QMenuBar *bar = (QMenuBar *)mb;
	    if(!bar->mac_dirty_menubar)
		return;
	    bar->mac_dirty_menubar = 0;
	    updateMenuBar(bar);
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

#endif //QMENUBAR_NATIVE
#endif //WS_MAC
