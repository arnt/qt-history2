/****************************************************************************
**
** Implementation of Q3MenuData class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt Compat Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "q3menudata.h"
#ifndef QT_NO_MENUDATA
#include "q3popupmenu.h"
#include "q3menubar.h"
#include "qapplication.h"
#include "qpointer.h"
#include "qsignal.h"

class Q3MenuItemData {
public:
    Q3CustomMenuItem    *custom_item;        // custom menu item
};

class Q3MenuDataData {
    // attention: also defined in q3menubar.cpp and q3popupmenu.cpp
public:
    Q3MenuDataData();
    QPointer<QWidget> aWidget;
    int aInt;
};
Q3MenuDataData::Q3MenuDataData()
    : aInt(-1)
{}

/*!
    \class Q3MenuData
    \brief The Q3MenuData class is a base class for Q3MenuBar and Q3PopupMenu.

    \ingroup misc

    Q3MenuData has an internal list of menu items. A menu item can have
    a text(), an \link accel() accelerator\endlink, a pixmap(), an
    iconSet(), a whatsThis() text and a popup menu (unless it is a
    separator). Menu items may optionally be \link setItemChecked()
    checked\endlink (except for separators).

    The menu item sends out an \link Q3MenuBar::activated()
    activated()\endlink signal when it is chosen and a \link
    Q3MenuBar::highlighted() highlighted()\endlink signal when it
    receives the user input focus.

    \keyword menu identifier

    Menu items are assigned the menu identifier \e id that is passed
    in insertItem() or an automatically generated identifier if \e id
    is < 0 (the default). The generated identifiers (negative
    integers) are guaranteed to be unique within the entire
    application. The identifier is used to access the menu item in
    other functions.

    Menu items can be removed with removeItem() and removeItemAt(), or
    changed with changeItem(). All menu items can be removed with
    clear(). Accelerators can be changed or set with setAccel().
    Checkable items can be checked or unchecked with setItemChecked().
    Items can be enabled or disabled using setItemEnabled() and
    connected and disconnected with connectItem() and disconnectItem()
    respectively. By default, newly created menu items are visible.
    They can be hidden (and shown again) with setItemVisible().

    Menu items are stored in a list. Use findItem() to find an item by
    its list position or by its menu identifier. (See also indexOf()
    and idAt().)

    \sa QAccel Q3PopupMenu QAction
*/


/*****************************************************************************
  Q3MenuItem member functions
 *****************************************************************************/

Q3MenuItem::Q3MenuItem()
    :ident(-1), iconset_data(0), pixmap_data(0), popup_menu(0),
     widget_item(0), signal_data(0), signal_value(0), is_separator(false), is_enabled(true),
     is_checked(false), is_dirty(true), is_visible(true), d(0)
{}

Q3MenuItem::~Q3MenuItem()
{
    delete iconset_data;
    delete pixmap_data;
    delete signal_data;
    delete widget_item;
    if (d)
        delete d->custom_item;
    delete d;
}


/*****************************************************************************
  Q3MenuData member functions
 *****************************************************************************/

Q3MenuItemData* Q3MenuItem::extra()
{
    if (!d) d = new Q3MenuItemData;
    return d;
}

Q3CustomMenuItem *Q3MenuItem::custom() const
{
    if (!d) return 0;
    return d->custom_item;
}


static int get_seq_id()
{
    static int seq_no = -2;
    return seq_no--;
}


/*!
    Constructs an empty menu data list.
*/

Q3MenuData::Q3MenuData()
{
    actItem = -1;                                // no active menu item
    mitems = new Q3MenuItemList;                        // create list of menu items
    mitemsAutoDelete = true;
    parentMenu = 0;                                // assume top-level
    isPopupMenu = false;
    isMenuBar = false;
    mouseBtDn = false;
    badSize = true;
    avoid_circularity = 0;
    actItemDown = false;
    d = new Q3MenuDataData;
}

/*!
    Removes all menu items and disconnects any signals that have been
    connected.
*/

Q3MenuData::~Q3MenuData()
{
    if (mitemsAutoDelete) {
        while (!mitems->isEmpty())
            delete mitems->takeFirst();
    }
    delete mitems;
    delete d;
}


/*!
    Virtual function; notifies subclasses about an item with \a id
    that has been changed.
*/

void Q3MenuData::updateItem(int /* id */)        // reimplemented in subclass
{
}

/*!
    Virtual function; notifies subclasses that one or more items have
    been inserted or removed.
*/

void Q3MenuData::menuContentsChanged()                // reimplemented in subclass
{
}

/*!
    Virtual function; notifies subclasses that one or more items have
    changed state (enabled/disabled or checked/unchecked).
*/

void Q3MenuData::menuStateChanged()                // reimplemented in subclass
{
}

/*!
    Virtual function; notifies subclasses that a popup menu item has
    been inserted.
*/

void Q3MenuData::menuInsPopup(Q3PopupMenu *)        // reimplemented in subclass
{
}

/*!
    Virtual function; notifies subclasses that a popup menu item has
    been removed.
*/

void Q3MenuData::menuDelPopup(Q3PopupMenu *)        // reimplemented in subclass
{
}


/*!
    Returns the number of items in the menu.
*/

uint Q3MenuData::count() const
{
    return mitems->count();
}

/*!
  \internal

  Internal function that insert a menu item. Called by all insert()
  functions.
*/

int Q3MenuData::insertAny(const QString *text, const QPixmap *pixmap,
                          Q3PopupMenu *popup, const QIconSet* iconset, int id, int index,
                          QWidget* widget, Q3CustomMenuItem* custom)
{
    if (index < 0) {        // append, but not if the rightmost item is an mdi separator in the menubar
        index = mitems->count();
        if (isMenuBar && !mitems->isEmpty() && mitems->last()->widget()
             && mitems->last()->isSeparator())
            index--;
    } else if (index > (int) mitems->count()) { // append
        index = mitems->count();
    }
    if (id < 0)                                // -2, -3 etc.
        id = get_seq_id();

    Q3MenuItem *mi = new Q3MenuItem;
    mi->ident = id;
    if (widget != 0) {
        mi->widget_item = widget;
        mi->is_separator = !widget->isFocusEnabled();
    } else if (custom != 0) {
        mi->extra()->custom_item = custom;
        mi->is_separator = custom->isSeparator();
        if (iconset && !iconset->isNull())
            mi->iconset_data = new QIconSet(*iconset);
    } else if (text == 0 && pixmap == 0 && popup == 0) {
        mi->is_separator = true;                // separator
    } else {
#ifndef Q_OS_TEMP
        mi->text_data = text?*text:QString();
#else
        QString newText(*text);
        newText.truncate(newText.findRev('\t'));
        mi->text_data = newText.isEmpty()?QString():newText;
#endif
#ifndef QT_NO_ACCEL
        mi->accel_key = Qt::Key_unknown;
#endif
        if (pixmap && !pixmap->isNull())
            mi->pixmap_data = new QPixmap(*pixmap);
        if ((mi->popup_menu = popup))
            menuInsPopup(popup);
        if (iconset && !iconset->isNull())
            mi->iconset_data = new QIconSet(*iconset);
    }

    mitems->insert(index, mi);
    Q3PopupMenu* p = (Q3PopupMenu*)(QWidget*)Q3MenuData::d->aWidget;
    if ( p && p->isVisible() ) {
	p->mitems->clear();
	for (int i = 0; i < mitems->size(); ++i) {
	    if (mitems->at(i)->id() != Q3MenuData::d->aInt && !mitems->at(i)->widget())
		p->mitems->append(mitems->at(i));
	}
    }
    menuContentsChanged();                        // menu data changed
    return mi->ident;
}

/*!
    \internal

  Internal function that finds the menu item where \a popup is located,
  storing its index at \a index if \a index is not NULL.
*/
Q3MenuItem *Q3MenuData::findPopup(Q3PopupMenu *popup, int *index)
{
    int i;
    Q3MenuItem *mi = 0;
    for (i = 0; i < mitems->size(); ++i) {
        mi = mitems->at(i);
        if (mi->popup_menu == popup)                // found popup
            break;
    }
    if (index && mi)
        *index = i;
    return mi;
}

void Q3MenuData::removePopup(Q3PopupMenu *popup)
{
    int index = 0;
    Q3MenuItem *mi = findPopup(popup, &index);
    if (mi) {
        mi->popup_menu = 0;
        removeItemAt(index);
    }
}


/*!
    The family of insertItem() functions inserts menu items into a
    popup menu or a menu bar.

    A menu item is usually either a text string or a pixmap, both with
    an optional icon or keyboard accelerator. For special cases it is
    also possible to insert custom items (see \l{Q3CustomMenuItem}) or
    even widgets into popup menus.

    Some insertItem() members take a popup menu as an additional
    argument. Use this to insert submenus into existing menus or
    pulldown menus into a menu bar.

    The number of insert functions may look confusing, but they are
    actually quite simple to use.

    This default version inserts a menu item with the text \a text,
    the accelerator key \a accel, an id and an optional index and
    connects it to the slot \a member in the object \a receiver.

    Example:
    \code
        Q3MenuBar   *mainMenu = new Q3MenuBar;
        Q3PopupMenu *fileMenu = new Q3PopupMenu;
        fileMenu->insertItem("New",  myView, SLOT(newFile()), Qt::CTRL+Qt::Key_N);
        fileMenu->insertItem("Open", myView, SLOT(open()),    Qt::CTRL+Qt::Key_O);
        mainMenu->insertItem("File", fileMenu);
    \endcode

    Not all insert functions take an object/slot parameter or an
    accelerator key. Use connectItem() and setAccel() on those items.

    If you need to translate accelerators, use tr() with the text and
    accelerator. (For translations use a string \link QKeySequence key
    sequence\endlink.):
    \code
        fileMenu->insertItem(tr("Open"), myView, SLOT(open()),
                              tr("Ctrl+O"));
    \endcode

    In the example above, pressing Ctrl+O or selecting "Open" from the
    menu activates the myView->open() function.

    Some insert functions take a QIconSet parameter to specify the
    little menu item icon. Note that you can always pass a QPixmap
    object instead.

    The \a id specifies the identification number associated with the
    menu item. Note that only positive values are valid, as a negative
    value will make Qt select a unique id for the item.

    The \a index specifies the position in the menu. The menu item is
    appended at the end of the list if \a index is negative.

    Note that keyboard accelerators in Qt are not application-global,
    instead they are bound to a certain top-level window. For example,
    accelerators in Q3PopupMenu items only work for menus that are
    associated with a certain window. This is true for popup menus
    that live in a menu bar since their accelerators will then be
    installed in the menu bar itself. This also applies to stand-alone
    popup menus that have a top-level widget in their parentWidget()
    chain. The menu will then install its accelerator object on that
    top-level widget. For all other cases use an independent QAccel
    object.

    \warning Be careful when passing a literal 0 to insertItem()
    because some C++ compilers choose the wrong overloaded function.
    Cast the 0 to what you mean, e.g. \c{(QObject*)0}.

    \warning On Mac OS X, items that connect to a slot that are inserted into a
    menubar will not function as we use the native menubar that knows nothing
    about signals or slots. Instead insert the items into a popup menu and
    insert the popup menu into the menubar. This may be fixed in a future Qt
    version.

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa removeItem(), changeItem(), setAccel(), connectItem(), QAccel,
    qnamespace.h
*/

int Q3MenuData::insertItem(const QString &text,
                           const QObject *receiver, const char* member,
                           const QKeySequence& accel, int id, int index)
{
    int actualID = insertAny(&text, 0, 0, 0, id, index);
    connectItem(actualID, receiver, member);
#ifndef QT_NO_ACCEL
    if (accel)
        setAccel(accel, actualID);
#endif
    return actualID;
}

/*!
    \overload

    Inserts a menu item with icon \a icon, text \a text, accelerator
    \a accel, optional id \a id, and optional \a index position. The
    menu item is connected it to the \a receiver's \a member slot. The
    icon will be displayed to the left of the text in the item.

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa removeItem(), changeItem(), setAccel(), connectItem(), QAccel,
    qnamespace.h
*/

int Q3MenuData::insertItem(const QIconSet& icon,
                           const QString &text,
                           const QObject *receiver, const char* member,
                           const QKeySequence& accel, int id, int index)
{
    int actualID = insertAny(&text, 0, 0, &icon, id, index);
    connectItem(actualID, receiver, member);
#ifndef QT_NO_ACCEL
    if (accel)
        setAccel(accel, actualID);
#endif
    return actualID;
}

/*!
    \overload

    Inserts a menu item with pixmap \a pixmap, accelerator \a accel,
    optional id \a id, and optional \a index position. The menu item
    is connected it to the \a receiver's \a member slot. The icon will
    be displayed to the left of the text in the item.

    To look best when being highlighted as a menu item, the pixmap
    should provide a mask (see QPixmap::mask()).

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int Q3MenuData::insertItem(const QPixmap &pixmap,
                           const QObject *receiver, const char* member,
                           const QKeySequence& accel, int id, int index)
{
    int actualID = insertAny(0, &pixmap, 0, 0, id, index);
    connectItem(actualID, receiver, member);
#ifndef QT_NO_ACCEL
    if (accel)
        setAccel(accel, actualID);
#endif
    return actualID;
}


/*!
    \overload

    Inserts a menu item with icon \a icon, pixmap \a pixmap,
    accelerator \a accel, optional id \a id, and optional \a index
    position. The icon will be displayed to the left of the pixmap in
    the item. The item is connected to the \a member slot in the \a
    receiver object.

    To look best when being highlighted as a menu item, the pixmap
    should provide a mask (see QPixmap::mask()).

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa removeItem(), changeItem(), setAccel(), connectItem(), QAccel,
    qnamespace.h
*/

int Q3MenuData::insertItem(const QIconSet& icon,
                           const QPixmap &pixmap,
                           const QObject *receiver, const char* member,
                           const QKeySequence& accel, int id, int index)
{
    int actualID = insertAny(0, &pixmap, 0, &icon, id, index);
    connectItem(actualID, receiver, member);
#ifndef QT_NO_ACCEL
    if (accel)
        setAccel(accel, actualID);
#endif
    return actualID;
}



/*!
    \overload

    Inserts a menu item with text \a text, optional id \a id, and
    optional \a index position.

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int Q3MenuData::insertItem(const QString &text, int id, int index)
{
    return insertAny(&text, 0, 0, 0, id, index);
}

/*!
    \overload

    Inserts a menu item with icon \a icon, text \a text, optional id
    \a id, and optional \a index position. The icon will be displayed
    to the left of the text in the item.

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int Q3MenuData::insertItem(const QIconSet& icon,
                           const QString &text, int id, int index)
{
    return insertAny(&text, 0, 0, &icon, id, index);
}

/*!
    \overload

    Inserts a menu item with text \a text, submenu \a popup, optional
    id \a id, and optional \a index position.

    The \a popup must be deleted by the programmer or by its parent
    widget. It is not deleted when this menu item is removed or when
    the menu is deleted.

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int Q3MenuData::insertItem(const QString &text, Q3PopupMenu *popup,
                           int id, int index)
{
    return insertAny(&text, 0, popup, 0, id, index);
}

/*!
    \overload

    Inserts a menu item with icon \a icon, text \a text, submenu \a
    popup, optional id \a id, and optional \a index position. The icon
    will be displayed to the left of the text in the item.

    The \a popup must be deleted by the programmer or by its parent
    widget. It is not deleted when this menu item is removed or when
    the menu is deleted.

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int Q3MenuData::insertItem(const QIconSet& icon,
                           const QString &text, Q3PopupMenu *popup,
                           int id, int index)
{
    return insertAny(&text, 0, popup, &icon, id, index);
}

/*!
    \overload

    Inserts a menu item with pixmap \a pixmap, optional id \a id, and
    optional \a index position.

    To look best when being highlighted as a menu item, the pixmap
    should provide a mask (see QPixmap::mask()).

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int Q3MenuData::insertItem(const QPixmap &pixmap, int id, int index)
{
    return insertAny(0, &pixmap, 0, 0, id, index);
}

/*!
    \overload

    Inserts a menu item with icon \a icon, pixmap \a pixmap, optional
    id \a id, and optional \a index position. The icon will be
    displayed to the left of the pixmap in the item.

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int Q3MenuData::insertItem(const QIconSet& icon,
                           const QPixmap &pixmap, int id, int index)
{
    return insertAny(0, &pixmap, 0, &icon, id, index);
}


/*!
    \overload

    Inserts a menu item with pixmap \a pixmap, submenu \a popup,
    optional id \a id, and optional \a index position.

    The \a popup must be deleted by the programmer or by its parent
    widget. It is not deleted when this menu item is removed or when
    the menu is deleted.

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int Q3MenuData::insertItem(const QPixmap &pixmap, Q3PopupMenu *popup,
                           int id, int index)
{
    return insertAny(0, &pixmap, popup, 0, id, index);
}


/*!
    \overload

    Inserts a menu item with icon \a icon, pixmap \a pixmap submenu \a
    popup, optional id \a id, and optional \a index position. The icon
    will be displayed to the left of the pixmap in the item.

    The \a popup must be deleted by the programmer or by its parent
    widget. It is not deleted when this menu item is removed or when
    the menu is deleted.

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int Q3MenuData::insertItem(const QIconSet& icon,
                           const QPixmap &pixmap, Q3PopupMenu *popup,
                           int id, int index)
{
    return insertAny(0, &pixmap, popup, &icon, id, index);
}



/*!
    \overload

    Inserts a menu item that consists of the widget \a widget with
    optional id \a id, and optional \a index position.

    Ownership of \a widget is transferred to the popup menu or to the
    menu bar.

    Theoretically, any widget can be inserted into a popup menu. In
    practice, this only makes sense with certain widgets.

    If a widget is not focus-enabled (see
    \l{QWidget::isFocusEnabled()}), the menu treats it as a separator;
    this means that the item is not selectable and will never get
    focus. In this way you can, for example, simply insert a QLabel if
    you need a popup menu with a title.

    If the widget is focus-enabled it will get focus when the user
    traverses the popup menu with the arrow keys. If the widget does
    not accept \c ArrowUp and \c ArrowDown in its key event handler,
    the focus will move back to the menu when the respective arrow key
    is hit one more time. This works with a QLineEdit, for example. If
    the widget accepts the arrow key itself, it must also provide the
    possibility to put the focus back on the menu again by calling
    QWidget::focusNextPrevChild(). Futhermore, if the embedded widget
    closes the menu when the user made a selection, this can be done
    safely by calling:
    \code
        if (isVisible() &&
             parentWidget() &&
             parentWidget()->inherits("Q3PopupMenu"))
            parentWidget()->close();
    \endcode

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa removeItem()
*/
int Q3MenuData::insertItem(QWidget* widget, int id, int index)
{
    return insertAny(0, 0, 0, 0, id, index, widget);
}


/*!
    \overload

    Inserts a custom menu item \a custom with optional id \a id, and
    optional \a index position.

    This only works with popup menus. It is not supported for menu
    bars. Ownership of \a custom is transferred to the popup menu.

    If you want to connect a custom item to a slot, use connectItem().

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa connectItem(), removeItem(), Q3CustomMenuItem
*/
int Q3MenuData::insertItem(Q3CustomMenuItem* custom, int id, int index)
{
    return insertAny(0, 0, 0, 0, id, index, 0, custom);
}

/*!
    \overload

    Inserts a custom menu item \a custom with an \a icon and with
    optional id \a id, and optional \a index position.

    This only works with popup menus. It is not supported for menu
    bars. Ownership of \a custom is transferred to the popup menu.

    If you want to connect a custom item to a slot, use connectItem().

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa connectItem(), removeItem(), Q3CustomMenuItem
*/
int Q3MenuData::insertItem(const QIconSet& icon, Q3CustomMenuItem* custom, int id, int index)
{
    return insertAny(0, 0, 0, &icon, id, index, 0, custom);
}


/*!
    Inserts a separator at position \a index. The separator becomes
    the last menu item if \a index is negative.

    In a popup menu a separator is rendered as a horizontal line. In a
    Motif menu bar a separator is spacing, so the rest of the items
    (normally just "Help") are drawn right-justified. In a Windows
    menu bar separators are ignored (to comply with the Windows style
    guidelines).
*/
int Q3MenuData::insertSeparator(int index)
{
    return insertAny(0, 0, 0, 0, -1, index);
}

/*!
    \fn void Q3MenuData::removeItem(int id)

    Removes the menu item that has the identifier \a id.

    \sa removeItemAt(), clear()
*/

void Q3MenuData::removeItem(int id)
{
    Q3MenuData *parent;
    if (findItem(id, &parent))
        parent->removeItemAt(parent->indexOf(id));
}

/*!
    Removes the menu item at position \a index.

    \sa removeItem(), clear()
*/

void Q3MenuData::removeItemAt(int index)
{
    if (index < 0 || index >= (int)mitems->count()) {
        qWarning("Q3MenuData::removeItem: Index %d out of range", index);
        return;
    }
    Q3MenuItem *mi = mitems->at(index);
    if (mi->popup_menu)
        menuDelPopup(mi->popup_menu);
    mitems->removeAt(index);
    if (mitemsAutoDelete)
        delete mi;
    if (!QApplication::closingDown())                // avoid trouble
        menuContentsChanged();
}


/*!
    Removes all menu items.

    \sa removeItem(), removeItemAt()
*/

void Q3MenuData::clear()
{
    for (int i = 0; i < mitems->size(); ++i) {
        Q3MenuItem *mi = mitems->at(i);
        if (mi->popup_menu)
            menuDelPopup(mi->popup_menu);
    }
    if (mitemsAutoDelete) {
        while (!mitems->isEmpty())
            delete mitems->takeFirst();
    } else {
        mitems->clear();
    }
    Q3PopupMenu* p = (Q3PopupMenu*)(QWidget*)Q3MenuData::d->aWidget;
    if ( p && p->isVisible() ) {
	p->mitems->clear();
    }
    if (!QApplication::closingDown())                // avoid trouble
        menuContentsChanged();
}

#ifndef QT_NO_ACCEL

/*!
    Returns the accelerator key that has been defined for the menu
    item \a id, or 0 if it has no accelerator key or if there is no
    such menu item.

    \sa setAccel(), QAccel
*/

QKeySequence Q3MenuData::accel(int id) const
{
    Q3MenuItem *mi = findItem(id);
    if (mi)
        return mi->key();
    return QKeySequence();
}

/*!
    Sets the accelerator key for the menu item \a id to \a key.

    An accelerator key consists of a key code and a combination of the
    modifiers \c Qt::SHIFT, \c Qt::CTRL, \c Qt::ALT or \c Qt::UNICODE_ACCEL (OR'ed or
    added). The header file \c qnamespace.h contains a list of key
    codes.

    Defining an accelerator key produces a text that is added to the
    menu item; for instance, \c Qt::CTRL + \c Qt::Key_O produces "Ctrl+O". The
    text is formatted differently for different platforms.

    Note that keyboard accelerators in Qt are not application-global,
    instead they are bound to a certain top-level window. For example,
    accelerators in Q3PopupMenu items only work for menus that are
    associated with a certain window. This is true for popup menus
    that live in a menu bar since their accelerators will then be
    installed in the menu bar itself. This also applies to stand-alone
    popup menus that have a top-level widget in their parentWidget()
    chain. The menu will then install its accelerator object on that
    top-level widget. For all other cases use an independent QAccel
    object.

    Example:
    \code
        Q3MenuBar *mainMenu = new Q3MenuBar;
        Q3PopupMenu *fileMenu = new Q3PopupMenu;       // file sub menu
        fileMenu->insertItem("Open Document", 67); // add "Open" item
        fileMenu->setAccel(Qt::CTRL + Qt::Key_O, 67);      // Ctrl+O to open
        fileMenu->insertItem("Quit", 69);          // add "Quit" item
        fileMenu->setAccel(Qt::CTRL + Qt::ALT + Qt::Key_Delete, 69); // add Alt+Del to quit
        mainMenu->insertItem("File", fileMenu);    // add the file menu
    \endcode

    If you need to translate accelerators, use tr() with a string:
    \code
        fileMenu->setAccel(tr("Ctrl+O"), 67);
    \endcode

    You can also specify the accelerator in the insertItem() function.
    You may prefer to use QAction to associate accelerators with menu
    items.

    \sa accel() insertItem() QAccel QAction
*/

void Q3MenuData::setAccel(const QKeySequence& key, int id)
{
    Q3MenuData *parent;
    Q3MenuItem *mi = findItem(id, &parent);
    if (mi) {
        mi->accel_key = key;
        parent->menuContentsChanged();
    }
}

#endif // QT_NO_ACCEL

/*!
    Returns the icon set that has been set for menu item \a id, or 0
    if no icon set has been set.

    \sa changeItem(), text(), pixmap()
*/

QIconSet* Q3MenuData::iconSet(int id) const
{
    Q3MenuItem *mi = findItem(id);
    return mi ? mi->iconSet() : 0;
}

/*!
    Returns the text that has been set for menu item \a id, or
    QString::null if no text has been set.

    \sa changeItem(), pixmap(), iconSet()
*/

QString Q3MenuData::text(int id) const
{
    Q3MenuItem *mi = findItem(id);
    return mi ? mi->text() : QString();
}

/*!
    Returns the pixmap that has been set for menu item \a id, or 0 if
    no pixmap has been set.

    \sa changeItem(), text(), iconSet()
*/

QPixmap *Q3MenuData::pixmap(int id) const
{
    Q3MenuItem *mi = findItem(id);
    return mi ? mi->pixmap() : 0;
}

/*!
  \fn void Q3MenuData::changeItem(const QString &, int)
  \obsolete

  Changes the text of the menu item \a id. If the item has an icon,
  the icon remains unchanged.

  \sa text()
*/
/*!
  \fn void Q3MenuData::changeItem(const QPixmap &, int)
  \obsolete

  Changes the pixmap of the menu item \a id. If the item has an icon,
  the icon remains unchanged.

  \sa pixmap()
*/

/*!
  \fn void Q3MenuData::changeItem(const QIconSet &, const QString &, int)
  \obsolete

  Changes the icon and text of the menu item \a id.

  \sa pixmap()
*/

/*!
    Changes the text of the menu item \a id to \a text. If the item
    has an icon, the icon remains unchanged.

    \sa text()
*/

void Q3MenuData::changeItem(int id, const QString &text)
{
    Q3MenuData *parent;
    Q3MenuItem *mi = findItem(id, &parent);
    if (mi) {                                        // item found
        if (mi->text_data == text)                // same string
            return;
        if (mi->pixmap_data) {                // delete pixmap
            delete mi->pixmap_data;
            mi->pixmap_data = 0;
        }
        mi->text_data = text;
#ifndef QT_NO_ACCEL
        if (!mi->accel_key && text.indexOf('\t') != -1)
            mi->accel_key = Qt::Key_unknown;
#endif
        parent->menuContentsChanged();
    }
}

/*!
    \overload

    Changes the pixmap of the menu item \a id to the pixmap \a pixmap.
    If the item has an icon, the icon is unchanged.

    \sa pixmap()
*/

void Q3MenuData::changeItem(int id, const QPixmap &pixmap)
{
    Q3MenuData *parent;
    Q3MenuItem *mi = findItem(id, &parent);
    if (mi) {                                        // item found
        register QPixmap *i = mi->pixmap_data;
        bool fast_refresh = i != 0 &&
            i->width() == pixmap.width() &&
            i->height() == pixmap.height() &&
            mi->text().size() == 0;
        if (!mi->text_data.isNull())                // delete text
            mi->text_data = QString::null;
        if (!pixmap.isNull())
            mi->pixmap_data = new QPixmap(pixmap);
        else
            mi->pixmap_data = 0;
        delete i; // old mi->pixmap_data, could be &pixmap
        if (fast_refresh)
            parent->updateItem(id);
        else
            parent->menuContentsChanged();
    }
}

/*!
    \overload

    Changes the iconset and text of the menu item \a id to the \a icon
    and \a text respectively.

    \sa pixmap()
*/

void Q3MenuData::changeItem(int id, const QIconSet &icon, const QString &text)
{
    changeItem(id, text);
    changeItemIconSet(id, icon);
}

/*!
    \overload

    Changes the iconset and pixmap of the menu item \a id to \a icon
    and \a pixmap respectively.

    \sa pixmap()
*/

void Q3MenuData::changeItem(int id, const QIconSet &icon, const QPixmap &pixmap)
{
    changeItem(id, pixmap);
    changeItemIconSet(id, icon);
}



/*!
    Changes the icon of the menu item \a id to \a icon.

    \sa pixmap()
*/

void Q3MenuData::changeItemIconSet(int id, const QIconSet &icon)
{
    Q3MenuData *parent;
    Q3MenuItem *mi = findItem(id, &parent);
    if (mi) {                                        // item found
        register QIconSet *i = mi->iconset_data;
        bool fast_refresh = i != 0;
        if (!icon.isNull())
            mi->iconset_data = new QIconSet(icon);
        else
            mi->iconset_data = 0;
        delete i; // old mi->iconset_data, could be &icon
        if (fast_refresh)
            parent->updateItem(id);
        else
            parent->menuContentsChanged();
    }
}


/*!
    Returns true if the item with identifier \a id is enabled;
    otherwise returns false

    \sa setItemEnabled(), isItemVisible()
*/

bool Q3MenuData::isItemEnabled(int id) const
{
    Q3MenuItem *mi = findItem(id);
    return mi ? mi->isEnabled() : false;
}

/*!
    If \a enable is true, enables the menu item with identifier \a id;
    otherwise disables the menu item with identifier \a id.

    \sa isItemEnabled()
*/

void Q3MenuData::setItemEnabled(int id, bool enable)
{
    Q3MenuData *parent;
    Q3MenuItem *mi = findItem(id, &parent);
    if (mi && (bool)mi->is_enabled != enable) {
        mi->is_enabled = enable;
#if !defined(QT_NO_ACCEL) && !defined(QT_NO_POPUPMENU)
        if (mi->popup())
            mi->popup()->enableAccel(enable);
#endif
        parent->menuStateChanged();
    }
}


/*!
    Returns true if the menu item with the id \a id is currently
    active; otherwise returns false.
*/
bool Q3MenuData::isItemActive(int id) const
{
    if (actItem == -1)
        return false;
    return indexOf(id) == actItem;
}

/*!
    Returns true if the menu item with the id \a id has been checked;
    otherwise returns false.

    \sa setItemChecked()
*/

bool Q3MenuData::isItemChecked(int id) const
{
    Q3MenuItem *mi = findItem(id);
    return mi ? mi->isChecked() : false;
}

/*!
    If \a check is true, checks the menu item with id \a id; otherwise
    unchecks the menu item with id \a id. Calls
    Q3PopupMenu::setCheckable(true) if necessary.

    \sa isItemChecked()
*/

void Q3MenuData::setItemChecked(int id, bool check)
{
    Q3MenuData *parent;
    Q3MenuItem *mi = findItem(id, &parent);
    if (mi && (bool)mi->is_checked != check) {
        mi->is_checked = check;
#ifndef QT_NO_POPUPMENU
        if (parent->isPopupMenu && !((Q3PopupMenu *)parent)->isCheckable())
            ((Q3PopupMenu *)parent)->setCheckable(true);
#endif
        parent->menuStateChanged();
    }
}

/*!
  Returns true if the menu item with the id \a id is  visible;
  otherwise returns false.

  \sa setItemVisible()
*/

bool Q3MenuData::isItemVisible(int id) const
{
    Q3MenuItem *mi = findItem(id);
    return mi ? mi->isVisible() : false;
}

/*!
  If \a visible is true, shows the menu item with id \a id; otherwise
  hides the menu item with id \a id.

  \sa isItemVisible(), isItemEnabled()
*/

void Q3MenuData::setItemVisible(int id, bool visible)
{
    Q3MenuData *parent;
    Q3MenuItem *mi = findItem(id, &parent);
    if (mi && (bool)mi->is_visible != visible) {
        mi->is_visible = visible;
        parent->menuContentsChanged();
    }
}


/*!
    Returns the menu item with identifier \a id, or 0 if there is no
    item with this identifier.

    Note that Q3MenuItem is an internal class, and that you should not
    need to call this function. Use the higher level functions like
    text(), pixmap() and changeItem() to get and modify menu item
    attributes instead.

    \sa indexOf()
*/

Q3MenuItem *Q3MenuData::findItem(int id) const
{
    return findItem(id, 0);
}


/*!
    \overload

    Returns the menu item with identifier \a id, or 0 if there is no
    item with this identifier. Changes \a *parent to point to the
    parent of the return value.

    Note that Q3MenuItem is an internal class, and that you should not
    need to call this function. Use the higher level functions like
    text(), pixmap() and changeItem() to get and modify menu item
    attributes instead.

    \sa indexOf()
*/

Q3MenuItem * Q3MenuData::findItem(int id, Q3MenuData ** parent) const
{
    if (parent)
        *parent = (Q3MenuData *)this;                // ###

    if (id == -1)                                // bad identifier
        return 0;
    // search this menu
    for (int i = 0; i < mitems->size(); ++i) {
        Q3MenuItem *mi = mitems->at(i);
        if (mi->ident == id)                        // found item
            return mi;
    }
    // search submenus
    for (int i = 0; i < mitems->size(); ++i) {
        Q3MenuItem *mi = mitems->at(i);
#ifndef QT_NO_POPUPMENU
        if (mi->popup_menu) {
            Q3PopupMenu *p = mi->popup_menu;
            if (!p->avoid_circularity) {
                p->avoid_circularity = 1;
                mi = mi->popup_menu->findItem(id, parent);
                p->avoid_circularity = 0;
                if (mi)                                // found item
                    return mi;
            }
        }
#endif
    }
    return 0;                                        // not found
}

/*!
    Returns the index of the menu item with identifier \a id, or -1 if
    there is no item with this identifier.

    \sa idAt(), findItem()
*/

int Q3MenuData::indexOf(int id) const
{
    if (id == -1)                                // bad identifier
        return -1;
    for (int i = 0; i < mitems->size(); ++i) {
        Q3MenuItem *mi = mitems->at(i);
        if (mi->ident == id)                        // this one?
            return i;
    }
    return -1;                                        // not found
}

/*!
    Returns the identifier of the menu item at position \a index in
    the internal list, or -1 if \a index is out of range.

    \sa setId(), indexOf()
*/

int Q3MenuData::idAt(int index) const
{
    return index < (int)mitems->count() && index >= 0 ?
           mitems->at(index)->id() : -1;
}

/*!
    Sets the menu identifier of the item at \a index to \a id.

    If \a index is out of range, the operation is ignored.

    \sa idAt()
*/

void Q3MenuData::setId(int index, int id)
{
    if (index < (int)mitems->count())
        mitems->at(index)->ident = id;
}


/*!
    Sets the parameter of the activation signal of item \a id to \a
    param.

    If any receiver takes an integer parameter, this value is passed.

    \sa connectItem(), disconnectItem(), itemParameter()
*/
bool Q3MenuData::setItemParameter(int id, int param) {
    Q3MenuItem *mi = findItem(id);
    if (!mi)                                        // no such identifier
        return false;
    if (!mi->signal_data) {                        // create new signal
        mi->signal_data = new QSignalEmitter("int");
    }
    mi->signal_value = param;
    return true;
}


/*!
    Returns the parameter of the activation signal of item \a id.

    If no parameter has been specified for this item with
    setItemParameter(), the value defaults to \a id.

    \sa connectItem(), disconnectItem(), setItemParameter()
*/
int Q3MenuData::itemParameter(int id) const
{
    Q3MenuItem *mi = findItem(id);
    if (!mi || !mi->signal_data)
        return id;
    return mi->signal_value;
}


/*!
    Connects the menu item with identifier \a id to \a{receiver}'s \a
    member slot or signal.

    The receiver's slot (or signal) is activated when the menu item is
    activated.

    \sa disconnectItem(), setItemParameter()
*/

bool Q3MenuData::connectItem(int id, const QObject *receiver,
                             const char* member)
{
    Q3MenuItem *mi = findItem(id);
    if (!mi)                                        // no such identifier
        return false;
    if (!mi->signal_data) {                        // create new signal
        mi->signal_data = new QSignalEmitter("int");
        mi->signal_value = id;
    }
    return mi->signal_data->connect(receiver, member);
}


/*!
    Disconnects the \a{receiver}'s \a member from the menu item with
    identifier \a id.

    All connections are removed when the menu data object is
    destroyed.

    \sa connectItem(), setItemParameter()
*/

bool Q3MenuData::disconnectItem(int id, const QObject *receiver,
                                const char* member)
{
    Q3MenuItem *mi = findItem(id);
    if (!mi || !mi->signal_data)                // no identifier or no signal
        return false;
    return mi->signal_data->disconnect(receiver, member);
}

/*!
    Sets \a text as What's This help for the menu item with identifier
    \a id.

    \sa whatsThis()
*/
void Q3MenuData::setWhatsThis(int id, const QString& text)
{

    Q3MenuData *parent;
    Q3MenuItem *mi = findItem(id, &parent);
    if (mi) {
        mi->setWhatsThis(text);
        parent->menuContentsChanged();
    }
}

/*!
    Returns the What's This help text for the item with identifier \a
    id or QString::null if no text has yet been defined.

    \sa setWhatsThis()
*/
QString Q3MenuData::whatsThis(int id) const
{

    Q3MenuItem *mi = findItem(id);
    return mi? mi->whatsThis() : QString();
}



/*!
    \class Q3CustomMenuItem q3menudata.h
    \brief The Q3CustomMenuItem class is an abstract base class for custom menu items in popup menus.

    \ingroup misc

    A custom menu item is a menu item that is defined by two pure
    virtual functions, paint() and sizeHint(). The size hint tells the
    menu how much space it needs to reserve for this item, and paint
    is called whenever the item needs painting.

    This simple mechanism allows you to create all kinds of
    application specific menu items. Examples are items showing
    different fonts in a word processor or menus that allow the
    selection of drawing utilities in a vector drawing program.

    A custom item is inserted into a popup menu with
    Q3PopupMenu::insertItem().

    By default, a custom item can also have an icon and a keyboard
    accelerator. You can reimplement fullSpan() to return true if you
    want the item to span the entire popup menu width. This is
    particularly useful for labels.

    If you want the custom item to be treated just as a separator,
    reimplement isSeparator() to return true.

    Note that you can insert pixmaps or bitmaps as items into a popup
    menu without needing to create a Q3CustomMenuItem. However, custom
    menu items offer more flexibility, and -- especially important
    with Windows style -- provide the possibility of drawing the item
    with a different color when it is highlighted.

    \link menu-example.html menu/menu.cpp\endlink shows a simple
    example how custom menu items can be used.

    Note: the current implementation of Q3CustomMenuItem will not
    recognize shortcut keys that are from text with ampersands. Normal
    accelerators work though.

    \sa Q3MenuData, Q3PopupMenu
*/



/*!
    Constructs a Q3CustomMenuItem
*/
Q3CustomMenuItem::Q3CustomMenuItem()
{
}

/*!
    Destroys a Q3CustomMenuItem
*/
Q3CustomMenuItem::~Q3CustomMenuItem()
{
}


/*!
    Sets the font of the custom menu item to \a font.

    This function is called whenever the font in the popup menu
    changes. For menu items that show their own individual font entry,
    you want to ignore this.
*/
void Q3CustomMenuItem::setFont(const QFont& /* font */)
{
}



/*!
    Returns true if this item wants to span the entire popup menu
    width; otherwise returns false. The default is false, meaning that
    the menu may show an icon and an accelerator key for this item as
    well.
*/
bool Q3CustomMenuItem::fullSpan() const
{
    return false;
}

/*!
    Returns true if this item is just a separator; otherwise returns
    false.
*/
bool Q3CustomMenuItem::isSeparator() const
{
    return false;
}


/*!
    \fn void Q3CustomMenuItem::paint(QPainter* p, const QPalette& pal, bool act,  bool enabled, int x, int y, int w, int h);

    Paints this item. When this function is invoked, the painter \a p
    is set to a font and foreground color suitable for a menu item
    text using color group \a pal. The item is active if \a act is true
    and enabled if \a enabled is true. The geometry values \a x, \a y,
    \a w and \a h specify where to draw the item.

    Do not draw any background, this has already been done by the
    popup menu according to the current GUI style.
*/


/*!
    \fn QSize Q3CustomMenuItem::sizeHint();

    Returns the item's size hint.
*/



/*!
    Activates the menu item at position \a index.

    If the index is invalid (for example, -1), the object itself is
    deactivated.
*/
void Q3MenuData::activateItemAt(int index)
{
#ifndef QT_NO_MENUBAR
    if (isMenuBar)
        ((Q3MenuBar*)this)->activateItemAt(index);
    else
#endif
    {
#ifndef QT_NO_POPUPMENU
    if (isPopupMenu)
        ((Q3PopupMenu*)this)->activateItemAt(index);
#endif
    }
}

#endif
