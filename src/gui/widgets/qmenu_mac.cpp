/****************************************************************************
**
** Implementation of QMenu and QMenuBar classes for mac.
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
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
#include "qregexp.h"
#ifdef QT_COMPAT
# include "q3menubar.h"
#endif

#include "private/qmenu_p.h"
#define d d_func()
#define q q_func()

/*****************************************************************************
  QMenu debug facilities
 *****************************************************************************/

/*****************************************************************************
  QMenu globals
 *****************************************************************************/
bool qt_mac_no_menubar_icons = false;
bool qt_mac_no_native_menubar = false;
bool qt_mac_no_menubar_merge = false;

static uint qt_mac_menu_static_cmd_id = 'QT00';
const UInt32 kMenuCreatorQt = 'cute';
enum {
    kMenuPropertyQAction = 'QAcT',
    kMenuPropertyQWidget = 'QWId',
    kMenuPropertyCausedQWidget = 'QCAU',
    kMenuPropertyMergeMenu = 'QApP',

    kHICommandAboutQt = 'AOQT'
};

/*****************************************************************************
  Externals
 *****************************************************************************/
extern IconRef qt_mac_create_iconref(const QPixmap &px); //qpixmap_mac.cpp
extern QString cfstring2qstring(CFStringRef); //qglobal.cpp
extern CFStringRef qstring2cfstring(const QString &); //qglobal.cpp
extern bool qt_modal_state(); //qapplication_mac.cpp

/*****************************************************************************
  QMenu utility functions
 *****************************************************************************/
inline static QString qt_mac_no_ampersands(QString str, CFStringRef *cf=NULL) {
    for(int w = -1; (w=str.indexOf('&', w+1)) != -1;) {
        if(w < (int)str.length()-1)
            str.remove(w, 1);
    }
    if(cf)
        *cf = qstring2cfstring(str);
    return str;
}

//lookup a QMacMenuAction in a menu
static short qt_mac_menu_find_action(MenuRef menu, MenuCommand cmd)
{
    MenuItemIndex ret_idx;
    MenuRef ret_menu;
    if(GetIndMenuItemWithCommandID(menu, cmd, 1, &ret_menu, &ret_idx) == noErr) {
        if(ret_menu == menu)
            return (short)ret_idx;
    }
    return -1;
}
static short qt_mac_menu_find_action(MenuRef menu, QMacMenuAction *action)
{
    return qt_mac_menu_find_action(menu, action->command);
}

//enabling of commands
void qt_mac_command_set_enabled(MenuRef menu, UInt32 cmd, bool b)
{
#if 0
    qDebug("setting %c%c%c%c to %s", (char)(cmd >> 24) & 0xFF, (char)(cmd >> 16) & 0xFF,
           (char)(cmd >> 8) & 0xFF, (char)cmd & 0xFF,  b ? "on" : "off");
#endif

    short index = qt_mac_menu_find_action(menu, cmd);
    if(index != -1) {
        UInt32 size;
        if(GetMenuItemPropertySize(menu, index, kMenuCreatorQt, kMenuPropertyQAction, &size) != noErr || size != sizeof(QAction*)) 
            return;
    } else {
        return;
    }

    if(b) {
        EnableMenuCommand(menu, cmd);
        if(MenuRef dock_menu = GetApplicationDockTileMenu())
            EnableMenuCommand(dock_menu, cmd);
    } else {
        DisableMenuCommand(menu, cmd);
        if(MenuRef dock_menu = GetApplicationDockTileMenu())
            DisableMenuCommand(dock_menu, cmd);
    }
}

//toggling of modal state
void qt_mac_set_modal_state(MenuRef menu, bool b)
{
    for(int i = 1; i < CountMenuItems(menu); i++) {
        MenuRef submenu;
        GetMenuItemHierarchicalMenu(menu, i+1, &submenu);
        if(b)
            DisableMenuItem(submenu, 0);
        else
            EnableMenuItem(submenu, 0);
    }

    UInt32 commands[] = { kHICommandQuit, kHICommandPreferences, kHICommandAbout, kHICommandAboutQt, 0 };
    for(int c = 0; commands[c]; c++)
        qt_mac_command_set_enabled(menu, commands[c], !b);
}

void qt_mac_clear_menubar()
{
    ClearMenuBar();
    qt_mac_command_set_enabled(0, kHICommandPreferences, false);
    InvalMenuBar();
}

static MenuCommand qt_mac_menu_merge_action(QMacMenuAction *action)
{
    if(qt_mac_no_menubar_merge || action->action->menu() || action->action->isSeparator())
        return 0;

    QString t = qt_mac_no_ampersands(action->action->text().toLower());
    int st = t.lastIndexOf('\t');
    if(st != -1)
        t.remove(st, t.length()-st);
    t.replace(QRegExp(QString::fromLatin1("\\.*$")), ""); //no ellipses
    //now the fun part
    MenuCommand ret = 0;
#define DO_SOMETHING(x) QString(x) //this really will DO_SOMETHING later..
    if(t.startsWith(DO_SOMETHING("About").toLower())) {
        if(t.indexOf(QRegExp(QString::fromLatin1("qt$"), false)) == -1)
            ret = kHICommandAbout;
        else
            ret = kHICommandAboutQt;
    } else if(t.startsWith(DO_SOMETHING("Config").toLower()) || t.startsWith(DO_SOMETHING("Preference").toLower()) ||
              t.startsWith(DO_SOMETHING("Options").toLower()) || t.startsWith(DO_SOMETHING("Setting").toLower()) ||
              t.startsWith(DO_SOMETHING("Setup").toLower())) {
        ret = kHICommandPreferences;
    } else if(t.startsWith(DO_SOMETHING("Quit").toLower()) || t.startsWith(DO_SOMETHING("Exit").toLower())) {
        ret = kHICommandQuit;
    }
#undef DO_SOMETHING
    return ret;
}

static bool qt_mac_auto_apple_menu(MenuCommand cmd)
{
    return (cmd == kHICommandPreferences || cmd == kHICommandQuit);
}

static QString qt_mac_menu_merge_text(MenuCommand cmd)
{
    QString ret;
    if(cmd == kHICommandAbout)
        ret = "About " + QString(qAppName());
    else if(cmd == kHICommandAboutQt)
        ret = "About Qt";
    else if(cmd == kHICommandPreferences)
        ret = "Preferences";
    else if(cmd == kHICommandQuit)
        ret = "Quit " + QString(qAppName());
    return ret;
}

//backdoors to disable/enable certain features of the menubar bindings
void qt_mac_set_no_menubar_icons(bool b) { qt_mac_no_menubar_icons = b; } //disable menubar icons
void qt_mac_set_no_native_menubar(bool b) { qt_mac_no_native_menubar = b; } //disable menubars entirely
void qt_mac_set_no_menubar_merge(bool b) { qt_mac_no_menubar_merge = b; } //disable command merging

bool qt_mac_activate_action(MenuRef menu, uint command, QAction::ActionEvent action_e, bool by_accel)
{
    MenuItemIndex index;
    {
        MenuRef tmp_menu;
        if(GetIndMenuItemWithCommandID(menu, command, 1, &tmp_menu, &index) == noErr) {
            if(!menu)
                menu = tmp_menu;
            else if(tmp_menu != menu)
                qWarning("This cannot happen!!");
        }
    }

    //fire event
    QMacMenuAction *action = 0;
    if(GetMenuItemProperty(menu, index, kMenuCreatorQt, kMenuPropertyQAction, sizeof(action), 0, &action) != noErr)
        return false;
    if(action_e == QAction::Trigger && by_accel && action->ignore_accel) //no, not a real accel (ie tab)
        return false;
    action->action->activate(action_e);

    //now walk up firing for each "caused" widget (like in the platform independant menu)
    while(menu) {
        //fire
        QWidget *widget = 0;
        GetMenuItemProperty(menu, 0, kMenuCreatorQt, kMenuPropertyQWidget, sizeof(widget), 0, &widget);
        if(QMenu *qmenu = ::qt_cast<QMenu*>(widget)) {
            if(action_e == QAction::Trigger)
                emit qmenu->activated(action->action);
            else if(action_e == QAction::Hover)
                emit qmenu->highlighted(action->action);
        } else if(Q4MenuBar *qmenubar = ::qt_cast<Q4MenuBar*>(widget)) {
            if(action_e == QAction::Trigger)
                emit qmenubar->activated(action->action);
            else if(action_e == QAction::Hover)
                emit qmenubar->highlighted(action->action);
            break; //nothing more..
        }

        //walk up
        QWidget *caused = 0;
        if(GetMenuItemProperty(menu, 0, kMenuCreatorQt, kMenuPropertyCausedQWidget, sizeof(caused), 0, &caused) != noErr)
            break;
        if(QMenu *qmenu2 = ::qt_cast<QMenu*>(caused))
            menu = qmenu2->macMenu();
        else if(Q4MenuBar *qmenubar2 = ::qt_cast<Q4MenuBar*>(caused))
            menu = qmenubar2->macMenu();
        else
            menu = 0;
    }
    if(action_e == QAction::Trigger)
        HiliteMenu(0);
    return true;
}

//handling of events for menurefs created by Qt..
static EventTypeSpec menu_events[] = {
    { kEventClassCommand, kEventCommandProcess },
    { kEventClassMenu, kEventMenuTargetItem },
    { kEventClassMenu, kEventMenuOpening },
    { kEventClassMenu, kEventMenuClosed },
};
OSStatus qt_mac_menu_event(EventHandlerCallRef er, EventRef event, void *)
{
    bool handled_event = true;
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    switch(eclass) {
    case kEventClassCommand: {
        HICommand cmd;
        GetEventParameter(event, kEventParamDirectObject, typeHICommand,
                          0, sizeof(cmd), 0, &cmd);
        UInt32 context;
        GetEventParameter(event, kEventParamMenuContext, typeUInt32,
                          0, sizeof(context), 0, &context);
        handled_event = qt_mac_activate_action(cmd.menu.menuRef, cmd.commandID,
                                               QAction::Trigger, context & kMenuContextKeyMatching);
#ifdef QT_COMPAT
        if(!handled_event) {
            if(Q3MenuBar::activate(cmd.menu.menuRef, cmd.menu.menuItemIndex, false, context & kMenuContextKeyMatching))
                handled_event = true;
        }
#endif
        break; }
    case kEventClassMenu: {
        MenuRef menu;
        GetEventParameter(event, kEventParamDirectObject, typeMenuRef, NULL, sizeof(menu), NULL, &menu);
        if(ekind == kEventMenuTargetItem) {
            MenuCommand command;
            GetEventParameter(event, kEventParamMenuCommand, typeMenuCommand,
                              0, sizeof(command), 0, &command);
            handled_event = qt_mac_activate_action(menu, command, QAction::Hover, false);
#ifdef QT_COMPAT
            if(!handled_event) {
                MenuItemIndex idx;
                GetEventParameter(event, kEventParamMenuItemIndex, typeMenuItemIndex,
                                  0, sizeof(idx), 0, &idx);
                if(!Q3MenuBar::activate(menu, idx, true))
                    handled_event = false;
            }
#endif
#ifdef QT_COMPAT
        } else if(ekind == kEventMenuOpening || ekind == kEventMenuClosed) {
            MenuRef mr;
            GetEventParameter(event, kEventParamDirectObject, typeMenuRef,
                              0, sizeof(mr), 0, &mr);
            if(ekind == kEventMenuOpening) {
                Boolean first;
                GetEventParameter(event, kEventParamMenuFirstOpen, typeBoolean,
                                  0, sizeof(first), 0, &first);
                if(first && !Q3MenuBar::macUpdatePopup(mr))
                    handled_event = false;
            }
            if(handled_event) {
                if(!Q3MenuBar::macUpdatePopupVisible(mr, ekind == kEventMenuOpening))
                    handled_event = false;
            }
#endif
        } else {
            handled_event = false;
        }
        break; }
    default:
        handled_event = false;
        break;
    }
    if(!handled_event) //let the event go through
        return CallNextEventHandler(er, event);
    return noErr; //we eat the event
}
static EventHandlerRef mac_menu_event_handler = 0;
static EventHandlerUPP mac_menu_eventUPP = 0;
static void qt_mac_cleanup_menu_event()
{
    if(mac_menu_event_handler) {
        RemoveEventHandler(mac_menu_event_handler);
        mac_menu_event_handler = 0;
    }
    if(mac_menu_eventUPP) {
        DisposeEventHandlerUPP(mac_menu_eventUPP);
        mac_menu_eventUPP = 0;
    }
}
static inline void qt_mac_create_menu_event_handler()
{
    if(!mac_menu_event_handler) {
        mac_menu_eventUPP = NewEventHandlerUPP(qt_mac_menu_event);
        InstallEventHandler(GetApplicationEventTarget(), mac_menu_eventUPP,
                            GetEventTypeCount(menu_events), menu_events, 0,
                            &mac_menu_event_handler);
        qAddPostRoutine(qt_mac_cleanup_menu_event);
    }
}

//creation of the MenuRef
static MenuRef qt_mac_create_menu(QWidget *w)
{
    MenuRef ret = 0;
    if(CreateNewMenu(0, 0, &ret) == noErr) {
        qt_mac_create_menu_event_handler();
        SetMenuItemProperty(ret, 0, kMenuCreatorQt, kMenuPropertyQWidget, sizeof(w), &w);
    } else {
        qWarning("This really cannot happen!!");
    }
    return ret;
}


/*****************************************************************************
  QMenu bindings
 *****************************************************************************/
QMenuPrivate::QMacMenuPrivate::QMacMenuPrivate() : menu(0)
{
}

QMenuPrivate::QMacMenuPrivate::~QMacMenuPrivate()
{
    for(QList<QMacMenuAction*>::Iterator it = actionItems.begin(); it != actionItems.end(); ++it)
        delete (*it);
    if(menu)
        ReleaseMenu(menu);
}

void
QMenuPrivate::QMacMenuPrivate::addAction(QAction *a, QMacMenuAction *before)
{
    if(a->isSeparator())
        return;
    QMacMenuAction *action = new QMacMenuAction;
    action->action = a;
    action->command = qt_mac_menu_static_cmd_id++;
    action->ignore_accel = 0;
    action->menu = 0;
    addAction(action, before);
}

void
QMenuPrivate::QMacMenuPrivate::addAction(QMacMenuAction *action, QMacMenuAction *before)
{
    if(!action)
        return;
    int before_index = actionItems.indexOf(before);
    actionItems.insert(before_index, action);

    short index = qt_mac_menu_find_action(menu, action);
    action->menu = menu;
    /* I don't know if this is a bug or a feature, but when the action is considered a mergable action it
       will stay that way, until removed.. */
    if(!qt_mac_no_menubar_merge) {
        if(MenuCommand cmd = qt_mac_menu_merge_action(action)) {
            GetMenuItemProperty(menu, 0, kMenuCreatorQt, kMenuPropertyMergeMenu, sizeof(action->menu), 0, &action->menu);
            action->command = cmd;
            if(qt_mac_auto_apple_menu(cmd))
                index = 0; //no need
        }
    }

    if(index == -1) {
        index = before_index;
        MenuItemAttributes attr = kMenuItemAttrAutoRepeat;
        if(before)
            InsertMenuItemTextWithCFString(action->menu, 0, before_index-1, attr, action->command);
        else
            AppendMenuItemTextWithCFString(action->menu, 0, attr, action->command, (MenuItemIndex*)&index);
    } else {
        qt_mac_command_set_enabled(action->menu, action->command, true);
    }
    SetMenuItemProperty(action->menu, index, kMenuCreatorQt, kMenuPropertyQAction, sizeof(action), &action);
    syncAction(action);
}

void
QMenuPrivate::QMacMenuPrivate::syncAction(QMacMenuAction *action)
{
    if(!action)
        return;
    short index = qt_mac_menu_find_action(action->menu, action);

    if(!action->action->isVisible()) {
        ChangeMenuItemAttributes(action->menu, index, kMenuItemAttrHidden, 0);
        return;
    }
    ChangeMenuItemAttributes(action->menu, index, 0, kMenuItemAttrHidden);

    if(action->action->isSeparator()) {
        ChangeMenuItemAttributes(action->menu, index, kMenuItemAttrSeparator, 0);
        return;
    }
    ChangeMenuItemAttributes(action->menu, index, 0, kMenuItemAttrSeparator);

    //find text (and accel)
    action->ignore_accel = 0;
    QString text = action->action->text();
    QKeySequence accel = action->action->accel();
    {
        int st = text.lastIndexOf('\t');
        if(st != -1) {
            action->ignore_accel = 1;
            accel = QKeySequence(text.right(text.length()-(st+1)));
            text.remove(st, text.length()-st);
        }
    }
    {
        QString cmd_text = qt_mac_menu_merge_text(action->command);
        if(!cmd_text.isNull())
            text = cmd_text;
    }
    if(accel.count() > 1)
        text += " (****)"; //just to denote a multi stroke accelerator

    MenuItemDataRec data;
    memset(&data, '\0', sizeof(data));

    //string
    data.whichData |= kMenuItemDataCFString;
    qt_mac_no_ampersands(text, &data.cfText);

    //enabled
    data.whichData |= kMenuItemDataEnabled;
    data.enabled = action->action->isEnabled();

    //icon
    data.whichData |= kMenuItemDataIconHandle;
    if(!action->action->icon().isNull() && !qt_mac_no_menubar_icons) {
        data.iconType = kMenuIconRefType;
        data.iconHandle = (Handle)qt_mac_create_iconref(action->action->icon().pixmap(QIconSet::Small, QIconSet::Normal));
    } else {
        data.iconType = kMenuNoIcon;
    }

    data.whichData |= kMenuItemDataSubmenuHandle;
    if(action->action->menu()) { //submenu
        data.submenuHandle = action->action->menu()->macMenu();
        QWidget *caused = 0;
        GetMenuItemProperty(action->menu, 0, kMenuCreatorQt, kMenuPropertyQWidget, sizeof(caused), 0, &caused);
        SetMenuItemProperty(data.submenuHandle, 0, kMenuCreatorQt, kMenuPropertyCausedQWidget, sizeof(caused), &caused);
    } else { //respect some other items
        //accelerators
        if(accel.isEmpty()) {
            data.whichData |= kMenuItemDataCmdKeyModifiers;
            data.whichData |= kMenuItemDataCmdKeyGlyph;
        } else {
            int accel_key = accel[0];
            data.whichData |= kMenuItemDataCmdKeyModifiers;
            if((accel_key & Qt::CTRL) != Qt::CTRL)
                data.cmdKeyModifiers |= kMenuNoCommandModifier;
            if((accel_key & Qt::META) == Qt::META)
                data.cmdKeyModifiers |= kMenuControlModifier;
            if((accel_key & Qt::ALT) == Qt::ALT)
                data.cmdKeyModifiers |= kMenuOptionModifier;
            if((accel_key & Qt::SHIFT) == Qt::SHIFT)
                data.cmdKeyModifiers |= kMenuShiftModifier;

            accel_key &= ~(Qt::MODIFIER_MASK | Qt::UNICODE_ACCEL);
            if(accel_key == Qt::Key_Return)
                data.cmdKeyGlyph = kMenuReturnGlyph;
            else if(accel_key == Qt::Key_Enter)
                data.cmdKeyGlyph = kMenuEnterGlyph;
            else if(accel_key == Qt::Key_Tab)
                data.cmdKeyGlyph = kMenuTabRightGlyph;
            else if(accel_key == Qt::Key_Backspace)
                data.cmdKeyGlyph = kMenuDeleteLeftGlyph;
            else if(accel_key == Qt::Key_Delete)
                data.cmdKeyGlyph = kMenuDeleteRightGlyph;
            else if(accel_key == Qt::Key_Escape)
                data.cmdKeyGlyph = kMenuEscapeGlyph;
            else if(accel_key == Qt::Key_PageUp)
                data.cmdKeyGlyph = kMenuPageUpGlyph;
            else if(accel_key == Qt::Key_PageDown)
                data.cmdKeyGlyph = kMenuPageDownGlyph;
            else if(accel_key == Qt::Key_Up)
                data.cmdKeyGlyph = kMenuUpArrowGlyph;
            else if(accel_key == Qt::Key_Down)
                data.cmdKeyGlyph = kMenuDownArrowGlyph;
            else if(accel_key == Qt::Key_Left)
                data.cmdKeyGlyph = kMenuLeftArrowGlyph;
            else if(accel_key == Qt::Key_Right)
                data.cmdKeyGlyph = kMenuRightArrowGlyph;
            else if(accel_key == Qt::Key_CapsLock)
                data.cmdKeyGlyph = kMenuCapsLockGlyph;
            else if(accel_key >= Qt::Key_F1 && accel_key <= Qt::Key_F15)
                data.cmdKeyGlyph = (accel_key - Qt::Key_F1) + kMenuF1Glyph;
            else if(accel_key == Qt::Key_Home)
                data.cmdKeyGlyph = kMenuNorthwestArrowGlyph;
            else if(accel_key == Qt::Key_End)
                data.cmdKeyGlyph = kMenuSoutheastArrowGlyph;
            if(data.cmdKeyGlyph) {
                data.whichData |= kMenuItemDataCmdKeyGlyph;
            } else {
                data.whichData |= kMenuItemDataCmdKey;
                data.cmdKey = (UniChar)accel_key;
            }
        }
    }

    //actually set it
    SetMenuItemData(action->menu, action->command, true, &data);

    //grrrrr.. why isn't checked in the data?
    MacCheckMenuItem(action->menu, index, action->action->isChecked() && action->action->isCheckable());
}

void
QMenuPrivate::QMacMenuPrivate::removeAction(QMacMenuAction *action)
{
    if(!action)
        return;
    if(action->command == kHICommandQuit || action->command == kHICommandPreferences)
        qt_mac_command_set_enabled(action->menu, action->command, false);
    else
        DeleteMenuItem(action->menu, qt_mac_menu_find_action(action->menu, action));
    actionItems.remove(action);
}

MenuRef
QMenuPrivate::macMenu(MenuRef merge)
{
    if(mac_menu && mac_menu->menu)
        return mac_menu->menu;
    if(!mac_menu)
        mac_menu = new QMacMenuPrivate;
    mac_menu->menu = qt_mac_create_menu(q);
    if(merge)
        SetMenuItemProperty(mac_menu->menu, 0, kMenuCreatorQt, kMenuPropertyMergeMenu, sizeof(merge), &merge);

    QList<QAction*> items = q->actions();
    for(int i = 0; i < items.count(); i++) 
        mac_menu->addAction(items[i]);
    return mac_menu->menu;
}
MenuRef QMenu::macMenu(MenuRef merge) { return d->macMenu(merge); }

/*****************************************************************************
  Q4MenuBar bindings
 *****************************************************************************/
QHash<QWidget *, Q4MenuBar *> Q4MenuBarPrivate::QMacMenuBarPrivate::menubars;
QPointer<Q4MenuBar> Q4MenuBarPrivate::QMacMenuBarPrivate::fallback;

Q4MenuBarPrivate::QMacMenuBarPrivate::QMacMenuBarPrivate() : menu(0), apple_menu(0)
{
}

Q4MenuBarPrivate::QMacMenuBarPrivate::~QMacMenuBarPrivate()
{
    for(QList<QMacMenuAction*>::Iterator it = actionItems.begin(); it != actionItems.end(); ++it)
        delete (*it);
    if(apple_menu)
        ReleaseMenu(apple_menu);
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
    action->ignore_accel = 1;
    action->menu = 0;
    action->command = qt_mac_menu_static_cmd_id++;
    addAction(action, before);
}

void
Q4MenuBarPrivate::QMacMenuBarPrivate::addAction(QMacMenuAction *action, QMacMenuAction *before)
{
    if(!action || !menu)
        return;
    if(!qt_mac_no_menubar_merge && !apple_menu) { //handle the apple menu
        QWidget *widget = 0;
        GetMenuItemProperty(menu, 0, kMenuCreatorQt, kMenuPropertyQWidget, sizeof(widget), 0, &widget);

        //create
        MenuItemIndex index;
        apple_menu = qt_mac_create_menu(widget);
        AppendMenuItemTextWithCFString(menu, 0, 0, 0, &index);

        //set it up
        CFStringRef cfref = qstring2cfstring(QString(QChar(0x14)));
        SetMenuTitleWithCFString(apple_menu, cfref);
        CFRelease(cfref);
        SetMenuItemHierarchicalMenu(menu, index, apple_menu);
        SetMenuItemProperty(apple_menu, 0, kMenuCreatorQt, kMenuPropertyQWidget, sizeof(widget), &widget);
    }

    int before_index = actionItems.indexOf(before);
    actionItems.insert(before_index, action);

    action->menu = menu;
    MenuItemIndex index = before_index;
    if(before)
        InsertMenuItemTextWithCFString(action->menu, 0, before_index-1, 0, action->command);
    else
        AppendMenuItemTextWithCFString(action->menu, 0, 0, action->command, &index);
    SetMenuItemProperty(action->menu, index, kMenuCreatorQt, kMenuPropertyQAction, sizeof(action), &action);
    syncAction(action);
}

void
Q4MenuBarPrivate::QMacMenuBarPrivate::syncAction(QMacMenuAction *action)
{
    if(!action || !menu)
        return;
    const short index = qt_mac_menu_find_action(action->menu, action);

    if(!action->action->isVisible()) {
        ChangeMenuItemAttributes(action->menu, index, kMenuItemAttrHidden, 0);
        return;
    }
    ChangeMenuItemAttributes(action->menu, index, 0, kMenuItemAttrHidden);

    if(action->action->isSeparator()) {
        ChangeMenuItemAttributes(action->menu, index, kMenuItemAttrSeparator, 0);
        return;
    }
    ChangeMenuItemAttributes(action->menu, index, 0, kMenuItemAttrSeparator);

    MenuRef submenu = 0;
    bool release_submenu = false;
    if(action->action->menu()) {
        submenu = action->action->menu()->macMenu(apple_menu);
        QWidget *caused = 0;
        GetMenuItemProperty(action->menu, 0, kMenuCreatorQt, kMenuPropertyQWidget, sizeof(caused), 0, &caused);
        SetMenuItemProperty(submenu, 0, kMenuCreatorQt, kMenuPropertyCausedQWidget, sizeof(caused), &caused);
    } else {
        release_submenu = true;
        CreateNewMenu(0, 0, &submenu);
    }
    SetMenuItemHierarchicalMenu(action->menu, qt_mac_menu_find_action(action->menu, action), submenu);
    CFStringRef cfref;
    qt_mac_no_ampersands(action->action->text(), &cfref);
    SetMenuTitleWithCFString(submenu, cfref);
    if(release_submenu)
        ReleaseMenu(submenu);
}

void
Q4MenuBarPrivate::QMacMenuBarPrivate::removeAction(QMacMenuAction *action)
{
    if(!action || !menu)
        return;
    DeleteMenuItem(action->menu, qt_mac_menu_find_action(action->menu, action));
    actionItems.remove(action);
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
    mac_menubar = 0;
}

MenuRef Q4MenuBarPrivate::macMenu()
{
    if(!mac_menubar) {
        return 0;
    } else if(!mac_menubar->menu) {
        mac_menubar->menu = qt_mac_create_menu(q);

        QList<QAction*> items = q->actions();
        for(int i = 0; i < items.count(); i++)
            mac_menubar->addAction(items[i]);
    }
    return mac_menubar->menu;
}
MenuRef Q4MenuBar::macMenu() {  return d->macMenu(); }

bool Q4MenuBar::macUpdateMenuBar()
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
    bool ret = false;
    if(mb) {
        MenuRef menu = mb->macMenu();
        SetRootMenu(menu);
        if(mb != Q4MenuBarPrivate::QMacMenuBarPrivate::menubars.value(qApp->activeModalWidget()))
            qt_mac_set_modal_state(menu, qt_modal_state());
        ret = true;
#ifdef QT_COMPAT
    } else if(Q3MenuBar::macUpdateMenuBar()) {
        qt_mac_create_menu_event_handler();
        ret = true;
#endif
    } else if(first || fall_back_to_empty) {
        first = false;
        qt_mac_clear_menubar();
    }
    return ret;
}
