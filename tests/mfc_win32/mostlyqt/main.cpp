#include <qapplication.h>
#include <qmessagebox.h>
#include <qmainwindow.h>
#include <qstatusbar.h>
#include <qlineedit.h>
#include <qvbox.h>

#include <windows.h>

QMainWindow *mainWindow;
static WCHAR wndclass[] = L"qtest";
static WCHAR apptitle[] = L"qtest - left click to see \"modal\" QDialog";

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) 
    {
    case WM_LBUTTONDOWN:
	::SetFocus( hWnd );
	break;
    case WM_SETFOCUS:
	mainWindow->statusBar()->message( "SetFocus for Win32 window!" );
	break;
    case WM_KILLFOCUS:
	mainWindow->statusBar()->message( "KillFocus for Win32 window!" );
	break;
    case WM_KEYDOWN:
	mainWindow->statusBar()->message( "Key Pressed!" );
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
    
    wcex.style			= CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc	= (WNDPROC)WndProc;
    wcex.cbClsExtra		= 0;
    wcex.cbWndExtra		= 0;
    wcex.hInstance		= hInstance;
    wcex.hIcon			= NULL;
    wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName	= NULL;
    wcex.lpszClassName	= wndclass;
    wcex.hIconSm		= NULL;
    
    RegisterClassEx(&wcex);
    
    // Perform application initialization:
    int argc = 0;
    QApplication a( argc, 0 );
    
    QMainWindow mw;
    mainWindow = &mw;
    mw.menuBar();
    mw.statusBar();
    QVBox vbox( &mw );
    mw.setCentralWidget( &vbox );
    QLineEdit edit( &vbox );
    QWidget host( &vbox );
    mw.show();
    
    HWND hWnd = CreateWindow(wndclass, apptitle, WS_CHILD|WS_CLIPSIBLINGS,
	CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, host.winId(), NULL, hInstance, NULL);
    SetWindowPos( hWnd, HWND_TOP, 0, 0, host.width(), host.height(), SWP_SHOWWINDOW );
    
    if (!hWnd)
	return FALSE;
      
    a.setMainWidget( &mw );
    
    return a.exec();
}
