#include <Carbon.h>

WindowRef foo;

OSStatus
event_processor(EventHandlerCallRef ref, EventRef event, void *data)
{
    printf("selecting.. %d\n", foo);
    SelectWindow(foo);
    return noErr;
}

int
main()
{
    EventTypeSpec e = { kEventClassWindow, kEventWindowBoundsChanged };
    EventHandlerRef ehr;
    EventRef event;
    WindowRef w;

    Rect r;

    SetRect(&r, 20, 20, 100, 100);
    CreateNewWindow(kDocumentWindowClass, kWindowStandardDocumentAttributes|kWindowStandardHandlerAttribute, &r, &w);
    printf("Created %d\n", w);
    foo = w;
    ShowWindow(w);

    InstallEventHandler( GetApplicationEventTarget(), NewEventHandlerUPP(event_processor),
			 GetEventTypeCount(e), &e, NULL, &ehr);
#if 0
    while(1) {
	if(ReceiveNextEvent( 0, 0, kEventDurationForever, TRUE, &event ) != noErr)
	    break;
	SendEventToApplication(event);
	ReleaseEvent(event);
    }
#else
    RunCurrentEventLoop(kEventDurationForever);
#endif
    printf("done..\n");
    return 1;
}
	

