/****************************************************************************
**
** Implementation of QAccessible class for Mac OS
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

#include "qaccessible.h"

#ifndef QT_NO_ACCESSIBILITY
#include <private/qt_mac_p.h>
#include "qhash.h"
#include "qpointer.h"
#include "qapplication.h"

/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/
static EventHandlerRef access_proc_handler = 0;
static HIObjectClassRef accessibility_class = 0;
static EventHandlerUPP access_proc_handlerUPP = 0;
static CFStringRef kObjectQtAccessibility = CFSTR("com.trolltech.qt.accessibility");
static EventTypeSpec events[] = {
    { kEventClassHIObject,  kEventHIObjectConstruct },
    { kEventClassHIObject,  kEventHIObjectInitialize },
    { kEventClassHIObject,  kEventHIObjectDestruct },
    { kEventClassHIObject,  kEventHIObjectPrintDebugInfo },

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

struct QAccessibleObjectWrapper
{
    QPointer<QObject> object;
    AXUIElementRef access;
};

typedef QHash<QObject *, QAccessibleObjectWrapper *> ObjectHash;
Q_GLOBAL_STATIC(ObjectHash, qt_mac_object_map)

enum {
    kEventParamQObject = 'qobj',   /* typeQObject */
    typeQObject = 1  /* QObject *  */
};
static QObject *qt_mac_find_access_object(HIObjectRef objref)
{
    if(QAccessibleObjectWrapper *wrap = (QAccessibleObjectWrapper*)HIObjectDynamicCast(objref, kObjectQtAccessibility))
        return wrap->object;
    return 0;
}
static QObject *qt_mac_find_access_object(AXUIElementRef element)
{
    return qt_mac_find_access_object(AXUIElementGetHIObject(element));
}
AXUIElementRef qt_mac_find_uielement(QObject *o)
{

    QAccessibleObjectWrapper *obj_wrap = qt_mac_object_map()->value(o);
    if(!obj_wrap) {
        if(!accessibility_class) {
            OSStatus err = HIObjectRegisterSubclass(kObjectQtAccessibility, 0,
                                                    0, access_proc_handlerUPP, GetEventTypeCount(events),
                                                    events, 0, &accessibility_class);
            if(err != noErr)
                return 0;
        }
        HIObjectRef hiobj;
        EventRef init_event;
        CreateEvent(0, kEventClassHIObject, kEventHIObjectInitialize, GetCurrentEventTime(),
                    kEventAttributeUserEvent, &init_event);
        SetEventParameter(init_event, kEventParamQObject, typeQObject, sizeof(o), &o);
        if(HIObjectCreate(kObjectQtAccessibility, init_event, &hiobj) == noErr) {
            HIObjectSetAccessibilityIgnored(hiobj, false);
            AXUIElementRef ref = AXUIElementCreateWithHIObjectAndIdentifier(hiobj, (UInt32)o);
            obj_wrap = qt_mac_object_map()->value(o);
            obj_wrap->access = ref;
        }
        ReleaseEvent(init_event);
    }
    return obj_wrap ? obj_wrap->access : 0;
}

/*****************************************************************************
  Platform specific QAccessible members
 *****************************************************************************/

void QAccessible::initialize()
{
    if(!AXAPIEnabled()) //no point in any of this code..
        return;

    if(!access_proc_handler) {
        access_proc_handlerUPP = NewEventHandlerUPP(QAccessible::globalEventProcessor);
        InstallEventHandler(GetApplicationEventTarget(), access_proc_handlerUPP,
                            GetEventTypeCount(events), events, 0, &access_proc_handler);
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
    if(control != 0) {
        //need to look up the proper object..
    }
    CFStringRef notification = 0;
    if(object->isWidgetType()) {
        if(reason == ObjectCreated)
            notification = kAXWindowCreatedNotification;
    } else if(reason == ValueChanged) {
        notification = kAXValueChangedNotification;
    } else if(reason == MenuStart) {
        notification = kAXMenuOpenedNotification;
    } else if(reason == MenuEnd) {
        notification = kAXMenuClosedNotification;
    } else if(reason == LocationChanged) {
        notification = kAXWindowMovedNotification;
    }

    if(notification) {
        AXUIElementRef access = qt_mac_find_uielement(object);
        UInt64 id;
        AXUIElementGetIdentifier(access, &id);
        AXNotificationHIObjectNotify(notification, AXUIElementGetHIObject(access), id);
    }
}

struct {
    int qt;
    CFStringRef mac;
    bool settable;
} text_bindings[][10] = {
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
    { { QAccessible::MenuItem, kAXMenuItemRole, false },
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
    { { QAccessible::ComboBox, kAXComboBoxRole, false },
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

QMAC_PASCAL OSStatus
QAccessible::globalEventProcessor(EventHandlerCallRef next_ref, EventRef event, void *data)
{
    bool handled_event = true;
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    switch(eclass) {
    case kEventClassAccessibility: {
        AXUIElementRef element;
        GetEventParameter(event, kEventParamAccessibleObject, typeCFTypeRef, 0, sizeof(element), 0, &element);
        QObject *object = qt_mac_find_access_object(element);
        if(ekind == kEventAccessibleGetFocusedChild) {
            if(QWidget *widget = qApp->focusWidget()) {
                if(widget != object) {
                    AXUIElementRef element = qt_mac_find_uielement(widget);
                    SetEventParameter(event, kEventParamAccessibleChild, typeCFTypeRef, sizeof(element), &element);
                }
            }
        } else if(ekind == kEventAccessibleGetChildAtPoint) {
            Point where;
            GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, 0,
                              sizeof(where), 0, &where);
            QObject *child_object = 0;
            if(object) {
                if(QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(object)) {
                    int child = iface->childAt(where.h, where.v);
                    if(child > 0) {
                        QAccessibleInterface *child_iface;
                        if(iface->navigate(Child, child, &child_iface) != -1) {
                            if(child_iface) {
                                child_object = child_iface->object();
                                delete child_iface;
                            }
                        }
                    }
                    delete iface;
                } else if(object->isWidgetType()) { //just as a backup plan!
                    QWidget *widget = (QWidget*)object;
                    child_object = widget->childAt(widget->mapFromGlobal(QPoint(where.h, where.v)));
                }
            } else {
                child_object = QApplication::widgetAt(where.h, where.v);
            }
            if(child_object && child_object != object) {
                AXUIElementRef element = qt_mac_find_uielement(child_object);
                SetEventParameter(event, kEventParamAccessibleChild, typeCFTypeRef, sizeof(element), &element);
            }
        } else if(!object) { //the below are not mine then..
            OSStatus err = CallNextEventHandler(next_ref, event);
            if(err != noErr)
                return err;
        } else if(ekind == kEventAccessibleGetAllAttributeNames) {
            if(QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(object)) {
                CFMutableArrayRef attrs;
                GetEventParameter(event, kEventParamAccessibleAttributeNames, typeCFMutableArrayRef, 0,
                                  sizeof(attrs), 0, &attrs);
                for(int r = 0; text_bindings[r][0].qt != -1; r++) {
                    if(iface->role(0) == (QAccessible::Role)text_bindings[r][0].qt) {
                        for(int a = 1; text_bindings[r][a].qt != -1; a++) {
                            CFArrayAppendValue(attrs, text_bindings[r][a].mac);
                        }
                        break;
                    }
                }
                CFArrayAppendValue(attrs, kAXChildrenAttribute);
                CFArrayAppendValue(attrs, kAXParentAttribute);
                CFArrayAppendValue(attrs, kAXPositionAttribute);
                CFArrayAppendValue(attrs, kAXSizeAttribute);
                CFArrayAppendValue(attrs, kAXRoleAttribute);
                CFArrayAppendValue(attrs, kAXEnabledAttribute);
                CFArrayAppendValue(attrs, kAXExpandedAttribute);
                CFArrayAppendValue(attrs, kAXSelectedAttribute);
                CFArrayAppendValue(attrs, kAXFocusedAttribute);
                CFArrayAppendValue(attrs, kAXSelectedChildrenAttribute);
                if(object->isWidgetType() && ((QWidget*)object)->isTopLevel()) {
                    CFArrayAppendValue(attrs, kAXMainAttribute);
                    CFArrayAppendValue(attrs, kAXMinimizedAttribute);
                    CFArrayAppendValue(attrs, kAXCloseButtonAttribute);
                    CFArrayAppendValue(attrs, kAXZoomButtonAttribute);
                    CFArrayAppendValue(attrs, kAXMinimizeButtonAttribute);
                    CFArrayAppendValue(attrs, kAXToolbarButtonAttribute);
                    CFArrayAppendValue(attrs, kAXGrowAreaAttribute);
                }
            }
        } else if(ekind == kEventAccessibleGetNamedAttribute) {
            if(QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(object)) {
                CFStringRef var;
                GetEventParameter(event, kEventParamAccessibleAttributeName, typeCFStringRef, 0,
                                  sizeof(var), 0, &var);
                if(CFStringCompare(var, kAXChildrenAttribute, 0) == kCFCompareEqualTo) {
                    int children_count = iface->childCount();
                    QAccessibleInterface *child_iface;
                    AXUIElementRef *children = (AXUIElementRef *)malloc(sizeof(AXUIElementRef) * children_count);
                    for(int i = 0; i < children_count; i++) {
                        if(iface->navigate(Child, i, &child_iface) != -1) {
                            if(child_iface) {
                                QObject *child = child_iface->object();
                                children[i] = qt_mac_find_uielement(child);
                                delete child_iface;
                            }
                        }
                    }
                    CFArrayRef arr = CFArrayCreate(0, (const void **)children, children_count, 0);
                    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFMutableArrayRef, sizeof(arr), &arr);
                } else if(CFStringCompare(var, kAXParentAttribute, 0) == kCFCompareEqualTo) {
                    QAccessibleInterface *parent_iface;
                    if(iface->navigate(Ancestor, 1, &parent_iface) != -1) {
                        if(parent_iface) {
                            QObject *parent = parent_iface->object();
                            AXUIElementRef element = qt_mac_find_uielement(parent);
                            SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof(element), &element);
                            delete parent_iface;
                        }
                    }
                } else if(CFStringCompare(var, kAXPositionAttribute, 0) == kCFCompareEqualTo) {
                    QPoint qpoint(iface->rect(0).topLeft());
                    HIPoint point;
                    point.x = qpoint.x();
                    point.y = qpoint.y();
                    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeHIPoint, sizeof(point), &point);
                } else if(CFStringCompare(var, kAXSizeAttribute, 0) == kCFCompareEqualTo) {
                    QSize sz(iface->rect(0).size());
                    HISize size;
                    size.width = sz.width();
                    size.height = sz.height();
                    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeHISize, sizeof(size), &size);
                } else if(CFStringCompare(var, kAXRoleAttribute, 0) == kCFCompareEqualTo) {
                    CFStringRef role = kAXUnknownRole;
                    for(int r = 0; text_bindings[r][0].qt != -1; r++) {
                        if(iface->role(0) == (QAccessible::Role)text_bindings[r][0].qt) {
                            role = text_bindings[r][0].mac;
                            break;
                        }
                    }
                    if(role == kAXUnknownRole && object->isWidgetType() && static_cast<QWidget*>(object)->isTopLevel())
                        role = kAXWindowRole;
                    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFStringRef, sizeof(role), &role);
                } else if(CFStringCompare(var, kAXEnabledAttribute, 0) == kCFCompareEqualTo) {
                    Boolean val = !((iface->state(0) & Unavailable)) ? true : false;
                    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean, sizeof(val), &val);
                } else if(CFStringCompare(var, kAXExpandedAttribute, 0) == kCFCompareEqualTo) {
                    Boolean val = (iface->state(0) & Expanded) ? true : false;
                    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean, sizeof(val), &val);
                } else if(CFStringCompare(var, kAXSelectedAttribute, 0) == kCFCompareEqualTo) {
                    Boolean val = (iface->state(0) & Selection) ? true : false;
                    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean, sizeof(val), &val);
                } else if(CFStringCompare(var, kAXFocusedAttribute, 0) == kCFCompareEqualTo) {
                    Boolean val = (iface->state(0) & Focus) ? true : false;
                    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean, sizeof(val), &val);
                } else if(CFStringCompare(var, kAXSelectedChildrenAttribute, 0) == kCFCompareEqualTo) {
                    const int cc = iface->childCount();
                    QVector<int> sel(cc);
                    int selIndex = 0;
                    for (int i = 1; i <= cc; ++i) {
                        QAccessibleInterface *child_iface = 0;
                        int i2 = iface->navigate(Child, i, &child_iface);
                        bool isSelected = false;
                        if (child_iface) {
                            isSelected = child_iface->state(0) & Selected;
                            delete child_iface;
                            child_iface = 0;
                        } else {
                            isSelected = iface->state(i2) & Selected;
                        }
                        if (isSelected)
                            sel[selIndex++] = i;
                    }
                    sel.resize(selIndex);
                    AXUIElementRef *arr = (AXUIElementRef *)malloc(sizeof(AXUIElementRef) * sel.count());
                    for(int i = 0; i < sel.count(); i++) {
                        QAccessibleInterface *child_iface;
                        if(iface->navigate(Child, sel[i], &child_iface) != -1) {
                            if(child_iface) {
                                arr[i] = qt_mac_find_uielement(child_iface->object());
                                delete child_iface;
                            }
                        }
                    }
                    CFArrayRef cfList = CFArrayCreate(0, (const void **)arr, sel.count(), 0);
                    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof(cfList), &cfList);
                } else if(CFStringCompare(var, kAXMainAttribute, 0) == kCFCompareEqualTo) {
                    Boolean val = (object->isWidgetType() && qApp->mainWidget() == object) ? true : false;
                    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean, sizeof(val), &val);
                } else if(CFStringCompare(var, kAXCloseButtonAttribute, 0) == kCFCompareEqualTo) {
                    if(object->isWidgetType()) {
                        Boolean val = true; //do we want to add a WState for this?
                        SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean, sizeof(val), &val);
                    }
                } else if(CFStringCompare(var, kAXZoomButtonAttribute, 0) == kCFCompareEqualTo) {
                    if(object->isWidgetType()) {
                        QWidget *widget = (QWidget*)object;
                        Boolean val = (widget->testWFlags(Qt::WStyle_Maximize)) ? true : false;
                        SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean, sizeof(val), &val);
                    }
                } else if(CFStringCompare(var, kAXMinimizeButtonAttribute, 0) == kCFCompareEqualTo) {
                    if(object->isWidgetType()) {
                        QWidget *widget = (QWidget*)object;
                        Boolean val = (widget->testWFlags(Qt::WStyle_Minimize)) ? true : false;
                        SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean, sizeof(val), &val);
                    }
                } else if(CFStringCompare(var, kAXToolbarButtonAttribute, 0) == kCFCompareEqualTo) {
                    if(object->isWidgetType()) {
                        QWidget *widget = (QWidget*)object;
                        Boolean val = (widget->inherits("QMainWindow")) ? true : false;
                        SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean, sizeof(val), &val);
                    }
                } else if(CFStringCompare(var, kAXGrowAreaAttribute, 0) == kCFCompareEqualTo) {
                    if(object->isWidgetType()) {
                        Boolean val = true; //do we want to add a WState for this?
                        SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean, sizeof(val), &val);
                    }
                } else if(CFStringCompare(var, kAXMinimizedAttribute, 0) == kCFCompareEqualTo) {
                    if(object->isWidgetType()) {
                        QWidget *widget = (QWidget*)object;
                        Boolean val = (widget->testWState(Qt::WState_Minimized)) ? true : false;
                        SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean, sizeof(val), &val);
                    }
                } else {
                    bool found = false;
                    for(int r = 0; text_bindings[r][0].qt != -1; r++) {
                        if(iface->role(0) == (QAccessible::Role)text_bindings[r][0].qt) {
                            for(int a = 1; text_bindings[r][a].qt != -1; a++) {
                                if(CFStringCompare(var, text_bindings[r][a].mac, 0) == kCFCompareEqualTo) {
                                    const QString qstr = iface->text((QAccessible::Text)text_bindings[r][a].qt, 0);
                                    if(1 || !qstr.isNull()) {
                                        CFStringRef cfstr = QCFString::toCFStringRef(qstr);
                                        SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFStringRef,
                                                          sizeof(cfstr), &cfstr);
                                    }
                                    found = true;
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    if(!found)
                        qWarning("Unknown [kEventAccessibleGetNamedAttribute]: %s",
                               QCFString::toQString(var).latin1());
                }
                delete iface;
            }
        } else if(ekind == kEventAccessibleSetNamedAttribute) {
            if(QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(object)) {
                CFStringRef var;
                GetEventParameter(event, kEventParamAccessibleAttributeName, typeCFStringRef, 0,
                                  sizeof(var), 0, &var);
                if(CFStringCompare(var, kAXFocusedAttribute, 0) == kCFCompareEqualTo) {
                    CFTypeRef val;
                    if(GetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFTypeRef, 0,
                                         sizeof(val), 0, &val) == noErr) {
                        if(CFGetTypeID(val) == CFBooleanGetTypeID() &&
                           CFEqual(static_cast<CFBooleanRef>(val), kCFBooleanTrue))
                            iface->doAction(0, SetFocus);
                    }
                } else {
                    bool found = false;
                    for(int r = 0; text_bindings[r][0].qt != -1; r++) {
                        if(iface->role(0) == (QAccessible::Role)text_bindings[r][0].qt) {
                            for(int a = 1; text_bindings[r][a].qt != -1; a++) {
                                if(CFStringCompare(var, text_bindings[r][a].mac, 0) == kCFCompareEqualTo) {
                                    if(!text_bindings[r][a].settable) {
                                        qWarning("Attempt to set unknown [kEventAccessibleGetNamedAttribute]: %s",
                                                 QCFString::toQString(var).latin1());
                                    } else {
                                        CFTypeRef val;
                                        if(GetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFTypeRef, 0,
                                                             sizeof(val), 0, &val) == noErr) {
                                            if(CFGetTypeID(val) == CFStringGetTypeID()) 
                                                iface->setText((QAccessible::Text)text_bindings[r][a].qt,
                                                               0, QCFString::toQString(static_cast<CFStringRef>(val)));
                                            else
                                                qWarning("Unable to handle settable type: %s [%ld]", 
                                                         QCFString::toQString(var).latin1(), CFGetTypeID(val));
                                                         
                                        }
                                    }
                                    found = true;
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    if(!found)
                        qWarning("Unknown [kEventAccessibleSetNamedAttribute]: %s",
                                 QCFString::toQString(var).latin1());
                }
                delete iface;
            }
        } else if(ekind == kEventAccessibleIsNamedAttributeSettable) {
            if(QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(object)) {
                CFStringRef var;
                GetEventParameter(event, kEventParamAccessibleAttributeName, typeCFStringRef, 0,
                                  sizeof(var), 0, &var);
                Boolean settable = false;
                if(CFStringCompare(var, kAXFocusedAttribute, 0) == kCFCompareEqualTo) {
                    settable = true;
                } else {
                    for(int r = 0; text_bindings[r][0].qt != -1; r++) {
                        if(iface->role(0) == (QAccessible::Role)text_bindings[r][0].qt) {
                            for(int a = 1; text_bindings[r][a].qt != -1; a++) {
                                if(CFStringCompare(var, text_bindings[r][a].mac, 0) == kCFCompareEqualTo) {
                                    settable = text_bindings[r][a].settable;
                                    break;
                                }
                            }
                        }
                    }
                }
                SetEventParameter(event, kEventParamAccessibleAttributeSettable, typeBoolean, sizeof(settable), &settable);
                delete iface;
            }
        } else if(ekind == kEventAccessibleGetAllActionNames) {
            if(QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(object)) {
                const int actCount = iface->userActionCount(0);
                CFStringRef *arr = (CFStringRef *)malloc(sizeof(AXUIElementRef) * actCount);
                for(int i = 0; i < actCount; i++) {
                    QString actName = iface->actionText(i, Name, 0);
                    arr[i] = QCFString::toCFStringRef(actName);
                }
                QCFType<CFArrayRef> cfList
                    = CFArrayCreate(0, (const void **)arr, actCount, 0);
                SetEventParameter(event, kEventParamAccessibleActionNames, typeCFTypeRef,
                                  sizeof(cfList), &cfList);
                delete iface;
            }
        } else if(ekind == kEventAccessiblePerformNamedAction) {
            if(QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(object)) {
                CFStringRef act;
                GetEventParameter(event, kEventParamAccessibleActionName, typeCFStringRef, 0,
                                  sizeof(act), 0, &act);
                if(CFStringCompare(act, kAXPressAction, 0) == kCFCompareEqualTo) {
                    iface->doAction(0, Press, QVariantList());
                } else if(CFStringCompare(act, kAXIncrementAction, 0) == kCFCompareEqualTo) {
                    iface->doAction(0, Increase, QVariantList());
                } else if(CFStringCompare(act, kAXDecrementAction, 0) == kCFCompareEqualTo) {
                    iface->doAction(0, Decrease, QVariantList());
                } else if(CFStringCompare(act, kAXConfirmAction, 0) == kCFCompareEqualTo) {
                    iface->doAction(0, Accept, QVariantList());
                } else if(CFStringCompare(act, kAXPickAction, 0) == kCFCompareEqualTo) {
                    iface->doAction(0, Select, QVariantList());
                } else if(CFStringCompare(act, kAXCancelAction, 0) == kCFCompareEqualTo) {
                    iface->doAction(0, Cancel, QVariantList());
                } else {
                    bool found_act = false;
                    const int actCount = iface->userActionCount(0);
                    const QString qAct = QCFString::toQString(act);
                    for(int i = 0; i < actCount; i++) {
                        if(iface->actionText(i, Name, 0) == qAct) {
                            iface->doAction(0, i, QVariantList());
                            found_act = true;
                            break;
                        }
                    }
                    if(!found_act)
                        qWarning("Unknown [kEventAccessiblePerformNamedAction]: %s",
                                 qAct.latin1());
                }
                delete iface;
            }
        } else if(ekind == kEventAccessibleGetNamedActionDescription) {
            if(QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(object)) {
                CFStringRef act;
                GetEventParameter(event, kEventParamAccessibleActionName, typeCFStringRef, 0,
                                  sizeof(act), 0, &act);
                QString actDesc;
                if(CFStringCompare(act, kAXPressAction, 0) == kCFCompareEqualTo) {
                    actDesc = iface->actionText(Press, Description, 0);
                } else if(CFStringCompare(act, kAXIncrementAction, 0) == kCFCompareEqualTo) {
                    actDesc = iface->actionText(Increase, Description, 0);
                } else if(CFStringCompare(act, kAXDecrementAction, 0) == kCFCompareEqualTo) {
                    actDesc = iface->actionText(Decrease, Description, 0);
                } else if(CFStringCompare(act, kAXConfirmAction, 0) == kCFCompareEqualTo) {
                    actDesc = iface->actionText(Accept, Description, 0);
                } else if(CFStringCompare(act, kAXPickAction, 0) == kCFCompareEqualTo) {
                    actDesc = iface->actionText(Select, Description, 0);
                } else if(CFStringCompare(act, kAXCancelAction, 0) == kCFCompareEqualTo) {
                    actDesc = iface->actionText(Cancel, Description, 0);
                } else {
                    bool found_act = false;
                    const QString qAct = QCFString::toQString(act);
                    const int actCount = iface->userActionCount(0);
                    for(int i = 0; i < actCount; i++) {
                        if(iface->actionText(i, Name, 0) == qAct) {
                            actDesc = iface->actionText(i, Description, 0);
                            found_act = true;
                            break;
                        }
                    }
                    if(!found_act)
                        qWarning("Unknown [kEventAccessibleGetNamedActionDescription]: %s",
                                 qAct.latin1());
                }
                if(!actDesc.isNull()) {
                    CFStringRef cfActDesc = QCFString::toCFStringRef(actDesc);
                    SetEventParameter(event, kEventParamAccessibleActionDescription, typeCFStringRef,
                                      sizeof(cfActDesc), &cfActDesc);
                }
                delete iface;
            }
        } else {
            handled_event = false;
        }
        break; }
    case kEventClassHIObject: {
        QAccessibleObjectWrapper *wrap = (QAccessibleObjectWrapper*)data;
        if(ekind == kEventHIObjectConstruct) {
            QAccessibleObjectWrapper *w = (QAccessibleObjectWrapper*)malloc(sizeof(QAccessibleObjectWrapper));
            memset(w, '\0', sizeof(QAccessibleObjectWrapper));
            SetEventParameter(event, kEventParamHIObjectInstance, typeVoidPtr, sizeof(w), &w);
        } else if(ekind == kEventHIObjectInitialize) {
            OSStatus err = CallNextEventHandler(next_ref, event);
            if(err != noErr)
                return err;
            QObject *qobj;
            GetEventParameter(event, kEventParamQObject, typeQObject, 0, sizeof(qobj), 0, &qobj);
            QAccessibleObjectWrapper *wrap = (QAccessibleObjectWrapper*)data;
            wrap->object = qobj;
            qt_mac_object_map()->insert(wrap->object, wrap);
        } else if(ekind == kEventHIObjectDestruct) {
            free(data);
        } else if(ekind == kEventHIObjectPrintDebugInfo) {
            qDebug("%s::%s", wrap->object->metaObject()->className(), wrap->object->objectName().local8Bit());
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
