#include <Carbon.h>
#include <qdatetime.h>

#define blah 1000

int
main()
{
    QTime t;
    t.start();

    RgnHandle foos[blah];
    for(int i = 0; i < blah; i++)
	foos[i] = NewRgn();
    qDebug("%d", t.elapsed());
    t.restart();

    for(int i = 0; i < blah; i++) {
	Rect rect;
	SetRect(&rect, 20, 20, 500, 500);
	OpenRgn();
	FrameRect(&rect);
	CloseRgn(foos[i]);
    }
    qDebug("%d", t.elapsed());
    t.restart();

    RgnHandle tst = NewRgn();
    for(int x = 0; x < 500; x+=20) {
	for(int y = 0; y < 500; y+=20) {
	    Rect rect;
	    SetRect(&rect, x, y, 30, 30);
	    OpenRgn();
	    FrameRect(&rect);
	    CloseRgn(tst);
	}
	SectRgn(foos[0], tst, foos[0]);
    }
    DisposeRgn(tst);
    qDebug("%d", t.elapsed());
    t.restart();
    
    for(int i = 1; i < blah; i++) 
	SectRgn(foos[i], foos[0], foos[i]);
    qDebug("%d", t.elapsed());
    t.restart();

    for(int i = 1; i < blah; i++) 
	SetEmptyRgn(foos[i]);
    qDebug("%d", t.elapsed());
    t.restart();
    
    for(int i = 1; i < blah; i++) 
	SectRgn(foos[i], foos[0], foos[i]);
    qDebug("%d", t.elapsed());
    t.restart();

    for(int i = 0; i < blah; i++) 
	DisposeRgn(foos[i]);
    qDebug("%d", t.elapsed());
    t.restart();
    
    return 1;
}
	

