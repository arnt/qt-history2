/****************************************************************************
**
** Implementation of Q3MenuBar bindings for Apple System Menubar.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt Compat Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qplatformdefs.h"
#if defined(Q_WS_MAC)
#include "qt_mac.h"
#if !defined(QMAC_Q3MENUBAR_NO_NATIVE)

#define QMAC_Q3MENUBAR_CPP_FILE
#include <private/qmenubar_p.h>
#include <qaccel.h>
#include <qapplication.h>
#include <qdockwindow.h>
#include <qpointer.h>
#include <qhash.h>
#include <qmainwindow.h>
#include <q3menubar.h>
#include <qmessagebox.h>
#include <q3popupmenu.h>
#include <qregexp.h>
#include <qstring.h>
#include <qtoolbar.h>

/*****************************************************************************
  Q3Menubar debug facilities
 *****************************************************************************/
//#define DEBUG_MENUBAR_ACTIVATE

IconRef qt_mac_create_iconref(const QPixmap &px); //qpixmap_mac.cpp
QByteArray p2qstring(const unsigned char *); //qglobal.cpp
void qt_event_request_menubarupdate(); //qapplication_mac.cpp
bool qt_modal_state(); //qapplication_mac.cpp
void qt_mac_clear_menubar(); //qmenu_mac.cpp

extern bool qt_mac_no_menubar_icons; //qmenu_mac.cpp
extern bool qt_mac_no_native_menubar; //qmenu_mac.cpp
extern bool qt_mac_no_menubar_merge; //qmenu_mac.cpp

#endif

void qt_mac_command_set_enabled(MenuRef menu, UInt32 cmd, bool b); //qmenu_mac.cpp
static inline void qt_mac_command_set_enabled(UInt32 cmd, bool b) { qt_mac_command_set_enabled(0, cmd, b); }

#if !defined(QMAC_Q3MENUBAR_NO_NATIVE)

//internal class
class Q3MenuBar::MacPrivate {
public:
    MacPrivate() : mac_menubar(0), apple_menu(0), in_apple(0), dirty(false), modal(false) { }
    ~MacPrivate() { clear(); }

    class CommandBinding {
    public:
        CommandBinding(Q3PopupMenu *m = 0, uint i = 0) : qpopup(m), index(i) { }
        Q3PopupMenu *qpopup;
        int index;
    };
    QHash<int, CommandBinding> commands;

    class PopupBinding {
    public:
        PopupBinding(Q3PopupMenu *m, MenuRef r, int i, bool b) : qpopup(m), macpopup(r), id(i), tl(b) { }
        ~PopupBinding() { if(tl) DeleteMenu(GetMenuID(macpopup)); DisposeMenu(macpopup); }
        Q3PopupMenu *qpopup;
        MenuRef macpopup;
        int id;
        bool tl;
    };
    QHash<int, PopupBinding*> popups;
    MenuBarHandle mac_menubar;
    MenuRef apple_menu;
    int in_apple;
    bool dirty, modal;

    void clear() {
        in_apple = 0;
        if (apple_menu) {
            DeleteMenu(GetMenuID(apple_menu));
            DisposeMenu(apple_menu);
        }
        QHash<int, PopupBinding*>::ConstIterator it = popups.constBegin();
        while (it != popups.constEnd()) {
            PopupBinding *val = it.value();
            delete val;
            ++it;
        }
        popups.clear();
        commands.clear();
        if (mac_menubar) {
            DisposeMenuBar(mac_menubar);
            mac_menubar = 0;
        }
    }
};
static QPointer<Q3MenuBar> fallbackMenuBar; //The current global menubar
static QPointer<Q3MenuBar> activeMenuBar; //The current global menubar

extern void qt_mac_set_modal_state(MenuRef, bool); //qmenu_mac.cpp
static void qt_mac_set_modal_state(bool b, Q3MenuBar *mb)
{
    if(mb && mb != activeMenuBar) { //shouldn't be possible, but just in case
        qWarning("%s:%d: This cannot happen!", __FILE__, __LINE__);
        mb = 0;
    }
    MenuRef mr = AcquireRootMenu();
    qt_mac_set_modal_state(mr, b);
    ReleaseMenu(mr);
}

/* utility functions */
static QString qt_mac_no_ampersands(QString str) {
    for(int w = 0; (w=str.indexOf('&', w)) != -1;) {
        if(w < (int)str.length()-1) {
            str.remove(w, 1);
            if(str[w] == '&')
                w++;
        }
    }
    return str;
}

/*!
    \internal
*/
bool Q3PopupMenu::macPopupMenu(const QPoint &p, int index)
{
#if 0
    if(index == -1 && activeMenuBar && activeMenuBar->mac_d->popups) {
        MenuRef ref = 0;
        for(QIntDictIterator<Q3MenuBar::MacPrivate::PopupBinding> it(*(activeMenuBar->mac_d->popups)); it.current(); ++it) {
            if(it.current()->qpopup == this) {
                ref = it.current()->macpopup;
                break;
            }
        }
        if(ref) {
            activeMenuBar->syncPopups(ref, this);
            InvalidateMenuSize(ref);
            PopUpMenuSelect(ref, p.y(), p.x(), -1);
            return true; //we did it for Qt..
        }
    }
#else
    Q_UNUSED(p);
    Q_UNUSED(index);
#endif
    return false;
}


#if !defined(QMAC_Q3MENUBAR_NO_MERGE)
uint Q3MenuBar::isCommand(Q3MenuItem *it, bool just_check)
{
    if(qt_mac_no_menubar_merge || it->popup() || it->custom() || it->isSeparator())
        return 0;

    QString t = qt_mac_no_ampersands(it->text().toLower());
    int st = t.lastIndexOf('\t');
    if(st != -1)
        t.remove(st, t.length()-st);
    t.replace(QRegExp(QString::fromLatin1("\\.*$")), ""); //no ellipses
    //now the fun part
    uint ret = 0;
    if(t.startsWith(tr("About").toLower())) {
        if(t.indexOf(QRegExp(QString::fromLatin1("qt$"), false)) == -1)
            ret = kHICommandAbout;
        else
            ret = 'CUTE';
    } else if(t.startsWith(tr("Config").toLower()) || t.startsWith(tr("Preference").toLower()) ||
              t.startsWith(tr("Options").toLower()) || t.startsWith(tr("Setting").toLower()) ||
              t.startsWith(tr("Setup").toLower())) {
        ret = kHICommandPreferences;
    } else if(t.startsWith(tr("Quit").toLower()) || t.startsWith(tr("Exit").toLower())) {
        ret = kHICommandQuit;
    }
    //shall we?
    if(just_check) {
        //do nothing, we already checked
    } else if(ret && activeMenuBar && !activeMenuBar->mac_d->commands.contains(ret)) {
        if(ret == kHICommandAbout || ret == 'CUTE') {
            if(activeMenuBar->mac_d->apple_menu) {
                QString text = qt_mac_no_ampersands(it->text());
                int st = text.lastIndexOf('\t');
                if(st != -1)
                    text.remove(st, text.length()-st);
                text.replace(QRegExp(QString::fromLatin1("\\.*$")), ""); //no ellipses
                if(ret == kHICommandAbout && text.toLower() == tr("About").toLower())
                    text += " " + QString(qAppName());
                InsertMenuItemTextWithCFString(activeMenuBar->mac_d->apple_menu,
                                               QCFString(qt_mac_no_ampersands(text)),
                                               activeMenuBar->mac_d->in_apple++,
                                               kMenuItemAttrAutoRepeat, ret);
            }
        }
        qt_mac_command_set_enabled(ret, it->isEnabled());
    } else {
        ret = 0;
    }
    return ret;
}
#endif

bool Q3MenuBar::syncPopups(MenuRef ret, Q3PopupMenu *d)
{
    int added = 0;
    if(d) {
        if(!d->isEnabled())
            DisableMenuItem(ret, 0);
        ChangeMenuAttributes(ret, !d->isCheckable() ? kMenuAttrExcludesMarkColumn : 0,
                             d->isCheckable() ? kMenuAttrExcludesMarkColumn : 0);
                if(qMacVersion() >= Qt::MV_PANTHER) { //insert a separator
                    static int sep_id = 'SEP0';
                    InsertMenuItemTextWithCFString(activeMenuBar->mac_d->apple_menu,
                                                   0, activeMenuBar->mac_d->in_apple++,
                                                   kMenuItemAttrSeparator, sep_id++);
                }
        int id = 1;

        for (int index = 0; index < d->mitems->size(); ++index) {
#if !defined(QMAC_Q3MENUBAR_NO_MERGE)
            bool found = false;
            QHash<int, Q3MenuBar::MacPrivate::CommandBinding>::Iterator cmd_it =
                activeMenuBar->mac_d->commands.begin();
            for (; cmd_it != mac_d->commands.end() && !found; ++cmd_it)
                found = (cmd_it.value().index == index && cmd_it.value().qpopup == d);
            if (found)
                continue;
#endif

            Q3MenuItem *item = d->mitems->at(index);
            if (item->custom())
                continue;
            if (item->widget())
                continue;
            if (!item->isVisible())
                continue;

            QString text = "empty",
                    accel; //Yes, I need this.
            if(!item->isSeparator()) {
#if !defined(QMAC_Q3MENUBAR_NO_MERGE)
                if (int cmd = isCommand(item)) {
                    activeMenuBar->mac_d->commands.insert(cmd,
                            Q3MenuBar::MacPrivate::CommandBinding(d, index));
                    continue;
                }
#endif
                text = item->text();
                int st = text.lastIndexOf('\t');
                if(st != -1) {
                    accel = text.right(text.length()-(st+1));
                    text.remove(st, text.length()-st);
                }
            }
#if !defined(QMAC_Q3MENUBAR_NO_MERGE)
            else if(index != d->mitems->count()-1) {
                if(isCommand(d->mitems->at(index+1)), true) {
                    if(index+1 == d->mitems->count()-1)
                        continue;
                    if(d->mitems->at(index+2)->isSeparator())
                        continue;
                }
            }
#endif

            MenuItemAttributes attr = kMenuItemAttrAutoRepeat;

            //figure out an accelerator
            int accel_key = Qt::Key_unknown;
            if(accel.isEmpty() && item->key().count() > 1)
                text += " (****)"; //just to denote a multi stroke accelerator
            if(item->key().count() == 1 && (int)item->key() != accel_key)
                accel_key = (int)item->key();
            else if(!accel.isEmpty())
                accel_key = QKeySequence(accel);

            InsertMenuItemTextWithCFString(ret, QCFString(qt_mac_no_ampersands(text)),
                                           id, attr, item->id());
            if(item->isSeparator()) {
                ChangeMenuItemAttributes(ret, id, kMenuItemAttrSeparator, 0);
            } else {
                if(!qt_mac_no_menubar_icons) {
                    if(item->pixmap()) {                     //handle pixmaps..
                        IconRef ico = qt_mac_create_iconref(*item->pixmap());
                        SetMenuItemIconHandle(ret, id, kMenuIconRefType, (Handle)ico);
                    } else if(item->iconSet()) {
                        IconRef ico = qt_mac_create_iconref(item->iconSet()->pixmap(QIconSet::Small, QIconSet::Normal));
                        SetMenuItemIconHandle(ret, id, kMenuIconRefType, (Handle)ico);
                    }
                }
                if(item->isEnabled())
                    EnableMenuItem(ret, id);
                else
                    DisableMenuItem(ret, id);
                CheckMenuItem(ret, id, item->isChecked() ? true : false);
                if(item->popup()) {
                    SetMenuItemHierarchicalMenu(ret, id, createMacPopup(item->popup(), item->id()));
                } else if(accel_key != Qt::Key_unknown) {
                    char mod = 0;
                    if((accel_key & Qt::CTRL) != Qt::CTRL)
                        mod |= kMenuNoCommandModifier;
                    if((accel_key & Qt::META) == Qt::META)
                        mod |= kMenuControlModifier;
                    if((accel_key & Qt::ALT) == Qt::ALT)
                        mod |= kMenuOptionModifier;
                    if((accel_key & Qt::SHIFT) == Qt::SHIFT)
                        mod |= kMenuShiftModifier;
                    int keycode = (accel_key & ~(Qt::MODIFIER_MASK | Qt::UNICODE_ACCEL));
                    if(keycode) {
                        SetMenuItemModifiers(ret, id, mod);
                        bool do_glyph = true;
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
                        else if(keycode == Qt::Key_Home)
                            keycode = kMenuNorthwestArrowGlyph;
                        else if(keycode == Qt::Key_End)
                            keycode = kMenuSoutheastArrowGlyph;
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
                            do_glyph = false;
                            if(keycode < 127) { //regular ascii accel
                                SetItemCmd(ret, id, (CharParameter)keycode);
                            } else { //I guess I missed one, fix above if this happens
                                QKeySequence key(keycode);
                                qDebug("Q3MenuBar: Not sure how to handle accelerator 0x%04x (%s)",
                                       keycode, ((QString)key).latin1());
                            }
                        }
                        if(do_glyph) //"special" accelerator..
                            SetMenuItemKeyGlyph(ret, id, (SInt16)keycode);
                    }
                }
            }
            id++;
            added++;
        }
    }
    return added != 0;
}

MenuRef Q3MenuBar::createMacPopup(Q3PopupMenu *d, int id, bool top_level)
{
    static int mid = 0;
    MenuAttributes attr = 0;
#if 0
    attr |= kMenuAttrAutoDisable;
#endif
    MenuRef ret;
    if(CreateNewMenu(0, attr, &ret) != noErr)
        return 0;
    if(d && !syncPopups(ret, d) && d->count() && !d->receivers(SIGNAL(aboutToShow()))) {
        ReleaseMenu(ret);
        ret = 0;
    } else {
        SetMenuID(ret, ++mid);
        activeMenuBar->mac_d->popups.insert(mid,
                                            new Q3MenuBar::MacPrivate::PopupBinding(d, ret, id,
                                                                                   top_level));
    }
    return ret;
}

bool Q3MenuBar::updateMenuBar()
{
    if(this != activeMenuBar)
        qDebug("Shouldn't have happened! %s:%d", __FILE__, __LINE__);
    qt_mac_clear_menubar();
    if(mac_d)
        mac_d->clear();
    if(!CreateNewMenu(0, 0, &mac_d->apple_menu)) {
        SetMenuTitleWithCFString(mac_d->apple_menu, QCFString(QString(QChar(0x14))));
        InsertMenu(mac_d->apple_menu, 0);
    }

    for (int i = 0; i < mitems->size(); ++i) {
        Q3MenuItem *item = mitems->at(i);
        if (item->isSeparator() || !item->isVisible()) //mac doesn't support these
            continue;
        if(MenuRef mp = createMacPopup(item->popup(), item->id(), true)) {
            SetMenuTitleWithCFString(mp, QCFString(qt_mac_no_ampersands(item->text())));
            InsertMenu(mp, 0);
            if(item->isEnabled())
                EnableMenuItem(mp, 0);
            else
                DisableMenuItem(mp, 0);
        }
    }
    return true;
}

/* q3menubar functions */

/*!
    \internal
*/
bool Q3MenuBar::activateCommand(uint cmd)
{
#if !defined(QMAC_Q3MENUBAR_NO_MERGE)
    if(activeMenuBar) {
        MacPrivate::CommandBinding mcb = activeMenuBar->mac_d->commands.value(cmd);
        if (mcb.qpopup) {
#ifdef DEBUG_MENUBAR_ACTIVATE
            qDebug("ActivateCommand: activating internal merging '%s'",
                    mcb.qpopup->text(mcb.qpopup->idAt(mcb.index)).latin1());
#endif
            mcb.qpopup->activateItemAt(mcb.index);
            HiliteMenu(0);
            return true;
        }
    }
#endif
    HiliteMenu(0);
    return false;
}

/*!
    \internal
*/
bool Q3MenuBar::activate(MenuRef menu, short idx, bool highlight, bool by_accel)
{
    if(!activeMenuBar)
        return false;

    int mid = GetMenuID(menu);
    if(MacPrivate::PopupBinding *mpb = activeMenuBar->mac_d->popups.value(mid)) {
        MenuCommand cmd;
        GetMenuItemCommandID(mpb->macpopup, idx, &cmd);
        if(by_accel) {
            int key = mpb->qpopup->findItem(cmd)->key();
            if(key == Qt::Key_unknown) {
#ifdef DEBUG_MENUBAR_ACTIVATE
                qDebug("ActivateMenuitem: ignored due to fake accelerator '%s' %d",
                       mpb->qpopup->text(cmd).latin1(), highlight);
#endif
                return false;
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
        return true;
    }
    if(!highlight)
        HiliteMenu(0);
    return false;
}

static QHash<QWidget *, Q3MenuBar *> menubars;
/*!
  \internal
  Internal function that cleans up the menubar.
*/
void Q3MenuBar::macCreateNativeMenubar()
{
    menubars.ensure_constructed();
    macDirtyNativeMenubar();
    QWidget *p = parentWidget();
    mac_eaten_menubar = false;
    if(qt_mac_no_native_menubar) {
        //do nothing..
    } else if(!p && !fallbackMenuBar) {
        fallbackMenuBar = this;
        mac_eaten_menubar = true;
        if(!mac_d)
            mac_d = new MacPrivate;
    } else if(p && (menubars.isEmpty() || !menubars.find(topLevelWidget())) &&
              (((p->isDialog() || ::qt_cast<QMainWindow *>(p)) && p->isTopLevel())
               || ::qt_cast<QToolBar *>(p) || topLevelWidget() == qApp->mainWidget()
               || !qApp->mainWidget())) {
        mac_eaten_menubar = true;
        menubars.insert(topLevelWidget(), this);
        if(!mac_d)
            mac_d = new MacPrivate;
    }
    if(mac_eaten_menubar) {
        static bool first = true;
        if(first) {
            Q3MenuBarCallBacks *cb = new Q3MenuBarCallBacks;
            cb->activate = Q3MenuBar::activate;
            cb->updatePopup = Q3MenuBar::macUpdatePopup;
            cb->updatePopupVisible = Q3MenuBar::macUpdatePopupVisible;
            cb->updateMenuBar = Q3MenuBar::macUpdateMenuBar;
        }
    }
}
void Q3MenuBar::macRemoveNativeMenubar()
{
    if (mac_eaten_menubar) {
        for(QHash<QWidget *, Q3MenuBar *>::Iterator it = menubars.begin(); it != menubars.end();) {
            if (*it == this)
                it = menubars.erase(it);
            else
                ++it;
        }
    }
    mac_eaten_menubar = false;
    if (this == activeMenuBar) {
        activeMenuBar = 0;
        qt_mac_clear_menubar();
    }
    if (mac_d) {
        delete mac_d;
        mac_d = 0;
    }
}

/*!  \internal */
void Q3MenuBar::macDirtyNativeMenubar()
{
    if(mac_eaten_menubar && mac_d) {
        mac_d->dirty = 1;
        qt_event_request_menubarupdate();
    }
}

/*!
    \internal
*/
bool Q3MenuBar::macUpdateMenuBar()
{
    if(qt_mac_no_native_menubar) //nothing to be done..
        return true;
    menubars.ensure_constructed();

    Q3MenuBar *mb = 0;
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
        mb = menubars.value(w);
        if(!mb && (!w->parentWidget() || w->parentWidget()->isDesktop())
           && ::qt_cast<QDockWindow *>(w)) {
            if(QWidget *area = ((QDockWindow*)w)->mainWindow()) {
                QWidget *areaTL = area->topLevelWidget();
                if((mb = menubars.value(areaTL)))
                    w = areaTL;
            }
        }
        while(w && /*!w->testWFlags(Qt::WShowModal) &&*/ !mb)
            mb = menubars.value((w = w->parentWidget()));
    }
    if(!w || (!w->testWFlags(Qt::WStyle_Tool) && !w->testWFlags(Qt::WType_Popup)))
        fall_back_to_empty = true;
    if(!mb)
        mb = fallbackMenuBar;
    //now set it
    static bool first = true;
    if(mb) {
        if(!mb->mac_eaten_menubar || (!first && !mb->mac_d->dirty && (mb == activeMenuBar))) {
            if(mb->mac_d->modal != qt_modal_state()) {
                bool qms = qt_modal_state();
                if(!qms || (menubars.value(qApp->activeModalWidget()) != mb))
                    qt_mac_set_modal_state(mb->mac_d->modal = qms, mb);
            }
            return mb->mac_eaten_menubar;
        }
        first = false;
        activeMenuBar = mb;
        if(mb->mac_d->dirty || !mb->mac_d->mac_menubar) {
            mb->mac_d->dirty = 0;
            mb->updateMenuBar();
            mb->mac_d->mac_menubar = GetMenuBar();
        } else {
            SetMenuBar(mb->mac_d->mac_menubar);
            QHash<int, Q3MenuBar::MacPrivate::CommandBinding>::Iterator it
                = mb->mac_d->commands.begin();
            for(; it != mb->mac_d->commands.end(); ++it)
                qt_mac_command_set_enabled(it.key(), true);
            InvalMenuBar();
        }
        if(mb->mac_d->modal != qt_modal_state()) {
            bool qms = qt_modal_state();
            if(!qms || (menubars.value(qApp->activeModalWidget()) != mb))
                qt_mac_set_modal_state(mb->mac_d->modal = qms, mb);
        }
        return true;
    } else if(first || fall_back_to_empty) {
        first = false;
        activeMenuBar = 0;
        qt_mac_clear_menubar();
    }
    return false;
}

/*!
    \internal
*/
bool Q3MenuBar::macUpdatePopup(MenuRef mr)
{
    if(!mr || !activeMenuBar)
        return false;

    int mid = GetMenuID(mr);
    if (MacPrivate::PopupBinding *mpb = activeMenuBar->mac_d->popups.value(mid)) {
        if(mpb->qpopup) {
            emit mpb->qpopup->aboutToShow();
            if (1 || mpb->qpopup->mac_dirty_popup) {
                mpb->qpopup->mac_dirty_popup = 0;
                DeleteMenuItems(mr, 1, CountMenuItems(mr));
                activeMenuBar->syncPopups(mr, mpb->qpopup);
                return true;
            }
        } else {
            activeMenuBar->activateItemAt(activeMenuBar->indexOf(mpb->id));
            HiliteMenu(0);
        }
    } else if (mr == activeMenuBar->mac_d->apple_menu) {
        QHash<int, MacPrivate::CommandBinding>::const_iterator it
                                                    = activeMenuBar->mac_d->commands.constBegin();
        for (; it != activeMenuBar->mac_d->commands.constEnd(); ++it) {
            const MacPrivate::CommandBinding &cb = it.value();
            qt_mac_command_set_enabled(it.key(),
                                       cb.qpopup->isItemEnabled(cb.qpopup->idAt(cb.index)));
        }
    }
    return false;
}

/*!
    \internal
*/
bool Q3MenuBar::macUpdatePopupVisible(MenuRef mr, bool vis)
{
    Q_UNUSED(vis);
    if(!mr || !activeMenuBar || !qApp)
        return false;

    int mid = GetMenuID(mr);
    if(MacPrivate::PopupBinding *mpb = activeMenuBar->mac_d->popups.value(mid)) {
        if(mpb->qpopup) {
            if (!vis)
                emit mpb->qpopup->aboutToHide();
            return true;
        }
    }
    return false;
}

/*!
    \internal
*/
void Q3MenuBar::macWidgetChangedWindow()
{
    int was_eaten = mac_eaten_menubar;
    macRemoveNativeMenubar();
    macCreateNativeMenubar();
    if(was_eaten)
        menuContentsChanged();
}

#endif //!defined(QMAC_Q3MENUBAR_NO_NATIVE)
#endif //WS_MAC
