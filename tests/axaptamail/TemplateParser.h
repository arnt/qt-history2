// TemplateParser.h : Declaration of the CTemplateParser

#ifndef __TEMPLATEPARSER_H_
#define __TEMPLATEPARSER_H_

#include "resource.h"       // main symbols

#include "qmap.h"
#include "qstring.h"

/////////////////////////////////////////////////////////////////////////////
// CTemplateParser
class ATL_NO_VTABLE CTemplateParser : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CTemplateParser, &CLSID_TemplateParser>,
	public IDispatchImpl<ITemplateParser, &IID_ITemplateParser, &LIBID_AXAPTAMAILLib>
{
public:
	CTemplateParser()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_TEMPLATEPARSER)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CTemplateParser)
	COM_INTERFACE_ENTRY(ITemplateParser)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

private:
	QMap<QString,QString> dictionary;
// ITemplateParser
public:
	STDMETHOD(dropDictionary)();
	STDMETHOD(parseTemplate)(BSTR inStream,/*[out,retval]*/BSTR* outStream);
	STDMETHOD(addVariable)(BSTR name,BSTR value);
};

#endif //__TEMPLATEPARSER_H_
