// TestControl.cpp : Implementation of CTestControl

#include "stdafx.h"
#include "AtlTest.h"
#include "TestControl.h"

/////////////////////////////////////////////////////////////////////////////
// CTestControl


HRESULT CTestControl::OnDraw(ATL_DRAWINFO& di)
{
    RECT& rc = *(RECT*)di.prcBounds;
    Rectangle(di.hdcDraw, rc.left, rc.top, rc.right, rc.bottom);

    SetTextAlign(di.hdcDraw, TA_CENTER|TA_BASELINE);
    LPCTSTR pszText = _T("ATL 3.0 : TestControl");
    TextOut(di.hdcDraw, 
	    (rc.left + rc.right) / 2, 
	    (rc.top + rc.bottom) / 2, 
	    pszText, 
	    lstrlen(pszText));

    return S_OK;
}
