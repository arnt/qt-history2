#include <Carbon.h>
#include <stdio.h>

OSStatus
event_processor(EventHandlerCallRef a, EventRef b, void *c)
{
    static int i=0;
    char foo[255];
    int x;
    MenuRef mp;
    ClearMenuBar();
    InvalMenuBar();

    for(x = 0; x < 5; x++) {
	if(CreateNewMenu(0, 0, &mp) != noErr) {
	    fprintf(stderr, "Woop, this shouldn't happen..");
	} else {
	    sprintf(foo+1, "Hello%d", i++);
	    foo[0] = strlen(foo+1);
	    SetMenuTitle(mp, foo);
	    InsertMenu(mp, 0);
	}
    }
    return noErr;
}

int
main()
{
    EventTypeSpec e = { kEventClassMouse, kEventMouseDown };
    EventHandlerRef ehr;
    EventRef event;
    WindowRef w;

    Rect r;

    SetRect(&r, 20, 20, 100, 100);
    CreateNewWindow(kDocumentWindowClass, kWindowStandardDocumentAttributes|kWindowStandardHandlerAttribute,
		    &r, &w);
    ShowWindow(w);

    InstallEventHandler( GetApplicationEventTarget(), NewEventHandlerUPP(event_processor),
			 GetEventTypeCount(e), &e, NULL, &ehr);
    while(1) {
	if(ReceiveNextEvent( 0, 0, kEventDurationForever, TRUE, &event ) != noErr)
	    break;
	SendEventToWindow(event, w);
	ReleaseEvent(event);
    }
    return 1;
}
	

