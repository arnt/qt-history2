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
	DrawText(di.hdcDraw, _T("ATL 2.0"), -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	return S_OK;
}
