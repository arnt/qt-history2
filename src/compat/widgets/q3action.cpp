/****************************************************************************
**
** Implementation of Q3Action class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "q3action.h"

#ifndef QT_NO_ACTION

#include "qevent.h"
#include "qtoolbar.h"
#include "qlist.h"
#include "q3popupmenu.h"
#include "qpopupmenu.h"
#include "qaccel.h"
#include "qtoolbutton.h"
#include "qcombobox.h"
#include "qtooltip.h"
#include "qwhatsthis.h"
#include "qstatusbar.h"
#include "qaction.h"

/*!
    \class Q3Action q3action.h
    \brief The Q3Action class provides an abstract user interface
    action that can appear both in menus and tool bars.

    \ingroup basic
    \ingroup application
    \mainclass

    In GUI applications many commands can be invoked via a menu
    option, a toolbar button and a keyboard accelerator. Since the
    same action must be performed regardless of how the action was
    invoked, and since the menu and toolbar should be kept in sync, it
    is useful to represent a command as an \e action. An action can be
    added to a menu and a toolbar and will automatically keep them in
    sync. For example, if the user presses a Bold toolbar button the
    Bold menu item will automatically be checked.

    A Q3Action may contain an icon, a menu text, an accelerator, a
    status text, a "What's This?" text and a tool tip. Most of these can
    be set in the constructor. They can also be set independently with
    setIconSet(), setText(), setMenuText(), setToolTip(),
    setStatusTip(), setWhatsThis() and setAccel().

    An action may be a toggle action e.g. a Bold toolbar button, or a
    command action, e.g. 'Open File' to invoke an open file dialog.
    Toggle actions emit the toggled() signal when their state changes.
    Both command and toggle actions emit the activated() signal when
    they are invoked. Use setToggleAction() to set an action's toggled
    status. To see if an action is a toggle action use
    isToggleAction(). A toggle action may be "on", isOn() returns
    true, or "off", isOn() returns false.

    Actions are added to widgets (menus or toolbars) using addTo(),
    and removed using removeFrom().

    Once a Q3Action has been created it should be added to the relevant
    menu and toolbar and then connected to the slot which will perform
    the action. For example:

    \quotefile action/application.cpp
    \skipto QPixmap(fileopen
    \printuntil connect

    We create a "File Save" action with a menu text of "&Save" and
    \e{Ctrl+S} as the keyboard accelerator. We connect the
    fileSaveAction's activated() signal to our own save() slot. Note
    that at this point there is no menu or toolbar action, we'll add
    them next:

    \skipto new QToolBar
    \printline
    \skipto fileSaveAction->addTo
    \printline
    \skipto new Q3PopupMenu
    \printuntil insertItem
    \skipto fileSaveAction->addTo
    \printline

    We create a toolbar and add our fileSaveAction to it. Similarly we
    create a menu, add a top-level menu item, and add our
    fileSaveAction.

    We recommend that actions are created as children of the window
    that they are used in. In most cases actions will be children of
    the application's main window.

    To prevent recursion, don't create an action as a child of a
    widget that the action is later added to.
*/

class Q3ActionPrivate
{
public:
    Q3ActionPrivate(Q3Action *act);
    ~Q3ActionPrivate();
    QIconSet *iconset;
    QString text;
    QString menutext;
    QString tooltip;
    QString statustip;
    QString whatsthis;
#ifndef QT_NO_ACCEL
    QKeySequence key;
    QAccel* accel;
    int accelid;
#endif
    uint enabled : 1;
    uint visible : 1;
    uint toggleaction : 1;
    uint on : 1;
    uint forceDisabled : 1;
    uint forceInvisible : 1;
    Q3ActionGroupPrivate* d_group;
    Q3Action *action;

    struct MenuItem {
        MenuItem():popup(0),id(0){}
        Q3PopupMenu* popup;
        int id;
    };
    // ComboItem is only necessary for actions that are
    // in dropdown/exclusive actiongroups. The actiongroup
    // will clean this up
    struct ComboItem {
        ComboItem():combo(0), id(0) {}
        QComboBox *combo;
        int id;
    };
    //just bindings to the Qt4.0 widgets
    struct Action4Item {
        Action4Item():widget(0){}
        QWidget* widget;
        static QAction *action;
    };
    QList<Action4Item *> action4items;
    QList<MenuItem *> menuitems;
    QList<QToolButton *> toolbuttons;
    QList<ComboItem *> comboitems;

    enum Update { Icons = 1, Visibility = 2, State = 4, EverythingElse = 8 };
    void update(uint upd = EverythingElse);

    QString menuText() const;
    QString toolTip() const;
    QString statusTip() const;
};
QAction *Q3ActionPrivate::Action4Item::action = 0;

Q3ActionPrivate::Q3ActionPrivate(Q3Action *act)
    : iconset(0),
#ifndef QT_NO_ACCEL
      key(0), accel(0), accelid(0),
#endif
      enabled(true), visible(true), toggleaction(false), on(false),
      forceDisabled(false), forceInvisible(false)
      , d_group(0), action(act)
{
}

Q3ActionPrivate::~Q3ActionPrivate()
{
    QList<QToolButton*>::Iterator ittb(toolbuttons.begin());
    QToolButton *tb;

    while (ittb != toolbuttons.end()) {
        tb = *ittb;
        ++ittb;
        delete tb;
    }

    QList<Q3ActionPrivate::MenuItem*>::Iterator itmi(menuitems.begin());
    Q3ActionPrivate::MenuItem* mi;
    while (itmi != menuitems.end()) {
        mi = *itmi;
        ++itmi;
        Q3PopupMenu* menu = mi->popup;
        if (menu->findItem(mi->id))
            menu->removeItem(mi->id);
    }

    QList<Q3ActionPrivate::Action4Item*>::Iterator itmi4(action4items.begin());
    Q3ActionPrivate::Action4Item* mi4;
    while (itmi != menuitems.end()) {
        mi4 = *itmi4;
        ++itmi4;
        mi4->widget->removeAction(mi4->action);
    }
    delete Q3ActionPrivate::Action4Item::action;
    Q3ActionPrivate::Action4Item::action = 0;

    QList<Q3ActionPrivate::ComboItem*>::Iterator itci(comboitems.begin());
    Q3ActionPrivate::ComboItem* ci;
    while (itci != comboitems.end()) {
        ci = *itci;
        ++itci;
        QComboBox* combo = ci->combo;
        combo->clear();
        Q3ActionGroup *group = ::qt_cast<Q3ActionGroup*>(action->parent());
        if (group) {
            QObjectList siblings = group->queryList("Q3Action");

            for (int i = 0; i < siblings.size(); ++i) {
                Q3Action *sib = ::qt_cast<Q3Action*>(siblings.at(i));
                sib->removeFrom(combo);
            }
            for (int i = 0; i < siblings.size(); ++i) {
                Q3Action *sib = ::qt_cast<Q3Action*>(siblings.at(i));
                if (sib == action)
                    continue;
                sib->addTo(combo);
            }
        }
    }

#ifndef QT_NO_ACCEL
    delete accel;
#endif
    delete iconset;
}

class Q3ActionGroupPrivate
{
public:
    uint exclusive: 1;
    uint dropdown: 1;
    QList<Q3Action*> actions;
    Q3Action* selected;
    Q3Action* separatorAction;

    struct MenuItem {
        MenuItem():popup(0),id(0){}
        Q3PopupMenu* popup;
        int id;
    };
    struct Action4Item {
        Action4Item():widget(0){}
        QWidget* widget;
        static QAction *action;
    };
    QList<Action4Item *> action4items;
    QList<QComboBox*> comboboxes;
    QList<QToolButton*> menubuttons;
    QList<MenuItem*> menuitems;
    QList<Q3PopupMenu*> popupmenus;

    void update(const Q3ActionGroup *);
};
QAction *Q3ActionGroupPrivate::Action4Item::action = 0;

void Q3ActionPrivate::update(uint upd)
{
    for (QList<MenuItem*>::Iterator it(menuitems.begin()); it != menuitems.end(); ++it) {
        MenuItem* mi = *it;
        QString t = menuText();
#ifndef QT_NO_ACCEL
        if (key)
            t += '\t' + (QString)QKeySequence(key);
#endif
        if (upd & State) {
            mi->popup->setItemEnabled(mi->id, enabled);
            if (toggleaction)
                mi->popup->setItemChecked(mi->id, on);
        }
        if (upd & Visibility)
            mi->popup->setItemVisible(mi->id, visible);

        if (upd & Icons)
            if (iconset)
                mi->popup->changeItem(mi->id, *iconset, t);
            else
                mi->popup->changeItem(mi->id, QIconSet(), t);
        if (upd & EverythingElse) {
            mi->popup->changeItem(mi->id, t);
            if (!whatsthis.isEmpty())
                    mi->popup->setWhatsThis(mi->id, whatsthis);
            if (toggleaction) {
                mi->popup->setCheckable(true);
                mi->popup->setItemChecked(mi->id, on);
            }
        }
    }
    if(QAction *act = Action4Item::action) {
        if (upd & Visibility)
            act->setVisible(visible);
        if (upd & Icons) {
            if (iconset)
                act->setIcon(*iconset);
            else
                act->setIcon(QIconSet());
        }
        if (upd & EverythingElse) {
            QString text = action->menuText();
#ifndef QT_NO_ACCEL
            if (key)
                text += '\t' + (QString)QKeySequence(key);
#endif
            act->setText(text);
            act->setToolTip(statusTip());
            act->setWhatsThis(whatsthis);
        }
    }
    for (QList<QToolButton*>::Iterator it2(toolbuttons.begin()); it2 != toolbuttons.end(); ++it2) {
        QToolButton* btn = *it2;
        if (upd & State) {
            btn->setEnabled(enabled);
            if (toggleaction)
                btn->setOn(on);
        }
        if (upd & Visibility)
            visible ? btn->show() : btn->hide();
        if (upd & Icons) {
            if (iconset)
                btn->setIconSet(*iconset);
            else
                btn->setIconSet(QIconSet());
        }
        if (upd & EverythingElse) {
            btn->setToggleButton(toggleaction);
            if (!text.isEmpty())
                btn->setTextLabel(text, false);
            btn->setToolTip(toolTip());
            btn->setStatusTip(statusTip());
#ifndef QT_NO_WHATSTHIS
            QWhatsThis::remove(btn);
            if (!whatsthis.isEmpty())
                QWhatsThis::add(btn, whatsthis);
#endif
        }
    }
#ifndef QT_NO_ACCEL
    if (accel) {
        accel->setEnabled(enabled && visible);
        if (!whatsthis.isEmpty())
            accel->setWhatsThis(accelid, whatsthis);
    }
#endif
    // Only used by actiongroup
    for (QList<ComboItem*>::Iterator it3(comboitems.begin()); it3 != comboitems.end(); ++it3) {
        ComboItem *ci = *it3;
        if (!ci->combo)
            return;
        if (iconset)
            ci->combo->changeItem(iconset->pixmap(), text, ci->id);
        else
            ci->combo->changeItem(text, ci->id);
    }
}

QString Q3ActionPrivate::menuText() const
{
    if (menutext.isNull()) {
        QString t(text);
        t.replace('&', "&&");
        return t;
    }
    return menutext;
}

QString Q3ActionPrivate::toolTip() const
{
    if (tooltip.isNull()) {
#ifndef QT_NO_ACCEL
        if (accel)
            return text + " (" + (QString)QKeySequence(accel->key(accelid)) + ")";
#endif
        return text;
    }
    return tooltip;
}

QString Q3ActionPrivate::statusTip() const
{
    if (statustip.isNull())
        return toolTip();
    return statustip;
}

/*
  internal: guesses a descriptive text from a menu text
 */
static QString qt_stripMenuText(QString s)
{
    s.remove(QLatin1String("..."));
    s.remove(QChar('&'));
    return s.trimmed();
};

/*!
    Constructs an action called \a name with parent \a parent.

    If \a parent is a Q3ActionGroup, the new action inserts itself into
    \a parent.

    For accelerators and status tips to work, \a parent must either be
    a widget, or an action group whose parent is a widget.

    \warning To prevent recursion, don't create an action as a child
    of a widget that the action is later added to.
*/
Q3Action::Q3Action(QObject* parent, const char* name)
    : QObject(parent, name)
{
    d = new Q3ActionPrivate(this);
    init();
}

/*! \obsolete
    Constructs an action called \a name with parent \a parent.

    If \a toggle is true the action will be a toggle action, otherwise
    it will be a command action.

    If \a parent is a Q3ActionGroup, the new action inserts itself into
    \a parent.

    For accelerators and status tips to work, \a parent must either be
    a widget, or an action group whose parent is a widget.
*/
Q3Action::Q3Action(QObject* parent, const char* name, bool toggle)
    : QObject(parent, name)
{
    d = new Q3ActionPrivate(this);
    d->toggleaction = toggle;
    init();
}


#ifndef QT_NO_ACCEL

/*!
    This constructor creates an action with the following properties:
    the icon or iconset \a icon, the menu text \a menuText and
    keyboard accelerator \a accel. It is a child of \a parent and
    called \a name.

    If \a parent is a Q3ActionGroup, the action automatically becomes
    a member of it.

    For accelerators and status tips to work, \a parent must either be
    a widget, or an action group whose parent is a widget.

    The action uses a stripped version of \a menuText (e.g. "\&Menu
    Option..." becomes "Menu Option") as descriptive text for
    toolbuttons. You can override this by setting a specific
    description with setText(). The same text and \a accel will be
    used for tool tips and status tips unless you provide text for
    these using setToolTip() and setStatusTip().

    Call setToggleAction(true) to make the action a toggle action.

    \warning To prevent recursion, don't create an action as a child
    of a widget that the action is later added to.
*/
Q3Action::Q3Action(const QIconSet& icon, const QString& menuText, QKeySequence accel,
                  QObject* parent, const char* name)
    : QObject(parent, name)
{
    d = new Q3ActionPrivate(this);
    if (!icon.isNull())
        setIconSet(icon);
    d->text = qt_stripMenuText(menuText);
    d->menutext = menuText;
    setAccel(accel);
    init();
}

/*!
    This constructor results in an icon-less action with the the menu
    text \a menuText and keyboard accelerator \a accel. It is a child
    of \a parent and called \a name.

    If  \a parent is a Q3ActionGroup, the action automatically becomes
    a member of it.

    For accelerators and status tips to work, \a parent must either be
    a widget, or an action group whose parent is a widget.

    The action uses a stripped version of \a menuText (e.g. "\&Menu
    Option..." becomes "Menu Option") as descriptive text for
    toolbuttons. You can override this by setting a specific
    description with setText(). The same text and \a accel will be
    used for tool tips and status tips unless you provide text for
    these using setToolTip() and setStatusTip().

    Call setToggleAction(true) to make the action a toggle action.

    \warning To prevent recursion, don't create an action as a child
    of a widget that the action is later added to.
*/
Q3Action::Q3Action(const QString& menuText, QKeySequence accel,
                  QObject* parent, const char* name)
    : QObject(parent, name)
{
    d = new Q3ActionPrivate(this);
    d->text = qt_stripMenuText(menuText);
    d->menutext = menuText;
    setAccel(accel);
    init();
}

/*! \obsolete
    This constructor creates an action with the following properties:
    the description \a text, the icon or iconset \a icon, the menu
    text \a menuText and keyboard accelerator \a accel. It is a child
    of \a parent and called \a name. If \a toggle is true the action
    will be a toggle action, otherwise it will be a command action.

    If  \a parent is a Q3ActionGroup, the action automatically becomes
    a member of it.

    For accelerators and status tips to work, \a parent must either be
    a widget, or an action group whose parent is a widget.

    The \a text and \a accel will be used for tool tips and status
    tips unless you provide specific text for these using setToolTip()
    and setStatusTip().
*/
Q3Action::Q3Action(const QString& text, const QIconSet& icon, const QString& menuText, QKeySequence accel, QObject* parent, const char* name, bool toggle)
    : QObject(parent, name)
{
    d = new Q3ActionPrivate(this);
    d->toggleaction = toggle;
    if (!icon.isNull())
        setIconSet(icon);

    d->text = text;
    d->menutext = menuText;
    setAccel(accel);
    init();
}

/*! \obsolete
    This constructor results in an icon-less action with the
    description \a text, the menu text \a menuText and the keyboard
    accelerator \a accel. Its parent is \a parent and it is called \a
    name. If \a toggle is true the action will be a toggle action,
    otherwise it will be a command action.

    The action automatically becomes a member of \a parent if \a
    parent is a Q3ActionGroup.

    For accelerators and status tips to work, \a parent must either be
    a widget, or an action group whose parent is a widget.

    The \a text and \a accel will be used for tool tips and status
    tips unless you provide specific text for these using setToolTip()
    and setStatusTip().
*/
Q3Action::Q3Action(const QString& text, const QString& menuText, QKeySequence accel, QObject* parent, const char* name, bool toggle)
    : QObject(parent, name)
{
    d = new Q3ActionPrivate(this);
    d->toggleaction = toggle;
    d->text = text;
    d->menutext = menuText;
    setAccel(accel);
    init();
}
#endif

/*!
  \internal
*/
void Q3Action::init()
{
    if (qt_cast<Q3ActionGroup*>(parent()))
        ((Q3ActionGroup*) parent())->add(this);                // insert into action group
}

/*!
    Destroys the object and frees allocated resources.
*/

Q3Action::~Q3Action()
{
    delete d;
}

/*!
    \property Q3Action::iconSet
    \brief  the action's icon

    The icon is used as the tool button icon and in the menu to the
    left of the menu text. There is no default icon.

    If a null icon (QIconSet::isNull() is passed into this function,
    the icon of the action is cleared.

    (See the action/toggleaction/toggleaction.cpp example.)

*/
void Q3Action::setIconSet(const QIconSet& icon)
{
    register QIconSet *i = d->iconset;
    if (!icon.isNull())
        d->iconset = new QIconSet(icon);
    else
        d->iconset = 0;
    delete i;
    d->update(Q3ActionPrivate::Icons);
}

QIconSet Q3Action::iconSet() const
{
    if (d->iconset)
        return *d->iconset;
    return QIconSet();
}

/*!
    \property Q3Action::text
    \brief the action's descriptive text

    If \l QMainWindow::usesTextLabel is true, the text appears as a
    label in the relevant tool button. It also serves as the default
    text in menus and tool tips if these have not been specifically
    defined. There is no default text.

    \sa setMenuText() setToolTip() setStatusTip()
*/
void Q3Action::setText(const QString& text)
{
    d->text = text;
    d->update();
}

QString Q3Action::text() const
{
    return d->text;
}


/*!
    \property Q3Action::menuText
    \brief the action's menu text

    If the action is added to a menu the menu option will consist of
    the icon (if there is one), the menu text and the accelerator (if
    there is one). If the menu text is not explicitly set in the
    constructor or by using setMenuText() the action's description
    text will be used as the menu text. There is no default menu text.

    \sa text
*/
void Q3Action::setMenuText(const QString& text)
{
    if (d->menutext == text)
        return;

    d->menutext = text;
    d->update();
}

QString Q3Action::menuText() const
{
    return d->menuText();
}

/*!
    \property Q3Action::toolTip
    \brief the action's tool tip

    This text is used for the tool tip. If no status tip has been set
    the tool tip will be used for the status tip.

    If no tool tip is specified the action's text is used, and if that
    hasn't been specified the description text is used as the tool tip
    text.

    There is no default tool tip text.

    \sa setStatusTip() setAccel()
*/
void Q3Action::setToolTip(const QString& tip)
{
    if (d->tooltip == tip)
        return;

    d->tooltip = tip;
    d->update();
}

QString Q3Action::toolTip() const
{
    return d->toolTip();
}

/*!
    \property Q3Action::statusTip
    \brief the action's status tip

    The statusTip is displayed on all status bars that this action's
    top-level parent widget provides, and is provided as the parameter
    of the showStatusMessage() signal.

    If no status tip is defined, the action uses the tool tip text.

    There is no default statusTip text.

    \sa setStatusTip() setToolTip() showStatusMessage()
*/
//#### Please reimp for Q3ActionGroup!
//#### For consistency reasons even action groups should show
//#### status tips (as they already do with tool tips)
//#### Please change Q3ActionGroup class doc appropriately after
//#### reimplementation.
void Q3Action::setStatusTip(const QString& tip)
{
    if (d->statustip == tip)
        return;

    d->statustip = tip;
    d->update();
}

QString Q3Action::statusTip() const
{
    return d->statusTip();
}

/*!
    \property Q3Action::whatsThis
    \brief the action's "What's This?" help text

    The whats this text is used to provide a brief description of the
    action. The text may contain rich text (HTML-like tags -- see
    QStyleSheet for the list of supported tags). There is no default
    "What's This?" text.

    If the "What's This?" text contains a hyperlink the whatsThisClicked()
    signal is emitted when the user clicks inside the "What's This?"
    window.

    \sa QWhatsThis whatsThisClicked()
*/
void Q3Action::setWhatsThis(const QString& whatsThis)
{
    if (d->whatsthis == whatsThis)
        return;
    d->whatsthis = whatsThis;
    d->update();
}

QString Q3Action::whatsThis() const
{
    return d->whatsthis;
}


#ifndef QT_NO_ACCEL
/*!
    \property Q3Action::accel
    \brief the action's accelerator key

    The keycodes can be found in \l Qt::Key and \l Qt::Modifier. There
    is no default accelerator key.
*/
//#### Please reimp for Q3ActionGroup!
//#### For consistency reasons even Q3ActionGroups should respond to
//#### their accelerators and e.g. open the relevant submenu.
//#### Please change appropriate Q3ActionGroup class doc after
//#### reimplementation.
void Q3Action::setAccel(const QKeySequence& key)
{
    if (d->key == key)
        return;

    d->key = key;
    delete d->accel;
    d->accel = 0;

    if (!(int)key) {
        d->update();
        return;
    }

    QObject* p = parent();
    while (p && !p->isWidgetType()) {
        p = p->parent();
    }
    if (p) {
        d->accel = new QAccel((QWidget*)p, this, "qt_action_accel");
        d->accelid = d->accel->insertItem(d->key);
        d->accel->connectItem(d->accelid, this, SLOT(internalActivation()));
    } else
        qWarning("Q3Action::setAccel() (%s) requires widget in parent chain", objectName().local8Bit());
    d->update();
}


QKeySequence Q3Action::accel() const
{
    return d->key;
}
#endif


/*!
    \property Q3Action::toggleAction
    \brief whether the action is a toggle action

    A toggle action is one which has an on/off state. For example a
    Bold toolbar button is either on or off. An action which is not a
    toggle action is a command action; a command action is simply
    executed, e.g. file save. This property's default is false.

    In some situations, the state of one toggle action should depend
    on the state of others. For example, "Left Align", "Center" and
    "Right Align" toggle actions are mutually exclusive. To achieve
    exclusive toggling, add the relevant toggle actions to a
    Q3ActionGroup with the \l Q3ActionGroup::exclusive property set to
    true.
*/
void Q3Action::setToggleAction(bool enable)
{
    if (enable == (bool)d->toggleaction)
        return;

    if (!enable)
        d->on = false;

    d->toggleaction = enable;
    d->update();
}

bool Q3Action::isToggleAction() const
{
    return d->toggleaction;
}

/*!
    Activates the action and executes all connected slots.
    This only works for actions that are not toggle actions.

    \sa toggle()
*/
void Q3Action::activate()
{
    if (isToggleAction()) {
#if defined(QT_CHECK_STATE)
        qWarning("Q3Action::%s() (%s) Toggle actions "
                  "can not be activated", "activate", objectName().local8Bit());
#endif
        return;
    }
    emit activated();
}

/*!
    Toggles the state of a toggle action.

    \sa on, activate(), toggled(), isToggleAction()
*/
void Q3Action::toggle()
{
    if (!isToggleAction()) {
        qWarning("Q3Action::%s() (%s) Only toggle actions "
                  "can be switched", "toggle", objectName().local8Bit());
        return;
    }
    setOn(!isOn());
}

/*!
    \property Q3Action::on
    \brief whether a toggle action is on

    This property is always on (true) for command actions and
    \l{Q3ActionGroup}s; setOn() has no effect on them. For action's
    where isToggleAction() is true, this property's default value is
    off (false).

    \sa toggleAction
*/
void Q3Action::setOn(bool enable)
{
    if (!isToggleAction()) {
        if (enable)
            qWarning("Q3Action::%s() (%s) Only toggle actions "
                      "can be switched", "setOn", objectName().local8Bit());
        return;
    }
    if (enable == (bool)d->on)
        return;
    d->on = enable;
    d->update(Q3ActionPrivate::State);
    emit toggled(enable);
}

bool Q3Action::isOn() const
{
    return d->on;
}

/*!
    \property Q3Action::enabled
    \brief whether the action is enabled

    Disabled actions can't be chosen by the user. They don't disappear
    from the menu/tool bar but are displayed in a way which indicates
    that they are unavailable, e.g. they might be displayed grayed
    out.

    What's this? help on disabled actions is still available provided
    the \l Q3Action::whatsThis property is set.
*/
void Q3Action::setEnabled(bool enable)
{
    d->forceDisabled = !enable;

    if ((bool)d->enabled == enable)
        return;

    d->enabled = enable;
    d->update(Q3ActionPrivate::State);
}

bool Q3Action::isEnabled() const
{
    return d->enabled;
}

/*!
    Disables the action if \a disable is true; otherwise
    enables the action.

    See the \l enabled documentation for more information.
*/
void Q3Action::setDisabled(bool disable)
{
    setEnabled(!disable);
}

/*!
    \property Q3Action::visible
    \brief whether the action can be seen (e.g. in menus and toolbars)

    If \e visible is true the action can be seen (e.g. in menus and
    toolbars) and chosen by the user; if \e visible is false the
    action cannot be seen or chosen by the user.

    Actions which are not visible are \e not grayed out; they do not
    appear at all.
*/
void Q3Action::setVisible(bool visible)
{
    d->forceInvisible = !visible;

    if ((bool)d->visible == visible)
        return;

    d->visible = visible;
    d->update(Q3ActionPrivate::Visibility);
}

/*
    Returns true if the action is visible (e.g. in menus and
    toolbars); otherwise returns false.
*/
bool Q3Action::isVisible() const
{
    return d->visible;
}

/*! \internal
*/
void Q3Action::internalActivation()
{
    if (isToggleAction())
        setOn(!isOn());
    emit activated();
}

/*! \internal
*/
void Q3Action::toolButtonToggled(bool on)
{
    if (!isToggleAction())
        return;
    setOn(on);
}

/*!
    Adds this action to widget \a w.

    Currently actions may be added to QToolBar and Q3PopupMenu widgets.

    An action added to a tool bar is automatically displayed as a tool
    button; an action added to a pop up menu appears as a menu option.

    addTo() returns true if the action was added successfully and
    false otherwise. (If \a w is not a QToolBar or Q3PopupMenu the
    action will not be added and false will be returned.)

    \sa removeFrom()
*/
bool Q3Action::addTo(QWidget* w)
{
#ifndef QT_NO_TOOLBAR
    if (::qt_cast<QToolBar*>(w)) {
        if (objectName() == QLatin1String("qt_separator_action")) {
            ((QToolBar*)w)->addSeparator();
        } else {
            QString bname = objectName() + QLatin1String("_action_button");
            QToolButton* btn = new QToolButton((QToolBar*) w);
            btn->setObjectName(bname);
            addedTo(btn, w);
            btn->setToggleButton(d->toggleaction);
            d->toolbuttons.append(btn);
            if (d->iconset)
                btn->setIconSet(*d->iconset);
            d->update(Q3ActionPrivate::State | Q3ActionPrivate::Visibility | Q3ActionPrivate::EverythingElse) ;
            connect(btn, SIGNAL(clicked()), this, SIGNAL(activated()));
            connect(btn, SIGNAL(toggled(bool)), this, SLOT(toolButtonToggled(bool)));
            connect(btn, SIGNAL(destroyed()), this, SLOT(objectDestroyed()));
        }
    } else
#endif
    if (qt_cast<Q3PopupMenu*>(w)) {
        Q3ActionPrivate::MenuItem* mi = new Q3ActionPrivate::MenuItem;
        mi->popup = (Q3PopupMenu*) w;
        QIconSet* diconset = d->iconset;
        if (objectName() == QLatin1String("qt_separator_action"))
            mi->id = ((Q3PopupMenu*)w)->insertSeparator();
        else if (diconset)
            mi->id = mi->popup->insertItem(*diconset, QString::fromLatin1(""));
        else
            mi->id = mi->popup->insertItem(QString::fromLatin1(""));
        addedTo(mi->popup->indexOf(mi->id), mi->popup);
        mi->popup->connectItem(mi->id, this, SLOT(internalActivation()));
        d->menuitems.append(mi);
        d->update(Q3ActionPrivate::State | Q3ActionPrivate::Visibility | Q3ActionPrivate::EverythingElse);
        connect(mi->popup, SIGNAL(highlighted(int)), this, SLOT(menuStatusText(int)));
        connect(mi->popup, SIGNAL(aboutToHide()), this, SLOT(clearStatusText()));
        connect(mi->popup, SIGNAL(destroyed()), this, SLOT(objectDestroyed()));
    // Makes only sense when called by Q3ActionGroup::addTo
    } else if (qt_cast<QComboBox*>(w)) {
        Q3ActionPrivate::ComboItem *ci = new Q3ActionPrivate::ComboItem;
        ci->combo = (QComboBox*)w;
        connect(ci->combo, SIGNAL(destroyed()), this, SLOT(objectDestroyed()));
        ci->id = ci->combo->count();
        if (objectName() == QLatin1String("qt_separator_action")) {
            if (d->iconset)
                ci->combo->insertItem(d->iconset->pixmap(), text());
            else
                ci->combo->insertItem(text());
        } else {
            ci->id = -1;
        }
        d->comboitems.append(ci);
        d->update(Q3ActionPrivate::State | Q3ActionPrivate::EverythingElse);
    } else if(qt_cast<QMenu*>(w)) {
        Q3ActionPrivate::Action4Item *act = new Q3ActionPrivate::Action4Item;
        if(!act->action) { //static
            act->action = new QAction;
            if (objectName() == QLatin1String("qt_separator_action"))
                act->action->setSeparator(true);
        }
        act->widget = w;
        act->widget->addAction(act->action);
        d->action4items.append(act);
        d->update(Q3ActionPrivate::State | Q3ActionPrivate::EverythingElse);
    } else {
        qWarning("Q3Action::addTo(), unknown object");
        return false;
    }
    return true;
}

/*!
    This function is called from the addTo() function when it has
    created a widget (\a actionWidget) for the action in the \a
    container.
*/

void Q3Action::addedTo(QWidget *actionWidget, QWidget *container)
{
    Q_UNUSED(actionWidget);
    Q_UNUSED(container);
}

/*!
    \overload

    This function is called from the addTo() function when it has
    created a menu item at the index position \a index in the popup
    menu \a menu.
*/

void Q3Action::addedTo(int index, Q3PopupMenu *menu)
{
    Q_UNUSED(index);
    Q_UNUSED(menu);
}

/*!
    Sets the status message to \a text
*/
void Q3Action::showStatusText(const QString& text)
{
#ifndef QT_NO_STATUSBAR
    // find out whether we are clearing the status bar by the popup that actually set the text
    static Q3PopupMenu *lastmenu = 0;
    QObject *s = (QObject*)sender();
    if (s) {
        Q3PopupMenu *menu = qt_cast<Q3PopupMenu*>(s);
        if (menu && text.size())
            lastmenu = menu;
        else if (menu && text.isEmpty()) {
            if (lastmenu && menu != lastmenu)
                return;
            lastmenu = 0;
        }
    }

    QObject* par = parent();
    QObject* lpar = 0;
    QStatusBar *bar = 0;
    while (par && !bar) {
        lpar = par;
        bar = (QStatusBar*)par->child(0, "QStatusBar", false);
        par = par->parent();
    }
    if (!bar && lpar) {
        QObjectList l = lpar->queryList("QStatusBar");
        if (l.isEmpty())
            return;
        // #### hopefully the last one is the one of the mainwindow...
        bar = static_cast<QStatusBar*>(l.at(l.size()-1));
    }
    if (bar) {
        if (text.isEmpty())
            bar->clear();
        else
            bar->message(text);
    }
#endif
}

/*!
    Sets the status message to the menu item's status text, or to the
    tooltip, if there is no status text.
*/
void Q3Action::menuStatusText(int id)
{
    static int lastId = 0;
    QString text;
    QList<Q3ActionPrivate::MenuItem*>::Iterator it(d->menuitems.begin());
    while (it != d->menuitems.end()) {
        if ((*it)->id == id) {
            text = statusTip();
            break;
        }
        ++it;
    }

    if (!text.isEmpty())
        showStatusText(text);
    else if (id != lastId)
        clearStatusText();
    lastId = id;
}

/*!
    Clears the status text.
*/
void Q3Action::clearStatusText()
{
    if (!statusTip().isEmpty())
        showStatusText(QString::null);
}

/*!
    Removes the action from widget \a w.

    Returns true if the action was removed successfully; otherwise
    returns false.

    \sa addTo()
*/
bool Q3Action::removeFrom(QWidget* w)
{
#ifndef QT_NO_TOOLBAR
    if (::qt_cast<QToolBar*>(w)) {
        QList<QToolButton*>::Iterator it(d->toolbuttons.begin());
        QToolButton* btn;
        while (it != d->toolbuttons.end()) {
            btn = *it;
            ++it;
            if (btn->parentWidget() == w) {
                d->toolbuttons.removeAll(btn);
                disconnect(btn, SIGNAL(destroyed()), this, SLOT(objectDestroyed()));
                delete btn;
                // no need to disconnect from statusbar
            }
        }
    } else
#endif
    if (::qt_cast<Q3PopupMenu*>(w)) {
        QList<Q3ActionPrivate::MenuItem*>::Iterator it(d->menuitems.begin());
        Q3ActionPrivate::MenuItem* mi;
        while (it != d->menuitems.end()) {
            mi = *it;
            ++it;
            if (mi->popup == w) {
                disconnect(mi->popup, SIGNAL(highlighted(int)), this, SLOT(menuStatusText(int)));
                disconnect(mi->popup, SIGNAL(aboutToHide()), this, SLOT(clearStatusText()));
                disconnect(mi->popup, SIGNAL(destroyed()), this, SLOT(objectDestroyed()));
                mi->popup->removeItem(mi->id);
                d->menuitems.removeAll(mi);
                delete mi;
            }
        }
    } else if (::qt_cast<QComboBox*>(w)) {
        QList<Q3ActionPrivate::ComboItem*>::Iterator it(d->comboitems.begin());
        Q3ActionPrivate::ComboItem *ci;
        while (it != d->comboitems.end()) {
            ci = *it;
            ++it;
            if (ci->combo == w) {
                disconnect(ci->combo, SIGNAL(destroyed()), this, SLOT(objectDestroyed()));
                d->comboitems.removeAll(ci);
                delete ci;
            }
        }
    } else if (::qt_cast<QMenu*>(w)) {
        QList<Q3ActionPrivate::Action4Item*>::Iterator it(d->action4items.begin());
        Q3ActionPrivate::Action4Item *a4i;
        while (it != d->action4items.end()) {
            a4i = *it;
            ++it;
            if (a4i->widget == w) {
                a4i->widget->removeAction(a4i->action);
                d->action4items.removeAll(a4i);
                delete a4i;
            }
        }
    } else {
        qWarning("Q3Action::removeFrom(), unknown object");
        return false;
    }
    return true;
}

/*!
  \internal
*/
void Q3Action::objectDestroyed()
{
    const QObject* obj = sender();
    Q3ActionPrivate::MenuItem* mi;
    for (int i = 0; i < d->menuitems.size();) {
        mi = d->menuitems.at(i);
        ++i;
        if (mi->popup == obj) {
            d->menuitems.removeAll(mi);
            delete mi;
        }
    }
    Q3ActionPrivate::ComboItem *ci;
    QList<Q3ActionPrivate::ComboItem*>::Iterator it2(d->comboitems.begin());
    while (it2 != d->comboitems.end()) {
        ci = *it2;
        ++it2;
        if (ci->combo == obj) {
            d->comboitems.removeAll(ci);
            delete ci;
        }
    }
    d->toolbuttons.removeAll((QToolButton *)obj);
}

/*!
    \fn void Q3Action::activated()

    This signal is emitted when an action is activated by the user,
    e.g. when the user clicks a menu option or a toolbar button or
    presses an action's accelerator key combination.

    Connect to this signal for command actions. Connect to the
    toggled() signal for toggle actions.
*/

/*!
    \fn void Q3Action::toggled(bool on)

    This signal is emitted when a toggle action changes state; command
    actions and \l{Q3ActionGroup}s don't emit toggled().

    The \a on argument denotes the new state: If \a on is true the
    toggle action is switched on, and if \a on is false the toggle
    action is switched off.

    To trigger a user command depending on whether a toggle action has
    been switched on or off connect it to a slot that takes a bool to
    indicate the state, e.g.

    \quotefile action/toggleaction/toggleaction.cpp
    \skipto QMainWindow * window
    \printline QMainWindow * window
    \skipto labelonoffaction
    \printline labelonoffaction
    \skipto connect
    \printuntil setUsesTextLabel

    \sa activated() setToggleAction() setOn()
*/

/*!
    \fn void Q3Action::showStatusMessage(const QString &text)

    This signal is emitted before \a text is displayed in the
    application's status bar. \a text can be QString::null when the
    status bar is cleared.

    Connect to this signal to provide additional handling of
    the status messages.

*/

/*!
    \fn void Q3Action::whatsThisClicked(const QString &href)

    If the whatsThis property of this action includes a hyperlink
    this signal is emitted when the user clicks inside the
    "What's This?" window. \a href is the link the user clicked on,
    or QString::null if there was no link.

    \sa QWhatsThis
*/

void Q3ActionGroupPrivate::update(const Q3ActionGroup* that)
{
    for (QList<Q3Action*>::Iterator it(actions.begin()); it != actions.end(); ++it) {
        if (that->isEnabled() && !(*it)->d->forceDisabled)
            (*it)->setEnabled(true);
        else if (!that->isEnabled() && (*it)->isEnabled()) {
            (*it)->setEnabled(false);
            (*it)->d->forceDisabled = false;
        }
	if (that->isVisible() && !(*it)->d->forceInvisible) {
	    (*it)->setVisible(true);
	} else if (!that->isVisible() && (*it)->isVisible()) {
	    (*it)->setVisible(false);
	    (*it)->d->forceInvisible = false;
	}
    }
    for (QList<QComboBox*>::Iterator cb(comboboxes.begin()); cb != comboboxes.end(); ++cb) {
        QComboBox *combobox = *cb;
        combobox->setEnabled(that->isEnabled());
        combobox->setShown(that->isVisible());

#ifndef QT_NO_TOOLTIP
        QToolTip::remove(combobox);
        if (that->toolTip().size())
            QToolTip::add(combobox, that->toolTip());
#endif
#ifndef QT_NO_WHATSTHIS
        QWhatsThis::remove(combobox);
        if (that->whatsThis().size())
            QWhatsThis::add(combobox, that->whatsThis());
#endif

    }
    for (QList<QToolButton*>::Iterator mb(menubuttons.begin()); mb != menubuttons.end(); ++mb) {
        QToolButton *button = *mb;
        button->setEnabled(that->isEnabled());
        button->setShown(that->isVisible());

        if (!that->text().isNull())
            button->setTextLabel(that->text());
        if (!that->iconSet().isNull())
            button->setIconSet(that->iconSet());

#ifndef QT_NO_TOOLTIP
        QToolTip::remove(*mb);
        if (that->toolTip().size())
            QToolTip::add(button, that->toolTip());
#endif
#ifndef QT_NO_WHATSTHIS
        QWhatsThis::remove(button);
        if (that->whatsThis().size())
            QWhatsThis::add(button, that->whatsThis());
#endif
    }
    if(QAction *act = Q3ActionGroupPrivate::Action4Item::action) {
        act->setVisible(that->isVisible());
        act->setEnabled(that->isEnabled());
    }
    for (QList<Q3ActionGroupPrivate::MenuItem*>::Iterator pu(menuitems.begin()); pu != menuitems.end(); ++pu) {
        QWidget* parent = (*pu)->popup->parentWidget();
        if (::qt_cast<Q3PopupMenu*>(parent)) {
            Q3PopupMenu* ppopup = (Q3PopupMenu*)parent;
            ppopup->setItemEnabled((*pu)->id, that->isEnabled());
            ppopup->setItemVisible((*pu)->id, that->isVisible());
        } else {
            (*pu)->popup->setEnabled(that->isEnabled());
        }
    }
    for (QList<Q3PopupMenu*>::Iterator pm(popupmenus.begin()); pm != popupmenus.end(); ++pm) {
        Q3PopupMenu *popup = *pm;
        Q3PopupMenu *parent = ::qt_cast<Q3PopupMenu*>(popup->parentWidget());
        if (!parent)
            continue;

        int index;
        parent->findPopup(popup, &index);
        int id = parent->idAt(index);
        if (!that->iconSet().isNull())
            parent->changeItem(id, that->iconSet(), that->menuText());
        else
            parent->changeItem(id, that->menuText());
        parent->setItemEnabled(id, that->isEnabled());
#ifndef QT_NO_ACCEL
        parent->setAccel(that->accel(), id);
#endif
    }
}

/*!
    \class Q3ActionGroup q3action.h
    \brief The Q3ActionGroup class groups actions together.

    \ingroup basic
    \ingroup application

    In some situations it is useful to group actions together. For
    example, if you have a left justify action, a right justify action
    and a center action, only one of these actions should be active at
    any one time, and one simple way of achieving this is to group the
    actions together in an action group.

    An action group can also be added to a menu or a toolbar as a
    single unit, with all the actions within the action group
    appearing as separate menu options and toolbar buttons.

    Here's an example from examples/textedit:
    \quotefile textedit/textedit.cpp
    \skipto Q3ActionGroup
    \printuntil connect

    Here we create a new action group. Since the action group is exclusive
    by default, only one of the actions in the group is ever active at any
    one time. We then connect the group's selected() signal to our
    textAlign() slot.

    \printuntil actionAlignLeft->setToggleAction

    We create a left align action, add it to the toolbar and the menu
    and make it a toggle action. We create center and right align
    actions in exactly the same way.

    \omit
    A Q3ActionGroup emits an activated() signal when one of its actions
    is activated.
    \endomit
    The actions in an action group emit their activated() (and for
    toggle actions, toggled()) signals as usual.

    The setExclusive() function is used to ensure that only one action
    is active at any one time: it should be used with actions which
    have their \c toggleAction set to true.

    Action group actions appear as individual menu options and toolbar
    buttons. For exclusive action groups use setUsesDropDown() to
    display the actions in a subwidget of any widget the action group
    is added to. For example, the actions would appear in a combobox
    in a toolbar or as a submenu in a menu.

    Actions can be added to an action group using add(), but normally
    they are added by creating the action with the action group as
    parent. Actions can have separators dividing them using
    addSeparator(). Action groups are added to widgets with addTo().
*/

/*!
    Constructs an action group called \a name, with parent \a parent.

    The action group is exclusive by default. Call setExclusive(false) to make
    the action group non-exclusive.
*/
Q3ActionGroup::Q3ActionGroup(QObject* parent, const char* name)
    : Q3Action(parent, name)
{
    d = new Q3ActionGroupPrivate;
    d->exclusive = true;
    d->dropdown = false;
    d->selected = 0;
    d->separatorAction = 0;

    connect(this, SIGNAL(selected(Q3Action*)), SLOT(internalToggle(Q3Action*)));
}

/*!
    Constructs an action group called \a name, with parent \a parent.

    If \a exclusive is true only one toggle action in the group will
    ever be active.

    \sa exclusive
*/
Q3ActionGroup::Q3ActionGroup(QObject* parent, const char* name, bool exclusive)
    : Q3Action(parent, name)
{
    d = new Q3ActionGroupPrivate;
    d->exclusive = exclusive;
    d->dropdown = false;
    d->selected = 0;
    d->separatorAction = 0;

    connect(this, SIGNAL(selected(Q3Action*)), SLOT(internalToggle(Q3Action*)));
}

/*!
    Destroys the object and frees allocated resources.
*/

Q3ActionGroup::~Q3ActionGroup()
{
    QList<Q3ActionGroupPrivate::MenuItem*>::Iterator mit(d->menuitems.begin());
    while (mit != d->menuitems.end()) {
        Q3ActionGroupPrivate::MenuItem *mi = *mit;
        ++mit;
        if (mi->popup)
            mi->popup->disconnect(SIGNAL(destroyed()), this, SLOT(objectDestroyed()));
    }

    QList<QComboBox*>::Iterator cbit(d->comboboxes.begin());
    while (cbit != d->comboboxes.end()) {
        QComboBox *cb = *cbit;
        ++cbit;
        cb->disconnect(SIGNAL(destroyed()), this, SLOT(objectDestroyed()));
    }
    QList<QToolButton*>::Iterator mbit(d->menubuttons.begin());
    while (mbit != d->menubuttons.end()) {
        QToolButton *mb = *mbit;
        ++mbit;
        mb->disconnect(SIGNAL(destroyed()), this, SLOT(objectDestroyed()));
    }
    QList<Q3PopupMenu*>::Iterator pmit(d->popupmenus.begin());
    while (pmit != d->popupmenus.end()) {
        Q3PopupMenu *pm = *pmit;
        ++pmit;
        pm->disconnect(SIGNAL(destroyed()), this, SLOT(objectDestroyed()));
    }

    QList<Q3ActionGroupPrivate::Action4Item*>::Iterator itmi4(d->action4items.begin());
    Q3ActionGroupPrivate::Action4Item* mi4;
    while (itmi4 != d->action4items.end()) {
        mi4 = *itmi4;
        ++itmi4;
        mi4->widget->removeAction(mi4->action);
    }
    delete Q3ActionPrivate::Action4Item::action;
    Q3ActionPrivate::Action4Item::action = 0;

    delete d->separatorAction;
    while (!d->menubuttons.isEmpty())
        delete d->menubuttons.takeFirst();
    while (!d->comboboxes.isEmpty())
        delete d->comboboxes.takeFirst();
    while (!d->menuitems.isEmpty())
        delete d->menuitems.takeFirst();
    while (!d->popupmenus.isEmpty())
        delete d->popupmenus.takeFirst();
    delete d;
}

/*!
    \property Q3ActionGroup::exclusive
    \brief whether the action group does exclusive toggling

    If exclusive is true only one toggle action in the action group
    can ever be active at any one time. If the user chooses another
    toggle action in the group the one they chose becomes active and
    the one that was active becomes inactive.

    \sa Q3Action::toggleAction
*/
void Q3ActionGroup::setExclusive(bool enable)
{
    d->exclusive = enable;
}

bool Q3ActionGroup::isExclusive() const
{
    return d->exclusive;
}

/*!
    \property Q3ActionGroup::usesDropDown
    \brief whether the group's actions are displayed in a subwidget of
    the widgets the action group is added to

    Exclusive action groups added to a toolbar display their actions
    in a combobox with the action's \l Q3Action::text and \l
    Q3Action::iconSet properties shown. Non-exclusive groups are
    represented by a tool button showing their \l Q3Action::iconSet and
    -- depending on \l QMainWindow::usesTextLabel() -- text()
    property.

    In a popup menu the member actions are displayed in a submenu.

    Changing usesDropDown only affects \e subsequent calls to addTo().

    This property's default is false.

*/
void Q3ActionGroup::setUsesDropDown(bool enable)
{
    d->dropdown = enable;
}

bool Q3ActionGroup::usesDropDown() const
{
    return d->dropdown;
}

/*!
    Adds action \a action to this group.

    Normally an action is added to a group by creating it with the
    group as parent, so this function is not usually used.

    \sa addTo()
*/
void Q3ActionGroup::add(Q3Action* action)
{
    if (d->actions.contains(action))
        return;

    d->actions.append(action);

    if (action->whatsThis().isNull())
        action->setWhatsThis(whatsThis());
    if (action->toolTip().isNull())
        action->setToolTip(toolTip());
    if (!action->d->forceDisabled)
	action->d->enabled = isEnabled();
    if (!action->d->forceInvisible)
	action->d->visible = isVisible();

    connect(action, SIGNAL(destroyed()), this, SLOT(childDestroyed()));
    connect(action, SIGNAL(activated()), this, SIGNAL(activated()));
    connect(action, SIGNAL(toggled(bool)), this, SLOT(childToggled(bool)));
    connect(action, SIGNAL(activated()), this, SLOT(childActivated()));

    for (QList<QComboBox*>::Iterator cb(d->comboboxes.begin()); cb != d->comboboxes.end(); ++cb)
        action->addTo(*cb);
    for (QList<QToolButton*>::Iterator mb(d->menubuttons.begin()); mb != d->menubuttons.end(); ++mb) {
        QMenu* menu = (*mb)->popup();
        if (!menu)
            continue;
        action->addTo(menu);
    }
    for (QList<Q3ActionGroupPrivate::Action4Item*>::Iterator ac(d->action4items.begin());
         ac != d->action4items.end(); ++ac)
        action->addTo((*ac)->action->menu());
    for (QList<Q3ActionGroupPrivate::MenuItem*>::Iterator mi(d->menuitems.begin());
         mi != d->menuitems.end(); ++mi) {
        Q3PopupMenu* popup = (*mi)->popup;
        if (!popup)
            continue;
        action->addTo(popup);
    }
}

/*!
    Adds a separator to the group.
*/
void Q3ActionGroup::addSeparator()
{
    if (!d->separatorAction)
        d->separatorAction = new Q3Action(0, "qt_separator_action");
    d->actions.append(d->separatorAction);
}


/*!
    Adds this action group to the widget \a w.

    If isExclusive() is false or usesDropDown() is false, the actions within
    the group are added to the widget individually. For example, if the widget
    is a menu, the actions will appear as individual menu options, and
    if the widget is a toolbar, the actions will appear as toolbar buttons.

    If both isExclusive() and usesDropDown() are true, the actions
    are presented either in a combobox (if \a w is a toolbar) or in a
    submenu (if \a w is a menu).

    All actions should be added to the action group \e before the
    action group is added to the widget. If actions are added to the
    action group \e after the action group has been added to the
    widget these later actions will \e not appear.

    \sa setExclusive() setUsesDropDown() removeFrom()
*/
bool Q3ActionGroup::addTo(QWidget *w)
{
#ifndef QT_NO_TOOLBAR
    if (qt_cast<QToolBar*>(w)) {
        if (d->dropdown) {
            if (!d->exclusive) {
                QList<Q3Action*>::Iterator it(d->actions.begin());
                if (!(*it))
                    return true;

                Q3Action *defAction = *it;

                QToolButton* btn = new QToolButton((QToolBar*) w, "qt_actiongroup_btn");
                addedTo(btn, w);
                connect(btn, SIGNAL(destroyed()), SLOT(objectDestroyed()));
                d->menubuttons.append(btn);

                if (!iconSet().isNull())
                    btn->setIconSet(iconSet());
                else if (!defAction->iconSet().isNull())
                    btn->setIconSet(defAction->iconSet());
                if (text().size())
                    btn->setTextLabel(text());
                else if (defAction->text().size())
                    btn->setTextLabel(defAction->text());
#ifndef QT_NO_TOOLTIP
                if (toolTip().size())
                    QToolTip::add(btn, toolTip());
                else if (defAction->toolTip().size())
                    QToolTip::add(btn, defAction->toolTip());
#endif
#ifndef QT_NO_WHATSTHIS
                if (whatsThis().size())
                    QWhatsThis::add(btn, whatsThis());
                else if (defAction->whatsThis().size())
                    QWhatsThis::add(btn, defAction->whatsThis());
#endif

                connect(btn, SIGNAL(clicked()), defAction, SIGNAL(activated()));
                connect(btn, SIGNAL(toggled(bool)), defAction, SLOT(toolButtonToggled(bool)));
                connect(btn, SIGNAL(destroyed()), defAction, SLOT(objectDestroyed()));

                QPopupMenu *menu = new QPopupMenu(btn, "qt_actiongroup_menu");
                btn->setPopupDelay(0);
                btn->setPopup(menu);

                while (it != d->actions.end()) {
                    (*it)->addTo(menu);
                    ++it;
                }
                d->update(this);
                return true;
            } else {
                QComboBox *box = new QComboBox(false, w, "qt_actiongroup_combo");
                addedTo(box, w);
                connect(box, SIGNAL(destroyed()), SLOT(objectDestroyed()));
                d->comboboxes.append(box);
#ifndef QT_NO_TOOLTIP
                if (toolTip().size())
                    QToolTip::add(box, toolTip());
#endif
#ifndef QT_NO_WHATSTHIS
                if (whatsThis().size())
                    QWhatsThis::add(box, whatsThis());
#endif
                int onIndex = 0;
                bool foundOn = false;
                for (QList<Q3Action*>::Iterator it(d->actions.begin()); it != d->actions.end(); ++it) {
                    Q3Action *action = *it;
                    if (!foundOn)
                        foundOn = action->isOn();
                    if (action->objectName() != QLatin1String("qt_separator_action") && !foundOn)
                        onIndex++;
                    action->addTo(box);
                }
                if (foundOn)
                    box->setCurrentItem(onIndex);
                connect(box, SIGNAL(activated(int)), this, SLOT(internalComboBoxActivated(int)));
                connect(box, SIGNAL(highlighted(int)), this, SLOT(internalComboBoxHighlighted(int)));
                d->update(this);
                return true;
            }
        }
    } else
#endif
    if (::qt_cast<Q3PopupMenu*>(w)) {
        Q3PopupMenu *popup;
        if (d->dropdown) {
            Q3PopupMenu *menu = (Q3PopupMenu*)w;
            popup = new Q3PopupMenu(w, "qt_actiongroup_menu");
            d->popupmenus.append(popup);
            connect(popup, SIGNAL(destroyed()), SLOT(objectDestroyed()));

            int id;
            if (!iconSet().isNull()) {
                if (menuText().isEmpty())
                    id = menu->insertItem(iconSet(), text(), popup);
                else
                    id = menu->insertItem(iconSet(), menuText(), popup);
            } else {
                if (menuText().isEmpty())
                    id = menu->insertItem(text(), popup);
                else
                    id = menu->insertItem(menuText(), popup);
            }

            addedTo(menu->indexOf(id), menu);

            Q3ActionGroupPrivate::MenuItem *item = new Q3ActionGroupPrivate::MenuItem;
            item->id = id;
            item->popup = popup;
            d->menuitems.append(item);
        } else {
            popup = (Q3PopupMenu*)w;
        }
        for (QList<Q3Action*>::Iterator it(d->actions.begin()); it != d->actions.end(); ++it) {
            // #### do an addedTo(index, popup, action), need to find out index
            (*it)->addTo(popup);
        }
        return true;
    }
    if (::qt_cast<QMenu*>(w)) {
        QMenu *menu = (QMenu*)w;
        if (d->dropdown) {
            Q3ActionGroupPrivate::Action4Item *ai = new Q3ActionGroupPrivate::Action4Item;
            if(!ai->action)  { //static
                ai->action = new QAction;
                ai->action->setMenu(new QMenu);
                if (!iconSet().isNull())
                    ai->action->setIcon(iconSet());
                if (menuText().isEmpty())
                    ai->action->setText(text());
                else
                    ai->action->setText(menuText());
            }
            addedTo(w, w);
            ai->widget = w;
            ai->widget->addAction(Q3ActionGroupPrivate::Action4Item::action);
            d->action4items.append(ai);
            menu = ai->action->menu();
        }
        for (QList<Q3Action*>::Iterator it(d->actions.begin()); it != d->actions.end(); ++it)
            (*it)->addTo(menu);
        return true;
    }
    for (QList<Q3Action*>::Iterator it(d->actions.begin()); it != d->actions.end(); ++it) {
        // #### do an addedTo(index, popup, action), need to find out index
        (*it)->addTo(w);
    }
    return true;
}

/*! \reimp
*/
bool Q3ActionGroup::removeFrom(QWidget* w)
{
    for (QList<Q3Action*>::Iterator it(d->actions.begin()); it != d->actions.end(); ++it)
        (*it)->removeFrom(w);

#ifndef QT_NO_TOOLBAR
    if (::qt_cast<QToolBar*>(w)) {
        QList<QComboBox*>::Iterator cb(d->comboboxes.begin());
        while (cb != d->comboboxes.end()) {
            QComboBox *box = *cb;
            ++cb;
            if (box->parentWidget() == w)
                delete box;
        }
        QList<QToolButton*>::Iterator mb(d->menubuttons.begin());
        while (mb != d->menubuttons.end()) {
            QToolButton *btn = *mb;
            ++mb;
            if (btn->parentWidget() == w)
                delete btn;
        }
    } else
#endif
    if (::qt_cast<Q3PopupMenu*>(w)) {
        QList<Q3ActionGroupPrivate::MenuItem*>::Iterator pu(d->menuitems.begin());
        while (pu != d->menuitems.end()) {
            Q3ActionGroupPrivate::MenuItem *mi = *pu;
            ++pu;
            if (d->dropdown && mi->popup)
                ((Q3PopupMenu*)w)->removeItem(mi->id);
            delete mi->popup;
        }
    }
    if (::qt_cast<QMenu*>(w)) {
        QList<Q3ActionGroupPrivate::Action4Item*>::Iterator it(d->action4items.begin());
        Q3ActionGroupPrivate::Action4Item *a4i;
        while (it != d->action4items.end()) {
            a4i = *it;
            ++it;
            if (a4i->widget == w) {
                a4i->widget->removeAction(a4i->action);
                d->action4items.removeAll(a4i);
                delete a4i;
            }
        }
    }
    return true;
}

/*! \internal
*/
void Q3ActionGroup::childToggled(bool b)
{
    if (!isExclusive())
        return;
    Q3Action* s = qt_cast<Q3Action*>(sender());
    if (!s)
        return;
    if (b) {
        if (s != d->selected) {
            d->selected = s;
            for (QList<Q3Action*>::Iterator it(d->actions.begin()); it != d->actions.end(); ++it) {
                if ((*it)->isToggleAction() && (*it) != s)
                    (*it)->setOn(false);
            }
            emit selected(s);
        }
    } else {
        if (s == d->selected) {
            // at least one has to be selected
            s->setOn(true);
        }
    }
}

/*! \internal
*/
void Q3ActionGroup::childActivated()
{
    Q3Action* s = qt_cast<Q3Action*>(sender());
    if (s) {
        emit activated(s);
        emit Q3Action::activated();
    }
}


/*! \internal
*/
void Q3ActionGroup::childDestroyed()
{
    d->actions.removeAll((Q3Action *)sender());
    if (d->selected == sender())
        d->selected = 0;
}

/*! \reimp
*/
void Q3ActionGroup::setEnabled(bool enable)
{
    if (enable == isEnabled())
        return;
    Q3Action::setEnabled(enable);
    d->update(this);
}

/*! \reimp
*/
void Q3ActionGroup::setToggleAction(bool toggle)
{
    for (QList<Q3Action*>::Iterator it(d->actions.begin()); it != d->actions.end(); ++it)
        (*it)->setToggleAction(toggle);
    Q3Action::setToggleAction(true);
    d->update(this);
}

/*! \reimp
*/
void Q3ActionGroup::setOn(bool on)
{
    for (QList<Q3Action*>::Iterator it(d->actions.begin()); it != d->actions.end(); ++it) {
        Q3Action *act = *it;
        if (act->isToggleAction())
            act->setOn(on);
    }
    Q3Action::setOn(on);
    d->update(this);
}

/*! \reimp
 */
void Q3ActionGroup::setVisible(bool visible)
{
    Q3Action::setVisible(visible);
    d->update(this);
}

/*! \reimp
*/
void Q3ActionGroup::setIconSet(const QIconSet& icon)
{
    Q3Action::setIconSet(icon);
    d->update(this);
}

/*! \reimp
*/
void Q3ActionGroup::setText(const QString& txt)
{
    if (txt == text())
        return;

    Q3Action::setText(txt);
    d->update(this);
}

/*! \reimp
*/
void Q3ActionGroup::setMenuText(const QString& text)
{
    if (text == menuText())
        return;

    Q3Action::setMenuText(text);
    d->update(this);
}

/*! \reimp
*/
void Q3ActionGroup::setToolTip(const QString& text)
{
    if (text == toolTip())
        return;
    for (QList<Q3Action*>::Iterator it(d->actions.begin()); it != d->actions.end(); ++it) {
        if ((*it)->toolTip().isNull())
            (*it)->setToolTip(text);
    }
    Q3Action::setToolTip(text);
    d->update(this);
}

/*! \reimp
*/
void Q3ActionGroup::setWhatsThis(const QString& text)
{
    if (text == whatsThis())
        return;
    for (QList<Q3Action*>::Iterator it(d->actions.begin()); it != d->actions.end(); ++it) {
        if ((*it)->whatsThis().isNull())
            (*it)->setWhatsThis(text);
    }
    Q3Action::setWhatsThis(text);
    d->update(this);
}

/*! \reimp
*/
void Q3ActionGroup::childEvent(QChildEvent *e)
{
    if (!e->removed())
        return;

    Q3Action *action = qt_cast<Q3Action*>(e->child());
    if (!action)
        return;

    for (QList<QComboBox*>::Iterator cb(d->comboboxes.begin());
         cb != d->comboboxes.end(); ++cb) {
        for (int i = 0; i < (*cb)->count(); i++) {
            if ((*cb)->text(i) == action->text()) {
                (*cb)->removeItem(i);
                break;
            }
        }
    }
    for (QList<QToolButton*>::Iterator mb(d->menubuttons.begin());
         mb != d->menubuttons.end(); ++mb) {
        QMenu* popup = (*mb)->popup();
        if (!popup)
            continue;
        action->removeFrom(popup);
    }
    for (QList<Q3ActionGroupPrivate::MenuItem*>::Iterator mi(d->menuitems.begin());
         mi != d->menuitems.end(); ++mi) {
        Q3PopupMenu* popup = (*mi)->popup;
        if (!popup)
            continue;
        action->removeFrom(popup);
    }
    if(QAction *act = Q3ActionGroupPrivate::Action4Item::action)
        action->removeFrom(act->menu());
}

/*!
    \fn void Q3ActionGroup::selected(Q3Action*)

    This signal is emitted from exclusive groups when toggle actions
    change state.

    The argument is the action whose state changed to "on".

    \sa setExclusive(), isOn() Q3Action::toggled()
*/

/*!
    \fn void Q3ActionGroup::activated(Q3Action*)

    This signal is emitted from groups when one of it's actions gets
    activated.

    The argument is the action which got activated.

    \sa setExclusive(), isOn() Q3Action::toggled()
*/


/*! \internal
*/
void Q3ActionGroup::internalComboBoxActivated(int index)
{
    Q3Action *a = 0;
    for (int i = 0; i <= index && i < (int)d->actions.count(); ++i) {
        a = d->actions.at(i);
        if (a && a->objectName() == QLatin1String("qt_separator_action"))
            index++;
    }
    a = d->actions.at(index);
    if (a) {
        if (a != d->selected) {
            d->selected = a;
            for (QList<Q3Action*>::Iterator it(d->actions.begin()); it != d->actions.end(); ++it) {
                if ((*it)->isToggleAction() && (*it) != a)
                    (*it)->setOn(false);
            }
            if (a->isToggleAction())
                a->setOn(true);

            emit activated(a);
            emit Q3Action::activated();
            emit a->activated();
            if (a->isToggleAction())
                emit selected(d->selected);
        } else if (!a->isToggleAction()) {
            emit activated(a);
            emit Q3Action::activated();
            emit a->activated();
        }
        a->clearStatusText();
    }
}

/*! \internal
*/
void Q3ActionGroup::internalComboBoxHighlighted(int index)
{
    Q3Action *a = 0;
    for (int i = 0; i <= index && i < (int)d->actions.count(); ++i) {
        a = d->actions.at(i);
        if (a && a->objectName() == QLatin1String("qt_separator_action"))
            index++;
    }
    a = d->actions.at(index);
    if (a)
        a->showStatusText(a->statusTip());
    else
        clearStatusText();
}

/*! \internal
*/
void Q3ActionGroup::internalToggle(Q3Action *a)
{
    int index = d->actions.indexOf(a);
    if (index == -1)
        return;

    int lastItem = index;
    for (int i=0; i<lastItem; ++i) {
        Q3Action *action = d->actions.at(i);
        if (action->objectName() == QLatin1String("qt_separator_action"))
            --index;
    }

    for (QList<QComboBox*>::Iterator it(d->comboboxes.begin());
         it != d->comboboxes.end(); ++it)
            (*it)->setCurrentItem(index);
}

/*! \internal
*/
void Q3ActionGroup::objectDestroyed()
{
    const QObject* obj = sender();
    d->menubuttons.removeAll((QToolButton *)obj);
    for (QList<Q3ActionGroupPrivate::MenuItem *>::Iterator mi(d->menuitems.begin());
         mi != d->menuitems.end(); ++mi) {
        if ((*mi)->popup == obj) {
            d->menuitems.removeAll(*mi);
            delete *mi;
            break;
        }
    }
    d->popupmenus.removeAll((Q3PopupMenu*)obj);
    d->comboboxes.removeAll((QComboBox*)obj);
}

/*!
    This function is called from the addTo() function when it has
    created a widget (\a actionWidget) for the child action \a a in
    the \a container.
*/

void Q3ActionGroup::addedTo(QWidget *actionWidget, QWidget *container, Q3Action *a)
{
    Q_UNUSED(actionWidget);
    Q_UNUSED(container);
    Q_UNUSED(a);
}

/*!
    \overload

    This function is called from the addTo() function when it has
    created a menu item for the child action at the index position \a
    index in the popup menu \a menu.
*/

void Q3ActionGroup::addedTo(int index, Q3PopupMenu *menu, Q3Action *a)
{
    Q_UNUSED(index);
    Q_UNUSED(menu);
    Q_UNUSED(a);
}

/*!
    \reimp
    \overload

    This function is called from the addTo() function when it has
    created a widget (\a actionWidget) in the \a container.
*/

void Q3ActionGroup::addedTo(QWidget *actionWidget, QWidget *container)
{
    Q_UNUSED(actionWidget);
    Q_UNUSED(container);
}

/*!
    \reimp
    \overload

    This function is called from the addTo() function when it has
    created a menu item at the index position \a index in the popup
    menu \a menu.
*/

void Q3ActionGroup::addedTo(int index, Q3PopupMenu *menu)
{
    Q_UNUSED(index);
    Q_UNUSED(menu);
}

#endif
