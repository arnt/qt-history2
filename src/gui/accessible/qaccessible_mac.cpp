/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qaccessible.h"

#ifndef QT_NO_ACCESSIBILITY
#include "qhash.h"
#include "qset.h"
#include "qpointer.h"
#include "qapplication.h"
#include "qmainwindow.h"
#include "qdebug.h"

#include <private/qt_mac_p.h>
#include <CoreFoundation/CoreFoundation.h>

/*****************************************************************************
  Externals
 *****************************************************************************/
extern bool qt_mac_is_macsheet(const QWidget *w); //qwidget_mac.cpp
extern bool qt_mac_is_macdrawer(const QWidget *w); //qwidget_mac.cpp

/*****************************************************************************
  QAccessible Bindings
 *****************************************************************************/
//hardcoded bindings between control info and (known) QWidgets
struct QAccessibleTextBinding {
    int qt;
    CFStringRef mac;
    bool settable;
} text_bindings[][10] = {
    { { QAccessible::MenuItem, kAXMenuItemRole, false },
      { -1, 0, false }
    },
    { { QAccessible::MenuBar, kAXMenuBarRole, false },
      { -1, 0, false }
    },
    { { QAccessible::ScrollBar, kAXScrollBarRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Grip, kAXGrowAreaRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Window, kAXWindowRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Dialog, kAXWindowRole, false },
      { -1, 0, false }
    },
    { { QAccessible::AlertMessage, kAXWindowRole, false },
      { -1, 0, false }
    },
    { { QAccessible::ToolTip, kAXWindowRole, false },
      { -1, 0, false }
    },
    { { QAccessible::HelpBalloon, kAXWindowRole, false },
      { -1, 0, false }
    },
    { { QAccessible::PopupMenu, kAXMenuRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Application, kAXApplicationRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Pane, kAXGroupRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Grouping, kAXGroupRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Separator, kAXSplitterRole, false },
      { -1, 0, false }
    },
    { { QAccessible::ToolBar, kAXToolbarRole, false },
      { -1, 0, false }
    },
    { { QAccessible::PageTabList, kAXTabGroupRole, false },
      { -1, 0, false }
    },
    { { QAccessible::ButtonMenu, kAXMenuButtonRole, false },
      { -1, 0, false }
    },
    { { QAccessible::ButtonDropDown, kAXPopUpButtonRole, false },
      { -1, 0, false }
    },
    { { QAccessible::SpinBox, kAXIncrementorRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Slider, kAXSliderRole, false },
      { -1, 0, false }
    },
    { { QAccessible::ProgressBar, kAXProgressIndicatorRole, false },
      { -1, 0, false }
    },
    { { QAccessible::ComboBox, kAXPopUpButtonRole, false },
      { -1, 0, false }
    },
    { { QAccessible::RadioButton, kAXRadioButtonRole, false },
      { -1, 0, false }
    },
    { { QAccessible::CheckBox, kAXCheckBoxRole, false },
      { -1, 0, false }
    },
    { { QAccessible::StaticText, kAXStaticTextRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Table, kAXTableRole, false },
      { -1, 0, false }
    },
    { { QAccessible::StatusBar, kAXStaticTextRole, false },
      { -1, 0, false }
    },
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    { { QAccessible::Column, kAXColumnRole, false },
      { -1, 0, false }
    },
    { { QAccessible::ColumnHeader, kAXColumnRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Row, kAXRowRole, false },
      { -1, 0, false }
    },
    { { QAccessible::RowHeader, kAXRowRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Cell, kAXUnknownRole, false },
      { -1, 0, false }
    },
    { { QAccessible::PushButton, kAXButtonRole, false },
      { -1, 0, false }
    },
    { { QAccessible::EditableText, kAXTextFieldRole, true },
      { -1, 0, false }
    },
    { { QAccessible::Link, kAXTextFieldRole, false },
      { -1, 0, false }
    },
#else
    { { QAccessible::TitleBar, kAXWindowTitleRole, false },
      { -1, 0, false }
    },
    { { QAccessible::ColumnHeader, kAXTableHeaderViewRole, false },
      { -1, 0, false }
    },
    { { QAccessible::RowHeader, kAXTableHeaderViewRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Column, kAXTableColumnRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Row, kAXTableRowRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Cell, kAXOutlineCellRole, false },
      { -1, 0, false }
    },
    { { QAccessible::EditableText, kAXTextRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { -1, 0, false }
    },
    { { QAccessible::Link, kAXTextRole, false },
      { -1, 0, false }
    },
    { { QAccessible::PushButton, kAXPushButtonRole, false },
      { -1, 0, false }
    },
#endif
    { { -1, 0, false } }
};


// The root of the Qt accessible hiearchy.
static QObject *rootObject = 0;

static EventHandlerUPP applicationEventHandlerUPP = 0;
static EventTypeSpec application_events[] = {
    { kEventClassAccessibility,  kEventAccessibleGetChildAtPoint },
    { kEventClassAccessibility,  kEventAccessibleGetNamedAttribute }
};

static CFStringRef kObjectQtAccessibility = CFSTR("com.trolltech.qt.accessibility");
static EventHandlerUPP objectCreateEventHandlerUPP = 0;
static EventTypeSpec objectCreateEvents[] = {
    { kEventClassHIObject,  kEventHIObjectConstruct },
    { kEventClassHIObject,  kEventHIObjectInitialize },
    { kEventClassHIObject,  kEventHIObjectDestruct },
    { kEventClassHIObject,  kEventHIObjectPrintDebugInfo }
};

OSStatus accessibilityEventHandler(EventHandlerCallRef next_ref, EventRef event, void *data);
static EventHandlerUPP accessibilityEventHandlerUPP = 0;
static EventTypeSpec accessibilityEvents[] = {
    { kEventClassAccessibility,  kEventAccessibleGetChildAtPoint },
    { kEventClassAccessibility,  kEventAccessibleGetFocusedChild },
    { kEventClassAccessibility,  kEventAccessibleGetAllAttributeNames },
    { kEventClassAccessibility,  kEventAccessibleGetNamedAttribute },
    { kEventClassAccessibility,  kEventAccessibleSetNamedAttribute },
    { kEventClassAccessibility,  kEventAccessibleIsNamedAttributeSettable },
    { kEventClassAccessibility,  kEventAccessibleGetAllActionNames },
    { kEventClassAccessibility,  kEventAccessiblePerformNamedAction },
    { kEventClassAccessibility,  kEventAccessibleGetNamedActionDescription }
};

/*
    InterfaceItem represents one accessiblity item. It hides the fact that
    one QAccessibleInterface may represent more than one item.

    It has the same API as QAccessibleInterface, minus the child parameter
    in the funcitons.
*/
class InterfaceItem : public QAccessible
{
public: 
    InterfaceItem(QAccessibleInterface *interface, int pchild = 0)
    :interface(interface), child(pchild) {}

    ~InterfaceItem();
    
    inline QString actionText (int action, Text text) const 
    { return interface->actionText(action, text, child); }
    
    inline int childCount() const
    { return interface->childCount(); }
    
    inline void doAction(int action, const QVariantList &params = QVariantList())
    { interface->doAction(action, child, params); }
    
    InterfaceItem *navigate(RelationFlag relation, int entry) const;
   
    inline QObject * object() const
    { return interface->object(); } 
        
    inline Role role() const
    { return interface->role(child); }
    
    inline void setText(Text t, const QString &text)
    { interface->setText(t, child, text); }
    
    inline State state() const
    { return interface->state(child); }
    
    inline QString text (Text text) const
    { return interface->text(text, child); }
    
    inline int userActionCount()
    { return interface->userActionCount(child); }

    QAccessibleInterface *interface;  // The interface that handles this item.
    int child;                        // child id, positive for child items that share an interface with its parent.
};

InterfaceItem::~InterfaceItem()
{ 
    // The InterfaceItem with child == 0 owns the QAccessibleInterface.
    if (child == 0)
        delete (interface); 
}

InterfaceItem *InterfaceItem::navigate(RelationFlag relation, int entry) const
{ 
    QAccessibleInterface *child_iface = 0;
    const int status = interface->navigate(relation, entry, &child_iface);
    if (status == -1)
        return 0; // not found;
    
    // Check if target is a child of this interface.
    if (!child_iface) {
        return new InterfaceItem(interface, status);
    } else {
        // Target is child_iface or a child of that (status decides).
        return new InterfaceItem(child_iface, status);
    }
}
inline void deleteInterface(InterfaceItem *interface)
{
    delete interface;
}

/*
    AccessibleHierarchyManager holds info about all known accessibility objects:
    AXUIElementRefs, HIObjectRefs, InterfaceItem and QAccessibleInterface pointers
    
    The class can translate between an AXUIElementRef and an InterfaceItem,
    in both directions.

    AccessibleHierarchyManager also recieves QObject::destroyed signals and removes
    the accessibility info for that object.
*/
class AccessibleHierarchyManager : public QObject
{
Q_OBJECT
public:
    AccessibleHierarchyManager() {};
    ~AccessibleHierarchyManager();
    void reset();
    AXUIElementRef createElementForInterface(InterfaceItem* interface);
    void addElementInterfacePair(AXUIElementRef element, InterfaceItem* interface);
    InterfaceItem* lookup(const AXUIElementRef element);
    AXUIElementRef lookup(InterfaceItem * const interface);
    InterfaceItem* lookup(QObject * const object);
    HIObjectRef lookupHIObject(QObject * const object);
private slots:
    void objectDestroyed();
private:
    typedef QHash<HIObjectRef, QObject*> HIObjectQInterfaceHash;
    typedef QHash<QObject*, HIObjectRef> QInterfaceHIObjectHash;
    typedef QMultiHash<QObject*, InterfaceItem *> QObjectInterfaceHash;
    HIObjectQInterfaceHash hiToQ;
    QInterfaceHIObjectHash qToHi;
    QObjectInterfaceHash qobjectToInterface;
};

AccessibleHierarchyManager::~AccessibleHierarchyManager()
{
    reset();
}

/*
    Reomves all accessibility info accosiated with the sender object from the hash. 
*/
void AccessibleHierarchyManager::objectDestroyed()
{ 
    QObject * const destroyed = sender();
    const HIObjectRef hiobj = qToHi.value(destroyed);
    qToHi.remove(destroyed);
    hiToQ.remove(hiobj);
    deleteInterface(qobjectToInterface.value(destroyed));
    qobjectToInterface.remove(destroyed);
}

/*
    Removes all stored items, deletes InterfaceItems.
*/
void AccessibleHierarchyManager::reset()
{
    QObjectInterfaceHash::const_iterator i = qobjectToInterface.constBegin();
    while (i != qobjectToInterface.constEnd()) {
        deleteInterface(i.value());
        ++i;
    }
    hiToQ.clear();
    qToHi.clear();
    qobjectToInterface.clear();
}

/*
    Decides if a QAccessibleInterface is interesting from an accessibility users point of view.
*/
bool isItInteresting(QAccessibleInterface * const interface)
{
    // If the object is 0 it's probably not that intersting.
    QObject * const object = interface->object();
    if (!object)
        return false; 
    
    // Not widget = boring.
    const QWidget *widget = qobject_cast<const QWidget *>(object);
    if (!widget)
        return false; 
    
    const QString className = object->metaObject()->className();
    
    // VoiceOver focusing on tool tips can be confusing. The contents of the
    // tool tip is avalible through the description attribute anyway, so 
    // we disable accessibility for tool tips.
    if (className == QLatin1String("QTipLabel"))
        return false; 
    
    // Some roles are not interesting:
   if (interface->role(0) == QAccessible::Client || // QWidget 
       interface->role(0) == QAccessible::Border )  // QFrame 
        return false; 

    return true;
}

/*

*/
AXUIElementRef AccessibleHierarchyManager::createElementForInterface(InterfaceItem* interface)
{
    if (!interface)
        return 0;
    
    HIObjectRef hiobj = 0;
    const QObject *object = interface->object();
    if (object) 
        connect(object, SIGNAL(destroyed()), SLOT(objectDestroyed()));
        
    const QWidget *widget = qobject_cast<const QWidget *>(object);
    if (widget)
        hiobj = (HIObjectRef)widget->winId();

    if (!hiobj) {
        qDebug() << "creating hiobj";
        if (HIObjectCreate(kObjectQtAccessibility, 0, &hiobj) != noErr) {
            qDebug() << "qaccessible_mac: Failed to create Qt accessibility HIObject";
            return 0;
        }    
    }
    
    // If the interface is not interesting to the accessibility user we disable
    // accessibility for it it. This means that it won't show up for the user, 
    // but it is still a part of the hierarcy.
    // This also gets rid of the "empty_widget" created in QEventDispatcherMac::processEvents().
    
    HIObjectSetAccessibilityIgnored(hiobj, !isItInteresting(interface->interface));
    
    // Install accessibility event handler on object
    if (!accessibilityEventHandlerUPP)
        accessibilityEventHandlerUPP = NewEventHandlerUPP(accessibilityEventHandler);
        
    OSErr err = InstallHIObjectEventHandler(hiobj, accessibilityEventHandlerUPP,
                                        GetEventTypeCount(accessibilityEvents),
                                        accessibilityEvents, 0, 0);        
    if (err) 
        qDebug() << "qaccessible_mac: Could not install accessibility event handler"; 
    return AXUIElementCreateWithHIObjectAndIdentifier(hiobj, interface->child); 
}

void AccessibleHierarchyManager::addElementInterfacePair(AXUIElementRef element, InterfaceItem* interface)
{
    const HIObjectRef hiObject = AXUIElementGetHIObject(element);
    QObject * const qObject = interface->object();
    if (!qObject)
        return;
    if(!hiObject)
        return;
    qToHi.insert(qObject, hiObject);
    hiToQ.insert(hiObject, qObject);
    qobjectToInterface.insert(qObject, interface);
}

InterfaceItem* AccessibleHierarchyManager::lookup(const AXUIElementRef element)
{
    QObject * const object = hiToQ.value(AXUIElementGetHIObject(element));
    InterfaceItem * const interface = qobjectToInterface.value(object);
    return interface;
}

AXUIElementRef AccessibleHierarchyManager::lookup(InterfaceItem * const interface)
{
    if (!interface)
        return 0;
    else
        return AXUIElementCreateWithHIObjectAndIdentifier(qToHi.value(interface->object()), interface->child);
}

InterfaceItem* AccessibleHierarchyManager::lookup(QObject * const object)
{
    return qobjectToInterface.value(object);
}

HIObjectRef AccessibleHierarchyManager::lookupHIObject(QObject * const object)
{
    return qToHi.value(object);
}

Q_GLOBAL_STATIC(AccessibleHierarchyManager, accessibleHierarchyManager)

// Debug output helpers:
QString nameForEventKind(UInt32 kind)
{
    switch(kind) {
        case kEventAccessibleGetChildAtPoint:       return QString("GetChildAtPoint");      break;
        case kEventAccessibleGetAllAttributeNames:  return QString("GetAllAttributeNames"); break;
        case kEventAccessibleGetNamedAttribute:     return QString("GetNamedAttribute");    break;
        case kEventAccessibleSetNamedAttribute:     return QString("SetNamedAttribute");    break;
        case kEventAccessibleGetAllActionNames:     return QString("GetAllActionNames");    break;
        case kEventAccessibleGetFocusedChild:       return QString("GetFocusedChild");      break;
        default:
            return QString("Unknown accessibility event type: %1").arg(kind);
        break;
    };
}

static bool qt_mac_append_cf_uniq(CFMutableArrayRef array, CFTypeRef value)
{
    CFRange range;
    range.location = 0;
    range.length = CFArrayGetCount(array);
    if(!CFArrayContainsValue(array, range, value)) {
        CFArrayAppendValue(array, value);
        return true;
    }
    return false;
}

/*
    Gets the AccessibleObject paramter from an event.
*/
inline AXUIElementRef getAccessibleObjectParameter(EventRef event)
{
        AXUIElementRef element;
        GetEventParameter(event, kEventParamAccessibleObject, typeCFTypeRef, 0,
                          sizeof(element), 0, &element);
        return element;
}

/*
    Returns an AXUIElementRef for the given child index of interface. Creates the element
    if neccesary. Returns 0 if there is no child at that index. childIndex is 1-based.
*/
AXUIElementRef lookupCreateChild(InterfaceItem * const interface, const int childIndex)
{
    InterfaceItem * const child_iface = interface->navigate(QAccessible::Child, childIndex);
    if (!child_iface)
        return 0;
    AXUIElementRef childElement = accessibleHierarchyManager()->lookup(child_iface);
    if (!childElement) {
        childElement = accessibleHierarchyManager()->createElementForInterface(child_iface);
        accessibleHierarchyManager()->addElementInterfacePair(childElement, child_iface); 
    } else {
        deleteInterface(child_iface);
    } 
    return childElement;
}

void createElementsForRootInterface()
{
    QAccessibleInterface *appInterface = QAccessible::queryAccessibleInterface(rootObject);
    if (!appInterface)
        return;

    InterfaceItem appInterfaceItem(appInterface, 0); //NB deletes appInterface in destructor
    
    // Add the children of the root accessiblity interface
    const int childCount = appInterface->childCount();
    for (int i = 0; i < childCount; ++i)
        lookupCreateChild(&appInterfaceItem, i + 1);
}

/*
    The application event handler makes sure that all top-level qt windows are registered
    before any accessibility events are handeled.
*/
OSStatus applicationEventHandler(EventHandlerCallRef next_ref, EventRef event, void *data)
{
    Q_UNUSED(data);
    createElementsForRootInterface();
    return CallNextEventHandler(next_ref, event);
}

/*
    Installs the applicationEventHandler on the application
*/
void installApplicationEventhandler()
{
    if (!applicationEventHandlerUPP)
        applicationEventHandlerUPP = NewEventHandlerUPP(applicationEventHandler);
    
    OSStatus err = InstallApplicationEventHandler(applicationEventHandlerUPP,
                            GetEventTypeCount(application_events), application_events,
                            0, 0);
    
    if (err && err != eventHandlerAlreadyInstalledErr)
        qDebug() << "qaccessible_mac: Error installing application event handler:" << err;
}

void removeEventhandler(EventHandlerUPP eventHandler)
{
    if (eventHandler) {
        DisposeEventHandlerUPP(eventHandler);
        eventHandler = 0;
    }
}


/*
    Returns the value for element by combining the QAccessibility::Checked and 
    QAccessibility::Mixed flags into an int value that the Mac accessibilty
    system understands. This works for checkboxes, radiobuttons, and the like. 
    The return values are:
    0: unchecked
    1: checked
    2: undecided
*/
int buttonValue(InterfaceItem *element)
{
    const QAccessible::State state = element->state();
    if (state & QAccessible::Mixed)
        return 2;
    else if(state & QAccessible::Checked)
        return 1;
    else 
        return 0;
}

QString getValue(InterfaceItem *interface)
{
    const QAccessible::Role role = interface->role();
    if (role == QAccessible::RadioButton || role == QAccessible::CheckBox)
        return QString::number(buttonValue(interface));
    else
        return interface->text(QAccessible::Value);
}

/*
    Translates a QAccessible::Role into a mac accessibility role
    using the text_bindings table. 
*/
CFStringRef macRoleForQtRole(QAccessible::Role role)
{
    int i = 0;
    int testRole = text_bindings[i][0].qt;
    while (testRole != -1) {
        if (testRole == role)
            return text_bindings[i][0].mac;
        ++i;
        testRole = text_bindings[i][0].qt;
    }
    return kAXUnknownRole;
}

/*
    Translates a QAccessible::Role and an attribute name into a QAccessible::Text, taking into
    account execptions listed in text_bindings.
*/
int textForRoleAndAttribute(QAccessible::Role role, CFStringRef attribute)
{
     // Search for exception, return it if found.
    int testRole = text_bindings[0][0].qt;
    int i = 0;
    while (testRole != -1) {
        if (testRole == role) {
            int j = 1;
            int qtRole = text_bindings[i][j].qt;
            CFStringRef testAttribute = text_bindings[i][j].mac;
            while (qtRole != -1) {
                if (CFStringCompare(attribute, testAttribute, 0) == kCFCompareEqualTo) {
                    return (QAccessible::Text)qtRole;
                }
                ++j;
                testAttribute = text_bindings[i][j].mac;
                qtRole = text_bindings[i][j].qt;
            }
            break;
        }
        ++i;
        testRole = text_bindings[i][0].qt;
    }

    // Return default mappping
    if (CFStringCompare(attribute, kAXTitleAttribute, 0) == kCFCompareEqualTo)
        return QAccessible::Name;
    else if (CFStringCompare(attribute, kAXValueAttribute, 0) == kCFCompareEqualTo)
        return QAccessible::Value;
    else if (CFStringCompare(attribute, kAXHelpAttribute, 0) == kCFCompareEqualTo)
        return QAccessible::Help;
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    else if (CFStringCompare(attribute, kAXDescriptionAttribute, 0) == kCFCompareEqualTo)
        return QAccessible::Description;
#endif
    else
        return -1;
}


/*
    Returns the label (buddy) interface for interface, or 0 if it has none.
*/
InterfaceItem * findLabel(InterfaceItem *interface)
{
    return interface->navigate(QAccessible::Label, 1);
}

/*
    Returns a list of interfaces this interface labels, or an empty list if it doesn't label any.
*/
QList<InterfaceItem *> findLabelled(InterfaceItem *interface)
{
    QList<InterfaceItem *> interfaceList;
    
    int count = 1;
    InterfaceItem * const labelled = interface->navigate(QAccessible::Labelled, count);
    while (labelled != 0) {
        interfaceList.append(labelled);
        ++count;
    }
    return interfaceList;
}

/*
    Tests if the given InterfaceItem has data for a mac attribute.
*/
bool supportsAttribute(CFStringRef attribute, InterfaceItem *interface)
{
    const int text = textForRoleAndAttribute(interface->role(), attribute);
    // Return true if we the attribute matched a QAccessible::Role and we get text for that role from the interface.
    if (text != -1) {
        if (text == QAccessible::Value) // Special case for Value, see getValue()
            return !getValue(interface).isEmpty();
        else
            return !interface->text((QAccessible::Text)text).isEmpty();
    }

    if (CFStringCompare(attribute, kAXChildrenAttribute,  0) == kCFCompareEqualTo) {
        if (interface->childCount() > 0)
            return true;
    }
    
    return false;
}

OSStatus getAllAttributeNames(EventRef event, InterfaceItem *interface, EventHandlerCallRef next_ref)
{
    OSStatus err = CallNextEventHandler(next_ref, event);
    if(err != noErr && err != eventNotHandledErr)
        return err;
    CFMutableArrayRef attrs = 0;
    GetEventParameter(event, kEventParamAccessibleAttributeNames, typeCFMutableArrayRef, 0,
                      sizeof(attrs), 0, &attrs);

    if (!attrs)
        return eventNotHandledErr;
    
    if (supportsAttribute(kAXTitleAttribute, interface))
        qt_mac_append_cf_uniq(attrs, kAXTitleAttribute);
    if (supportsAttribute(kAXValueAttribute, interface))
        qt_mac_append_cf_uniq(attrs, kAXValueAttribute);
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    if (supportsAttribute(kAXDescriptionAttribute, interface))
        qt_mac_append_cf_uniq(attrs, kAXDescriptionAttribute);
    if (supportsAttribute(kAXLinkedUIElementsAttribute, interface))
        qt_mac_append_cf_uniq(attrs, kAXLinkedUIElementsAttribute);
#endif
    if (supportsAttribute(kAXHelpAttribute, interface))
        qt_mac_append_cf_uniq(attrs, kAXHelpAttribute);
    if (supportsAttribute(kAXTitleUIElementAttribute, interface))
        qt_mac_append_cf_uniq(attrs, kAXTitleUIElementAttribute);
    if (supportsAttribute(kAXChildrenAttribute, interface))
        qt_mac_append_cf_uniq(attrs, kAXChildrenAttribute);
    
    qt_mac_append_cf_uniq(attrs, kAXPositionAttribute);
    qt_mac_append_cf_uniq(attrs, kAXSizeAttribute);
    qt_mac_append_cf_uniq(attrs, kAXRoleAttribute);
    qt_mac_append_cf_uniq(attrs, kAXEnabledAttribute);
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
    qt_mac_append_cf_uniq(attrs, kAXTopLevelUIElementAttribute);
#endif
    if (interface->role() == QAccessible::Window) {
        qt_mac_append_cf_uniq(attrs, kAXMainAttribute);
        qt_mac_append_cf_uniq(attrs, kAXMinimizedAttribute);
        qt_mac_append_cf_uniq(attrs, kAXCloseButtonAttribute);
        qt_mac_append_cf_uniq(attrs, kAXZoomButtonAttribute);
        qt_mac_append_cf_uniq(attrs, kAXMinimizeButtonAttribute);
        qt_mac_append_cf_uniq(attrs, kAXToolbarButtonAttribute);
        qt_mac_append_cf_uniq(attrs, kAXGrowAreaAttribute);
    }
    return noErr;
}

void mapChildrenForInterface(InterfaceItem * const interface)
{
    const int children_count = interface->childCount();
    for (int i = 0; i < children_count; ++i) {
        lookupCreateChild(interface, i + 1);
    }    
}

void handleStringAttribute(EventRef event, QAccessible::Text text, InterfaceItem *interface)
{
    QString str;
    if (text == QAccessible::Value)
        str = getValue(interface);
    else
        str = interface->text(text);
    
    if (str.isEmpty())
        return;
    CFStringRef cfstr = QCFString::toCFStringRef(str);
    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFStringRef, sizeof(cfstr), &cfstr);
}

OSStatus getNamedAttribute(EventHandlerCallRef next_ref, EventRef event, InterfaceItem *interface)
{
    CFStringRef var;
    GetEventParameter(event, kEventParamAccessibleAttributeName, typeCFStringRef, 0,
                              sizeof(var), 0, &var);

    if (CFStringCompare(var, kAXChildrenAttribute, 0) == kCFCompareEqualTo) {
        mapChildrenForInterface(interface);
        return CallNextEventHandler(next_ref, event);
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
    } else if(CFStringCompare(var, kAXTopLevelUIElementAttribute, 0) == kCFCompareEqualTo) {
        return CallNextEventHandler(next_ref, event);
#endif
    } else if(CFStringCompare(var, kAXWindowAttribute, 0) == kCFCompareEqualTo) {
        return CallNextEventHandler(next_ref, event);    
    } else if(CFStringCompare(var, kAXParentAttribute, 0) == kCFCompareEqualTo) {
        return CallNextEventHandler(next_ref, event);
    } else if (CFStringCompare(var, kAXPositionAttribute, 0) == kCFCompareEqualTo) {
        return CallNextEventHandler(next_ref, event);
    } else if (CFStringCompare(var, kAXSizeAttribute, 0) == kCFCompareEqualTo) {
        return CallNextEventHandler(next_ref, event);
    } else  if (CFStringCompare(var, kAXRoleAttribute, 0) == kCFCompareEqualTo) {
        CFStringRef role = kAXUnknownRole;
        for (int r = 0; text_bindings[r][0].qt != -1; r++) {
            if (interface->role() == (QAccessible::Role)text_bindings[r][0].qt) {
                role = text_bindings[r][0].mac;
                break;
            }
        }
        
        QWidget * const widget = qobject_cast<QWidget *>(interface->object()); 
        if (role == kAXUnknownRole && widget && widget->isWindow())
            role = kAXWindowRole;
        
        SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFStringRef,
                          sizeof(role), &role);
    
    } else if (CFStringCompare(var, kAXEnabledAttribute, 0) == kCFCompareEqualTo) {
        Boolean val = !((interface->state() & QAccessible::Unavailable));
        SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                          sizeof(val), &val);
    } else if (CFStringCompare(var, kAXExpandedAttribute, 0) == kCFCompareEqualTo) {
        Boolean val = (interface->state() & QAccessible::Expanded);
        SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                          sizeof(val), &val);
    } else if (CFStringCompare(var, kAXSelectedAttribute, 0) == kCFCompareEqualTo) {
        Boolean val = (interface->state() & QAccessible::Selection);
        SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                          sizeof(val), &val);
    } else if (CFStringCompare(var, kAXFocusedAttribute, 0) == kCFCompareEqualTo) {
        Boolean val = (interface->state() & QAccessible::Focus);
        SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                          sizeof(val), &val);
    } else if (CFStringCompare(var, kAXSelectedChildrenAttribute, 0) == kCFCompareEqualTo) {
        const int cc = interface->childCount();
        QList<AXUIElementRef> sel;
        for (int i = 1; i <= cc; ++i) {
            InterfaceItem * const child_iface = interface->navigate(QAccessible::Child, i);
            if (child_iface && child_iface->state() & QAccessible::Selected)
                sel.append(accessibleHierarchyManager()->lookup(child_iface));
            deleteInterface(child_iface);
        }
        AXUIElementRef *arr = (AXUIElementRef *)malloc(sizeof(AXUIElementRef) * sel.count());
        for(int i = 0; i < sel.count(); i++)
            arr[i] = sel[i];
        CFArrayRef cfList = CFArrayCreate(0, (const void **)arr, sel.count(), 0);
        SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFTypeRef,
                          sizeof(cfList), &cfList);
    } else if (CFStringCompare(var, kAXCloseButtonAttribute, 0) == kCFCompareEqualTo) {
        if(interface->object() && interface->object()->isWidgetType()) {
            Boolean val = true; //do we want to add a WState for this?
            SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                              sizeof(val), &val);
        }
    } else if (CFStringCompare(var, kAXZoomButtonAttribute, 0) == kCFCompareEqualTo) {
        if(interface->object() && interface->object()->isWidgetType()) {
            QWidget *widget = (QWidget*)interface->object();
            Boolean val = (widget->windowFlags() & Qt::WindowMaximizeButtonHint);
            SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                              sizeof(val), &val);
        }
    } else if (CFStringCompare(var, kAXMinimizeButtonAttribute, 0) == kCFCompareEqualTo) {
        if(interface->object() && interface->object()->isWidgetType()) {
            QWidget *widget = (QWidget*)interface->object();
            Boolean val = (widget->windowFlags() & Qt::WindowMinimizeButtonHint);
            SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                              sizeof(val), &val);
        }
    } else if (CFStringCompare(var, kAXToolbarButtonAttribute, 0) == kCFCompareEqualTo) {
        if(interface->object() && interface->object()->isWidgetType()) {
            QWidget *widget = (QWidget*)interface->object();
            Boolean val = qobject_cast<QMainWindow *>(widget) != 0;
            SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                              sizeof(val), &val);
        }
    } else if (CFStringCompare(var, kAXGrowAreaAttribute, 0) == kCFCompareEqualTo) {
        if(interface->object() && interface->object()->isWidgetType()) {
            Boolean val = true; //do we want to add a WState for this?
            SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                              sizeof(val), &val);
        }
    } else if (CFStringCompare(var, kAXMinimizedAttribute, 0) == kCFCompareEqualTo) {
        if (interface->object() && interface->object()->isWidgetType()) {
            QWidget *widget = (QWidget*)interface->object();
            Boolean val = (widget->windowState() & Qt::WindowMinimized);
            SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                              sizeof(val), &val);
        }
    } else if (CFStringCompare(var, kAXSubroleAttribute, 0) == kCFCompareEqualTo) {
        SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFStringRef,
                          sizeof(kAXUnknownSubrole), kAXUnknownSubrole);
    } else if (CFStringCompare(var, kAXRoleDescriptionAttribute, 0) == kCFCompareEqualTo) {
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
        const QAccessible::Role qtRole = interface->role();
        const CFStringRef macRole = macRoleForQtRole(qtRole);
        if (HICopyAccessibilityRoleDescription) {
            const CFStringRef roleDescription = HICopyAccessibilityRoleDescription(macRole, 0);
            SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFStringRef,
                          sizeof(roleDescription), &roleDescription);
        } else
#endif
        {
            // Just use Qt::Description on 10.3
            handleStringAttribute(event, QAccessible::Description, interface);
        }
    } else if (CFStringCompare(var, kAXTitleAttribute, 0) == kCFCompareEqualTo) {
        const QAccessible::Role role = interface->role();
        const QAccessible::Text text = (QAccessible::Text)textForRoleAndAttribute(role, var);
        handleStringAttribute(event, text, interface);
    } else if (CFStringCompare(var, kAXValueAttribute, 0) == kCFCompareEqualTo) {
        const QAccessible::Role role = interface->role();
        const QAccessible::Text text = (QAccessible::Text)textForRoleAndAttribute(role, var);
        handleStringAttribute(event, text, interface);
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    } else if (CFStringCompare(var, kAXDescriptionAttribute, 0) == kCFCompareEqualTo) {
        const QAccessible::Role role = interface->role();
        const QAccessible::Text text = (QAccessible::Text)textForRoleAndAttribute(role, var);
        handleStringAttribute(event, text, interface);
    } else if (CFStringCompare(var, kAXLinkedUIElementsAttribute, 0) == kCFCompareEqualTo) {
        return CallNextEventHandler(next_ref, event); 
#endif
    } else if (CFStringCompare(var, kAXHelpAttribute, 0) == kCFCompareEqualTo) {
        const QAccessible::Role role = interface->role();
        const QAccessible::Text text = (QAccessible::Text)textForRoleAndAttribute(role, var);
        handleStringAttribute(event, text, interface);
    } else if (CFStringCompare(var, kAXTitleUIElementAttribute, 0) == kCFCompareEqualTo) {
        return CallNextEventHandler(next_ref, event); 
    } else {
        return CallNextEventHandler(next_ref, event); 
    }
    return noErr;
}

OSStatus isNamedAttributeSettable(EventRef event, InterfaceItem *interface)
{
    CFStringRef var;
    GetEventParameter(event, kEventParamAccessibleAttributeName, typeCFStringRef, 0,
                      sizeof(var), 0, &var);
    Boolean settable = false;
    if (CFStringCompare(var, kAXFocusedAttribute, 0) == kCFCompareEqualTo) {
        settable = true;
    } else {
        for (int r = 0; text_bindings[r][0].qt != -1; r++) {
            if(interface ->role() == (QAccessible::Role)text_bindings[r][0].qt) {
                for (int a = 1; text_bindings[r][a].qt != -1; a++) {
                    if (CFStringCompare(var, text_bindings[r][a].mac, 0) == kCFCompareEqualTo) {
                        settable = text_bindings[r][a].settable;
                        break;
                    }
                }
            }
        }
    }
    SetEventParameter(event, kEventParamAccessibleAttributeSettable, typeBoolean,
                      sizeof(settable), &settable);
    return noErr;
}

OSStatus getChildAtPoint(EventHandlerCallRef next_ref, EventRef event, InterfaceItem *interface)
{
    if (interface)
        mapChildrenForInterface(interface);
    return CallNextEventHandler(next_ref, event);
}

OSStatus getAllActionNames(EventHandlerCallRef next_ref, EventRef event, InterfaceItem *interface)
{
    Q_UNUSED(next_ref);
    Q_UNUSED(event);
    const int actCount = interface->userActionCount();
    CFStringRef *arr = (CFStringRef *)malloc(sizeof(AXUIElementRef) * actCount);
    for (int i = 0; i < actCount; i++) {
        QString actName = interface->actionText(i, QAccessible::Name);
        arr[i] = QCFString::toCFStringRef(actName);
    }
    QCFType<CFArrayRef> cfList
        = CFArrayCreate(0, (const void **)arr, actCount, 0);
    SetEventParameter(event, kEventParamAccessibleActionNames, typeCFTypeRef,
                        sizeof(cfList), &cfList);
    return noErr;
}

OSStatus performNamedAction(EventHandlerCallRef next_ref, EventRef event, InterfaceItem *interface)
{
    Q_UNUSED(next_ref);
    Q_UNUSED(event);

    CFStringRef act;
    GetEventParameter(event, kEventParamAccessibleActionName, typeCFStringRef, 0,
                      sizeof(act), 0, &act);
    if(CFStringCompare(act, kAXPressAction, 0) == kCFCompareEqualTo) {
        interface->doAction(QAccessible::Press, QVariantList());
    } else if(CFStringCompare(act, kAXIncrementAction, 0) == kCFCompareEqualTo) {
        interface->doAction(QAccessible::Increase, QVariantList());
    } else if(CFStringCompare(act, kAXDecrementAction, 0) == kCFCompareEqualTo) {
        interface->doAction(QAccessible::Decrease, QVariantList());
    } else if(CFStringCompare(act, kAXConfirmAction, 0) == kCFCompareEqualTo) {
        interface->doAction(QAccessible::Accept, QVariantList());
    } else if(CFStringCompare(act, kAXPickAction, 0) == kCFCompareEqualTo) {
        interface->doAction(QAccessible::Select, QVariantList());
    } else if(CFStringCompare(act, kAXCancelAction, 0) == kCFCompareEqualTo) {
        interface->doAction(QAccessible::Cancel, QVariantList());
    } else {
        bool found_act = false;
        const int actCount = interface->userActionCount();
        const QString qAct = QCFString::toQString(act);
        for(int i = 0; i < actCount; i++) {
            if(interface->actionText(i, QAccessible::Name) == qAct) {
                interface->doAction(i, QVariantList());
                found_act = true;
                break;
            }
        }
    }
    return noErr;
}

OSStatus setNamedAttribute(EventHandlerCallRef next_ref, EventRef event, InterfaceItem *interface)
{
    Q_UNUSED(next_ref);
    Q_UNUSED(event);

    CFStringRef var;
    GetEventParameter(event, kEventParamAccessibleAttributeName, typeCFStringRef, 0,
                      sizeof(var), 0, &var);
    if(CFStringCompare(var, kAXFocusedAttribute, 0) == kCFCompareEqualTo) {
        CFTypeRef val;
        if(GetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFTypeRef, 0,
                             sizeof(val), 0, &val) == noErr) {
            if(CFGetTypeID(val) == CFBooleanGetTypeID() &&
               CFEqual(static_cast<CFBooleanRef>(val), kCFBooleanTrue)) {
                interface->doAction(QAccessible::SetFocus);
            }
        }
    } else {
        bool found = false;
        for(int r = 0; text_bindings[r][0].qt != -1; r++) {
            if(interface->role() == (QAccessible::Role)text_bindings[r][0].qt) {
                for(int a = 1; text_bindings[r][a].qt != -1; a++) {
                    if(CFStringCompare(var, text_bindings[r][a].mac, 0) == kCFCompareEqualTo) {
                        if(!text_bindings[r][a].settable) {
                        } else {
                            CFTypeRef val;
                            if(GetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFTypeRef, 0,
                                                 sizeof(val), 0, &val) == noErr) {
                                if(CFGetTypeID(val) == CFStringGetTypeID())
                                    interface->setText((QAccessible::Text)text_bindings[r][a].qt,
                                                   QCFString::toQString(static_cast<CFStringRef>(val)));

                            }
                        }
                        found = true;
                        break;
                    }
                }
                break;
            }
        }
    }
    return noErr;
}

/*
    This is the main accessibility event handler.
*/
OSStatus accessibilityEventHandler(EventHandlerCallRef next_ref, EventRef event, void *data)
{
    Q_UNUSED(next_ref)
    Q_UNUSED(data)
    
    // Return if this event is not a AccessibleGetNamedAttribute event.
    const UInt32 eclass = GetEventClass(event);
    if (eclass != kEventClassAccessibility)
        return eventNotHandledErr;
    
    // Get the AXUIElementRef and InterfaceItem pointer
    AXUIElementRef element = 0;
    GetEventParameter(event, kEventParamAccessibleObject, typeCFTypeRef, 0, sizeof(element), 0, &element);
    InterfaceItem * const interface = accessibleHierarchyManager()->lookup(element);
    if (!interface)
        return eventNotHandledErr;
        
    const UInt32 ekind = GetEventKind(event);
    OSStatus status = noErr;
    switch (ekind) {
        case kEventAccessibleGetAllAttributeNames:
             status = getAllAttributeNames(event, interface, next_ref);
        break;
        case kEventAccessibleGetNamedAttribute:
             status = getNamedAttribute(next_ref, event, interface);
        break;
        case kEventAccessibleIsNamedAttributeSettable:
             status = isNamedAttributeSettable(event, interface);
        break;
        case kEventAccessibleGetChildAtPoint:
            status = getChildAtPoint(next_ref, event, interface);
        break;
        case kEventAccessibleGetAllActionNames:
            status = getAllActionNames(next_ref, event, interface);
        break;
        case kEventAccessibleGetFocusedChild:
            status = CallNextEventHandler(next_ref, event);
        break;
        case kEventAccessibleSetNamedAttribute:
            status = setNamedAttribute(next_ref, event, interface);
        break;
        case kEventAccessiblePerformNamedAction:
            status = performNamedAction(next_ref, event, interface);
        break;
        default:
            status = CallNextEventHandler(next_ref, event);
        break;
    };
    return status;
}

OSStatus objectCreateEventHandler(EventHandlerCallRef next_ref, EventRef event, void *data)
{
    Q_UNUSED(data)
    Q_UNUSED(event)
    Q_UNUSED(next_ref)
    return noErr;
}

/*
    Registers the HIObject subclass used by the Qt mac accessibility framework.
*/
void registerQtAccessibilityHIObjectSubclass()
{
    if (!objectCreateEventHandlerUPP) 
        objectCreateEventHandlerUPP = NewEventHandlerUPP(objectCreateEventHandler);
    OSStatus err = HIObjectRegisterSubclass(kObjectQtAccessibility, 0, 0, objectCreateEventHandlerUPP,
                                         GetEventTypeCount(objectCreateEvents), objectCreateEvents, 0, 0);
    if (err && err != hiObjectClassExistsErr)
        qDebug() << "Error registreing subclass" << err;
}

void QAccessible::initialize()
{
    // Return if mac accessibility is not enabled.
    if (!AXAPIEnabled())
        return;
    
    registerQtAccessibilityHIObjectSubclass();
}

// Sets thre root object for the application
void QAccessible::setRootObject(QObject *object)
{
    // Call installed root object handler if we have one
    if (rootObjectHandler) {
        rootObjectHandler(object);
        return;
    }
    
    rootObject = object;
    
    installApplicationEventhandler();
}

void QAccessible::cleanup()
{
    accessibleHierarchyManager()->reset();
    removeEventhandler(applicationEventHandlerUPP);
    removeEventhandler(objectCreateEventHandlerUPP);
    removeEventhandler(accessibilityEventHandlerUPP);
}

void QAccessible::updateAccessibility(QObject *object, int child, Event reason)
{
     // Call installed update handler if we have one.
    if (updateHandler) {
        updateHandler(object, child, reason);
        return;
    }
    
    // Return if the mac accessibility is not enabled.
    if(!AXAPIEnabled()) 
        return;
    
    CFStringRef notification = 0;
    if(object && object->isWidgetType() && reason == ObjectCreated) {
        notification = kAXWindowCreatedNotification;
    } else if(reason == ValueChanged) {
        notification = kAXValueChangedNotification;
    } else if(reason == MenuStart) {
        notification = kAXMenuOpenedNotification;
    } else if(reason == MenuEnd) {
        notification = kAXMenuClosedNotification;
    } else if(reason == LocationChanged) {
        notification = kAXWindowMovedNotification;
    } else if(reason == Focus) {
        if(object && object->isWidgetType()) {
            QWidget *w = static_cast<QWidget*>(object);
            if(w->isWindow())
                notification = kAXFocusedWindowChangedNotification;
            else
                notification = kAXFocusedUIElementChangedNotification;
        }
    }

    if (!notification)
        return;

    const HIObjectRef hiObject = accessibleHierarchyManager()->lookupHIObject(object);
    if (!hiObject)
        return;

    AXNotificationHIObjectNotify(notification, hiObject, (UInt64)child);
}

#include "qaccessible_mac.moc"
#endif
