/****************************************************************************
**
** Implementation of QWidget and QWindow classes for mac.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qmenu.h"
#include "qhash.h"
#include "qmainwindow.h"
#include "qtoolbar.h"
#include "qapplication.h"
#include "qdockarea.h"
#include "qt_mac.h"

#include "private/qmenu_p.h"
#define d d_func()
#define q q_func()


/*****************************************************************************
  QMenu debug facilities
 *****************************************************************************/

/*****************************************************************************
  QMenu globals
 *****************************************************************************/
static int qt_mac_menu_command = 'QT00';
static bool qt_mac_no_native_menubar = false;

/*****************************************************************************
  Externals
 *****************************************************************************/
IconRef qt_mac_create_iconref(const QPixmap &px); //qpixmap_mac.cpp
extern QString cfstring2qstring(CFStringRef); //qglobal.cpp
extern CFStringRef qstring2cfstring(const QString &); //qglobal.cpp

/*****************************************************************************
  QMenu utility functions
 *****************************************************************************/
inline static QString qt_mac_no_ampersands(QString str, CFStringRef *cf=NULL) {
    for(int w = 0; (w=str.indexOf('&', w)) != -1; ) {
	if(w < (int)str.length()-1) {
	    str.remove(w, 1);
	    if(str[w] == '&')
		w++;
	}
    }
    if(cf)
	*cf = qstring2cfstring(str);
    return str;
}

inline static void qt_mac_clear_menubar()
{
    ClearMenuBar();
    //qt_mac_command_set_enabled(kHICommandPreferences, false);
    InvalMenuBar();
}

/*****************************************************************************
  Q4Menu bindings
 *****************************************************************************/
Q4MenuPrivate::QMacMenuPrivate::QMacMenuPrivate() : menu(0)
{
}

Q4MenuPrivate::QMacMenuPrivate::~QMacMenuPrivate()
{
    for(QList<QMacMenuAction*>::Iterator it = actionItems.begin(); it != actionItems.end(); ++it) 
	delete (*it);
    if(menu)
	ReleaseMenu(menu); 
}

void 
Q4MenuPrivate::QMacMenuPrivate::addAction(QAction *a, QMacMenuAction *before)
{
    if(a->isSeparator())
	return;
    QMacMenuAction *action = new QMacMenuAction;
    action->action = a;
    action->visible = 0;
    action->command = qt_mac_menu_command++;
    addAction(action, before);
}

void 
Q4MenuPrivate::QMacMenuPrivate::addAction(QMacMenuAction *action, QMacMenuAction *before)
{
    if(!action)
	return;
    int before_index = actionItems.indexOf(before);
    if(actionItems.indexOf(action) == -1)
	actionItems.insert(before_index, action);
    if(action->action->isVisible()) {
	action->visible = 1;

	if(before)
	    InsertMenuItemTextWithCFString(menu, 0, before_index-1, 0, action->command);
	else
	    AppendMenuItemTextWithCFString(menu, 0, 0, action->command, 0);
	syncAction(action);
    }
}

void 
Q4MenuPrivate::QMacMenuPrivate::syncAction(QMacMenuAction *action)
{
    if(!action)
	return;
    if(!action->action->isVisible()) {
	if(action->visible) {
	    DeleteMenuItem(menu, findActionIndex(action));
	    action->visible = 0;
	}
	return;
    } else if(!action->visible) {
	QMacMenuAction *before = 0;
	int index = actionItems.indexOf(action);
	if(index+1 < actionItems.size())
	    before = actionItems[index+1];
	addAction(action, before);
	return;
    }

    MenuItemDataRec data;
    data.whichData = 0;

    //string
    data.whichData |= kMenuItemDataCFString;
    qt_mac_no_ampersands(action->action->text(), &data.cfText);

    if(action->action->menu()) { //just submenu
	data.whichData |= kMenuItemDataSubmenuHandle;
	data.submenuHandle = action->action->menu()->macMenu();
    } else {
    }

    //actually set it
    SetMenuItemData(menu, action->command, true, &data);
}

void 
Q4MenuPrivate::QMacMenuPrivate::removeAction(QMacMenuAction *action)
{
    if(!action)
	return;
    if(action->visible)
	DeleteMenuItem(menu, findActionIndex(action));
    actionItems.remove(action);
}

short
Q4MenuPrivate::QMacMenuPrivate::findActionIndex(QMacMenuAction *action)
{
    MenuItemIndex ret_idx;
    MenuRef ret_menu;
    if(GetIndMenuItemWithCommandID(menu, action->command, 1, &ret_menu, &ret_idx) == noErr) {
	if(ret_menu == menu)
	    return (short)ret_idx;
    }
    qWarning("Item not found in menu!"); //sanity check
    return -1;
}

MenuRef
Q4MenuPrivate::macMenu()
{
    if(mac_menu && mac_menu->menu)
	return mac_menu->menu;
    if(!mac_menu)
	mac_menu = new QMacMenuPrivate;
    CreateNewMenu(0, 0, &mac_menu->menu);	
    return mac_menu->menu;
}
MenuRef Q4Menu::macMenu() { return d->macMenu(); }

/*****************************************************************************
  Q4MenuBar bindings
 *****************************************************************************/
QHash<QWidget *, Q4MenuBar *> Q4MenuBarPrivate::QMacMenuBarPrivate::menubars;
QPointer<Q4MenuBar> Q4MenuBarPrivate::QMacMenuBarPrivate::fallback;

Q4MenuBarPrivate::QMacMenuBarPrivate::QMacMenuBarPrivate() : menu(0) 
{ 
}

Q4MenuBarPrivate::QMacMenuBarPrivate::~QMacMenuBarPrivate() 
{ 
    for(QList<QMacMenuAction*>::Iterator it = actionItems.begin(); it != actionItems.end(); ++it) 
	delete (*it);
    if(menu)
	ReleaseMenu(menu); 
}

void 
Q4MenuBarPrivate::QMacMenuBarPrivate::addAction(QAction *a, QMacMenuAction *before)
{
    if(a->isSeparator())
	return;
    QMacMenuAction *action = new QMacMenuAction;
    action->action = a;
    action->visible = 0;
    action->command = qt_mac_menu_command++;
    addAction(action, before);
}

void 
Q4MenuBarPrivate::QMacMenuBarPrivate::addAction(QMacMenuAction *action, QMacMenuAction *before)
{
    if(!action)
	return;
    int before_index = actionItems.indexOf(before);
    if(actionItems.indexOf(action) == -1)
	actionItems.insert(before_index, action);
    if(action->action->isVisible()) {
	action->visible = 1;
	if(before)
	    InsertMenuItemTextWithCFString(menu, 0, before_index-1, 0, action->command);
	else
	    AppendMenuItemTextWithCFString(menu, 0, 0, action->command, 0);
	syncAction(action);
    }
}

void 
Q4MenuBarPrivate::QMacMenuBarPrivate::syncAction(QMacMenuAction *action)
{
    if(!action)
	return;
    if(!action->action->isVisible()) {
	if(action->visible) {
	    DeleteMenuItem(menu, findActionIndex(action));
	    action->visible = 0;
	}
	return;
    } else if(!action->visible) {
	QMacMenuAction *before = 0;
	int index = actionItems.indexOf(action);
	if(index+1 < actionItems.size())
	    before = actionItems[index+1];
	addAction(action, before);
	return;
    }

    MenuRef submenu = 0;
    bool release_submenu = false;
    if(action->action->menu()) {
	submenu = action->action->menu()->macMenu();
    } else {
	release_submenu = true;
	CreateNewMenu(0, 0, &submenu);
    }
    SetMenuItemHierarchicalMenu(menu, findActionIndex(action), submenu);
    CFStringRef cfref;
    qt_mac_no_ampersands(action->action->text(), &cfref);
    SetMenuTitleWithCFString(submenu, cfref);
    if(release_submenu)
	ReleaseMenu(submenu);
}

void 
Q4MenuBarPrivate::QMacMenuBarPrivate::removeAction(QMacMenuAction *action)
{
    if(!action)
	return;
    if(action->visible)
	DeleteMenuItem(menu, findActionIndex(action));
    actionItems.remove(action);
}

short
Q4MenuBarPrivate::QMacMenuBarPrivate::findActionIndex(QMacMenuAction *action)
{
    MenuItemIndex ret_idx;
    MenuRef ret_menu;
    if(GetIndMenuItemWithCommandID(menu, action->command, 1, &ret_menu, &ret_idx) == noErr) {
	if(ret_menu == menu)
	    return (short)ret_idx;
    }
    qWarning("Item not found in menubar!"); //sanity check
    return -1;
}

void 
Q4MenuBarPrivate::macCreateMenuBar(QWidget *parent)
{
    if(!qt_mac_no_native_menubar) {
	if(!parent && !QMacMenuBarPrivate::fallback) {
	    QMacMenuBarPrivate::fallback = q;
	    mac_menubar = new QMacMenuBarPrivate;
	} else {
	    QWidget *tlw = q->topLevelWidget();
	    QMacMenuBarPrivate::menubars.ensure_constructed();
	    if(parent && (QMacMenuBarPrivate::menubars.isEmpty() || !QMacMenuBarPrivate::menubars.find(tlw)) &&
	       (((parent->isDialog() || ::qt_cast<QMainWindow *>(parent)) && parent == tlw) ||
		::qt_cast<QToolBar *>(parent) || tlw == qApp->mainWidget() || !qApp->mainWidget())) {
		QMacMenuBarPrivate::menubars.insert(tlw, q);
		mac_menubar = new QMacMenuBarPrivate;
	    }
	}
    }
}

void Q4MenuBarPrivate::macDestroyMenuBar()
{
    delete mac_menubar;
}

MenuRef Q4MenuBarPrivate::macMenu()
{
    if(!mac_menubar) {
	return 0;
    } else if(!mac_menubar->menu) {
	CreateNewMenu(0, 0, &mac_menubar->menu);

	QList<QAction*> items = q->actions();
	for(int i = 0; i < items.count(); i++) 
	    mac_menubar->addAction(items[i]);
    }
    return mac_menubar->menu;
}
MenuRef Q4MenuBar::macMenu() {  return d->macMenu(); }


/*****************************************************************************
 This is a huge temporary hack that will go away....
*****************************************************************************/
#if 1
#include "qmenubar.h"
bool QMenuBar::macUpdateMenuBar()
{
    if(qt_mac_no_native_menubar) //nothing to be done..
	return true;
    Q4MenuBarPrivate::QMacMenuBarPrivate::menubars.ensure_constructed();

    Q4MenuBar *mb = 0;
    bool fall_back_to_empty = false;
    //find a menubar
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
	mb = Q4MenuBarPrivate::QMacMenuBarPrivate::menubars.value(w);
	if(!mb && (!w->parentWidget() || w->parentWidget()->isDesktop()) &&
	   ::qt_cast<QDockWindow *>(w)) {
	    if(QWidget *area = ((QDockWindow*)w)->area()) {
		QWidget *areaTL = area->topLevelWidget();
		if((mb = Q4MenuBarPrivate::QMacMenuBarPrivate::menubars.value(areaTL)))
		    w = areaTL;
	    }
	}
	while(w && !mb)
	    mb = Q4MenuBarPrivate::QMacMenuBarPrivate::menubars.value((w = w->parentWidget()));
    }
    if(!w || (!w->testWFlags(WStyle_Tool) && !w->testWFlags(WType_Popup)))
	fall_back_to_empty = true;
    if(!mb)
	mb = Q4MenuBarPrivate::QMacMenuBarPrivate::fallback;
    //now set it
    static bool first = true;
    if(mb) {
	SetRootMenu(mb->macMenu());
    } else if(first || fall_back_to_empty) {
	first = false;
	qt_mac_clear_menubar();
    }
    return false;
}
#endif
