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
#include <private/qt_mac_p.h>
#include "qhash.h"
#include "qpointer.h"
#include "qapplication.h"
#include "qmainwindow.h"
#include <CoreFoundation/CoreFoundation.h>

/*****************************************************************************
  QAccessible debug facilities
 *****************************************************************************/
//#define DEBUG_DROPPED_ACCESSIBILITY
//#define DEBUG_ACCESSIBILITY

/*****************************************************************************
  QAccessible globals
 *****************************************************************************/
static EventHandlerRef access_proc_handler = 0;
static HIObjectClassRef accessibility_class = 0;
static EventHandlerUPP access_proc_handlerUPP = 0;
static CFStringRef kObjectQtAccessibility = CFSTR("com.trolltech.qt.accessibility");
static EventTypeSpec qaccess_events[] = {
    { kEventClassHIObject,  kEventHIObjectConstruct },
    { kEventClassHIObject,  kEventHIObjectInitialize },
    { kEventClassHIObject,  kEventHIObjectDestruct },
    { kEventClassHIObject,  kEventHIObjectPrintDebugInfo }
};
static EventTypeSpec hiobject_events[] = {
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

/*****************************************************************************
  Externals
 *****************************************************************************/
extern bool qt_mac_is_macsheet(const QWidget *w); //qwidget_mac.cpp
extern bool qt_mac_is_macdrawer(const QWidget *w); //qwidget_mac.cpp

/*****************************************************************************
  QAccessible member functions
 *****************************************************************************/
class QAccessibleInterfaceWrapper : private QObject
{
    Q_OBJECT

private:
    HIObjectRef hiobject;
    QAccessibleInterface *iface;
    QHash<int, AXUIElementRef> children;

public:
    void setInterface(QAccessibleInterface *i) {
        iface = i;
        if(iface->object())
            QObject::connect(iface->object(), SIGNAL(destroyed()), this, SLOT(objectDestroyed()));
    }
    HIObjectRef object() const { return hiobject; }
    QAccessibleInterface *interface() const { return iface; }
    AXUIElementRef child(int c) {
        if(!children.contains(c)) {
            AXUIElementRef ret = AXUIElementCreateWithHIObjectAndIdentifier(hiobject, c);
#ifdef DEBUG_ACCESSIBILITY
            qDebug("%p::%p) creating for %s(%d) %p", this, iface,
                   iface->object() ? iface->object()->metaObject()->className() : "*none*",
                   c, ret);
#endif
            children.insert(c, ret);
            return ret;
        }
        return children.value(c);
    }

    QAccessibleInterfaceWrapper(HIObjectRef h) : hiobject(h), iface(0) { }
    ~QAccessibleInterfaceWrapper();

private slots:
    void objectDestroyed() { deleteLater(); }
};
#include "qaccessible_mac.moc"

typedef QHash<QAccessibleInterface *, QAccessibleInterfaceWrapper *> AccessibilityHash;
Q_GLOBAL_STATIC(AccessibilityHash, qt_mac_access_iface_map)

QAccessibleInterfaceWrapper::~QAccessibleInterfaceWrapper() {
    qt_mac_access_iface_map()->remove(iface);
    for(QHash<int, AXUIElementRef>::iterator it = children.begin(); it != children.end(); ++it)
        CFRelease(*it);
    children.clear();
    delete iface;
//    CFRelease(hiobject);
}

enum {
    kEventParamQAccessiblityInterface = 'qacc',   /* typeQAccessibleInterface */
    typeQAccessibleInterface = 1  /* QAccessibleInterface *  */
};


/*****************************************************************************
  QAccessible utility functions
 *****************************************************************************/

static QAccessibleInterface *qt_mac_find_access_interface(AXUIElementRef element, int *child=0)
{
    HIObjectRef objref = AXUIElementGetHIObject(element);
    QAccessibleInterfaceWrapper *wrap = 0;
    if(!wrap) {
        AccessibilityHash *hash = qt_mac_access_iface_map();
        for(AccessibilityHash::iterator it = hash->begin(); it != hash->end(); ++it) {
            if((*it)->object() == objref) {
                wrap = (*it);
                break;
            }
        }
    }
    if(wrap) {
        if(child) { //lookup the child
            UInt64 id;
            AXUIElementGetIdentifier(element, &id);
            *child = id;
        }
        return wrap->interface();
    }
    if(child)
        *child = -1;
    return 0;
}
static AXUIElementRef qt_mac_find_uielement(QAccessibleInterface *&iface, int child=0)
{
    QAccessibleInterfaceWrapper *iface_wrap = qt_mac_access_iface_map()->value(iface);
    if(!iface_wrap && iface->object()) {
        AccessibilityHash *hash = qt_mac_access_iface_map();
        for(AccessibilityHash::iterator it = hash->begin(); it != hash->end(); ++it) {
            if((*it)->interface()->object() == iface->object()) {
                delete iface;
                iface_wrap = (*it);
                iface = iface_wrap->interface();
                break;
            }
        }
    }

    if(!iface_wrap) {
        HIObjectRef hiobj = 0;
        if(0 && iface->object() && iface->object()->isWidgetType()) {
            hiobj = (HIObjectRef)static_cast<QWidget*>(iface->object())->winId();
            iface_wrap = new QAccessibleInterfaceWrapper(hiobj);
        } else {
            if(!accessibility_class)
                HIObjectRegisterSubclass(kObjectQtAccessibility, 0, 0, access_proc_handlerUPP,
                                         GetEventTypeCount(qaccess_events), qaccess_events, 0,
                                         &accessibility_class);
            if(HIObjectCreate(kObjectQtAccessibility, 0, &hiobj) != noErr)
                hiobj = 0;
            if(hiobj)
                iface_wrap = (QAccessibleInterfaceWrapper*)HIObjectDynamicCast(hiobj, kObjectQtAccessibility);
        }
        if(hiobj) {
            Q_ASSERT(iface_wrap);
            iface_wrap->setInterface(iface);
            qt_mac_access_iface_map()->insert(iface, iface_wrap);

            HIObjectSetAccessibilityIgnored(hiobj, false);
            InstallHIObjectEventHandler(hiobj, access_proc_handlerUPP,
                                        GetEventTypeCount(hiobject_events),
                                        hiobject_events, 0, 0);
        }
    }
    if(iface_wrap)
        return iface_wrap->child(child);
    return 0;
}
static AXUIElementRef qt_mac_find_uielement(QObject *object, int child=0)
{
    AccessibilityHash *hash = qt_mac_access_iface_map();
    for(AccessibilityHash::iterator it = hash->begin(); it != hash->end(); ++it) {
        if((*it)->interface()->object() == object)
            return (*it)->child(child);
    }
    if(QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(object))
        return qt_mac_find_uielement(iface, child);
    return 0;
}

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
      { QAccessible::Name, kAXTitleAttribute, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::MenuBar, kAXMenuBarRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::ScrollBar, kAXScrollBarRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::Grip, kAXGrowAreaRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::Window, kAXWindowRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::Dialog, kAXWindowRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::AlertMessage, kAXWindowRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::ToolTip, kAXWindowRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::HelpBalloon, kAXWindowRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::PopupMenu, kAXMenuRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::Application, kAXApplicationRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::Pane, kAXGroupRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::Grouping, kAXGroupRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::Separator, kAXSplitterRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::ToolBar, kAXToolbarRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::PageTabList, kAXTabGroupRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::ButtonMenu, kAXMenuButtonRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::ButtonDropDown, kAXPopUpButtonRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::SpinBox, kAXIncrementorRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::Slider, kAXSliderRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::ProgressBar, kAXProgressIndicatorRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::ComboBox, kAXPopUpButtonRole, false },
      { QAccessible::Name, kAXTitleAttribute, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::RadioButton, kAXRadioButtonRole, false },
      { QAccessible::Name, kAXTitleAttribute, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::CheckBox, kAXCheckBoxRole, false },
      { QAccessible::Name, kAXTitleAttribute, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::StaticText, kAXStaticTextRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::Table, kAXTableRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::StatusBar, kAXStaticTextRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    { { QAccessible::Column, kAXColumnRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::ColumnHeader, kAXColumnRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::Row, kAXRowRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::RowHeader, kAXRowRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::Cell, kAXUnknownRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::PushButton, kAXButtonRole, false },
      { QAccessible::Name, kAXTitleAttribute, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::EditableText, kAXTextFieldRole, true },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::Link, kAXTextFieldRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
#else
    { { QAccessible::TitleBar, kAXWindowTitleRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::ColumnHeader, kAXTableHeaderViewRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::RowHeader, kAXTableHeaderViewRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::Column, kAXTableColumnRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::Row, kAXTableRowRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::Cell, kAXOutlineCellRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::EditableText, kAXTextRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::Link, kAXTextRole, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::PushButton, kAXPushButtonRole, false },
      { QAccessible::Name, kAXTitleAttribute, false },
      { QAccessible::Value, kAXValueAttribute, false },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, 0, false }
    },
#endif
    { { -1, 0, false } }
};

void QAccessible::initialize()
{
    if(!AXAPIEnabled()) //no point in any of this code..
        return;

    if(!access_proc_handler) {
        access_proc_handlerUPP = NewEventHandlerUPP(QAccessible::globalEventProcessor);
        InstallEventHandler(GetApplicationEventTarget(), access_proc_handlerUPP,
                            GetEventTypeCount(hiobject_events), hiobject_events,
                            0, &access_proc_handler);
    }
}
void QAccessible::setRootObject(QObject *o)
{
    if (rootObjectHandler) {
        rootObjectHandler(o);
        return;
    }
}
void QAccessible::cleanup()
{
    if(access_proc_handler) {
        RemoveEventHandler(access_proc_handler);
        access_proc_handler = 0;
    }
    if(access_proc_handlerUPP) {
        DisposeEventHandlerUPP(access_proc_handlerUPP);
        access_proc_handlerUPP = 0;
    }
}

void QAccessible::updateAccessibility(QObject *object, int control, Event reason)
{
    if (updateHandler) {
        updateHandler(object, control, reason);
        return;
    }
    if(!AXAPIEnabled()) //no point in any of this code..
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
    if(notification) {
        if(AXUIElementRef access = qt_mac_find_uielement(object, control)) {

#ifdef DEBUG_ACCESSIBILITY
            qDebug("Sending notification %d(%s) to %p[%d]", reason,
                   QCFString::toQString(notification).toLatin1().constData(),
                   access, control);
#endif
            AXNotificationHIObjectNotify(notification, AXUIElementGetHIObject(access), (UInt64)control);
        }
    }
}

static bool qt_mac_append_cf_uniq(CFMutableArrayRef array, CFStringRef value)
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

OSStatus
QAccessible::globalEventProcessor(EventHandlerCallRef next_ref, EventRef event, void *data)
{
    bool handled_event = true;
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    switch(eclass) {
    case kEventClassAccessibility: {
        AXUIElementRef req_element;
        GetEventParameter(event, kEventParamAccessibleObject, typeCFTypeRef, 0,
                          sizeof(req_element), 0, &req_element);
        int req_child = 0;
        QAccessibleInterface *req_iface = qt_mac_find_access_interface(req_element, &req_child);
#ifdef DEBUG_ACCESSIBILITY
        qDebug("handling %d :: %p(%p), %d [%s]", (int)ekind, req_iface, req_element, req_child,
               (req_iface && req_iface->object()) ? req_iface->object()->metaObject()->className() : "*Unkonwn*");
#endif
        if(ekind == kEventAccessibleGetFocusedChild) {
            AXUIElementRef element = 0;
            if(req_iface) {
                {
                    int child = 0;
                    QAccessibleInterface *focus_iface = 0;
                    if((child = req_iface->navigate(QAccessible::FocusChild, 0, &focus_iface)) > 0) {
                        if(focus_iface)
                            element = qt_mac_find_uielement(focus_iface, 0);
                        else
                            element = qt_mac_find_uielement(req_iface, child);
                    }
                }
                if(!element) {
                    const int cc = req_iface->childCount();
                    for (int i = 1; !element && i <= cc; ++i) {
                        QAccessibleInterface *child_iface = 0;
                        int i2 = req_iface->navigate(Child, i, &child_iface);
                        if (child_iface) {
                            if(child_iface->state(0) & Focus)
                                element = qt_mac_find_uielement(child_iface, 0);
                        } else if(req_iface->state(i2) & Focus) {
                            element = qt_mac_find_uielement(req_iface, i2);
                        }
                    }
                }
            } else if(QWidget *window = QApplication::activeWindow()) {
                element = qt_mac_find_uielement(window);
            }
            if(element && element != req_element) {
#ifdef DEBUG_ACCESSIBILITY
                QAccessibleInterface *iface = qt_mac_find_access_interface(element);
                qDebug(" Returning focus child: %p [%s]", element,
                       (iface && iface->object()) ? iface->object()->metaObject()->className() : "*Unkonwn*");
#endif
                SetEventParameter(event, kEventParamAccessibleChild, typeCFTypeRef,
                                  sizeof(element), &element);
            }
        } else if(ekind == kEventAccessibleGetChildAtPoint) {
            Point where;
            GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, 0,
                              sizeof(where), 0, &where);
            AXUIElementRef element = 0;
            if(req_iface) {
                int child = req_iface->childAt(where.h, where.v);
                if(child > 0) {
                    QAccessibleInterface *child_iface = 0;
                    if((child = req_iface->navigate(QAccessible::Child, child, &child_iface)) != -1 && child_iface)
                        element = qt_mac_find_uielement(child_iface, child);
                }
            } else if(QWidget *tlw = QApplication::topLevelAt(where.h, where.v)) {
                element = qt_mac_find_uielement(tlw);
            }
            if(element != req_element) {
#ifdef DEBUG_ACCESSIBILITY
                qDebug(" Returning child at point: %p [%d,%d]", element, where.h, where.v);
#endif
                SetEventParameter(event, kEventParamAccessibleChild, typeCFTypeRef,
                                  sizeof(element), &element);
            }
        } else if(!req_iface) { //the below are not mine then..
            OSStatus err = CallNextEventHandler(next_ref, event);
            if(err != noErr)
                return err;
        } else if(ekind == kEventAccessibleGetAllAttributeNames) {
            OSStatus err = CallNextEventHandler(next_ref, event);
            if(err != noErr && err != eventNotHandledErr)
                return err;
            CFMutableArrayRef attrs;
            GetEventParameter(event, kEventParamAccessibleAttributeNames, typeCFMutableArrayRef, 0,
                              sizeof(attrs), 0, &attrs);
            for(int r = 0; text_bindings[r][0].qt != -1; r++) {
                if(req_iface->role(req_child) == (QAccessible::Role)text_bindings[r][0].qt) {
                    for(int a = 1; text_bindings[r][a].qt != -1; a++) {
                        CFArrayAppendValue(attrs, text_bindings[r][a].mac);
                    }
                    break;
                }
            }
            qt_mac_append_cf_uniq(attrs, kAXChildrenAttribute);
            qt_mac_append_cf_uniq(attrs, kAXPositionAttribute);
            qt_mac_append_cf_uniq(attrs, kAXSizeAttribute);
            qt_mac_append_cf_uniq(attrs, kAXRoleAttribute);
            qt_mac_append_cf_uniq(attrs, kAXEnabledAttribute);
            qt_mac_append_cf_uniq(attrs, kAXExpandedAttribute);
            qt_mac_append_cf_uniq(attrs, kAXSelectedAttribute);
            qt_mac_append_cf_uniq(attrs, kAXFocusedAttribute);
            qt_mac_append_cf_uniq(attrs, kAXSelectedChildrenAttribute);
            qt_mac_append_cf_uniq(attrs, kAXWindowAttribute);
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
            qt_mac_append_cf_uniq(attrs, kAXTopLevelUIElementAttribute);
#endif
            if(!req_child && req_iface->role(0) == QAccessible::Window) {
                qt_mac_append_cf_uniq(attrs, kAXMainAttribute);
                qt_mac_append_cf_uniq(attrs, kAXMinimizedAttribute);
                qt_mac_append_cf_uniq(attrs, kAXCloseButtonAttribute);
                qt_mac_append_cf_uniq(attrs, kAXZoomButtonAttribute);
                qt_mac_append_cf_uniq(attrs, kAXMinimizeButtonAttribute);
                qt_mac_append_cf_uniq(attrs, kAXToolbarButtonAttribute);
                qt_mac_append_cf_uniq(attrs, kAXGrowAreaAttribute);
            }
        } else if(ekind == kEventAccessibleGetNamedAttribute) {
            CFStringRef var;
            GetEventParameter(event, kEventParamAccessibleAttributeName, typeCFStringRef, 0,
                              sizeof(var), 0, &var);
#ifdef DEBUG_ACCESSIBILITY
            qDebug(" Looking up named attribute: %s", QCFString::toQString(var).toLatin1().constData());
#endif
            if(CFStringCompare(var, kAXChildrenAttribute, 0) == kCFCompareEqualTo) {
                const int children_count = req_child ? 0 : req_iface->childCount();
                QAccessibleInterface *child_iface;
                AXUIElementRef *children = (AXUIElementRef *)malloc(sizeof(AXUIElementRef) * children_count);
                for(int i = 0; i < children_count; i++) {
                    if(req_iface->navigate(Child, i, &child_iface) != -1 && child_iface)
                        children[i] = qt_mac_find_uielement(child_iface);
                    else
                        children[i] = qt_mac_find_uielement(req_iface, i);
#ifdef DEBUG_ACCESSIBILITY
                    qDebug("  Setting child %d/%d: %p(%p)", i, children_count, child_iface, children[i]);
#endif
                }
                CFArrayRef arr = CFArrayCreate(0, (const void **)children, children_count, 0);
                SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFMutableArrayRef,
                                  sizeof(arr), &arr);
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
            } else if(CFStringCompare(var, kAXTopLevelUIElementAttribute, 0) == kCFCompareEqualTo) {
                AXUIElementRef element = 0;
                QAccessibleInterface *iface = req_iface;
                while(iface->navigate(Ancestor, 1, &iface) != -1 && iface) {
                    if(iface->object() && iface->object()->isWidgetType() &&
                       static_cast<QWidget*>(iface->object())->isWindow()) {
                        element = qt_mac_find_uielement(iface);
                        break;
                    }
//                    delete iface;
                }
                if(element)
                    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFTypeRef,
                                      sizeof(element), &element);
#endif
            } else if(CFStringCompare(var, kAXWindowAttribute, 0) == kCFCompareEqualTo) {
                AXUIElementRef element = 0;
                QAccessibleInterface *iface = req_iface;
                while(iface->navigate(Ancestor, 1, &iface) != -1 && iface) {
                    if(iface->role(0) == QAccessible::Window ||
                       (iface->object() && iface->object()->isWidgetType() &&
                        static_cast<QWidget*>(iface->object())->isWindow()) &&
                       !qt_mac_is_macdrawer(static_cast<QWidget*>(iface->object())) &&
                       !qt_mac_is_macsheet(static_cast<QWidget*>(iface->object()))) {
                        element = qt_mac_find_uielement(iface);
                        break;
                    }
//                    delete iface;
                }
                if(element)
                    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFTypeRef,
                                      sizeof(element), &element);
            } else if(CFStringCompare(var, kAXParentAttribute, 0) == kCFCompareEqualTo) {
                AXUIElementRef element = 0;
                if(req_child) {
                    QAccessibleInterface *parent_iface;
                    if(req_iface->navigate(Self, 0, &parent_iface) != -1 && parent_iface)
                        element = qt_mac_find_uielement(parent_iface);
                } else {
                    QAccessibleInterface *parent_iface;
                    if(req_iface->navigate(Ancestor, 1, &parent_iface) != -1 && parent_iface)
                        element = qt_mac_find_uielement(parent_iface);
                }
                if(element)
                    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFTypeRef,
                                      sizeof(element), &element);
            } else if(CFStringCompare(var, kAXPositionAttribute, 0) == kCFCompareEqualTo) {
                QPoint qpoint(req_iface->rect(req_child).topLeft());
                HIPoint point;
                point.x = qpoint.x();
                point.y = qpoint.y();
                SetEventParameter(event, kEventParamAccessibleAttributeValue, typeHIPoint,
                                  sizeof(point), &point);
            } else if(CFStringCompare(var, kAXSizeAttribute, 0) == kCFCompareEqualTo) {
                QSize sz(req_iface->rect(req_child).size());
                HISize size;
                size.width = sz.width();
                size.height = sz.height();
                SetEventParameter(event, kEventParamAccessibleAttributeValue, typeHISize,
                                  sizeof(size), &size);
            } else if(CFStringCompare(var, kAXRoleAttribute, 0) == kCFCompareEqualTo) {
                CFStringRef role = kAXUnknownRole;
                for(int r = 0; text_bindings[r][0].qt != -1; r++) {
                    if(req_iface->role(req_child) == (QAccessible::Role)text_bindings[r][0].qt) {
                        role = text_bindings[r][0].mac;
                        break;
                    }
                }
#if 0
                if(role == kAXUnknownRole) {
                    qDebug("%s is unknown [%d]!!!", req_iface->object() ?
                           req_iface->object()->metaObject()->className() : "Unknown!!!",
                           req_iface->role(req_child));
                }
#endif
                if(role == kAXUnknownRole && !req_child && req_iface->object()
                   && req_iface->object()->isWidgetType()
                   && static_cast<QWidget*>(req_iface->object())->isWindow())
                    role = kAXWindowRole;
                SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFStringRef,
                                  sizeof(role), &role);
            } else if(CFStringCompare(var, kAXEnabledAttribute, 0) == kCFCompareEqualTo) {
                Boolean val = !((req_iface->state(req_child) & Unavailable));
                SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                                  sizeof(val), &val);
            } else if(CFStringCompare(var, kAXExpandedAttribute, 0) == kCFCompareEqualTo) {
                Boolean val = (req_iface->state(req_child) & Expanded);
                SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                                  sizeof(val), &val);
            } else if(CFStringCompare(var, kAXSelectedAttribute, 0) == kCFCompareEqualTo) {
                Boolean val = (req_iface->state(req_child) & Selection);
                SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                                  sizeof(val), &val);
            } else if(CFStringCompare(var, kAXFocusedAttribute, 0) == kCFCompareEqualTo) {
                Boolean val = (req_iface->state(req_child) & Focus);
                SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                                  sizeof(val), &val);
            } else if(CFStringCompare(var, kAXSelectedChildrenAttribute, 0) == kCFCompareEqualTo) {
                const int cc = req_iface->childCount();
                QList<AXUIElementRef> sel;
                for (int i = 1; i <= cc; ++i) {
                    QAccessibleInterface *child_iface = 0;
                    int i2 = req_iface->navigate(Child, i, &child_iface);
                    if (child_iface) {
                        if(child_iface->state(0) & Selected)
                            sel.append(qt_mac_find_uielement(child_iface, 0));
                        else
                            delete child_iface;
                    } else if(req_iface->state(i2) & Selected) {
                        sel.append(qt_mac_find_uielement(req_iface, i2));
                    }
                }
                AXUIElementRef *arr = (AXUIElementRef *)malloc(sizeof(AXUIElementRef) * sel.count());
                for(int i = 0; i < sel.count(); i++)
                    arr[i] = sel[i];
                CFArrayRef cfList = CFArrayCreate(0, (const void **)arr, sel.count(), 0);
                SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFTypeRef,
                                  sizeof(cfList), &cfList);
            } else if(CFStringCompare(var, kAXCloseButtonAttribute, 0) == kCFCompareEqualTo) {
                if(req_iface->object() && req_iface->object()->isWidgetType()) {
                    Boolean val = true; //do we want to add a WState for this?
                    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                                      sizeof(val), &val);
                }
            } else if(CFStringCompare(var, kAXZoomButtonAttribute, 0) == kCFCompareEqualTo) {
                if(req_iface->object() && req_iface->object()->isWidgetType()) {
                    QWidget *widget = (QWidget*)req_iface->object();
                    Boolean val = (widget->windowFlags() & Qt::WindowMaximizeButtonHint);
                    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                                      sizeof(val), &val);
                }
            } else if(CFStringCompare(var, kAXMinimizeButtonAttribute, 0) == kCFCompareEqualTo) {
                if(req_iface->object() && req_iface->object()->isWidgetType()) {
                    QWidget *widget = (QWidget*)req_iface->object();
                    Boolean val = (widget->windowFlags() & Qt::WindowMinimizeButtonHint);
                    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                                      sizeof(val), &val);
                }
            } else if(CFStringCompare(var, kAXToolbarButtonAttribute, 0) == kCFCompareEqualTo) {
                if(req_iface->object() && req_iface->object()->isWidgetType()) {
                    QWidget *widget = (QWidget*)req_iface->object();
                    Boolean val = qobject_cast<QMainWindow *>(widget) != 0;
                    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                                      sizeof(val), &val);
                }
            } else if(CFStringCompare(var, kAXGrowAreaAttribute, 0) == kCFCompareEqualTo) {
                if(req_iface->object() && req_iface->object()->isWidgetType()) {
                    Boolean val = true; //do we want to add a WState for this?
                    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                                      sizeof(val), &val);
                }
            } else if(CFStringCompare(var, kAXMinimizedAttribute, 0) == kCFCompareEqualTo) {
                if(req_iface->object() && req_iface->object()->isWidgetType()) {
                    QWidget *widget = (QWidget*)req_iface->object();
                    Boolean val = (widget->windowState() & Qt::WindowMinimized);
                    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                                      sizeof(val), &val);
                }
            } else if(CFStringCompare(var, kAXSubroleAttribute, 0) == kCFCompareEqualTo) {
                SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFStringRef,
                                  sizeof(kAXUnknownSubrole), kAXUnknownSubrole);
            } else {
                bool found = false;
                for(int r = 0; text_bindings[r][0].qt != -1; r++) {
                    if(req_iface->role(req_child) == (QAccessible::Role)text_bindings[r][0].qt) {
                        for(int a = 1; text_bindings[r][a].qt != -1; a++) {
                            if(CFStringCompare(var, text_bindings[r][a].mac, 0) == kCFCompareEqualTo) {
                                const QString qstr = req_iface->text((QAccessible::Text)text_bindings[r][a].qt,
                                                                     req_child);
                                CFStringRef cfstr = 0;
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
                                if(qstr.isEmpty() &&
                                   CFStringCompare(var, kAXRoleDescriptionAttribute, 0) == kCFCompareEqualTo)
                                    cfstr = HICopyAccessibilityRoleDescription(text_bindings[r][0].mac, 0);
                                else
#endif
                                    cfstr = QCFString::toCFStringRef(qstr);
                                if(cfstr) {
                                    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFStringRef,
                                                      sizeof(cfstr), &cfstr);
                                    found = true;
                                    break;
                                }
                            }
                        }
                        break;
                    }
                }
#ifdef DEBUG_DROPPED_ACCESSIBILITY
                if(!found)
                    qWarning("Unknown [kEventAccessibleGetNamedAttribute]: %s",
                             QCFString::toQString(var).latin1());
#endif
            }
        } else if(ekind == kEventAccessibleSetNamedAttribute) {
            CFStringRef var;
            GetEventParameter(event, kEventParamAccessibleAttributeName, typeCFStringRef, 0,
                              sizeof(var), 0, &var);
            if(CFStringCompare(var, kAXFocusedAttribute, 0) == kCFCompareEqualTo) {
                CFTypeRef val;
                if(GetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFTypeRef, 0,
                                     sizeof(val), 0, &val) == noErr) {
                    if(CFGetTypeID(val) == CFBooleanGetTypeID() &&
                       CFEqual(static_cast<CFBooleanRef>(val), kCFBooleanTrue))
                        req_iface->doAction(req_child, SetFocus);
                }
            } else {
                bool found = false;
                for(int r = 0; text_bindings[r][0].qt != -1; r++) {
                    if(req_iface->role(req_child) == (QAccessible::Role)text_bindings[r][0].qt) {
                        for(int a = 1; text_bindings[r][a].qt != -1; a++) {
                            if(CFStringCompare(var, text_bindings[r][a].mac, 0) == kCFCompareEqualTo) {
                                if(!text_bindings[r][a].settable) {
#ifdef DEBUG_DROPPED_ACCESSIBILITY
                                    qWarning("Attempt to set unknown [kEventAccessibleSetNamedAttribute]: %s",
                                             QCFString::toQString(var).latin1());
#endif
                                } else {
                                    CFTypeRef val;
                                    if(GetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFTypeRef, 0,
                                                         sizeof(val), 0, &val) == noErr) {
                                        if(CFGetTypeID(val) == CFStringGetTypeID())
                                            req_iface->setText((QAccessible::Text)text_bindings[r][a].qt,
                                                           0, QCFString::toQString(static_cast<CFStringRef>(val)));
#ifdef DEBUG_DROPPED_ACCESSIBILITY
                                        else
                                            qWarning("Unable to handle settable type: %s [%ld]",
                                                     QCFString::toQString(var).latin1(), CFGetTypeID(val));
#endif

                                    }
                                }
                                found = true;
                                break;
                            }
                        }
                        break;
                    }
                }
#ifdef DEBUG_DROPPED_ACCESSIBILITY
                if(!found)
                    qWarning("Unknown [kEventAccessibleSetNamedAttribute]: %s",
                             QCFString::toQString(var).latin1());
#endif
            }
        } else if(ekind == kEventAccessibleIsNamedAttributeSettable) {
            CFStringRef var;
            GetEventParameter(event, kEventParamAccessibleAttributeName, typeCFStringRef, 0,
                              sizeof(var), 0, &var);
            Boolean settable = false;
            if(CFStringCompare(var, kAXFocusedAttribute, 0) == kCFCompareEqualTo) {
                settable = true;
            } else {
                for(int r = 0; text_bindings[r][0].qt != -1; r++) {
                    if(req_iface->role(req_child) == (QAccessible::Role)text_bindings[r][0].qt) {
                        for(int a = 1; text_bindings[r][a].qt != -1; a++) {
                            if(CFStringCompare(var, text_bindings[r][a].mac, 0) == kCFCompareEqualTo) {
                                settable = text_bindings[r][a].settable;
                                break;
                            }
                        }
                    }
                }
            }
            SetEventParameter(event, kEventParamAccessibleAttributeSettable, typeBoolean,
                              sizeof(settable), &settable);
        } else if(ekind == kEventAccessibleGetAllActionNames) {
            const int actCount = req_iface->userActionCount(req_child);
            CFStringRef *arr = (CFStringRef *)malloc(sizeof(AXUIElementRef) * actCount);
            for(int i = 0; i < actCount; i++) {
                QString actName = req_iface->actionText(i, Name, req_child);
                arr[i] = QCFString::toCFStringRef(actName);
            }
            QCFType<CFArrayRef> cfList
                = CFArrayCreate(0, (const void **)arr, actCount, 0);
            SetEventParameter(event, kEventParamAccessibleActionNames, typeCFTypeRef,
                              sizeof(cfList), &cfList);
        } else if(ekind == kEventAccessiblePerformNamedAction) {
            CFStringRef act;
            GetEventParameter(event, kEventParamAccessibleActionName, typeCFStringRef, 0,
                              sizeof(act), 0, &act);
            if(CFStringCompare(act, kAXPressAction, 0) == kCFCompareEqualTo) {
                req_iface->doAction(req_child, Press, QVariantList());
            } else if(CFStringCompare(act, kAXIncrementAction, 0) == kCFCompareEqualTo) {
                req_iface->doAction(req_child, Increase, QVariantList());
            } else if(CFStringCompare(act, kAXDecrementAction, 0) == kCFCompareEqualTo) {
                req_iface->doAction(req_child, Decrease, QVariantList());
            } else if(CFStringCompare(act, kAXConfirmAction, 0) == kCFCompareEqualTo) {
                req_iface->doAction(req_child, Accept, QVariantList());
            } else if(CFStringCompare(act, kAXPickAction, 0) == kCFCompareEqualTo) {
                req_iface->doAction(req_child, Select, QVariantList());
            } else if(CFStringCompare(act, kAXCancelAction, 0) == kCFCompareEqualTo) {
                req_iface->doAction(req_child, Cancel, QVariantList());
            } else {
                bool found_act = false;
                const int actCount = req_iface->userActionCount(req_child);
                const QString qAct = QCFString::toQString(act);
                for(int i = 0; i < actCount; i++) {
                    if(req_iface->actionText(i, Name, req_child) == qAct) {
                        req_iface->doAction(req_child, i, QVariantList());
                        found_act = true;
                        break;
                    }
                }
#ifdef DEBUG_DROPPED_ACCESSIBILITY
                if(!found_act)
                    qWarning("Unknown [kEventAccessiblePerformNamedAction]: %s",
                             qAct.latin1());
#endif
            }
        } else if(ekind == kEventAccessibleGetNamedActionDescription) {
            CFStringRef act;
            GetEventParameter(event, kEventParamAccessibleActionName, typeCFStringRef, 0,
                              sizeof(act), 0, &act);
            QString actDesc;
            if(CFStringCompare(act, kAXPressAction, 0) == kCFCompareEqualTo) {
                actDesc = req_iface->actionText(Press, Description, 0);
            } else if(CFStringCompare(act, kAXIncrementAction, 0) == kCFCompareEqualTo) {
                actDesc = req_iface->actionText(Increase, Description, 0);
            } else if(CFStringCompare(act, kAXDecrementAction, 0) == kCFCompareEqualTo) {
                actDesc = req_iface->actionText(Decrease, Description, 0);
            } else if(CFStringCompare(act, kAXConfirmAction, 0) == kCFCompareEqualTo) {
                actDesc = req_iface->actionText(Accept, Description, 0);
            } else if(CFStringCompare(act, kAXPickAction, 0) == kCFCompareEqualTo) {
                actDesc = req_iface->actionText(Select, Description, 0);
            } else if(CFStringCompare(act, kAXCancelAction, 0) == kCFCompareEqualTo) {
                actDesc = req_iface->actionText(Cancel, Description, 0);
            } else {
                bool found_act = false;
                const QString qAct = QCFString::toQString(act);
                const int actCount = req_iface->userActionCount(req_child);
                for(int i = 0; i < actCount; i++) {
                    if(req_iface->actionText(i, Name, req_child) == qAct) {
                        actDesc = req_iface->actionText(i, Description, req_child);
                        found_act = true;
                        break;
                    }
                }
#ifdef DEBUG_DROPPED_ACCESSIBILITY
                if(!found_act)
                    qWarning("Unknown [kEventAccessibleGetNamedActionDescription]: %s",
                             qAct.latin1());
#endif
            }
            if(!actDesc.isNull()) {
                CFStringRef cfActDesc = QCFString::toCFStringRef(actDesc);
                SetEventParameter(event, kEventParamAccessibleActionDescription, typeCFStringRef,
                                  sizeof(cfActDesc), &cfActDesc);
            }
        } else {
            handled_event = false;
        }
#ifdef DEBUG_ACCESSIBILITY
        qDebug("Finished event with %d", handled_event);
#endif
        break; }
    case kEventClassHIObject: {
        QAccessibleInterfaceWrapper *wrap = (QAccessibleInterfaceWrapper*)data;
        if(ekind == kEventHIObjectConstruct) {
            HIObjectRef hiobj = 0;
            GetEventParameter(event, kEventParamHIObjectInstance, typeHIObjectRef,
                              NULL, sizeof(hiobj), NULL, &hiobj);
            wrap = new QAccessibleInterfaceWrapper(hiobj);
            SetEventParameter(event, kEventParamHIObjectInstance, typeVoidPtr, sizeof(wrap), &wrap);
        } else if(ekind == kEventHIObjectInitialize) {
        } else if(ekind == kEventHIObjectDestruct) {
            free(data);
        } else if(ekind == kEventHIObjectPrintDebugInfo) {
            if(wrap->interface()->object())
                qDebug("%s::%s", wrap->interface()->object()->metaObject()->className(),
                       wrap->interface()->object()->objectName().toLocal8Bit().constData());
        } else {
            handled_event = false;
        }
        break; }
    default:
        handled_event = false;
        break;
    }
    if(!handled_event) //let the event go through
        return eventNotHandledErr;
    return noErr; //we eat the event
}

#endif
