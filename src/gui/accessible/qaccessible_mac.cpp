/****************************************************************************
**
** Implementation of QAccessible class for Mac OS
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
#include "qt_mac.h"
#include "qhash.h"
#include "qpointer.h"
#include "qapplication.h"

/*****************************************************************************
  External functions
 *****************************************************************************/
CFStringRef qstring2cfstring(const QString &s); //qglobal.cpp
QString cfstring2qstring(CFStringRef); //qglobal.cpp

/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/
static EventHandlerRef access_proc_handler = NULL;
static HIObjectClassRef accessibility_class = NULL;
static EventHandlerUPP access_proc_handlerUPP = NULL;
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
static QHash<QObject *, QAccessibleObjectWrapper *> qt_mac_object_map;
enum {
    kEventParamQObject = 'qobj',   /* typeQObject */
    typeQObject = 1  /* QObject *  */
};
static QObject *qt_mac_find_access_object(HIObjectRef objref) 
{
    if(QAccessibleObjectWrapper *wrap = (QAccessibleObjectWrapper*)HIObjectDynamicCast(objref, kObjectQtAccessibility))
	return wrap->object;
    return NULL;
}
static QObject *qt_mac_find_access_object(AXUIElementRef element)
{
    return qt_mac_find_access_object(AXUIElementGetHIObject(element));
}
AXUIElementRef qt_mac_find_uielement(QObject *o)
{
    qt_mac_object_map.ensure_constructed();
    QAccessibleObjectWrapper *obj_wrap = qt_mac_object_map.value(o);
    if(!obj_wrap) {
	if(!accessibility_class) {
	    OSStatus err = HIObjectRegisterSubclass(kObjectQtAccessibility, NULL, 
						    0, access_proc_handlerUPP, GetEventTypeCount(events), 
						    events, NULL, &accessibility_class);
	    if(err != noErr) 
		return 0;
	}
	HIObjectRef hiobj;
	EventRef init_event;
        CreateEvent(NULL, kEventClassHIObject, kEventHIObjectInitialize, GetCurrentEventTime(),
		    kEventAttributeUserEvent, &init_event);
	SetEventParameter(init_event, kEventParamQObject, typeQObject, sizeof(o), &o);
	if(HIObjectCreate(kObjectQtAccessibility, init_event, &hiobj) == noErr) {
	    HIObjectSetAccessibilityIgnored(hiobj, false);
	    AXUIElementRef ref = AXUIElementCreateWithHIObjectAndIdentifier(hiobj, (UInt32)o);
	    obj_wrap = qt_mac_object_map.value(o);
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
			    GetEventTypeCount(events), events, NULL, &access_proc_handler);
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
	access_proc_handler = NULL;
    }
    if(access_proc_handlerUPP) {
	DisposeEventHandlerUPP(access_proc_handlerUPP);
	access_proc_handlerUPP = NULL;
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
	//need to look up the proper object.. FIXME
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
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::ScrollBar, kAXScrollBarRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::Grip, kAXGrowAreaRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::Window, kAXWindowRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::Dialog, kAXWindowRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::AlertMessage, kAXWindowRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::ToolTip, kAXWindowRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::HelpBalloon, kAXWindowRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::PopupMenu, kAXMenuRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::MenuItem, kAXMenuItemRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::Application, kAXApplicationRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::Pane, kAXGroupRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::Grouping, kAXGroupRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::Separator, kAXSplitterRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::ToolBar, kAXToolbarRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::PageTabList, kAXTabGroupRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::ButtonMenu, kAXMenuButtonRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::ButtonDropDown, kAXPopUpButtonRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::SpinBox, kAXIncrementorRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::Slider, kAXSliderRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::ProgressBar, kAXProgressIndicatorRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::ComboBox, kAXComboBoxRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::RadioButton, kAXRadioButtonRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::CheckBox, kAXCheckBoxRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::StaticText, kAXStaticTextRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::Table, kAXTableRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::StatusBar, kAXStaticTextRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
#if QT_MACOSX_VERSION >= 0x1030
    { { QAccessible::Column, kAXColumnRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::ColumnHeader, kAXColumnRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::Row, kAXRowRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::RowHeader, kAXRowRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::Cell, kAXUnknownRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::PushButton, kAXButtonRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::EditableText, kAXTextFieldRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::Link, kAXTextFieldRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
#else
    { { QAccessible::TitleBar, kAXWindowTitleRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::ColumnHeader, kAXTableHeaderViewRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::RowHeader, kAXTableHeaderViewRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::Column, kAXTableColumnRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::Row, kAXTableRowRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::Cell, kAXOutlineCellRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::EditableText, kAXTextRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::Link, kAXTextRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
    { { QAccessible::PushButton, kAXPushButtonRole, false },
      { QAccessible::Value, kAXValueAttribute, true },
      { QAccessible::Description, kAXRoleDescriptionAttribute, false },
      { QAccessible::Help, kAXHelpAttribute, false },
      { -1, NULL, false }
    },
#endif
    { { -1, NULL, false } }
};

QMAC_PASCAL OSStatus
QAccessible::globalEventProcessor(EventHandlerCallRef next_ref, EventRef event, void *data)
{
    bool handled_event = TRUE;
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    switch(eclass) {
    case kEventClassAccessibility: {
	AXUIElementRef element;
	GetEventParameter(event, kEventParamAccessibleObject, typeCFTypeRef, NULL, sizeof(element), NULL, &element);
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
		GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, NULL,
				  sizeof(where), NULL, &where);
		QObject *child_object = NULL;
		if(object) {
		    QAccessibleInterface *iface;
		    if(QAccessible::queryAccessibleInterface(object, &iface)) {
			int child = iface->childAt(where.h, where.v);
			if(child > 0) {
			    QAccessibleInterface *child_iface;
			    if(iface->navigate(Child, child, &child_iface) != -1) {
				if(child_iface) {
				    child_object = child_iface->object();
				    child_iface->release();
				}
			    }
			}
			iface->release();
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
		QAccessibleInterface *iface;
		if(QAccessible::queryAccessibleInterface(object, &iface)) {
		    CFMutableArrayRef attrs;
		    GetEventParameter(event, kEventParamAccessibleAttributeNames, typeCFMutableArrayRef, NULL,
				      sizeof(attrs), NULL, &attrs);
		    for(int r = 0; text_bindings[r][0].qt != -1; r++) {
			if(iface->role(0) == (QAccessible::Role)text_bindings[r][0].qt) {
			    for(int a = 1; text_bindings[r][a].qt != -1; a++)
				CFArrayAppendValue(attrs, text_bindings[r][a].mac);
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
		QAccessibleInterface *iface;
		if(QAccessible::queryAccessibleInterface(object, &iface)) {
		    CFStringRef str;
		    GetEventParameter(event, kEventParamAccessibleAttributeName, typeCFStringRef, NULL,
				      sizeof(str), NULL, &str);
		    if(CFStringCompare(str, kAXChildrenAttribute, 0) == kCFCompareEqualTo) {
			int children_count = iface->childCount();
			QAccessibleInterface *child_iface;
			AXUIElementRef *children = (AXUIElementRef *)malloc(sizeof(AXUIElementRef) * children_count);
			for(int i = 0; i < children_count; i++) {
			    if(iface->navigate(Child, i, &child_iface) != -1) {
				if(child_iface) {
				    QObject *child = child_iface->object();
				    children[i] = qt_mac_find_uielement(child);
				    child_iface->release();
				}
			    }
			}
			CFArrayRef arr = CFArrayCreate(NULL, (const void **)children, children_count, NULL);
			SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFMutableArrayRef, sizeof(arr), &arr);
		    } else if(CFStringCompare(str, kAXParentAttribute, 0) == kCFCompareEqualTo) {
			QAccessibleInterface *parent_iface;
			if(iface->navigate(Ancestor, 1, &parent_iface) != -1) {
			    if(parent_iface) {
				QObject *parent = parent_iface->object();
				AXUIElementRef element = qt_mac_find_uielement(parent);
				SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof(element), &element);
				parent_iface->release();
			    }
			}
		    } else if(CFStringCompare(str, kAXPositionAttribute, 0) == kCFCompareEqualTo) {
			QPoint qpoint(iface->rect(0).topLeft());
			if(object->isWidgetType()) {
			    QWidget *widget = (QWidget*)object;
			    if(!widget->isTopLevel())
				qpoint = widget->mapTo(widget->topLevelWidget(), qpoint);
			}
			Point point;
			point.h = qpoint.x();
			point.v = qpoint.y();
			SetEventParameter(event, kEventParamAccessibleAttributeValue, typeQDPoint, sizeof(point), &point);
		    } else if(CFStringCompare(str, kAXSizeAttribute, 0) == kCFCompareEqualTo) {
			QSize sz(iface->rect(0).size());
			Point size;
			size.h = sz.width();
			size.v = sz.height();
			SetEventParameter(event, kEventParamAccessibleAttributeValue, typeQDPoint, sizeof(size), &size);
		    } else if(CFStringCompare(str, kAXRoleAttribute, 0) == kCFCompareEqualTo) {
			CFStringRef role = kAXUnknownRole;
			for(int r = 0; text_bindings[r][0].qt != -1; r++) {
			    if(iface->role(0) == (QAccessible::Role)text_bindings[r][0].qt) {
				role = text_bindings[r][0].mac;
				break;
			    }
			}
			SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFStringRef, sizeof(role), &role);
		    } else if(CFStringCompare(str, kAXEnabledAttribute, 0) == kCFCompareEqualTo) {
			Boolean val = !((iface->state(0) & Unavailable)) ? true : false;
			SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean, sizeof(val), &val);
		    } else if(CFStringCompare(str, kAXExpandedAttribute, 0) == kCFCompareEqualTo) {
			Boolean val = (iface->state(0) & Expanded) ? true : false;
			SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean, sizeof(val), &val);
		    } else if(CFStringCompare(str, kAXSelectedAttribute, 0) == kCFCompareEqualTo) {
			Boolean val = (iface->state(0) & Selection) ? true : false;
			SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean, sizeof(val), &val);
		    } else if(CFStringCompare(str, kAXFocusedAttribute, 0) == kCFCompareEqualTo) {
			Boolean val = (iface->state(0) & Focus) ? true : false;
			SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean, sizeof(val), &val);
		    } else if(CFStringCompare(str, kAXSelectedChildrenAttribute, 0) == kCFCompareEqualTo) {
			const int cc = iface->childCount();
			QVector<int> sel(cc);
			int selIndex = 0;
			for (int i = 1; i <= cc; ++i) {
			    QAccessibleInterface *child = 0;
			    int i2 = iface->navigate(Child, i, &child);
			    bool isSelected = false;
			    if (child) {
				isSelected = child->state(0) & Selected;
				child->release();
				child = 0;
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
				    child_iface->release();
				}
			    }
			}
			CFArrayRef cfList = CFArrayCreate(NULL, (const void **)arr, sel.count(), NULL);
			SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof(cfList), &cfList);
		    } else if(CFStringCompare(str, kAXMainAttribute, 0) == kCFCompareEqualTo) {
			Boolean val = (object->isWidgetType() && qApp->mainWidget() == object) ? true : false;
			SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean, sizeof(val), &val);
		    } else if(CFStringCompare(str, kAXCloseButtonAttribute, 0) == kCFCompareEqualTo) {
			if(object->isWidgetType()) {
			    Boolean val = true; //FIXME
			    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean, sizeof(val), &val);
			}
		    } else if(CFStringCompare(str, kAXZoomButtonAttribute, 0) == kCFCompareEqualTo) {
			if(object->isWidgetType()) {
			    QWidget *widget = (QWidget*)object;
			    Boolean val = (widget->testWFlags(Qt::WStyle_Maximize)) ? true : false;
			    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean, sizeof(val), &val);
			}
		    } else if(CFStringCompare(str, kAXMinimizeButtonAttribute, 0) == kCFCompareEqualTo) {
			if(object->isWidgetType()) {
			    QWidget *widget = (QWidget*)object;
			    Boolean val = (widget->testWFlags(Qt::WStyle_Minimize)) ? true : false;
			    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean, sizeof(val), &val);
			}
		    } else if(CFStringCompare(str, kAXToolbarButtonAttribute, 0) == kCFCompareEqualTo) {
			if(object->isWidgetType()) {
			    QWidget *widget = (QWidget*)object;
			    Boolean val = (widget->inherits("QMainWindow")) ? true : false;
			    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean, sizeof(val), &val);
			}
		    } else if(CFStringCompare(str, kAXGrowAreaAttribute, 0) == kCFCompareEqualTo) {
			if(object->isWidgetType()) {
			    Boolean val = true; //FIXME
			    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean, sizeof(val), &val);
			}
		    } else if(CFStringCompare(str, kAXMinimizedAttribute, 0) == kCFCompareEqualTo) {
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
				    if(CFStringCompare(str, text_bindings[r][a].mac, 0) == kCFCompareEqualTo) {
					CFStringRef str = qstring2cfstring(iface->text((QAccessible::Text)text_bindings[r][a].qt, 0));
					SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFStringRef, 
							  sizeof(str), &str);
					found = true;
					break;
				    }
				}
				break;
			    }
			}
			if(!found)
			    qDebug("Unknown [kEventAccessibleGetNamedAttribute]: %s", cfstring2qstring(str).latin1());
		    }
		    iface->release();
		}
	    } else if(ekind == kEventAccessibleSetNamedAttribute) {
		QAccessibleInterface *iface;
		if(QAccessible::queryAccessibleInterface(object, &iface)) {
		    CFStringRef str;
		    GetEventParameter(event, kEventParamAccessibleAttributeName, typeCFStringRef, NULL,
				      sizeof(str), NULL, &str);
#if 0
		    if(CFStringCompare(str, kAXFocusedAttribute, 0) == kCFCompareEqualTo) {
			Boolean val;
			if(GetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean, NULL,
					     sizeof(val), NULL, &val) == noErr) {
			    if(val)
				iface->doAction(0, SetFocus);
			} 
		    } else 
#endif
			
		    {
			bool found = false;
			for(int r = 0; text_bindings[r][0].qt != -1; r++) {
			    if(iface->role(0) == (QAccessible::Role)text_bindings[r][0].qt) {
				for(int a = 1; text_bindings[r][a].qt != -1; a++) {
				    if(CFStringCompare(str, text_bindings[r][a].mac, 0) == kCFCompareEqualTo) {
					CFStringRef val;
					if(GetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFStringRef, NULL,
							     sizeof(val), NULL, &val) == noErr)
					    iface->setText((QAccessible::Text)text_bindings[r][a].qt, 0, cfstring2qstring(val));
					found = true;
					break;
				    }
				}
				break;
			    }
			}
			if(!found)
			    qDebug("Unknown [kEventAccessibleSetNamedAttribute]: %s", cfstring2qstring(str).latin1());
		    }
		    iface->release();
		}
	    } else if(ekind == kEventAccessibleIsNamedAttributeSettable) {
		QAccessibleInterface *iface;
		if(QAccessible::queryAccessibleInterface(object, &iface)) {
		    CFStringRef str;
		    GetEventParameter(event, kEventParamAccessibleAttributeName, typeCFStringRef, NULL,
				      sizeof(str), NULL, &str);
		    Boolean settable = false;
		    if(CFStringCompare(str, kAXFocusedAttribute, 0) == kCFCompareEqualTo) {
			settable = true;
		    } else {
		    }
		    SetEventParameter(event, kEventParamAccessibleAttributeSettable, typeBoolean, sizeof(settable), &settable);
		    iface->release();
		}
	    } else if(ekind == kEventAccessibleGetAllActionNames) {
		QAccessibleInterface *iface;
		if(QAccessible::queryAccessibleInterface(object, &iface)) {
		    const int actCount = iface->numActions(0);
		    CFStringRef *arr = (CFStringRef *)malloc(sizeof(AXUIElementRef) * actCount);
		    for(int i = 0; i < actCount; i++) {
			QString actName = iface->actionText(i, Name, 0);
			arr[i] = CFStringCreateWithCharacters(NULL, (UniChar *)actName.unicode(), actName.length());
		    }
		    CFArrayRef cfList = CFArrayCreate(NULL, (const void **)arr, actCount, NULL);
		    SetEventParameter(event, kEventParamAccessibleActionNames, typeCFTypeRef, sizeof(cfList), &cfList);
		    iface->release();
		}
	    } else if(ekind == kEventAccessiblePerformNamedAction) {
		QAccessibleInterface *iface;
		if(QAccessible::queryAccessibleInterface(object, &iface)) {
		    CFStringRef act;
		    GetEventParameter(event, kEventParamAccessibleActionName, typeCFStringRef, NULL,
				      sizeof(act), NULL, &act);
#if 0
		    if(CFStringCompare(act, kAXPressAction, 0) == kCFCompareEqualTo) {
			iface->doAction(0, Press);
		    } else if(CFStringCompare(act, kAXIncrementAction, 0) == kCFCompareEqualTo) {
			iface->doAction(0, Increase);
		    } else if(CFStringCompare(act, kAXDecrementAction, 0) == kCFCompareEqualTo) {
			iface->doAction(0, Decrease);
		    } else if(CFStringCompare(act, kAXConfirmAction, 0) == kCFCompareEqualTo) {
			iface->doAction(0, Accept);
		    } else if(CFStringCompare(act, kAXPickAction, 0) == kCFCompareEqualTo) {
			iface->doAction(0, Select);
		    } else if(CFStringCompare(act, kAXCancelAction, 0) == kCFCompareEqualTo) {
			iface->doAction(0, Cancel);
		    } else 
#endif
		    {
			bool found_act = FALSE;
			const QString qAct = cfstring2qstring(act);
			const int actCount = iface->numActions(0);
			for(int i = 0; i < actCount; i++) {
			    if(iface->actionText(i, Name, 0) == qAct) {
				iface->doAction(0, i);
				found_act = TRUE;
				break;
			    }
			}
			if(!found_act)
			    qWarning("Unknown [kEventAccessiblePerformNamedAction]: %s", qAct.latin1());
		    }
		    iface->release();			
		}
	    } else if(ekind == kEventAccessibleGetNamedActionDescription) {
		QAccessibleInterface *iface;
		if(QAccessible::queryAccessibleInterface(object, &iface)) {
		    CFStringRef act;
		    GetEventParameter(event, kEventParamAccessibleActionName, typeCFStringRef, NULL,
				      sizeof(act), NULL, &act);
		    QString actDesc;
#if 0
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
		    } else 
#endif
		    {
			bool found_act = FALSE;
			const QString qAct = cfstring2qstring(act);
			const int actCount = iface->numActions(0);
			for(int i = 0; i < actCount; i++) {
			    if(iface->actionText(i, Name, 0) == qAct) {
				actDesc = iface->actionText(i, Description, 0);
				found_act = TRUE;
				break;
			    }
			}
			if(!found_act)
			    qWarning("Unknown [kEventAccessibleGetNamedActionDescription]: %s", qAct.latin1());
		    }
		    if(!actDesc.isNull()) {
			CFStringRef cfActDesc = qstring2cfstring(actDesc);
			SetEventParameter(event, kEventParamAccessibleActionDescription, typeCFStringRef, 
					  sizeof(cfActDesc), &cfActDesc);
		    }
		    iface->release();			
		}
	    } else {
		handled_event = FALSE;
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
	    GetEventParameter(event, kEventParamQObject, typeQObject, NULL, sizeof(qobj), NULL, &qobj);
	    QAccessibleObjectWrapper *wrap = (QAccessibleObjectWrapper*)data;
	    wrap->object = qobj;
	    qt_mac_object_map.ensure_constructed();
	    qt_mac_object_map.insert(wrap->object, wrap);
	} else if(ekind == kEventHIObjectDestruct) {
	    free(data);
	} else if(ekind == kEventHIObjectPrintDebugInfo) {
	    qDebug("%s::%s", wrap->object->className(), wrap->object->objectName());
	} else {
	    handled_event = FALSE;
	}
	break; }
    default:
	handled_event = FALSE;
	break;
    }
    if(!handled_event) //let the event go through
	return eventNotHandledErr;
    return noErr; //we eat the event
}

#endif
