#include <qapplication.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <windows.h>

HINSTANCE hInst;
static WCHAR wndclass[] = L"qtest";
static WCHAR apptitle[] = L"qtest - left click to see \"modal\" QDialog";

class HwndWidget : public QWidget
{
    HWND hParentWnd;
public:
    HwndWidget(HWND hWnd)
	: QWidget(0, 0, WStyle_NoBorder | WStyle_Customize)
	, hParentWnd(hWnd)
    {
	// make the widget window style be WS_CHILD so SetParent will work
	SetWindowLong(winId(), GWL_STYLE, GetWindowLong(winId(), GWL_STYLE) | WS_CHILD);
	SetParent(winId(), hParentWnd);
	// don't want the widget to actually draw anything
	setBackgroundMode(NoBackground);
	// make widget cover the parent window so owned dialgs will be properly positioned
	RECT r;
	GetWindowRect(hParentWnd, &r);
	setGeometry((r.right-r.left)/2, (r.bottom-r.top)/2,0,0);
	// disable the parent window so dialog will be modal
	EnableWindow(hParentWnd, FALSE);
	// pretent to be visible so that child dialogs are placed correctly
	setWState( WState_Visible );
    }
    ~HwndWidget()
    {
	// re-enable parent window now that dialog is done
	EnableWindow(hParentWnd, TRUE);
	// for some reason, focus is lost, so get it back
	SetFocus(hParentWnd);
    }

    void resetFstrut()
    {
	topData()->ftop = 0;
	topData()->fleft = 0;
	topData()->fbottom = 0;
	topData()->fright = 0;
    }
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) 
    {
    case WM_LBUTTONUP:
	{
	    HwndWidget w(hWnd);
	    QMessageBox mb( "qtest",
		"Is this dialog modal?",
		QMessageBox::NoIcon,
		QMessageBox::Yes | QMessageBox::Default,
		QMessageBox::No  | QMessageBox::Escape,
		QMessageBox::NoButton, &w);
	    int result = mb.exec();
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
    
    QPushButton *pb = new QPushButton( "Qt command button", 0 );
    ::SetWindowLong( pb->winId(), GWL_STYLE, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS );
    ::SetParent( pb->winId(), hWnd );
    ((HwndWidget*)pb)->resetFstrut();
    pb->move( 0, 0 );
    pb->show();
    
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return a.exec();
}
