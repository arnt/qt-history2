#include <Carbon.h>
#include <stdlib.h>				// defines rand() function
#define ITERATIONS 10000
#define RECT_WIDTH 20 //(rand() % (r.right - r.left))
#define RECT_HEIGHT 20 //(rand() % (r.bottom - r.top))

//#define DO_CLIP
#define DO_LINES_TEST
//#define DO_RECT_TEST


int
main(int argc, char **argv)
{
    CGContextRef ctx = NULL;
    int x, y, i, t, w, h;
    RGBColor c = { 0, 0, 0 };
    WindowRef window;
    Rect r, r3;
    CGRect r2;
#ifdef DO_CLIP    
    RgnHandle rgn = NewRgn();

    OpenRgn();
    SetRect(&r, 20, 20, 300, 300);
    FrameOval(&r);
    CloseRgn(rgn);
#endif    

    SetRect(&r, 50, 50, 900, 900);
    CreateNewWindow(kDocumentWindowClass, kWindowStandardDocumentAttributes, &r, &window);
    ShowWindow(window);

    if(argc == 2) {     //slower?
	SetPortWindowPort(window);
#ifdef DO_CLIP	
	SetClip( rgn );
#endif	
	RGBForeColor( &c );

	t = time(NULL);
	for(i = 0; i < ITERATIONS; i++) {
	    x = rand() % (r.right - r.left);
	    y = rand() % (r.bottom - r.top);
#ifdef DO_RECT_TEST	    
	    SetRect(&r3, x, y, x+(RECT_WIDTH), y+RECT_HEIGHT);
	    PaintRect(&r3);
#elif defined(DO_LINES_TEST)
	    w = rand() % (r.right - r.left);
	    h = rand() % (r.bottom - r.top);
	    MoveTo(x, y);
	    LineTo(w, h);
#endif	    
	}
	printf("Took %d\n", time(NULL) - t);

    } else {  //faster?!
	CreateCGContextForPort(GetWindowPort(window), &ctx);
//	CGContextSetShouldAntialias(ctx, 0 );
#ifdef DO_CLIP		
	ClipCGContextToRegion( ctx, &r, rgn );
#endif	
	
	t = time(NULL);
	for(i = 0; i < ITERATIONS; i++) {
	    x = rand() % (r.right - r.left);
	    y = rand() % (r.bottom - r.top);
#ifdef DO_RECT_TEST	    
	    r2 = CGRectMake(x, y, (RECT_WIDTH), (RECT_HEIGHT));
	    CGContextBeginPath(ctx);
	    CGContextAddRect( ctx, r2 );
	    CGContextFillPath( ctx );
#elif defined(DO_LINES_TEST)
	    w = rand() % (r.right - r.left);
	    h = rand() % (r.bottom - r.top);
	    CGContextBeginPath(ctx);
	    CGContextMoveToPoint(ctx, x, y);
	    CGContextAddLineToPoint(ctx, w, h);
	    CGContextStrokePath(ctx);
#endif	    
	}
	printf("Took %d\n", time(NULL) - t);
	CGContextFlush(ctx);
	CGContextRelease(ctx);
    }

    QDFlushPortBuffer(GetWindowPort(window), NULL);
    while(1); //let me see..
    return 1;
}
	

