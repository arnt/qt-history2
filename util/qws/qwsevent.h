// Copyright 1999 Troll Tech AS

#ifndef QWSEVENT_H
#define QWSEVENT_H

struct QWSHeader {
    int width;
    int height;
    int depth;
    int shmid;
};

struct QWSAnyEvent {
    int type;
    int window;
};

struct QWSMouseEvent {
    int type;
    int zero_window;
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

union QWSEvent {
    enum Type {
	NoEvent,
	Mouse, Focus, Key,
	RegionAdd, RegionRemove,
	Creation,
	NEvent
    };

    int type;
    QWSAnyEvent any;
    QWSMouseEvent mouse;
    QWSFocusEvent focus;
    QWSKeyEvent key;
    QWSRegionAddEvent region_add;
    QWSRegionRemoveEvent region_remove;
};

struct EventRec {
    char* name; // for debugging
    int size;
};

extern EventRec eventrec[];

#endif
