#include <Carbon.h>

Boolean DoEvent(EventRecord *event)
{
    short part;
    Boolean hit;
    Rect tempRect;
    WindowRef whichWindow;

    switch(event->what)
    {
    case mouseDown:
	part = FindWindow(event->where, &whichWindow);
	switch(part)
	{
	case inContent:
	    if(whichWindow != FrontWindow())
		SelectWindow(whichWindow);
	    break;
	case inDrag:
	    GetRegionBounds(GetGrayRgn(), &tempRect);
	    DragWindow(whichWindow, event->where, &tempRect);
	    break;
	case inGoAway:
	    DisposeWindow(whichWindow);
	    ExitToShell();
	    return true;
	case inZoomIn:
	case inZoomOut:
	    hit = TrackBox(whichWindow, event->where, part);
	    if(hit) {
		SetPort(GetWindowPort(whichWindow));
		EraseRect(GetWindowPortBounds(whichWindow, &tempRect));
		ZoomWindow(whichWindow, part, true);
		InvalWindowRect(whichWindow, GetWindowPortBounds(whichWindow, &tempRect));
	    }
	    break;
	}
	break;
    }
    return false;
}

void EventLoop()
{
    Boolean	gotEvent, done=false;
    EventRecord event;
    do {
	gotEvent = WaitNextEvent(everyEvent,&event,32767,nil);
	if (gotEvent)
	    done = DoEvent(&event);
    } while (!done);
    ExitToShell();
}

int
main()
{
    WindowRef window;
    Rect r;
    CGContextRef ctx = NULL;
    CGRect r2;
    
    SetRect(&r, 50, 50, 400, 400);
    CreateNewWindow(kDocumentWindowClass, kWindowStandardDocumentAttributes, &r, &window);
    ShowWindow(window);

    CreateCGContextForPort(GetWindowPort(window), &ctx);
    r2 = CGRectMake(0, 0, 50, 50);
    CGContextClearRect(ctx, r2);
    CGContextFlush(ctx);

    EventLoop(); //let me see..
    return 1;
}
	
