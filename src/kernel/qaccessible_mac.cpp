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
#include "qapplication.h"

/*****************************************************************************
  External functions
 *****************************************************************************/
QString cfstring2qstring(CFStringRef); //qglobal.cpp

/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/
static EventHandlerRef access_proc_handler = NULL;
static EventHandlerUPP access_proc_handlerUPP = NULL;
static EventHandlerUPP access_create_widget_proc_handlerUPP = NULL;
static CFStringRef qt_mac_static_class_str = NULL;
static EventTypeSpec events[] = {
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

static CFStringRef qt_mac_class_str()
{
    if(!qt_mac_static_class_str) {
	QString qc("com.trolltech.widget");
	qt_mac_static_class_str = CFStringCreateWithCharacters(0, (UniChar *)qc.unicode(), qc.length());	    
    }
    return qt_mac_static_class_str;
}
struct QAccessibleWidgetWrapper
{
    QWidget *widget;
};
enum {
    kEventParamQWidget = 'qwid',   /* typeQWidget */
    typeQWidget = 1  /* QWidget *  */
};
QMAC_PASCAL OSStatus
qt_mac_access_create_widget(EventHandlerCallRef next_ref, EventRef event, void *data)
{
    QAccessibleWidgetWrapper *wrap = (QAccessibleWidgetWrapper*)data;

    bool handled_event = TRUE;
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    switch(eclass) {
    case kEventClassHIObject: {
	if(ekind == kEventHIObjectConstruct) {
	    QAccessibleWidgetWrapper *w = (QAccessibleWidgetWrapper*)malloc(sizeof(QAccessibleWidgetWrapper));
	    memset(w, '\0', sizeof(QAccessibleWidgetWrapper));
	    SetEventParameter(event, kEventParamHIObjectInstance, typeVoidPtr, sizeof(w), &w);
	} else if(ekind == kEventHIObjectInitialize) {
	    OSStatus err = CallNextEventHandler(next_ref, event);
	    if(err != noErr)
		return err;
	    GetEventParameter(event, kEventParamQWidget, typeQWidget, NULL,
			      sizeof(wrap->widget), NULL, &wrap->widget);
	} else if(ekind == kEventHIObjectDestruct) {
	    free(wrap);
	} else if(ekind == kEventHIObjectPrintDebugInfo) {
	    qDebug("%s::%s", wrap->widget->className(), wrap->widget->name());
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
static QWidget *qt_mac_find_access_widget(HIObjectRef objref) 
{
    if(QAccessibleWidgetWrapper *wrap = (QAccessibleWidgetWrapper*)HIObjectDynamicCast(objref, qt_mac_class_str()))
	return wrap->widget;
    return NULL;
}
static QWidget *qt_mac_find_access_widget(AXUIElementRef element)
{
    return qt_mac_find_access_widget(AXUIElementGetHIObject(element));
}
AXUIElementRef qt_mac_find_uielement(QWidget *w)
{
    QWExtra *extra = w->extraData();
    if(!extra->access) {
	if(!access_create_widget_proc_handlerUPP) {
	    access_create_widget_proc_handlerUPP = NewEventHandlerUPP(qt_mac_access_create_widget);
	    static EventTypeSpec create_widget_events[] = {
		{ kEventClassHIObject,  kEventHIObjectConstruct },
		{ kEventClassHIObject,  kEventHIObjectInitialize },
		{ kEventClassHIObject,  kEventHIObjectDestruct },
		{ kEventClassHIObject,  kEventHIObjectPrintDebugInfo }
	    };
	    HIObjectRegisterSubclass(qt_mac_class_str(), NULL, 0, access_create_widget_proc_handlerUPP, 
				     GetEventTypeCount(create_widget_events), create_widget_events, NULL, NULL);
	}
	EventRef event;
        CreateEvent(NULL, kEventClassHIObject, kEventHIObjectInitialize, GetCurrentEventTime(),
		    kEventAttributeUserEvent, &event);
	SetEventParameter(event, kEventParamQWidget, typeQWidget, sizeof(w), &w);
	HIObjectRef obj;
	if(HIObjectCreate(qt_mac_class_str(), event, &obj) == noErr) 
	    extra->access = AXUIElementCreateWithHIObjectAndIdentifier(obj, w->winId());
	ReleaseEvent(event);
    }
    return extra->access;
}
static HIObjectRef qt_mac_find_hiobject(QWidget *w)
{
    return AXUIElementGetHIObject(qt_mac_find_uielement(w));
}


/*****************************************************************************
  Platform specific QAccessible members
 *****************************************************************************/

void QAccessible::initialize()
{
    if(!AXAPIEnabled()) {
	qDebug("bugger..");
	return;
    }
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
    if(access_create_widget_proc_handlerUPP) {
	DisposeEventHandlerUPP(access_create_widget_proc_handlerUPP);
	access_create_widget_proc_handlerUPP = NULL;
    }
    if(qt_mac_static_class_str) {
	CFRelease(qt_mac_static_class_str);
	qt_mac_static_class_str = 0;
    }
}

void QAccessible::updateAccessibility(QObject *, int, Event)
{
}

QMAC_PASCAL OSStatus
QAccessible::globalEventProcessor(EventHandlerCallRef, EventRef event, void *)
{
    bool handled_event = TRUE;
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    switch(eclass) {
    case kEventClassAccessibility: {
	if(ekind == kEventAccessibleGetFocusedChild) {
	    if(QWidget *widget = qApp->focusWidget()) {
		AXUIElementRef element = qt_mac_find_uielement(widget);
		SetEventParameter(event, kEventParamAccessibleChild, typeCFTypeRef, sizeof(element), &element);
	    }
	} else {
	    AXUIElementRef element;
	    GetEventParameter(event, kEventParamAccessibleObject, typeCFTypeRef, NULL, sizeof(element), NULL, &element);
	    QWidget *widget = qt_mac_find_access_widget(element);
	    if(ekind == kEventAccessibleGetChildAtPoint) {
		Point where;
		GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, NULL,
				  sizeof(where), NULL, &where);
		if(widget) 
		    widget = widget->childAt(where.h, where.v); //should be doings this through the interface..
		else 
		    widget = QApplication::widgetAt(where.h, where.v);
		AXUIElementRef element = qt_mac_find_uielement(widget);
		SetEventParameter(event, kEventParamAccessibleChild, typeCFTypeRef, sizeof(element), &element);
	    } else if(ekind == kEventAccessibleGetAllAttributeNames) {
	    } else if(ekind == kEventAccessibleGetNamedAttribute) {
	    } else if(ekind == kEventAccessibleSetNamedAttribute) {
	    } else if(ekind == kEventAccessibleIsNamedAttributeSettable) {
	    } else if(ekind == kEventAccessibleGetAllActionNames) {
	    } else if(ekind == kEventAccessiblePerformNamedAction) {
	    } else if(ekind == kEventAccessibleGetNamedActionDescription) {
	    } else {
		handled_event = FALSE;
	    }
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
