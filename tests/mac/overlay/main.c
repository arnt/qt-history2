#include <Carbon.h>

#define WIDTH 1000
#define HEIGHT 1000

static void qt_activate_timers(EventLoopTimerRef blah, void *data) {
    static RGBColor f;
    static Rect rect;
    static int first = 1;

    SetPortWindowPort((WindowRef)data);
    
    if(!first) {
	EraseRect(&rect);
    } else {
	PenMode(patXor);
    }

    f.red = (rand() % 255) * 256;
    f.green = (rand() % 255) * 256;
    f.blue = (rand() % 255) * 256;
    RGBForeColor( &f );
    
    first = 0;
    SetRect( &rect, rand() % WIDTH, rand() % HEIGHT,
	     rand() % WIDTH, rand() % HEIGHT);
    FrameRect(&rect);
}

int
main()
{
    WindowRef w;
    Rect r;
    EventLoopTimerRef timer;
    const EventTimerInterval mint = 1.0;

    SetRect(&r, 50, 50, 1000, 1000);
    CreateNewWindow(kOverlayWindowClass, kWindowNoAttributes, &r, &w);
    ShowWindow(w);

    if(InstallEventLoopTimer(GetMainEventLoop(), mint, mint, NewEventLoopTimerUPP(qt_activate_timers),
			     w, &timer) ) {
	return 0;
    }
    RunCurrentEventLoop(kEventDurationForever);
    return 1;
}
	

