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

    \value Normal
    \value Unavailable
    \value Selected
    \value Focused
    \value Pressed
    \value Checked
    \value Mixed
    \value ReadOnly
    \value HotTracked
    \value DefaultButton
    \value Expanded
    \value Collapsed
    \value Busy
    \value Floating
    \value Marqueed
    \value Animated
    \value Invisible
    \value Offscreen
    \value Sizeable
    \value Moveable
    \value SelfVoicing
    \value Focusable
    \value Selectable
    \value Linked
    \value Traversed
    \value MultiSelectable
    \value ExtSelectable
    \value AlertLow
    \value AlertMedium
    \value AlertHigh
    \value Protected
    \value Valid
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

    \value NoRole
    \value TitleBar
    \value MenuBar
    \value ScrollBar
    \value Grip
    \value Sound
    \value Cursor
    \value Caret
    \value AlertMessage
    \value Window
    \value Client
    \value PopupMenu
    \value MenuItem
    \value ToolTip
    \value Application
    \value Document
    \value Pane
    \value Chart
    \value Dialog
    \value Border
    \value Grouping
    \value Separator
    \value ToolBar
    \value StatusBar
    \value Table
    \value ColumnHeader
    \value RowHeader
    \value Column
    \value Row
    \value Cell
    \value Link
    \value HelpBalloon
    \value Character
    \value List
    \value ListItem
    \value Outline
    \value OutlineItem
    \value PageTab
    \value PropertyPage
    \value Indicator
    \value Graphic
    \value StaticText
    \value EditableText
    \value PushButton
    \value CheckBox
    \value RadioButton
    \value ComboBox
    \value DropLest
    \value ProgressBar
    \value Dial
    \value HotkeyField
    \value Slider
    \value SpinBox
    \value Diagram
    \value Animation
    \value Equation
    \value ButtonDropDown
    \value ButtonMenu
    \value ButtonDropGrid
    \value Whitespace
    \value PageTabList
    \value Clock
*/

/*!
    \enum QAccessible::Relation

    This enum specifies the relationship between two objects.

    \value Unrelated	    The objects are unrelated
    \value Self		    The objects are the same
    \value Ancestor	    The first object is a parent of the
			    second object
    \value Child	    The first object is a direct child
			    of the second object
    \value Descendent	    The first object is an indirect child
			    of the second object
    \value Sibling	    The objects are siblings
    \value FocusChild	    The first object is the second object's
			    focus child
    \value Label	    The first object is the label of the
			    second object
    \value Buddy	    The second object is the label of the
			    first object
    \value Controller	    The first object controls the second object
    \value Controlled	    The first object is controlled by the the
			    second object
    \value LogicalMask	    A mask for the values above
    \value Above	    The first object is above the second object
    \value Below	    The first object is below the second object
    \value Left		    The first object is left from the second object
    \value Right	    The first object is right from the second object
    \value GeometricalMask  A mask

    relationTo() can return a combination of the different values
    (some values are obviously mutually exclusive). navigate() only
    accepts one value.
*/

/*!
    \enum QAccessible::Text

    This enum specifies string information that an accessible object
    returns.

    \value Name		    The name of the object
    \value Description	    A short text describing the object
    \value Value	    The value of the object
    \value Help		    A longer text giving information about how
			    to use the object
    \value Accelerator	    The keyboard shortcut that executes the
			    default action
*/

/*!
    \enum QAccessible::Action

    This enum specifies predefined actions that an accessible object
    can execute.

    \value NoAction	    Do nothing
    \value Press	    Press
    \value SetFocus	    Take the focus
    \value Increase	    Increase the value
    \value Decrease	    Decrease the value
    \value Accept	    Accept changes
    \value Select	    Set selection
    \value Cancel	    Discard changes

    Accessible objects can defines custom actions in addition.
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
static QHash<QObject*,QAccessibleInterface*> *qAccessibleInterface = 0;
static QList<void*> *qAccessibleFactories;
static bool cleanupAdded = FALSE;

QAccessible::UpdateHandler QAccessible::updateHandler = 0;
QAccessible::RootObjectHandler QAccessible::rootObjectHandler = 0;

static void qAccessibleCleanup()
{
    if ( qAccessibleInterface && qAccessibleInterface->count() && qAccessibleManager )
	qAccessibleManager->setAutoUnload( FALSE );

    delete qAccessibleInterface;
    qAccessibleInterface = 0;
    delete qAccessibleManager;
    qAccessibleManager = 0;
    delete qAccessibleFactories;
    qAccessibleFactories = 0;
}

void qInsertAccessibleObject(QObject *object, QAccessibleInterface *iface)
{
    if ( !qAccessibleInterface ) {
	qAccessibleInterface = new QHash<QObject*,QAccessibleInterface*>();
	if (!cleanupAdded) {
	    qAddPostRoutine(qAccessibleCleanup);
	    cleanupAdded = TRUE;
	}
    }

    qAccessibleInterface->insert(object, iface);
}

void qRemoveAccessibleObject(QObject *object)
{
    if ( qAccessibleInterface ) {
	qAccessibleInterface->remove(object);
	if (!qAccessibleInterface->count()) {
	    delete qAccessibleInterface;
	    qAccessibleInterface = 0;
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

#ifdef Q_WS_MAC
#warning "queryAccessibleObject is obsolete"
QObject *QAccessible::queryAccessibleObject(QAccessibleInterface *o)
{
    return o ? o->object() : 0;
}
#endif

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
    find an implementation for the object's parent class.

    The caller has to release the interface returned in \a iface.
*/
bool QAccessible::queryAccessibleInterface( QObject *object, QAccessibleInterface **iface )
{
    *iface = 0;
    if ( !object )
	return FALSE;

    QEvent e(QEvent::Accessibility);
    QApplication::sendEvent(object, &e);

    if ( qAccessibleInterface ) {
	*iface = qAccessibleInterface->value(object);
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
    return qAccessibleManager != 0;
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
    widget contents, but can also be content, e.g. sounds.

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

    \section2 Properties

    text(), setText(), role(), state(), rect()

    \section2 Actions

    actionCount(), defaultAction(), actionText(), doAction()
    setSelected(), clearSelection(), selection()

    \section2 Relations and Navigation

    childCount(), indexOfChild(), relationTo(), childAt(), navigate()

    \section2 Objects and children

    A QAccessibleInterface provides information about the accessible object, and
    can also provide information for the children of that object if those children
    don't provide a QAccessibleInterface implementation themselves. This is
    practical if the object has many children (ie. items in a listview), or if the
    children are an integral part of the object itself (ie. the different sections
    in a scrollbar).

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

    Returns the QObject this interface implementation provides information for.

    \sa isValid()
*/

/*!
    \fn int QAccessibleInterface::childCount() const

    Returns the number of children that belong to this object. A child
    can provide accessibility information on it's own (e.g. a child
    widget), or be a sub-element of this accessible object.

    All objects provide this information.
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
    e.g. if \a other is child of this object the return value will be \c Ancestor.

    The return value is a combination of the bitflags in the \c Relation enumeration.

    All objects provide this information.
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
*/

/*!
    \fn int QAccessibleInterface::navigate(Relation relation, int entry, QAccessibleInterface **target) const

    Navigates from this object to an object that has a relationship
    \a relation to this object.

    The \a entry parameter has two different meanings:
    \list
    \i Logical relations -  if multiple object with the requested relationship
    exist \a entry specifies which one to return. \a entry is 1-based, e.g. use 1 to
    get the first (and possibly only) object with the requested relationship.
    \i Geometrical relations - the index of the child from which to start navigating
    into the specified direction. \a entry can be 0 to navigate to a sibling of this
    object, or non-null to navigate within contained children that don't provide their
    own accessible information.
    \endlist

    If an object is found \a target is set to point to the object, and the index
    of the child in \a target is returned. The return value is 0 if \a target itself
    is the requested object. \a target is set to null if this object is the target
    object (ie. the requested object is a handled by this object).

    If no object is found \a target is set to null, and the return value is -1.

    The following code demonstrates how to use this function to navigate
    to the next sibling of an object. The example doesn't check for errors
    for simplicity.

    \code
    QAccessibleInterface *parent = 0;
    QAccessibleInterface *sibling = 0;
    int index = 0;
    object->navigate(Ancestor, 0, &parent);
    index = parent->indexOfChild(object);
    parent->navigate(Child, index + 1, &sibling);
    parent->release();
    \endcode

    Note that \c Descendent as a value for \a relation is not supported.

    All objects support navigation.
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

    \sa role(), state(), selection()

    All objects provide a string for \e Name.
*/

/*!
    \fn void QAccessibleInterface::setText(Text t, int child, const QString &text)

    Sets the text property \a t of the object or of the object's
    child if \a child is not 0 to \a text.

    Note that the text properties of most objects are read-only.
*/

/*!
    \fn QRect QAccessibleInterface::rect(int child) const

    Returns the geometry of the object or of the object's child if \a child
    is not 0. The geometry is in screen coordinates.

    This function is only relyable for visible objects (invisible object might
    not be layouted correctly).

    All visual objects provide this information.
*/

/*!
    \fn QAccessible::Role QAccessibleInterface::role(int child) const

    Returns the role of the object or of the object's child if \a child
    is not 0. The role of an object is usually static.

    All accessible objects have a role.

    \sa text(), state(), selection()
*/

/*!
    \fn QAccessible::State QAccessibleInterface::state(int child) const

    Returns the current state of the object or of the object's child if
    \a child is not 0.

    All accessible objects have a state.

    \sa text(), role(), selection()
*/

/*!
    \fn QVector<int> QAccessibleInterface::selection() const

    Returns a vector holding the indices of all selected children.

    Not all objects support selections.

    \sa text(), role(), state()
*/

/*!
    \fn bool QAccessibleInterface::setSelected( int child, bool on, bool extend )

    Sets the selection of the child specified with \a child to \a
    on. If \a extend is TRUE, all children between the focused child and
    the specified child object have their selection set to \a on.

    Returns TRUE if the selection could be set; otherwise returns
    FALSE.

    Not all objects support selections.

    \sa selection(), clearSelection()
*/

/*!
    \fn void QAccessibleInterface::clearSelection()

    Removes any selection from the object.

    \sa setSelected()
*/

/*!
    \fn int QAccessibleInterface::actionCount(int child) const

    Returns the number of custom actions of the object or
    the object's child if \a child is not 0.

    The \c Action type enumerates predefined actions - those
    are not included in the returned value.

    \sa defaultAction(), actionText()
*/

/*!
    \fn int QAccessibleInterface::defaultAction(int child) const

    Returns the ID of the default action of the object or the
    object's child if \a child is not 0.

    The returned value can be a predefined or a custom action.

    \sa actionCount(), actionText()
*/

/*!
    \fn QString QAccessibleInterface::actionText(int action, Text t, int child) const

    Returns the text property \a t of the action \a action supported by
    the object or the object's child if \a child is not 0.

    \sa text(), actionCount()
*/

/*!
    \fn bool QAccessibleInterface::doAction(int action, int child)

    Asks the object or the object's \a child to execute \a action, and returns
    TRUE if the action could be executed, otherwise returns FALSE.

    \a action can be a predefined or a custom action.

    \sa defaultAction(), actionCount(), actionText()
*/

#endif
