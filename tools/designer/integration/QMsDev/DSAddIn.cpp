/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the Qt integration for Microsoft Visual Studio.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "stdafx.h"
#include "QMsDev.h"
#include "DSAddIn.h"
#include "Commands.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Dieser Code wird beim ersten Laden des Add-Ins und beim Starten der Anwendung aufgerufen
//  jeder nachfolgenden Developer Studio-Sitzung
STDMETHODIMP CDSAddIn::OnConnection(IApplication* pApp, VARIANT_BOOL bFirstTime,
		long dwCookie, VARIANT_BOOL* OnConnection)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// An uns �bergebene Info speichern
	IApplication* pApplication = NULL;
	if (FAILED(pApp->QueryInterface(IID_IApplication, (void**) &pApplication))
		|| pApplication == NULL)
	{
		*OnConnection = VARIANT_FALSE;
		return S_OK;
	}

	m_dwCookie = dwCookie;

	// Befehlsverteilung erzeugen, R�ckmeldung an DevStudio
	CCommandsObj::CreateInstance(&m_pCommands);
	m_pCommands->AddRef();

	// Das obige QueryInterface hat AddRef auf das Objekt Application angewendet. Es
	//  wird im Destruktor von CCommand freigegeben.
	m_pCommands->SetApplicationObject(pApplication);

	// (siehe Definition von VERIFY_OK in stdafx.h)

	VERIFY_OK(pApplication->SetAddInInfo((long) AfxGetInstanceHandle(),
		(LPDISPATCH) m_pCommands, IDR_TOOLBAR_MEDIUM, IDR_TOOLBAR_LARGE, m_dwCookie));

	// DevStudio �ber die implementierten Befehle informieren

	VARIANT_BOOL bRet;
	CString strCmdString;

	LPCTSTR szNewQtProject = _T("New Qt Project");
	strCmdString.LoadString(IDS_NEWQTPROJECT_STRING);
	VERIFY_OK(pApplication->AddCommand(CComBSTR(szNewQtProject + strCmdString), CComBSTR(_T("QMsDevNewQtProject")), 0, m_dwCookie, &bRet));

	LPCTSTR szNewQtDialog = _T("New Qt Dialog");
	strCmdString.LoadString(IDS_NEWQTDIALOG_STRING);
	VERIFY_OK(pApplication->AddCommand(CComBSTR(szNewQtDialog + strCmdString), CComBSTR(_T("QMsDevNewQtDialog")), 1, m_dwCookie, &bRet));

	LPCTSTR szOpenDesigner = _T("Open Qt Designer");
	strCmdString.LoadString(IDS_OPENDESIGNER_STRING);
	VERIFY_OK(pApplication->AddCommand(CComBSTR(szOpenDesigner + strCmdString), CComBSTR(_T("QMsDevStartDesigner")), 2, m_dwCookie, &bRet));
	if ( bFirstTime == VARIANT_TRUE )
	{
	    CComBSTR bszKeystroke("CTRL+SHIFT+D");
	    VERIFY_OK(pApplication->AddKeyBinding(bszKeystroke, CComBSTR(szOpenDesigner), CComBSTR("Text") ));
	    VERIFY_OK(pApplication->AddKeyBinding(bszKeystroke, CComBSTR(szOpenDesigner), CComBSTR("Main") ));
	}

	LPCTSTR szCreateDSP = _T( "Open Qt Project" );
	strCmdString.LoadString( IDS_GENERATEDSP_STRING );
	VERIFY_OK( pApplication->AddCommand( CComBSTR( szCreateDSP + strCmdString ), CComBSTR( _T( "QMsDevCreateDSP" ) ), 3, m_dwCookie, &bRet ) );

	LPCTSTR szGenerateQtProject = _T("Write Qt Project");
	strCmdString.LoadString(IDS_GENERATEQTPROJECT_STRING);
	VERIFY_OK(pApplication->AddCommand(CComBSTR(szGenerateQtProject + strCmdString), CComBSTR(_T("QMsDevGenerateQtProject")), 4, m_dwCookie, &bRet));

	LPCTSTR szUseQt = _T("Add Qt to Project");
  	strCmdString.LoadString(IDS_USEQT_STRING);
  	VERIFY_OK(pApplication->AddCommand(CComBSTR(szUseQt + strCmdString), CComBSTR(_T("QMsDevUseQt")), 5, m_dwCookie, &bRet));

	LPCTSTR szAddMOCStep = _T("Add MOC step");
	strCmdString.LoadString(IDS_ADDMOCSTEP_STRING);
	VERIFY_OK(pApplication->AddCommand(CComBSTR(szAddMOCStep + strCmdString), CComBSTR(_T("QMsDevAddMOCStep")), 6 , m_dwCookie, &bRet));

	if ( bFirstTime == VARIANT_TRUE )
	{
	    CComBSTR bszKeystroke("CTRL+SHIFT+M");
	    VERIFY_OK(pApplication->AddKeyBinding(bszKeystroke, CComBSTR(szAddMOCStep), CComBSTR("Text") ));
	    VERIFY_OK(pApplication->AddKeyBinding(bszKeystroke, CComBSTR(szAddMOCStep), CComBSTR("Main") ));
	}

	if (bRet == VARIANT_FALSE)
	{
		*OnConnection = VARIANT_FALSE;
		return S_OK;
	}

	if (bFirstTime == VARIANT_TRUE)
	{
		VERIFY_OK(pApplication->
			AddCommandBarButton(dsGlyph, CComBSTR(szNewQtProject), m_dwCookie));
		VERIFY_OK(pApplication->
			AddCommandBarButton(dsGlyph, CComBSTR(szNewQtDialog), m_dwCookie));
		VERIFY_OK(pApplication->
			AddCommandBarButton(dsGlyph, CComBSTR(szOpenDesigner), m_dwCookie));
		VERIFY_OK( pApplication->
			AddCommandBarButton( dsGlyph, CComBSTR(szCreateDSP), m_dwCookie ) );
		VERIFY_OK(pApplication->
			AddCommandBarButton(dsGlyph, CComBSTR(szGenerateQtProject), m_dwCookie));
		VERIFY_OK(pApplication->
  			AddCommandBarButton(dsGlyph, CComBSTR(szUseQt), m_dwCookie));
		VERIFY_OK(pApplication->
			AddCommandBarButton(dsGlyph, CComBSTR(szAddMOCStep), m_dwCookie));
	}

	*OnConnection = VARIANT_TRUE;
	return S_OK;
}

// Dieser Code wird bei Shut-Down-Vorg�ngen und beim Entfernen des Add-Ins aus dem Speicher aufgerufen
STDMETHODIMP CDSAddIn::OnDisconnection(VARIANT_BOOL bLastTime)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	m_pCommands->UnadviseFromEvents();
	m_pCommands->Release();
	m_pCommands = NULL;

	// ZU ERLEDIGEN: Hier alle anfallenden Bereinigungsarbeiten durchf�hren

	return S_OK;
}
