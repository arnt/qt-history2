// QActiveX.cpp : Implementation of CQActiveX

#include "stdafx.h"
#include "ActiveQtEXE.h"
#include "QActiveX.h"

/////////////////////////////////////////////////////////////////////////////
// CQActiveX


QActiveX::QActiveX()
{
	m_bWindowOnly = true;
}

HRESULT QActiveX::OnDraw(ATL_DRAWINFO& di)
{
	RECT rcWindow;

	if ( !m_bWidgetReady )
	{
		// Get our parent
		m_hWndParent = ::GetParent( m_hWnd );

		GetWindowRect( &rcWindow );		// We are the ActiveX window, so we need our size
		ScreenToClient( &rcWindow );	// Translate to parents client system

		setGeometry( rcWindow.left, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top );

		// Set our Qt window to be our sibling
		::SetParent( winId(), m_hWndParent );
		// And make sure that it is displayed in front of us
		::SetWindowPos( winId(), m_hWnd, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE );
		// Let the Qt widget to its initialization
		InitWidget();
	}
	return S_OK;
}

LRESULT QActiveX::OnMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int x,y;
	if ( m_bWidgetReady )
	{
		x = LOWORD( lParam );
		y = HIWORD( lParam );
		move( x, y );
	}

	return 0;
}

/*
** This handler function is called in response to resize events from the Windows
** system, and not from the Qt system.  It should be safe to call resize() on
** ourself, as that is a resize event in the Qt system.
*/
LRESULT QActiveX::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int w,h;
	if ( m_bWidgetReady )
	{
		w = LOWORD( lParam );
		h = HIWORD( lParam );
		resize( w, h );
	}
	return 0;
}

void QActiveX::ScreenToClient(RECT *pRect)
{
	POINT ptUL, ptLR;

	ptUL.x = pRect->left;
	ptUL.y = pRect->top;
	ptLR.x = pRect->right;
	ptLR.y = pRect->bottom;

	// Translate the rect to client coordinates (we are a child window... )
	::ScreenToClient( m_hWndParent, &ptUL );
	::ScreenToClient( m_hWndParent, &ptLR );

	pRect->left = ptUL.x;
	pRect->top = ptUL.y;
	pRect->right = ptLR.x;
	pRect->bottom = ptLR.y;
}

