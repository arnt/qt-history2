/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#if defined(Q_CC_MSVC)
#pragma warning(disable:4305) // init: truncation from const double to float
#endif

#include "glinfo.h"

#include "qstring.h"

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>

LONG WINAPI
WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static PAINTSTRUCT ps;
	
	switch(uMsg) {
	case WM_PAINT:
		BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		return 0;
	
	case WM_SIZE:
		glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
		PostMessage(hWnd, WM_PAINT, 0, 0);
		return 0;
	
	case WM_CHAR:
		switch (wParam) {
		case 27:/* ESC key */
			PostQuitMessage(0);
			break;
		}
		return 0;
		
		case WM_CLOSE:
			PostQuitMessage(0);
			return 0;
		}
	
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

HWND CreateOpenGLWindow(char* title, int x, int y, int width, int height,
						BYTE type, DWORD flags)
{
    int         pf;
    HDC         hDC;
    HWND        hWnd;
    WNDCLASS    wc;
    PIXELFORMATDESCRIPTOR pfd;
    static HINSTANCE hInstance = 0;
	
	/* only register the window class once - use hInstance as a flag. */
    if (!hInstance) {
		hInstance = GetModuleHandle(NULL);
		wc.style         = CS_OWNDC;
		wc.lpfnWndProc   = (WNDPROC)WindowProc;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.hInstance     = hInstance;
		wc.hIcon         = LoadIcon(NULL, IDI_WINLOGO);
		wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName  = NULL;
		wc.lpszClassName = LPCTSTR("OpenGL");
		
		if (!RegisterClass(&wc)) {
			MessageBox(NULL, LPCTSTR("RegisterClass() failed:  Cannot register window class."), LPCTSTR("Error"), MB_OK);
			return NULL;
		}
    }     

	hWnd = CreateWindow(LPCTSTR("OpenGL"), LPCTSTR(title), WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
						x, y, width, height, NULL, NULL, hInstance, NULL);
	
	if (hWnd == NULL) {
		MessageBox(NULL, LPCTSTR("CreateWindow() failed:  Cannot create a window."),
					LPCTSTR("Error"), MB_OK);
		return NULL;
    }

	hDC = GetDC(hWnd);
	
	/* there is no guarantee that the contents of the stack that become
		the pfd are zeroed, therefore _make sure_ to clear these bits. */
    memset(&pfd, 0, sizeof(pfd));
    pfd.nSize        = sizeof(pfd);
    pfd.nVersion     = 1;
    pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | flags;
    pfd.iPixelType   = type;
    pfd.cColorBits   = 32;
	
	pf = ChoosePixelFormat(hDC, &pfd);
    if (pf == 0) {
		MessageBox(NULL, LPCTSTR("ChoosePixelFormat() failed:  Cannot find a suitable pixel format."), 
					LPCTSTR("Error"), MB_OK);
		return 0;
    }

	if (SetPixelFormat(hDC, pf, &pfd) == FALSE) {
		MessageBox(NULL, LPCTSTR("SetPixelFormat() failed:  Cannot set format specified."), 
			LPCTSTR("Error"), MB_OK);
		return 0;
    }

	DescribePixelFormat(hDC, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
	
	ReleaseDC(hWnd, hDC);
	return hWnd;
}

GLInfo::GLInfo()
{
    infotext = new QString("GLTest:\n");
    viewlist = new QStringList();
	int   i;
    char* s;
    char  t[80];
    char* p;
	HDC hDC;/* device context */
    HGLRC hRC;/* opengl context */
    HWND  hWnd;/* window */
    MSG   msg;/* message */

	hWnd = CreateOpenGLWindow("wglinfo", 0, 0, 100, 100, PFD_TYPE_RGBA, 0);
    if (hWnd == NULL)
		exit(1);
	
	hDC = GetDC(hWnd);
    hRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hRC);
	
	ShowWindow(hWnd, SW_HIDE);

	infotext->sprintf("%sdisplay: N/A\n", (const char*)*infotext);
    infotext->sprintf("%sserver wgl vendor string: N/A\n", (const char*)*infotext);
    infotext->sprintf("%sserver wgl version string: N/A\n", (const char*)*infotext);
    infotext->sprintf("%sserver wgl extensions (WGL_): N/A\n", (const char*)*infotext);
    infotext->sprintf("%sclient wgl version: N/A\n", (const char*)*infotext);
    infotext->sprintf("%sclient wgl extensions (WGL_): none\n", (const char*)*infotext);
    infotext->sprintf("%sOpenGL vendor string: %s\n", (const char*)*infotext, ((const char*)glGetString(GL_VENDOR)));
	qDebug("%s", glGetString(GL_VENDOR));
    infotext->sprintf("%sOpenGL renderer string: %s\n", (const char*)*infotext, glGetString(GL_RENDERER));
    infotext->sprintf("%sOpenGL version string: %s\n", (const char*)*infotext, glGetString(GL_VERSION));
    infotext->sprintf("%sOpenGL extensions (GL_): \n", (const char*)*infotext);

	/* do the magic to separate all extensions with comma's, except
       for the last one that _may_ terminate in a space. */
    i = 0;
    s = (char*)glGetString(GL_EXTENSIONS);
	t[79] = '\0';
    while(*s) {
		t[i++] = *s;
		if(*s == ' ') {
			if (*(s+1) != '\0') {
				t[i-1] = ',';
				t[i] = ' ';
				p = &t[i++];
			} else {       // zoinks! last one terminated in a space! //
				t[i-1] = '\0';
			}
		}
		if(i > 80 - 5) {
			*p = t[i] = '\0';
			infotext->sprintf("%s    %s\n", (const char*)*infotext, t);
			p++;
			i = strlen(p);
			strcpy(t, p);
		}
		s++;
	}
    t[i] = '\0';
    infotext->sprintf("%s    %s.", (const char*)*infotext, t);

	VisualInfo(hDC);	

	PostQuitMessage(0);
    while(GetMessage(&msg, hWnd, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
    }
	
	wglMakeCurrent(NULL, NULL);
    ReleaseDC(hWnd, hDC);
    wglDeleteContext(hRC);
    DestroyWindow(hWnd);
};

QString GLInfo::getText()
{
  return *infotext;
}

QStringList GLInfo::getViewList()
{
  return *viewlist;
}

void GLInfo::VisualInfo(HDC hDC)
{
	QString str;
    int i, maxpf;
    PIXELFORMATDESCRIPTOR pfd;
	
	/* calling DescribePixelFormat() with NULL args return maximum
       number of pixel formats */
    maxpf = DescribePixelFormat(hDC, 0, 0, NULL);
	

	printf("   visual  x  bf lv rg d st  r  g  b a  ax dp st accum buffs  ms \n");
	printf(" id dep cl sp sz l  ci b ro sz sz sz sz bf th cl  r  g  b  a ns b\n");
	printf("-----------------------------------------------------------------\n");
	
	/* loop through all the pixel formats */
    for(i = 1; i <= maxpf; i++) { 
		str = "";
		DescribePixelFormat(hDC, i, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
		
		/* only describe this format if it supports OpenGL */
		if(!(pfd.dwFlags & PFD_SUPPORT_OPENGL))
			continue;
			
		/* other criteria could be tested here for actual pixel format
           choosing in an application:
  
		for (...each pixel format...) {
		
			if (pfd.dwFlags & PFD_SUPPORT_OPENGL &&
			pfd.dwFlags & PFD_DOUBLEBUFFER &&
			pfd.cDepthBits >= 24 &&
			pfd.cColorBits >= 24)
			{
				goto found;
			}
		}
		... not found so exit ...
   
		found:
		... found so use it ...
		*/ 
		
		/* print out the information for this pixel format */
		str.sprintf("0x%02x %d ", i, pfd.cColorBits);
		
		//printf("%2d ", pfd.cColorBits);
		if(pfd.dwFlags & PFD_DRAW_TO_WINDOW)
			str.sprintf("%swindow ", (const char*)str);
		else if(pfd.dwFlags & PFD_DRAW_TO_BITMAP) 
			str.sprintf("%sbitmap ", (const char*)str);
		else 
			str.sprintf("%s. ", (const char*)str);
		
		/* should find transparent pixel from LAYERPLANEDESCRIPTOR */
		str.sprintf("%s0 %2d ", (const char*)str, pfd.cColorBits);
		
		/* bReserved field indicates number of over/underlays */
		if(pfd.bReserved) 
			str.sprintf("%s%d ", (const char*)str, pfd.bReserved);
		else 
			str.sprintf("%s0 ", (const char*)str);
		
		if(pfd.iPixelType == PFD_TYPE_RGBA)
			str.sprintf("%srgba ", (const char*)str);
		else
			str.sprintf("%scolor_index ", (const char*)str);

		str.sprintf("%s%c %c ", (const char*)str, 
								   pfd.dwFlags & PFD_DOUBLEBUFFER ? 'y' : '0',
								   pfd.dwFlags & PFD_STEREO ? 'y' : '0');
		
		if(pfd.cRedBits && pfd.iPixelType == PFD_TYPE_RGBA)
			str.sprintf("%s%d ", (const char*)str, pfd.cRedBits);
		else 
			str.sprintf("%s0 ", (const char*)str); 
		
		if(pfd.cGreenBits && pfd.iPixelType == PFD_TYPE_RGBA)
			str.sprintf("%s%d ", (const char*)str, pfd.cGreenBits);
		else 
			str.sprintf("%s0 ", (const char*)str);
		
		if(pfd.cBlueBits && pfd.iPixelType == PFD_TYPE_RGBA)
			str.sprintf("%s%d ", (const char*)str, pfd.cBlueBits);
		else 
			str.sprintf("%s0 ", (const char*)str);
		
		if(pfd.cAlphaBits && pfd.iPixelType == PFD_TYPE_RGBA)
			str.sprintf("%s%d ", (const char*)str, pfd.cAlphaBits);
		else 
			str.sprintf("%s0 ", (const char*)str);
		
		if(pfd.cAuxBuffers)     
			str.sprintf("%s%d ", (const char*)str, pfd.cAuxBuffers);
		else 
			str.sprintf("%s0 ", (const char*)str);
		
		if(pfd.cDepthBits)      
			str.sprintf("%s%d ", (const char*)str, pfd.cDepthBits);
		else 
			str.sprintf("%s0 ", (const char*)str);
		
		if(pfd.cStencilBits)
			str.sprintf("%s%d ", (const char*)str, pfd.cStencilBits);
		else 
			str.sprintf("%s0 ", (const char*)str);
		
		if(pfd.cAccumRedBits)
			str.sprintf("%s%d ", (const char*)str, pfd.cAccumRedBits);
		else 
			str.sprintf("%s0 ", (const char*)str);
		
		if(pfd.cAccumGreenBits)
			str.sprintf("%s%d ", (const char*)str, pfd.cAccumGreenBits);
		else 
			str.sprintf("%s0 ", (const char*)str);
		
		if(pfd.cAccumBlueBits)
			str.sprintf("%s%d ", (const char*)str, pfd.cAccumBlueBits);
		else 
			str.sprintf("%s0 ", (const char*)str);
		
		if(pfd.cAccumAlphaBits)
			str.sprintf("%s%d ", (const char*)str, pfd.cAccumAlphaBits);
		else 
			str.sprintf("%s0 ", (const char*)str);

		/* no multisample in Win32 */
		str.sprintf("%s0 0", (const char*)str);

		viewlist->append(str);
	}
	
	/* print table footer */
	printf("-----------------------------------------------------------------\n");
	printf("   visual  x  bf lv rg d st  r  g  b a  ax dp st accum buffs  ms \n");
	printf(" id dep cl sp sz l  ci b ro sz sz sz sz bf th cl  r  g  b  a ns b\n");
	printf("-----------------------------------------------------------------\n");
}