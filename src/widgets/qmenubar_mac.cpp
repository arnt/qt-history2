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

static bool syncPopup(MenuRef ret, QPopupMenu *d)
{
    if(d) {
	for(int id = 0, x = 0; x < (int)d->count(); x++) {
	    QMenuItem *item = d->findItem(d->idAt(x));
	
	    id++; //starts at 1, because 0 means all in mac..
	    //Yes I need this, stupid!
	    QString text = item->isSeparator() ? "empty" : item->text().latin1();
	    InsertMenuItem(ret, no_ampersands(text), id);

	    if(item->isSeparator())
		ChangeMenuItemAttributes(ret, id, kMenuItemAttrSeparator, 0);
	    else {
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
	    if(item->widget() || item->custom())
		qDebug("Ooops, don't think I can handle that yet! %s:%d", __FILE__, __LINE__);
	}
    }
    return TRUE;
}

static MenuRef createPopup(QPopupMenu *d) 
{
    MenuRef ret;
    if(CreateNewMenu(textMenuProc, 0, &ret) != noErr)
	return NULL;

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
	InsertMenu(mp, x);
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
	mpb->qpopup->activateItemAt(index-1);
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
