// Copyright 1999 Troll Tech AS

#ifndef QWSEVENT_H
#define QWSEVENT_H

struct QWSHeader {
    int width;
    int height;
    int depth;
    int shmid;
    int fbid;
};

struct QWSAnyEvent {
    int type;
    int window;
};

struct QWSMouseEvent {
    int type;
    int window;
    int x_root, y_root, state;
    int time; // milliseconds
};

struct QWSFocusEvent {
    int type;
    int window;
    uint get_focus:1;
};

struct QWSKeyEvent {
    int type;
    int window;
    int unicode;
    int modifiers;
    uint is_press:1;
    uint is_auto_repeat:1;
};

struct QWSRegionAddEvent {
    int type;
    int window;
    int nrectangles;
    struct {
	int x, y, width, height;
    } rectangles[1];
};

struct QWSRegionRemoveEvent {
    int type;
    int window;
    int eventid;
    int nrectangles;
    struct {
	int x, y, width, height;
    } rectangles[1];
};

struct QWSCreationEvent {
    int type;
    int objectid;
};

struct QWSPropertyNotifyEvent {
    int type;
    int window;
    int property;
    int state;
};

struct QWSPropertyReplyEvent {
    int type;
    int window;
    int property;
    int len;
    char *data;
};

struct QWSSelectionClearEvent {
    int type;
    int window;
};

struct QWSSelectionRequestEvent {
    int type;
    int window;
    int requestor; // window which wants the selection
    int property; // property on requestor into which the selection should be stored, normally QWSProperty::PropSelection
    int mimeTypes; // Value is stored in the property mimeType on the requestor window. This value may contain
    // multiple mimeTypes seperated by ;; where the order reflects the priority
};

struct QWSSelectionNotifyEvent {
    int type;
    int window;
    int requestor; // the window which wanted the selection and to which this event is sent
    int property; // property of requestor in which the data of the selection is stored
    int mimeType; // a property on the requestor in which the mime type in which the selection is, is stored
};

union QWSEvent {
    enum Type {
	NoEvent,
	Mouse, Focus, Key,
	RegionAdd, RegionRemove,
	Creation,
	PropertyNotify,
	PropertyReply,
	SelectionClear,
	SelectionRequest,
	SelectionNotify,
	NEvent
    };

    enum State {
	PropertyNewValue,
	PropertyDeleted
    };

    int type;
    QWSAnyEvent any;
    QWSMouseEvent mouse;
    QWSFocusEvent focus;
    QWSKeyEvent key;
    QWSRegionAddEvent region_add;
    QWSRegionRemoveEvent region_remove;
    QWSPropertyNotifyEvent property_notify;
    QWSPropertyReplyEvent property_reply;
    QWSSelectionClearEvent selection_clear;
    QWSSelectionRequestEvent selection_request;
    QWSSelectionNotifyEvent selection_notify;
};

struct EventRec {
    char* name; // for debugging
    int size;
};

extern EventRec eventrec[];

#endif
