/****************************************************************************
** $Id$
**
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt integration for Microsoft Visual Studio.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#if !defined(AFX_COMMANDS_H__BCA8D1B1_12D4_456D_B34A_73C08840E419__INCLUDED_)
#define AFX_COMMANDS_H__BCA8D1B1_12D4_456D_B34A_73C08840E419__INCLUDED_

#include "QMsDevTypes.h"

class CCommands : 
	public CComDualImpl<ICommands, &IID_ICommands, &LIBID_QMsDev>, 
	public CComObjectRoot,
	public CComCoClass<CCommands, &CLSID_Commands>
{
protected:
	IApplication* m_pApplication;
	
	CString getActiveFileName();
	int getActiveProject(CComQIPtr<IBuildProject, &IID_IBuildProject>&);
	int getConfigurations(CComQIPtr<IBuildProject, &IID_IBuildProject>, CComQIPtr<IConfigurations, &IID_IConfigurations>&);
	void addSharedSettings( CComPtr<IConfiguration>, bool useThreads );
	void addStaticSettings( CComPtr<IConfiguration>, bool useThreads );
	void addMOC( CComQIPtr<IBuildProject, &IID_IBuildProject> pProject, CString file );
	void addUIC( CComQIPtr<IBuildProject, &IID_IBuildProject> pProject, CString file );
	CString replaceTemplateStrings( const CString& t, const CString& classheader, 
					const CString& classname, const CString& instance, 
					const CString& instcall, const CString& projekt, 
					const CString& runapp = "return app.exec();" );

public:
	CCommands();
	~CCommands();

	void splitFileName( CString &file, CString &filepath, CString &filetitle, CString& fileext );
	void runDesigner( const CString& file );
	void runLinguist( const CString& file );

	void SetApplicationObject(IApplication* m_pApplication);
	IApplication* GetApplicationObject() { return m_pApplication; }
	void UnadviseFromEvents();

	BEGIN_COM_MAP(CCommands)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(ICommands)
	END_COM_MAP()
	DECLARE_NOT_AGGREGATABLE(CCommands)

protected:
	// Diese Klassenvorlage wird als Basisklasse f�r die
	//  Ereignis-Handler-Objekte "Application" und "Debugger" verwendet,
	//  die weiter unten deklariert werden.
	template <class IEvents, const IID* piidEvents, const GUID* plibid,
		class XEvents, const CLSID* pClsidEvents>
	class XEventHandler :
		public CComDualImpl<IEvents, piidEvents, plibid>,
		public CComObjectRoot,
		public CComCoClass<XEvents, pClsidEvents>
	{
	public:
		BEGIN_COM_MAP(XEvents)
			COM_INTERFACE_ENTRY(IDispatch)
			COM_INTERFACE_ENTRY_IID(*piidEvents, IEvents)
		END_COM_MAP()
		DECLARE_NOT_AGGREGATABLE(XEvents)
		void Connect(IUnknown* pUnk)
		{ VERIFY(SUCCEEDED(AtlAdvise(pUnk, this, *piidEvents, &m_dwAdvise))); }
		void Disconnect(IUnknown* pUnk)
		{ AtlUnadvise(pUnk, *piidEvents, m_dwAdvise); }

		CCommands* m_pCommands;

	protected:
		DWORD m_dwAdvise;
	};

	// Dieses Objekt behandelt vom Objekt Application ausgel�ste Ereignisse
	class XApplicationEvents : public XEventHandler<IApplicationEvents, 
		&IID_IApplicationEvents, &LIBID_QMsDev, 
		XApplicationEvents, &CLSID_ApplicationEvents>
	{
	public:
		// IApplicationEvents-Methoden
		STDMETHOD(BeforeBuildStart)(THIS);
		STDMETHOD(BuildFinish)(THIS_ long nNumErrors, long nNumWarnings);
		STDMETHOD(BeforeApplicationShutDown)(THIS);
		STDMETHOD(DocumentOpen)(THIS_ IDispatch * theDocument);
		STDMETHOD(BeforeDocumentClose)(THIS_ IDispatch * theDocument);
		STDMETHOD(DocumentSave)(THIS_ IDispatch * theDocument);
		STDMETHOD(NewDocument)(THIS_ IDispatch * theDocument);
		STDMETHOD(WindowActivate)(THIS_ IDispatch * theWindow);
		STDMETHOD(WindowDeactivate)(THIS_ IDispatch * theWindow);
		STDMETHOD(WorkspaceOpen)(THIS);
		STDMETHOD(WorkspaceClose)(THIS);
		STDMETHOD(NewWorkspace)(THIS);

	};
	typedef CComObject<XApplicationEvents> XApplicationEventsObj;
	XApplicationEventsObj* m_pApplicationEventsObj;

	// Dieses Objekt behandelt vom Objekt Application ausgel�ste Ereignisse
	class XDebuggerEvents : public XEventHandler<IDebuggerEvents, 
		&IID_IDebuggerEvents, &LIBID_QMsDev, 
		XDebuggerEvents, &CLSID_DebuggerEvents>
	{
	public:
		// IDebuggerEvents-Methode
	    STDMETHOD(BreakpointHit)(THIS_ IDispatch * pBreakpoint);
	};
	typedef CComObject<XDebuggerEvents> XDebuggerEventsObj;
	XDebuggerEventsObj* m_pDebuggerEventsObj;

public:
	STDMETHOD(QMsDevStartAssistant)();
	STDMETHOD(QMsDevStartLinguist)();
// ICommands methods
	STDMETHOD(QMsDevNewQtProject)();
	STDMETHOD(QMsDevNewQtDialog)();
	STDMETHOD(QMsDevStartDesigner)(THIS);
	STDMETHOD(QMsDevCreateDSP)();
	STDMETHOD(QMsDevGenerateQtProject)();
	STDMETHOD(QMsDevUseQt)();
	STDMETHOD(QMsDevAddMOCStep)();
	STDMETHOD(QMsDevAddUICStep)();
};

typedef CComObject<CCommands> CCommandsObj;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ f�gt unmittelbar vor der vorhergehenden Zeile zus�tzliche Deklarationen ein.

#endif // !defined(AFX_COMMANDS_H__BCA8D1B1_12D4_456D_B34A_73C08840E419__INCLUDED)
