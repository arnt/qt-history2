/****************************************************************************
** $Id$
**
** Implementation of QAccessible class for Mac OS
**
** Created : 001018
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Macintosh may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qaccessible.h"

#if defined(QT_ACCESSIBILITY_SUPPORT)
#include "qt_mac.h"
#include "qptrdict.h"
#include "qapplication.h"

/*****************************************************************************
  External functions
 *****************************************************************************/
QString cfstring2qstring(CFStringRef); //qglobal.cpp

/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/
static EventHandlerRef access_proc_handler = NULL;
static HIObjectClassRef widget_create_class = NULL;
static EventHandlerUPP access_proc_handlerUPP = NULL;
static CFStringRef qt_mac_static_class_str = NULL;
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

static CFStringRef qstring2cfstring(const QString &s) {
    return CFStringCreateWithCharacters(0, (UniChar *)s.unicode(), s.length());
}

static CFStringRef qt_mac_class_str()
{
    if(!qt_mac_static_class_str) 
	qt_mac_static_class_str = qstring2cfstring(QString("com.trolltech.object"));
    return qt_mac_static_class_str;
}
struct QAccessibleObjectWrapper
{
    QGuardedPtr<QObject> object;
    AXUIElementRef access;
};
static QPtrDict<QAccessibleObjectWrapper> *qt_mac_object_map = 0;
enum {
    kEventParamQObject = 'qobj',   /* typeQObject */
    typeQObject = 1  /* QObject *  */
};
static QObject *qt_mac_find_access_object(HIObjectRef objref) 
{
    if(QAccessibleObjectWrapper *wrap = (QAccessibleObjectWrapper*)HIObjectDynamicCast(objref, qt_mac_class_str())) 
	return wrap->object;
    return NULL;
}
static QObject *qt_mac_find_access_object(AXUIElementRef element)
{
    return qt_mac_find_access_object(AXUIElementGetHIObject(element));
}
AXUIElementRef qt_mac_find_uielement(QObject *o)
{
    QAccessibleObjectWrapper *obj_wrap = (qt_mac_object_map ? qt_mac_object_map->find(o) : NULL);
    if(!obj_wrap) {
	if(!widget_create_class) {
	    OSStatus err = HIObjectRegisterSubclass(qt_mac_class_str(), NULL, 
						    0, access_proc_handlerUPP, GetEventTypeCount(events), 
						    events, NULL, &widget_create_class);
	    if(err != noErr) {
		qDebug("That shouldn't happen! %ld", err);
		return 0;
	    }
	}
	EventRef event;
        CreateEvent(NULL, kEventClassHIObject, kEventHIObjectInitialize, GetCurrentEventTime(),
		    kEventAttributeUserEvent, &event);
	SetEventParameter(event, kEventParamQObject, typeQObject, sizeof(o), &o);
	HIObjectRef hiobj;
	if(HIObjectCreate(qt_mac_class_str(), event, &hiobj) == noErr) {
	    HIObjectSetAccessibilityIgnored(hiobj, false);
	    AXUIElementRef ref = AXUIElementCreateWithHIObjectAndIdentifier(hiobj, (UInt64)o);
	    obj_wrap = qt_mac_object_map->find(o);
	    obj_wrap->access = ref;
	}
	ReleaseEvent(event);
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
    if(qt_mac_static_class_str) {
	CFRelease(qt_mac_static_class_str);
	qt_mac_static_class_str = 0;
    }
}

void QAccessible::updateAccessibility(QObject *object, int control, Event reason)
{
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
		if(object) {
		    QAccessibleInterface *iface;
		    if(queryAccessibleInterface(object, &iface) == QS_OK) {
			QPoint p(where.h, where.v);
			if(object->isWidgetType())
			    ((QWidget*)object)->mapFromGlobal(p);
			object = NULL;
			int child = iface->controlAt(p.x(), p.y());
			if(child > 0) {
			    QAccessibleInterface *child_iface;
			    if(iface->queryChild(child, &child_iface) == QS_OK) {
				object = queryAccessibleObject(child_iface);
				child_iface->release();
			    }
			}
			iface->release();
		    }
		} else {
		    object = QApplication::widgetAt(where.h, where.v);
		}
		if(object) {
		    AXUIElementRef element = qt_mac_find_uielement(object);
		    SetEventParameter(event, kEventParamAccessibleChild, typeCFTypeRef, sizeof(element), &element);
		}
	    } else if(!object) { //the below are not mine then..
		OSStatus err = CallNextEventHandler(next_ref, event);
		if(err != noErr)
		    return err;
	    } else if(ekind == kEventAccessibleGetAllAttributeNames) {
		QAccessibleInterface *iface;
		if(queryAccessibleInterface(object, &iface) == QS_OK) {
		    CFMutableArrayRef attrs;
		    GetEventParameter(event, kEventParamAccessibleAttributeNames, typeCFMutableArrayRef, NULL,
				      sizeof(attrs), NULL, &attrs);
		    CFArrayAppendValue(attrs, kAXChildrenAttribute);
		    CFArrayAppendValue(attrs, kAXParentAttribute);
		    CFArrayAppendValue(attrs, kAXPositionAttribute);
		    CFArrayAppendValue(attrs, kAXSizeAttribute);
		    CFArrayAppendValue(attrs, kAXRoleDescriptionAttribute);
		    CFArrayAppendValue(attrs, kAXValueAttribute);
		    CFArrayAppendValue(attrs, kAXHelpAttribute);
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
		if(queryAccessibleInterface(object, &iface) == QS_OK) {
		    CFStringRef str;
		    GetEventParameter(event, kEventParamAccessibleAttributeName, typeCFStringRef, NULL,
				      sizeof(str), NULL, &str);
		    if(CFStringCompare(str, kAXChildrenAttribute, 0) == kCFCompareEqualTo) {
			int children_count = iface->childCount();
			QAccessibleInterface *child_iface;
			AXUIElementRef *children = (AXUIElementRef *)malloc(sizeof(AXUIElementRef) * children_count);
			for(int i = 0; i < children_count; i++) {
			    if(iface->queryChild(i, &child_iface) == QS_OK) {
				QObject *child = queryAccessibleObject(child_iface);
				children[i] = qt_mac_find_uielement(child);
				child_iface->release();
			    }
			}
			CFArrayRef arr = CFArrayCreate(NULL, (const void **)children, children_count, NULL);
			SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFMutableArrayRef, sizeof(arr), &arr);
		    } else if(CFStringCompare(str, kAXParentAttribute, 0) == kCFCompareEqualTo) {
			QAccessibleInterface *parent_iface;
			if(iface->queryParent(&parent_iface) == QS_OK) {
			    QObject *parent = queryAccessibleObject(parent_iface);
			    AXUIElementRef element = qt_mac_find_uielement(parent);
			    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof(element), &element);
			    parent_iface->release();
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
		    } else if(CFStringCompare(str, kAXRoleDescriptionAttribute, 0) == kCFCompareEqualTo) {
			CFStringRef str = qstring2cfstring(iface->text(Description, 0));
			SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFStringRef, sizeof(str), &str);
		    } else if(CFStringCompare(str, kAXValueAttribute, 0) == kCFCompareEqualTo) {
			bool ok;
			int val = iface->text(Value, 0).toInt(&ok);
			if(!ok)
			    val = 0;
			SetEventParameter(event, kEventParamAccessibleAttributeValue, typeInteger, sizeof(val), &val);
		    } else if(CFStringCompare(str, kAXHelpAttribute, 0) == kCFCompareEqualTo) {
			CFStringRef str = qstring2cfstring(iface->text(Help, 0));
			SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFStringRef, sizeof(str), &str);
		    } else if(CFStringCompare(str, kAXRoleAttribute, 0) == kCFCompareEqualTo) {
			CFStringRef role = kAXUnknownRole;
			Role qrole = iface->role(0);
			if(qrole == TitleBar)
			    role = kAXWindowTitleRole;
			else if(qrole == MenuBar)
			    role = kAXMenuBarRole;
			else if(qrole == ScrollBar)
			    role = kAXScrollBarRole;
			else if(qrole == Grip)
			    role = kAXGrowAreaAttribute;
			else if(qrole == Window || qrole == Dialog || qrole == AlertMessage || qrole == ToolTip ||
				qrole == HelpBalloon)
			    role = kAXWindowRole;
			else if(qrole == PopupMenu)
			    role = kAXMenuRole;
			else if(qrole == MenuItem)
			    role = kAXMenuItemRole;
			else if(qrole == Application)
			    role = kAXApplicationRole;
			else if(qrole == Pane || qrole == Grouping)
			    role = kAXGroupRole;
			else if(qrole == Separator)
			    role = kAXSplitterRole;
			else if(qrole == ToolBar)
			    role = kAXToolbarRole;
			else if(qrole == List)
			    role = kAXListRole;
			else if(qrole == StatusBar)
			    role = kAXStaticTextRole;
			else if(qrole == Table)
			    role = kAXTableRole;
			else if(qrole == ColumnHeader || qrole == RowHeader)
			    role = kAXTableHeaderViewRole;
			else if(qrole == Column)
			    role = kAXTableColumnRole;
			else if(qrole == Row)
			    role = kAXTableRowRole;
			else if(qrole == Cell)
			    role = kAXOutlineCellRole;
			else if(qrole == Link)
			    role = kAXTextRole;
			else if(qrole == Outline)
			    role = kAXBoxRole;
			else if(qrole == StaticText)
			    role = kAXStaticTextRole;
			else if(qrole == EditableText)
			    role = kAXTextFieldRole;
			else if(qrole == PushButton)
			    role = kAXPushButtonRole;
			else if(qrole == CheckBox)
			    role = kAXCheckBoxRole;
			else if(qrole == RadioButton)
			    role = kAXRadioButtonRole;
			else if(qrole == ComboBox)
			    role = kAXComboBoxRole;
			else if(qrole == DropLest)
			    role = kAXListRole;
			else if(qrole == ProgressBar)
			    role = kAXProgressIndicatorRole;
			else if(qrole == Slider)
			    role = kAXSliderRole;
			else if(qrole == SpinBox)
			    role = kAXIncrementButtonAttribute;
			else if(qrole == ButtonDropDown)
			    role = kAXPopUpButtonRole;
			else if(qrole == ButtonMenu)
			    role = kAXMenuButtonRole;
			else if(qrole == PageTabList)
			    role = kAXTabGroupRole;
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
			QMemArray<int> sel = iface->selection();
			QAccessibleInterface *child_iface;
			for(uint i = 0; i < sel.count(); i++) {
			    if(iface->queryChild(sel[i], &child_iface) == QS_OK) {
				AXUIElementRef element = qt_mac_find_uielement(queryAccessibleObject(child_iface));
				SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof(element), &element);
				child_iface->release();
			    }
			}
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
			qDebug("Unknown [kEventAccessibleGetNamedAttribute]: %s", cfstring2qstring(str).latin1());
		    }
		    iface->release();
		}
	    } else if(ekind == kEventAccessibleSetNamedAttribute) {
		QAccessibleInterface *iface;
		if(queryAccessibleInterface(object, &iface) == QS_OK) {
		    CFStringRef str;
		    GetEventParameter(event, kEventParamAccessibleAttributeName, typeCFStringRef, NULL,
				      sizeof(str), NULL, &str);
		    if(CFStringCompare(str, kAXRoleDescriptionAttribute, 0) == kCFCompareEqualTo) {
			CFStringRef val;
			if(GetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFStringRef, NULL,
					     sizeof(val), NULL, &val) == noErr)
			    iface->setText(Description, 0, cfstring2qstring(val));
		    } else if(CFStringCompare(str, kAXTextAttribute, 0) == kCFCompareEqualTo) {
			int val;
			if(GetEventParameter(event, kEventParamAccessibleAttributeValue, typeInteger, NULL,
					     sizeof(val), NULL, &val) == noErr)
			    iface->setText(Value, 0, QString::number(val));
		    } else if(CFStringCompare(str, kAXHelpAttribute, 0) == kCFCompareEqualTo) {
			CFStringRef val;
			if(GetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFStringRef, NULL,
					     sizeof(val), NULL, &val) == noErr)
			    iface->setText(Help, 0, cfstring2qstring(val));
		    } else if(CFStringCompare(str, kAXFocusedAttribute, 0) == kCFCompareEqualTo) {
			Boolean val;
			if(GetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean, NULL,
					     sizeof(val), NULL, &val) == noErr) {
			    if(val)
				iface->setFocus(0);
			}
		    } else {
			qDebug("Unknown [kEventAccessibleSetNamedAttribute]: %s", cfstring2qstring(str).latin1());
		    }
		    iface->release();
		}
	    } else if(ekind == kEventAccessibleIsNamedAttributeSettable) {
		QAccessibleInterface *iface;
		if(queryAccessibleInterface(object, &iface) == QS_OK) {
		    CFStringRef str;
		    GetEventParameter(event, kEventParamAccessibleAttributeName, typeCFStringRef, NULL,
				      sizeof(str), NULL, &str);
		    Boolean settable = false;
		    if(CFStringCompare(str, kAXRoleDescriptionAttribute, 0) == kCFCompareEqualTo ||
		       CFStringCompare(str, kAXValueAttribute, 0) == kCFCompareEqualTo ||
		       CFStringCompare(str, kAXHelpAttribute, 0) == kCFCompareEqualTo ||
		       CFStringCompare(str, kAXFocusedAttribute, 0) == kCFCompareEqualTo) 
			settable = true;
		    SetEventParameter(event, kEventParamAccessibleAttributeSettable, typeBoolean, sizeof(settable), &settable);
		    iface->release();
		}
	    } else if(ekind == kEventAccessibleGetAllActionNames) {
	    } else if(ekind == kEventAccessiblePerformNamedAction) {
	    } else if(ekind == kEventAccessibleGetNamedActionDescription) {
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
	    if(!qt_mac_object_map)
		qt_mac_object_map = new QPtrDict<QAccessibleObjectWrapper>;
	    qt_mac_object_map->insert(wrap->object, wrap);
	} else if(ekind == kEventHIObjectDestruct) {
	    free(data);
	} else if(ekind == kEventHIObjectPrintDebugInfo) {
	    qDebug("%s::%s", wrap->object->className(), wrap->object->name());
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
