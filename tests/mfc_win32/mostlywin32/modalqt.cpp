#include <qapplication.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <windows.h>

HINSTANCE hInst;
static WCHAR wndclass[] = L"qtest";
static WCHAR apptitle[] = L"qtest - left click to see \"modal\" QDialog";

class QWinWidget : public QWidget
{
public:
    QWinWidget( HWND hParentWnd, QObject *parent = 0, const char *name = 0 )
	: QWidget( 0, name ), hParent( hParentWnd )
    {
	if ( parent )
	    parent->insertChild( this );
	// make the widget window style be WS_CHILD so SetParent will work
	SetWindowLong(winId(), GWL_STYLE, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS );
	SetParent(winId(), hParentWnd);
	topData()->ftop = 0;
	topData()->fleft = 0;
	topData()->fbottom = 0;
	topData()->fright = 0;
	
	setBackgroundMode( QWidget::NoBackground );
    }

    void childEvent( QChildEvent *e )
    {
	QObject *obj = e->child();
	if ( e->inserted() ) {
	    if ( obj->isWidgetType() ) {
		QWidget *w = (QWidget*)obj;
		if ( w->isModal() ) {
		    EnableWindow( hParent, FALSE );
		    w->installEventFilter( this );
		}
	    }
	} else {
	    EnableWindow( hParent, TRUE );
	}
    }

protected:
    bool eventFilter( QObject *o, QEvent *e )
    {
	if ( e->type() == QEvent::Hide )
	    EnableWindow( hParent, TRUE );

	return QWidget::eventFilter( o, e );
    }

private:
    HWND hParent;
};


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) 
    {
    case WM_LBUTTONUP:
	{
	    HWND oldFocus = GetFocus();
	    QWinWidget w( hWnd, 0, 0 );

	    RECT r;
	    GetWindowRect(hWnd, &r);
	    w.setGeometry((r.right-r.left)/2, (r.bottom-r.top)/2,0,0);
	    w.show();

	    QMessageBox mb( "qtest",
		"Is this dialog modal?",
		QMessageBox::NoIcon,
		QMessageBox::Yes | QMessageBox::Default,
		QMessageBox::No  | QMessageBox::Escape,
		QMessageBox::NoButton, &w);
	    int result = mb.exec();

	    SetFocus( oldFocus );
	}
	break;

    case WM_DESTROY:
	PostQuitMessage(0);
	break;

    default:
	return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int APIENTRY wWinMain(HINSTANCE hInstance,
		      HINSTANCE hPrevInstance,
		      LPTSTR    lpCmdLine,
		      int       nCmdShow)
{
    WNDCLASSEX wcex;
    
    wcex.cbSize = sizeof(WNDCLASSEX); 
    
    wcex.style		= CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc	= (WNDPROC)WndProc;
    wcex.cbClsExtra	= 0;
    wcex.cbWndExtra	= 0;
    wcex.hInstance	= hInstance;
    wcex.hIcon		= NULL;
    wcex.hCursor	= LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName	= NULL;
    wcex.lpszClassName	= wndclass;
    wcex.hIconSm	= NULL;
    
    RegisterClassEx(&wcex);
    
    // Perform application initialization:
    HWND hWnd;
    
    hInst = hInstance; // Store instance handle in our global variable
    
    hWnd = CreateWindow(wndclass, apptitle, WS_OVERLAPPEDWINDOW,
	CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
    if (!hWnd)
	return FALSE;
    
    int argc = 0;
    QApplication a( argc, 0 );

    QWinWidget win( hWnd );
    QHBoxLayout hbox( &win );
    hbox.setAutoAdd( TRUE );

    win.move( 0, 0 );
    QPushButton *pb = new QPushButton( "Qt command button", &win );
    win.show();
    
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return a.exec();
}
