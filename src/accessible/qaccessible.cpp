/****************************************************************************
**
** Implementation of the QAccessible class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qaccessible.h"

#if defined(QT_ACCESSIBILITY_SUPPORT)

#include "qaccessiblewidget.h"
#include "qapplication.h"
#include "qhash.h"
#include "qmetaobject.h"
#include <private/qpluginmanager_p.h>
#include <stdlib.h>

#include "qwidget.h"

/*!
    \class QAccessible qaccessible.h
    \brief The QAccessible class provides enums and static functions
    relating to accessibility.
    \ingroup misc

    The functions in this class are used for the communication between
    accessible applications (also called AT Servers) and tools that
    make applications usable for impaired users (also called AT Clients,
    e.g. screen readers). The communication between clients and servers
    is a two-way communication:

    \e AT \e Servers notify the clients about events through calls to the
    updateAccessibility() function.

    \e AT \e Clients request information about the objects in the server.
    The QAccessibleInterface class is the core interface and encapsulates
    this information in a pure virtual API. Implementations of the interface
    are provided by Qt through the queryAccessibleInterface() API.

    The communication between servers and clients is initialized by the
    setRootObject() function.

    Function pointers can be installed to replace or extend the default behavior
    of the static functions in QAccessible.
*/

/*!
    \enum QAccessible::State

    This enum type defines bitflags that can be combined to indicate
    the state of an accessible object. The values are:

    \value Normal	    The normal state.
    \value Unavailable	    The object is unavailable to the user, e.g. a disabled widget.
    \value Selectable	    The object can be selectable.
    \value MultiSelectable  The object can have multiple selected items.
    \value ExtSelectable    The object supports extended selection.
    \value Selected	    The object is selected.
    \value Focusable	    The object can receive focus. Only objects on the active
    \value Focused	    The object has keyboard focus.
    \value Pressed	    The object is pressed.
    \value Checked	    The object's check box is checked.
    \value Mixed	    The state of the object is not determined, e.g. a tri-state
			    check box that is neither checked nor unchecked.
    \value ReadOnly	    The object can usually be edited, but is explicitly set to
			    read-only.
    \value Protected	    The object is password protected, e.g. a line edit for entering 
			    a Password.
    \value HotTracked	    The object's appearance is sensitive to the mouse cursor position.
    \value DefaultButton    The object represents the default button in a dialog.
    \value Expanded	    The object is expandable, and currently the children are visible.
    \value Collapsed	    The object is collapsed, e.g. a closed listview item, or an
			    iconified window.
    \value Busy		    The object cannot accept input temporarily.
    \omit
    \value Floating
    \endomit
    \value Marqueed	    The object displays scrolling contents, e.g. a log view.
    \value Animated	    The object's appearance changes frequently.
    \value Invisible	    The object is not visible to the user.
    \value Offscreen	    The object is clipped by the visible area. Objects that are
			    offscreen are also invisible.
    \value Sizeable	    The object can be resized, e.g. toplevel windows.
    \value Moveable	    The object can be moved.
    \value SelfVoicing	    The object describes itself through speach or sound.
			    window can receive focus.
    \value Linked	    The object is linked to another object, e.g. a hyperlink.
    \value Traversed	    The object is linked and has been visited.
    \omit
    \value AlertLow
    \value AlertMedium
    \value AlertHigh
    \endomit
    \value HasPopup	    The object opens a popup.
    \value Modal	    The object blocks input from other objects.

    Implementations of QAccessibleInterface::state() return a combination
    of those flags.
*/

/*!
    \enum QAccessible::Event

    This enum type defines accessible event types. The event types are:

    \value SoundPlayed
    \value Alert
    \value ForegroundChanged
    \value MenuStart
    \value MenuEnd
    \value PopupMenuStart
    \value PopupMenuEnd
    \value ContextHelpStart
    \value ContextHelpEnd
    \value DragDropStart
    \value DragDropEnd
    \value DialogStart
    \value DialogEnd
    \value ScrollingStart
    \value ScrollingEnd
    \value ObjectCreated
    \value ObjectDestroyed
    \value ObjectShow
    \value ObjectHide
    \value ObjectReorder
    \value Focus
    \value Selection
    \value SelectionAdd
    \value SelectionRemove
    \value SelectionWithin
    \value StateChanged
    \value LocationChanged
    \value NameChanged
    \value DescriptionChanged
    \value ValueChanged
    \value ParentChanged
    \value HelpChanged
    \value DefaultActionChanged
    \value AcceleratorChanged
    \value MenuCommand
*/

/*!
    \enum QAccessible::Role

    This enum defines the role of an accessible object. The roles are:

    \value NoRole	    The object has no role. This usually indicates an invalid object.
    \value TitleBar	    A title bar caption of a window.
    \value MenuBar	    A menu bar from which menus are opened by the user.
    \value MenuItem	    An item in a menu or menu bar.
    \value PopupMenu	    A menu which lists options that the user can select to perform an
			    action.
    \value ScrollBar	    A scroll bar, which allows the user to scroll the visible area.
    \value ToolBar	    A tool bar, which groups controls that the user accesses frequently.
    \value StatusBar	    A status bar, displaying different information.
    \value Grip		    A grip that the user can drag to change the size of controls.
    \value Sound	    An object representing sound.
    \value Cursor	    The object represents the mouse cursor.
    \value Caret	    The object represents the system caret.
    \value AlertMessage	    The object is used to alert the user.
    \value Window	    A top level window.
    \value Client	    The client area in a window.
    \value Application	    The application main window.
    \value Document	    A document window, usually in an MDI environment.
    \value Pane		    A generic container.
    \value Chart	    An object displays a graphical representation of data.
    \value Dialog	    A dialog box.
    \value Border	    The object represents a border.
    \value Grouping	    The object represents logical grouping of other objects.
    \value LayeredPane	    An object that can contain layered children, e.g. in a stack.
    \value Splitter	    A splitter distributing available space between the children.
    \value Separator	    A separator that divides space into logical areas.
    \value Table	    A table representing data in a grid of rows and columns.
    \value ColumnHeader	    A header for a column of data.
    \value RowHeader	    A header for a row of data.
    \value Column	    A column of cells, usually within a table.
    \value Row		    A row of cells, usually within a table.
    \value Cell		    A cell in a table.
    \value Link		    A link to something else.
    \value ToolTip	    A tool tip which provides information about other objects.
    \value HelpBalloon	    An object displaying help in a separate, short lived window.
    \value Assistant	    An object providing help interactively.
    \value List		    A list of items, allowing the user to select one or more items.
    \value ListItem	    An item in a list of items.
    \value Tree		    A list of items in a tree structure.
    \value TreeItem	    An item in a tree structure.
    \value PageTab	    A page tab that the user can select to switch to a different
			    page in a dialog.
    \value PageTabList	    A list of page tabs.
    \value PropertyPage	    A property page where the user can change options and settings.
    \value Indicator	    An indicator that represents a current value or item.
    \value Graphic	    A graphic or picture, e.g. an icon.
    \value StaticText	    Static text, such as labels for other controls.
    \value EditableText	    Editable text
    \value PushButton	    A push button control.
    \value CheckBox	    The object represents an option that can be checked or
			    unchecked. Some options provide a "mixed" state, e.g. neither
			    checked nor unchecked.
    \value RadioButton	    The object represents an option that is mutually exclusive
			    with other options.
    \value ComboBox	    A list of choices that the user can select from.
    \omit
    \value DropList
    \endomit
    \value ProgressBar	    The object displays the progress of an operation in progress.
    \value Dial		    The object represents a dial or knob.
    \value HotkeyField	    A hotkey field that allows the user to enter a key sequence.			    
    \value Slider	    A slider that allows the user to select a value within a
			    given range.
    \value SpinBox	    A spin box control that allows the user to enter a value within
			    a given range.
    \value Canvas	    An object displaying graphics that the user can interact with.
    \value Animation	    An object displaying an animation.
    \value Equation	    The object represents a mathematical equation.
    \value ButtonDropDown   A button that drops down a list of items.
    \value ButtonMenu	    A button that drops down a menu.
    \value ButtonDropGrid   A button that drops down a grid.
    \value Whitespace	    Blank space between other objects.
    \value Clock	    A clock displaying time
    \value UserRole	    The first value to be used for user defined roles.
*/

/*!
    \enum QAccessible::Relation

    This enum type defines bitflags that can be combined to indicate
    the relationship between two accessible objects.

    \value Unrelated	    The objects are unrelated
    \value Self		    The objects are the same
    \value Ancestor	    The first object is a parent of the second object
    \value Child	    The first object is a direct child of the second object
    \value Descendent	    The first object is an indirect child of the second object
    \value Sibling	    The objects are siblings
    \value HierarchyMask    A mask for hierarchical relationships

    \value Up		    The first object is above the second object
    \value Down		    The first object is below the second object
    \value Left		    The first object is left from the second object
    \value Right	    The first object is right from the second object
    \value Covers	    The first object covers the second object
    \value Covered	    The first object is covered by the second object
    \value GeometryMask	    A mask for geometrical relationships. 
			    Geometrical relationships are only relevant between siblings.

    \value FocusChild	    The first object is the second object's focus child
    \value Label	    The first object is the label of the second object
    \value Labelled	    The first object is labelled by the second object
    \value Controller	    The first object controls the second object
    \value Controlled	    The first object is controlled by the second object
    \value LogicalMask	    A mask for logical relationships

    Implementations of relationTo() return a combination of those flags.
    (some values are obviously mutually exclusive).

    Implementations of navigate() can accepts only one distinct value.
*/

/*!
    \enum QAccessible::Text

    This enum specifies string information that an accessible object
    returns.

    \value Name		    The name of the object
    \value Description	    A short text describing the object
    \value Value	    The value of the object
    \value Help		    A longer text giving information about how to use the object
    \value Accelerator	    The keyboard shortcut that executes the default action
    \value UserText	    The first value to be used for user defined text.
*/

/*!
    \fn void QAccessible::initialize()
    \internal
*/

/*!
    \fn void QAccessible::cleanup()
    \internal
*/

/*!
    \fn static void QAccessible::updateAccessibility( QObject *object, int child, Event reason )

    Notifies accessibility clients about a change in \a object's
    accessibility information.

    \a reason specifies the cause of the change, for example,
    ValueChange when the position of a slider has been changed. \a
    child is the (1-based) index of the child element that has changed.
    When \a child is 0, the object itself has changed.

    Call this function whenever the state of your accessible object or
    one of it's sub-elements has been changed either programmatically
    (e.g. by calling QLabel::setText()) or by user interaction.

    If there are no accessibility tools listening to this event, the
    performance penalty for calling this function is minor, but if determining
    the parameters of the call is expensive you can test isActive() to
    avoid unnecessary computations.
*/

static QPluginManager<QAccessibleFactoryInterface> *qAccessibleManager = 0;
class InterfaceCache : public QObject, public QHash<QObject*,QAccessibleInterface*>
{
    Q_OBJECT
public:
    InterfaceCache() {}

    void insert(QObject *object, QAccessibleInterface *iface);
    void remove(QObject *object);

private slots:
    void objectDestroyed(QObject *object)
    {
	QHash<QObject*, QAccessibleInterface*>::remove(object);
    }

};

inline void InterfaceCache::insert(QObject *object, QAccessibleInterface *iface)
{
    if (!object || !iface)
	return;
    connect(object, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)));
    QHash<QObject*, QAccessibleInterface*>::insert(object, iface);
}

inline void InterfaceCache::remove(QObject *object)
{
    if(object) {
	object->disconnect(this);
	QHash<QObject*, QAccessibleInterface*>::remove(object);
    }
}

#include "qaccessible.moc"

static InterfaceCache *qInterfaceCache = 0;
static QList<void*> *qAccessibleFactories;
static bool cleanupAdded = FALSE;

QAccessible::UpdateHandler QAccessible::updateHandler = 0;
QAccessible::RootObjectHandler QAccessible::rootObjectHandler = 0;

static void qAccessibleCleanup()
{
    if (qInterfaceCache && qInterfaceCache->count() && qAccessibleManager)
	qAccessibleManager->setAutoUnload( FALSE );

    delete qInterfaceCache;
    qInterfaceCache = 0;
    delete qAccessibleManager;
    qAccessibleManager = 0;
    delete qAccessibleFactories;
    qAccessibleFactories = 0;
}

void qInsertAccessibleObject(QObject *object, QAccessibleInterface *iface)
{
    if ( !qInterfaceCache ) {
	qInterfaceCache = new InterfaceCache();
	if (!cleanupAdded) {
	    qAddPostRoutine(qAccessibleCleanup);
	    cleanupAdded = TRUE;
	}
    }

    qInterfaceCache->insert(object, iface);
}

void qRemoveAccessibleObject(QObject *object)
{
    if ( qInterfaceCache ) {
	qInterfaceCache->remove(object);
	Q_ASSERT(!qInterfaceCache->value(object));
	if (!qInterfaceCache->count()) {
	    delete qInterfaceCache;
	    qInterfaceCache = 0;
	}
    }
}

/*!
    \enum QAccessible::InterfaceFactory

    A function pointer type. Use a function with that prototype to install
    interface factories with installFactory.

    The function receives a QObject pointer, set the second
    parameter to the pointer of the corresponding QAccessibleInterface, and
    return TRUE, or return FALSE if it doesn't provide a QAccessibleInterface
    for the QObject.

    Installed factories are called by queryAccessibilityInterface() until
    one provides an interface.
*/

/*!
    \enum QAccessible::UpdateHandler

    A function pointer type. Use a function with that prototype to install
    your own update function.

    The function is called by updateAccessibility().
*/

/*!
    \enum QAccessible::RootObjectHandler

    A function pointer type. Use a function with that prototype to install
    your own root object handler.

    The function is called by setRootObject().
*/

/*!
    Installs the InterfaceFactory \a factory. The last factory added
    is the first one used in queryAccessibleInterface.
*/
void QAccessible::installFactory(InterfaceFactory factory)
{
    if (!factory)
	return;

    if (!qAccessibleFactories) {
	qAccessibleFactories = new QList<void*>();
	if (!cleanupAdded) {
	    qAddPostRoutine(qAccessibleCleanup);
	    cleanupAdded = TRUE;
	}
    }

    if (qAccessibleFactories->contains((void*)factory))
	return;
    qAccessibleFactories->append((void*)factory);
}

/*!
    Removes \a factory from the list of installed InterfaceFactories.
*/
void QAccessible::removeFactory(InterfaceFactory factory)
{
    if (!qAccessibleFactories || !factory)
	return;

    qAccessibleFactories->remove((void*)factory);
}

/*!
    Installs \a handler as the function to be used by updateAccessibility, and
    returns the previously installed function pointer.
*/
QAccessible::UpdateHandler QAccessible::installUpdateHandler(UpdateHandler handler)
{
    UpdateHandler old = updateHandler;
    updateHandler = handler;
    return old;
}

/*!
    Installs \a handler as the function to be used by setRootObject, and
    returns the previously installed function pointer.
*/
QAccessible::RootObjectHandler QAccessible::installRootObjectHandler(RootObjectHandler handler)
{
    RootObjectHandler old = rootObjectHandler;
    rootObjectHandler = handler;
    return old;
}

/*!
    Sets \a iface to point to the implementation of the
    QAccessibleInterface for \a object, and returns TRUE if
    successful, or sets \a iface to 0 and returns FALSE if
    no accessibility implementation for \a object exists.

    The function calls all installed factory functions (in reverse
    order) until one factory provides an interface for the class of
    \a object. If no factory can provide an accessibility implementation
    for the class the function loads installed accessibility plugins and tests
    if one plugin can provide the implementation.

    If no implementation for the object's class is available the function tries to
    find an implementation for the object's parent class, using above strategy.

    The caller has to call release() on the interface returned in \a iface.
*/
bool QAccessible::queryAccessibleInterface( QObject *object, QAccessibleInterface **iface )
{
    *iface = 0;
    if ( !object )
	return FALSE;

    QEvent e(QEvent::Accessibility);
    QApplication::sendEvent(object, &e);

    if (qInterfaceCache) {
	*iface = qInterfaceCache->value(object);
	if ( *iface ) {
	    if ((*iface)->isValid()) {
		(*iface)->addRef();
		return TRUE;
	    } else {
		(*iface)->release();
		qRemoveAccessibleObject(object);
	    }
	}
    }

    QInterfacePtr<QAccessibleFactoryInterface> factory = 0;
    const QMetaObject *mo = object->metaObject();
    while ( mo ) {
	const QString cn(mo->className());
	if (qAccessibleFactories) {
	    for (int i = qAccessibleFactories->count(); i > 0; --i) {
		InterfaceFactory factory = (InterfaceFactory)qAccessibleFactories->at(i-1);
		QAccessibleInterface *aiface = factory(cn, object);
		if (aiface) {
		    aiface->addRef();
		    *iface = aiface;
		    qInsertAccessibleObject(object, *iface);
		    return TRUE;
		}
	    }
	}
	if ( !qAccessibleManager ) {
	    qAccessibleManager = new QPluginManager<QAccessibleFactoryInterface>
		(IID_QAccessibleFactory, QApplication::libraryPaths(), "/accessible");
	    if ( !cleanupAdded ) {
		qAddPostRoutine( qAccessibleCleanup );
		cleanupAdded = TRUE;
	    }
	}
	qAccessibleManager->queryInterface( mo->className(), &factory );
	if ( factory ) {
	    factory->createAccessibleInterface( cn, object, iface );
	    if (*iface)
		return TRUE;
	}
	mo = mo->superClass();
    }

    QWidget *widget = qt_cast<QWidget*>(object);
    if (widget)
	*iface = new QAccessibleWidget(widget);
    else if (object == qApp)
	*iface = new QAccessibleApplication();
    else
	return FALSE;

    (*iface)->addRef();
    return TRUE;
}

/*!
    Returns TRUE if an accessibility implementation has been requested,
    during the runtime of the application, otherwise returns FALSE.

    Use this function to prevent potentially expensive notifications via
    updateAccessibility().

    \omit
    QListView uses this function to prevent index-lookups for item based
    notifications.
    \endomit
*/
bool QAccessible::isActive()
{
    return qInterfaceCache != 0;
}

/*!
    \fn void QAccessible::setRootObject(QObject *object)

    Sets the root accessible object of this application to \a object.
    All other accessible objects in the application can be reached by the
    client using object navigation.

    You should never need to call this function. Qt sets the QApplication
    object as the root object immediately before the event loop is entered
    in QApplication::exec().

    Use installRootObjectHandler() to redirect the function call to a
    customized handler function.

    \sa RootObjectHandler, queryAccessibleInterface()
*/


/*!
    \class QAccessibleInterface qaccessible.h
    \brief The QAccessibleInterface class defines an interface that exposes information
    about accessible objects.
    \ingroup misc

    Accessibility tools (also called AT Clients, e.g. screen readers or braille displays)
    require high-level information about the accessible objects in an application to
    provide specialized output and input methods that make it possible for impaired
    users to use the application (applications providing this information are also called
    AT Servers).

    Every element that the user needs to interact with or react to is an accessible object,
    and should provide this information. These are mainly visual objects, e.g. widgets and
    widget elements, but can also be content, e.g. sounds.

    The AT client uses three basic concepts to acquire information about any accessible
    object in an application:
    \list
    \i \c Properties - The client can read information about accessible objects. In some
    cases the client can also modify those properties (ie. text in a line edit)
    \i \c Actions - The client can invoke actions of the object, e.g. press a push button
    \i \c Relations and Navigation - The client can traverse from one accessible object to
    another, using the relations between objects
    \endlist

    The QAccessibleInterface defines the API for those three concepts.

    \section2 Relations and Navigation

    The functions childCount() and indexOfChild() return the number
    of children of an accessible object and the index a child object has in its
    parent. The childAt() function returns the index of a child at a given position.
    
    The relationTo() function provides information about how two different objects
    relate to each other, and navigate() allows traversing from one object to another
    object with a given relation.

    \section2 Properties

    The central property of an accessible objects is which role() it has. Different
    objects can have the same role, e.g. both the "Add line" element in a scrollbar and
    the \c Ok button of a dialog have the same role "push buttons". The role implies
    what kind of interaction the user can perform with the user interface element.

    The state() property of an object is a combination of different state flags and
    can describe both how the object differs from a "normal" state (ie. it might be 
    unavailable), but also how it behaves, e.g. it might be selectable.

    The text() property provides textual information about the object. An object usually
    has a name, but can provide extended information like description, help texts or 
    information about keyboard accelerators is assigned to it. Some objects allow changing 
    the text() property through the setText() function, but this information is in most
    cases read-only.

    The rect() property provides information about the geometry of an accessible object.
    This information is usually only available for visual objects.

    \section2 Actions and Selection

    To enable the user to interac with an accessible object the object has to expose
    information about the actions that it can perform. numActions() returns the number
    of actions supported by an accessible object, and actionText() returns textual
    information about those actions. doAction() finally invokes an action.

    Objects that support selections can define actions to change the selection.

    \section2 Objects and children

    A QAccessibleInterface provides information about the accessible object, and
    can also provide information for the children of that object if those children
    don't provide a QAccessibleInterface implementation themselves. This is
    practical if the object has many similar children (ie. items in a listview), or
    if the children are an integral part of the object itself (ie. the different
    sections in a scrollbar).

    If an accessible object provides information about it's children through one
    QAccessibleInterface the children are referenced through indices. The index is
    1-based, e.g. 0 refers to the object itself, 1 to the first child etc.

    All functions in QAccessibleInterface that take a child index relate to the
    object itself if the index is 0, or to the child specified. If a child provides
    its own interface implementation (which can be retrieved through navigation)
    asking the parent for information about that child will usually not succeed.

    \section2 Reference counting

    The lifetime of QAccessibleInterface implementations is controlled by reference
    counting. Interfaces provided by QAccessible::queryAccessibleInterface() or
    through navigate() are referenced by the implementation, and the caller has to release()
    the interface when it is no longer in use. addRef() needs to be  called if references to
    the interface are added.

\omit
    Qt provides implementations of the QAccessibleInterface for most
    widget classes in a plugin. This plugin is located in the \e
    accessible subdirectory of the plugins installation directory.
    The default installation directory for plugins is \c INSTALL/plugins,
    where \c INSTALL is the directory where Qt was installed.  Calling
    queryAccessibleInterface( QObject *object, QAccessibleInterface
    **iface ) will ask all plugins located in this directory for an
    implementation that exposes the information for objects of the
    class of \e object.

    To make a Qt application accessible you have to distribute the
    accessibility plugin provded with Qt together with your
    application. Simply add the plugins created in
    INSTALL/plugins/accessible to your distribution process. Use \l
    QApplication::addLibraryPath() to specify a plugin directory for
    your application, and copy the files into an \e accessible
    subdirectory of one of those plugin directories. Qt's
    accessibility framework will load the plugins upon request and use
    the implementations provided to expose an object's accessibility
    information.

    See the \link plugins-howto.html plugin documentation \endlink for
    more details about how to redistribute Qt plugins.
\endomit
*/

/*!
    \fn bool QAccessibleInterface::isValid() const

    Returns TRUE if all the data necessary to use this interface
    implementation is valid (e.g. all pointers are non-null),
    otherwise returns FALSE.

    \sa object()
*/

/*!
    \fn QObject *QAccessibleInterface::object() const

    Returns a pointer to the QObject this interface implementation provides 
    information for.

    \sa isValid()
*/

/*!
    \fn int QAccessibleInterface::childCount() const

    Returns the number of children that belong to this object. A child
    can provide accessibility information on it's own (e.g. a child
    widget), or be a sub-element of this accessible object.

    All objects provide this information.

    \sa indexOfChild()
*/

/*!
    \fn int QAccessibleInterface::indexOfChild(const QAccessibleInterface *child) const

    Returns the 1-based index of the object \a child in this object's children list,
    or -1 if \a child is not a child of this object. 0 is not a possible return value.

    All objects provide this information about their children.

    \sa childCount()
*/

/*!
    \fn int QAccessibleInterface::relationTo(int child, const QAccessibleInterface *other, int otherChild) const

    Returns the relationship between this object's \a child and the \a other 's object
    \a otherChild. If \a child is zero the object's own relation is returned.

    The returned value indicates the relation of the called object to the \a other object,
    e.g. if this object is a child of \a other the return value will be \c Child.

    The return value is a combination of the bitflags in the \c QAccessible::Relation 
    enumeration.

    All objects provide this information.

    \sa indexOfChild(), navigate()
*/

/*!
    \fn int QAccessibleInterface::childAt(int x, int y) const

    Returns the 1-based index of the child that contains the screen coordinates
    (\a x, \a y). This function returns 0 if the point is positioned on the object
    itself. If the tested point is outside the boundaries of the object this
    function returns -1.

    This function is only relyable for visible objects (invisible object might
    not be layouted correctly).

    All visual objects provide this information.

    \sa rect()
*/

/*!
    \fn int QAccessibleInterface::navigate(Relation relation, int entry, QAccessibleInterface **target) const

    Navigates from this object to an object that has a relationship
    \a relation to this object, and returns the respective object in \a target.

    If an object is found \a target is set to point to the object, and the index
    of the child in \a target is returned. The return value is 0 if \a target itself
    is the requested object. \a target is set to null if this object is the target
    object (ie. the requested object is a handled by this object).

    If no object is found \a target is set to null, and the return value is -1.

    The \a entry parameter has two different meanings:
    \list
    \i Hierarchical and Logical relations -  if multiple object with the requested 
    relationship exist \a entry specifies which one to return. \a entry is 1-based,
    e.g. use 1 to get the first (and possibly only) object with the requested relationship.

    The following code demonstrates how to use this function to navigate
    to the first child of an object:

    \code
    QAccessibleInterface *child = 0;
    int targetChild = object->navigate(Child, 1, &child);
    if (child) {
        // ...
	child->release();
    }
    \endcode

    \i Geometrical relations - the index of the child from which to start navigating
    into the specified direction. \a entry can be 0 to navigate to a sibling of this
    object, or non-null to navigate within contained children that don't provide their
    own accessible information.
    \endlist

    Note that \c Descendent as a value for \a relation is not supported.

    All objects support navigation.

    \sa relationTo(), childCount()
*/

/*!
    \fn QString QAccessibleInterface::text(Text t, int child) const

    Returns the value of the text property \a t of the object or of
    the object's child if \a child is not 0.

    The \e Name is a string used by clients to identify, find or
    announce an accessible object for the user. All objects must have
    a name that is unique within their container.

    An accessible object's \e Description provides textual information
    about an object's visual appearance. The description is primarily
    used to provide greater context for low-vision or blind users, but
    is also used for context searching or other applications. Not all
    objects have a description. An "OK" button would not need a
    description, but a toolbutton that shows a picture of a smiley
    would.

    The \e Value of an accessible object represents visual information
    contained by the object, e.g. the text in a line edit. Usually,
    the value can be modified by the user. Not all objects have a
    value, e.g. static text labels don't, and some objects have a
    state that already is the value, e.g. toggle buttons.

    The \e Help text provides information about the function and
    usage of an accessible object. Not all objects provide this
    information.

    The \e Accelerator is a keyboard shortcut that activates the default
    action of the object. A keyboard shortcut is the underlined
    character in the text of a menu, menu item or control, and is
    either the character itself, or a combination of this character
    and a modifier key like ALT, CTRL or SHIFT. Command controls like
    tool buttons also have shortcut keys and usually display them in
    their tooltip.

    All objects provide a string for \e Name.

    \sa role(), state()
*/

/*!
    \fn void QAccessibleInterface::setText(Text t, int child, const QString &text)

    Sets the text property \a t of the object or of the object's
    child if \a child is not 0 to \a text.

    Note that the text properties of most objects are read-only.

    \sa text()
*/

/*!
    \fn QRect QAccessibleInterface::rect(int child) const

    Returns the geometry of the object or of the object's child if \a child
    is not 0. The geometry is in screen coordinates.

    This function is only relyable for visible objects (invisible object might
    not be layouted correctly).

    All visual objects provide this information.

    \sa childAt()
*/

/*!
    \fn QAccessible::Role QAccessibleInterface::role(int child) const

    Returns the role of the object or of the object's child if \a child
    is not 0. The role of an object is usually static.

    All accessible objects have a role.

    \sa text(), state()
*/

/*!
    \fn int QAccessibleInterface::state(int child) const

    Returns the current state of the object or of the object's child if
    \a child is not 0. The returned value is a combination of the flags in
    the QAccessible::State enumeration.

    All accessible objects have a state.

    \sa text(), role()
*/

/*!
    \fn int QAccessibleInterface::numActions(int child) const

    Returns the number of custom actions of the object or
    the object's child if \a child is not 0.

    The \c Action type enumerates predefined actions - those
    are not included in the returned value.

    \sa actionText(), doAction()
*/

/*!
    \fn QString QAccessibleInterface::actionText(int action, Text t, int child) const

    Returns the text property \a t of the action \a action supported by
    the object or the object's child if \a child is not 0.

    \sa text(), numActions()
*/

/*!
    \fn bool QAccessibleInterface::doAction(int action, int child)

    Asks the object or the object's \a child to execute \a action, and returns
    TRUE if the action could be executed, otherwise returns FALSE.

    \a action can be a predefined or a custom action.

    \sa numActions(), actionText()
*/

#endif
