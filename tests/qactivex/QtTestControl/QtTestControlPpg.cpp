// QtTestControlPpg.cpp : Implementation of the CQtTestControlPropPage property page class.

#include "stdafx.h"
#include "QtTestControl.h"
#include "QtTestControlPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CQtTestControlPropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CQtTestControlPropPage, COlePropertyPage)
	//{{AFX_MSG_MAP(CQtTestControlPropPage)
	// NOTE - ClassWizard will add and remove message map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CQtTestControlPropPage, "QTTESTCONTROL.QtTestControlPropPage.1",
	0xb8c35682, 0x26b9, 0x453d, 0x8b, 0xe7, 0xa8, 0xb8, 0xfc, 0x6d, 0xce, 0xef)


/////////////////////////////////////////////////////////////////////////////
// CQtTestControlPropPage::CQtTestControlPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CQtTestControlPropPage

BOOL CQtTestControlPropPage::CQtTestControlPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_QTTESTCONTROL_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CQtTestControlPropPage::CQtTestControlPropPage - Constructor

CQtTestControlPropPage::CQtTestControlPropPage() :
	COlePropertyPage(IDD, IDS_QTTESTCONTROL_PPG_CAPTION)
{
	//{{AFX_DATA_INIT(CQtTestControlPropPage)
	// NOTE: ClassWizard will add member initialization here
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA_INIT
}


/////////////////////////////////////////////////////////////////////////////
// CQtTestControlPropPage::DoDataExchange - Moves data between page and properties

void CQtTestControlPropPage::DoDataExchange(CDataExchange* pDX)
{
	//{{AFX_DATA_MAP(CQtTestControlPropPage)
	// NOTE: ClassWizard will add DDP, DDX, and DDV calls here
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA_MAP
	DDP_PostProcessing(pDX);
}


/////////////////////////////////////////////////////////////////////////////
// CQtTestControlPropPage message handlers
